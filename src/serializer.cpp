//
// Created by Akshat Jain on 21/06/25.
//

#include "simpledb/serializer.h"

#include <cstdint>
#include <cstring>

namespace serializer {
    std::vector<char> serialize(const std::vector<std::string>& data) {
        std::vector<char> record_data;
        for (const auto& value : data) {
            // First, we need to store the length of the value as a 16-bit unsigned integer
            uint16_t length = static_cast<uint16_t>(value.size());
            // Create a pointer to the length and reinterpret it as a char array
            const char* len_ptr = reinterpret_cast<const char*>(&length);
            // Insert the length as a 2-byte value
            record_data.insert(record_data.end(), len_ptr, len_ptr + sizeof(uint16_t));
            // Now insert the actual value
            record_data.insert(record_data.end(), value.begin(), value.end());
        }
        return record_data;
    }

    std::vector<std::string> deserialize(const std::vector<char>& data) {
        std::vector<std::string> values;
        size_t pos = 0;
        while (pos < data.size()) {
            // 1. Read the 2-byte length prefix.
            uint16_t len;
            memcpy(&len, &data[pos], sizeof(uint16_t));
            pos += sizeof(uint16_t);

            // 2. Read the string data of that length.
            std::string value(data.begin() + pos, data.begin() + pos + len);
            values.push_back(value);
            pos += len;
        }
        return values;
    }
}  // namespace serializer
