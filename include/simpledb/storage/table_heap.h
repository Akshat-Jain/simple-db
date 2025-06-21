//
// Created by Akshat Jain on 14/06/25.
//

#ifndef SIMPLE_DB_TABLE_HEAP_H
#define SIMPLE_DB_TABLE_HEAP_H

#include <cstdint>
#include <fstream>
#include <string>
#include <optional>
#include <vector>

#include "simpledb/execution/row.h"
#include "simpledb/storage/page.h"

namespace simpledb::storage {

    // A PageId is a unique identifier for a page in the storage system.
    // Type alias is mostly for clarity, and future maintainability.
    using PageId = uint32_t;

    /**
     * @brief Manages the collection of pages on disk that store a single table's data.
     *
     * The TableHeap class provides an abstraction over a single data file (e.g., "users.data")
     * that contains all the records for a table. The term "heap" signifies that the records
     * within the file are not stored in any particular logical order (e.g., they are not
     * sorted by a primary key). Records are simply "heaped" into pages as they are inserted.
     *
     * This class is a core component of the **Storage Layer**. It is used by the
     * **Execution Layer**, specifically by the `TableScanOperator`, to retrieve the physical
     * data for a table. It abstracts away the details of page management, file I/O, and
     * record placement from the higher-level query processing logic.
     *
     * Internally, the file is a sequence of fixed-size pages, and each page uses a
     * slotted page layout to manage variable-length records.
     *
     * Its primary responsibilities include:
     *  - Inserting new records into the table.
     *  - Providing an iterator to scan all records in the table sequentially.
     */
    class TableHeap {
       public:
        /**
         * @brief An iterator for performing a sequential scan over all records in a TableHeap.
         *
         * This iterator moves from the first record of the first page to the last record
         * of the last page. It is the foundational tool for full table scans.
         */
        class Iterator {
           public:
            explicit Iterator(TableHeap* parent_heap, PageId page_id, uint16_t slot_num);

            std::optional<std::vector<char>> next();

           private:
            // A pointer to the parent TableHeap, used to call its private methods like ReadPage.
            TableHeap* parent_heap_;

            // The ID of the page the iterator is currently scanning.
            PageId current_page_id_;

            // The number of the slot on the current page that the iterator will read next.
            uint16_t current_slot_num_;
        };

        /**
         * Constructor that opens the table's data file.
         * @param table_data_path The path to the file that stores the table's data.
         */
        explicit TableHeap(const std::string& table_data_path);

        /**
         * Inserts a new record into the table.
         * @param record_data The binary data of the record to insert.
         * @return True if the insertion was successful, false otherwise.
         */
        bool InsertRecord(const std::vector<char>& record_data);

        /**
         * @brief Returns an iterator pointing to the first record in the table.
         *
         * This method creates an iterator initialized to the very beginning of the heap
         * (page 0, slot 0). This design, where the starting point is passed to the
         * iterator's constructor, allows for future flexibility, such as creating
         * iterators that start at arbitrary pages for parallel scans.
         *
         * @return An iterator initialized to the start of the heap.
         */
        TableHeap::Iterator begin() { return Iterator(this, 0, 0); }

       private:
        /**
         * Reads a specific page from the data file into the provided Page object.
         * @param page_id The ID of the page to read.
         * @param page A pointer to a Page object to be populated with data.
         */
        void ReadPage(PageId page_id, Page* page);

        /**
         * Writes the data from a Page object to a specific page in the data file.
         * @param page_id The ID of the page to write to.
         * @param page A pointer to the Page object containing the data to write.
         */
        void WritePage(PageId page_id, const Page* page);

        /**
         * Calculates the number of pages currently in the table file.
         * @return The total number of pages.
         */
        uint32_t GetNumPages();

        // The file stream used to read/write the table's data.
        std::fstream file_stream_;

        // The path to the file that stores the table's data.
        std::string file_path_;
    };
}  // namespace simpledb::storage

#endif  // SIMPLE_DB_TABLE_HEAP_H
