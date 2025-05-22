//
// Created by Akshat Jain on 22/05/25.
//

#include "simpledb/history.h"

#include <iostream>
#include <string>
#include <readline/history.h>
#include "simpledb/config.h"
#include "simpledb/utils/logging.h"

namespace history {

    static const int MAX_HISTORY_ENTRIES = 500;

    void init() {
        stifle_history(MAX_HISTORY_ENTRIES);

        std::filesystem::path history_file_path = config::get_config().history_file;
        std::filesystem::path parent = history_file_path.parent_path();
        if (!std::filesystem::exists(parent)) {
            logging::log.warn("Parent directory for history file doesn't exist.");
            return;
        }

        // Read the history file if it exists.
        if (std::filesystem::exists(history_file_path)) {
            if (read_history(history_file_path.c_str()) == 0) {
                logging::log.info("Successfully loaded history from: {}", history_file_path.string());
            } else {
                logging::log.warn("Failed to read history from: {}.", history_file_path.string());
            }
        }
    }

    void save() {
        std::filesystem::path history_file_path = config::get_config().history_file;
        if (history_file_path.empty()) {
            logging::log.warn("History file path is not configured. History will not be saved.");
            return;
        }

        if (write_history(history_file_path.c_str()) == 0) {
            logging::log.info("Successfully saved history to: {}", history_file_path.string());
        } else {
            logging::log.error("Failed to write history to: {}. Check permissions and path.", history_file_path.string());
        }
    }
}
