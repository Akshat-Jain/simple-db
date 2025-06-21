//
// Created by Akshat Jain on 21/06/25.
//

#include "simpledb/storage/page.h"
#include "simpledb/storage/table_heap.h"

#include <filesystem>
#include <gtest/gtest.h>
#include <vector>

class TableHeapIteratorTest : public ::testing::Test {
   protected:
    std::string test_file_path;

    void SetUp() override {
        // Create a unique temporary file path for each test.
        const ::testing::TestInfo* const test_info = ::testing::UnitTest::GetInstance()->current_test_info();
        test_file_path = std::filesystem::temp_directory_path().string() + "/simpledb_table_heap_iterator_" +
                         test_info->test_suite_name() + "_" + test_info->name() + ".data";
    }

    void TearDown() override { std::filesystem::remove(test_file_path); }
};

TEST_F(TableHeapIteratorTest, IterateOverEmptyTable) {
    simpledb::storage::TableHeap tableHeap(test_file_path);
    simpledb::storage::TableHeap::Iterator iterator = tableHeap.begin();
    ASSERT_FALSE(iterator.next().has_value());
}

TEST_F(TableHeapIteratorTest, IterateOverSingleRecord) {
    simpledb::storage::TableHeap tableHeap(test_file_path);
    std::vector<char> record_data(100, 'A');
    ASSERT_TRUE(tableHeap.InsertRecord(record_data));

    simpledb::storage::TableHeap::Iterator iterator = tableHeap.begin();
    ASSERT_EQ(record_data, iterator.next().value());

    // Validate that there are no more records
    ASSERT_FALSE(iterator.next().has_value());
}

TEST_F(TableHeapIteratorTest, IterateOverMultipleRecordsInSinglePage) {
    simpledb::storage::TableHeap tableHeap(test_file_path);
    for (int i = 0; i < 5; ++i) {
        std::vector<char> record_data(100 + i, 'A');
        ASSERT_TRUE(tableHeap.InsertRecord(record_data));
    }

    simpledb::storage::TableHeap::Iterator iterator = tableHeap.begin();
    for (int i = 0; i < 5; ++i) {
        std::vector<char> expected_record_data(100 + i, 'A');
        ASSERT_EQ(expected_record_data, iterator.next().value());
    }

    // Validate that there are no more records
    ASSERT_FALSE(iterator.next().has_value());
}

TEST_F(TableHeapIteratorTest, IterateOverMultiplePages) {
    simpledb::storage::TableHeap tableHeap(test_file_path);
    const int record_size = 100;  // Size of each record to be added
    const size_t usable_space = simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE;
    const size_t space_per_record = record_size + sizeof(simpledb::storage::Page::Slot);
    const int num_records_to_fill_page = usable_space / space_per_record;
    std::vector<char> record_data(record_size, 'A');

    // Fill 2 pages with records
    for (int i = 0; i < num_records_to_fill_page * 2; ++i) {
        ASSERT_TRUE(tableHeap.InsertRecord(record_data));
    }

    simpledb::storage::TableHeap::Iterator iterator = tableHeap.begin();
    for (int i = 0; i < num_records_to_fill_page * 2; ++i) {
        ASSERT_EQ(record_data, iterator.next().value());
    }

    // Validate that there are no more records
    ASSERT_FALSE(iterator.next().has_value());
}

TEST_F(TableHeapIteratorTest, IterateOverTableWithEmptyPageInBetween) {
    std::vector<char> record_data(100, 'A');

    simpledb::storage::Page page0;
    page0.Initialize();
    int num_records_page0 = 5;
    for (int i = 0; i < num_records_page0; ++i) {
        page0.AddRecord(record_data);
    }

    simpledb::storage::Page page1;
    page1.Initialize();  // This page will be empty

    simpledb::storage::Page page2;
    page2.Initialize();
    int num_records_page2 = 3;
    for (int i = 0; i < num_records_page2; ++i) {
        page2.AddRecord(record_data);
    }

    // Write pages to the file directly for testing
    std::ofstream file(test_file_path, std::ios::binary);
    file.write(page0.GetData(), simpledb::storage::PAGE_SIZE);
    file.write(page1.GetData(), simpledb::storage::PAGE_SIZE);
    file.write(page2.GetData(), simpledb::storage::PAGE_SIZE);
    file.close();

    simpledb::storage::TableHeap tableHeap(test_file_path);
    simpledb::storage::TableHeap::Iterator iterator = tableHeap.begin();
    for (int i = 0; i < num_records_page0 + num_records_page2; ++i) {
        ASSERT_EQ(record_data, iterator.next().value());
    }
    ASSERT_FALSE(iterator.next().has_value());
}
