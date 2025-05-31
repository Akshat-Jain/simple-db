//
// Created by Akshat Jain on 25/05/25.
//

#include "simpledb/executor.h"
#include "simpledb/catalog.h"

#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

class ExecutorTestBase : public ::testing::Test {
   protected:
    std::filesystem::path test_data_dir;
    std::filesystem::path expected_catalog_json_path;

    void SetUp() override {
        // Create a unique temporary directory for each test run using this fixture
        // to ensure isolation.
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        test_data_dir = std::filesystem::temp_directory_path() /
                        (std::string("simpledb_tests_") + test_info->test_suite_name() + "_" + test_info->name());

        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove(test_data_dir);
        }
        std::filesystem::create_directories(test_data_dir);
        expected_catalog_json_path = test_data_dir / "catalog.json";

        catalog::initialize(test_data_dir);
    }

    void TearDown() override {
        // Remove the entire temporary directory and its contents
        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
    }

    std::optional<std::vector<catalog::TableSchema>> loadCatalogFromDisk() {
        if (!std::filesystem::exists(expected_catalog_json_path)) {
            return std::vector<catalog::TableSchema>{};
        }
        std::ifstream ifs(expected_catalog_json_path);
        if (!ifs.is_open()) return std::nullopt;
        try {
            json j;
            ifs >> j;
            if (ifs.fail() && !ifs.eof()) return std::nullopt;
            return j.get<std::vector<catalog::TableSchema>>();
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    static void AssertCatalogDataEqual(const std::vector<catalog::TableSchema>& expected,
                                       const std::vector<catalog::TableSchema>& actual) {
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
};

class ExecutorCreateTableTest : public ExecutorTestBase {
    void SetUp() override { ExecutorTestBase::SetUp(); }
    void TearDown() override { ExecutorTestBase::TearDown(); }
};

TEST_F(ExecutorCreateTableTest, SuccessfulCreateTable) {
    command::CreateTableCommand cmd;
    cmd.table_name = "test_table";
    cmd.column_definitions.push_back({"id", command::Datatype::INT});
    cmd.column_definitions.push_back({"name", command::Datatype::TEXT});

    std::string result = executor::execute_create_table_command(cmd, test_data_dir);

    // Check the result
    ASSERT_EQ(result, "OK (Table 'test_table' created successfully)");

    // Verify the in-memory catalog was updated
    const auto& in_memory_catalog_after_create = catalog::get_all_schemas();
    ASSERT_EQ(in_memory_catalog_after_create.size(), 1);
    ASSERT_EQ(in_memory_catalog_after_create[0].table_name, "test_table");
    ASSERT_EQ(in_memory_catalog_after_create[0].column_definitions.size(), 2);
    ASSERT_EQ(in_memory_catalog_after_create[0].column_definitions[0].column_name, "id");
    ASSERT_EQ(in_memory_catalog_after_create[0].column_definitions[0].type, command::Datatype::INT);
    ASSERT_EQ(in_memory_catalog_after_create[0].column_definitions[1].column_name, "name");
    ASSERT_EQ(in_memory_catalog_after_create[0].column_definitions[1].type, command::Datatype::TEXT);

    // Verify the catalog file was created and contains the correct data
    auto loaded_catalog = loadCatalogFromDisk();
    ASSERT_TRUE(loaded_catalog.has_value());
    AssertCatalogDataEqual(in_memory_catalog_after_create, loaded_catalog.value());

    // Verify the data file was created
    std::filesystem::path data_file_path = test_data_dir / "test_table.data";
    ASSERT_TRUE(std::filesystem::exists(data_file_path));
}

TEST_F(ExecutorCreateTableTest, DuplicateTableName) {
    // Create the first table
    command::CreateTableCommand cmd1;
    cmd1.table_name = "duplicate_table_name";
    cmd1.column_definitions.push_back({"id", command::Datatype::INT});
    cmd1.column_definitions.push_back({"name", command::Datatype::TEXT});

    std::string result1 = executor::execute_create_table_command(cmd1, test_data_dir);
    ASSERT_EQ(result1, "OK (Table 'duplicate_table_name' created successfully)");

    // Now try to create a table with the same name
    command::CreateTableCommand cmd2;
    cmd2.table_name = "duplicate_table_name";  // Same name as before
    cmd2.column_definitions.push_back({"id", command::Datatype::INT});
    cmd2.column_definitions.push_back({"description", command::Datatype::TEXT});

    std::string result2 = executor::execute_create_table_command(cmd2, test_data_dir);

    // Check the result
    ASSERT_EQ(result2, "ERROR: Table duplicate_table_name already exists.");

    // Verify the in-memory catalog was not updated
    const auto& in_memory_catalog = catalog::get_all_schemas();
    ASSERT_EQ(in_memory_catalog.size(), 1);
    ASSERT_EQ(in_memory_catalog[0].table_name, "duplicate_table_name");
    ASSERT_EQ(in_memory_catalog[0].column_definitions.size(), 2);
    ASSERT_EQ(in_memory_catalog[0].column_definitions[0].column_name, "id");
    ASSERT_EQ(in_memory_catalog[0].column_definitions[1].column_name, "name");

    // Verify the catalog file was created and contains the correct data
    auto loaded_catalog = loadCatalogFromDisk();
    ASSERT_TRUE(loaded_catalog.has_value());
    AssertCatalogDataEqual(in_memory_catalog, loaded_catalog.value());

    // Verify the data file for the first table was created and still exists
    std::filesystem::path data_file_path = test_data_dir / "duplicate_table_name.data";
    ASSERT_TRUE(std::filesystem::exists(data_file_path));
}
