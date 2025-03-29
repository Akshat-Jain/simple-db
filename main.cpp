//
// Created by Akshat Jain on 29/03/25.
//

#include <iostream>
#include <vector> // Just including something C++ specific
#include <string> // And another one

int main() {
    // Print a simple message to confirm execution
    std::cout << "Simple DB project started!" << std::endl;
    std::cout << "C++ Standard: " << __cplusplus << std::endl; // Check C++ standard

    // You can add a small test using a C++17 feature if you like,
    // for example, structured binding:
    std::pair<int, std::string> myPair = {1, "test"};
    auto [id, name] = myPair; // C++17 structured binding
    std::cout << "Test structured binding: id=" << id << ", name=" << name << std::endl;


    // Your main database REPL (Read-Eval-Print Loop) will eventually go here.
    // For now, we just exit successfully.

    return 0; // Indicate successful execution
}
