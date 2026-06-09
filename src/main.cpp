#include <iostream>
#include <string>
#include <windows.h> // Bắt buộc để dùng Windows API
#include "shell.h"   // Giả định bạn đặt các hàm xử lý trong file này

extern volatile bool g_batchInterrupted;

// 1. Hàm xử lý tín hiệu (Handler Routine)
// Giúp Shell không bị tắt đột ngột khi nhấn Ctrl+C
BOOL WINAPI ConsoleHandler(DWORD dwType) {
    if (dwType == CTRL_C_EVENT) {
        g_batchInterrupted = true;
        ProcessManager::terminateForeground();
        return TRUE; // Trả về TRUE để báo Windows là mình đã xử lý, đừng tắt Shell
    }
    return FALSE;
}

int main() {
    // 2. Đăng ký Handler với Windows
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        std::cerr << "ERROR: Could not set control handler";
        return 1;
    }

    // 3. Hiển thị lời chào (Giống trong ảnh ví dụ)
    std::cout << "***********************************" << std::endl;
    std::cout << "* WELCOME TO MY SHELL       *" << std::endl;
    std::cout << "***********************************" << std::endl;

    std::string command;

    while (true) {
        std::cout << "MyShell> ";
        
        // Kiểm tra nếu getline thất bại (ví dụ khi nhấn Ctrl+Z hoặc lỗi luồng)
        if (!std::getline(std::cin, command)) break;

        // Xóa khoảng trắng thừa ở đầu/cuối (nếu có hàm trim)
        // command = trim(command);

        if (command == "msh-exit") {
            // 4. Trước khi thoát, phải dọn dẹp tiến trình con (Kill all background processes)
            ProcessManager::cleanup(); 
            break;
        }

        if (!command.empty()) {
            processCommand(command);
        }
    }

    std::cout << "Shell exited. Goodbye!" << std::endl;
    return 0;
}
//Build:g++ -std=c++11 -o bin\myShell.exe src\main.cpp src\shell.cpp src\process_manager.cpp src\builtins.cpp -I include
//Chạy terminal:.\bin\myShell.exe