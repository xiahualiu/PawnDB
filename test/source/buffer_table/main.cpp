/**
 * @file main.cpp
 * @brief Buffer table unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/buffer_table.h"
#include <thread>
#include <vector>

namespace PawnDB {

TEST_CASE("BufferTable Basic Operations #1") {
    BufferTable table;
    auto idx = table.request();
    CHECK(idx >= 0);
    table.release(idx);
}

TEST_CASE("BufferTable Multiple Buffers #1") {
    BufferTable table;
    std::vector<tbl_row_t> indices;
    
    // Request multiple buffers
    for(int i = 0; i < 5; i++) {
        indices.push_back(table.request());
    }
    
    // Verify and release
    for(auto idx : indices) {
        CHECK(idx >= 0);
        table[idx][0] = 'a';  // Test buffer access
        table.release(idx);
    }
}

TEST_CASE("BufferTable Concurrent Access #1") {
    BufferTable table;
    std::vector<std::thread> threads;
    
    for(int i = 0; i < 4; i++) {
        threads.emplace_back([&table]() {
            auto idx = table.request();
            table[idx][0] = 'x';
            table.release(idx);
        });
    }
    
    for(auto& t : threads) {
        t.join();
    }
}

TEST_CASE("BufferTable Full #1") {
    BufferTable table;
    std::vector<tbl_row_t> indices;
    
    // Fill buffer table
    for(int i = 0; i < BUFFER_ROWS; i++) {
        indices.push_back(table.request());
    }
    
    // Verify full
    auto idx = table.request();
    CHECK(idx == -1);
    
    // Release all
    for(auto idx : indices) {
        table.release(idx);
    }
}

}  // namespace PawnDB