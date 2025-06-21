//
// Created by Akshat Jain on 21/06/25.
//

#ifndef SIMPLE_DB_SERIALIZER_H
#define SIMPLE_DB_SERIALIZER_H

#include <string>
#include <vector>

namespace serializer {
    std::vector<char> serialize(const std::vector<std::string>& data);
    std::vector<std::string> deserialize(const std::vector<char>& data);
}

#endif  // SIMPLE_DB_SERIALIZER_H
