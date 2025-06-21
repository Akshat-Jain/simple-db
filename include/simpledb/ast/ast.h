//
// Created by Akshat Jain on 21/06/25.
//

#ifndef SIMPLE_DB_AST_H
#define SIMPLE_DB_AST_H

#include <string>
#include <vector>

namespace ast {
    struct SelectCommand {
        std::string table_name;            // The name of the table to select from.
        std::vector<std::string> columns;  // The columns to select, empty means all columns.
    };
}  // namespace ast

#endif  // SIMPLE_DB_AST_H
