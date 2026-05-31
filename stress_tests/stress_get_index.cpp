#include "piecetable.h"
#include "low_overhead_bench.h"
#include <iomanip>
#include <iostream>
#include <string>
#include <sys/resource.h>

double get_memory_usage()
{
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0)
    {
        return static_cast<double>(usage.ru_maxrss) / 1024.0;
    }
    return 0.0;
}

int main()
{
    AL::piece_table pt;
    const int num_lines = 1000000; // 1 Million
    const double cycles_per_sec = bench::estimate_cycles_per_second();

    std::cout << "Target: " << num_lines << " pieces (1 million)" << std::endl;

    double mem_before = get_memory_usage();

    uint64_t start_build = bench::rdtsc();
    for (int i = 1; i <= num_lines; ++i)
    {
        // This measures the time to append to buffer AND insert into the Treap
        pt.insert(pt.length(), "Line " + std::to_string(i) + "\n");

        if (i % 2000000 == 0)
        {
            std::cout << "  Progress: " << i << " pieces inserted..." << std::endl;
        }
    }
    uint64_t end_build = bench::rdtsc();

    double mem_after = get_memory_usage();
    uint64_t build_cycles = end_build - start_build;

    size_t total_chars = pt.length();
    double total_mb = static_cast<double>(total_chars) / (1024.0 * 1024.0);
    double cycles_per_mb = total_mb > 0.0 ? (static_cast<double>(build_cycles) / total_mb) : 0.0;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "\n[Tree Insertion Statistics]" << std::endl;
    std::cout << "Total Insertion Cycles: " << build_cycles << std::endl;
    std::cout << "Total Insertion Time:   " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(build_cycles), cycles_per_sec)) << std::endl;
    const double avg_insert_cycles = bench::cycles_per_op(build_cycles, num_lines);
    std::cout << "Avg per Insertion:      " << avg_insert_cycles << " cycles"
              << " (" << bench::format_seconds(bench::cycles_to_seconds(avg_insert_cycles, cycles_per_sec)) << ")" << std::endl;
    std::cout << "Cycles per MB:          " << cycles_per_mb << std::endl;
    std::cout << "Time per MB:            " << bench::format_seconds(bench::cycles_to_seconds(cycles_per_mb, cycles_per_sec)) << std::endl;

    std::cout << "\n[Memory Statistics]" << std::endl;
    std::cout << "Total Characters:     " << total_chars << " (" << total_mb << " MB)" << std::endl;
    std::cout << "Total RAM Used:       " << (mem_after - mem_before) << " MB" << std::endl;

    const int target = num_lines / 2;
    std::cout << "\n[Search Statistics]" << std::endl;
    std::cout << "Searching for index of line " << target << "..." << std::endl;

    uint64_t start_search = bench::rdtsc();
    size_t index = pt.get_index_for_line(target);
    uint64_t end_search = bench::rdtsc();

    uint64_t search_cycles = end_search - start_search;

    std::cout << "Found index:          " << index << std::endl;
    std::cout << "Search cycles:        " << search_cycles << std::endl;
    std::cout << "Search time:          " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(search_cycles), cycles_per_sec)) << std::endl;

    return 0;
}
