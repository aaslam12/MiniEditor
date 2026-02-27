#include "piecetable.h"
#include <chrono>
#include <iostream>
#include <string>

// Stress test for consecutive insertions and deletions at the beginning
int main()
{
    AL::piece_table pt;
    const int NUM_OPERATIONS = 100000;

    std::cout << "\n--- Front Operations Stress Test ---" << std::endl;
    std::cout << "This test stresses the worst-case scenario for piece tables:" << std::endl;
    std::cout << "Repeated insertions and deletions at the front." << std::endl;

    // Phase 1: Insert at front repeatedly
    std::cout << "\nPhase 1: Inserting " << NUM_OPERATIONS << " characters at the front..." << std::endl;
    auto start_insert = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; ++i)
    {
        pt.insert(0, "x");
    }

    auto end_insert = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> insert_diff = end_insert - start_insert;

    std::cout << "Insertion complete:" << std::endl;
    std::cout << "  Time: " << insert_diff.count() << " s" << std::endl;
    std::cout << "  Avg per insert: " << (insert_diff.count() * 1e6 / NUM_OPERATIONS) << " us" << std::endl;
    std::cout << "  Length: " << pt.length() << std::endl;

    // Phase 2: Delete from front repeatedly
    std::cout << "\nPhase 2: Deleting " << NUM_OPERATIONS << " characters from the front..." << std::endl;
    auto start_delete = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; ++i)
    {
        pt.remove(0, 1);
    }

    auto end_delete = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delete_diff = end_delete - start_delete;

    std::cout << "Deletion complete:" << std::endl;
    std::cout << "  Time: " << delete_diff.count() << " s" << std::endl;
    std::cout << "  Avg per delete: " << (delete_diff.count() * 1e6 / NUM_OPERATIONS) << " us" << std::endl;
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
