//
// Created by Akshat Jain on 22/06/25.
//

#ifndef SIMPLE_DB_FILTER_OPERATOR_H
#define SIMPLE_DB_FILTER_OPERATOR_H

#include <optional>
#include "simpledb/execution/operator.h"
#include "simpledb/execution/row.h"
#include "simpledb/storage/table_heap.h"
#include "simpledb/ast/ast.h"

#include <memory>
#include <string>
#include <vector>

namespace simpledb::execution {
    class FilterOperator : public Operator {
       public:
        explicit FilterOperator(std::string table_name,
                                std::unique_ptr<simpledb::execution::Operator> child,
                                ast::WhereClause where_clause);

        // The next method retrieves the next row from the child operator
        // and evaluates the WHERE clause on it. If the row satisfies the
        // condition, it is returned; otherwise, the next row is fetched.
        std::optional<row::Row> next() override;

       private:
        // The name of the table from which we are filtering columns.
        // This helps in looking up the table schema in the catalog.
        std::string table_name_;

        // The child operator from which we will get rows.
        std::unique_ptr<simpledb::execution::Operator> child_;

        // The WHERE clause to evaluate on each row.
        ast::WhereClause where_clause_;

        // The index of the column in the table schema that corresponds to the WHERE clause column.
        int where_column_index_;
    };
}  // namespace simpledb::execution

#endif  // SIMPLE_DB_FILTER_OPERATOR_H
