//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/storage/table_heap.h"
#include "simpledb/execution/table_scan_operator.h"
#include "simpledb/catalog.h"
#include "simpledb/command.h"
#include "simpledb/executor.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <algorithm>

class TableScanOperatorTest : public ::testing::Test {
   protected:
    std::filesystem::path test_data_dir;

    void SetUp() override {
        // Create a unique temporary file path for each test.
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        test_data_dir = std::filesystem::temp_directory_path().string() + "simpledb_" + test_info->test_suite_name() +
                        "_" + test_info->name();
        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
        std::filesystem::create_directories(test_data_dir);

        catalog::initialize(test_data_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_data_dir)) {
            std::filesystem::remove_all(test_data_dir);
        }
    }
};

TEST_F(TableScanOperatorTest, ScanEmptyTable) {
    command::CreateTableCommand create_cmd;
    create_cmd.table_name = "empty_table";
    create_cmd.column_definitions.push_back({"id", command::Datatype::INT});
    executor::execute_create_table_command(create_cmd, test_data_dir);

    simpledb::execution::TableScanOperator scan_operator("empty_table", test_data_dir);
    auto row = scan_operator.next();
    ASSERT_FALSE(row.has_value());  // Should return no rows for an empty table
}

TEST_F(TableScanOperatorTest, ScanSingleRowTable) {
    // Create a table and insert one logical row.
    command::CreateTableCommand create_cmd;
    create_cmd.table_name = "single_row_table";
    create_cmd.column_definitions.push_back({"name", command::Datatype::TEXT});
    create_cmd.column_definitions.push_back({"email", command::Datatype::TEXT});
    executor::execute_create_table_command(create_cmd, test_data_dir);

    row::Row row = {"AkJn", "akjn@db.com"};

    command::InsertCommand insert_cmd;
    insert_cmd.table_name = "single_row_table";
    insert_cmd.values = row;
    executor::execute_insert_command(insert_cmd, test_data_dir);

    simpledb::execution::TableScanOperator scan_operator("single_row_table", test_data_dir);

    // Assert: Check the first row.
    auto first_row = scan_operator.next();
    ASSERT_TRUE(first_row.has_value());
    ASSERT_EQ(*first_row, row);

    // There should be no more rows.
    ASSERT_FALSE(scan_operator.next().has_value());
}

TEST_F(TableScanOperatorTest, ScanMultiPageTable) {
    const int record_size = 100;  // Size of each record to be added
    const size_t usable_space = simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE;
    const size_t space_per_record = record_size + sizeof(simpledb::storage::Page::Slot);
    const int num_records_to_fill_page = usable_space / space_per_record;
    std::string record_data(record_size, 'A');

    command::CreateTableCommand create_cmd;
    create_cmd.table_name = "multi_page_table";
    create_cmd.column_definitions.push_back({"data", command::Datatype::TEXT});
    executor::execute_create_table_command(create_cmd, test_data_dir);
    command::InsertCommand insert_cmd;
    insert_cmd.table_name = "multi_page_table";

    // Fill 2 pages with records.
    for (int i = 0; i < num_records_to_fill_page * 2; ++i) {
        insert_cmd.values = {std::string(record_data)};
        executor::execute_insert_command(insert_cmd, test_data_dir);
    }
    simpledb::execution::TableScanOperator scan_operator("multi_page_table", test_data_dir);

    int count = 0;
    while (true) {
        auto row = scan_operator.next();
        if (!row.has_value()) {
            break;  // No more rows
        }
        ASSERT_EQ(row->size(), 1);
        ASSERT_EQ((*row)[0], record_data);
        count++;
    }
    ASSERT_EQ(num_records_to_fill_page * 2, count);
}
