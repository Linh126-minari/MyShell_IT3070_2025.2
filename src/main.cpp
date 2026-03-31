#include <iostream>
#include <string>

int main() {
    std::string command;

    while (true) {
        std::cout << "MyShell>";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }
        executeCommand(command);
    }
    return 0;
}