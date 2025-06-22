//
// Created by Akshat Jain on 14/06/25.
//

#include "simpledb/storage/page.h"
#include "simpledb/storage/table_heap.h"
#include "simpledb/utils/logging.h"

#include <iostream>
#include <stdexcept>

namespace simpledb::storage {

    TableHeap::Iterator::Iterator(TableHeap* parent_heap, PageId page_id, uint16_t slot_num)
        : parent_heap_(parent_heap), current_page_id_(page_id), current_slot_num_(slot_num) {}

    std::optional<std::vector<char>> TableHeap::Iterator::next() {
        uint32_t num_pages = parent_heap_->GetNumPages();
        while (true) {
            if (current_page_id_ >= num_pages) {
                // No more pages left, return std::nullopt to signal end of iteration.
                return std::nullopt;
            }

            Page page;
            parent_heap_->ReadPage(current_page_id_, &page);

            if (current_slot_num_ >= page.GetNumRecords()) {
                // If we have exhausted the current page, move to the next page.
                current_slot_num_ = 0;
                current_page_id_ += 1;
                continue;  // Re-check the new page.
            }

            // If we have a valid slot, break out of the loop to return the record.
            std::vector<char> record = page.GetRecord(page.GetSlot(current_slot_num_));
            current_slot_num_ += 1;  // Move to the next slot for the next call.
            return record;
        }
    }

    TableHeap::TableHeap(const std::string& table_data_path) : file_path_(table_data_path) {
        // Try to open the file for both reading and writing in binary mode.
        // This will succeed if the file already exists.
        file_stream_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);

        if (!file_stream_.is_open()) {
            // Clear the error state of the stream.
            file_stream_.clear();

            // Open the file in output mode. This will create an empty file.
            // We can close it immediately because we just wanted to ensure it exists.
            file_stream_.open(file_path_, std::ios::out | std::ios::binary);
            file_stream_.close();

            // Now that the file is guaranteed to exist, re-open it in the
            // correct read/write mode.
            file_stream_.open(file_path_, std::ios::in | std::ios::out | std::ios::binary);
        }

        // Perform a final check. If the file is still not open after all our attempts,
        // something is seriously wrong (e.g., bad permissions, invalid path).
        if (!file_stream_.is_open()) {
            throw std::runtime_error("Could not open or create table heap file: " + file_path_);
        }
    }

    bool TableHeap::InsertRecord(const std::vector<char>& record_data) {
        uint32_t num_pages = GetNumPages();
        if (num_pages > 0) {
            uint32_t last_page_id = num_pages - 1;
            Page last_page;
            ReadPage(last_page_id, &last_page);
            if (last_page.AddRecord(record_data)) {
                WritePage(last_page_id, &last_page);
                return true;
            }
        }

        logging::log.info("Need to allocate a new page for the record.");
        Page new_page;
        new_page.Initialize();
        if (!new_page.AddRecord(record_data)) {
            logging::log.error("Failed to add record to the new page. Record size may be too large: {} bytes.",
                               record_data.size());
            return false;
        }
        WritePage(num_pages, &new_page);
        return true;
    }

    void TableHeap::ReadPage(PageId page_id, Page* page) {
        if (!file_stream_.is_open()) {
            throw std::runtime_error("File stream is not open for reading.");
        }

        if (page_id >= GetNumPages()) {
            throw std::out_of_range("Page ID " + std::to_string(page_id) + " is out of range.");
        }

        // Calculate the offset for the page we want to read.
        size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;

        // Move the file pointer to the correct position.
        file_stream_.seekg(offset);
        if (file_stream_.fail()) {
            throw std::runtime_error("Failed to seek to page " + std::to_string(page_id));
        }

        // Read the page data into the provided Page object.
        file_stream_.read(page->GetData(), PAGE_SIZE);
        if (file_stream_.fail()) {
            throw std::runtime_error("Failed to read page " + std::to_string(page_id));
        }
        file_stream_.seekg(0);
    }

    void TableHeap::WritePage(PageId page_id, const Page* page) {
        if (!file_stream_.is_open()) {
            throw std::runtime_error("File stream is not open for writing.");
        }

        uint32_t num_pages = GetNumPages();
        if (page_id > num_pages) {
            throw std::out_of_range("Page ID " + std::to_string(page_id) + " is out of range.");
        } else if (page_id == num_pages) {
            // If the page_id is equal to the number of pages, we are appending a new page.
            logging::log.info("Appending new page with ID {}.", page_id);
        }

        // Calculate the offset for the page we want to write.
        size_t offset = static_cast<size_t>(page_id) * PAGE_SIZE;

        // Move the file pointer to the correct position.
        file_stream_.seekp(offset);
        if (file_stream_.fail()) {
            throw std::runtime_error("Failed to seek to page " + std::to_string(page_id));
        }

        // Write the page data from the provided Page object.
        file_stream_.write(page->GetData(), PAGE_SIZE);
        if (file_stream_.fail()) {
            throw std::runtime_error("Failed to write page " + std::to_string(page_id));
        }

        file_stream_.flush();
        file_stream_.seekp(0);
    }

    uint32_t TableHeap::GetNumPages() {
        file_stream_.seekg(0, std::ios::end);
        std::streamoff file_size = file_stream_.tellg();
        if (file_size <= 0) {
            // If the file is empty or an error occurred, return 0 pages.
            return 0;
        }
        return static_cast<uint32_t>(file_size / PAGE_SIZE);
    }

}  // namespace simpledb::storage
