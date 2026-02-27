#include "piecetable.h"
#include <chrono>
#include <iostream>
#include <random>
#include <string>

int main()
{
    AL::piece_table pt;
    const int NUM_OPERATIONS = 500'000;
    const int INITIAL_SIZE = 10'000'000;

    std::cout << "\n--- Random Edits Stress Test ---" << std::endl;
    std::cout << "Initializing with " << INITIAL_SIZE << " characters..." << std::endl;

    // Fill with some initial data
    std::string initial_data(INITIAL_SIZE, 'A');
    pt.insert(0, initial_data);

    std::mt19937 rng(42);                             // Fixed seed for reproducibility
    std::uniform_int_distribution<int> op_dist(0, 1); // 0 = Insert, 1 = Delete
    std::uniform_int_distribution<int> char_dist('a', 'z');
    std::uniform_int_distribution<int> len_dist(1, 100);

    std::cout << "Performing " << NUM_OPERATIONS << " random insertions/deletions..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_OPERATIONS; ++i)
    {
        size_t current_len = pt.length();
        int op = op_dist(rng);

        if (op == 0)
        { // INSERT
            // Create random string
            int str_len = len_dist(rng);
            std::string s;
            s.reserve(str_len);
            for (int k = 0; k < str_len; ++k)
                s += (char)char_dist(rng);

            // Random position
            std::uniform_int_distribution<size_t> pos_dist(0, current_len);
            size_t pos = pos_dist(rng);

            pt.insert(pos, s);
        }
        else
        { // DELETE
            if (current_len == 0)
                continue;

            // Random position and length
            std::uniform_int_distribution<size_t> pos_dist(0, current_len - 1);
            size_t pos = pos_dist(rng);

            // Don't delete past end
            size_t max_del = current_len - pos;
            std::uniform_int_distribution<size_t> del_len_dist(1, std::min((size_t)100, max_del));
            size_t len = del_len_dist(rng);

            pt.remove(pos, len);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "\n[Random Edit Statistics]" << std::endl;
    std::cout << "Time Elapsed:     " << diff.count() << " s" << std::endl;
    std::cout << "Avg per Edit:     " << (diff.count() * 1e6 / NUM_OPERATIONS) << " microseconds" << std::endl;
    std::cout << "Final Size:       " << pt.length() << " characters" << std::endl;

    // Sanity check: Retrieve the string to ensure the tree isn't broken
    // (This traverses the whole tree)
    auto string_start = std::chrono::high_resolution_clock::now();
    std::string final_str = pt.to_string();
    auto string_end = std::chrono::high_resolution_clock::now();

    std::cout << "Reconstruction:   " << std::chrono::duration<double>(string_end - string_start).count() << " s" << std::endl;
    std::cout << "Check: String length matches tree size? " << (final_str.length() == pt.length() ? "YES" : "NO") << std::endl;

    return 0;
}
