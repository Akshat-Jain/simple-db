//
// Created by Akshat Jain on 25/05/25.
//

#include "simpledb/executor.h"

#include "simpledb/config.h"
#include "simpledb/serializer.h"
#include "simpledb/execution/row.h"
#include "simpledb/storage/table_heap.h"
#include "simpledb/utils/logging.h"

namespace executor {
    results::ExecutionResult execute_create_table_command(const command::CreateTableCommand& cmd,
                                                          const std::filesystem::path& table_data_dir) {
        if (catalog::table_exists(cmd.table_name)) {
            logging::log.error("Table '{}' already exists in the catalog.", cmd.table_name);
            return results::ExecutionResult::Error("ERROR: Table " + cmd.table_name + " already exists.");
        }

        catalog::TableSchema table_schema = {cmd.table_name, cmd.column_definitions};

        // --- Transaction-like block for catalog update and data file creation ---
        bool catalog_successfully_updated = false;
        std::filesystem::path table_data_path;

        try {
            // --- Step 1: Add table to catalog (in-memory and disk) ---
            if (!catalog::add_table(table_schema)) {
                throw std::runtime_error("Failed to add table to catalog and persist catalog changes.");
            }
            catalog_successfully_updated = true;
            logging::log.info("Table '{}' successfully added to catalog (in-memory and on disk).",
                              table_schema.table_name);

            // --- Step 2: Create data file ---
            table_data_path = table_data_dir / (table_schema.table_name + ".data");
            std::ofstream table_data_file(table_data_path);
            if (!table_data_file.is_open()) {
                throw std::runtime_error("Failed to create data file (could not open). Path: " +
                                         table_data_path.string());
            }
            table_data_file.close();
            if (table_data_file.fail()) {
                throw std::runtime_error("Failed to create data file (error on close). Path: " +
                                         table_data_path.string());
            }
            logging::log.info("Data file created successfully for table '{}' at {}",
                              table_schema.table_name,
                              table_data_path.string());

            // If all steps succeeded
            return results::ExecutionResult::Ok("OK (Table '" + table_schema.table_name + "' created successfully)");
        } catch (const std::exception& e) {
            logging::log.error("Error occurred while creating table '{}': {}", cmd.table_name, e.what());

            // If the disk catalog was updated, we need to update it back.
            if (catalog_successfully_updated) {
                if (!catalog::remove_table(cmd.table_name)) {
                    logging::log.error("Failed to rollback disk catalog update for table: {}", cmd.table_name);
                } else {
                    logging::log.info("Rolled back disk catalog update for table: {}", cmd.table_name);
                }
            }

            // If the data file was created, remove it
            if (!table_data_path.empty() && std::filesystem::exists(table_data_path)) {
                std::filesystem::remove(table_data_path);
                logging::log.info("Removed data file for table: {}", cmd.table_name);
            }

            return results::ExecutionResult::Error("ERROR: " + std::string(e.what()) + " Table creation aborted.");
        }
    }

    results::ExecutionResult execute_drop_table_command(const command::DropTableCommand& cmd,
                                                        const std::filesystem::path& table_data_dir) {
        std::string table_name = cmd.table_name;
        if (!catalog::table_exists(table_name)) {
            return results::ExecutionResult::Error("ERROR: Table '" + table_name + "' does not exist.");
        }
        logging::log.info("Attempting to drop table '{}'", table_name);
        std::filesystem::path table_data_path = table_data_dir / (table_name + ".data");
        // --- Transaction-like block for catalog update and data file deletion ---

        bool catalog_successfully_updated = false;
        try {
            // --- Step 1: Remove table from catalog (in-memory and disk) ---
            if (!catalog::remove_table(table_name)) {
                throw std::runtime_error("Failed to remove table from catalog.");
            }
            catalog_successfully_updated = true;

            // --- Step 2: Remove data file ---
            if (std::filesystem::exists(table_data_path)) {
                std::filesystem::remove(table_data_path);
                logging::log.info(
                    "Data file for table '{}' removed successfully at {}", table_name, table_data_path.string());
            } else {
                logging::log.warn(
                    "Data file for table '{}' does not exist at {}", table_name, table_data_path.string());
            }
            return results::ExecutionResult::Ok("OK (Table '" + table_name + "' dropped successfully)");
        } catch (const std::exception& e) {
            if (catalog_successfully_updated) {
                logging::log.error(
                    "CRITICAL: Table '{}' removed from catalog, but data file operation failed. Data file {} may be "
                    "orphaned. Reason: {}",
                    table_name,
                    table_data_path.string(),
                    e.what());
            }

            return results::ExecutionResult::Error("ERROR: DROP TABLE failed for table '" + table_name +
                                                   "'. Reason: " + e.what());
        }
    }

    results::ExecutionResult execute_show_tables_command() {
        const std::vector<catalog::TableSchema>& table_schemas = catalog::get_all_schemas();
        std::vector<std::string> headers = {"Table Name"};
        std::vector<row::Row> data;
        for (const auto& schema : table_schemas) {
            data.push_back({schema.table_name});
        }
        results::ResultSet result_set{headers, data};
        return results::ExecutionResult::SuccessWithData(result_set, std::nullopt);
    }

    results::ExecutionResult execute_insert_command(const command::InsertCommand& cmd,
                                                    const std::filesystem::path& table_data_dir) {
        std::optional<catalog::TableSchema> table_schema = catalog::get_table_schema(cmd.table_name);
        if (!table_schema.has_value()) {
            return results::ExecutionResult::Error("ERROR: Table '" + cmd.table_name + "' does not exist.");
        }
        logging::log.info("Inserting data into table '{}'", cmd.table_name);
        std::filesystem::path table_data_path = table_data_dir / (cmd.table_name + ".data");

        // INSERT INTO can be of 2 types:
        // 1. INSERT INTO table_name VALUES (val1, val2, ...);
        // 2. INSERT INTO table_name (col1, col2, ...) VALUES (val1, val2, ...);
        // todo: We don't handle multiple rows in a single INSERT command yet.

        std::vector<std::string> ordered_values;

        if (cmd.columns.empty()) {
            // Type 1: INSERT INTO table_name VALUES (val1, val2, ...);
            if (cmd.values.size() != table_schema->column_definitions.size()) {
                return results::ExecutionResult::Error(
                    "ERROR: Number of values does not match number of columns in table '" + cmd.table_name + "'.");
            }
            ordered_values = cmd.values;  // Use values directly as they are in order
        } else {
            // Type 2: INSERT INTO table_name (col1, col2, ...) VALUES (val1, val2, ...);
            if (cmd.columns.size() != cmd.values.size()) {
                return results::ExecutionResult::Error(
                    "ERROR: Number of columns does not match number of values in INSERT command for table '" +
                    cmd.table_name + "'.");
            }

            // Validate that the provided columns exist in the table schema
            std::unordered_map<std::string, size_t> column_index_map;
            for (size_t i = 0; i < table_schema->column_definitions.size(); ++i) {
                column_index_map[table_schema->column_definitions[i].column_name] = i;
            }

            ordered_values.resize(table_schema->column_definitions.size(), "");
            for (size_t i = 0; i < cmd.columns.size(); ++i) {
                const auto& col = cmd.columns[i];
                if (column_index_map.find(col) == column_index_map.end()) {
                    return results::ExecutionResult::Error("ERROR: Column '" + col + "' does not exist in table '" +
                                                           cmd.table_name + "'.");
                }
                ordered_values[column_index_map[col]] = cmd.values[i];
            }
        }

        // Validate the values against the table schema
        for (size_t i = 0; i < ordered_values.size(); ++i) {
            const auto& col_def = table_schema->column_definitions[i];
            if (col_def.type == command::Datatype::INT) {
                try {
                    std::stoi(ordered_values[i]);  // Check if it can be converted to int
                } catch (const std::invalid_argument&) {
                    return results::ExecutionResult::Error("ERROR: Value '" + ordered_values[i] + "' for column '" +
                                                           col_def.column_name + "' is not a valid integer.");
                }
            } else if (col_def.type == command::Datatype::TEXT) {
                // No specific validation for TEXT, but we could add length checks or other constraints later
            } else {
                return results::ExecutionResult::Error("ERROR: Unknown data type for column '" + col_def.column_name +
                                                       "'.");
            }
        }

        // Serialize the ordered values into a vector<char> format
        // We will use a format [length of value][value][length of value][value]...
        std::vector<char> record_data = serializer::serialize(ordered_values);

        // Insert the record into the table heap.
        simpledb::storage::TableHeap table_heap(table_data_path.string());
        if (table_heap.InsertRecord(record_data)) {
            return results::ExecutionResult::Ok("1 row inserted.");
        } else {
            // If it fails (e.g., record too big), return an error.
            return results::ExecutionResult::Error(
                "ERROR: Failed to insert row. The record may be too large for a page.");
        }
    }
}  // namespace executor
