#include <benchmark/benchmark.h>
#include <algorithm>
#include "simpledb/query_runner.h"

static void Benchmark_InsertSelect(benchmark::State& state) {
    const int numRows = state.range(0);
    std::filesystem::path test_data_dir = std::filesystem::temp_directory_path() / state.name();
    setenv("SIMPLE_DB_DATA_DIR", test_data_dir.c_str(), 1);
    config::init_config();
    catalog::initialize(test_data_dir);

    double sum_insert_query_time = 0.0;
    double sum_select_query_time = 0.0;

    for (auto _ : state) {
        query_runner::QueryRunner::run_query("create table benchmark_table (id INT, name TEXT);");
        // Insert rows
        auto start_insert = std::chrono::high_resolution_clock::now();
        for (int i = 1; i <= numRows; ++i) {
            query_runner::QueryRunner::run_query("insert into benchmark_table (id, name) values (" + std::to_string(i) +
                                                 ", 'Name" + std::to_string(i) + "');");
        }
        auto end_insert = std::chrono::high_resolution_clock::now();
        sum_insert_query_time +=
            std::chrono::duration_cast<std::chrono::duration<double>>(end_insert - start_insert).count();

        // Measure the size of the data file created.
        std::filesystem::path table_file = test_data_dir / "benchmark_table.data";
        if (std::filesystem::exists(table_file)) {
            auto file_size = std::filesystem::file_size(table_file);
            state.SetLabel("size = " + std::to_string(file_size) + " bytes");
        }

        auto start_select = std::chrono::high_resolution_clock::now();
        auto result = query_runner::QueryRunner::run_query("select * from benchmark_table;");
        benchmark::DoNotOptimize(result);
        auto end_select = std::chrono::high_resolution_clock::now();
        sum_select_query_time +=
            std::chrono::duration_cast<std::chrono::duration<double>>(end_select - start_select).count();

        // Clean up
        result = query_runner::QueryRunner::run_query("drop table benchmark_table;");
    }

    state.counters["InsertQueryTime_seconds"] =
        benchmark::Counter(sum_insert_query_time, benchmark::Counter::kAvgIterations);
    state.counters["SelectQueryTime_seconds"] =
        benchmark::Counter(sum_select_query_time, benchmark::Counter::kAvgIterations);
    // todo: Maybe change this to report rows per second?
}

BENCHMARK(Benchmark_InsertSelect)
    ->ArgsProduct({
        {10, 100, 1000, 10000, 100000, 1000000}  // number of rows
    })
    ->Iterations(3)
    ->ArgNames({"numRows"});

// todo: Maybe change it to report rows per second?
// todo: figure out how to log the results in a way that future comparisons are easy to do

/*
Current results:
--------------------------------------------------------------------------------------------------------------
Benchmark                                                    Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------------------------------
Benchmark_InsertSelect/numRows:10/iterations:3       258559833 ns    258224333 ns            3 InsertQueryTime_seconds=510.43u SelectQueryTime_seconds=0.256265 size = 4096 bytes
Benchmark_InsertSelect/numRows:100/iterations:3        5099028 ns      5095667 ns            3 InsertQueryTime_seconds=4.49613m SelectQueryTime_seconds=333.014u size = 4096 bytes
Benchmark_InsertSelect/numRows:1000/iterations:3      45797625 ns     45748667 ns            3 InsertQueryTime_seconds=0.0426586 SelectQueryTime_seconds=2.84476m size = 20480 bytes
Benchmark_InsertSelect/numRows:10000/iterations:3    496342639 ns    478422000 ns            3 InsertQueryTime_seconds=0.467834 SelectQueryTime_seconds=0.0279933 size = 200704 bytes
Benchmark_InsertSelect/numRows:100000/iterations:3  4998900889 ns   4832154333 ns            3 InsertQueryTime_seconds=4.70736 SelectQueryTime_seconds=0.289186 size = 2191360 bytes
Benchmark_InsertSelect/numRows:1000000/iterations:3 4.8859e+10 ns   4.7694e+10 ns            3 InsertQueryTime_seconds=46.0184 SelectQueryTime_seconds=2.82478 size = 23879680 bytes
*/
