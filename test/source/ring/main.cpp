/**
 * @file main.cpp
 * @brief Ring unit test
 * @version 0.1
 * @date 2025-01-04
 *
 * @copyright Copyright (c) 2025
 *
 */

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#include "doctest/doctest.h"
#include "pawndb/ds/ring.h"

namespace PawnDB {

TEST_CASE("Ring Empty #1") {
    Ring<int, 4> ring;
    CHECK(ring.empty());
    CHECK(!ring.full());
}

TEST_CASE("Ring Put #1") {
    Ring<int, 4> ring;
    ring.put(0);
    CHECK(!ring.empty());
    CHECK(!ring.full());
    CHECK(ring.get() == 0);
}

TEST_CASE("Ring Full #1") {
    Ring<int, 2> ring;
    ring.get();
    ring.get();
    CHECK(ring.full());
}

TEST_CASE("Ring Full #2") {
    Ring<int, 3> ring;
    CHECK(ring.get()==0);
    CHECK(ring.get()==1);
    CHECK(ring.get()==2);
    CHECK(ring.full());
    ring.put(1);
    CHECK(!ring.full());
    CHECK(ring.get()==1);
}

TEST_CASE("Ring Index #1") {
    Ring<int, 3> ring;
    CHECK(ring[0]==0);
}

}  // namespace PawnDB
