//
// Created by Akshat Jain on 15/06/25.
//

#include "simpledb/storage/page.h"
#include "simpledb/storage/table_heap.h"

#include <gtest/gtest.h>

class TableHeapTest : public ::testing::Test {
   protected:
    std::string test_file_path;

    void SetUp() override {
        // Create a unique temporary file path for each test.
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        test_file_path = std::filesystem::temp_directory_path().string() + "simpledb_table_heap_" +
                         test_info->test_suite_name() + "_" + test_info->name() + ".data";
    }

    void TearDown() override { std::filesystem::remove(test_file_path); }
};

TEST_F(TableHeapTest, InsertIntoEmptyFile) {
    simpledb::storage::TableHeap table_heap(test_file_path);
    std::vector<char> record_data(100, 'A');  // Create a record of 100 bytes filled with 'A'

    ASSERT_TRUE(table_heap.InsertRecord(record_data));
    ASSERT_TRUE(std::filesystem::exists(test_file_path));
    ASSERT_EQ(std::filesystem::file_size(test_file_path), simpledb::storage::PAGE_SIZE);
}

TEST_F(TableHeapTest, InsertFillsOnePageAndCreatesAnother) {
    simpledb::storage::TableHeap table_heap(test_file_path);
    const int record_size = 100;  // Size of each record to be added
    const size_t usable_space = simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE;
    const size_t space_per_record = record_size + sizeof(simpledb::storage::Page::Slot);
    const int num_records_to_fill_page = usable_space / space_per_record;

    std::vector<char> record_data(record_size, 'A');  // Create a record of 100 bytes filled with 'A'

    // Fill the first page
    for (int i = 0; i < num_records_to_fill_page; ++i) {
        ASSERT_TRUE(table_heap.InsertRecord(record_data));
    }

    // Check that the file exists and has the correct size (one page)
    ASSERT_TRUE(std::filesystem::exists(test_file_path));
    ASSERT_EQ(std::filesystem::file_size(test_file_path), simpledb::storage::PAGE_SIZE);

    // Now insert one more record to trigger a new page allocation
    ASSERT_TRUE(table_heap.InsertRecord(record_data));

    // Check that the file size is now two pages
    ASSERT_EQ(std::filesystem::file_size(test_file_path), 2 * simpledb::storage::PAGE_SIZE);
}

TEST_F(TableHeapTest, InsertFailsIfRecordTooLarge) {
    simpledb::storage::TableHeap table_heap(test_file_path);
    std::vector<char> oversized_record_data(simpledb::storage::PAGE_SIZE, 'A');  // Create a record larger than a page

    // Attempt to insert a record that is too large
    ASSERT_FALSE(table_heap.InsertRecord(oversized_record_data));

    // Check that the file still exists and is empty
    ASSERT_TRUE(std::filesystem::exists(test_file_path));
    ASSERT_EQ(std::filesystem::file_size(test_file_path), 0);
}
