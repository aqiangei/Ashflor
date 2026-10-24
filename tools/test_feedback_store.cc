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
    for(int i=0;i<3;++i){assert(post("Alice","message "+std::to_string(i)) == kFeedbackPosted);advance(61000);}
    assert(post("Alice","fourth") == kFeedbackQuota);
    FeedbackStore::load(); assert(post("Alice","restart fourth") == kFeedbackQuota);
    advance(601000); assert(post("Alice","after window") == kFeedbackPosted);

    fixture("daily-quota");
    for(int i=0;i<10;++i){assert(post("Alice","daily "+std::to_string(i)) == kFeedbackPosted);advance(661000);}
    assert(post("Alice","eleventh") == kFeedbackQuota);
    FeedbackStore::load(); assert(post("Alice","daily after restart") == kFeedbackQuota);
    advance(86400001); assert(post("Alice","new day") == kFeedbackPosted);

    fixture("network-quota");
    for(int i=0;i<10;++i){assert(post("User"+std::to_string(i),"ip "+std::to_string(i),"198.51.100.2") == kFeedbackPosted);advance(55000);}
    FeedbackStore::load();
    assert(post("Sockpuppet","ip bypass","::ffff:198.51.100.2") == kFeedbackQuota);
    advance(2100); assert(post("Separate","other network","198.51.100.3") == kFeedbackPosted);
    fixture("ipv6");
    assert(post("A","same IPv6 content","2001:db8:1234:5::1") == kFeedbackPosted);advance(2100);
    assert(post("B","same IPv6 content","2001:db8:1234:5::9999") == kFeedbackDuplicate);

    fixture("global-quota");
    for(int i=0;i<6;++i){assert(post("User"+std::to_string(i),"global "+std::to_string(i),"203.0.113."+std::to_string(i+1)) == kFeedbackPosted);advance(2100);}
    assert(post("Seventh","global limit","203.0.113.99") == kFeedbackBusy);
    FeedbackStore::load(); assert(post("AfterRestart","still global limit","203.0.113.100") == kFeedbackBusy);

    fixture("global-hour");
    for(int i=0;i<60;++i){assert(post("User"+std::to_string(i),"hour "+std::to_string(i),"203.0.113."+std::to_string(i+1)) == kFeedbackPosted);advance(59000);}
    FeedbackStore::load();assert(post("HourLimit","hour full","203.0.113.100") == kFeedbackBusy);
    fixture("global-day");
    for(int i=0;i<200;++i){assert(post("User"+std::to_string(i),"day "+std::to_string(i),"203.0.113."+std::to_string(i+1)) == kFeedbackPosted);advance(61000);}
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
