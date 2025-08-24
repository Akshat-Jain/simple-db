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
