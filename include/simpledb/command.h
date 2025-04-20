//
// Created by Akshat Jain on 20/04/25.
//

#ifndef COMMAND_H
#define COMMAND_H
#include <string>
#include <vector>

namespace command {

    enum class Datatype {
        INT,
        TEXT,
        UNKNOWN
    };

    struct ColumnDefinition {
        std::string column_name;
        Datatype type = Datatype::UNKNOWN;
    };

    struct CreateTableCommand {
        std::string table_name;
        std::vector<ColumnDefinition> column_definitions;
    };

    struct InsertCommand {
        std::string table_name;
        std::vector<std::string> values; // Assumption: The values match column order/type
    };
}

#endif //COMMAND_H
