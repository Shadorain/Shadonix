#include <syscall.h>
#include <fcntl.h>
#include "../include/libshado.h"

int main () {
    char *msg = "Shadonix 0.0.0.1 Initializing...\n";

    sleep_sec(1);
    str_print(msg);
    sleep_sec(1);

    char buff[255];
    char *fn = "/src/init.c";
    str_print("Opening file: ");
    str_print(fn);
    str_print("...\n");

    unsigned long fd = file_open(fn, O_RDONLY);
    file_read(fd, buff, sizeof(buff));

    str_print(buff);

    for (int t=0; t<3; t++) {
        sleep_sec(1);
        str_print(msg);
    }

    return 0;
}
