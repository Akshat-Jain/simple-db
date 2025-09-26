//
// Created by Akshat Jain on 14/06/25.
//

#include "simpledb/storage/data_page.h"

#include <cstring>
#include <vector>

namespace simpledb::storage {

    uint8_t DataPage::GetVersion() const { return data_[DataPage::VERSION_OFFSET]; }

    void DataPage::SetVersion(uint8_t version) { data_[DataPage::VERSION_OFFSET] = version; }

    uint16_t DataPage::GetNumRecords() const {
        uint16_t num_records;
        memcpy(&num_records, &data_[DataPage::NUM_RECORDS_OFFSET], sizeof(uint16_t));
        return num_records;
    }

    void DataPage::SetNumRecords(uint16_t num_records) {
        memcpy(&data_[DataPage::NUM_RECORDS_OFFSET], &num_records, sizeof(uint16_t));
    }

    uint16_t DataPage::GetFreeSpacePointer() const {
        uint16_t free_space_ptr;
        memcpy(&free_space_ptr, &data_[DataPage::FREE_SPACE_PTR_OFFSET], sizeof(uint16_t));
        return free_space_ptr;
    }

    void DataPage::SetFreeSpacePointer(uint16_t free_space_ptr) {
        memcpy(&data_[DataPage::FREE_SPACE_PTR_OFFSET], &free_space_ptr, sizeof(uint16_t));
    }

    DataPage::Slot DataPage::GetSlot(uint16_t slot_number) const {
        DataPage::Slot slot;
        size_t offset = DataPage::HEADER_SIZE + slot_number * sizeof(DataPage::Slot);
        memcpy(&slot, &data_[offset], sizeof(DataPage::Slot));
        return slot;
    }

    uint16_t DataPage::GetFreeSpace() const {
        // The free space is the difference between the free space pointer and the end of the slot array.
        const uint16_t slot_array_end = DataPage::HEADER_SIZE + (GetNumRecords() * sizeof(DataPage::Slot));
        const uint16_t free_space_ptr = GetFreeSpacePointer();
        if (free_space_ptr < slot_array_end) {
            // Defensive check to ensure this function doesn't return a gigantic value because of underflow.
            return 0;
        }
        return free_space_ptr - slot_array_end;
    }

    void DataPage::Initialize() {
        SetVersion(1);
        SetNumRecords(0);
        SetFreeSpacePointer(PAGE_SIZE);
    }

    bool DataPage::AddRecord(const std::vector<char>& record_data) {
        const size_t record_size = record_data.size();

        // 1. Check if there is enough free space for the new record and a new slot.
        if (record_size + sizeof(DataPage::Slot) > GetFreeSpace()) {
            return false;  // Not enough space
        }

        // 2. Get current header values needed for calculations.
        const uint16_t num_records = GetNumRecords();
        const uint16_t free_space_ptr = GetFreeSpacePointer();

        // 3. Calculate new positions for the record data and the new slot.
        const uint16_t new_slot_offset = DataPage::HEADER_SIZE + num_records * sizeof(DataPage::Slot);
        const uint16_t new_record_offset = free_space_ptr - record_size;

        // 4. Write the record data into the page.
        memcpy(&data_[new_record_offset], record_data.data(), record_size);

        // 5. Create and write a new slot for the record.
        DataPage::Slot new_slot;
        new_slot.record_length = static_cast<uint16_t>(record_size);
        new_slot.record_offset = new_record_offset;
        memcpy(&data_[new_slot_offset], &new_slot, sizeof(DataPage::Slot));

        // 6. Update the page header.
        SetNumRecords(num_records + 1);
        SetFreeSpacePointer(new_record_offset);
        return true;
    }

    std::vector<char> DataPage::GetRecord(const DataPage::Slot& slot) const {
        std::vector<char> record_data(slot.record_length);
        memcpy(record_data.data(), &data_[slot.record_offset], slot.record_length);
        return record_data;
    }

    const char* DataPage::GetData() const { return data_.data(); }

    char* DataPage::GetData() { return data_.data(); }

}  // namespace simpledb::storage
