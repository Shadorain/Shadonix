#include "../include/libshado.h"
#include "../include/printf.h"
#include <fcntl.h>

unsigned long console_fd = 0;

void console_open () {
    console_fd = file_open("/dev/console", O_RDWR | O_NDELAY);
}

char console_read () {
    char c = 0;
    while (!file_read(console_fd, &c, 1)) {
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10000;
        sys_nanosleep(&ts, NULL);
    }
    return c;
}

void console_write (char c) {
    file_write(console_fd, &c, 1);
}

int line_read (char *buf, int max) {
    int i=0;

    for (; i<max; i++) {
        char c = console_read();
        if (c == 0) {
            i--;
            continue;
        }
        console_write(c);
        buf[i] = c;
        if (c == '\b') {
            i--;
            buf[i] = 0;
        }

        if (c == '\n') {
            buf[i] = 0;
            return i;
        }
    }
    return i;
}

unsigned long cur_brk = 0;

void proc_cmd (char *cmd) {
    int end = str_pos(cmd, ' ');
    char *arg = 0;
    if (end != -1) {
        cmd[end] = 0;
        arg = cmd + end + 1;
    }

    if (str_eq(cmd, "reboot")) {
        str_print("\n\n*** REBOOTING ***\n");
        sys_reboot();
    }

    if (str_eq(cmd, "brk")) {
        int size = 0;
        if (arg) size = str_to_int(arg);

        void *new_val = (void*)(cur_brk + size);
        void *addr = mem_brk(new_val);
        printf("BRK(%X) = %X\n", new_val, addr);

        cur_brk = (unsigned long)mem_brk(0);
    }

    if (str_eq(cmd, "store")) {
        end = str_pos(arg, ' ');
        arg[end] = 0;

        char *val = arg + end + 1;
        unsigned long addr = hex_str_to_ulong(arg);
        int n = str_to_int(val);
        printf("Storing %d at %X\n", n , addr); 

        int *p = (int*)addr;
        *p = n;
    }

    if (str_eq(cmd, "fetch")) {
        unsigned long addr = hex_str_to_ulong(arg);
        int *p = (int*)addr;

        printf("Fetched %d from %X\n", *p, addr);
    }
}

int main () {
    str_print("\033[H\033[J");
    str_print("Sh v0.0.0.2\n");
    cur_brk = (long)mem_brk(0);
    printf("BRK: %X\n", cur_brk);
    str_print("|=-> ");

    console_open();
    while (1) {
        char buf[1024];
        line_read(buf, sizeof(buf));
        proc_cmd(buf);

        str_print ("|=-> ");
    }
}
