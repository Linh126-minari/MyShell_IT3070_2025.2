#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <limits>
#include <fstream>
#include <cctype>
#include <map>
#include "shell.h"
#include "builtins.h"

volatile bool g_batchInterrupted = false;

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

    std::string ltrim(const std::string& text) {
        size_t start = 0;
        while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start]))) {
            ++start;
        }
        return text.substr(start);
    }

    std::string rtrim(const std::string& text) {
        if (text.empty()) return text;
        size_t end = text.size();
        while (end > 0 && std::isspace(static_cast<unsigned char>(text[end - 1]))) {
            --end;
        }
        return text.substr(0, end);
    }

    std::string trim(const std::string& text) {
        return rtrim(ltrim(text));
    }

    std::vector<std::string> splitByComma(const std::string& str) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, ',')) {
            std::string trimmed = trim(token);
            if (!trimmed.empty()) {
                result.push_back(trimmed);
            }
        }
        return result;
    }

    std::string toLower(std::string value) {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool startsWithIgnoreCase(const std::string& text, const std::string& prefix) {
        if (text.size() < prefix.size()) return false;
        return toLower(text.substr(0, prefix.size())) == toLower(prefix);
    }

    bool isBatchFile(const std::string& cmd) {
        std::string lower = toLower(cmd);
        if (lower.size() < 4) return false;
        return lower.rfind(".bat") == lower.size() - 4;
    }

    std::vector<std::string> tokenize(const std::string& input) {
        std::vector<std::string> tokens;
        std::string current;
        bool inQuotes = false;
        for (size_t i = 0; i < input.size(); ++i) {
            char c = input[i];
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (std::isspace(static_cast<unsigned char>(c)) && !inQuotes) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current.push_back(c);
            }
        }
        if (!current.empty()) {
            tokens.push_back(current);
        }
        return tokens;
    }

    void processCommandInternal(const std::string& input, bool fromBatch);
    bool executeBatchFile(const std::string& path);
}

void processCommand(const std::string& input) {
    processCommandInternal(input, false);
}

namespace {
    void processCommandInternal(const std::string& input, bool fromBatch) {
        if (input.empty()) return;

        auto args = tokenize(input);
        if (args.empty()) return;

        bool isBackground = false;

        if (args.back() == "&") {
            isBackground = true;
            args.pop_back();
        }

        std::string cmd = args[0];

        if (isBatchFile(cmd)) {
            executeBatchFile(cmd);
            return;
        }

        if (Builtins::handleBuiltin(args, isBackground)) {
            return;
        }

        if (cmd == "msh-list") {
            ProcessManager::list();
        } else if (cmd == "msh-killall") {
            ProcessManager::killAll();
        } else if (cmd == "msh-kill" && args.size() > 1) {
            std::vector<std::string> pidStrings;
            for (size_t i = 1; i < args.size(); ++i) {
                auto splitParts = splitByComma(args[i]);
                pidStrings.insert(pidStrings.end(), splitParts.begin(), splitParts.end());
            }
            if (pidStrings.empty()) {
                std::cout << "Usage: msh-kill <PID1> [PID2] ...\n";
            } else {
                for (const auto& pidStr : pidStrings) {
                    DWORD pid;
                    if (!tryParsePid(pidStr, pid)) {
                        std::cout << "Error: Invalid PID: " << pidStr << ".\n";
                    } else if (!ProcessManager::kill(pid)) {
                        std::cout << "Error: PID " << pid << " not found.\n";
                    }
                }
            }
        } else if (cmd == "msh-stop" && args.size() > 1) {
            DWORD pid;
            if (!tryParsePid(args[1], pid)) {
                std::cout << "Error: Invalid PID.\n";
            } else if (!ProcessManager::stop(pid)) {
                std::cout << "Error: Cannot stop process.\n";
            }
        } else if (cmd == "msh-resume" && args.size() > 1) {
            DWORD pid;
            if (!tryParsePid(args[1], pid)) {
                std::cout << "Error: Invalid PID.\n";
            } else if (!ProcessManager::resume(pid)) {
                std::cout << "Error: Cannot resume process.\n";
            }
        } else if ((cmd == "msh-kill" || cmd == "msh-stop" || cmd == "msh-resume") && args.size() <= 1) {
            if (cmd == "msh-kill") {
                std::cout << "Usage: msh-kill <PID1> [PID2] ...\n";
            } else {
                std::cout << "Usage: " << cmd << " <PID>\n";
            }
        } else if (cmd == "exit" && fromBatch) {
            return;
        } else {
            ProcessManager::launch(args, isBackground);
        }
    }

    bool executeBatchFile(const std::string& path) {
        g_batchInterrupted = false;
        std::ifstream file(path.c_str());
        if (!file.is_open()) {
            std::cout << "Error: Cannot open batch file: " << path << "\n";
            return false;
        }

        bool echoEnabled = true;
        std::string line;
        std::vector<std::string> lines;
        std::map<std::string, size_t> labels;

        // First pass: read all lines and build label map
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            lines.push_back(line);

            // Check for labels
            std::string trimmed = trim(line);
            if (!trimmed.empty() && trimmed[0] == ':' && trimmed[1] != ':') {
                std::string labelName = toLower(trimmed.substr(1));
                labels[labelName] = lines.size() - 1;
            }
        }

        file.close();

        // Second pass: execute lines
        for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
            if (g_batchInterrupted) {
                std::cout << "\nBatch execution interrupted by user.\n";
                break;
            }

            line = lines[lineIndex];

            bool suppressEcho = false;
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;

            // Skip labels (lines starting with :)
            if (!trimmed.empty() && trimmed[0] == ':' && trimmed[1] != ':') {
                continue;
            }

            if (trimmed[0] == '@') {
                suppressEcho = true;
                trimmed = trim(trimmed.substr(1));
            }

            if (trimmed.empty()) continue;

            if (startsWithIgnoreCase(trimmed, "rem") || startsWithIgnoreCase(trimmed, "::")) {
                continue;
            }

            if (startsWithIgnoreCase(trimmed, "echo off")) {
                echoEnabled = false;
                continue;
            }

            if (startsWithIgnoreCase(trimmed, "echo on")) {
                echoEnabled = true;
                continue;
            }

            // Check for echo command
            bool isEchoCmd = false;
            std::string echoText;
            if (toLower(trimmed) == "echo") {
                isEchoCmd = true;
                echoText = echoEnabled ? "ECHO is on." : "ECHO is off.";
            } else if (toLower(trimmed) == "echo." || startsWithIgnoreCase(trimmed, "echo. ")) {
                isEchoCmd = true;
                echoText = "";
            } else if (startsWithIgnoreCase(trimmed, "echo ")) {
                isEchoCmd = true;
                echoText = trimmed.substr(5);
            }

            if (isEchoCmd) {
                if (echoEnabled && !suppressEcho) {
                    std::cout << trimmed << "\n";
                }
                std::cout << echoText << "\n";
                continue;
            }

            if (startsWithIgnoreCase(trimmed, "goto ")) {
                std::string labelName = toLower(trim(trimmed.substr(5)));
                if (labels.find(labelName) != labels.end()) {
                    lineIndex = labels[labelName];
                } else {
                    std::cout << "Error: Label '" << labelName << "' not found.\n";
                }
                continue;
            }

            if (echoEnabled && !suppressEcho) {
                std::cout << trimmed << "\n";
            }

            if (toLower(trimmed) == "exit") {
                break;
            }

            processCommandInternal(trimmed, true);
        }

        return true;
    }
}
