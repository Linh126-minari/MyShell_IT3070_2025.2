@echo off

echo === MYSHELL AUTOMATED INTEGRATION TEST ===
echo This script demonstrates built-in commands and external processes.
echo.

echo [TEST] Executing built-in command: help
help
timeout /t 1 /nobreak >nul

echo [TEST] Executing built-in command: date
date
timeout /t 1 /nobreak >nul

echo [TEST] Executing built-in command: time
time
timeout /t 1 /nobreak >nul

echo [TEST] Executing built-in command: dir
dir
timeout /t 1 /nobreak >nul

echo [TEST] Executing external command: whoami
whoami
timeout /t 1 /nobreak >nul

echo [TEST] Executing external command: ping 127.0.0.1 -n 3
ping 127.0.0.1 -n 3
timeout /t 1 /nobreak >nul

echo.
echo === ALL TESTS COMPLETED SUCCESSFULLY ===
pause

