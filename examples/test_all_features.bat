@echo off
REM Test comprehensive shell features
REM Include: foreground command, background process, msh-list, msh-kill, msh-stop, msh-resume

echo === Test 1: Show help ===
msh-help

echo.
echo === Test 2: Show date and time ===
msh-date
msh-time

echo.
echo === Test 3: List directory (msh-dir) ===
msh-dir bin

echo.
echo === Test 4: Background process - Notepad ===
echo Starting notepad in background...
notepad &

echo.
echo === Test 5: List all background processes ===
msh-list

echo.
echo === Test 6: Demo foreground msh-dir command ===
msh-dir include

echo.
echo === Test: Signal handling ===
echo To test CTRL+C, run: examples\test_signal.bat
echo Then press CTRL+C to interrupt the foreground loop

echo.
echo === All tests completed ===
exit

