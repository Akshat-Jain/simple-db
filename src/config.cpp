//
// Created by Akshat Jain on 19/04/25.
//


#include "simpledb/config.h"

#include <iostream>
#include <filesystem>

namespace config {
    static Config config;
    static bool initialized = false;

    const static std::string ENV_DATA_DIR = "SIMPLE_DB_DATA_DIR";
    const static std::string DEFAULT_DATA_DIR = "data";

    void init_config() {
        if (initialized) {
            return;
        }

        // read from env
        const char* env_data_dir = std::getenv("SIMPLE_DB_DATA_DIR");
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

        initialized = true;
    }

    const Config& get_config() {
        if (initialized) {
            init_config();
        }
        return config;
    }
}
