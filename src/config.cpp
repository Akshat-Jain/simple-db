//
// Created by Akshat Jain on 19/04/25.
//


#include "simpledb/config.h"

#include <iostream>
#include <filesystem>
#include <cstring>

namespace config {
    static Config config;
    static bool initialized = false;

    const static char* ENV_DATA_DIR = "SIMPLE_DB_DATA_DIR";
    const static char* DEFAULT_DATA_DIR = "data";

    void init_config() {
        if (initialized) {
            return;
        }

        const char* env_data_dir = std::getenv(ENV_DATA_DIR);
        if (env_data_dir != nullptr && std::strlen(env_data_dir) > 0) {
            config.data_dir = env_data_dir;
        } else {
            config.data_dir = DEFAULT_DATA_DIR;
        }
        // create the directory if it doesn't exist
        if (!std::filesystem::exists(config.data_dir)) {
            std::filesystem::create_directory(config.data_dir);
            std::cout << "Created data directory: " << config.data_dir << std::endl;
        }

        std::cout << "Using config:\n" << config << std::endl;

        initialized = true;
    }

    const Config& get_config() {
        if (initialized) {
            init_config();
        }
        return config;
    }

    /*
     * This is an example of operator overloading.
     * This function overloads the stream insertion operator (`<<`).
     * It dictates that when the compiler sees an expression like `std::cout << myConfigObject`,
     * where the left operand is an output stream (like `std::cout`, which is a `std::ostream`)
     * and the right operand is a `const Config&`, this function should be called.
     */
    std::ostream & operator<<(std::ostream &os, const Config &obj) {
        return os << "data_dir: " << obj.data_dir;
    }
}
