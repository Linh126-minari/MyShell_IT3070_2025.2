#include <stdio.h>
#include <windows.h>

int main() {
    DWORD pid = GetCurrentProcessId();
    printf("[test_bg_print] Starting process with PID: %lu\n", pid);
    fflush(stdout);

    int count = 0;
    while (1) {
        printf("[test_bg_print - PID %lu] Iteration %d\n", pid, ++count);
        fflush(stdout);
        Sleep(2000);
    }
    return 0;
}
