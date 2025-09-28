//
// Created by Akshat Jain on 28/09/25.
//

#include "simpledb/storage/index_page.h"

namespace simpledb::storage {

    void IndexPage::Initialize(bool isInternalPage) {
        uint16_t page_type = isInternalPage ? 0 : 1;
        BTreePageHeader header = {page_type, 0, 0, 0};
        memcpy(data_.data(), &header, sizeof(BTreePageHeader));
    }

    bool IndexPage::IsInternalPage() const { return GetHeader().page_type == 0; }

    uint16_t IndexPage::GetNumEntries() const { return GetHeader().num_entries; }

    void IndexPage::SetNumEntries(uint16_t num_entries) { memcpy(&data_[2], &num_entries, sizeof(uint16_t)); }

    uint32_t IndexPage::GetPrevPage() const { return GetHeader().prev_page; }

    void IndexPage::SetPrevPage(uint32_t prev_page_id) { memcpy(&data_[4], &prev_page_id, sizeof(uint32_t)); }

    uint32_t IndexPage::GetNextPage() const { return GetHeader().next_page; }

    void IndexPage::SetNextPage(uint32_t next_page_id) { memcpy(&data_[8], &next_page_id, sizeof(uint32_t)); }

    uint32_t* IndexPage::GetLeftmostChild() {
        if (IsInternalPage()) {
            return reinterpret_cast<uint32_t*>(&data_[HEADER_SIZE]);
        }
        return nullptr;  // Not an internal page
    }

    void IndexPage::SetLeftmostChild(uint32_t leftmost_child) {
        if (IsInternalPage()) {
            memcpy(&data_[HEADER_SIZE], &leftmost_child, sizeof(uint32_t));
        }
    }

    bool IndexPage::IsFull() const {
        if (IsInternalPage()) {
            return GetNumEntries() >= MAX_INTERNAL_ENTRIES;
        } else {
            return GetNumEntries() >= MAX_LEAF_ENTRIES;
        }
    }

    InternalEntry* IndexPage::GetInternalEntry(size_t index) {
        if (IsInternalPage() && index < GetNumEntries()) {
            size_t offset = HEADER_SIZE + 4 + index * sizeof(InternalEntry);
            return reinterpret_cast<InternalEntry*>(&data_[offset]);
        }
        return nullptr;  // Invalid access
    }

    LeafEntry* IndexPage::GetLeafEntry(size_t index) {
        if (!IsInternalPage() && index < GetNumEntries()) {
            size_t offset = HEADER_SIZE + index * sizeof(LeafEntry);
            return reinterpret_cast<LeafEntry*>(&data_[offset]);
        }
        return nullptr;  // Invalid access
    }

    void IndexPage::SetInternalEntry(size_t index, const InternalEntry& entry) {
        if (IsInternalPage() && index < MAX_INTERNAL_ENTRIES) {
            size_t offset = HEADER_SIZE + 4 + index * sizeof(InternalEntry);
            memcpy(&data_[offset], &entry, sizeof(InternalEntry));
        }
    }
    void IndexPage::SetLeafEntry(size_t index, const LeafEntry& entry) {
        if (!IsInternalPage() && index < MAX_LEAF_ENTRIES) {
            size_t offset = HEADER_SIZE + index * sizeof(LeafEntry);
            memcpy(&data_[offset], &entry, sizeof(LeafEntry));
        }
    }

    const char* IndexPage::GetData() const { return data_.data(); }

    char* IndexPage::GetData() { return data_.data(); }

    BTreePageHeader IndexPage::GetHeader() const {
        BTreePageHeader header;
        memcpy(&header, data_.data(), sizeof(BTreePageHeader));
        return header;
    }
}  // namespace simpledb::storage