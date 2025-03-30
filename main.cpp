//
// Created by Akshat Jain on 29/03/25.
//

#include <iostream>
#include <sstream>


std::string handle_create_table(const std::string& query) {
    std::cout << "DEBUG: Placeholder handle_create_table called for: " << query << std::endl;
    // TODO: Parse table name, columns, types
    return "OK (Placeholder - Table creation)";
}

std::string handle_insert(const std::string& query) {
    std::cout << "DEBUG: Placeholder handle_insert called for: " << query << std::endl;
    // TODO: Parse table name, values
    return "OK (Placeholder - Insertion)";
}

std::string handle_select(const std::string& query) {
    std::cout << "DEBUG: Placeholder handle_select called for: " << query << std::endl;
    // TODO: Parse table name, potentially WHERE clause
    return "OK (Placeholder - Selection results would go here)";
}

std::string parse_and_execute(const std::string& query) {
    std::string command;
    // Use stringstream to easily extract the first word (the command)
    std::stringstream ss(query);
    ss >> command;

    // Convert command to upper case for case-insensitive comparison
    for (char &c : command) {
        c = std::toupper(c);
    }

    // Very basic command routing
    // Note: This assumes perfect spacing and syntax for now!
    if (command == "CREATE") {
        // Check for "TABLE" next (a slightly better check)
        std::string maybe_table;
        ss >> maybe_table;
         for (char &c : maybe_table) c = std::toupper(c);
         if (maybe_table == "TABLE") {
             return handle_create_table(query);
         }
    } else if (command == "INSERT") {
        // Check for "INTO" next
        std::string maybe_into;
        ss >> maybe_into;
        for (char &c : maybe_into) c = std::toupper(c);
        if (maybe_into == "INTO") {
            return handle_insert(query);
        }
    } else if (command == "SELECT") {
         return handle_select(query); // SELECT * FROM ... or SELECT * FROM ... WHERE ...
    }
    // Add more commands like UPDATE, DELETE later if needed

    return "ERROR: Unknown or unsupported command.";
}


int main() {
    std::string input_line;

    std::cout << "Welcome to simple-db!" << std::endl;

    while (true) {
        std::cout << "mydb> "; // Print prompt
        if (!std::getline(std::cin, input_line)) {
            // Handle potential end-of-file (e.g., Ctrl+D) or input error
            std::cout << "\nExiting." << std::endl;
            break;
        }

        // Simple exit command check
        if (input_line == "exit" || input_line == "quit") {
            std::cout << "Exiting." << std::endl;
            break;
        }

        // Ignore empty lines
        if (input_line.empty()) {
            continue;
        }

        // Parse and execute the command
        std::string result = parse_and_execute(input_line);
        std::cout << result << std::endl;
    }

    return 0;
}
