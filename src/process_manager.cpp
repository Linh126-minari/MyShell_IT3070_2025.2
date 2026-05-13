#include <iostream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <atomic>
#include "process_manager.h"
#include <tlhelp32.h>

namespace {
    using NtSuspendProcessFn = LONG (NTAPI*)(HANDLE);
    using NtResumeProcessFn = LONG (NTAPI*)(HANDLE);

#ifndef PROCESS_SUSPEND_RESUME
#define PROCESS_SUSPEND_RESUME 0x0800
#endif

    bool suspendProcessByThreads(DWORD pid) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;

        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        bool suspendedAny = false;

        if (Thread32First(snapshot, &te)) {
            do {
                if (te.th32OwnerProcessID == pid) {
                    HANDLE th = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                    if (th != NULL) {
                        if (SuspendThread(th) != (DWORD)-1) {
                            suspendedAny = true;
                        }
                        CloseHandle(th);
                    }
                }
            } while (Thread32Next(snapshot, &te));
        }

        CloseHandle(snapshot);
        return suspendedAny;
    }

    bool resumeProcessByThreads(DWORD pid) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return false;

        THREADENTRY32 te;
        te.dwSize = sizeof(te);
        bool resumedAny = false;

        if (Thread32First(snapshot, &te)) {
            do {
                if (te.th32OwnerProcessID == pid) {
                    HANDLE th = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                    if (th != NULL) {
                        DWORD prev = ResumeThread(th);
                        if (prev != (DWORD)-1) {
                            resumedAny = true;
                            // If suspended multiple times, keep resuming to running state.
                            while (prev > 1) {
                                prev = ResumeThread(th);
                                if (prev == (DWORD)-1) break;
                            }
                        }
                        CloseHandle(th);
                    }
                }
            } while (Thread32Next(snapshot, &te));
        }

        CloseHandle(snapshot);
        return resumedAny;
    }

    bool suspendProcess(HANDLE processHandle, DWORD pid) {
        HANDLE proc = OpenProcess(PROCESS_SUSPEND_RESUME | PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (proc == NULL) proc = processHandle;

        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (ntdll == NULL) ntdll = LoadLibraryA("ntdll.dll");

        if (ntdll != NULL) {
            FARPROC raw = GetProcAddress(ntdll, "NtSuspendProcess");
            NtSuspendProcessFn fn = nullptr;
            if (raw != nullptr) {
                std::memcpy(&fn, &raw, sizeof(fn));
            }
            if (fn != nullptr) {
                LONG status = fn(proc);
                if (status >= 0) {
                    if (proc != processHandle) CloseHandle(proc);
                    return true;
                }
            }
        }

        bool ok = suspendProcessByThreads(pid);
        if (proc != processHandle) CloseHandle(proc);
        return ok;
    }

    bool resumeProcess(HANDLE processHandle, DWORD pid) {
        HANDLE proc = OpenProcess(PROCESS_SUSPEND_RESUME | PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (proc == NULL) proc = processHandle;

        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
        if (ntdll == NULL) ntdll = LoadLibraryA("ntdll.dll");

        if (ntdll != NULL) {
            FARPROC raw = GetProcAddress(ntdll, "NtResumeProcess");
            NtResumeProcessFn fn = nullptr;
            if (raw != nullptr) {
                std::memcpy(&fn, &raw, sizeof(fn));
            }
            if (fn != nullptr) {
                LONG status = fn(proc);
                if (status >= 0) {
                    if (proc != processHandle) CloseHandle(proc);
                    return true;
                }
            }
        }

        bool ok = resumeProcessByThreads(pid);
        if (proc != processHandle) CloseHandle(proc);
        return ok;
    }

    std::atomic<HANDLE> fgHandle{ NULL };

    void setForegroundProcess(HANDLE handle, DWORD pid) {
        fgHandle.store(handle, std::memory_order_release);
        (void)pid;
    }

    void clearForegroundProcess() {
        fgHandle.store(NULL, std::memory_order_release);
    }

    std::string getEnvVar(const char* name) {
        DWORD size = GetEnvironmentVariableA(name, nullptr, 0);
        if (size == 0) return std::string();
        std::string value(size, '\0');
        DWORD written = GetEnvironmentVariableA(name, &value[0], size);
        if (written == 0) return std::string();
        if (!value.empty() && value.back() == '\0') {
            value.pop_back();
        }
        return value;
    }

    bool setEnvVar(const char* name, const std::string& value) {
        return SetEnvironmentVariableA(name, value.c_str()) != 0;
    }

    bool pathContains(const std::string& pathList, const std::string& entry) {
        if (entry.empty()) return false;
        size_t start = 0;
        while (start <= pathList.size()) {
            size_t end = pathList.find(';', start);
            if (end == std::string::npos) end = pathList.size();
            if (pathList.substr(start, end - start) == entry) {
                return true;
            }
            if (end == pathList.size()) break;
            start = end + 1;
        }
        return false;
    }
}

namespace ProcessManager {
    std::vector<ProcessInfo> bgProcesses;

    // --- Tạo tiến trình ---
    void launch(const std::vector<std::string>& args, bool isBackground) {
        if (args.empty()) {
            std::cout << "Bad command or file name..." << std::endl;
            return;
        }

        std::string cmdLine = "";
        for (const auto& arg : args) cmdLine += arg + " ";

        STARTUPINFOA si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi));

        if (CreateProcessA(NULL, &cmdLine[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            if (isBackground) {
                std::cout << "[Background] Started PID: " << pi.dwProcessId << std::endl;
                // LƯU CẢ hProcess VÀ hThread
                bgProcesses.push_back({ pi.dwProcessId, pi.hProcess, pi.hThread, args[0], "Running" });
            } else {
                setForegroundProcess(pi.hProcess, pi.dwProcessId);
                // Foreground: Đợi và dọn dẹp ngay
                WaitForSingleObject(pi.hProcess, INFINITE);
                clearForegroundProcess();
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        } else {
            std::cout << "Bad command or file name..." << std::endl;
        }
    }

    // --- Dừng tiến trình (Kill) ---
    bool kill(DWORD pid) {
        for (auto it = bgProcesses.begin(); it != bgProcesses.end(); ++it) {
            if (it->pid == pid) {
                TerminateProcess(it->hProcess, 0);
                CloseHandle(it->hProcess);
                CloseHandle(it->hThread);
                bgProcesses.erase(it);
                return true;
            }
        }
        return false;
    }

    // --- Tạm dừng (Stop) ---
    bool stop(DWORD pid) {
        for (auto& p : bgProcesses) {
            if (p.pid == pid) {
                if (p.status == "Stopped") return true;

                if (suspendProcess(p.hProcess, p.pid)) {
                    p.status = "Stopped";
                    return true;
                }
            }
        }
        return false;
    }

    // --- Tiếp tục (Resume) ---
    bool resume(DWORD pid) {
        for (auto& p : bgProcesses) {
            if (p.pid == pid) {
                if (p.status == "Running") return true;

                if (resumeProcess(p.hProcess, p.pid)) {
                    p.status = "Running";
                    return true;
                }
            }
        }
        return false;
    }

    // --- Dọn dẹp tiến trình ngầm khi thoát Shell ---
    void cleanup() {
        if (bgProcesses.empty()) return;

        std::cout << "\n[System] Sending kill signal to " << bgProcesses.size() 
                  << " background process(es)..." << std::endl;

        for (auto& p : bgProcesses) {
            // 1. Ép buộc dừng tiến trình
            TerminateProcess(p.hProcess, 0);

            // 2. Đóng Handle tiến trình (Giải phóng bộ nhớ hệ thống)
            CloseHandle(p.hProcess);

            // 3. Đóng Handle luồng (Quan trọng vì chúng ta đã giữ lại để Stop/Resume)
            if (p.hThread != NULL) {
                CloseHandle(p.hThread);
            }
        }

        // 4. Xóa sạch danh sách trong bộ nhớ RAM của Shell
        bgProcesses.clear();
        
        std::cout << "[System] Cleanup complete." << std::endl;
    }

    bool terminateForeground() {
        HANDLE handle = fgHandle.load(std::memory_order_acquire);
        if (handle == NULL) {
            return false;
        }

        TerminateProcess(handle, 0);
        return true;
    }

    // --- Trợ giúp ---
    void help() {
        std::cout << "\nWELCOME TO MY SHELL\n";
        std::cout << "myShell supports the following commands:\n";
        std::cout << "dir     : List the contents of the current directory\n";
        std::cout << "list    : List all background processes\n";
        std::cout << "kill    : Terminate a process (kill <PID>)\n";
        std::cout << "stop    : Suspend a process (stop <PID>)\n";
        std::cout << "resume  : Resume a process (resume <PID>)\n";
        std::cout << "date/time: Show current system date and time\n";
        std::cout << "path    : Show or set PATH (path [value])\n";
        std::cout << "addpath : Append a folder to PATH (addpath <value>)\n";
        std::cout << "exit    : Exit my shell\n";
    }

    // --- Hiển thị ngày giờ ---
    void showDateTime(bool isDate) {
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        if (isDate)
            std::cout << "Current Date: " << (now->tm_year + 1900) << "-" << (now->tm_mon + 1) << "-" << now->tm_mday << std::endl;
        else
            std::cout << "Current Time: " << now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << std::endl;
    }
    // --- Liệt kê ---
    void list() {
        std::cout << std::left << std::setw(10) << "PID" << std::setw(20) << "Command" << "Status" << std::endl;
        std::cout << "----------------------------------------------" << std::endl;
        for (auto& p : bgProcesses) {
            std::cout << std::left << std::setw(10) << p.pid << std::setw(20) << p.command << p.status << std::endl;
        }
    }

    void showPath() {
        std::string path = getEnvVar("PATH");
        std::cout << "PATH=" << path << std::endl;
    }

    bool setPath(const std::string& value) {
        return setEnvVar("PATH", value);
    }

    bool addPath(const std::string& value) {
        if (value.empty()) return false;
        std::string current = getEnvVar("PATH");
        if (pathContains(current, value)) {
            return true;
        }
        std::string updated = current;
        if (!updated.empty() && updated.back() != ';') {
            updated += ';';
        }
        updated += value;
        return setEnvVar("PATH", updated);
    }
}