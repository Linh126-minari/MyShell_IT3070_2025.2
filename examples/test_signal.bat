@echo off
REM Test CTRL+C signal handling
REM Mục đích: Kiểm tra tính năng ngắt tiến trình foreground từ bàn phím

REM Test 1: Quick command (foreground)
echo === Test 1: Quick foreground - msh-dir command ===
msh-dir bin

REM Test 2: Lặp vô hạn để test CTRL+C (foreground)
REM Note: Nhấn CTRL+C khi chạy để kiểm tra tính năng ngắt tiến trình
echo.
echo === Test 2: Infinite loop - Test CTRL+C ===
echo Starting infinite loop... Press CTRL+C to test signal handling
echo.
:loop
echo Running iteration - Press CTRL+C now...
goto loop



