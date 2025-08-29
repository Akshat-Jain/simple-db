//
// Created by Akshat Jain on 28/08/25.
//

#ifndef SIMPLEDB_QUERY_RUNNER_H
#define SIMPLEDB_QUERY_RUNNER_H

#include "simpledb/result.h"
#include "simpledb/parser.h"
#include "simpledb/executor.h"
#include "simpledb/planner.h"
#include "simpledb/config.h"

namespace query_runner {
    class QueryRunner {
       public:
        // Runs the given SQL query and returns the result.
        static results::ExecutionResult run_query(const std::string& query);
    };

}  // namespace query_runner

#endif  // SIMPLEDB_QUERY_RUNNER_H
