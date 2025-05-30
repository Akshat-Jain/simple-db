//
// Created by Akshat Jain on 22/05/25.
//

#ifndef HISTORY_H
#define HISTORY_H

namespace history {
    // Initialize the readline history subsystem.
    void init();

    // Save the in-memory history back to the history file.
    // Should be called just before program exit.
    void save();
}  // namespace history

#endif  // HISTORY_H
