//
// Created by Akshat Jain on 29/03/25.
//

#include <iostream>
#include <sstream>
#include <readline/readline.h>
#include <readline/history.h>


std::string handle_create_table(const std::string& query) {
    std::cout << "DEBUG: Placeholder handle_create_table called for: " << query << std::endl;
    // TODO: Parse table name, columns, types
    return "OK (Placeholder - Table creation)";
}

std::string handle_insert(const std::string& query) {
    std::cout << "DEBUG: Placeholder handle_insert called for: " << query << std::endl;
    // TODO: Parse table name, values
    return "OK (Placeholder - Insertion)";
}

std::string handle_select(const std::string& query) {
    std::cout << "DEBUG: Placeholder handle_select called for: " << query << std::endl;
    // TODO: Parse table name, potentially WHERE clause
    return "OK (Placeholder - Selection results would go here)";
}

std::string parse_and_execute(const std::string& query) {
    std::string command;

    // Create an std::stringstream object named 'ss'.
    // Initialize it with the content of the input 'query' string.
    // This allows treating the 'query' string like an input stream (e.g., std::cin).
    std::stringstream ss(query);

    // Use the stream extraction operator (>>) to read from the stringstream 'ss'.
    // By default, >> skips leading whitespace and reads characters until the *next* whitespace.
    // The extracted word ("CREATE", "INSERT", "SELECT", etc.) is stored in 'command'.
    // The stream's internal position moves past the extracted word and the subsequent whitespace.
    ss >> command;

    // Convert command to upper case for case-insensitive comparison
    for (char &c : command) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }

    // Very basic command routing
    // Note: This assumes perfect spacing and syntax for now!
    if (command == "CREATE") {
        // Check for "TABLE" next (a slightly better check)
        std::string maybe_table;
        ss >> maybe_table;
         for (char &c : maybe_table) {
             c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
         }
         if (maybe_table == "TABLE") {
             return handle_create_table(query);
         }
    } else if (command == "INSERT") {
        // Check for "INTO" next
        std::string maybe_into;
        ss >> maybe_into;
        for (char &c : maybe_into) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        if (maybe_into == "INTO") {
            return handle_insert(query);
        }
    } else if (command == "SELECT") {
         return handle_select(query); // SELECT * FROM ... or SELECT * FROM ... WHERE ...
    }
    // Add more commands like UPDATE, DELETE later if needed

    return "ERROR: Unknown or unsupported command.";
}

int main() {
    std::cout << "Welcome to simple-db!" << std::endl;
    std::cout << "Enter exit or quit to exit." << std::endl;

    // Configure readline behavior if desired (optional)
    // rl_bind_key('\t', rl_complete); // Example: enable basic tab completion later

    // The Read-Eval-Print Loop (REPL)
    while (true) {
        // Use readline to read input. It handles the prompt, line editing, and history.
        // It returns a C-style string (char*) that we need to manage.
        char* line_read_c = readline("mydb> ");

        // Check for EOF (Ctrl+D) or error
        if (!line_read_c) {
            std::cout << "\nExiting." << std::endl;
            break;
        }

        // Convert the C-style string to std::string for easier handling in C++.
        // readline allocates memory, so we make a copy.
        std::string input_line(line_read_c);

        // Add the line to history *if* it's not empty and not an exit command.
        // Use the original C-string `line_read_c` with add_history.
        if (!input_line.empty() && input_line != "exit" && input_line != "quit") {
            add_history(line_read_c);
            // You might want to save/load history to a file here or on exit/startup
            // using read_history() and write_history().
        }

        // IMPORTANT: Free the memory allocated by readline()
        free(line_read_c);
        line_read_c = nullptr; // Good practice to null dangling pointers

        // --- Process the input line (copied into input_line) ---

        // Simple exit command check
        if (input_line == "exit" || input_line == "quit") {
            std::cout << "Exiting." << std::endl;
            break;
        }

        // Ignore empty lines (already handled by history add check, but safe to keep)
        if (input_line.empty()) {
            continue;
        }

        // Parse and "execute" the command
        std::string result = parse_and_execute(input_line);

        // Print the result
        std::cout << result << std::endl;
    }

    // Optional: Save history before exiting
    // write_history("simple_db_history.txt");

    return 0; // Indicate successful program termination
}
