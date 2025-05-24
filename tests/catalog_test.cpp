//
// Created by Akshat Jain on 24/05/25.
//

#include "simpledb/catalog.h"

#include <gtest/gtest.h>
#include <fstream>

using json = nlohmann::json;

TEST(JsonSerde, TableSchema) {
    catalog::TableSchema original_ts;
    original_ts.table_name = "my_table";
    original_ts.column_definitions.push_back({"column1", command::Datatype::INT});
    original_ts.column_definitions.push_back({"column2", command::Datatype::TEXT});
    original_ts.column_definitions.push_back({"column3", command::Datatype::INT});

    // Serialize it to JSON.
    json j = original_ts;

    json expected_j = {
        {"table_name", "my_table"},
        {"column_definitions", json::array(
            {
                {{"column_name", "column1"}, {"type", "INT"}},
                {{"column_name", "column2"}, {"type", "TEXT"}},
                {{"column_name", "column3"}, {"type", "INT"}}
            }
        )}
    };
    ASSERT_EQ(j, expected_j); // Verify the JSON produced is what you expect

    // Deserialize it back to a TableSchema object.
    catalog::TableSchema deserialized_ts = j.get<catalog::TableSchema>();
    ASSERT_EQ(original_ts.table_name, deserialized_ts.table_name);
    ASSERT_EQ(original_ts.column_definitions.size(), deserialized_ts.column_definitions.size());
    for (size_t i = 0; i < original_ts.column_definitions.size(); ++i) {
        ASSERT_EQ(original_ts.column_definitions[i].column_name, deserialized_ts.column_definitions[i].column_name);
        ASSERT_EQ(original_ts.column_definitions[i].type, deserialized_ts.column_definitions[i].type);
    }
}


TEST(JsonSerde, CatalogData) {
    catalog::CatalogData original_data;
    original_data.push_back({
        "table1",
        {
            {"column1", command::Datatype::INT},
            {"column2", command::Datatype::TEXT}
        }
    });
    original_data.push_back({
        "table2",
        {
            {"column3", command::Datatype::INT},
            {"column4", command::Datatype::TEXT}
        }
    });

    // Serialize it to JSON.
    json original_json = original_data;

    json expected_j = json::array({
        {
            {"table_name", "table1"},
            {"column_definitions", json::array(
                {
                    {{"column_name", "column1"}, {"type", "INT"}},
                    {{"column_name", "column2"}, {"type", "TEXT"}}
                }
            )}
        },
        {
            {"table_name", "table2"},
            {"column_definitions", json::array(
                {
                    {{"column_name", "column3"}, {"type", "INT"}},
                    {{"column_name", "column4"}, {"type", "TEXT"}}
                }
            )}
        }
    });

    ASSERT_EQ(original_json, expected_j); // Verify the JSON produced is what you expect

    // Deserialize it back to a CatalogData object.
    catalog::CatalogData deserialized_data = original_json.get<catalog::CatalogData>();

    ASSERT_EQ(original_data.size(), deserialized_data.size());
    for (size_t i = 0; i < original_data.size(); ++i) {
        ASSERT_EQ(original_data[i].table_name, deserialized_data[i].table_name);
        ASSERT_EQ(original_data[i].column_definitions.size(), deserialized_data[i].column_definitions.size());
        for (size_t j = 0; j < original_data[i].column_definitions.size(); ++j) {
            ASSERT_EQ(original_data[i].column_definitions[j].column_name, deserialized_data[i].column_definitions[j].column_name);
            ASSERT_EQ(original_data[i].column_definitions[j].type, deserialized_data[i].column_definitions[j].type);
        }
    }
}


TEST(CatalogFileIO, SaveAndLoadCatalog) {
    catalog::CatalogData catalog_data = {
            {"table1", {{"column1", command::Datatype::INT}, {"column2", command::Datatype::TEXT}}},
            {"table2", {{"column3", command::Datatype::INT}, {"column4", command::Datatype::TEXT}}}
    };

    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path catalog_path = temp_dir / "test_catalog.json";

    if (std::filesystem::exists(catalog_path)) {
        std::filesystem::remove(catalog_path);
    }

    ASSERT_TRUE(catalog::save_catalog(catalog_path, catalog_data));

    std::optional<catalog::CatalogData> loaded_catalog_optional = catalog::load_catalog(catalog_path);
    ASSERT_TRUE(loaded_catalog_optional.has_value());

    const catalog::CatalogData& loaded_catalog = loaded_catalog_optional.value();
    ASSERT_EQ(catalog_data.size(), loaded_catalog.size());

    for (size_t i = 0; i < catalog_data.size(); ++i) {
        ASSERT_EQ(catalog_data[i].table_name, loaded_catalog[i].table_name);
        ASSERT_EQ(catalog_data[i].column_definitions.size(), loaded_catalog[i].column_definitions.size());
        for (size_t j = 0; j < catalog_data[i].column_definitions.size(); ++j) {
            ASSERT_EQ(catalog_data[i].column_definitions[j].column_name, loaded_catalog[i].column_definitions[j].column_name);
            ASSERT_EQ(catalog_data[i].column_definitions[j].type, loaded_catalog[i].column_definitions[j].type);
        }
    }

    std::filesystem::remove(catalog_path);
}

TEST(CatalogFileIO, LoadNonExistentFileReturnsEmptyCatalog) {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path catalog_path = temp_dir / "non_existent_test_catalog.json";
    std::optional<catalog::CatalogData> loaded_catalog_optional = catalog::load_catalog(catalog_path);
    ASSERT_TRUE(loaded_catalog_optional.has_value());
    ASSERT_EQ(0, loaded_catalog_optional.value().size());
}


TEST(CatalogFileIO, LoadEmptyFileReturnsNullopt) {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path catalog_path = temp_dir / "empty_catalog.json";
    std::ofstream out(catalog_path);
    out.close(); // Create an empty file

    std::optional<catalog::CatalogData> loaded_catalog_optional = catalog::load_catalog(catalog_path);
    ASSERT_FALSE(loaded_catalog_optional.has_value());
    std::filesystem::remove(catalog_path);
}

TEST(CatalogFileIO, LoadMalformedJsonReturnsNullopt) {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path catalog_path = temp_dir / "malformed_catalog.json";
    std::ofstream out(catalog_path);
    out << "{ this is not valid json"; // Write malformed JSON
    out.close();

    std::optional<catalog::CatalogData> loaded_catalog_optional = catalog::load_catalog(catalog_path);
    ASSERT_FALSE(loaded_catalog_optional.has_value());
    std::filesystem::remove(catalog_path);
}

TEST(CatalogFileIO, LoadIncorrectJsonStructureReturnsNullopt) {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path catalog_path = temp_dir / "incorrect_structure_catalog.json";

    json j = {{"key1", "value1"}};
    std::ofstream out(catalog_path);
    out << j; // Write incorrect structure JSON
    out.close();

    std::optional<catalog::CatalogData> loaded_catalog_optional = catalog::load_catalog(catalog_path);
    ASSERT_FALSE(loaded_catalog_optional.has_value());
    std::filesystem::remove(catalog_path);
}

TEST(CatalogFileIO, LoadValidEmptyArrayJsonReturnsEmptyCatalog) {
    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path catalog_path = temp_dir / "valid_empty_array_catalog.json";

    json j = json::array(); // Valid empty array
    std::ofstream out(catalog_path);
    out << j.dump(2); // Write valid empty array JSON
    out.close();

    std::optional<catalog::CatalogData> loaded_catalog_optional = catalog::load_catalog(catalog_path);
    ASSERT_TRUE(loaded_catalog_optional.has_value());
    ASSERT_EQ(0, loaded_catalog_optional.value().size());
    std::filesystem::remove(catalog_path);
}
