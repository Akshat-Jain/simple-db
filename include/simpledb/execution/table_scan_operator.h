//
// Created by Akshat Jain on 22/06/25.
//

#ifndef SIMPLE_DB_TABLE_SCAN_OPERATOR_H
#define SIMPLE_DB_TABLE_SCAN_OPERATOR_H

#include "simpledb/execution/operator.h"
#include "simpledb/execution/row.h"
#include "simpledb/storage/table_heap.h"

#include <optional>
#include <string>

namespace simpledb::execution {
    /**
     * @brief The TableScanOperator is responsible for scanning a table and returning rows one by one.
     * It implements the Operator interface, which defines the contract for all operators in the Volcano model.
     */
    class TableScanOperator : public Operator {
       public:
        // Constructor that initializes the TableScanOperator with a table name.
        explicit TableScanOperator(const std::string& table_name, const std::filesystem::path& data_dir);

        // The next method retrieves the next row from the table.
        std::optional<row::Row> next() override;

       private:
        /**
         * @brief The TableHeap object that manages the table's data file.
         *
         * This operator must own the TableHeap object to ensure its lifetime persists
         * as long as the scan is in progress. The iterator_ holds a pointer to this
         * heap, so the heap cannot be a temporary object that dies after the constructor.
         */
        storage::TableHeap table_heap_;

        /**
         * @brief The iterator used to perform the sequential scan over the heap file.
         *
         * This iterator maintains the current position (page and slot) of the scan.
         */
        storage::TableHeap::Iterator iterator_;
    };
}  // namespace simpledb::execution

#endif  // SIMPLE_DB_TABLE_SCAN_OPERATOR_H
