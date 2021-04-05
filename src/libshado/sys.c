#include <syscall.h>
#include "../include/libshado.h"

unsigned long file_open (char *fn, int flags) {
    return _syscall(SYS_open, fn, (void*)(long)flags, 0, 0, 0, 0);
}

unsigned long file_read (unsigned long fd, char *buff, unsigned long size) {
    return _syscall(SYS_read, (void*)fd, buff, (void*)size, 0, 0, 0);
}

unsigned long file_write (unsigned long fd, char *buf, unsigned long len) {
    return _syscall(SYS_write, (void*)fd, buf, (void*)len, 0, 0, 0);
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

long ps_fork () {
    return _syscall(SYS_fork, 0, 0, 0, 0, 0, 0);
}

long ps_execve (char *filename, char **argv, char **envp) {
    return _syscall(SYS_execve, filename, argv, envp, 0, 0, 0);
}

int ps_execute (char *filename) {
    long pid = ps_fork();
    if (!pid) {
        char *argv[2];
        argv[0] = 0;
        
        char *envp[1];
        envp[0] = 0;

        return ps_execve(filename, argv, envp);
    }
}

void *mem_brk (void *p) {
    return (void*)_syscall(SYS_brk, p, 0, 0, 0, 0, 0);
}
