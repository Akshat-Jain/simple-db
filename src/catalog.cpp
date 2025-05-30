//
// Created by Akshat Jain on 24/05/25.
//

#include <iostream>
#include <fstream>
#include "simpledb/catalog.h"
#include "simpledb/utils/logging.h"

using json = nlohmann::json;

namespace catalog {

    static std::vector<TableSchema> catalog;
    static std::filesystem::path catalog_file_path;

    void initialize(const std::filesystem::path &data_directory) {
        std::filesystem::path new_path = data_directory / "catalog.json";

        // If it’s already initialized for *this* path, do nothing
        if (catalog_file_path == new_path) {
            logging::log.warn("Catalog already initialized for {}", new_path.string());
            return;
        }

        // If it was initialized for a *different* path, clear out the old state
        if (!catalog_file_path.empty() && catalog_file_path != new_path) {
            // This is a weird way of handling things and it exists this way only to allow {@link catalog_test.cpp}
            // to be tested with different catalog files.
            // todo: In the future, we can consider refactoring this to maybe having a Catalog class returned to the
            // caller, which gets passed around instead of using a global state.
            logging::log.info("Re-initializing catalog: clearing previous state");
            catalog.clear();
            catalog_file_path.clear();
        }

        catalog_file_path = new_path;
        logging::log.info("Initializing catalog: {}", catalog_file_path.string());

        if (!std::filesystem::exists(catalog_file_path)) {
            logging::log.warn("Catalog file does not exist: {}", catalog_file_path.string());
            catalog = {};
            return;
        }

        std::ifstream in(catalog_file_path);
        if (!in.is_open()) {
            logging::log.critical("FATAL ERROR: Could not open catalog file for reading: {}.",
                                  catalog_file_path.string());
            std::cerr << "FATAL ERROR: Database catalog file is corrupt or unreadable: " << catalog_file_path
                      << std::endl;
            std::exit(EXIT_FAILURE);
        }

        try {
            json j;
            in >> j;
            if (in.fail()) {
                logging::log.critical("Failed to read data from catalog file: {}", catalog_file_path.string());
                std::cerr << "ERROR: Failed to read data from catalog file: " << catalog_file_path << std::endl;
                std::exit(EXIT_FAILURE);
            }
            catalog = j.get<std::vector<TableSchema>>();
            logging::log.info(
                "Catalog loaded successfully from {}. Found {} table(s).", catalog_file_path.string(), catalog.size());
        } catch (const json::parse_error &e) {
            logging::log.critical(
                "Failed to parse JSON from catalog file {}: {}", catalog_file_path.string(), e.what());
            std::cerr << "ERROR: Failed to parse JSON from catalog file: " << catalog_file_path << std::endl;
            std::exit(EXIT_FAILURE);
        } catch (const json::type_error &e) {
            logging::log.critical("JSON structure in {} does not match expected CatalogData structure: {}",
                                  catalog_file_path.string(),
                                  e.what());
            std::cerr << "ERROR: JSON structure in catalog file does not match expected structure: "
                      << catalog_file_path << std::endl;
            std::exit(EXIT_FAILURE);
        } catch (const std::exception &e) {
            logging::log.critical("An unexpected standard exception occurred while loading catalog {}: {}",
                                  catalog_file_path.string(),
                                  e.what());
            std::cerr << "ERROR: An unexpected error occurred while loading catalog file: " << catalog_file_path
                      << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    bool table_exists(const std::string &table_name) {
        return std::any_of(
            catalog.begin(), catalog.end(), [&](const TableSchema &ts) { return ts.table_name == table_name; });
    }

    bool add_table(const TableSchema &table_schema) {
        if (catalog_file_path.empty()) {
            logging::log.critical("Catalog has not been initialized. Call initialize() first.");
            return false;
        }

        if (table_exists(table_schema.table_name)) {
            logging::log.warn("Table '{}' already exists in the catalog.", table_schema.table_name);
            return false;
        }

        catalog.push_back(table_schema);
        logging::log.info("Adding table '{}' to catalog.", table_schema.table_name);

        std::ofstream out(catalog_file_path);
        if (!out.is_open()) {
            logging::log.error("Failed to open catalog file for writing: {}", catalog_file_path.string());
            std::cerr << "ERROR: Failed to open catalog file for writing: " << catalog_file_path << std::endl;
            catalog.pop_back();  // Rollback
            return false;
        }

        try {
            json j = catalog;
            out << j.dump(2);
            if (out.fail()) {
                logging::log.error("Failed to write updated catalog to disk: {}", catalog_file_path.string());
                std::cerr << "ERROR: Failed to write updated catalog to disk: " << catalog_file_path << std::endl;
                catalog.pop_back();  // Rollback
                return false;
            }
        } catch (const json::exception &e) {
            logging::log.error(
                "JSON serialization error while saving catalog for table '{}': {}", table_schema.table_name, e.what());
            catalog.pop_back();  // Rollback
            return false;
        } catch (const std::exception &e) {
            logging::log.error(
                "Unexpected std::exception while saving catalog for table '{}': {}", table_schema.table_name, e.what());
            catalog.pop_back();  // Rollback
            return false;
        }

        logging::log.info("Table '{}' added successfully and catalog saved.", table_schema.table_name);
        return true;
    }

    bool remove_table(const std::string &table_name) {
        if (catalog_file_path.empty()) {
            logging::log.critical("Catalog has not been initialized. Call initialize() first.");
            return false;
        }

        // Find the table
        auto it = std::find_if(
            catalog.begin(), catalog.end(), [&](const TableSchema &ts) { return ts.table_name == table_name; });

        if (it == catalog.end()) {
            logging::log.warn("Attempt to remove table '{}', but it was not found in the catalog.", table_name);
            return false;  // Table not found
        }

        // Table found, erase it from in-memory catalog
        catalog.erase(it);
        logging::log.info("Removing table '{}' from catalog.", table_name);

        std::ofstream out(catalog_file_path);
        if (!out.is_open()) {
            logging::log.error("Failed to open catalog file for writing: {}", catalog_file_path.string());
            std::cerr << "ERROR: Failed to open catalog file for writing: " << catalog_file_path << std::endl;
            return false;
        }

        try {
            json j = catalog;
            out << j.dump(2);
            if (out.fail()) {
                logging::log.error("Failed to write updated catalog to disk: {}", catalog_file_path.string());
                std::cerr << "ERROR: Failed to write updated catalog to disk: " << catalog_file_path << std::endl;
                return false;
            }
        } catch (const json::exception &e) {
            logging::log.error(
                "JSON serialization error while saving catalog after removing table '{}': {}", table_name, e.what());
            return false;
        } catch (const std::exception &e) {
            logging::log.error(
                "Unexpected std::exception while saving catalog after removing table '{}': {}", table_name, e.what());
            return false;
        }

        logging::log.info("Table '{}' removed successfully and catalog saved.", table_name);
        return true;
    }

    std::optional<TableSchema> get_table_schema(const std::string &table_name) {
        auto it = std::find_if(
            catalog.begin(), catalog.end(), [&](const TableSchema &ts) { return ts.table_name == table_name; });
        if (it != catalog.end()) {
            return *it;
        }
        return std::nullopt;
    }

    const std::vector<TableSchema> &get_all_schemas() { return catalog; }
}  // namespace catalog
