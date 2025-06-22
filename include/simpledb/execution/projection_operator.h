//
// Created by Akshat Jain on 22/06/25.
//

#ifndef SIMPLE_DB_PROJECTION_OPERATOR_H
#define SIMPLE_DB_PROJECTION_OPERATOR_H

#include <optional>
#include "simpledb/execution/operator.h"
#include "simpledb/execution/row.h"
#include "simpledb/storage/table_heap.h"

#include <memory>
#include <string>
#include <vector>

namespace simpledb::execution {
    class ProjectionOperator : public Operator {
       public:
        explicit ProjectionOperator(std::string table_name,
                                    std::unique_ptr<simpledb::execution::Operator> child,
                                    const std::vector<std::string> &projection_columns);

        // The next method retrieves the next row from the child operator
        // and projects it based on the specified columns.
        std::optional<row::Row> next() override;

       private:
        // The name of the table from which we are projecting columns.
        // This helps in looking up the table schema in the catalog.
        std::string table_name_;

        // The child operator from which we will get rows.
        std::unique_ptr<simpledb::execution::Operator> child_;

        // The list of columns to project.
        std::vector<std::string> projection_columns_;

        // The indices of the columns in the table schema that correspond to the projection columns.
        std::vector<int> projected_column_indices_;
    };
}  // namespace simpledb::execution

#endif  // SIMPLE_DB_PROJECTION_OPERATOR_H
