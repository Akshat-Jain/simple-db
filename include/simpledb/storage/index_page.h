//
// Created by Akshat Jain on 26/09/25.
//

#ifndef SIMPLE_DB_INDEX_PAGE_H
#define SIMPLE_DB_INDEX_PAGE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace simpledb::storage {
    constexpr size_t PAGE_SIZE = 4096;  // Size of a page in bytes

    /*
     * B+ Tree Index Page Layouts
     * ==========================
     *
     * INTERNAL PAGE LAYOUT (page_type = 0):
     * <------------------------------------- 4096 bytes ------------------------------------->
     * +------------------+---------------+----------+----------+-----+----------+-----------+
     * | BTreePageHeader  | LeftmostChild | Entry 0  | Entry 1  | ... | Entry K  |Free Space |
     * +------------------+---------------+----------+----------+-----+----------+-----------+
     * ^                  ^               ^          ^              ^            ^
     * 0                  12              16         24             16+K*8       4096
     *
     * BTreePageHeader (12 bytes):
     * +----------+------------+----------+----------+
     * | Type(0)  | NumEntries | PrevPage | NextPage |
     * +----------+------------+----------+----------+
     * 2 bytes    2 bytes      4 bytes    4 bytes
     *
     * LeftmostChild (4 bytes): Page ID for values < all separator keys
     *
     * InternalEntry (8 bytes each, max 510 entries):
     * +---------------+------------+
     * | SeparatorKey  | RightChild |
     * +---------------+------------+
     * 4 bytes         4 bytes
     *
     * Example Internal Page with 2 separator keys (3 children total):
     * Bytes 0-11:   [0, 2, 0, 0]              <- Header: internal, 2 keys, no siblings
     * Bytes 12-15:  [5]                       <- leftmost_child = page_5 (for keys < 100)
     * Bytes 16-23:  [100, 6]                  <- separator_key=100, right_child=page_6 (for keys >= 100)
     * Bytes 24-31:  [500, 7]                  <- separator_key=500, right_child=page_7 (for keys >= 500)
     * Bytes 32+:    [free space...]
     *
     * Child mapping:
     * - Values < 100      → Page 5  (leftmost_child)
     * - Values 100-499    → Page 6  (entries[0].right_child)
     * - Values >= 500     → Page 7  (entries[1].right_child)
     *
     * This gives us k=2 separator keys and k+1=3 children, as required by B+ tree invariants.
     *
     * ========================================================================================
     *
     * LEAF PAGE LAYOUT (page_type = 1):
     * <------------------------------------ 4096 bytes ------------------------------------>
     * +------------------+----------+----------+----------+-----+----------+--------------+
     * | BTreePageHeader  | Entry 0  | Entry 1  | Entry 2  | ... | Entry N  | Free Space   |
     * +------------------+----------+----------+----------+-----+----------+--------------+
     * ^                  ^          ^          ^              ^            ^
     * 0                  12         22         32             12+N*10      4096
     *
     * BTreePageHeader (12 bytes): Same as internal pages
     * +----------+------------+----------+----------+
     * | Type(1)  | NumEntries | PrevPage | NextPage |
     * +----------+------------+----------+----------+
     * 2 bytes    2 bytes      4 bytes    4 bytes
     *
     * LeafEntry (10 bytes each, max 408 entries):
     * +----------+-------------+----------------+
     * | Key      | TablePageID | TableRowOffset |
     * +----------+-------------+----------------+
     * 4 bytes    4 bytes       2 bytes
     *
     * Example Leaf Page with 3 entries:
     * Bytes 0-11:   [1, 3, 0, 8]              <- Header: leaf, 3 entries, no prev, next=page8
     * Bytes 12-21:  [15, 2, 100]              <- key=15 at table_page_2, offset_100
     * Bytes 22-31:  [42, 2, 200]              <- key=42 at table_page_2, offset_200
     * Bytes 32-41:  [89, 3, 50]               <- key=89 at table_page_3, offset_50
     * Bytes 42+:    [free space...]
     */

    /**
     * B+ Tree Page Header - Common to both leaf and internal pages
     */
    struct BTreePageHeader {
        uint16_t page_type;    // 0=internal, 1=leaf
        uint16_t num_entries;  // Number of keys (for internal pages) or data entries (for leaf pages)
        uint32_t prev_page;    // Left sibling page ID (0 if none)
        uint32_t next_page;    // Right sibling page ID (0 if none)
    };

    /**
     * Entry stored in internal pages
     * Represents: "values >= separator_key go to right_child page"
     */
    struct InternalEntry {
        int32_t separator_key;  // Boundary value
        uint32_t right_child;   // Child page for values >= separator_key
    };

    /**
     * Entry stored in leaf pages
     * Maps index key to actual table row location
     * __attribute__((packed)) ensures no padding bytes are added by the compiler.
     */
    struct LeafEntry {
        int32_t key;                // The indexed value (e.g., user_id)
        uint32_t table_page_id;     // Page in table data file
        uint16_t table_row_offset;  // Offset within that page
    } __attribute__((packed));

    // Capacity calculations for 4KB pages
    constexpr size_t HEADER_SIZE = sizeof(BTreePageHeader);                                         // 12 bytes
    constexpr size_t MAX_LEAF_ENTRIES = (PAGE_SIZE - HEADER_SIZE) / sizeof(LeafEntry);              // 408 entries
    constexpr size_t MAX_INTERNAL_ENTRIES = (PAGE_SIZE - HEADER_SIZE - 4) / sizeof(InternalEntry);  // 510 entries
    // Note: Internal pages need 4 extra bytes for leftmost_child, hence the -4

    class IndexPage {
       public:
        /**
         * Sets up the page header for a new, empty page.
         */
        void Initialize(bool isInternalPage);

        /**
         * @brief Returns true if this is an internal page, false if it's a leaf page.
         */
        bool IsInternalPage() const;

        /**
         * @brief Returns the number of entries in this page.
         * For internal pages, this is the number of separator keys.
         * For leaf pages, this is the number of data entries.
         */
        uint16_t GetNumEntries() const;

        /**
         * @brief Sets the number of entries in this page.
         * For internal pages, this is the number of separator keys.
         * For leaf pages, this is the number of data entries.
         */
        void SetNumEntries(uint16_t num_entries);

        /**
         * @brief Returns the previous (left sibling) page ID, or 0 if none.
         */
        uint32_t GetPrevPage() const;

        /**
         * @brief Sets the previous (left sibling) page ID.
         */
        void SetPrevPage(uint32_t prev_page_id);

        /**
         * @brief Returns the next (right sibling) page ID, or 0 if none.
         */
        uint32_t GetNextPage() const;

        /**
         * @brief Sets the next (right sibling) page ID.
         */
        void SetNextPage(uint32_t next_page_id);

        /**
         * @brief Returns a pointer to the leftmost child page ID (only for internal pages).
         */
        uint32_t* GetLeftmostChild();

        /**
         * @brief Sets the leftmost child page ID (only for internal pages).
         */
        void SetLeftmostChild(uint32_t leftmost_child);

        /**
         * @brief Checks if the page is full and cannot accommodate more entries.
         */
        bool IsFull() const;

        /**
         * @brief Returns a pointer to the InternalEntry at the given index.
         */
        InternalEntry* GetInternalEntry(size_t index);

        /**
         * @brief Returns a pointer to the LeafEntry at the given index.
         */
        LeafEntry* GetLeafEntry(size_t index);

        /**
         * @brief Sets the InternalEntry at the given index.
         */
        void SetInternalEntry(size_t index, const InternalEntry& entry);

        /**
         * @brief Sets the InternalEntry at the given index.
         */
        void SetLeafEntry(size_t index, const LeafEntry& entry);

        /**
         * @brief Returns a const pointer to the page's raw data, for read-only access.
         */
        const char* GetData() const;

        /**
         * @brief Returns a pointer to the page's raw data, allowing write access.
         */
        char* GetData();

       private:
        /**
         * @brief Retrieves the BTreePageHeader from the page's data.
         */
        BTreePageHeader GetHeader() const;

        std::array<char, PAGE_SIZE> data_;  // Data stored in the page
    };

}  // namespace simpledb::storage

#endif  // SIMPLE_DB_INDEX_PAGE_H
