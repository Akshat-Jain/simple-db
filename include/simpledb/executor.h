//
// Created by Akshat Jain on 25/05/25.
//

#ifndef SIMPLE_DB_EXECUTOR_H
#define SIMPLE_DB_EXECUTOR_H

#include <string>
#include "simpledb/command.h"
#include "simpledb/catalog.h"
#include "simpledb/result.h"

namespace executor {
    /**
     * @brief Executes a Create Table command.
     *
     * This function handles the logic for creating a new table, including:
     * <li> Checking for table name conflicts in the catalog.
     * <li> Adding the new table schema to the in-memory catalog.
     * <li> Persisting the updated catalog to disk.
     * <li> Creating the physical data file for the table.
     * <p>
     * <br>
     * It performs rollbacks if any step fails to maintain consistency.
     *
     * @param cmd The parsed CreateTableCommand object containing table details.
     * @param table_data_dir The directory where table data files will be stored (e.g., "data/").
     * @return ExecutionResult A string indicating success or failure.
     */
    results::ExecutionResult execute_create_table_command(const command::CreateTableCommand& cmd,
                                                          const std::filesystem::path& table_data_dir);

    results::ExecutionResult execute_drop_table_command(const command::DropTableCommand& cmd,
                                                        const std::filesystem::path& table_data_dir);

    results::ExecutionResult execute_show_tables_command();
}  // namespace executor

#endif  // SIMPLE_DB_EXECUTOR_H
