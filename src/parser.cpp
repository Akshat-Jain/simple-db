//
// Created by Akshat Jain on 20/04/25.
//

#include "simpledb/parser.h"

#include <iostream>
#include <set>
#include <sstream>

#include "simpledb/utils/logging.h"
#include "simpledb/utils/stringutils.h"

namespace {  // Anonymous namespace to keep helper functions private to this translation unit
    parser::ParseResult<std::vector<std::string>> parse_parenthesized_list(const std::string &input, size_t &pos) {
        while (pos < input.length() && std::isspace(input[pos])) {
            pos++;
        }

        if (pos >= input.length() || input[pos] != '(') {
            return {std::nullopt, "ERROR: Expected '(' to start list."};
        }
        pos++;  // Consume '('

        std::vector<std::string> items;
        std::string current_item;
        bool in_string_literal = false;

        while (pos < input.length()) {
            char current_char = input[pos];

            if (in_string_literal) {
                if (current_char == '\'') {
                    // Check for escaped single quote ('')
                    if (pos + 1 < input.length() && input[pos + 1] == '\'') {
                        // This means we have '' in the string literal, which should become a single quote.
                        current_item += '\'';
                        pos++;
                    } else {
                        in_string_literal = false;
                    }
                } else {
                    current_item += current_char;
                }
            } else {  // Not in a string literal
                switch (current_char) {
                    case '\'':
                        in_string_literal = true;
                        break;

                    case ',':
                        items.push_back(stringutils::trim(current_item));
                        current_item.clear();
                        break;

                    case ')':
                        // Push the last item before finishing.
                        if (!current_item.empty() || items.empty()) {
                            items.push_back(stringutils::trim(current_item));
                        }
                        pos++;                         // Consume ')'
                        return {items, std::nullopt};  // Success

                    default:
                        // Ignore whitespace between items
                        if (!std::isspace(current_char)) {
                            current_item += current_char;
                        }
                        break;
                }
            }
            pos++;
        }

        return {std::nullopt, "ERROR: Unmatched opening parenthesis in list."};
    }
}  // anonymous namespace

namespace parser {

    CommandType get_command_type(const std::string &query) {
        std::string command;

        // Create an std::stringstream object named 'ss'.
        // Initialize it with the content of the input 'query' string.
        // This allows treating the 'query' string like an input stream (e.g., std::cin).
        std::stringstream ss(query);

        // Use the stream extraction operator (>>) to read from the stringstream 'ss'.
        // By default, >> skips leading whitespace and reads characters until the *next* whitespace.
        // The extracted word ("CREATE", "INSERT", "SELECT", etc.) is stored in 'command'.
        // The stream's internal position moves past the extracted word and the subsequent whitespace.
        ss >> command;

        // Convert command to upper case for case-insensitive comparison
        for (char &c : command) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }

        if (command == "CREATE") {
            std::string maybe_table;
            ss >> maybe_table;
            for (char &c : maybe_table) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            if (maybe_table == "TABLE") {
                return CommandType::CREATE_TABLE;
            }
        } else if (command == "DROP") {
            std::string maybe_table;
            ss >> maybe_table;
            for (char &c : maybe_table) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            if (maybe_table == "TABLE") {
                return CommandType::DROP_TABLE;
            }
        } else if (command == "SHOW") {
            std::string maybe_tables;
            ss >> maybe_tables;
            for (char &c : maybe_tables) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            if (maybe_tables == "TABLES") {
                return CommandType::SHOW_TABLES;
            }
        } else if (command == "INSERT") {
            // Check for "INTO" next
            std::string maybe_into;
            ss >> maybe_into;
            for (char &c : maybe_into) {
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            }
            if (maybe_into == "INTO") {
                return CommandType::INSERT;
            }
        } else if (command == "SELECT") {
            return CommandType::SELECT;
        }

        return CommandType::UNKNOWN;
    }

    ParseResult<command::CreateTableCommand> parse_create_table(const std::string &query) {
        logging::log.debug("Query is: {}", query);
        std::string trimmed_query = stringutils::trim(query);
        logging::log.debug("Query after trimming is: {}", trimmed_query);

        command::CreateTableCommand command;

        std::stringstream ss(trimmed_query);
        std::string token;

        // 1. The first word should be CREATE
        if (!(ss >> token) || stringutils::to_upper(token) != "CREATE") {
            return {std::nullopt, "ERROR: Expected CREATE keyword."};
        }

        // 2. The second word should be TABLE
        if (!(ss >> token) || stringutils::to_upper(token) != "TABLE") {
            return {std::nullopt, "ERROR: Expected TABLE keyword."};
        }

        // 3. Extract the table name
        size_t after_table_stream_pos = ss.tellg();

        // Find the start of table name by finding first non-whitespace character after after_table_stream_pos
        size_t find_start_of_table_name = trimmed_query.find_first_not_of(" \t\n\r\f\v", after_table_stream_pos);
        if (find_start_of_table_name == std::string::npos) {
            return {std::nullopt, "ERROR: No table name found."};
        }

        // find first "(" after find_start_of_table_name
        size_t open_paren_pos = trimmed_query.find('(', find_start_of_table_name);
        if (open_paren_pos == std::string::npos) {
            // No opening parenthesis found anywhere after the potential start of the table name
            return {std::nullopt, "ERROR: No opening parenthesis found."};
        }

        // if found, set table_name to the string between find_start_of_table_name and open_paren_pos
        command.table_name = stringutils::trim(
            trimmed_query.substr(find_start_of_table_name, open_paren_pos - find_start_of_table_name));
        // Check if table_name is empty
        if (command.table_name.empty()) {
            return {std::nullopt, "ERROR: Table name is empty."};
        }

        // Check if table_name is a valid identifier (alphanumeric and underscores)
        if (!stringutils::is_alpha_num_underscore(command.table_name)) {
            return {std::nullopt, "ERROR: Table name '" + command.table_name + "' contains invalid characters."};
        }

        // 4. Extract the columns and types
        size_t close_paren_pos = trimmed_query.find(')', open_paren_pos + 1);
        if (close_paren_pos == std::string::npos) {
            return {std::nullopt, "ERROR: Closing parenthesis ')' not found."};
        }

        std::string column_definition_string =
            stringutils::trim(trimmed_query.substr(open_paren_pos + 1, close_paren_pos - open_paren_pos - 1));
        logging::log.debug("Column definition part: [{}]", column_definition_string);

        if (!column_definition_string.empty()) {
            std::stringstream column_definitions_stream(column_definition_string);
            std::string column_definition;

            while (std::getline(column_definitions_stream, column_definition, ',')) {
                // Split by comma and trim each part
                column_definition = stringutils::trim(column_definition);
                logging::log.debug("Column definition: [{}]", column_definition);

                std::stringstream column_definition_sstream(column_definition);
                std::string col_name_str;
                std::string col_type_str;
                std::string remaining;

                if (!(column_definition_sstream >> col_name_str)) {
                    return {std::nullopt,
                            "ERROR: Failed to parse column name in column definition: [" + column_definition + "]."};
                }
                if (!(column_definition_sstream >> col_type_str)) {
                    return {std::nullopt,
                            "ERROR: Failed to parse column type in column definition: [" + column_definition + "]."};
                }
                // Check if there are any extra tokens after the type
                if (column_definition_sstream >> remaining) {
                    return {std::nullopt,
                            "ERROR: Extra tokens after column type in column definition: [" + column_definition + "]."};
                }

                // Validate column name
                if (!stringutils::is_alpha_num_underscore(col_name_str)) {
                    return {std::nullopt, "ERROR: Column name '" + col_name_str + "' contains invalid characters."};
                }
                // TODO: Check against reserved keywords?

                // Get column type enum from string
                command::Datatype col_data_type;
                if (stringutils::to_upper(col_type_str) == "INT") {
                    col_data_type = command::Datatype::INT;
                } else if (stringutils::to_upper(col_type_str) == "TEXT") {
                    col_data_type = command::Datatype::TEXT;
                } else {
                    return {std::nullopt,
                            "ERROR: Unknown column type '" + col_type_str + "'. Supported types are INT and TEXT."};
                }
                command.column_definitions.push_back({col_name_str, col_data_type});
            }
        }

        return {command, std::nullopt};
    }

    ParseResult<command::DropTableCommand> parse_drop_table(const std::string &query) {
        logging::log.debug("Query is: {}", query);
        std::string trimmed_query = stringutils::trim(query);
        logging::log.debug("Query after trimming is: {}", trimmed_query);

        command::DropTableCommand command;

        std::stringstream ss(trimmed_query);
        std::string token;

        // 1. The first word should be DROP
        if (!(ss >> token) || stringutils::to_upper(token) != "DROP") {
            return {std::nullopt, "ERROR: Expected DROP keyword."};
        }

        // 2. The second word should be TABLE
        if (!(ss >> token) || stringutils::to_upper(token) != "TABLE") {
            return {std::nullopt, "ERROR: Expected TABLE keyword."};
        }

        // 3. The third word should be the table name.
        ss >> command.table_name;

        // 4. Table name should be valid.
        if (command.table_name.empty()) {
            return {std::nullopt, "ERROR: Table name is empty."};
        }

        // Check if table_name is a valid identifier (alphanumeric and underscores)
        if (!stringutils::is_alpha_num_underscore(command.table_name)) {
            return {std::nullopt, "ERROR: Table name '" + command.table_name + "' contains invalid characters."};
        }

        // 5. The stream shouldn't have any more tokens after the table name.
        if (ss >> token) {
            return {std::nullopt,
                    "ERROR: Invalid DROP TABLE command. Parsed table name as '" + command.table_name +
                        "', but found extra tokens after it: '" + token + "'."};
        }

        logging::log.debug("Parsed DROP TABLE command for table: {}", command.table_name);
        return {command, std::nullopt};
    }

    ParseResult<command::ShowTablesCommand> parse_show_tables(const std::string &query) {
        logging::log.debug("Query is: {}", query);
        std::string trimmed_query = stringutils::trim(query);
        logging::log.debug("Query after trimming is: {}", trimmed_query);

        command::ShowTablesCommand command;

        std::stringstream ss(trimmed_query);
        std::string token;

        // 1. The first word should be SHOW
        if (!(ss >> token) || stringutils::to_upper(token) != "SHOW") {
            return {std::nullopt, "ERROR: Expected SHOW keyword."};
        }

        // 2. The second word should be TABLES
        if (!(ss >> token) || stringutils::to_upper(token) != "TABLES") {
            return {std::nullopt, "ERROR: Expected TABLES keyword."};
        }

        // 3. The stream shouldn't have any more tokens after the table name.
        if (ss >> token) {
            return {std::nullopt, "ERROR: Invalid SHOW TABLES command. Found extra tokens: '" + token + "'."};
        }

        logging::log.debug("Parsed SHOW TABLES command successfully.");
        return {command, std::nullopt};
    }

    ParseResult<command::InsertCommand> parse_insert([[maybe_unused]] const std::string &query) {
        // INSERT INTO table_name (col1, col2, ...) VALUES (val1, val2, ...);
        // INSERT INTO table_name VALUES (val1, val2, ...);
        // Both of the above forms are valid.

        logging::log.debug("Query is: {}", query);
        std::string trimmed_query = stringutils::trim(query);
        logging::log.debug("Query after trimming is: {}", trimmed_query);

        command::InsertCommand command;

        std::stringstream ss(trimmed_query);
        std::string token;
        // 1. The first word should be INSERT
        if (!(ss >> token) || stringutils::to_upper(token) != "INSERT") {
            return {std::nullopt, "ERROR: Expected INSERT keyword."};
        }
        // 2. The second word should be INTO
        if (!(ss >> token) || stringutils::to_upper(token) != "INTO") {
            return {std::nullopt, "ERROR: Expected INTO keyword."};
        }
        // 3. The third word should be the table name
        if (!(ss >> command.table_name) || command.table_name.empty()) {
            return {std::nullopt, "ERROR: Expected table name after INTO keyword."};
        }
        // Check if table_name is a valid identifier (alphanumeric and underscores)
        if (!stringutils::is_alpha_num_underscore(command.table_name)) {
            return {std::nullopt, "ERROR: Table name '" + command.table_name + "' contains invalid characters."};
        }

        // 4. After the table name, we need to find either a '(' for columns or "VALUES".
        // Let's get the remainder of the string to work with.
        std::string remainder;
        std::getline(ss, remainder);  // Read the rest of the line into remainder

        size_t pos = 0;
        // Skip any leading whitespace in the remainder.
        while (pos < remainder.length() && std::isspace(remainder[pos])) {
            pos++;
        }

        if (pos >= remainder.length()) {
            return {std::nullopt, "ERROR: Incomplete INSERT statement. Expected columns list or 'VALUES'."};
        }

        // 5. Check for the optional column list
        if (remainder[pos] == '(') {
            auto columns_result = parse_parenthesized_list(remainder, pos);
            if (!columns_result) {
                return {std::nullopt, *columns_result.error_message};
            }
            command.columns = *columns_result.command;

            // Check if any column has been supplied more than once
            std::set<std::string> unique_columns(command.columns.begin(), command.columns.end());
            if (unique_columns.size() != command.columns.size()) {
                return {std::nullopt, "ERROR: Duplicate column names found in the column list."};
            }

            // After parsing columns, we need to skip more whitespace.
            while (pos < remainder.length() && std::isspace(remainder[pos])) {
                pos++;
            }
        }

        // 6. The next token MUST be "VALUES". We can check this by finding the keyword at our current position.
        std::string values_keyword = "VALUES";
        if (pos + values_keyword.length() > remainder.length() ||
            stringutils::to_upper(remainder.substr(pos, values_keyword.length())) != values_keyword) {
            return {std::nullopt, "ERROR: Expected VALUES keyword."};
        }
        pos += values_keyword.length();  // Advance past "VALUES"

        // 7. The next part MUST be the values list.
        auto values_result = parse_parenthesized_list(remainder, pos);
        if (!values_result) {
            return {std::nullopt, *values_result.error_message};
        }
        command.values = *values_result.command;

        // 8. Check for any trailing tokens.
        while (pos < remainder.length() && std::isspace(remainder[pos])) {
            pos++;
        }
        if (pos < remainder.length()) {
            return {std::nullopt, "ERROR: Unexpected tokens after values list: '" + remainder.substr(pos) + "'."};
        }

        // 9. Final validation
        if (!command.columns.empty() && command.columns.size() != command.values.size()) {
            return {std::nullopt, "ERROR: Column count does not match value count."};
        }

        // 10. Phew! If we reach here, we have a valid INSERT command.
        return {command, std::nullopt};
    }
}  // namespace parser
