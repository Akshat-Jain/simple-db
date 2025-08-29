# Benchmarks

This directory contains Google Benchmark-based performance tests for simple-db.

## Building

```bash
# Configure and build the benchmark executable
cmake . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target run_benchmarks
```

## Running Benchmarks

### Basic Usage
```bash
# Run all benchmarks
./build/run_benchmarks

# Get help and see all available options
./build/run_benchmarks --help
```

### Filtering Benchmarks
```bash
# Run only benchmarks matching a pattern
./build/run_benchmarks --benchmark_filter=".*10.*"
./build/run_benchmarks --benchmark_filter="Benchmark_StringOperations.*"
```

### Output Formats
```bash
# Console output (default)
./build/run_benchmarks

# JSON output
./build/run_benchmarks --benchmark_format=json

# CSV output
./build/run_benchmarks --benchmark_format=csv

# Save output to file
./build/run_benchmarks --benchmark_format=json --benchmark_out=results.json
```

### Timing Controls
```bash
# Run each benchmark for at least 10 seconds
./build/run_benchmarks --benchmark_min_time=10s

# Run with multiple repetitions for statistical analysis
./build/run_benchmarks --benchmark_repetitions=3
```

### Debugging and Testing
```bash
# List all available benchmarks without running them
./build/run_benchmarks --benchmark_list_tests=true

# Dry run to see what would be executed
./build/run_benchmarks --benchmark_dry_run=true
```

### Useful Combinations
```bash
# Quick test run with shorter duration
./build/run_benchmarks --benchmark_min_time=5s

# Statistical analysis with multiple runs
./build/run_benchmarks --benchmark_repetitions=5 --benchmark_report_aggregates_only=true

# Detailed JSON output for analysis
./build/run_benchmarks --benchmark_format=json --benchmark_out=detailed_results.json --benchmark_repetitions=3
```

### Implementation Notes

1. `state.SetLabel(...)` is used to add custom labels to the benchmark output.
```
state.SetLabel("length=" + std::to_string(state.range(0)) + ",op=" + std::to_string(state.range(1)));

The above shows the results as:
----------------------------------------------------------------------------
Benchmark                                  Time             CPU   Iterations
----------------------------------------------------------------------------
Benchmark_StringOperations/10/0         6.97 ns         6.97 ns     98754285 length=10,op=0
Benchmark_StringOperations/100/0        20.8 ns         20.8 ns     33159954 length=100,op=0
Benchmark_StringOperations/1000/0       58.9 ns         58.8 ns     11792254 length=1000,op=0
Benchmark_StringOperations/10/1         2.13 ns         2.13 ns    327275617 length=10,op=1
Benchmark_StringOperations/100/1        15.8 ns         15.8 ns     45435073 length=100,op=1
Benchmark_StringOperations/1000/1       47.7 ns         47.7 ns     12716637 length=1000,op=1
Benchmark_StringOperations/10/2         5.54 ns         5.54 ns    125589824 length=10,op=2
Benchmark_StringOperations/100/2        28.4 ns         28.4 ns     24889597 length=100,op=2
Benchmark_StringOperations/1000/2       57.1 ns         57.1 ns     12097123 length=1000,op=2
```

2. `Benchmark->ArgNames(...)` is used to name the arguments in the results for better readability.
```
BENCHMARK(Benchmark_StringOperations)
    ->ArgsProduct({
        {10, 100, 1000},  // string lengths
        {0, 1, 2}         // operation types
    })
    ->ArgNames({"length", "operation"});

This will show the results as:
---------------------------------------------------------------------------------------------
Benchmark                                                   Time             CPU   Iterations
---------------------------------------------------------------------------------------------
Benchmark_StringOperations/length:10/operation:0         7.51 ns         7.51 ns     94323097
Benchmark_StringOperations/length:100/operation:0        25.1 ns         25.1 ns     31146549
Benchmark_StringOperations/length:1000/operation:0       59.3 ns         59.3 ns     11609779
Benchmark_StringOperations/length:10/operation:1         2.13 ns         2.13 ns    328817572
Benchmark_StringOperations/length:100/operation:1        15.4 ns         15.4 ns     45417680
Benchmark_StringOperations/length:1000/operation:1       54.3 ns         54.3 ns     12742796
Benchmark_StringOperations/length:10/operation:2         6.01 ns         6.01 ns    118738656
Benchmark_StringOperations/length:100/operation:2        26.0 ns         25.8 ns     24234619
Benchmark_StringOperations/length:1000/operation:2       58.4 ns         58.4 ns     11496707
```