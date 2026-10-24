#pragma once

#include <arpa/inet.h>
#include <string>
#include <string_view>

namespace ClientAddress {
inline std::string normalize(std::string_view value) {
    while (!value.empty() && value.front() == ' ') value.remove_prefix(1);
    while (!value.empty() && value.back() == ' ') value.remove_suffix(1);
    if (value.size() > 64) return {};
    std::string text(value);
    unsigned char bytes[16];
    char result[INET6_ADDRSTRLEN];
    if (inet_pton(AF_INET, text.c_str(), bytes) == 1) {
        inet_ntop(AF_INET, bytes, result, sizeof(result)); return result;
    }
    if (inet_pton(AF_INET6, text.c_str(), bytes) != 1) return {};
    bool mapped = true;
    for (int i = 0; i < 10; ++i) mapped &= bytes[i] == 0;
    if (mapped && bytes[10] == 255 && bytes[11] == 255)
        inet_ntop(AF_INET, bytes + 12, result, sizeof(result));
    else inet_ntop(AF_INET6, bytes, result, sizeof(result));
    return result;
}
inline bool trusted(std::string const &ip) {
    return ip == "127.0.0.1" || ip == "::1" || ip == "154.219.105.173";
}
// Walk from the actual peer toward the client. Ignore attacker-supplied left hops.
inline std::string resolve(std::string_view peer, std::string_view forwarded) {
    std::string ip = normalize(peer);
    if (ip.empty()) return "unknown";
    for (int hops = 0; trusted(ip) && !forwarded.empty() && hops < 16; ++hops) {
        size_t comma = forwarded.rfind(',');
        std::string next = normalize(comma == std::string_view::npos ? forwarded : forwarded.substr(comma + 1));
        if (next.empty()) break;
        ip = next;
        if (comma == std::string_view::npos) break;
        forwarded = forwarded.substr(0, comma);
    }
    return ip;
}
inline std::string quota_key(std::string_view source) {
    auto ip = normalize(source);
    if (ip.empty()) return "unknown";
    if (ip.find(':') != std::string::npos) {
        unsigned char bytes[16]; char out[INET6_ADDRSTRLEN];
        inet_pton(AF_INET6, ip.c_str(), bytes);
        for (int i = 8; i < 16; ++i) bytes[i] = 0;
        inet_ntop(AF_INET6, bytes, out, sizeof(out));
        return std::string(out) + "/64";
    }
    return ip;
}
}
