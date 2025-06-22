//
// Created by Akshat Jain on 22/06/25.
//

#include "simpledb/planner.h"

#include "simpledb/execution/table_scan_operator.h"
#include "simpledb/execution/projection_operator.h"

namespace planner {
    std::unique_ptr<simpledb::execution::Operator> plan_select(const ast::SelectCommand& cmd,
                                                               const std::filesystem::path& data_dir) {
        // 1. Create the bottom-most operator: the TableScan.
        auto scan_op = std::make_unique<simpledb::execution::TableScanOperator>(cmd.table_name, data_dir);

        // 2. Create the ProjectionOperator, giving it the TableScanOperator as its child.
        // We transfer ownership of the scan_operator using std::move.
        auto proj_op = std::make_unique<simpledb::execution::ProjectionOperator>(
            cmd.table_name, std::move(scan_op), cmd.projection);

        // 3. Return the top-most operator in the pipeline.
        return proj_op;
    }
}  // namespace planner
