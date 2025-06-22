//
// Created by Akshat Jain on 20/04/25.
//

#ifndef SIMPLEDB_PARSER_H
#define SIMPLEDB_PARSER_H
#include <optional>

#include "simpledb/ast/ast.h"
#include "command.h"

namespace parser {

    template <typename T>
    struct ParseResult {
        std::optional<T> command;
        std::optional<std::string> error_message;

        // This is a helper method to make checking for success easy by doing if(result) instead of
        // if(result.command.has_value()). The 'explicit' keyword prevents accidental implicit conversions to bool.
        explicit operator bool() const { return command.has_value(); }
    };

    enum class CommandType { CREATE_TABLE, INSERT, SELECT, DROP_TABLE, SHOW_TABLES, UNKNOWN };

    CommandType get_command_type(const std::string& query);

    ParseResult<command::CreateTableCommand> parse_create_table(const std::string& query);

    ParseResult<command::DropTableCommand> parse_drop_table(const std::string& query);

    ParseResult<command::ShowTablesCommand> parse_show_tables(const std::string& query);

    ParseResult<command::InsertCommand> parse_insert(const std::string& query);

    ParseResult<ast::SelectCommand> parse_select(const std::string& query);
}  // namespace parser

#endif  // SIMPLEDB_PARSER_H
