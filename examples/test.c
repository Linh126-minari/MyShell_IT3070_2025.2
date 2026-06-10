#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main() {
    while (1) {
        printf("Hello, World!\n");
        fflush(stdout);
        Sleep(1000); 
    }
}