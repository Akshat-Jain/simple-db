//
// Created by Akshat Jain on 19/04/25.
//

#ifndef CONFIG_H
#define CONFIG_H
#include <ostream>
#include <__filesystem/filesystem_error.h>

namespace config {

    struct Config {
        std::filesystem::path data_dir;

        friend std::ostream & operator<<(std::ostream &os, const Config &obj);
    };

    void init_config();

    const Config& get_config();
}

#endif //CONFIG_H
