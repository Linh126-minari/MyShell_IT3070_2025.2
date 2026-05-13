#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <vector>
#include <string>
#include <windows.h>
#include "common.h"

namespace ProcessManager {
    // Lưu trữ danh sách tiến trình ngầm
    extern std::vector<ProcessInfo> bgProcesses;

    // Các hàm thực thi chính
    void launch(const std::vector<std::string>& args, bool isBackground);
    bool kill(DWORD pid);
    bool stop(DWORD pid);
    bool resume(DWORD pid);
    void help();
    void showDateTime(bool isDate);
    void list();
    void cleanup(); // Gọi khi thoát Shell
    bool terminateForeground();
    void showPath();
    bool setPath(const std::string& value);
    bool addPath(const std::string& value);
}

#endif