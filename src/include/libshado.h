#pragma once

extern unsigned long _syscall (int num, void *a0, void *a1, void *a2, void *a3, void *a4, void *a5);

extern unsigned long file_open (char *fn, int flags);
extern unsigned long file_read (unsigned long fd, char *buff, unsigned long size);
extern unsigned long sys_reboot ();
void sleep_sec (int sec);

extern unsigned long str_len (char *s);
extern void str_print (char *s);

extern void delay (int ticks);
