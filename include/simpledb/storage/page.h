//
// Created by Akshat Jain on 14/06/25.
//

#ifndef SIMPLEDB_STORAGE_PAGE_H
#define SIMPLEDB_STORAGE_PAGE_H

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

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

        uint8_t GetVersion() const { return data_[VERSION_OFFSET]; }

        void SetVersion(uint8_t version) { data_[VERSION_OFFSET] = version; }

        uint16_t GetNumRecords() const {
            uint16_t num_records;
            memcpy(&num_records, &data_[NUM_RECORDS_OFFSET], sizeof(uint16_t));
            return num_records;
        }

        void SetNumRecords(uint16_t num_records) { memcpy(&data_[NUM_RECORDS_OFFSET], &num_records, sizeof(uint16_t)); }

        uint16_t GetFreeSpacePointer() const {
            uint16_t free_space_ptr;
            memcpy(&free_space_ptr, &data_[FREE_SPACE_PTR_OFFSET], sizeof(uint16_t));
            return free_space_ptr;
        }

        void SetFreeSpacePointer(uint16_t free_space_ptr) {
            memcpy(&data_[FREE_SPACE_PTR_OFFSET], &free_space_ptr, sizeof(uint16_t));
        }

        /**
         * Reads the slot at the given slot number from the page.
         */
        Slot GetSlot(uint16_t slot_number) const {
            Slot slot;
            size_t offset = HEADER_SIZE + slot_number * sizeof(Slot);
            memcpy(&slot, &data_[offset], sizeof(Slot));
            return slot;
        }

        /**
         * Calculates the amount of contiguous free space left on the page.
         */
        uint16_t GetFreeSpace() const {
            // The free space is the difference between the free space pointer and the end of the slot array.
            const uint16_t slot_array_end = HEADER_SIZE + (GetNumRecords() * sizeof(Slot));
            const uint16_t free_space_ptr = GetFreeSpacePointer();
            if (free_space_ptr < slot_array_end) {
                // Defensive check to ensure this function doesn't return a gigantic value because of underflow.
                return 0;
            }
            return free_space_ptr - slot_array_end;
        }

        /**
         * Sets up the page header for a new, empty page.
         */
        void Initialize() {
            SetVersion(1);
            SetNumRecords(0);
            SetFreeSpacePointer(PAGE_SIZE);
        }

        bool AddRecord(const std::vector<char>& record_data) {
            const size_t record_size = record_data.size();

            // 1. Check if there is enough free space for the new record and a new slot.
            if (record_size + sizeof(Slot) > GetFreeSpace()) {
                return false;  // Not enough space
            }

            // 2. Get current header values needed for calculations.
            const uint16_t num_records = GetNumRecords();
            const uint16_t free_space_ptr = GetFreeSpacePointer();

            // 3. Calculate new positions for the record data and the new slot.
            const uint16_t new_slot_offset = HEADER_SIZE + num_records * sizeof(Slot);
            const uint16_t new_record_offset = free_space_ptr - record_size;

            // 4. Write the record data into the page.
            memcpy(&data_[new_record_offset], record_data.data(), record_size);

            // 5. Create and write a new slot for the record.
            Slot new_slot;
            new_slot.record_length = static_cast<uint16_t>(record_size);
            new_slot.record_offset = new_record_offset;
            memcpy(&data_[new_slot_offset], &new_slot, sizeof(Slot));

            // 6. Update the page header.
            SetNumRecords(num_records + 1);
            SetFreeSpacePointer(new_record_offset);
            return true;
        }

       private:
        std::array<char, PAGE_SIZE> data_;  // Data stored in the page
    };
}  // namespace simpledb::storage

#endif  // SIMPLEDB_STORAGE_PAGE_H
