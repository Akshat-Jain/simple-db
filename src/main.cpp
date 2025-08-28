//
// Created by Akshat Jain on 29/03/25.
//

#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
#include <variant>

#include "simpledb/history.h"
#include "simpledb/config.h"
#include "simpledb/parser.h"
#include "simpledb/utils/logging.h"
#include "simpledb/catalog.h"
#include "simpledb/executor.h"
#include "simpledb/planner.h"

results::ExecutionResult parse_and_execute(const std::string& query) {
    try {
        auto parse_result = parser::parse_sql(query);

        if (!parse_result) {
            return results::ExecutionResult::Error("Invalid SQL syntax.");
        }

        // Check if the variant holds a CreateTableCommand
        if (auto* cmd = std::get_if<command::CreateTableCommand>(&(*parse_result))) {
            return executor::execute_create_table_command(*cmd, config::get_config().data_dir);
        }

        // Check if it holds a DropTableCommand
        if (auto* cmd = std::get_if<command::DropTableCommand>(&(*parse_result))) {
            return executor::execute_drop_table_command(*cmd, config::get_config().data_dir);
        }

        // Check if it holds an InsertCommand
        if (auto* cmd = std::get_if<command::InsertCommand>(&(*parse_result))) {
            return executor::execute_insert_command(*cmd, config::get_config().data_dir);
        }

        // Check if it holds a ShowTablesCommand
        if (auto* cmd = std::get_if<command::ShowTablesCommand>(&(*parse_result))) {
            return executor::execute_show_tables_command();
        }

        // Check if it holds a SelectCommand
        if (auto* cmd = std::get_if<ast::SelectCommand>(&(*parse_result))) {
            auto plan = planner::plan_select(*cmd, config::get_config().data_dir);

            // Collect headers
            std::vector<std::string> headers;
            if (cmd->projection.empty()) {  // SELECT *
                auto schema = catalog::get_table_schema(cmd->table_name).value();
                for (const auto& col_def : schema.column_definitions) {
                    headers.push_back(col_def.column_name);
                }
            } else {
                headers = cmd->projection;
            }

            // Collect rows
            std::vector<row::Row> rows;
            while (auto row = plan->next()) {
                rows.push_back(*row);
            }

            results::ResultSet result_set{headers, rows};
            return results::ExecutionResult::SuccessWithData(result_set);
        }

        // If it's none of the above, something is wrong.
        return results::ExecutionResult::Error("Unsupported command type.");

    } catch (const std::exception& e) {
        return results::ExecutionResult::Error(e.what());
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
