//
// Created by Akshat Jain on 19/04/25.
//

#ifndef CONFIG_H
#define CONFIG_H
#include <__filesystem/filesystem_error.h>

namespace config {

    struct Config {
        std::filesystem::path data_dir;
    };

    void init_config();

    const Config& get_config();
}

#endif //CONFIG_H
