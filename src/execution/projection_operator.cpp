//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/execution/projection_operator.h"

#include "simpledb/catalog.h"
#include "simpledb/serializer.h"
#include <string>
#include <vector>

namespace simpledb::execution {
    ProjectionOperator::ProjectionOperator(std::string table_name,
                                           std::unique_ptr<simpledb::execution::Operator> child,
                                           const std::vector<std::string>& projection_columns)
        : table_name_(table_name), child_(std::move(child)), projection_columns_(projection_columns) {
        std::optional<catalog::TableSchema> table_schema_optional = catalog::get_table_schema(table_name_);
        if (!table_schema_optional) {
            // Or throw a more specific exception
            throw std::runtime_error("Table not found in catalog: " + table_name_);
        }

        const catalog::TableSchema& table_schema = table_schema_optional.value();
        for (const auto& projection_column : projection_columns_) {
            bool found = false;
            for (size_t j = 0; j < table_schema.column_definitions.size(); ++j) {
                if (table_schema.column_definitions[j].column_name == projection_column) {
                    projected_column_indices_.push_back(j);
                    found = true;
                    break;  // Found the column, no need to check further.
                }
            }
            if (!found) {
                throw std::runtime_error("Projection column not found in table schema: " + projection_column);
            }
        }
    }

    std::optional<row::Row> ProjectionOperator::next() {
        std::optional<row::Row> next = child_->next();
        if (!next.has_value()) {
            // If the child operator has no more rows, return std::nullopt.
            return std::nullopt;
        }
        if (projection_columns_.empty()) {
            // If no projection columns are specified, then we want to project all columns.
            // This means we return the entire row as is.
            return next;
        }

        row::Row projected_row;
        for (int index : projected_column_indices_) {
            projected_row.push_back(next.value()[index]);
        }
        return projected_row;
    }
}  // namespace simpledb::execution
