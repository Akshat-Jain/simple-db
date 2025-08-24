#include <benchmark/benchmark.h>
#include <algorithm>
#include <string>

static void Benchmark_StringOperations(benchmark::State& state) {
    const int length = state.range(0);
    const int op_type = state.range(1);  // 0=concat, 1=find, 2=replace

    // Setup data
    std::string base_str(length, 'a');
    std::string search_str = "aaa";
    std::string replace_str = "xyz";

    for (auto _ : state) {
        std::string s = base_str;  // Copy for each iteration

        switch (op_type) {
            case 0:  // concatenation
                s += "hello";
                break;
            case 1:  // find
                benchmark::DoNotOptimize(s.find(search_str));
                break;
            case 2:  // replace
                s.replace(0, 3, replace_str);
                break;
        }

        benchmark::DoNotOptimize(s);
    }
}

BENCHMARK(Benchmark_StringOperations)
    ->ArgsProduct({
        {10, 100, 1000},  // string lengths
        {0, 1, 2}         // operation types
    });
