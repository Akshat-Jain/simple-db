//
// Created by Akshat Jain on 20/04/25.
//

#ifndef LOGGING_H
#define LOGGING_H

#ifdef DEBUG_MODE // Check if the control flag is defined
    // Only include iostream if logging is actually enabled to minimize dependencies
    #include <iostream>

    // Define the debug log macro to print to std::cerr
    #define DEBUG_LOG(message) \
    do { \
    std::cerr << "[DEBUG] (" << __FILE__ << ":" << __LINE__ << ") " \
    << message << std::endl; \
    } while (0)
    /* The do-while(0) loop ensures the macro behaves like a single statement,
       preventing potential issues with if/else without braces. */

#else // If DEBUG_MODE is NOT defined
    // Define the debug log macro as empty when disabled
    #define DEBUG_LOG(message) do {} while (0)

#endif // End of DEBUG_MODE conditional compilation

#endif //LOGGING_H
