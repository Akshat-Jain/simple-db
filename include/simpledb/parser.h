//
// Created by Akshat Jain on 20/04/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <optional>

#include "command.h"

namespace parser {

    std::optional<command::CreateTableCommand> parse_create_table(const std::string& query);

    std::optional<command::InsertCommand> parse_insert(const std::string& query);
}

#endif //PARSER_H
