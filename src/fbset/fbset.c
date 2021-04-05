#include "../include/libshado.h"
#include "../include/libfb.h"
#include "../include/printf.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>

int fill_fb (void *pbuffer, struct fb_var_screeninfo *vi, struct fb_fix_screeninfo *fi, uint32_t color) {
    for (int x=0; x<vi->xres; x++) {
        for (int y=0; y<vi->yres; y++) {
            /* Get location of specific pixel at x and y coord */
            int location = (x+vi->xoffset) * (vi->bits_per_pixel/8)
                + (y+vi->yoffset) * (fi->line_length);
            uint32_t *pb = (uint32_t*)(pbuffer+location);
            *pb = color;
        }
    }
}

int main (int argc, char **argv) {
    printf("Frame buffer setting...\n");
    if (argc < 3) {
        printf("Usage: fbset <xres> <yres>");
        return -1;
    }

    int fdcon = file_open("/dev/console", O_RDWR);
    struct fb_var_screeninfo vi;
    struct fb_fix_screeninfo fi;

    fb_get_info(&vi, &fi);
    int xres = str_to_int(argv[1]);
    int yres = str_to_int(argv[2]);
    printf("Current resolution: %dx%d", vi.xres, vi.yres);
    printf("Setting resolution: %dx%d", xres, yres);

    vi.xres = vi.xres_virtual = xres;
    vi.yres = vi.yres_virtual = yres;

    if (fb_put_info(&vi) != FB_SUCCESS) {
        printf("FAILED TO SET BUFFER: %d\n", errno);
        return -1;
    }
    void *pbuffer = fb_get_buffer();
    if (fb_get_info(&vi, &fi) != FB_SUCCESS) {
        printf("FAILED TO GET VIDEO INFO: %d\n", errno);
        return -1;
    }

    file_ioctl(fdcon, KDSETMODE, (void*)KD_GRAPHICS);
    sleep_sec(2);
    fill_fb(pbuffer, &vi, &fi, 0xFFFFFFFF); /* white */
    sleep_sec(2);
    fill_fb(pbuffer, &vi, &fi, 0xFFFF0000); /* red */
    sleep_sec(2);
    fill_fb(pbuffer, &vi, &fi, 0xFF00FF00); /* green */
    sleep_sec(2);
    fill_fb(pbuffer, &vi, &fi, 0xFF0000FF); /* blue */
    file_ioctl(fdcon, KDSETMODE, (void*)KD_TEXT);
}
