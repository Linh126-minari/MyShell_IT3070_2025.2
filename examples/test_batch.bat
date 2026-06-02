@echo off
rem Sample batch test for myShell (native runner)
echo ==== myShell batch test ====
echo Current PATH:
msh-path
echo List include folder:
msh-dir include
echo Current date/time:
msh-date
msh-time
echo Builtin help:
msh-help
echo Background test (msh-dir &):
msh-dir include &
echo Done.

