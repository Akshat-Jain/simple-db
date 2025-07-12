//
// Created by Akshat Jain on 20/04/25.
//

#ifndef SIMPLEDB_PARSER_H
#define SIMPLEDB_PARSER_H
#include <variant>
#include "simpledb/command.h"
#include "simpledb/ast/ast.h"

namespace parser {
    // A variant that can hold any of the possible parsed command types.
    using CommandVariant = std::variant<command::CreateTableCommand,
                                        command::DropTableCommand,
                                        command::InsertCommand,
                                        command::ShowTablesCommand,
                                        ast::SelectCommand>;

    std::optional<CommandVariant> parse_sql(const std::string& query);

}  // namespace parser

#endif  // SIMPLEDB_PARSER_H
