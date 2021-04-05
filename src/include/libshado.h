#pragma once
#include <time.h>
#include <stdbool.h>

extern unsigned long _syscall (int num, void *a0, void *a1, void *a2, void *a3, void *a4, void *a5);

extern unsigned long file_open (char *fn, int flags);
extern unsigned long file_read (unsigned long fd, char *buff, unsigned long size);
extern unsigned long file_write (unsigned long fd, char *buf, unsigned long len);

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

extern void *mem_brk (void *p);
