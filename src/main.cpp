//
// Created by Akshat Jain on 29/03/25.
//

#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

#include "simpledb/history.h"
#include "simpledb/config.h"
#include "simpledb/parser.h"
#include "simpledb/utils/logging.h"
#include "simpledb/catalog.h"
#include "simpledb/executor.h"

results::ExecutionResult parse_and_execute(const std::string& query) {
    parser::CommandType command_type = parser::get_command_type(query);
    switch (command_type) {
        case parser::CommandType::CREATE_TABLE: {
            std::optional<command::CreateTableCommand> cmd = parser::parse_create_table(query);
            if (!cmd) {
                return results::ExecutionResult::Error("ERROR: Failed to parse CREATE TABLE command.");
            }
            logging::log.info("Parsed CREATE TABLE command successfully for table: {}", cmd->table_name);
            return executor::execute_create_table_command(cmd.value(), config::get_config().data_dir);
        }
        case parser::CommandType::DROP_TABLE: {
            std::optional<command::DropTableCommand> cmd = parser::parse_drop_table(query);
            if (!cmd) {
                return results::ExecutionResult::Error("ERROR: Failed to parse DROP TABLE command.");
            }
            logging::log.info("Parsed DROP TABLE command successfully for table: {}", cmd->table_name);
            return executor::execute_drop_table_command(cmd.value(), config::get_config().data_dir);
        }
        case parser::CommandType::SHOW_TABLES: {
            std::optional<command::ShowTablesCommand> cmd = parser::parse_show_tables(query);
            if (!cmd) {
                return results::ExecutionResult::Error("ERROR: Failed to parse SHOW TABLES command.");
            }
            return executor::execute_show_tables_command();
        }
        case parser::CommandType::INSERT:
            return results::ExecutionResult::Ok("OK (Placeholder - INSERT not yet implemented)");
        case parser::CommandType::SELECT:
            return results::ExecutionResult::Ok("OK (Placeholder - SELECT not yet implemented)");
        case parser::CommandType::UNKNOWN:
        default:
            return results::ExecutionResult::Error("ERROR: Unknown or unsupported command.");
    }
}

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

        if (input_line == "exit" || input_line == "quit") {
            std::cout << "Exiting." << std::endl;
            break;
        }

        // Ignore empty lines (already handled by history add check, but safe to keep)
        if (input_line.empty()) {
            continue;
        }

        results::ExecutionResult result = parse_and_execute(input_line);

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
