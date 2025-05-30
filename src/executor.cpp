//
// Created by Akshat Jain on 25/05/25.
//

#include "simpledb/executor.h"
#include "simpledb/utils/logging.h"
#include "simpledb/config.h"

namespace executor {
    ExecutionResult execute_create_table_command(const command::CreateTableCommand& cmd,
                                                 const std::filesystem::path& table_data_dir) {
        if (catalog::table_exists(cmd.table_name)) {
            logging::log.error("Table '{}' already exists in the catalog.", cmd.table_name);
            return "ERROR: Table " + cmd.table_name + " already exists.";
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
            return "OK (Table '" + table_schema.table_name + "' created successfully)";
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

            return "ERROR: " + std::string(e.what()) + " Table creation aborted.";
        }
    }
}  // namespace executor
