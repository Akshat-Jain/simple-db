//
// Created by Akshat Jain on 25/05/25.
//

#ifndef SIMPLE_DB_EXECUTOR_H
#define SIMPLE_DB_EXECUTOR_H

#include <string>
#include "simpledb/command.h"
#include "simpledb/catalog.h"

namespace executor {
    // Using alias for the result type of execution.
    // For now, it's a string message, but it could become a more complex struct later
    // (e.g., to include status codes, row counts, or actual data for SELECT).
    using ExecutionResult = std::string;

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
     * @param catalog_data_ref A reference to the in-memory catalog data, which will be modified.
     * @param catalog_file_path The path to the catalog file on disk (e.g., "data/catalog.json").
     * @param table_data_dir The directory where table data files will be stored (e.g., "data/").
     * @return ExecutionResult A string indicating success or failure.
     */
    ExecutionResult execute_create_table_command(
            const command::CreateTableCommand& cmd,
            catalog::CatalogData& catalog_data,
            const std::filesystem::path& catalog_file_path,
            const std::filesystem::path& table_data_dir
    );
}

#endif //SIMPLE_DB_EXECUTOR_H
