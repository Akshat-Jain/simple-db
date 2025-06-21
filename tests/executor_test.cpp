//
// Created by Akshat Jain on 25/05/25.
//

#include "simpledb/executor.h"
#include "simpledb/catalog.h"
#include "simpledb/storage/page.h"
#include "simpledb/serializer.h"
#include "simpledb/storage/table_heap.h"

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
            std::filesystem::remove_all(test_data_dir);
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

class ExecutorDropTableTest : public ExecutorTestBase {
    void SetUp() override {
        ExecutorTestBase::SetUp();
        // Create a sample table to test drop functionality
        command::CreateTableCommand cmd;
        cmd.table_name = "test_table";
        cmd.column_definitions.push_back({"id", command::Datatype::INT});
        cmd.column_definitions.push_back({"name", command::Datatype::TEXT});
        executor::execute_create_table_command(cmd, test_data_dir);
    }
    void TearDown() override { ExecutorTestBase::TearDown(); }
};

class ExecutorShowTablesTest : public ExecutorTestBase {
    void SetUp() override {
        ExecutorTestBase::SetUp();
        // Create a sample table to test show functionality
        command::CreateTableCommand cmd;
        cmd.table_name = "test_table";
        cmd.column_definitions.push_back({"id", command::Datatype::INT});
        cmd.column_definitions.push_back({"name", command::Datatype::TEXT});
        executor::execute_create_table_command(cmd, test_data_dir);
    }
    void TearDown() override { ExecutorTestBase::TearDown(); }
};

class ExecutorInsertTablesTest : public ExecutorTestBase {
    void SetUp() override {
        ExecutorTestBase::SetUp();
        // Create a sample table to test show functionality
        command::CreateTableCommand cmd;
        cmd.table_name = "test_table";
        cmd.column_definitions.push_back({"id", command::Datatype::INT});
        cmd.column_definitions.push_back({"name", command::Datatype::TEXT});
        executor::execute_create_table_command(cmd, test_data_dir);
    }
    void TearDown() override { ExecutorTestBase::TearDown(); }

   protected:
    void AssertRecordForSlot(simpledb::storage::PageId page_id,
                             uint16_t slot_num,
                             const std::vector<std::string>& expected_values) {
        // 1. Read the file into a buffer
        std::filesystem::path data_file_path = test_data_dir / "test_table.data";
        std::ifstream file(data_file_path, std::ios::binary);

        // 2. Seek and read the page
        file.seekg(static_cast<size_t>(page_id) * simpledb::storage::PAGE_SIZE);
        simpledb::storage::Page page;
        file.read(page.GetData(), simpledb::storage::PAGE_SIZE);
        file.close();

        // 3. Get the slot and record
        ASSERT_LT(slot_num, page.GetNumRecords()) << "Slot number is out of bounds.";
        auto slot = page.GetSlot(slot_num);
        std::vector<char> record_blob = page.GetRecord(slot);

        // 4. Deserialize and assert
        std::vector<std::string> actual_values = serializer::deserialize(record_blob);
        ASSERT_EQ(expected_values, actual_values);
    }
};

TEST_F(ExecutorCreateTableTest, SuccessfulCreateTable) {
    command::CreateTableCommand cmd;
    cmd.table_name = "test_table";
    cmd.column_definitions.push_back({"id", command::Datatype::INT});
    cmd.column_definitions.push_back({"name", command::Datatype::TEXT});

    results::ExecutionResult result = executor::execute_create_table_command(cmd, test_data_dir);

    // Check the result
    ASSERT_EQ(result.get_message(), "OK (Table 'test_table' created successfully)");

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

    results::ExecutionResult result1 = executor::execute_create_table_command(cmd1, test_data_dir);
    ASSERT_EQ(result1.get_message(), "OK (Table 'duplicate_table_name' created successfully)");

    // Now try to create a table with the same name
    command::CreateTableCommand cmd2;
    cmd2.table_name = "duplicate_table_name";  // Same name as before
    cmd2.column_definitions.push_back({"id", command::Datatype::INT});
    cmd2.column_definitions.push_back({"description", command::Datatype::TEXT});

    results::ExecutionResult result2 = executor::execute_create_table_command(cmd2, test_data_dir);

    // Check the result
    ASSERT_EQ(result2.get_message(), "ERROR: Table duplicate_table_name already exists.");

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

TEST_F(ExecutorDropTableTest, SuccessfulDropTable) {
    command::DropTableCommand cmd = {"test_table"};

    // Before the DROP TABLE command, validate that the table related stuff exists.
    ASSERT_TRUE(catalog::table_exists(cmd.table_name));
    auto loaded_catalog = loadCatalogFromDisk();
    ASSERT_EQ(1, loaded_catalog->size());
    ASSERT_TRUE(std::filesystem::exists(test_data_dir / "test_table.data"));

    // Drop the table
    results::ExecutionResult result = executor::execute_drop_table_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "OK (Table 'test_table' dropped successfully)");
    ASSERT_FALSE(catalog::table_exists(cmd.table_name));

    // Validate the state after dropping the table
    loaded_catalog = loadCatalogFromDisk();
    ASSERT_TRUE(loaded_catalog.has_value());
    ASSERT_EQ(0, loaded_catalog->size());
    AssertCatalogDataEqual(catalog::get_all_schemas(), loaded_catalog.value());
    ASSERT_FALSE(std::filesystem::exists(test_data_dir / "test_table.data"));
}

TEST_F(ExecutorDropTableTest, DropNonExistentTable) {
    // Get state before DROP TABLE
    const std::vector<catalog::TableSchema>& expected_catalog_state_before_drop = catalog::get_all_schemas();

    // Attempt to drop a non-existent table
    command::DropTableCommand cmd = {"non_existent_table"};
    results::ExecutionResult result = executor::execute_drop_table_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "ERROR: Table 'non_existent_table' does not exist.");

    // Verify on-disk catalog is unchanged
    auto on_disk_catalog_opt = loadCatalogFromDisk();
    ASSERT_TRUE(on_disk_catalog_opt.has_value());
    AssertCatalogDataEqual(expected_catalog_state_before_drop, *on_disk_catalog_opt);

    // Verify that the in-memory catalog is consistent with the catalog.json on disk.
    auto loaded_catalog = loadCatalogFromDisk();
    ASSERT_TRUE(loaded_catalog.has_value());
    AssertCatalogDataEqual(catalog::get_all_schemas(), loaded_catalog.value());

    // Verify the data file for "test_table" still exists
    ASSERT_TRUE(std::filesystem::exists(test_data_dir / "test_table.data"));
}

TEST_F(ExecutorShowTablesTest, SuccessfulShowTables) {
    results::ExecutionResult result = executor::execute_show_tables_command();
    // 1. Assert that the execution was successful and returned data.
    ASSERT_EQ(result.get_status(), results::ResultStatus::SUCCESS);
    ASSERT_TRUE(result.has_data());

    // 2. Assert the result data.
    results::ResultSet expected_result_data;
    expected_result_data.headers = {"Table Name"};
    expected_result_data.rows = {{"test_table"}};
    ASSERT_EQ(expected_result_data, result.get_data());
}

TEST_F(ExecutorTestBase, SuccessfulShowTablesWhenNoTablesExist) {
    results::ExecutionResult result = executor::execute_show_tables_command();

    // 1. Assert that the execution was successful and returned data.
    ASSERT_EQ(result.get_status(), results::ResultStatus::SUCCESS);
    ASSERT_TRUE(result.has_data());

    // 2. Assert the result data is empty.
    results::ResultSet expected_result_data;
    expected_result_data.headers = {"Table Name"};
    expected_result_data.rows = {};  // No tables exist
    ASSERT_EQ(expected_result_data, result.get_data());
}

TEST_F(ExecutorInsertTablesTest, SuccessfulInsertIntoTable) {
    command::InsertCommand cmd;
    cmd.table_name = "test_table";
    cmd.values = {"1", "Alice"};

    results::ExecutionResult result = executor::execute_insert_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "1 row inserted.");
    AssertRecordForSlot(0, 0, std::vector<std::string>({"1", "Alice"}));
}

TEST_F(ExecutorInsertTablesTest, SuccessfulInsertIntoWithColumnsSpecified) {
    command::InsertCommand cmd;
    cmd.table_name = "test_table";
    cmd.values = {"1", "Alice"};
    cmd.columns = {"id", "name"};  // Specify columns explicitly

    results::ExecutionResult result = executor::execute_insert_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "1 row inserted.");
    AssertRecordForSlot(0, 0, std::vector<std::string>({"1", "Alice"}));
}

TEST_F(ExecutorInsertTablesTest, SuccessfulInsertIntoWithColumnsReordered) {
    command::InsertCommand cmd;
    cmd.table_name = "test_table";
    cmd.values = {"Alice", "1"};
    cmd.columns = {"name", "id"};  // Specify columns explicitly with a different order

    results::ExecutionResult result = executor::execute_insert_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "1 row inserted.");
    AssertRecordForSlot(0, 0, std::vector<std::string>({"1", "Alice"}));
}

TEST_F(ExecutorInsertTablesTest, InsertFailsWithTypeMismatchedValues) {
    command::InsertCommand cmd;
    cmd.table_name = "test_table";
    cmd.values = {"bad value for id", "Alice"};
    cmd.columns = {"id", "name"};

    results::ExecutionResult result = executor::execute_insert_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "ERROR: Value 'bad value for id' for column 'id' is not a valid integer.");
}

TEST_F(ExecutorInsertTablesTest, InsertFailsWithNonExistentColumn) {
    command::InsertCommand cmd;
    cmd.table_name = "test_table";
    cmd.values = {"1", "Alice"};
    cmd.columns = {"id", "nonexistentcolumn"};  // Specify a column that does not exist

    results::ExecutionResult result = executor::execute_insert_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "ERROR: Column 'nonexistentcolumn' does not exist in table 'test_table'.");
}

TEST_F(ExecutorInsertTablesTest, InsertFillsPageAndSpills) {
    // For simplicity, let's estimate the size.
    // id "0" is 1 byte. name is 100 bytes. 2 length prefixes (2*2=4 bytes).
    // Total ~ 1 + 100 + 4 = 105 bytes.
    const size_t record_size_estimate = 105;
    const size_t usable_space = simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE;
    const size_t space_per_record = record_size_estimate + sizeof(simpledb::storage::Page::Slot);
    const int num_records_to_fill_page = usable_space / space_per_record;
    const std::string fixed_name(100, 'A');  // A 100-byte string

    command::InsertCommand cmd;
    cmd.table_name = "test_table";

    // Fill the page
    for (int i = 0; i < num_records_to_fill_page; ++i) {
        cmd.values = {"0", fixed_name};
        results::ExecutionResult result = executor::execute_insert_command(cmd, test_data_dir);
        ASSERT_EQ(result.get_message(), "1 row inserted.");
    }

    // Assert that the first page is filled with num_records_to_fill_page records
    for (int i = 0; i < num_records_to_fill_page; ++i) {
        AssertRecordForSlot(0, i, std::vector<std::string>({"0", fixed_name}));
    }

    // Now insert one more record to trigger a spill
    // Use a different fixed name filled with 'B's to differentiate it
    const std::string fixed_name_b(100, 'B');  // A 100-byte string

    cmd.values = {"1", fixed_name_b};
    results::ExecutionResult result = executor::execute_insert_command(cmd, test_data_dir);
    ASSERT_EQ(result.get_message(), "1 row inserted.");

    // Assert the second page is created and contains the new record
    AssertRecordForSlot(1, 0, std::vector<std::string>({"1", fixed_name_b}));
}
