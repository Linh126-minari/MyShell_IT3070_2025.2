#include <stdio.h>
#include <windows.h>

int main() {
    char name[100];
    printf("[test_input] Hello! Please enter your name: ");
    fflush(stdout);
    
    if (fgets(name, sizeof(name), stdin) != NULL) {
        // Strip trailing newline character
        for (int i = 0; name[i] != '\0'; i++) {
            if (name[i] == '\n' || name[i] == '\r') {
                name[i] = '\0';
                break;
            }
        }
        printf("[test_input] Welcome to MyShell, %s!\n", name);
        fflush(stdout);
    } else {
        printf("[test_input] Failed to read input.\n");
        fflush(stdout);
    }
    return 0;
}
