#include <syscall.h>
#include <time.h>
#include "../include/libshado.h"

unsigned long file_open (char *fn, int flags) {
    return _syscall(SYS_open, fn, (void*)(long)flags, 0, 0, 0, 0);
}

unsigned long file_read (unsigned long fd, char *buff, unsigned long size) {
    return _syscall(SYS_read, (void*)fd, buff, (void*)size, 0, 0, 0);
}

unsigned long sys_reboot () {
    return _syscall(SYS_reboot, (void*)0xfee1dead, (void*)672274793, (void*)0x1234567, 0, 0, 0);
}

unsigned long sys_nanosleep (struct timespec *req, struct timespec *rem) {
    return _syscall(SYS_nanosleep, req, rem, 0, 0, 0, 0);
}

void sleep_sec (int sec) {
    struct timespec tm;
    tm.tv_nsec = 0;
    tm.tv_sec = sec;

    sys_nanosleep(&tm, NULL);
}
