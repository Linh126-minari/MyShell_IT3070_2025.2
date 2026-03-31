# Tiny Shell (myShell)
Tiny Shell là một trình thông dịch lệnh (Shell) tối giản được phát triển trên nền tảng Windows. Dự án tập trung vào việc mô phỏng cơ chế quản lý tiến trình của hệ điều hành, cho phép người dùng tương tác với hệ thống thông qua các câu lệnh dòng lệnh.

## Project Overview
Dự án được thực hiện trong khuôn khổ môn học Hệ điều hành (Chapter 2: Quản lý tiến trình).

Mục tiêu: Nghiên cứu và áp dụng các Windows API để khởi tạo, điều khiển và giám sát các tiến trình (Processes).

Phạm vi: Xây dựng một "myShell" có khả năng phân tích cú pháp lệnh, chạy ứng dụng ở chế độ đa nhiệm (Foreground/Background), quản lý danh sách tiến trình đang thực thi và xử lý các tệp kịch bản (.bat).

## Tech Stack
Language: C / C++ (Chuẩn C++11 trở lên).

Platform: Microsoft Windows.

APIs: * Windows.h: Thư viện lõi cho quản lý tiến trình (CreateProcess, TerminateProcess, OpenProcess).

Process.h: Hỗ trợ luồng và quản lý thực thi.

Compiler: MinGW (GCC) hoặc MSVC (Visual Studio).

## Project Structure
```
myShell/
├── src/                # Mã nguồn chính (.cpp)
│   ├── main.cpp        # Điểm khởi đầu (REPL Loop)
│   ├── shell.cpp       # Logic xử lý phân tích lệnh (Parsing)
│   ├── process_mgr.cpp # Quản lý danh sách tiến trình con
│   └── builtins.cpp    # Cài đặt các lệnh nội trú (dir, date, time...)
├── include/            # Các file tiêu đề (.h)
│   ├── shell.h
│   ├── process_mgr.h
│   └── constants.h
├── examples/           # Các file kịch bản mẫu
│   └── test.bat        # File chạy thử nghiệm lệnh hàng loạt
├── bin/                # Chứa file thực thi sau khi biên dịch (.exe)
└── README.md           # Tài liệu hướng dẫn dự án
```
## Core Features Implementation

- [ ] Parsing & CreateProcess: Phân tích lệnh & Tạo tiến trình
- [ ] Process Management: Quản lý tiến trình chạy ngầm
- [ ] Built-in Commands & Environment Management: Biến môi trường & Lệnh nội trú
- [ ] Signal Handling: Xử lý tín hiệu
- [ ] Batch Script Execution: Thực thi Batch File
