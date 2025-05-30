//
// Created by Akshat Jain on 24/05/25.
//

#ifndef CATALOG_H
#define CATALOG_H
#include <filesystem>
#include <optional>
#include <string>

#include "command.h"

namespace catalog {

    struct TableSchema {
        std::string table_name;
        std::vector<command::ColumnDefinition> column_definitions;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TableSchema, table_name, column_definitions)

    /**
     * @brief Initializes the catalog system.
     * This must be called once at application startup before other catalog functions are used.
     * It loads the catalog from disk (if it exists) or prepares an empty catalog.
     * Handles critical errors if an existing catalog is corrupt by exiting.
     * @param data_directory The base directory where the catalog file (e.g., "catalog.json") resides.
     */
    void initialize(const std::filesystem::path& data_directory);

    /**
     * @brief Checks if a table with the given name exists in the catalog.
     */
    bool table_exists(const std::string& table_name);

    /**
     * @brief Adds a new table schema to the catalog.
     * This updates both the in-memory catalog and persists the change to disk.
     * @return True if the table was successfully added and persisted, false otherwise (e.g., disk save failed).
     */
    bool add_table(const TableSchema& table_schema);

    /**
     * @brief Removes a table from the catalog.
     */
    bool remove_table(const std::string& table_name);

    /**
     * @brief Retrieves the schema for a given table name.
     * @param table_name The name of the table.
     * @return An optional containing the TableSchema (if found).
     */
    std::optional<TableSchema> get_table_schema(const std::string& table_name);

    /**
     * @brief Retrieves all table schemas currently in the catalog.
     * Useful for listing tables or internal operations.
     * @return A vector of TableSchema objects representing all tables in the catalog.
     */
    const std::vector<TableSchema>& get_all_schemas();
}  // namespace catalog
#endif  // CATALOG_H
