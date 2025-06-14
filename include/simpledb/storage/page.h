//
// Created by Akshat Jain on 14/06/25.
//

#ifndef SIMPLEDB_STORAGE_PAGE_H
#define SIMPLEDB_STORAGE_PAGE_H

#include <array>
#include <cstdint>

namespace simpledb::storage {
    /*
        <---------------------------------- 4096 bytes -------------------------------------->
        +---------------+-----------------+-...-+----------------------+---------------------+
        |  Page Header  | Slot 1 | Slot 2 | ... |      Free Space      | Record 2 | Record 1 |
        +---------------+-----------------+-...-+----------------------+---------------------+
        ^               ^                 ^     ^                      ^                     ^
        0             HeaderEnd         SlotEnd FreeSpaceStart       RecordHeapStart        4095
     */

    constexpr size_t PAGE_SIZE = 4096;  // Size of a page in bytes

    class Page {
       public:
        // These constants define the offsets of various fields in the page header.

        // Size of the page header in bytes.
        // 1 byte for version, 2 bytes for number of records, 2 bytes for free space pointer, and 3 bytes for padding.
        static constexpr size_t HEADER_SIZE = 8;

        // Stores the version of the page format.
        static constexpr size_t VERSION_OFFSET = 0;

        // Pointer to the number of records currently on this page.
        static constexpr size_t NUM_RECORDS_OFFSET = 1;

        // Pointer to the free space in the page (record data heap), where new records can be added.
        static constexpr size_t FREE_SPACE_PTR_OFFSET = 3;

        /**
         * A Slot stores metadata about a record on a page, specifically
         * its location (offset) and size.
         */
        struct Slot {
            uint16_t record_offset;
            uint16_t record_length;
        };

        uint8_t GetVersion() const;

        void SetVersion(uint8_t version);

        uint16_t GetNumRecords() const;

        void SetNumRecords(uint16_t num_records);

        uint16_t GetFreeSpacePointer() const;

        void SetFreeSpacePointer(uint16_t free_space_ptr);

        /**
         * Reads the slot at the given slot number from the page.
         */
        Slot GetSlot(uint16_t slot_number) const;

        /**
         * Calculates the amount of contiguous free space left on the page.
         */
        uint16_t GetFreeSpace() const;

        /**
         * Sets up the page header for a new, empty page.
         */
        void Initialize();

        bool AddRecord(const std::vector<char>& record_data);

       private:
        std::array<char, PAGE_SIZE> data_;  // Data stored in the page
    };
}  // namespace simpledb::storage

#endif  // SIMPLEDB_STORAGE_PAGE_H
