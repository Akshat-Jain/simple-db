//
// Created by Akshat Jain on 19/04/25.
//

#ifndef CONFIG_H
#define CONFIG_H
#include <ostream>
#include <filesystem>

namespace config {

    struct Config {
        std::filesystem::path data_dir;
        std::filesystem::path history_file;

        friend std::ostream &operator<<(std::ostream &os, const Config &obj);
    };

    void init_config();

    const Config &get_config();
}  // namespace config

#endif  // CONFIG_H
