#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <iomanip>
#include <windows.h>
#include "builtins.h"
#include "process_manager.h"

namespace {
	std::string joinArgs(const std::vector<std::string>& args, size_t startIndex) {
		std::string value;
		for (size_t i = startIndex; i < args.size(); ++i) {
			if (!value.empty()) value += " ";
			value += args[i];
		}
		return value;
	}

	std::wstring toWideFromConsole(const std::string& input) {
		if (input.empty()) return std::wstring();
		UINT codePage = GetConsoleCP();
		int needed = MultiByteToWideChar(codePage, 0, input.c_str(), -1, nullptr, 0);
		if (needed <= 0) return std::wstring();
		std::wstring wide(needed - 1, L'\0');
		MultiByteToWideChar(codePage, 0, input.c_str(), -1, &wide[0], needed);
		return wide;
	}

	bool listDirectoryWide(const std::wstring& rawPath) {
		std::wstring target = rawPath.empty() ? L"." : rawPath;
		std::wstring fullPath;
		DWORD fullLen = GetFullPathNameW(target.c_str(), 0, nullptr, nullptr);
		if (fullLen > 0) {
			fullPath.resize(fullLen - 1);
			GetFullPathNameW(target.c_str(), fullLen, &fullPath[0], nullptr);
		} else {
			fullPath = target;
		}

		std::wstring search = target;
		if (!search.empty() && (search.back() == L'\\' || search.back() == L'/')) {
			search += L"*";
		} else {
			search += L"\\*";
		}

		WIN32_FIND_DATAW data;
		HANDLE hFind = FindFirstFileW(search.c_str(), &data);
		if (hFind == INVALID_HANDLE_VALUE) {
			std::wcout << L"Directory not found: " << fullPath << L"\n";
			return false;
		}

		std::wcout << L"\n Directory of " << fullPath << L"\n\n";
		do {
			const std::wstring name = data.cFileName;
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				std::wcout << L"<DIR>     " << name << L"\n";
			} else {
				ULONGLONG size = (static_cast<ULONGLONG>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;
				std::wcout << std::setw(10) << size << L" " << name << L"\n";
			}
		} while (FindNextFileW(hFind, &data));

		FindClose(hFind);
		return true;
	}
}

namespace Builtins {
	bool handleBuiltin(const std::vector<std::string>& args, bool isBackground) {
		if (args.empty()) return false;

		const std::string& cmd = args[0];

		if (cmd == "help") {
			ProcessManager::help();
			return true;
		}

		if (cmd == "date") {
			ProcessManager::showDateTime(true);
			return true;
		}

		if (cmd == "time") {
			ProcessManager::showDateTime(false);
			return true;
		}

		if (cmd == "path") {
			if (args.size() == 1) {
				ProcessManager::showPath();
			} else {
				std::string value = joinArgs(args, 1);
				if (!ProcessManager::setPath(value)) {
					std::cout << "Error: Could not set PATH.\n";
				}
			}
			return true;
		}

		if (cmd == "addpath") {
			if (args.size() <= 1) {
				std::cout << "Usage: addpath <value>\n";
			} else {
				std::string value = joinArgs(args, 1);
				if (!ProcessManager::addPath(value)) {
					std::cout << "Error: Could not add to PATH.\n";
				}
			}
			return true;
		}

		if (cmd == "dir") {
			const std::string dirPath = (args.size() > 1) ? joinArgs(args, 1) : ".";
			const std::wstring widePath = toWideFromConsole(dirPath);
			(void)isBackground;
			listDirectoryWide(widePath);
			return true;
		}

		return false;
	}
}
