# This file contains the benchmark results.

### Benchmark_InsertSelect

```
--------------------------------------------------------------------------------------------------------------
Benchmark                                                    Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------------------------------------------
Benchmark_InsertSelect/numRows:10/iterations:3       258559833 ns    258224333 ns            3 InsertQueryTime_seconds=510.43u SelectQueryTime_seconds=0.256265 size = 4096 bytes
Benchmark_InsertSelect/numRows:100/iterations:3        5099028 ns      5095667 ns            3 InsertQueryTime_seconds=4.49613m SelectQueryTime_seconds=333.014u size = 4096 bytes
Benchmark_InsertSelect/numRows:1000/iterations:3      45797625 ns     45748667 ns            3 InsertQueryTime_seconds=0.0426586 SelectQueryTime_seconds=2.84476m size = 20480 bytes
Benchmark_InsertSelect/numRows:10000/iterations:3    496342639 ns    478422000 ns            3 InsertQueryTime_seconds=0.467834 SelectQueryTime_seconds=0.0279933 size = 200704 bytes
Benchmark_InsertSelect/numRows:100000/iterations:3  4998900889 ns   4832154333 ns            3 InsertQueryTime_seconds=4.70736 SelectQueryTime_seconds=0.289186 size = 2191360 bytes
Benchmark_InsertSelect/numRows:1000000/iterations:3 4.8859e+10 ns   4.7694e+10 ns            3 InsertQueryTime_seconds=46.0184 SelectQueryTime_seconds=2.82478 size = 23879680 bytes
```
