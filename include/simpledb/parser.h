//
// Created by Akshat Jain on 20/04/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <optional>

#include "command.h"

namespace parser {

    enum class CommandType { CREATE_TABLE, INSERT, SELECT, DROP_TABLE, SHOW_TABLES, UNKNOWN };

    CommandType get_command_type(const std::string& query);

    std::optional<command::CreateTableCommand> parse_create_table(const std::string& query);

    std::optional<command::DropTableCommand> parse_drop_table(const std::string& query);

    std::optional<command::ShowTablesCommand> parse_show_tables(const std::string& query);

    std::optional<command::InsertCommand> parse_insert(const std::string& query);
}  // namespace parser

#endif  // PARSER_H
