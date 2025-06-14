//
// Created by Akshat Jain on 14/06/25.
//

#include "simpledb/storage/page.h"

#include <gtest/gtest.h>

TEST(PageTest, Initialization) {
    simpledb::storage::Page page;
    page.Initialize();

    ASSERT_EQ(page.GetVersion(), 1);
    ASSERT_EQ(page.GetNumRecords(), 0);
    ASSERT_EQ(page.GetFreeSpacePointer(), simpledb::storage::PAGE_SIZE);
    ASSERT_EQ(page.GetFreeSpace(), simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE);
}

TEST(PageTest, AddOneRecord) {
    simpledb::storage::Page page;
    page.Initialize();

    size_t record_size = 100;                         // Size of the record to be added
    std::vector<char> record_data(record_size, 'A');  // Create a record of 100 bytes filled with 'A'
    ASSERT_TRUE(page.AddRecord(record_data));

    ASSERT_EQ(page.GetNumRecords(), 1);

    auto slot = page.GetSlot(0);
    ASSERT_EQ(slot.record_offset, simpledb::storage::PAGE_SIZE - record_size);
    ASSERT_EQ(slot.record_length, record_size);

    ASSERT_EQ(page.GetFreeSpace(),
              simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE - record_size -
                  sizeof(simpledb::storage::Page::Slot));

    ASSERT_EQ(record_data, page.GetRecord(slot));
}

TEST(PageTest, AddMultipleRecords) {
    simpledb::storage::Page page;
    page.Initialize();

    size_t record_size = 50;                          // Size of each record to be added
    std::vector<char> record_data(record_size, 'A');  // Create a record of 50 bytes filled with 'A'

    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(page.AddRecord(record_data));
    }

    ASSERT_EQ(page.GetNumRecords(), 5);

    for (int i = 0; i < 5; ++i) {
        auto slot = page.GetSlot(i);
        ASSERT_EQ(slot.record_offset, simpledb::storage::PAGE_SIZE - (i + 1) * record_size);
        ASSERT_EQ(slot.record_length, record_size);
        ASSERT_EQ(record_data, page.GetRecord(slot));
    }

    ASSERT_EQ(page.GetFreeSpace(),
              simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE - 5 * record_size -
                  5 * sizeof(simpledb::storage::Page::Slot));
}

TEST(PageTest, AddRecordExceedingFreeSpace) {
    simpledb::storage::Page page;
    page.Initialize();

    size_t record_size =
        simpledb::storage::PAGE_SIZE - simpledb::storage::Page::HEADER_SIZE - sizeof(simpledb::storage::Page::Slot) + 1;
    std::vector<char> record_data(record_size, 'A');  // Create a record that exceeds the free space

    ASSERT_FALSE(page.AddRecord(record_data));  // Should fail to add the record
    ASSERT_EQ(page.GetNumRecords(), 0);         // No records should be added
}
