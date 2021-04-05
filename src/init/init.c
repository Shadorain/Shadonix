#include <syscall.h>
#include <fcntl.h>
#include "../include/libshado.h"

int main () {
    char *msg = "Shadonix 0.0.0.2 Initializing...\n";
    sleep_sec(1);
    str_print(msg);
    sleep_sec(1);

    ps_execute("/bin/lash");

    while (1) {
        sleep_sec(1);
    }

    return 0;
}
