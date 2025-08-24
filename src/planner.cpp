//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/planner.h"

#include "simpledb/execution/filter_operator.h"
#include "simpledb/execution/table_scan_operator.h"
#include "simpledb/execution/projection_operator.h"

namespace planner {
    std::unique_ptr<simpledb::execution::Operator> plan_select(const ast::SelectCommand& cmd,
                                                               const std::filesystem::path& data_dir) {
        // 1. Create the bottom-most operator: the TableScan.
        std::unique_ptr<simpledb::execution::Operator> op =
            std::make_unique<simpledb::execution::TableScanOperator>(cmd.table_name, data_dir);

        // 2. If there's a WHERE clause, wrap the TableScan with a FilterOperator.
        // TODO: Implement predicate push-down optimization
        // Currently WHERE clauses are always applied as separate FilterOperator after TableScan.
        // We could push simple predicates down to storage layer: TableScan could accept WHERE conditions and filter
        // during page scanning We would continue to use FilterOperator for complex predicates that cannot be pushed
        // down. For example:
        // - Joins: Cannot push down predicates involving multiple tables
        // - Complex expressions: Functions or calculations that storage layer cannot evaluate
        // - Subqueries: Cannot push down predicates involving subqueries
        // - Aggregations: Cannot push down predicates on aggregated results
        if (cmd.where_clause.has_value()) {
            op = std::make_unique<simpledb::execution::FilterOperator>(
                cmd.table_name, std::move(op), cmd.where_clause.value());
        }

        // 3. Create the ProjectionOperator, giving it the current operator as its child.
        op = std::make_unique<simpledb::execution::ProjectionOperator>(cmd.table_name, std::move(op), cmd.projection);

        // 4. Return the top-most operator in the pipeline.
        return op;
    }
}  // namespace planner
