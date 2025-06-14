//
// Created by Akshat Jain on 14/06/25.
//

#include "simpledb/storage/page.h"

#include <cstring>
#include <vector>

namespace simpledb::storage {

    uint8_t Page::GetVersion() const { return data_[VERSION_OFFSET]; }

    void Page::SetVersion(uint8_t version) { data_[VERSION_OFFSET] = version; }

    uint16_t Page::GetNumRecords() const {
        uint16_t num_records;
        memcpy(&num_records, &data_[NUM_RECORDS_OFFSET], sizeof(uint16_t));
        return num_records;
    }

    void Page::SetNumRecords(uint16_t num_records) {
        memcpy(&data_[NUM_RECORDS_OFFSET], &num_records, sizeof(uint16_t));
    }

    uint16_t Page::GetFreeSpacePointer() const {
        uint16_t free_space_ptr;
        memcpy(&free_space_ptr, &data_[FREE_SPACE_PTR_OFFSET], sizeof(uint16_t));
        return free_space_ptr;
    }

    void Page::SetFreeSpacePointer(uint16_t free_space_ptr) {
        memcpy(&data_[FREE_SPACE_PTR_OFFSET], &free_space_ptr, sizeof(uint16_t));
    }

    Page::Slot Page::GetSlot(uint16_t slot_number) const {
        Slot slot;
        size_t offset = HEADER_SIZE + slot_number * sizeof(Slot);
        memcpy(&slot, &data_[offset], sizeof(Slot));
        return slot;
    }

    uint16_t Page::GetFreeSpace() const {
        // The free space is the difference between the free space pointer and the end of the slot array.
        const uint16_t slot_array_end = HEADER_SIZE + (GetNumRecords() * sizeof(Slot));
        const uint16_t free_space_ptr = GetFreeSpacePointer();
        if (free_space_ptr < slot_array_end) {
            // Defensive check to ensure this function doesn't return a gigantic value because of underflow.
            return 0;
        }
        return free_space_ptr - slot_array_end;
    }

    void Page::Initialize() {
        SetVersion(1);
        SetNumRecords(0);
        SetFreeSpacePointer(PAGE_SIZE);
    }

    bool Page::AddRecord(const std::vector<char>& record_data) {
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

    std::vector<char> Page::GetRecord(const Slot& slot) const {
        std::vector<char> record_data(slot.record_length);
        memcpy(record_data.data(), &data_[slot.record_offset], slot.record_length);
        return record_data;
    }

}  // namespace simpledb::storage
