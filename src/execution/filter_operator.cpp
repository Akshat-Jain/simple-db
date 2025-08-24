//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/execution/filter_operator.h"

#include "simpledb/catalog.h"
#include "simpledb/serializer.h"
#include <string>
#include <vector>

namespace simpledb::execution {
    // TODO: Implement proper type-aware comparison logic
    // Currently all comparisons are done as strings, which works for basic cases but has limitations:
    // Numeric comparisons use lexicographic ordering ("10" < "2" returns true)
    // To fix this, we should ideally preserve the type information during parsing, instead of blindly converting
    // everything to string.
    bool evaluate(ast::ComparisonOp op, const std::string& lhs, const std::string& rhs) {
        switch (op) {
            case ast::ComparisonOp::EQUALS:
                return lhs == rhs;
            case ast::ComparisonOp::NOT_EQUALS:
                return lhs != rhs;
            case ast::ComparisonOp::LESS_THAN:
                return lhs < rhs;
            case ast::ComparisonOp::LESS_THAN_OR_EQUAL:
                return lhs <= rhs;
            case ast::ComparisonOp::GREATER_THAN:
                return lhs > rhs;
            case ast::ComparisonOp::GREATER_THAN_OR_EQUAL:
                return lhs >= rhs;
            default:
                throw std::runtime_error("Unsupported comparison operator.");
        }
    }

    FilterOperator::FilterOperator(std::string table_name,
                                   std::unique_ptr<simpledb::execution::Operator> child,
                                   ast::WhereClause where_clause)
        : table_name_(table_name), child_(std::move(child)), where_clause_(where_clause) {
        std::optional<catalog::TableSchema> table_schema_optional = catalog::get_table_schema(table_name_);
        if (!table_schema_optional) {
            throw std::runtime_error("Table not found in catalog: " + table_name_);
        }

        const catalog::TableSchema& table_schema = table_schema_optional.value();
        for (size_t i = 0; i < table_schema.column_definitions.size(); ++i) {
            if (table_schema.column_definitions[i].column_name == where_clause_.column_name) {
                where_column_index_ = i;
                if (table_schema.column_definitions[i].type == command::Datatype::INT) {
                    // For INT columns, ensure the WHERE clause value is a valid integer.
                    try {
                        std::stoi(where_clause_.value);
                    } catch (const std::exception& e) {
                        throw std::runtime_error(
                            "WHERE clause value is not a valid integer for column: " + where_clause_.column_name +
                            ". Expected INT, got '" + where_clause_.value + "'");
                    }
                }
                return;
            }
        }

        throw std::runtime_error("WHERE clause column \"" + where_clause_.column_name + "\" not found in table " +
                                 table_name_);
    }

    std::optional<row::Row> FilterOperator::next() {
        while (true) {
            std::optional<row::Row> next = child_->next();
            if (!next.has_value()) {
                // If the child operator has no more rows, return std::nullopt.
                return std::nullopt;
            }

            if (!evaluate(where_clause_.op, next.value()[where_column_index_], where_clause_.value)) {
                continue;  // Row does not satisfy the WHERE clause, fetch the next row.
            }

            return next;
        }
    }
}  // namespace simpledb::execution
