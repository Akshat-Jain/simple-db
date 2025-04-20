//
// Created by Akshat Jain on 20/04/25.
//

#include "simpledb/utils/stringutils.h"

#include <algorithm>
#include <cctype>

namespace stringutils {
    std::string trim(const std::string &str) {
        // Find the first non-whitespace character
        size_t first = str.find_first_not_of(" \t\n\r\f\v");
        // If the string is all whitespace, return an empty string or the original
        if (std::string::npos == first) {
            return ""; // Return empty is often cleaner than returning all whitespace
            // return str;
        }
        // Find the last non-whitespace character
        size_t last = str.find_last_not_of(" \t\n\r\f\v");
        // Extract the substring
        return str.substr(first, (last - first + 1));
    }

    std::string to_upper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),[](unsigned char c){ return std::toupper(c); });
        return s;
    }

    bool is_alpha_num_underscore(std::string s) {
        return std::all_of(s.begin(), s.end(), [](char c){ return std::isalnum(c) || c == '_'; });
    }
}
