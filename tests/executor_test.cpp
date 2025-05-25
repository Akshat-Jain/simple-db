//
// Created by Akshat Jain on 25/05/25.
//

#include "simpledb/executor.h"

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>


void AssertCatalogDataEqual(const catalog::CatalogData& expected, const catalog::CatalogData& actual) {
    ASSERT_EQ(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_EQ(expected[i].table_name, actual[i].table_name);
        ASSERT_EQ(expected[i].column_definitions.size(), actual[i].column_definitions.size());
        for (size_t j = 0; j < expected[i].column_definitions.size(); ++j) {
            ASSERT_EQ(expected[i].column_definitions[j].column_name, actual[i].column_definitions[j].column_name);
            ASSERT_EQ(expected[i].column_definitions[j].type, actual[i].column_definitions[j].type);
        }
    }
}


class ExecutorCreateTableTest : public ::testing::Test {
protected:
    catalog::CatalogData test_in_memory_catalog; // This will be passed to the executor
    std::filesystem::path test_base_data_dir;    // Base for all test files for this fixture
    std::filesystem::path test_catalog_file_path;

    void SetUp() override {
        // Create a unique temporary directory for each test run using this fixture
        // to ensure isolation. Using the test name can help.
        const ::testing::TestInfo* const test_info =
                ::testing::UnitTest::GetInstance()->current_test_info();
        test_base_data_dir = std::filesystem::temp_directory_path() /
                             (std::string("simpledb_tests_") + test_info->test_suite_name() + "_" + test_info->name());

        std::filesystem::create_directories(test_base_data_dir); // Create the directory itself

        test_catalog_file_path = test_base_data_dir / "catalog.json";

        // Clear the in-memory catalog for each test
        test_in_memory_catalog.clear();

        // Ensure catalog file doesn't exist from a previous run (optional, TearDown should handle)
        if (std::filesystem::exists(test_catalog_file_path)) {
            std::filesystem::remove(test_catalog_file_path);
        }
    }

    void TearDown() override {
        // Remove the entire temporary directory and its contents
        if (std::filesystem::exists(test_base_data_dir)) {
            std::filesystem::remove_all(test_base_data_dir);
        }
    }

    // Helper to load catalog from the test_catalog_file_path
    std::optional<catalog::CatalogData> loadTestCatalogFromFile() {
        return catalog::load_catalog(test_catalog_file_path);
    }
};

TEST_F(ExecutorCreateTableTest, SuccessfulCreation) {
    command::CreateTableCommand cmd;
    cmd.table_name = "test_table";
    cmd.column_definitions.push_back({"id", command::Datatype::INT});
    cmd.column_definitions.push_back({"name", command::Datatype::TEXT});;

    std::string result = executor::execute_create_table_command(cmd, test_in_memory_catalog, test_catalog_file_path);

    // Check the result
    ASSERT_EQ(result, "OK (Table 'test_table' created successfully)");

    // Verify the in-memory catalog was updated
    ASSERT_EQ(test_in_memory_catalog.size(), 1);
    ASSERT_EQ(test_in_memory_catalog[0].table_name, "test_table");
    ASSERT_EQ(test_in_memory_catalog[0].column_definitions.size(), 2);
    ASSERT_EQ(test_in_memory_catalog[0].column_definitions[0].column_name, "id");
    ASSERT_EQ(test_in_memory_catalog[0].column_definitions[0].type, command::Datatype::INT);
    ASSERT_EQ(test_in_memory_catalog[0].column_definitions[1].column_name, "name");
    ASSERT_EQ(test_in_memory_catalog[0].column_definitions[1].type, command::Datatype::TEXT);

    // Verify the catalog file was created and contains the correct data
    auto loaded_catalog = loadTestCatalogFromFile();
    ASSERT_TRUE(loaded_catalog.has_value());
    AssertCatalogDataEqual(test_in_memory_catalog, loaded_catalog.value());

    // Verify the data file was created
    std::filesystem::path data_file_path = "test_table.data";
    ASSERT_TRUE(std::filesystem::exists(data_file_path));
}
