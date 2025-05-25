//
// Created by Akshat Jain on 24/05/25.
//

#include "simpledb/catalog.h"

#include <gtest/gtest.h>
#include <fstream>
#include "simpledb/command.h"
#include "simpledb/utils/logging.h"
#include <filesystem>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class CatalogTest : public ::testing::Test {
protected:
    std::filesystem::path test_data_dir; // test_data_dir will be unique for each test case run via this fixture
    std::filesystem::path expected_catalog_json_path; // Path to where catalog.json *should* be for this test

    void SetUp() override {
        // Create a unique temporary directory for this specific test case
        const ::testing::TestInfo* const test_info =
                ::testing::UnitTest::GetInstance()->current_test_info();
        test_data_dir = std::filesystem::temp_directory_path() /
                         (std::string("simpledb_catalog_") + test_info->test_suite_name() + "_" + test_info->name());

        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
        std::filesystem::create_directories(test_data_dir);
        expected_catalog_json_path = test_data_dir / "catalog.json";
    }

    void TearDown() override {
        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
    }

    std::optional<std::vector<catalog::TableSchema>> loadCatalogFromDisk() {
        if (!std::filesystem::exists(expected_catalog_json_path)) {
            return std::vector<catalog::TableSchema>{}; // Consistent with how initialize handles non-existent
        }
        std::ifstream ifs(expected_catalog_json_path);
        if (!ifs.is_open()) return std::nullopt;
        try {
            json j;
            ifs >> j;
            if (ifs.fail() && !ifs.eof()) return std::nullopt; // Basic stream check
            return j.get<std::vector<catalog::TableSchema>>();
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }
};

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
    std::vector<catalog::TableSchema> original_data;
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
    std::vector<catalog::TableSchema> deserialized_data = original_json.get<std::vector<catalog::TableSchema>>();

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


TEST_F(CatalogTest, AddMultipleTablesAndVerifyPersistence) {
    catalog::TableSchema table1 = {
        "table1",
        {
            {"column1", command::Datatype::INT},
            {"column2", command::Datatype::TEXT}
        }
    };
    catalog::TableSchema table2 = {
        "table2",
        {
            {"column3", command::Datatype::INT},
            {"column4", command::Datatype::TEXT}
        }
    };
    std::vector<catalog::TableSchema> catalog_data = {table1, table2};

    std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
    std::filesystem::path catalog_path = temp_dir / "test_catalog.json";

    if (std::filesystem::exists(catalog_path)) {
        std::filesystem::remove(catalog_path);
    }

    catalog::initialize(test_data_dir);
    ASSERT_TRUE(catalog::add_table(table1));
    ASSERT_TRUE(catalog::add_table(table2));

    std::optional<std::vector<catalog::TableSchema>> loaded_catalog_optional = loadCatalogFromDisk();
    ASSERT_TRUE(loaded_catalog_optional.has_value());

    const std::vector<catalog::TableSchema>& loaded_catalog = loaded_catalog_optional.value();
    ASSERT_EQ(2, loaded_catalog.size());

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

TEST_F(CatalogTest, InitializeWithNonExistentFileResultsInEmptyCatalog) {
    catalog::initialize(test_data_dir);
    const auto& schemas_in_memory = catalog::get_all_schemas();
    ASSERT_EQ(0, schemas_in_memory.size());
    ASSERT_FALSE(std::filesystem::exists(expected_catalog_json_path));
}


TEST_F(CatalogTest, InitializeWithEmptyFileCausesExit) {
    std::ofstream out(expected_catalog_json_path);
    if (!out.is_open()) { ADD_FAILURE() << "Setup: Failed to create empty file."; return; }
    out.close();
    if (out.fail()) { ADD_FAILURE() << "Setup: Failed to close empty file."; return; }

    ASSERT_DEATH({
                     catalog::initialize(test_data_dir);
                 }, "Failed to parse JSON from catalog file");
}

TEST_F(CatalogTest, InitializeWithMalformedJsonFileCausesExit) {
    std::ofstream out(expected_catalog_json_path);
    if (!out.is_open()) {
        ADD_FAILURE() << "Test setup: Failed to create malformed JSON file at " << expected_catalog_json_path;
        return;
    }
    out << "{ this is not valid json"; // Write malformed JSON
    out.close();
    if (out.fail()) {
        ADD_FAILURE() << "Test setup: Failed to close malformed JSON file at " << expected_catalog_json_path;
        if (std::filesystem::exists(expected_catalog_json_path)) {
            std::filesystem::remove(expected_catalog_json_path);
        }
        return;
    }
    ASSERT_TRUE(std::filesystem::exists(expected_catalog_json_path));
    ASSERT_DEATH({
                     catalog::initialize(test_data_dir);
                 }, "Failed to parse JSON from catalog file");
}

TEST_F(CatalogTest, InitializeWithIncorrectJsonStructureCausesExit) {
    json incorrect_structure_json = {
            {"some_object_key", "some_value"},
            {"another_key", 123}
    }; // This is a JSON object, CatalogData expects an array.

    std::ofstream out(expected_catalog_json_path);
    if (!out.is_open()) {
        ADD_FAILURE() << "Test setup: Failed to create incorrect structure JSON file at " << expected_catalog_json_path;
        return;
    }
    out << incorrect_structure_json.dump(2);
    out.close();
    if (out.fail()) {
        ADD_FAILURE() << "Test setup: Failed to close incorrect structure JSON file at " << expected_catalog_json_path;
        if (std::filesystem::exists(expected_catalog_json_path)) {
            std::filesystem::remove(expected_catalog_json_path);
        }
        return;
    }
    ASSERT_TRUE(std::filesystem::exists(expected_catalog_json_path));
    ASSERT_DEATH({
                     catalog::initialize(test_data_dir);
                 }, "JSON structure in catalog file does not match expected structure");
}

TEST_F(CatalogTest, InitializeWithValidEmptyArrayFileResultsInEmptyCatalog) {
    json empty_array_json = nlohmann::json::array(); // Creates "[]"

    std::ofstream out(expected_catalog_json_path);
    if (!out.is_open()) {
        ADD_FAILURE() << "Test setup: Failed to create valid empty array JSON file at " << expected_catalog_json_path;
        return;
    }
    out << empty_array_json.dump(2);
    out.close();
    if (out.fail()) {
        ADD_FAILURE() << "Test setup: Failed to close valid empty array JSON file at " << expected_catalog_json_path;
        if (std::filesystem::exists(expected_catalog_json_path)) {
            std::filesystem::remove(expected_catalog_json_path);
        }
        return;
    }
    ASSERT_TRUE(std::filesystem::exists(expected_catalog_json_path));

    catalog::initialize(test_data_dir);
    const auto& schemas_in_memory = catalog::get_all_schemas();
    ASSERT_TRUE(schemas_in_memory.empty())
                                << "Catalog should be empty after initializing with a file containing '[]'.";
    ASSERT_EQ(0, schemas_in_memory.size())
                                << "Catalog size should be 0 after initializing with a file containing '[]'.";

    // Also verify that the "catalog.json" file still exists on disk (initialize doesn't delete it if it's valid)
    ASSERT_TRUE(std::filesystem::exists(expected_catalog_json_path))
                                << "catalog.json containing '[]' should still exist after successful initialization.";
}
