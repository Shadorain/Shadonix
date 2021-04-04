#include <syscall.h>
#include "../include/libshado.h"

unsigned long str_len (char *s) {
    int count = 0;
    while (*s++) count++;
    return count;
}

void str_print (char *s) {
    _syscall(SYS_write, (void*)1 /*stdout*/, s, (void*)str_len(s), 0, 0, 0);
}

void delay (int ticks) {
    for (int i=0; i<ticks; i++) { /* nothing */ }
}
