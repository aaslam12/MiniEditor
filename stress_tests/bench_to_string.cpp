#include "piecetable.h"
#include "low_overhead_bench.h"
#include <iostream>

int main()
{
    const double cycles_per_sec = bench::estimate_cycles_per_second();

    // Test 1: stress_newlines scenario (100k lines, ~2.5 MB)
    {
        AL::piece_table pt;
        for (int i = 0; i < 100000; ++i)
        {
            std::string line = "This is line number " + std::to_string(i + 1) + "\n";
            pt.insert(pt.length(), line);
        }
        
        uint64_t start = bench::rdtsc();
        std::string result = pt.to_string();
        uint64_t end = bench::rdtsc();

        double mb = result.length() / 1024.0 / 1024.0;
        uint64_t cycles = end - start;
        double cycles_per_mb = mb > 0.0 ? (static_cast<double>(cycles) / mb) : 0.0;
        double cycles_per_byte = result.length() > 0 ? (static_cast<double>(cycles) / result.length()) : 0.0;

        std::cout << "100k lines (~" << mb << " MB):" << std::endl;
        std::cout << "  to_string() cycles: " << cycles << std::endl;
        std::cout << "  to_string() time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(cycles), cycles_per_sec)) << std::endl;
        std::cout << "  Cycles per MB: " << cycles_per_mb << std::endl;
        std::cout << "  Time per MB: " << bench::format_seconds(bench::cycles_to_seconds(cycles_per_mb, cycles_per_sec)) << std::endl;
        std::cout << "  Cycles per byte: " << cycles_per_byte << std::endl;
        std::cout << "  Time per byte: " << bench::format_seconds(bench::cycles_to_seconds(cycles_per_byte, cycles_per_sec)) << std::endl;
    }
    
    // Test 2: smaller scale of stress_get_index (1M pieces, ~12 MB)
    {
        AL::piece_table pt;
        for (int i = 1; i <= 1000000; ++i)
        {
            pt.insert(pt.length(), "Line " + std::to_string(i) + "\n");
        }
        
        uint64_t start = bench::rdtsc();
        std::string result = pt.to_string();
        uint64_t end = bench::rdtsc();

        double mb = result.length() / 1024.0 / 1024.0;
        uint64_t cycles = end - start;
        double cycles_per_mb = mb > 0.0 ? (static_cast<double>(cycles) / mb) : 0.0;
        double cycles_per_byte = result.length() > 0 ? (static_cast<double>(cycles) / result.length()) : 0.0;

        std::cout << "\n1M pieces (~" << mb << " MB):" << std::endl;
        std::cout << "  to_string() cycles: " << cycles << std::endl;
        std::cout << "  to_string() time: " << bench::format_seconds(bench::cycles_to_seconds(static_cast<double>(cycles), cycles_per_sec)) << std::endl;
        std::cout << "  Cycles per MB: " << cycles_per_mb << std::endl;
        std::cout << "  Time per MB: " << bench::format_seconds(bench::cycles_to_seconds(cycles_per_mb, cycles_per_sec)) << std::endl;
        std::cout << "  Cycles per byte: " << cycles_per_byte << std::endl;
        std::cout << "  Time per byte: " << bench::format_seconds(bench::cycles_to_seconds(cycles_per_byte, cycles_per_sec)) << std::endl;
    }
    
    return 0;
}
