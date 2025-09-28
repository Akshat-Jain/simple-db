//
// Created by Akshat Jain on 14/06/25.
//

#include "simpledb/storage/index_page.h"

#include <gtest/gtest.h>

TEST(InternalPageTest, Initialization) {
    simpledb::storage::IndexPage page;
    page.Initialize(true);  // Initialize as internal page
    EXPECT_TRUE(page.IsInternalPage());
    EXPECT_EQ(page.GetNumEntries(), 0);
    EXPECT_EQ(page.GetPrevPage(), 0);
    EXPECT_EQ(page.GetNextPage(), 0);
}

TEST(LeafPageTest, Initialization) {
    simpledb::storage::IndexPage page;
    page.Initialize(false);  // Initialize as leaf page
    EXPECT_FALSE(page.IsInternalPage());
    EXPECT_EQ(page.GetNumEntries(), 0);
    EXPECT_EQ(page.GetPrevPage(), 0);
    EXPECT_EQ(page.GetNextPage(), 0);
}

TEST(InternalPageTest, AddEntries) {
    simpledb::storage::IndexPage page;
    page.Initialize(true);  // Internal page

    // Set leftmost child
    page.SetLeftmostChild(5);
    EXPECT_EQ(*page.GetLeftmostChild(), 5);

    // Add entries
    for (uint16_t i = 0; i < 3; ++i) {
        simpledb::storage::InternalEntry entry;
        entry.separator_key = (i + 1) * 100;
        entry.right_child = (i + 1) * 10;
        page.SetInternalEntry(i, entry);
        page.SetNumEntries(i + 1);
    }

    EXPECT_EQ(page.GetNumEntries(), 3);

    // Verify entries
    for (uint16_t i = 0; i < 3; ++i) {
        auto entry = page.GetInternalEntry(i);
        EXPECT_EQ(entry->separator_key, (i + 1) * 100);
        EXPECT_EQ(entry->right_child, (i + 1) * 10);
    }
}

TEST(LeafPageTest, AddEntries) {
    simpledb::storage::IndexPage page;
    page.Initialize(false);  // Leaf page

    // Add entries
    for (uint16_t i = 0; i < 3; ++i) {
        simpledb::storage::LeafEntry entry;
        entry.key = (i + 1) * 100;
        entry.table_page_id = (i + 1) * 5;
        entry.table_row_offset = (i + 1) * 10;
        page.SetLeafEntry(i, entry);
        page.SetNumEntries(i + 1);
    }

    EXPECT_EQ(page.GetNumEntries(), 3);

    // Verify entries
    for (uint16_t i = 0; i < 3; ++i) {
        auto entry = page.GetLeafEntry(i);
        EXPECT_EQ(entry->key, (i + 1) * 100);
        EXPECT_EQ(entry->table_page_id, (i + 1) * 5);
        EXPECT_EQ(entry->table_row_offset, (i + 1) * 10);
    }
}

TEST(LeafPageTest, SetLeftRightSiblings) {
    simpledb::storage::IndexPage page;
    page.Initialize(false);  // Leaf page

    page.SetPrevPage(3);
    page.SetNextPage(7);

    EXPECT_EQ(page.GetPrevPage(), 3);
    EXPECT_EQ(page.GetNextPage(), 7);
}

TEST(InternalPageTest, LeftmostChild) {
    simpledb::storage::IndexPage page;
    page.Initialize(true);  // Internal page

    // Test setting and getting leftmost child
    page.SetLeftmostChild(42);
    EXPECT_EQ(*page.GetLeftmostChild(), 42);

    // Test different value
    page.SetLeftmostChild(100);
    EXPECT_EQ(*page.GetLeftmostChild(), 100);
}

TEST(LeafPageTest, LeftmostChildReturnsNull) {
    simpledb::storage::IndexPage page;
    page.Initialize(false);  // Leaf page

    // GetLeftmostChild should return nullptr for leaf pages
    EXPECT_EQ(page.GetLeftmostChild(), nullptr);

    // SetLeftmostChild should do nothing for leaf pages (no crash)
    page.SetLeftmostChild(42);  // Should be ignored
    EXPECT_EQ(page.GetLeftmostChild(), nullptr);
}

TEST(IndexPageTest, IsFull) {
    // Test internal page
    simpledb::storage::IndexPage internal_page;
    internal_page.Initialize(true);

    EXPECT_FALSE(internal_page.IsFull());

    // Set to max capacity - 1
    internal_page.SetNumEntries(simpledb::storage::MAX_INTERNAL_ENTRIES - 1);
    EXPECT_FALSE(internal_page.IsFull());

    // Set to max capacity
    internal_page.SetNumEntries(simpledb::storage::MAX_INTERNAL_ENTRIES);
    EXPECT_TRUE(internal_page.IsFull());

    // Test leaf page
    simpledb::storage::IndexPage leaf_page;
    leaf_page.Initialize(false);

    EXPECT_FALSE(leaf_page.IsFull());

    // Set to max capacity - 1
    leaf_page.SetNumEntries(simpledb::storage::MAX_LEAF_ENTRIES - 1);
    EXPECT_FALSE(leaf_page.IsFull());

    // Set to max capacity
    leaf_page.SetNumEntries(simpledb::storage::MAX_LEAF_ENTRIES);
    EXPECT_TRUE(leaf_page.IsFull());
}

TEST(IndexPageTest, OutOfBoundsAccess) {
    simpledb::storage::IndexPage internal_page;
    internal_page.Initialize(true);
    internal_page.SetNumEntries(2);  // Only 2 valid entries

    // Valid access
    EXPECT_NE(internal_page.GetInternalEntry(0), nullptr);
    EXPECT_NE(internal_page.GetInternalEntry(1), nullptr);

    // Out of bounds access (beyond num_entries)
    EXPECT_EQ(internal_page.GetInternalEntry(2), nullptr);
    EXPECT_EQ(internal_page.GetInternalEntry(100), nullptr);

    simpledb::storage::IndexPage leaf_page;
    leaf_page.Initialize(false);
    leaf_page.SetNumEntries(1);  // Only 1 valid entry

    // Valid access
    EXPECT_NE(leaf_page.GetLeafEntry(0), nullptr);

    // Out of bounds access
    EXPECT_EQ(leaf_page.GetLeafEntry(1), nullptr);
    EXPECT_EQ(leaf_page.GetLeafEntry(50), nullptr);
}

TEST(IndexPageTest, WrongPageTypeAccess) {
    // Test accessing leaf methods on internal page
    simpledb::storage::IndexPage internal_page;
    internal_page.Initialize(true);
    internal_page.SetNumEntries(1);

    EXPECT_EQ(internal_page.GetLeafEntry(0), nullptr);

    // Test accessing internal methods on leaf page
    simpledb::storage::IndexPage leaf_page;
    leaf_page.Initialize(false);
    leaf_page.SetNumEntries(1);

    EXPECT_EQ(leaf_page.GetInternalEntry(0), nullptr);
}

TEST(IndexPageTest, MaxCapacity) {
    // Test that we can set entries up to max capacity
    simpledb::storage::IndexPage internal_page;
    internal_page.Initialize(true);

    // Should be able to set entry at max index
    simpledb::storage::InternalEntry entry{999, 888};
    internal_page.SetInternalEntry(simpledb::storage::MAX_INTERNAL_ENTRIES - 1, entry);
    internal_page.SetNumEntries(simpledb::storage::MAX_INTERNAL_ENTRIES);

    auto retrieved = internal_page.GetInternalEntry(simpledb::storage::MAX_INTERNAL_ENTRIES - 1);
    EXPECT_EQ(retrieved->separator_key, 999);
    EXPECT_EQ(retrieved->right_child, 888);

    // Test leaf page max capacity
    simpledb::storage::IndexPage leaf_page;
    leaf_page.Initialize(false);

    simpledb::storage::LeafEntry leaf_entry{777, 666, 555};
    leaf_page.SetLeafEntry(simpledb::storage::MAX_LEAF_ENTRIES - 1, leaf_entry);
    leaf_page.SetNumEntries(simpledb::storage::MAX_LEAF_ENTRIES);

    auto retrieved_leaf = leaf_page.GetLeafEntry(simpledb::storage::MAX_LEAF_ENTRIES - 1);
    EXPECT_EQ(retrieved_leaf->key, 777);
    EXPECT_EQ(retrieved_leaf->table_page_id, 666);
    EXPECT_EQ(retrieved_leaf->table_row_offset, 555);
}

TEST(IndexPageTest, StructSizes) {
    // Verify struct sizes match expectations from documentation
    EXPECT_EQ(sizeof(simpledb::storage::BTreePageHeader), 12);
    EXPECT_EQ(sizeof(simpledb::storage::InternalEntry), 8);
    EXPECT_EQ(sizeof(simpledb::storage::LeafEntry), 10);  // With __attribute__((packed))

    // Verify capacity calculations
    EXPECT_EQ(simpledb::storage::MAX_LEAF_ENTRIES, 408);      // (4096-12)/10
    EXPECT_EQ(simpledb::storage::MAX_INTERNAL_ENTRIES, 510);  // (4096-12-4)/8
}
