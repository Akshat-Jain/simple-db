# simple-db

This is a simple database written in C++. I've read a lot of theory about databases, but I wanted to try implementing one myself.

**Goals:**
1. The primary goal is to learn database concepts by building a simple database from scratch, trying to integrate as many concepts and components as possible.
2. The secondary goal is to learn C++ and its ecosystem, including CMake, Google Test, and other tools. I'm also actively trying to use best practices, and also taking inspiration from other popular C++ projects in that regard.
3. The final goal is to eventually one day someday hopefully rename it to complicated-db 🤣

**Current state:**
1. CREATE TABLE, DROP TABLE, SHOW TABLES commands work
2. INSERT INTO command works, albeit with the limitation that it doesn't support inserting multiple rows at once (yet).
3. Basic SELECT queries (selecting all or a subset of columns)

**Note:**
1. This project isn't inspired by any specific database or book.
2. I am actively using Gemini 2.5 Pro as a learning companion to brainstorm on different approaches for achieving some xyz thing, or to get help with C++ syntax and concepts here and there, etc. With that said, I actively try to ensure that it doesn't just spoon-feed me the entire thing as that'll just defeat the whole purpose of this project.
3. I don't have an "end state" for the database, or a specific set of features I want to implement. I'm taking it one step at a time, and see what all features and components I can add to it along the way. With that said, I actively try to brainstorm with Gemini 2.5 Pro to try and make sure that the code structure would hopefully allow future extensibility, but I guess only future Akshat can tell how that'll turn out.

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

## clang-format

If you want to format the code using `clang-format`, you can run the following command:

```bash
clang-format src/**/*.{cpp,h} include/**/*.h tests/**/*.cpp -i
```

For validating the formatting, you can use:

```bash
clang-format src/**/*.{cpp,h} include/**/*.h tests/**/*.cpp --dry-run -Werror
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
