#include "piecetable.h"
#include "low_overhead_bench.h"
#include <iostream>
#include <string>

// Stress test for consecutive insertions and deletions at the beginning
int main()
{
    AL::piece_table pt;
    const int NUM_OPERATIONS = 100000;
    const double cycles_per_sec = bench::estimate_cycles_per_second();

    std::cout << "\n--- Front Operations Stress Test ---" << std::endl;
    std::cout << "This test stresses the worst-case scenario for piece tables:" << std::endl;
    std::cout << "Repeated insertions and deletions at the front." << std::endl;

    // Phase 1: Insert at front repeatedly
    std::cout << "\nPhase 1: Inserting " << NUM_OPERATIONS << " characters at the front..." << std::endl;
    uint64_t start_insert = bench::rdtsc();

    for (int i = 0; i < NUM_OPERATIONS; ++i)
    {
        pt.insert(0, "x");
    }

    uint64_t end_insert = bench::rdtsc();
    uint64_t insert_cycles = end_insert - start_insert;

    std::cout << "Insertion complete:" << std::endl;
    std::cout << "  Cycles: " << insert_cycles << std::endl;
    std::cout << "  Time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(insert_cycles), cycles_per_sec)) << std::endl;
    const double insert_cycles_per_op = bench::cycles_per_op(insert_cycles, NUM_OPERATIONS);
    std::cout << "  Avg per insert: " << insert_cycles_per_op << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(insert_cycles_per_op, cycles_per_sec)) << ")" << std::endl;
    std::cout << "  Length: " << pt.length() << std::endl;

    // Phase 2: Delete from front repeatedly
    std::cout << "\nPhase 2: Deleting " << NUM_OPERATIONS << " characters from the front..." << std::endl;
    uint64_t start_delete = bench::rdtsc();

    for (int i = 0; i < NUM_OPERATIONS; ++i)
    {
        pt.remove(0, 1);
    }

    uint64_t end_delete = bench::rdtsc();
    uint64_t delete_cycles = end_delete - start_delete;

    std::cout << "Deletion complete:" << std::endl;
    std::cout << "  Cycles: " << delete_cycles << std::endl;
    std::cout << "  Time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(delete_cycles), cycles_per_sec)) << std::endl;
    const double delete_cycles_per_op = bench::cycles_per_op(delete_cycles, NUM_OPERATIONS);
    std::cout << "  Avg per delete: " << delete_cycles_per_op << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(delete_cycles_per_op, cycles_per_sec)) << ")" << std::endl;
    std::cout << "  Final length: " << pt.length() << std::endl;

    // Sanity check
    if (pt.length() != 0)
    {
        std::cerr << "ERROR: Expected length 0, got " << pt.length() << std::endl;
        return 1;
    }

    std::cout << "\n[PASSED] Front operations stress test" << std::endl;
    return 0;
}
