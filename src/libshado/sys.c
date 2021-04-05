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

int sys_select (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
               struct timeval *timeout) {
    return _syscall(SYS_select, (void*)(long)nfds, readfds, writefds, exceptfds, timeout, 0);
}

void *mem_mmap (void *addr, size_t length, int prot, int flags, int fd,
               off_t offset) {
    return (void*)_syscall(SYS_mmap, addr, (void*)length, (void*)(long)prot,
           (void*)(long)flags, (void*)(long)fd, (void*)offset);
}

int mem_munmap (void *addr, size_t length) {
    return _syscall(SYS_munmap, addr, (void*)length, 0, 0, 0, 0);
}

int file_stat (const char *pathname, struct stat *statbuf) {
    return _syscall(SYS_stat, (void*)pathname, statbuf, 0, 0, 0, 0);
}

int file_close (unsigned long fd) {
    return _syscall(SYS_close, (void*)fd, 0, 0, 0, 0, 0);
}

int file_ioctl (unsigned long fd, unsigned long cmd, void *arg) {
    return _syscall(SYS_ioctl, (void*)fd, (void*)cmd, arg, 0, 0, 0);

}

int sys_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options) {
    return _syscall(SYS_waitid, (void*)(long)idtype, (void*)(long)id, infop, (void*)(long)options, 0, 0);
}
