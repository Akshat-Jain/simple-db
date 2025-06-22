//
// Created by Akshat Jain on 22/06/25.
//

#ifndef SIMPLE_DB_PLANNER_H
#define SIMPLE_DB_PLANNER_H

#include "simpledb/ast/ast.h"
#include "simpledb/execution/operator.h"
#include <memory>

namespace planner {
    /**
     * @brief Takes a SelectCommand AST and builds a physical execution plan.
     * @param cmd The abstract syntax tree for the SELECT query.
     * @return A unique_ptr to the root operator of the execution pipeline.
     */
    std::unique_ptr<simpledb::execution::Operator> plan_select(const ast::SelectCommand& cmd);
}  // namespace planner

#endif  // SIMPLE_DB_PLANNER_H
