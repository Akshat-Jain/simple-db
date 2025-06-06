//
// Created by Akshat Jain on 20/04/25.
//

#ifndef COMMAND_H
#define COMMAND_H
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

namespace command {
    enum class Datatype { INT, TEXT, UNKNOWN };
    NLOHMANN_JSON_SERIALIZE_ENUM(Datatype,
                                 {{Datatype::INT, "INT"}, {Datatype::TEXT, "TEXT"}, {Datatype::UNKNOWN, "UNKNOWN"}})

    struct ColumnDefinition {
        std::string column_name;
        Datatype type = Datatype::UNKNOWN;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ColumnDefinition, column_name, type)

    struct CreateTableCommand {
        std::string table_name;
        std::vector<ColumnDefinition> column_definitions;
    };

    struct DropTableCommand {
        std::string table_name;
    };

    struct InsertCommand {
        std::string table_name;
        std::vector<std::string> values;  // Assumption: The values match column order/type
    };

    struct ShowTablesCommand {};
}  // namespace command

#endif  // COMMAND_H
