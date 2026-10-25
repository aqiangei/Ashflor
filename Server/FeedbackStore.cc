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
};
constexpr char const *kModKey = "aqa";
std::deque<Entry> entries;
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
bool parse_line(std::string const &line, Entry &e, uint32_t previous) {
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
        } else {
            return false;
        }
    }
    return true;
}
// Best-effort: keep valid prefix/lines so one corrupt row cannot wipe the board.
bool parse(std::string const &raw, std::deque<Entry> &out) {
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
        if (!parse_line(line, e, previous)) {
            ++bad;
            continue;
        }
        previous = e.id;
        out.push_back(std::move(e));
        if (out.size() > max_entries) out.pop_front();
    }
    if (bad) std::cerr << "[feedback] skipped " << bad << " corrupt line(s), kept " << out.size() << '\n';
    // Empty file is ok; totally unreadable non-empty file is not.
    return bad == 0 || !out.empty() || raw.find_first_not_of(" \t\r\n") == std::string::npos;
}
std::string serialize() {
    std::ostringstream out;
    for (auto const &e : entries)
        out << e.id << ' ' << e.timestamp << ' ' << std::quoted(e.author)
            << ' ' << std::quoted(e.text) << " ip:" << e.source
            << " done:" << (unsigned) e.handled
            << " fix:" << (unsigned) e.resolved << '\n';
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
}

void FeedbackStore::load() {
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
        if (same_user && age < minute) return kFeedbackTooFast;
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
    if (user10 >= 3 || user24 >= 10 || ip10 >= 10 || ip24 >= 40) return kFeedbackQuota;
    if (all1 >= 6 || all60 >= 60 || all24 >= 200) return kFeedbackBusy;
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
            + ",\"resolved\":" + (i->resolved ? "1" : "0") + '}';
        last = i->id;
    }
    return out + "],\"next_before\":" + std::to_string(more ? last : 0) + '}';
}

std::string FeedbackStore::auth_json(std::string const &key) {
    if (key != kModKey)
        return "{\"ok\":false,\"msg\":\"密钥错误\"}";
    return "{\"ok\":true,\"msg\":\"已验证\"}";
}

std::string FeedbackStore::set_handled_json(uint32_t id, uint8_t handled, std::string const &key) {
    if (key != kModKey)
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
