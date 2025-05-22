# simple-db

## How to build and run

If the following steps run into build issues because of pkg-config on MacOS, try following the instructions in the [pkg-config (macOS)](#pkg-config-macos) section.

```shell
# 1. Navigate to project root
cd ~/github-personal/

# 2. Configure (generate build files in ./build directory)
cmake . -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build (compile the code using info in ./build directory)
cmake --build build

# 4. Run the executable (it will be inside the build directory)
./build/simple-db
```

## pkg-config (macOS)

If the `cmake -S . -B build` command fails with errors mentioning it cannot find some package like `readline` or `readline.pc` via `pkg-config`, it might be because `pkg-config` isn't searching the directory where Homebrew installed the `readline.pc` file.

1. Install `pkg-config` if you haven't already:
   ```bash
   brew install pkg-config
   ```

2. **Find the `readline.pc` directory:**
   ```bash
     ls /opt/homebrew/lib/pkgconfig | grep readline.pc
     ls $(brew --prefix readline)/lib/pkgconfig | grep readline.pc
   ```

    If either of these commands lists readline.pc, note the full path to the directory containing it (e.g., /opt/homebrew/lib/pkgconfig or /opt/homebrew/opt/readline/lib/pkgconfig).

3. **Export `PKG_CONFIG_PATH`:**
    Replace the path below from the location you found in the previous step. For example,
    ```bash
   export PKG_CONFIG_PATH="/opt/homebrew/opt/readline/lib/pkgconfig:${PKG_CONFIG_PATH}"
    ```

Add the above to `~/.bashrc` or `~/.zshrc` for your convenience.
