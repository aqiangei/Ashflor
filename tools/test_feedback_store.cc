#include <Server/FeedbackStore.hh>
#include <Shared/Binary.hh>
#include <Shared/ClientAddress.hh>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace {
uint64_t time_ms = 1792800000000ull;
std::filesystem::path base;
uint32_t id;
void advance(uint64_t n) { time_ms += n; FeedbackStore::set_test_time(time_ms); }
void fixture(char const *name) {
    auto path = base / name; std::filesystem::create_directory(path); std::filesystem::current_path(path);
    time_ms = 1792800000000ull; FeedbackStore::set_test_time(time_ms); FeedbackStore::load();
}
uint8_t post(std::string who, std::string text, std::string ip = "192.0.2.1") {
    id = 99; return FeedbackStore::post(who, text, id, ip);
}
std::string read(char const *name) { std::ifstream in(name); return {std::istreambuf_iterator<char>(in), {}}; }
void write(char const *name, std::string const &s) { std::ofstream(name) << s; }
}
int main() {
    base = std::filesystem::current_path();
    fixture("validation");
    assert(post("", "guest") == kFeedbackNeedAccount && id == 0);
    int invalid = 0;
    for (auto const &s : {std::string(" \n\t"), std::string(241,'x'), std::string("bad\x01"),
        std::string("\xc0\xaf"), std::string("\xed\xa0\x80"), std::string("a\xe2\x80\x8b" "b"),
        std::string("1\n2\n3\n4\n5\n6\n7\n8\n9\n10")}) {
        ++invalid;
        assert(post("Invalid" + std::to_string(invalid), s, "192.0.2." + std::to_string(invalid+10)) == kFeedbackInvalid);
        advance(2100);
    }
    std::string content = "公开留言\n<svg onload=alert(1)> & \"quoted\" \\";
    assert(post("Alice", content) == kFeedbackPosted && id == 1);
    assert(std::filesystem::file_size("feedback.dat.bak") > 0);
    assert(post("Alice", "again") == kFeedbackTooFast);
    advance(61000);
    assert(post("Alice", " PUBLIC  issue\t HERE ") == kFeedbackPosted);
    advance(61000);
    assert(post("Alice", "public issue here") == kFeedbackDuplicate);
    advance(2100);
    assert(post("Other", "public issue here") == kFeedbackDuplicate);
    advance(2100);
    std::string emoji; for(int i=0;i<240;++i) emoji += "🌸";
    assert(post("Bob",emoji,"192.0.2.2") == kFeedbackPosted);
    advance(2100);
    assert(post("Carol",emoji+"🌸","192.0.2.3") == kFeedbackInvalid);

    fixture("account-quota");
    for(int i=0;i<5;++i){assert(post("Alice","message "+std::to_string(i)) == kFeedbackPosted);advance(61000);}
    assert(post("Alice","sixth") == kFeedbackQuota);
    FeedbackStore::load(); assert(post("Alice","restart sixth") == kFeedbackQuota);
    advance(601000); assert(post("Alice","after window") == kFeedbackPosted);

    fixture("daily-quota");
    for(int i=0;i<15;++i){assert(post("Alice","daily "+std::to_string(i)) == kFeedbackPosted);advance(661000);}
    assert(post("Alice","sixteenth") == kFeedbackQuota);
    FeedbackStore::load(); assert(post("Alice","daily after restart") == kFeedbackQuota);
    advance(86400001); assert(post("Alice","new day") == kFeedbackPosted);

    fixture("network-quota");
    for(int i=0;i<20;++i){assert(post("User"+std::to_string(i),"ip "+std::to_string(i),"198.51.100.2") == kFeedbackPosted);advance(28000);}
    FeedbackStore::load();
    assert(post("Sockpuppet","ip bypass","::ffff:198.51.100.2") == kFeedbackQuota);
    advance(2100); assert(post("Separate","other network","198.51.100.3") == kFeedbackPosted);
    fixture("ipv6");
    assert(post("A","same IPv6 content","2001:db8:1234:5::1") == kFeedbackPosted);advance(2100);
    assert(post("B","same IPv6 content","2001:db8:1234:5::9999") == kFeedbackDuplicate);

    fixture("global-quota");
    for(int i=0;i<15;++i){assert(post("User"+std::to_string(i),"global "+std::to_string(i),"203.0.113."+std::to_string(i+1)) == kFeedbackPosted);advance(2100);}
    assert(post("Sixteenth","global limit","203.0.113.99") == kFeedbackBusy);
    FeedbackStore::load(); assert(post("AfterRestart","still global limit","203.0.113.100") == kFeedbackBusy);

    fixture("global-hour");
    for(int i=0;i<150;++i){assert(post("User"+std::to_string(i),"hour "+std::to_string(i),"203.0.113."+std::to_string(i+1)) == kFeedbackPosted);advance(23000);}
    FeedbackStore::load();assert(post("HourLimit","hour full","203.0.113.100") == kFeedbackBusy);
    fixture("global-day");
    for(int i=0;i<500;++i){assert(post("User"+std::to_string(i),"day "+std::to_string(i),"203.0."+std::to_string(113+i/250)+"."+std::to_string(i%250+1)) == kFeedbackPosted);advance(165000);}
    FeedbackStore::load();assert(post("DayLimit","day full","203.0.113.250") == kFeedbackBusy);

    fixture("storage");
    assert(post("Alice",content) == kFeedbackPosted);
    auto before = FeedbackStore::page_json(); advance(61000);
    std::filesystem::create_directory("feedback.dat.tmp");
    assert(post("Carol","storage failure","192.0.2.3") == kFeedbackStorageError && id == 0);
    assert(FeedbackStore::page_json() == before);
    std::filesystem::remove("feedback.dat.tmp"); advance(2100);
    assert(post("Carol","retry saved","192.0.2.3") == kFeedbackPosted);
    assert(read("feedback.dat.bak").find("Alice") != std::string::npos);
    assert(read("feedback.dat").find("192.0.2.") == std::string::npos);
    auto good = read("feedback.dat");
    std::filesystem::copy_file("feedback.dat","feedback.dat.bak",std::filesystem::copy_options::overwrite_existing);
    write("feedback.dat","truncated garbage");
    FeedbackStore::load(); assert(read("feedback.dat") == good);
    assert(FeedbackStore::page_json().find("Carol") != std::string::npos);
    write("feedback.dat","broken"); write("feedback.dat.bak","broken backup");
    FeedbackStore::load(); assert(post("Next","must not erase data") == kFeedbackStorageError);
    assert(read("feedback.dat") == "broken");

    fixture("legacy");
    write("feedback.dat","1 1792799000000 \"Legacy\" \"first\\\nline\"\n");
    // Use the original quoted format, including an actual newline inside text.
    write("feedback.dat","1 1792799000000 \"Legacy\" \"first\nline\"\n");
    FeedbackStore::load();assert(FeedbackStore::page_json().find("Legacy") != std::string::npos);
    assert(post("New","compatible save") == kFeedbackPosted);
    auto current = FeedbackStore::page_json(); FeedbackStore::load(); assert(FeedbackStore::page_json() == current);

    fixture("multiline-roundtrip");
    assert(post("Alice", "[来自 https://ashflor.com]\n多行留言：\"引号\"与\\路径\n第二行") == kFeedbackPosted);
    auto multiline = FeedbackStore::page_json();
    FeedbackStore::load(); assert(FeedbackStore::page_json() == multiline);

    fixture("unclosed-legacy");
    write("feedback.dat", "1 1792799000000 \"Bad\" \"unterminated\n"
                          "2 1792799000000 \"Good\" \"kept\"\n");
    FeedbackStore::load();
    assert(FeedbackStore::page_json().find("Good") != std::string::npos);
    assert(FeedbackStore::page_json().find("Bad") == std::string::npos);

    fixture("moderator-reply");
    assert(post("aqa", "A display name cannot authorize replies") == kFeedbackPosted);
    const auto reply_id = id;
    const auto without_reply = FeedbackStore::page_json();
    const auto failed = [](std::string const &result) {
        return result.find("\"ok\":false") != std::string::npos;
    };
    assert(failed(FeedbackStore::set_reply_json(reply_id, "forged", "")));
    assert(failed(FeedbackStore::set_reply_json(reply_id, "forged", "wrong")));
    assert(failed(FeedbackStore::set_reply_json(reply_id, " ", "aqa")));
    assert(failed(FeedbackStore::set_reply_json(reply_id, std::string(241, 'a'), "aqa")));
    assert(failed(FeedbackStore::set_reply_json(reply_id + 1, "missing", "aqa")));
    assert(FeedbackStore::page_json() == without_reply);
    assert(!failed(FeedbackStore::set_reply_json(reply_id, "已修复：\"引号\"\n第二行", "aqa")));
    const auto with_reply = FeedbackStore::page_json();
    assert(with_reply.find("已修复") != std::string::npos);
    FeedbackStore::load();
    assert(FeedbackStore::page_json() == with_reply);
    std::filesystem::create_directory("feedback.dat.tmp");
    assert(failed(FeedbackStore::set_reply_json(reply_id, "failed edit", "aqa")));
    assert(FeedbackStore::page_json() == with_reply);
    std::filesystem::remove("feedback.dat.tmp");
    assert(!failed(FeedbackStore::set_reply_json(reply_id, "已更新", "aqa")));
    FeedbackStore::load();
    assert(FeedbackStore::page_json().find("已更新") != std::string::npos);
    assert(FeedbackStore::page_json().find("第二行") == std::string::npos);

    fixture("private-inbox");
    const std::string inbox_key(64, 'f');
    assert(failed(FeedbackStore::post_private_json("Guest", "private message", "192.0.2.1")));
    write("feedback-private.key", inbox_key + "\n");
    FeedbackStore::load();
    assert(!failed(FeedbackStore::post_private_json("Guest", "private <message>\nsecond line", "192.0.2.1")));
    assert(FeedbackStore::page_json().find("private <message>") == std::string::npos);
    for (auto const &key : {std::string(), std::string("aqa"), std::string(64, 'e')}) {
        const auto denied = FeedbackStore::private_page_json(key);
        assert(failed(denied) && denied.find("private <message>") == std::string::npos);
    }
    const auto inbox = FeedbackStore::private_page_json(inbox_key);
    assert(!failed(inbox) && inbox.find("private <message>") != std::string::npos);
    FeedbackStore::load();
    assert(FeedbackStore::private_page_json(inbox_key) == inbox);
    assert(failed(FeedbackStore::post_private_json("Another name", "retry", "192.0.2.1")));
    advance(61000);
    std::filesystem::create_directory("feedback-private.chat.dat.tmp");
    assert(failed(FeedbackStore::post_private_json("Guest", "disk write failed", "192.0.2.1")));
    assert(FeedbackStore::private_page_json(inbox_key) == inbox);

    fixture("shared-operator-key");
    const std::string operator_key = "9876543210";
    write("feedback-operator.key", operator_key);
    FeedbackStore::load();
    assert(!failed(FeedbackStore::auth_json(operator_key)));
    assert(!failed(FeedbackStore::private_page_json(operator_key)));
    assert(failed(FeedbackStore::auth_json("aqa")));
    assert(failed(FeedbackStore::private_page_json("aqa")));
    for (int n = 0; n < 15; ++n)
        assert(FeedbackStore::operator_attempt_allowed("192.0.2.1"));
    assert(!FeedbackStore::operator_attempt_allowed("192.0.2.1"));
    assert(FeedbackStore::operator_attempt_allowed("192.0.2.2"));
    advance(61000);
    assert(FeedbackStore::operator_attempt_allowed("192.0.2.1"));
    assert(FeedbackStore::create_operator_session("wrong").empty());
    const auto operator_session = FeedbackStore::create_operator_session(operator_key);
    assert(operator_session.size() == 64 && operator_session != operator_key);
    FeedbackStore::load();
    assert(FeedbackStore::operator_session_key(operator_session) == operator_key);
    assert(FeedbackStore::revoke_operator_session(operator_session));
    FeedbackStore::load();
    assert(FeedbackStore::operator_session_key(operator_session).empty());

    fixture("private-conversation");
    write("feedback-operator.key", operator_key);
    write("feedback-private.dat", "1 1792799000000 \"Legacy\" \"legacy private message\" done:0 fix:0\n");
    FeedbackStore::load();
    const std::string sender_token = std::string(32, 'a') + std::string(64, 'b');
    const std::string thread = sender_token.substr(0, 32);
    const std::string different_token = std::string(96, 'c');
    assert(!failed(FeedbackStore::post_private_json("Guest", "private question", "192.0.2.1", sender_token)));
    assert(!failed(FeedbackStore::post_private_json("Other", "another chat", "192.0.2.2", different_token)));
    assert(failed(FeedbackStore::reply_private_json(thread, "forged reply", "")));
    assert(!failed(FeedbackStore::reply_private_json(thread, "aqa answer", operator_key)));
    const auto chat = FeedbackStore::private_chat_json(sender_token, 0, "", false);
    assert(chat.find("private question") != std::string::npos && chat.find("aqa answer") != std::string::npos);
    assert(chat.find("another chat") == std::string::npos && chat.find(sender_token) == std::string::npos);
    assert(failed(FeedbackStore::private_chat_json(thread + std::string(64, 'c'), 0, "", false)));
    assert(failed(FeedbackStore::private_chat_json(thread, 0, "wrong", true)));
    assert(FeedbackStore::private_chat_json(thread, 0, operator_key, true) == chat);
    assert(FeedbackStore::private_page_json(operator_key).find("legacy private message") != std::string::npos);
    assert(FeedbackStore::page_json().find("private question") == std::string::npos);
    FeedbackStore::load();
    assert(FeedbackStore::private_chat_json(sender_token, 0, "", false) == chat);

    fixture("pagination");
    for(int i=0;i<35;++i){assert(post("Player"+std::to_string(i),"message "+std::to_string(i)) == kFeedbackPosted);advance(61000);}
    auto latest = FeedbackStore::page_json(), older = FeedbackStore::page_json(6);
    assert(latest.find("\"id\":35") != std::string::npos && latest.find("\"next_before\":6") != std::string::npos);
    assert(older.find("\"next_before\":0") != std::string::npos);
    FeedbackStore::load(); assert(FeedbackStore::page_json() == latest && FeedbackStore::page_json(6) == older);

    assert(ClientAddress::resolve("198.51.100.9","1.2.3.4") == "198.51.100.9");
    assert(ClientAddress::resolve("127.0.0.1","1.2.3.4, 198.51.100.9, 154.219.105.173") == "198.51.100.9");
    assert(ClientAddress::resolve("::ffff:127.0.0.1","fake, 198.51.100.9") == "198.51.100.9");
    assert(ClientAddress::quota_key("2001:db8::1") == ClientAddress::quota_key("2001:0db8:0:0::ffff"));
    std::puts("PASS: account/IP/global quotas survive restart, normalized duplicates, IPv6 and proxy spoof defense, durable backup recovery, failure rollback, legacy history and pagination");
}
