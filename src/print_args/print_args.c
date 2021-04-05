#include "../include/libshado.h"
#include "../include/printf.h"

int main (int argc, char **argv) {
    printf ("Print Args: %d\n", argc);
    for (int i=1; i<argc; i++) {
        printf("\t%d = %s\n", i, argv[i]);
    }
    return 0;
}
