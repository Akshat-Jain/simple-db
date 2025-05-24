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

  using CatalogData = std::vector<TableSchema>;

  std::optional<CatalogData> load_catalog(const std::filesystem::path& catalog_path);

  bool save_catalog(const std::filesystem::path& catalog_path, const CatalogData& data);
}
#endif //CATALOG_H
