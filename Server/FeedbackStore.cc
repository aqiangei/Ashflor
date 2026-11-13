#include <Server/FeedbackStore.hh>
#include <Server/NodeFs.hh>
#include <Helpers/UTF8.hh>
#include <Shared/Binary.hh>
#include <Shared/ClientAddress.hh>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#ifndef WASM_SERVER
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace {
struct Entry {
    uint32_t id;
    uint64_t timestamp;
    std::string author, text;
    uint64_t source = 0;
    uint8_t handled = 0;  // ops marked as processed
    uint8_t resolved = 0; // author marked issue as fixed
    std::string reply;
    uint64_t reply_timestamp = 0;
    std::string thread_token;
    bool from_aqa = false;
};
constexpr char const *kModKey = "aqa";
std::deque<Entry> entries;
std::deque<Entry> private_entries;
std::string private_key;
std::string operator_key;
bool operator_configured = false;
std::unordered_map<uint64_t, std::deque<uint64_t>> operator_attempts;
std::deque<uint64_t> operator_global_attempts;
std::unordered_map<std::string, uint64_t> operator_sessions;
constexpr char operator_session_file[] = "feedback-operator.sessions";
bool private_writable = false;
constexpr char private_file[] = "feedback-private.chat.dat";
constexpr char private_backup[] = "feedback-private.chat.dat.bak";
constexpr size_t thread_id_size = 32, thread_token_size = 96;
std::unordered_map<std::string, uint64_t> attempts;
std::deque<uint64_t> global_attempts;
std::string last_saved;
uint64_t salt = 0, last_snapshot_hour = UINT64_MAX;
uint32_t next_id = 1;
bool writable = true;
constexpr char file[] = "feedback.dat";
constexpr char backup[] = "feedback.dat.bak";
constexpr char salt_file[] = "feedback-salt.dat";
constexpr size_t max_entries = 5000;
constexpr uint64_t minute = 60000, day = 24 * 60 * minute;
#ifdef ASHFLOR_FEEDBACK_TEST
uint64_t test_time = 1792800000000ull;
#endif

uint64_t now_ms() {
#ifdef ASHFLOR_FEEDBACK_TEST
    return test_time;
#else
    return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
#endif
}
std::string read_file(char const *path) {
#ifdef WASM_SERVER
    char *ptr = node_fs_read_alloc(path);
    if (!ptr) return {};
    std::string result(ptr); std::free(ptr); return result;
#else
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), {});
#endif
}
bool write_atomic(char const *path, std::string const &data) {
#ifdef WASM_SERVER
    return node_fs_write_durable_atomic(path, data.data(), (int)data.size()) != 0;
#else
    std::string temp = std::string(path) + ".tmp";
    int fd = ::open(temp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) return false;
    size_t at = 0;
    while (at < data.size()) {
        auto n = ::write(fd, data.data() + at, data.size() - at);
        if (n <= 0) { ::close(fd); return false; }
        at += (size_t)n;
    }
    bool ok = ::fsync(fd) == 0;
    ok &= ::close(fd) == 0;
    if (!ok || std::rename(temp.c_str(), path) != 0) return false;
    int dir = ::open(".", O_RDONLY | O_DIRECTORY);
    if (dir >= 0) { ::fsync(dir); ::close(dir); }
    return true;
#endif
}
std::string canonical(std::string const &text) {
    if (text.empty() || text.size() > 960 || !UTF8Parser::is_valid_utf8(text)) return {};
    UTF8Parser parser(text);
    uint32_t count = 0, lines = 0;
    std::string result;
    bool space = false;
    size_t offset = 0;
    while (!parser.at_end()) {
        uint32_t width = parser.next_symbol_len(), c = parser.next_symbol();
        if ((width == 2 && c < 0x80) || (width == 3 && c < 0x800)
            || (width == 4 && (c < 0x10000 || c > 0x10ffff))
            || (c >= 0xd800 && c <= 0xdfff) || ++count > 240
            || (c < 0x20 && c != '\n' && c != '\t') || c == 0x7f
            || (c >= 0x200b && c <= 0x200f) || (c >= 0x202a && c <= 0x202e)
            || (c >= 0x2060 && c <= 0x206f) || c == 0xfeff) return {};
        if (c == '\n' && ++lines > 8) return {};
        bool whitespace = c <= 0x20 || c == 0xa0 || c == 0x3000 || (c >= 0x2000 && c <= 0x200a);
        if (whitespace) space = !result.empty();
        else {
            if (space) result += ' ';
            space = false;
            if (c >= 'A' && c <= 'Z') result += (char)(c + 'a' - 'A');
            else result.append(text, offset, width);
        }
        offset += width;
    }
    return result;
}
std::string account_key(std::string name) {
    for (char &c : name) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
    return name;
}
uint64_t source_key(std::string const &source) {
    uint64_t value = 14695981039346656037ull ^ salt;
    for (unsigned char c : ClientAddress::quota_key(source)) value = (value ^ c) * 1099511628211ull;
    return value ? value : 1;
}
bool valid_hex(std::string_view value, size_t length) {
    return value.size() == length &&
        value.find_first_not_of("0123456789abcdef") == std::string_view::npos;
}
std::string_view thread_id(std::string_view token) { return token.substr(0, thread_id_size); }

bool parse_line(std::string const &line, Entry &e, uint32_t previous, bool allow_chat) {
    if (line.empty() || line == "\r") return false;
    std::istringstream in(line);
    e = {};
    if (!(in >> e.id >> e.timestamp >> std::quoted(e.author) >> std::quoted(e.text))
        || e.id <= previous || e.author.empty() || e.text.empty() || e.text.size() > 960
        || !UTF8Parser::is_valid_utf8(e.text) || !UTF8Parser::is_valid_utf8(e.author))
        return false;
    // Reject timestamps that become Invalid Date in JS (breaks feedback.html render).
    if (e.timestamp > 1ull << 53) return false;
    std::string tag;
    while (in >> tag) {
        if (tag.starts_with("ip:")) {
            try {
                size_t n;
                e.source = std::stoull(tag.substr(3), &n);
                if (n != tag.size() - 3) return false;
            } catch (...) { return false; }
        } else if (tag.starts_with("done:")) {
            std::string v = tag.substr(5);
            if (v != "0" && v != "1") return false;
            e.handled = (uint8_t) (v[0] - '0');
        } else if (tag.starts_with("fix:")) {
            // "fix:" is 4 chars — substr(5) made every row with fix:0/1 unreadable.
            std::string v = tag.substr(4);
            if (v != "0" && v != "1") return false;
            e.resolved = (uint8_t) (v[0] - '0');
        } else if (tag == "reply:") {
            if (!e.reply.empty() || !(in >> std::quoted(e.reply)) || canonical(e.reply).empty())
                return false;
        } else if (tag.starts_with("reply_at:")) {
            if (e.reply.empty() || e.reply_timestamp != 0) return false;
            const std::string value = tag.substr(9);
            if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos)
                return false;
            try {
                e.reply_timestamp = std::stoull(value);
            } catch (...) { return false; }
            if (!e.reply_timestamp || e.reply_timestamp > 8640000000000000ull) return false;
        } else if (allow_chat && tag.starts_with("chat:")) {
            if (!e.thread_token.empty() || !valid_hex(std::string_view(tag).substr(5), thread_token_size))
                return false;
            e.thread_token = tag.substr(5);
        } else if (allow_chat && tag == "aqa:1" && !e.thread_token.empty() && !e.from_aqa) {
            e.from_aqa = true;
        } else {
            return false;
        }
    }
    return true;
}
bool quoted_record_open(std::string const &text) {
    bool quoted = false, escaped = false;
    for (char c : text) {
        if (escaped) { escaped = false; continue; }
        if (quoted && c == '\\') { escaped = true; continue; }
        if (c == '"') quoted = !quoted;
    }
    return quoted;
}
// std::quoted preserves literal newlines in text. Read a bounded logical record
// before judging it corrupt, while keeping later rows recoverable on failure.
bool parse(std::string const &raw, std::deque<Entry> &out, bool strict = false) {
    out.clear();
    if (raw.empty()) return true;
    uint32_t previous = 0;
    size_t bad = 0;
    std::istringstream in(raw);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        Entry e{};
        bool valid = parse_line(line, e, previous, strict);
        if (!valid && quoted_record_open(line)) {
            auto const next_line = in.tellg();
            std::string record = line, continuation;
            while (record.size() <= 4096 && std::getline(in, continuation)) {
                if (!continuation.empty() && continuation.back() == '\r') continuation.pop_back();
                if (record.size() + 1 + continuation.size() > 4096) break;
                record += '\n';
                record += continuation;
                if (!quoted_record_open(record)) {
                    valid = parse_line(record, e, previous, strict);
                    break;
                }
            }
            if (!valid && next_line != std::streampos(-1)) {
                in.clear();
                in.seekg(next_line); // Skip only the corrupt first line.
            }
        }
        if (!valid) {
            ++bad;
            continue;
        }
        previous = e.id;
        out.push_back(std::move(e));
        if (out.size() > max_entries) out.pop_front();
    }
    if (bad) std::cerr << "[feedback] skipped " << bad << " corrupt line(s), kept " << out.size() << '\n';
    // Empty file is ok; totally unreadable non-empty file is not.
    return bad == 0 || (!strict && (!out.empty() ||
        raw.find_first_not_of(" \t\r\n") == std::string::npos));
}
std::string serialize(std::deque<Entry> const &rows = entries) {
    std::ostringstream out;
    for (auto const &e : rows) {
        out << e.id << ' ' << e.timestamp << ' ' << std::quoted(e.author)
            << ' ' << std::quoted(e.text) << " ip:" << e.source
            << " done:" << (unsigned) e.handled
            << " fix:" << (unsigned) e.resolved;
        if (!e.reply.empty()) out << " reply: " << std::quoted(e.reply);
        if (e.reply_timestamp) out << " reply_at:" << e.reply_timestamp;
        if (!e.thread_token.empty()) out << " chat:" << e.thread_token;
        if (e.from_aqa) out << " aqa:1";
        out << '\n';
    }
    return out.str();
}
void snapshot(uint64_t now) {
    uint64_t hour = now / (60 * minute);
    if (last_snapshot_hour == hour || last_saved.empty()) return;
#ifdef WASM_SERVER
    node_fs_snapshot(file);
#else
    try {
        std::filesystem::create_directories("backups");
        auto path = "backups/feedback-" + std::to_string(hour) + ".dat";
        if (!write_atomic(path.c_str(), last_saved)) throw 0;
        std::vector<std::filesystem::path> files;
        for (auto const &entry : std::filesystem::directory_iterator("backups")) {
            auto name = entry.path().filename().string();
            if (entry.is_regular_file() && name.starts_with("feedback-") && name.ends_with(".dat")
                && name.substr(9, name.size() - 13).find_first_not_of("0123456789") == std::string::npos)
                files.push_back(entry.path());
        }
        std::sort(files.begin(), files.end());
        while (files.size() > 40) { std::filesystem::remove(files.front()); files.erase(files.begin()); }
    } catch (...) { std::cerr << "[feedback] snapshot failed; primary and recovery copy retained\n"; return; }
#endif
    last_snapshot_hour = hour;
}
std::string json_string(std::string const &s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        if (c == '"' || c == '\\') { out += '\\'; out += (char)c; }
        else if (c < 0x20) { char buf[8]; std::snprintf(buf, sizeof(buf), "\\u%04x", c); out += buf; }
        else out += (char)c;
    }
    return out + '"';
}

void load_private() {
    private_entries.clear();
    private_writable = false;
    private_key = operator_configured ? operator_key : read_file("feedback-private.key");
    while (!private_key.empty() && (private_key.back() == '\n' || private_key.back() == '\r'))
        private_key.pop_back();
    if ((!operator_configured && private_key.size() != 64) || private_key.empty() ||
        private_key.find_first_not_of("0123456789abcdef") != std::string::npos) {
        private_key.clear();
        return;
    }
    std::string primary = read_file(private_file);
    std::string recovery = read_file(private_backup);
    // Keep the old inbox intact for rollback; import its rows only before the
    // first chat save, since they have no browser token to attach to a thread.
    if (primary.empty() && recovery.empty()) {
        primary = read_file("feedback-private.dat");
        recovery = read_file("feedback-private.dat.bak");
    }
    std::deque<Entry> loaded;
    if (!primary.empty() && parse(primary, loaded, true)) {
        private_entries = std::move(loaded);
        private_writable = true;
    } else if (!recovery.empty() && parse(recovery, loaded, true)) {
        private_entries = std::move(loaded);
        private_writable = write_atomic(private_file, recovery);
    } else if (primary.empty() && recovery.empty()) {
        private_writable = true;
    }
}

bool secret_matches(std::string const &key, std::string_view expected) {
    if (expected.empty() || key.size() != expected.size()) return false;
    unsigned difference = 0;
    for (size_t i = 0; i < key.size(); ++i)
        difference |= (unsigned char)key[i] ^ (unsigned char)expected[i];
    return difference == 0;
}

bool moderator_key_matches(std::string const &key) {
    return secret_matches(key, operator_configured ? std::string_view(operator_key) : kModKey);
}

bool save_operator_sessions() {
    std::ostringstream out;
    for (auto const &[token, expires] : operator_sessions) out << token << ' ' << expires << '\n';
    return write_atomic(operator_session_file, out.str());
}

const Entry* latest_in_thread(std::string_view id) {
    for (auto i = private_entries.rbegin(); i != private_entries.rend(); ++i)
        if (thread_id(i->thread_token) == id) return &*i;
    return nullptr;
}

std::string private_message_json(Entry const &entry, std::string const &author) {
    return "{\"id\":" + std::to_string(entry.id) + ",\"timestamp\":" +
        std::to_string(entry.timestamp) + ",\"author\":" + json_string(author) +
        ",\"text\":" + json_string(entry.text) + ",\"fromAqa\":" +
        (entry.from_aqa ? "true" : "false") + ",\"thread\":" +
        json_string(std::string(thread_id(entry.thread_token))) + '}';
}

std::string append_private(Entry entry) {
    if (!private_entries.empty() && private_entries.back().id == UINT32_MAX)
        return "{\"ok\":false,\"msg\":\"私信暂不可用\"}";
    entry.id = private_entries.empty() ? 1 : private_entries.back().id + 1;
    const std::string previous = serialize(private_entries);
    private_entries.push_back(std::move(entry));
    const bool full = private_entries.size() > max_entries;
    Entry oldest;
    if (full) { oldest = std::move(private_entries.front()); private_entries.pop_front(); }
    if ((!previous.empty() && !write_atomic(private_backup, previous)) ||
        !write_atomic(private_file, serialize(private_entries))) {
        private_entries.pop_back();
        if (full) private_entries.push_front(std::move(oldest));
        return "{\"ok\":false,\"msg\":\"私信保存失败，请稍后再试\"}";
    }
    const Entry &saved = private_entries.back();
    return "{\"ok\":true,\"timestamp\":" + std::to_string(saved.timestamp) +
        ",\"message\":" + private_message_json(saved, saved.author) + '}';
}
}

void FeedbackStore::load() {
    operator_attempts.clear();
    operator_global_attempts.clear();
    operator_key = read_file("feedback-operator.key");
    operator_configured = !operator_key.empty();
    while (!operator_key.empty() && (operator_key.back() == '\n' || operator_key.back() == '\r'))
        operator_key.pop_back();
    if (operator_key.size() < 10 || operator_key.size() > 128 ||
        operator_key.find_first_not_of("0123456789abcdef") != std::string::npos)
        operator_key.clear();
    operator_sessions.clear();
    if (!operator_key.empty()) {
        std::istringstream saved(read_file(operator_session_file));
        std::string token;
        uint64_t expires;
        const uint64_t now = now_ms();
        while (operator_sessions.size() < 64 && saved >> token >> expires) {
            if (valid_hex(token, 64) && expires > now && expires - now <= 30 * day)
                operator_sessions.emplace(std::move(token), expires);
        }
    }
    load_private();
    entries.clear(); attempts.clear(); global_attempts.clear(); next_id = 1;
    last_saved.clear(); writable = true; last_snapshot_hour = UINT64_MAX;
    std::string primary = read_file(file), recovery = read_file(backup);
    std::deque<Entry> loaded;
    if ((!primary.empty() && parse(primary, loaded)) || (primary.empty() && recovery.empty())) {
        last_saved = primary; entries = std::move(loaded);
    } else {
        loaded.clear();
        if (!recovery.empty() && parse(recovery, loaded)) {
            entries = std::move(loaded); last_saved = recovery;
            writable = write_atomic(file, recovery);
            std::cerr << "[feedback] recovered from backup\n";
        } else {
            writable = false;
            std::cerr << "[feedback] both data copies invalid; refusing to overwrite\n";
        }
    }
    for (auto const &e : entries) next_id = e.id == UINT32_MAX ? 0 : e.id + 1;
    salt = 0;
    std::istringstream(read_file(salt_file)) >> salt;
    if (!salt) {
        for (auto const &e : entries) if (e.source != 0) {
            writable = false;
            std::cerr << "[feedback] missing quota salt; restore feedback-salt.dat before posting\n";
            return;
        }
        // Private, persistent salt; IP addresses never appear in the public board or data records.
        std::random_device random;
        salt = ((uint64_t)random() << 32) ^ random() ^ now_ms();
        if (!salt) salt = 1;
        writable &= write_atomic(salt_file, std::to_string(salt));
    }
    if (!last_saved.empty()) {
        writable &= write_atomic(backup, last_saved);
        snapshot(now_ms());
    }
}

uint8_t FeedbackStore::post(std::string const &author, std::string const &text, uint32_t &id,
                            std::string const &source) {
    id = 0;
    if (author.empty()) return kFeedbackNeedAccount;
    if (!writable || next_id == 0) return kFeedbackStorageError;
    uint64_t now = now_ms();
    while (!global_attempts.empty() && now >= global_attempts.front() && now - global_attempts.front() >= 1000)
        global_attempts.pop_front();
    if (global_attempts.size() >= 20) return kFeedbackBusy;
    global_attempts.push_back(now);
    if (attempts.size() > 8192) {
        for (auto it = attempts.begin(); it != attempts.end();)
            if (now >= it->second && now - it->second > 10 * minute) it = attempts.erase(it); else ++it;
        if (attempts.size() > 8192) return kFeedbackBusy;
    }
    std::string account = account_key(author), ip = "ip:" + std::to_string(source_key(source));
    for (auto const &key : { "user:" + account, ip }) {
        auto found = attempts.find(key);
        if (found != attempts.end() && (now < found->second || now - found->second < 2000)) return kFeedbackTooFast;
    }
    attempts["user:" + account] = attempts[ip] = now;
    std::string normalized = canonical(text);
    if (normalized.empty()) return kFeedbackInvalid;
    uint64_t network = source_key(source);
    uint32_t user10 = 0, user24 = 0, ip10 = 0, ip24 = 0, all1 = 0, all60 = 0, all24 = 0, duplicates = 0;
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        bool same_user = account_key(it->author) == account, same_ip = it->source != 0 && it->source == network;
        uint64_t age = now >= it->timestamp ? now - it->timestamp : 0;

        if (age >= day) continue;
        ++all24;
        if (age < minute) ++all1;
        if (age < 60 * minute) ++all60;
        if (same_user) { ++user24; if (age < 10 * minute) ++user10; }
        if (same_ip) { ++ip24; if (age < 10 * minute) ++ip10; }
        if (canonical(it->text) == normalized) {
            if (same_user || same_ip) return kFeedbackDuplicate;
            ++duplicates;
        }
    }
    if (duplicates >= 5) return kFeedbackDuplicate;
    if (user10 >= 5 || user24 >= 15 || ip10 >= 20 || ip24 >= 80) return kFeedbackQuota;
    if (all1 >= 15 || all60 >= 150 || all24 >= 500) return kFeedbackBusy;
    entries.push_back({next_id, now, author, text, network});
    bool removed = entries.size() > max_entries;
    Entry oldest;
    if (removed) { oldest = std::move(entries.front()); entries.pop_front(); }
    std::string data = serialize();
    if (!write_atomic(backup, last_saved.empty() ? data : last_saved) || !write_atomic(file, data)) {
        entries.pop_back();
        if (removed) entries.push_front(std::move(oldest));
        return kFeedbackStorageError;
    }
    last_saved = std::move(data);
    id = next_id++;
    snapshot(now);
    return kFeedbackPosted;
}

std::string FeedbackStore::page_json(uint32_t before) {
    // Always serve the in-memory board for reads. Posts still require writable.
    std::string out = "{\"ok\":true,\"entries\":[";
    uint32_t n = 0, last = 0; bool more = false;
    for (auto i = entries.rbegin(); i != entries.rend(); ++i) {
        if (before && i->id >= before) continue;
        if (n == 30) { more = true; break; }
        if (n++) out += ',';
        out += "{\"id\":" + std::to_string(i->id) + ",\"timestamp\":" + std::to_string(i->timestamp)
            + ",\"author\":" + json_string(i->author) + ",\"text\":" + json_string(i->text)
            + ",\"handled\":" + (i->handled ? "1" : "0")
            + ",\"resolved\":" + (i->resolved ? "1" : "0")
            + ",\"reply\":" + json_string(i->reply)
            + ",\"replyTimestamp\":" + std::to_string(i->reply_timestamp) + '}';
        last = i->id;
    }
    return out + "],\"next_before\":" + std::to_string(more ? last : 0) + '}';
}

std::string FeedbackStore::auth_json(std::string const &key) {
    if (!moderator_key_matches(key))
        return "{\"ok\":false,\"msg\":\"密钥错误\"}";
    return "{\"ok\":true,\"msg\":\"已验证\"}";
}

bool FeedbackStore::operator_attempt_allowed(std::string const &source) {
    if (!operator_configured) return true;
    const uint64_t now = now_ms();
    const auto expired = [now](uint64_t at) { return now >= at && now - at >= minute; };
    while (!operator_global_attempts.empty() && expired(operator_global_attempts.front()))
        operator_global_attempts.pop_front();
    if (operator_global_attempts.size() >= 120) return false;
    if (operator_attempts.size() >= 4096) {
        for (auto it = operator_attempts.begin(); it != operator_attempts.end();) {
            if (expired(it->second.back())) it = operator_attempts.erase(it);
            else ++it;
        }
        if (operator_attempts.size() >= 4096) return false;
    }
    auto &recent = operator_attempts[source_key(source)];
    while (!recent.empty() && expired(recent.front())) recent.pop_front();
    if (recent.size() >= 15) return false;
    recent.push_back(now);
    operator_global_attempts.push_back(now);
    return true;
}

std::string FeedbackStore::create_operator_session(std::string const &key) {
    if (operator_key.empty() || !secret_matches(key, operator_key)) return {};
    const uint64_t now = now_ms();
    for (auto it = operator_sessions.begin(); it != operator_sessions.end();) {
        if (it->second <= now) it = operator_sessions.erase(it);
        else ++it;
    }
    if (operator_sessions.size() >= 64) return {};
    static constexpr char digits[] = "0123456789abcdef";
    std::random_device random;
    std::string token;
    token.reserve(64);
    for (int word = 0; word < 8; ++word) {
        uint32_t value = random();
        for (int nibble = 0; nibble < 8; ++nibble) {
            token.push_back(digits[value & 15]);
            value >>= 4;
        }
    }
    if (!operator_sessions.emplace(token, now + 30 * day).second) return {};
    if (save_operator_sessions()) return token;
    operator_sessions.erase(token);
    return {};
}

std::string FeedbackStore::operator_session_key(std::string const &token) {
    if (!valid_hex(token, 64)) return {};
    const auto it = operator_sessions.find(token);
    return it != operator_sessions.end() && it->second > now_ms() ? operator_key : std::string();
}

bool FeedbackStore::revoke_operator_session(std::string const &token) {
    const auto it = operator_sessions.find(token);
    if (it == operator_sessions.end()) return true;
    const uint64_t expires = it->second;
    operator_sessions.erase(it);
    if (save_operator_sessions()) return true;
    operator_sessions.emplace(token, expires);
    return false;
}

std::string FeedbackStore::post_private_json(std::string const &name, std::string const &text,
                                              std::string const &source, std::string const &thread) {
    if (!private_writable || !salt)
        return "{\"ok\":false,\"msg\":\"私信暂不可用\"}";
    if (canonical(text).empty())
        return "{\"ok\":false,\"msg\":\"请输入有效的私信（最多240字）\"}";
    if (!thread.empty()) {
        if (!valid_hex(thread, thread_token_size))
            return "{\"ok\":false,\"msg\":\"无效的私聊凭证\"}";
        if (const Entry *existing = latest_in_thread(thread_id(thread));
            existing && !secret_matches(thread, existing->thread_token))
            return "{\"ok\":false,\"msg\":\"无效的私聊凭证\"}";
    }
    const uint64_t now = now_ms(), network = source_key(source);
    unsigned recent = 0, global_recent = 0;
    for (auto it = private_entries.rbegin(); it != private_entries.rend(); ++it) {
        if (it->from_aqa) continue;
        const uint64_t age = now >= it->timestamp ? now - it->timestamp : 0;
        if (age >= day) continue;
        if (it->source == network) {
            ++recent;
            if (age < 3000) return "{\"ok\":false,\"msg\":\"请三秒后再发私信\"}";
        }
        if (age < minute) ++global_recent;
    }
    if (recent >= 50 || global_recent >= 35)
        return "{\"ok\":false,\"msg\":\"私信次数已达限额，请稍后再试\"}";
    const bool valid_name = name.size() <= 200 && name.find_first_of("\r\n\t") == std::string::npos
        && !canonical(name).empty();
    Entry entry{0, now, valid_name ? name : "游客", text, network};
    entry.thread_token = thread;
    return append_private(std::move(entry));
}

std::string FeedbackStore::private_page_json(std::string const &key, uint32_t before) {
    if (!secret_matches(key, private_key))
        return "{\"ok\":false,\"msg\":\"收件箱密钥错误或未启用\"}";
    std::string out = "{\"ok\":true,\"entries\":[";
    uint32_t count = 0, last = 0, next = 0;
    std::unordered_set<std::string_view> seen;
    std::unordered_map<std::string_view, std::string const*> names;
    for (auto const &entry : private_entries)
        if (!entry.from_aqa && !entry.thread_token.empty())
            names[thread_id(entry.thread_token)] = &entry.author;
    for (auto i = private_entries.rbegin(); i != private_entries.rend(); ++i) {
        const auto id = thread_id(i->thread_token);
        if (!id.empty() && !seen.insert(id).second) continue;
        if (before && i->id >= before) continue;
        if (count == 30) { next = last; break; }
        if (count++) out += ',';
        const auto name = names.find(id);
        out += private_message_json(*i, name == names.end() ? i->author : *name->second);
        last = i->id;
    }
    return out + "],\"next_before\":" + std::to_string(next) + '}';
}

std::string FeedbackStore::private_chat_json(std::string const &thread, uint32_t before,
                                              std::string const &key, bool operator_view) {
    if (operator_view && !secret_matches(key, private_key))
        return "{\"ok\":false,\"msg\":\"收件箱密钥错误或未启用\"}";
    if (!valid_hex(thread, operator_view ? thread_id_size : thread_token_size))
        return "{\"ok\":false,\"msg\":\"无效的私聊凭证\"}";
    const auto id = thread_id(thread);
    const Entry *latest = latest_in_thread(id);
    if (!operator_view && latest && !secret_matches(thread, latest->thread_token))
        return "{\"ok\":false,\"msg\":\"无效的私聊凭证\"}";
    std::string out = "{\"ok\":true,\"entries\":[";
    uint32_t count = 0, last = 0, next = 0;
    for (auto i = private_entries.rbegin(); i != private_entries.rend(); ++i) {
        if (thread_id(i->thread_token) != id || (before && i->id >= before)) continue;
        if (count == 30) { next = last; break; }
        if (count++) out += ',';
        out += private_message_json(*i, i->author);
        last = i->id;
    }
    return out + "],\"next_before\":" + std::to_string(next) + '}';
}

std::string FeedbackStore::reply_private_json(std::string const &thread, std::string const &text,
                                              std::string const &key) {
    if (!secret_matches(key, private_key))
        return "{\"ok\":false,\"msg\":\"收件箱密钥错误或未启用\"}";
    if (!private_writable || !valid_hex(thread, thread_id_size))
        return "{\"ok\":false,\"msg\":\"会话不可用\"}";
    const Entry *latest = latest_in_thread(thread);
    if (!latest) return "{\"ok\":false,\"msg\":\"会话不存在\"}";
    if (canonical(text).empty())
        return "{\"ok\":false,\"msg\":\"请输入有效的回复（最多240字）\"}";
    Entry reply{0, now_ms(), "aqa", text};
    reply.thread_token = latest->thread_token;
    reply.from_aqa = true;
    return append_private(std::move(reply));
}

std::string FeedbackStore::set_handled_json(uint32_t id, uint8_t handled, std::string const &key) {
    if (!moderator_key_matches(key))
        return "{\"ok\":false,\"msg\":\"密钥错误\"}";
    if (!writable)
        return "{\"ok\":false,\"msg\":\"留言板不可写\"}";
    if (id == 0)
        return "{\"ok\":false,\"msg\":\"无效留言\"}";
    handled = handled ? 1 : 0;
    Entry *found = nullptr;
    for (auto &e : entries) {
        if (e.id == id) { found = &e; break; }
    }
    if (!found)
        return "{\"ok\":false,\"msg\":\"留言不存在\"}";
    uint8_t prev = found->handled;
    found->handled = handled;
    std::string data = serialize();
    if (!write_atomic(backup, last_saved.empty() ? data : last_saved) || !write_atomic(file, data)) {
        found->handled = prev;
        return "{\"ok\":false,\"msg\":\"保存失败\"}";
    }
    last_saved = std::move(data);
    snapshot(now_ms());
    return "{\"ok\":true,\"msg\":\"已更新\",\"id\":" + std::to_string(id)
        + ",\"handled\":" + (handled ? "1" : "0") + '}';
}

std::string FeedbackStore::set_reply_json(uint32_t id, std::string const &text,
                                          std::string const &key) {
    if (!moderator_key_matches(key)) return "{\"ok\":false,\"msg\":\"密钥错误\"}";
    if (!writable) return "{\"ok\":false,\"msg\":\"留言板不可写\"}";
    if (canonical(text).empty()) return "{\"ok\":false,\"msg\":\"回复格式无效\"}";
    auto it = std::find_if(entries.begin(), entries.end(), [id](auto const &e) { return e.id == id; });
    if (it == entries.end()) return "{\"ok\":false,\"msg\":\"留言不存在\"}";
    if (it->reply != text) {
        std::string previous = std::exchange(it->reply, text);
        const uint64_t previous_timestamp = std::exchange(it->reply_timestamp, now_ms());
        std::string data = serialize();
        if (!write_atomic(backup, last_saved.empty() ? data : last_saved) || !write_atomic(file, data)) {
            it->reply = std::move(previous);
            it->reply_timestamp = previous_timestamp;
            return "{\"ok\":false,\"msg\":\"保存失败\"}";
        }
        last_saved = std::move(data);
        snapshot(now_ms());
    }
    return "{\"ok\":true,\"reply\":" + json_string(it->reply)
        + ",\"replyTimestamp\":" + std::to_string(it->reply_timestamp) + '}';
}

std::string FeedbackStore::delete_json(uint32_t id, std::string const &key) {
    if (!moderator_key_matches(key)) return "{\"ok\":false,\"msg\":\"密钥错误\"}";
    if (!writable) return "{\"ok\":false,\"msg\":\"留言板不可写\"}";
    auto it = std::find_if(entries.begin(), entries.end(), [id](auto const &e) { return e.id == id; });
    if (it == entries.end()) return "{\"ok\":false,\"msg\":\"留言不存在\"}";
    auto previous = entries;
    entries.erase(it);
    std::string data = serialize();
    if (!write_atomic(backup, last_saved.empty() ? data : last_saved) || !write_atomic(file, data)) {
        entries = std::move(previous);
        return "{\"ok\":false,\"msg\":\"保存失败\"}";
    }
    last_saved = std::move(data);
    snapshot(now_ms());
    return "{\"ok\":true}";
}

uint8_t FeedbackStore::set_resolved(std::string const &author, uint32_t id, uint8_t resolved) {
    if (author.empty()) return kFeedbackNeedAccount;
    if (!writable) return kFeedbackStorageError;
    if (id == 0) return kFeedbackInvalid;
    resolved = resolved ? 1 : 0;
    Entry *found = nullptr;
    for (auto &e : entries) {
        if (e.id == id) { found = &e; break; }
    }
    if (!found) return kFeedbackNotFound;
    if (account_key(found->author) != account_key(author)) return kFeedbackNotOwner;
    uint8_t prev = found->resolved;
    found->resolved = resolved;
    std::string data = serialize();
    if (!write_atomic(backup, last_saved.empty() ? data : last_saved) || !write_atomic(file, data)) {
        found->resolved = prev;
        return kFeedbackStorageError;
    }
    last_saved = std::move(data);
    snapshot(now_ms());
    return kFeedbackPosted;
}
#ifdef ASHFLOR_FEEDBACK_TEST
void FeedbackStore::set_test_time(uint64_t time) { test_time = time; }
#endif
