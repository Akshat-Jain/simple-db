//
// Created by Akshat Jain on 29/03/25.
//

#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <variant>

#include "simpledb/history.h"
#include "simpledb/config.h"
#include "simpledb/utils/logging.h"
#include "simpledb/catalog.h"
#include "simpledb/executor.h"
#include "simpledb/query_runner.h"

int main() {
    config::init_config();
    catalog::initialize(config::get_config().data_dir);
    history::init();
    // Register history::save to be called on normal program exit
    if (std::atexit(history::save) != 0) {
        logging::log.error("Failed to register history::save with atexit. History may not be saved.");
    }

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

        // todo: handle multiple commands in a single line (e.g., "DROP TABLE idk; DROP TABLE idk2")
        //  That would also help simplify the parser layer to not worry about semicolon handling.

        // Add the line to history *if* it's not empty and not an exit command.
        if (!input_line.empty() && input_line != "exit" && input_line != "quit") {
            add_history(line_read_c);
        }

        // IMPORTANT: Free the memory allocated by readline()
        free(line_read_c);
        line_read_c = nullptr;  // Good practice to null dangling pointers

        // Handle meta commands in the REPL loop instead of parser layer.
        // This is done to keep the door open for having meta commands that are not necessarily SQL commands.
        // For example, we can have a meta command to show the current database or configuration settings.
        // Or meta commands like \timing on/off, or \c <database_name> to change the current database.
        if (input_line == "\\dt") {
            input_line = "SHOW TABLES";
        }

        if (input_line == "exit" || input_line == "quit") {
            std::cout << "Exiting." << std::endl;
            break;
        }

        // Ignore empty lines (already handled by history add check, but safe to keep)
        if (input_line.empty()) {
            continue;
        }

        results::ExecutionResult result = query_runner::QueryRunner::run_query(input_line);

        if (result.get_message().has_value()) {
            std::cout << result.get_message().value() << std::endl;
        }
        if (result.has_data()) {
            // todo: Handle the data in a more structured way, maybe with a table format.
            const results::ResultSet& result_set = result.get_data();
            for (const auto& item : result_set.headers) {
                std::cout << item << "\t";
            }
            std::cout << std::endl;
            for (const auto& row : result_set.rows) {
                for (const auto& col : row) {
                    std::cout << col << "\t";
                }
                std::cout << std::endl;
            }
        }
    }

    return 0;
}
