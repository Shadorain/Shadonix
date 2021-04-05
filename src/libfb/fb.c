#include "../include/libshado.h"
#include "../include/libfb.h"
#include <fcntl.h>
#include <errno.h>

int fb_get_info (struct fb_var_screeninfo *vi, struct fb_fix_screeninfo *fi) {
    int fd = file_open("/dev/fb0", O_RDONLY);
    if (fd < 0) {
        errno = fd;
        return FB_ERR_NOFB0;
    }
    int ret;
    if ((ret = file_ioctl(fd, FBIOGET_VSCREENINFO, vi)) < 0) {
        errno = ret;
        file_close(fd);
        return FB_ERR_GETINFO;
    }
    if ((ret = file_ioctl(fd, FBIOGET_FSCREENINFO, fi)) < 0) {
        errno = ret;
        file_close(fd);
        return FB_ERR_GETINFO;
    }
    file_close(fd);
    return FB_SUCCESS;
}

int fb_put_info (struct fb_var_screeninfo *vi) {
    int fd = file_open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        errno = fd;
        return FB_ERR_NOFB0;
    }

    int ret;
    if ((ret = file_ioctl(fd, FBIOPUT_VSCREENINFO, vi)) < 0) {
        errno = ret;
        file_close(fd);
        return FB_ERR_PUTINFO;
    }
    file_close(fd);
    return FB_SUCCESS;
}

void *fb_get_buffer () {
    struct fb_fix_screeninfo fi;
    int fd = file_open("/dev/fb0", O_RDWR);
    if (fd < 0) {
        errno = fd;
        return (void*)FB_ERR_NOFB0;
    }

    int ret;
    if ((ret = file_ioctl(fd, FBIOGET_FSCREENINFO, &fi)) < 0) {
        errno = ret;
        file_close(fd);
        return (void*)FB_ERR_GETINFO;
    }

    void *fb = mem_mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    file_close(fd);
    return fb;
}
