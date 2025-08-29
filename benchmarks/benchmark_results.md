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

We should re-run this benchmark after future optimisations like the following:
1. Update volcano iterator model to vectorized model (batch processing)

### Benchmark_SelectWhereClause_1M_Rows

```
-------------------------------------------------------------------------------------------------------------------------
Benchmark                                                               Time             CPU   Iterations UserCounters...
-------------------------------------------------------------------------------------------------------------------------
Benchmark_SelectWhereClause_1M_Rows/rowId:1/iterations:3        2668543236 ns   2665045667 ns            3 SelectQueryTime_seconds=2.66854 size = 23879680 bytes
Benchmark_SelectWhereClause_1M_Rows/rowId:10/iterations:3       2674856042 ns   2672053667 ns            3 SelectQueryTime_seconds=2.67485 size = 23879680 bytes
Benchmark_SelectWhereClause_1M_Rows/rowId:100/iterations:3      2728845917 ns   2723703333 ns            3 SelectQueryTime_seconds=2.72884 size = 23879680 bytes
Benchmark_SelectWhereClause_1M_Rows/rowId:1000/iterations:3     2662023403 ns   2659831667 ns            3 SelectQueryTime_seconds=2.66202 size = 23879680 bytes
Benchmark_SelectWhereClause_1M_Rows/rowId:10000/iterations:3    2900679958 ns   2829217667 ns            3 SelectQueryTime_seconds=2.90068 size = 23879680 bytes
Benchmark_SelectWhereClause_1M_Rows/rowId:100000/iterations:3   2707444347 ns   2699827333 ns            3 SelectQueryTime_seconds=2.70744 size = 23879680 bytes
Benchmark_SelectWhereClause_1M_Rows/rowId:1000000/iterations:3  2775017167 ns   2772454667 ns            3 SelectQueryTime_seconds=2.77502 size = 23879680 bytes
Benchmark_SelectWhereClause_1M_Rows/rowId:10000000/iterations:3 2679569986 ns   2675854333 ns            3 SelectQueryTime_seconds=2.67957 size = 23879680 bytes
```

We should re-run this benchmark after future optimisations like the following:
1. Update volcano iterator model to vectorized model (batch processing)
2. Support for index scan
3. Support for UNIQUE constraints or primary keys (as we are currently doing a full table scan even after finding the row). Although, this should be added as a separate benchmark.
4. Predicate pushdown

