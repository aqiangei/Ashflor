#pragma once

#include <cstdint>

#ifdef WASM_SERVER
extern "C" {
    // Caller must free() a non-null result. Null = missing / error.
    char *node_fs_read_alloc(char const *path);
    // 1 = ok, 0 = fail
    int node_fs_write_atomic(char const *path, void const *data, int len);
    int node_fs_write_durable_atomic(char const *path, void const *data, int len);
    void node_fs_snapshot(char const *path);
}
#endif
