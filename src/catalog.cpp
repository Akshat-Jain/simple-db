//
// Created by Akshat Jain on 24/05/25.
//

#include <fstream>
#include "simpledb/catalog.h"
#include "simpledb/utils/logging.h"

using json = nlohmann::json;

namespace catalog {
    std::optional<CatalogData> load_catalog(const std::filesystem::path &catalog_path) {
        if (!std::filesystem::exists(catalog_path)) {
            logging::log.warn("Catalog file does not exist: {}", catalog_path.string());
            return catalog::CatalogData{};
        }

        std::ifstream in(catalog_path);
        if (!in.is_open()) {
            logging::log.error("Failed to open catalog file for reading: {}", catalog_path.string());
            return std::nullopt;
        }

        try {
            json j;
            in >> j;
            if (in.fail()) {
                logging::log.error("Failed to read data from catalog file: {}", catalog_path.string());
                return std::nullopt;
            }
            return j.get<CatalogData>();
        } catch (const nlohmann::json::parse_error& e) {
            logging::log.error("Failed to parse JSON from catalog file {}: {}", catalog_path.string(), e.what());
            return std::nullopt;
        } catch (const nlohmann::json::type_error& e) {
            logging::log.error("JSON structure in {} does not match expected CatalogData structure: {}", catalog_path.string(), e.what());
            return std::nullopt;
        } catch (const std::exception& e) {
            logging::log.error("An unexpected standard exception occurred while loading catalog {}: {}", catalog_path.string(), e.what());
            return std::nullopt;
        }
    }

    bool save_catalog(const std::filesystem::path &catalog_path, const CatalogData &data) {
        json j = data;
        std::ofstream out(catalog_path);
        if (!out.is_open()) {
            logging::log.error("Failed to open catalog file for writing: {}", catalog_path.string());
            return false;
        }
        out << j.dump(2);
        if (out.fail()) {
            logging::log.error("Failed to write data to catalog file: {}", catalog_path.string());
            return false;
        }
        out.close();
        if (out.fail()) {
            logging::log.error("Error occurred while closing catalog file: {}", catalog_path.string());
            return false;
        }
        logging::log.info("Successfully written catalog file: {}", catalog_path.string());
        return true;
    }
}
