# C++ Header Files (`.h`, `.hpp`)

Header files are a fundamental part of C++ development, essential for organizing code and enabling the **separate compilation** model.

## Purpose: Declarations, Not Definitions

*   **Primary Role:** To provide **declarations** to the compiler. A declaration tells the compiler *what* something is (its name, type, function signature, class structure) without necessarily providing the full implementation or definition (the *how*).
*   **Why Needed:** When compiling a `.cpp` file (e.g., `main.cpp`), the compiler needs to know about the classes, functions, types, etc., that `main.cpp` uses, even if those things are *defined* (implemented) in other files (like `table.cpp` or `database.cpp`). Header files provide these necessary "forward announcements."

## Common Contents of Header Files

Header files typically contain:

1.  **Class Definitions:**
    ```c++
    class Table {
    public:
        // Method declarations (signatures)
        Table(const std::string& name);
        void addRow(const Row& row);
        const Row& getRow(int index) const;

    private:
        // Member variable declarations
        std::string tableName;
        Schema tableSchema;
        std::vector<Row> rows;
        // Private helper method declarations (optional)
        void resizeIfNeeded();
    }; // Semicolon is important!
    ```
    *Note: The actual code (implementation) for methods like `Table::addRow` usually goes in the corresponding `.cpp` file.*

2.  **Function Prototypes (Declarations):** For non-member functions.
    ```c++
    std::string trimWhitespace(const std::string& input);
    bool isValidDataType(const std::string& typeStr);
    ```
    *The implementation of `trimWhitespace` would go in a `.cpp` file.*

3.  **Enum Definitions:**
    ```c++
    enum class DataType { INTEGER, TEXT };
    ```

4.  **Type Definitions:** Using `typedef` or `using`.
    ```c++
    using DataValue = std::variant<int, std::string>;
    using Row = std::vector<DataValue>;
    typedef std::map<std::string, Table> TableMap; // Older style
    ```

5.  **Constants:** Using `const` or `constexpr`.
    ```c++
    const int MAX_COLUMN_NAME_LENGTH = 64;
    constexpr double PI = 3.1415926535;
    ```

6.  **Template Definitions:** Both declaration and definition for templates usually go in headers.
    ```c++
    template<typename T>
    T add(T a, T b) {
        return a + b;
    }
    ```

7.  **Inline Function Definitions:** Functions defined entirely within the header, marked `inline`. The compiler tries to replace calls with the function body. Often used for short, simple functions defined within a class definition.
    ```c++
    class SimpleCounter {
        int count = 0;
    public:
        inline void increment() { count++; } // Inline definition
        int getValue() const; // Declaration only
    };
    ```

8.  **`extern` Variable Declarations:** To declare global variables defined elsewhere (use sparingly).
    ```c++
    extern int globalErrorCounter; // Definition (int globalErrorCounter = 0;) is in one .cpp file
    ```

## How They Are Used: `#include`

*   Source files (`.cpp`) use the `#include` preprocessor directive to incorporate the declarations from header files.
*   `#include "my_header.h"`: Typically used for headers within your own project. The preprocessor usually searches the current directory first.
*   `#include <standard_header>`: Used for standard library headers or system headers. The preprocessor searches standard system include paths.
*   The preprocessor replaces the `#include` line with the literal text content of the header file before compilation begins.

## Include Guards / `#pragma once`

*   **Problem:** If a `.cpp` file includes header A, and header A includes header B, and the `.cpp` file *also* directly includes header B, the content of header B would be pasted into the `.cpp` file twice without protection, leading to "multiple definition" compiler errors.
*   **Solution:** Headers *must* use include guards to prevent this.
    *   **Traditional Guard:**
        ```c++
        #ifndef MY_UNIQUE_HEADER_GUARD_H
        #define MY_UNIQUE_HEADER_GUARD_H
        // ... header content ...
        #endif // MY_UNIQUE_HEADER_GUARD_H
        ```
    *   **Common Pragma:**
        ```c++
        #pragma once
        // ... header content ...
        ```
    Both ensure the header's content is processed only once per translation unit (`.cpp` file being compiled). `#pragma once` is simpler but slightly less portable to very old/obscure compilers.

## Header Files vs. Source Files (`.cpp`)

| Feature          | Header File (`.h`, `.hpp`)                   | Source File (`.cpp`)                                |
|:-----------------|:---------------------------------------------|:----------------------------------------------------|
| **Primary Role** | Declarations (the "what")                    | Definitions / Implementations (the "how")           |
| **Contents**     | Class definitions, function prototypes, etc. | Function/Method bodies, global variable definitions |
| **Compilation**  | `#include`d (pasted) into `.cpp` files       | Compiled independently into object files (`.o`)     |
| **Purpose**      | Make interfaces known across files           | Provide the actual executable code                  |

## Summary

Header files are the "public face" or "blueprint" of your code modules. They contain declarations necessary for the compiler to understand how different parts of your project interact during the separate compilation process. They typically do not contain executable code definitions (except for templates and inline functions). Proper use of header files and include guards is essential for organizing any C++ project beyond a single file.

Good YouTube video on this topic: https://www.youtube.com/watch?v=9RJTQmK0YPI
