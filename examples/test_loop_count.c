#include <stdio.h>
#include <windows.h>

int main() {
    printf("[test_loop_count] Process started. Counting to 10...\n");
    fflush(stdout);
    for (int i = 1; i <= 10; i++) {
        printf("[test_loop_count] Iteration %d / 10\n", i);
        fflush(stdout);
        Sleep(800);
    }
    printf("[test_loop_count] Process finished!\n");
    fflush(stdout);
    return 0;
}
