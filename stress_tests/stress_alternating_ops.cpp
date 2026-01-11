#include "piecetable.h"
#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>

// Stress test for alternating insert/delete pattern
int main()
{
    AL::piece_table pt;
    const int NUM_CYCLES = 50000;

    std::cout << "\n--- Alternating Insert/Delete Stress Test ---" << std::endl;
    std::cout << "Testing rapid alternation between inserts and deletes" << std::endl;
    std::cout << "Cycles: " << NUM_CYCLES << std::endl;

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> char_dist('a', 'z');
    std::uniform_int_distribution<int> len_dist(5, 50);

    auto start = std::chrono::high_resolution_clock::now();

    for (int cycle = 0; cycle < NUM_CYCLES; ++cycle)
    {
        // Insert phase: add random text
        int insert_len = len_dist(rng);
        std::string text;
        text.reserve(insert_len);
        for (int i = 0; i < insert_len; ++i)
        {
            text += static_cast<char>(char_dist(rng));
        }

        size_t pos = pt.length() > 0 ? (rng() % pt.length()) : 0;
        pt.insert(pos, text);

        // Delete phase: remove some text
        if (pt.length() > 0)
        {
            size_t delete_len = std::min(len_dist(rng), static_cast<int>(pt.length()));
            size_t del_pos = pt.length() > delete_len ? (rng() % (pt.length() - delete_len)) : 0;
            pt.remove(del_pos, delete_len);
        }

        if ((cycle + 1) % 10000 == 0)
        {
            std::cout << "  Progress: " << (cycle + 1) << " cycles, length=" << pt.length() << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "\n[Results]" << std::endl;
    std::cout << "Total time: " << diff.count() << " s" << std::endl;
    std::cout << "Avg per cycle: " << (diff.count() * 1e6 / NUM_CYCLES) << " us" << std::endl;
    std::cout << "Final length: " << pt.length() << std::endl;

    // Sanity check: ensure we can still retrieve the string
    auto retrieve_start = std::chrono::high_resolution_clock::now();
    std::string result = pt.to_string();
    auto retrieve_end = std::chrono::high_resolution_clock::now();

    std::cout << "String retrieval time: " << std::chrono::duration<double>(retrieve_end - retrieve_start).count() * 1000.0 << " ms" << std::endl;

    if (result.length() != pt.length())
    {
        std::cerr << "ERROR: Retrieved string length doesn't match!" << std::endl;
        return 1;
    }

    std::cout << "\n[PASSED] Alternating insert/delete stress test" << std::endl;
    return 0;
}
