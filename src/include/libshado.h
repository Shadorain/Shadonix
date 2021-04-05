#pragma once
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

extern unsigned long _syscall (int num, void *a0, void *a1, void *a2, void *a3, void *a4, void *a5);

extern unsigned long file_open (char *fn, int flags);
extern unsigned long file_read (unsigned long fd, char *buff, unsigned long size);
extern unsigned long file_write (unsigned long fd, char *buf, unsigned long len);
extern int file_stat(const char *pathname, struct stat *statbuf);
extern int file_close(unsigned long fd);
extern int file_ioctl (unsigned long fd, unsigned long cmd, void *arg);

extern unsigned long sys_nanosleep (struct timespec *req, struct timespec *rem);
extern unsigned long sys_reboot ();
extern void sleep_sec (int sec);

extern unsigned long str_len (char *s);
extern void str_print (char *s);
extern int str_eq (char *a, char *b);

extern void delay (int ticks);

extern long ps_fork ();
extern long ps_execve (char *filename, char **argv, char **envp);
extern int ps_execute (char *filename);

extern int str_pos (char *s, char c);
extern int str_to_int (char *s);
extern unsigned long hex_str_to_ulong (char *s);

extern int sys_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                      struct timeval *timeout);

extern void *mem_brk (void *p);
extern void *mem_alloc (int size);
extern void mem_set (void *p, char n, size_t size);
extern void mem_cpy (void *dest, void *src, size_t size);
extern void str_cpy (char *dest, char *src);
extern void *mem_mmap(void *addr, size_t length, int prot, int flags, int fd,
                      off_t offset);
extern int mem_munmap(void *addr, size_t length);
extern void *malloc(size_t size);
extern void free(void *addr);
extern void print_heap ();

extern int sys_waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
