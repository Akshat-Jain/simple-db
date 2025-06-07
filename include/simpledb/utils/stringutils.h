//
// Created by Akshat Jain on 20/04/25.
//

#ifndef SIMPLEDB_STRINGUTILS_H
#define SIMPLEDB_STRINGUTILS_H

#include <string>

namespace stringutils {  // Group utility functions

    /**
     * @brief Removes leading and trailing whitespace from a string.
     * @param str The string to trim.
     * @return A new string with leading/trailing whitespace removed.
     */
    std::string trim(const std::string& str);

    /**
     * @brief Converts a string to uppercase.
     * @param s The string to convert.
     * @return A new string containing the uppercase version of the input.
     */
    std::string to_upper(std::string s);

    /**
     * @brief Checks if a string contains only alphanumeric characters and underscores.
     * @param s The string to check.
     * @return True if the string contains only alphanumeric characters and underscores, false otherwise.
     */
    bool is_alpha_num_underscore(std::string s);

}  // namespace stringutils

#endif  // SIMPLEDB_STRINGUTILS_H
