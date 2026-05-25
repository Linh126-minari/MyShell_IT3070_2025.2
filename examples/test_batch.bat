@echo off
rem Sample batch test for myShell (native runner)
echo ==== myShell batch test ====
echo Current PATH:
path
echo List include folder:
dir include
echo Current date/time:
date
time
echo Builtin help:
help
echo Background test (dir &):
dir include &
echo Done.

