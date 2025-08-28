//
// Created by Akshat Jain on 28/08/25.
//

#include "simpledb/query_runner.h"

namespace query_runner {
    results::ExecutionResult QueryRunner::run_query(const std::string& query) {
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
}  // namespace query_runner
