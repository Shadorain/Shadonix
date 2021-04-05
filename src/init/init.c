#include <syscall.h>
#include <fcntl.h>
#include "../include/libshado.h"
#include "../include/libfb.h"
#include <linux/vt.h>

uint32_t fix_color (uint32_t num) {
    return (num & 0xFF00FF00 | ((num&0xFF)<<16) | ((num >> 16) & 0xFF));
}

int main () {
    /* char *msg = "Shadonix 0.0.0.2 Initializing...\n"; */
    /* sleep_sec(1); */
    /* str_print(msg); */
    int fdcon = file_open("/dev/console", O_RDWR);
    file_ioctl(fdcon, KDSETMODE, (void*)KD_GRAPHICS);

    struct fb_var_screeninfo vi;
    struct fb_fix_screeninfo fi;
    fb_get_info(&vi, &fi);
    vi.xres = vi.xres_virtual = 1024;
    vi.yres = vi.yres_virtual = 768;

    fb_put_info(&vi);
    void *pbuffer = fb_get_buffer();
    int fd = file_open("/etc/shadonix.png", O_RDONLY);
    int buf_size = sizeof(int) * 1024 * 768;
    int *buf = malloc(buf_size);
    file_read(fd, (char*)buf, buf_size);
    file_close(fd);

    int i = 0;
    for (int y=0; y<vi.yres; y++) {
        for (int x=0; x<vi.xres; x++) {
            int location = (x+vi.xoffset) * (vi.bits_per_pixel/8) + (y + vi.yoffset)
                * fi.line_length;
            uint32_t *pb = (uint32_t*)(pbuffer+location);
            *pb = fix_color(buf[i]);
            i++;
        }
    }

    sleep_sec(5);
    file_ioctl(fdcon, KDSETMODE, (void*)KD_TEXT);

    ps_execute("/bin/lash");

    while (1) {
        sleep_sec(1);
    }

    return 0;
}
