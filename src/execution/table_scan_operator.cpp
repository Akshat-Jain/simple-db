//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/execution/table_scan_operator.h"

#include "simpledb/config.h"
#include "simpledb/serializer.h"
#include <string>
#include <vector>

namespace simpledb::execution {
    TableScanOperator::TableScanOperator(const std::string& table_name, const std::filesystem::path& data_dir)
        : table_heap_(data_dir / (table_name + ".data")), iterator_(table_heap_.begin()) {}

    std::optional<row::Row> TableScanOperator::next() {
        std::optional<std::vector<char>> next = iterator_.next();
        if (next.has_value()) {
            // Deserialize the next record into a Row object.
            return serializer::deserialize(next.value());
        }
        return std::nullopt;
    }
}  // namespace simpledb::execution
