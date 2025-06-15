//
// Created by Akshat Jain on 14/06/25.
//

#ifndef SIMPLE_DB_TABLE_HEAP_H
#define SIMPLE_DB_TABLE_HEAP_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "simpledb/storage/page.h"

namespace simpledb::storage {

    // A PageId is a unique identifier for a page in the storage system.
    // Type alias is mostly for clarity, and future maintainability.
    using PageId = uint32_t;

    class TableHeap {
       public:
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
