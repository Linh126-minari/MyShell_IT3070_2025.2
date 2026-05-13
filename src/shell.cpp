#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <limits>
#include "shell.h"
#include "builtins.h"

namespace {
    bool tryParsePid(const std::string& text, DWORD& pid) {
        try {
            size_t idx = 0;
            unsigned long value = std::stoul(text, &idx);
            if (idx != text.size() || value > std::numeric_limits<DWORD>::max()) {
                return false;
            }
            pid = static_cast<DWORD>(value);
            return true;
        } catch (...) {
            return false;
        }
    }
}

// Hàm tách chuỗi (Tokenize)
std::vector<std::string> tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream ss(input);
    std::string temp;
    while (ss >> temp) tokens.push_back(temp);
    return tokens;
}

void processCommand(const std::string& input) {
    if (input.empty()) return;

    auto args = tokenize(input);
    if (args.empty()) return;

    bool isBackground = false;

    // Kiểm tra & ở cuối
    if (args.back() == "&") {
        isBackground = true;
        args.pop_back();
    }

    std::string cmd = args[0];

    if (Builtins::handleBuiltin(args, isBackground)) {
        return;
    }

    // ROUTING: Điều hướng lệnh
    if (cmd == "list") {
        ProcessManager::list();
    }
    else if (cmd == "kill" && args.size() > 1) {
        DWORD pid;
        if (!tryParsePid(args[1], pid)) {
            std::cout << "Error: Invalid PID.\n";
        } else if (!ProcessManager::kill(pid)) {
            std::cout << "Error: PID not found.\n";
        }
    }
    else if (cmd == "stop" && args.size() > 1) {
        DWORD pid;
        if (!tryParsePid(args[1], pid)) {
            std::cout << "Error: Invalid PID.\n";
        } else if (!ProcessManager::stop(pid)) {
            std::cout << "Error: Cannot stop process.\n";
        }
    }
    else if (cmd == "resume" && args.size() > 1) {
        DWORD pid;
        if (!tryParsePid(args[1], pid)) {
            std::cout << "Error: Invalid PID.\n";
        } else if (!ProcessManager::resume(pid)) {
            std::cout << "Error: Cannot resume process.\n";
        }
    }
    else if ((cmd == "kill" || cmd == "stop" || cmd == "resume") && args.size() <= 1) {
        std::cout << "Usage: " << cmd << " <PID>\n";
    }
    else {
        // Mặc định gọi tiến trình ngoại trú
        ProcessManager::launch(args, isBackground);
    }
}