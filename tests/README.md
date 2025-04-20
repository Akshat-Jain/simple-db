## How to run tests

```shell
cmake . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build

./build/run_tests

# To run specific test suites:
./build/run_tests --gtest_filter='SanityCheck.*'
```
