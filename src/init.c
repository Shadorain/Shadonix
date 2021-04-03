#include <syscall.h>

long _syscall (int num, void *a0, void *a1, void *a2, void *a3, void *a4, void *a5);

unsigned long _strlen (char *s) {
    int count = 0;
    while (*s++) count++;
    return count;
}

void delay (int ticks) {
    for (int i=0; i<ticks; i++) { /* nothing */ }
}

void print_string (char *s) {
    _syscall(SYS_write, (void*)1 /*stdout*/, s, (void*)_strlen(s), 0, 0, 0);
}

int main () {
    char *msg = "Shadonix 0.0.0.1 Initializing...\n";

    delay(1000000000);
    print_string(msg);

    while (1) {
        delay(1000000000);
        print_string(msg);
    }

    return 0;
}
