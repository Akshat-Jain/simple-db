//
// Created by Akshat Jain on 20/04/25.
//

#include "simpledb/parser.h"

#include <iostream>
#include <sstream>

#include "simpledb/utils/stringutils.h"

namespace parser {
    std::optional<command::CreateTableCommand> parse_create_table(const std::string &query) {
        std::cout << "Query is: " << query << std::endl;
        std::string trimmed_query = stringutils::trim(query);
        std::cout << "Query after trimming is: " << trimmed_query << std::endl;

        command::CreateTableCommand command;

        std::stringstream ss(trimmed_query);
        std::string token;

        // 1. The first word should be CREATE
        if (!(ss >> token) || stringutils::to_upper(token) != "CREATE") {
            std::cerr << "ERROR: Expected CREATE keyword." << std::endl;
            return std::nullopt;
        }

        // 2. The second word should be TABLE
        if (!(ss >> token) || stringutils::to_upper(token) != "TABLE") {
            std::cerr << "ERROR: Expected TABLE keyword." << std::endl;
            return std::nullopt;
        }

        // 3. Extract the table name
        size_t after_table_stream_pos = ss.tellg();
        std::cout << "Stream position after TABLE: " << after_table_stream_pos << std::endl;

        // Find the start of table name by finding first non-whitespace character after after_table_stream_pos
        size_t find_start_of_table_name = trimmed_query.find_first_not_of(" \t\n\r\f\v", after_table_stream_pos);
        if (find_start_of_table_name == std::string::npos) {
            std::cerr << "ERROR: No table name found." << std::endl;
            return std::nullopt;
        }

        // find first "(" after find_start_of_table_name
        size_t open_paren_pos = trimmed_query.find('(', find_start_of_table_name);
        if (open_paren_pos == std::string::npos) {
            // No opening parenthesis found anywhere after the potential start of the table name
            std::cerr << "ERROR: No opening parenthesis found." << std::endl;
            return std::nullopt;
        }

        // if found, set table_name to the string between find_start_of_table_name and open_paren_pos
        command.table_name = stringutils::trim(trimmed_query.substr(find_start_of_table_name, open_paren_pos - find_start_of_table_name));
        std::cout << "Table name is: " << command.table_name << std::endl;
        // Check if table_name is empty
        if (command.table_name.empty()) {
            std::cerr << "ERROR: Table name is empty." << std::endl;
            return std::nullopt;
        }

        // Check if table_name is a valid identifier (alphanumeric and underscores)
        if (!stringutils::is_alpha_num_underscore(command.table_name)) {
            std::cerr << "ERROR: Table name contains invalid characters." << std::endl;
            return std::nullopt;
        }

        // 4. Extract the columns and types
        size_t after_table_name_stream_pos = ss.tellg();
        std::cout << "Stream position after table name: " << after_table_name_stream_pos << std::endl;

        size_t close_paren_pos = trimmed_query.find(')', open_paren_pos + 1);
        if (close_paren_pos == std::string::npos) {
            std::cerr << "ERROR: Closing parenthesis ')' not found." << std::endl;
            return std::nullopt;
        }

        std::string column_definition_string = stringutils::trim(trimmed_query.substr(open_paren_pos + 1, close_paren_pos - open_paren_pos - 1));
        std::cout << "DEBUG: Column definition part: [" << column_definition_string << "]" << std::endl;

        if (!column_definition_string.empty()) {
            std::stringstream column_definitions_stream(column_definition_string);
            std::string column_definition;

            while (std::getline(column_definitions_stream, column_definition, ',')) {
                // Split by comma and trim each part
                column_definition = stringutils::trim(column_definition);
                std::cout << "Column definition: [" << column_definition << "]" << std::endl;

                std::stringstream column_definition_sstream(column_definition);
                std::string col_name_str;
                std::string col_type_str;
                std::string remaining;

                if (!(column_definition_sstream >> col_name_str)) {
                    std::cerr << "ERROR: Failed to parse column name in column definition: [" << column_definition << "]." << std::endl;
                    return std::nullopt;
                }
                if (!(column_definition_sstream >> col_type_str)) {
                    std::cerr << "ERROR: Failed to parse column type in column definition: [" << column_definition << "]." << std::endl;
                    return std::nullopt;
                }
                // Check if there are any extra tokens after the type
                if (column_definition_sstream >> remaining) {
                    std::cerr << "ERROR: Extra tokens after column type in column definition: [" << column_definition << "]." << std::endl;
                    return std::nullopt;
                }

                // Validate column name
                if (!stringutils::is_alpha_num_underscore(col_name_str)) {
                    std::cerr << "ERROR: Column name '" << col_name_str << "' contains invalid characters." << std::endl;
                    return std::nullopt;
                }
                // TODO: Check against reserved keywords?

                // Get column type enum from string
                command::Datatype col_data_type;
                if (stringutils::to_upper(col_type_str) == "INT") {
                    col_data_type = command::Datatype::INT;
                } else if (stringutils::to_upper(col_type_str) == "TEXT") {
                    col_data_type = command::Datatype::TEXT;
                } else {
                    std::cerr << "ERROR: Unknown column type '" << col_type_str << "'." << std::endl;
                    return std::nullopt;
                }
                command.column_definitions.push_back({col_name_str, col_data_type});
            }
        }

        return command;
    }

    std::optional<command::InsertCommand> parse_insert(const std::string &query) {
        std::cout << "Not yet implemented: parse_insert" << std::endl;
        return std::nullopt;
    }
}
