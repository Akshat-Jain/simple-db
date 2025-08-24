//
// Created by Akshat Jain on 21/06/25.
//

#ifndef SIMPLE_DB_AST_H
#define SIMPLE_DB_AST_H

#include <optional>
#include <string>
#include <vector>

namespace ast {
    enum ComparisonOp { EQUALS, NOT_EQUALS, LESS_THAN, LESS_THAN_OR_EQUAL, GREATER_THAN, GREATER_THAN_OR_EQUAL };

    struct WhereClause {
        // WHERE column_name op value
        std::string column_name;
        ComparisonOp op;
        std::string value;
    };

    /**
     * @brief Represents the root node of the Abstract Syntax Tree (AST) for a SELECT query.
     *
     * This struct encapsulates the entire logical structure of a parsed SELECT statement.
     * It is not a simple command, but rather the entry point to a tree that describes
     * the data to be retrieved.
     *
     * While this root node itself is not recursive, its members will eventually point to
     * complex, recursive sub-trees (e.g., the `where_clause` will point to a tree of
     * `Expression` nodes).
     *
     * This AST is the output of the `Parser` and the input to the `Planner`, which uses
     * it to construct the physical execution plan (the Volcano operator tree).
     */
    struct SelectCommand {
        // The name of the table specified in the FROM clause.
        std::string table_name;

        // The projection list, representing the columns after the SELECT keyword.
        // An empty vector signifies `SELECT *`.
        std::vector<std::string> projection;

        // Optional WHERE clause for filtering results.
        std::optional<WhereClause> where_clause;
    };
}  // namespace ast

#endif  // SIMPLE_DB_AST_H
