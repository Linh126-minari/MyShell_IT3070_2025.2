#ifndef COMMON_H
#define COMMON_H

#include <windows.h>
#include <string>

struct ProcessInfo {
    DWORD pid;
    HANDLE hProcess;
    HANDLE hThread;     // Thêm hThread để Stop/Resume chính xác
    std::string command;
    std::string status; // "Running", "Stopped"
};

#endif