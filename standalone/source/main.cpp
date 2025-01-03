/**
 * @file main.cpp
 * @author Xiahua Liu @xiahualiu
 * @brief Demo application for PawnDB.
 * @version 0.1
 * @date 2025-01-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "pawndb/main_thread.h"
#include "pawndb/schema/demo.h"

using namespace PawnDB;

static __attribute__((no_destroy)) auto database = Database();

int main() {
  auto main_thread = MainThread{database};
  main_thread.start();
}
