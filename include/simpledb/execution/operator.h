//
// Created by Akshat Jain on 22/06/25.
//

#ifndef SIMPLE_DB_OPERATOR_H
#define SIMPLE_DB_OPERATOR_H

#include <optional>
#include "simpledb/execution/row.h"

namespace simpledb::execution {
    class Operator {
       public:
        virtual ~Operator() = default;

        // The pure virtual 'next' method. This is the core of the
        // Volcano model. It defines the contract that all concrete
        // operators MUST implement.
        virtual std::optional<row::Row> next() = 0;
    };
}  // namespace simpledb::execution

#endif  // SIMPLE_DB_OPERATOR_H
