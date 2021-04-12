#include "../include/libshado.h"
#include "../include/libdrm.h"
#include "../include/printf.h"
#include <fcntl.h>
#include <sys/select.h>

struct libdrm_buf {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t size;
    uint32_t handle;
    uint8_t *map;
    uint32_t fb;
};

struct libdrm_data {
    int fd;
    struct libdrm_buf framebuffer[2];
    uint32_t crt_id;
    bool pflip_pending;
    bool cleanup;
    int front_buf;
    uint32_t width;
    uint32_t height;
};

struct mouse_pos_info {
    int x;
    int y;
    int max_x;
    int max_y;
};

struct mouse_pos_info mouse_pos;
struct drm_mode_crtc saved_crtc;

int set_mode (struct libdrm_data *data, struct drm_mode_get_connector conn, struct drm_mode_modeinfo mode);

int main (int argc, char **argv) {
    printf("DRM Modes:\n");
    int fd = libdrm_open("/dev/dri/card0");
    struct drm_mode_card_res res;
    if (libdrm_get_resources(fd, &res)) {
        printf("Failed to open card0 resources\n");
        return -1;
    }

    int hres = 0;
    int vres = 0;
    if (argc == 3) {
        hres = str_to_int(argv[1]);
        vres = str_to_int(argv[2]);
        printf("Attempting to set res: %dx%d", hres, vres);
    }

    struct libdrm_data data;
    data.cleanup = false;
    data.pflip_pending = false;
    data.front_buf = 0;
    data.width = hres;
    data.height = vres;
    data.fd = fd;

    file_ioctl(fd, DRM_IOCTL_SET_MASTER, 0);

    printf("DRM Connectors: %d\n", res.count_connectors);
    sleep_sec(1);

    for (int i=0; i < res.count_connectors; i++) {
        uint32_t *connectors = (uint32_t*)res.connector_id_ptr;
        struct drm_mode_get_connector conn;

        int ret = libdrm_get_connector(fd, connectors[i], &conn);
        if (ret) { 
            printf("\tFailed to get connector: %d\n", ret);
            continue;
        }
        //printf("Found Connector: %d - %d, Modes %d\n", i, connectors[i], conn.count_modes);
        if (conn.connection != DRM_MODE_CONNECTED) {
            //printf("\tIgnoring unconnected connector. (%d)\n", conn.connection);
            continue;
        }

        struct drm_mode_modeinfo *modes = (struct drm_mode_modeinfo*)conn.modes_ptr;
        for (int m=0; m < conn.count_modes; m++) {
            printf("\tMode: %dx%d\n", modes[m].hdisplay, modes[m].vdisplay);
            //sleep_sec(1);

            if (hres == modes[m].hdisplay && vres == modes[m].vdisplay)
                return set_mode(&data, conn, modes[m]);
        }
    }
    file_ioctl(fd, DRM_IOCTL_DROP_MASTER, 0);
    return 0;
}

bool create_framebuffer (int fd, struct libdrm_buf *buf) {
    struct drm_mode_create_dumb creq;
    struct drm_mode_create_dumb dreq;
    struct drm_mode_map_dumb mreq;

    mem_set(&creq, 0, sizeof(creq));
    creq.width = buf->width;
    creq.height = buf->height;
    creq.bpp = 32; /* bits per pixel */

    int ret = libdrm_ioctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret < 0) {
        printf("Failed to create buffer: %d\n", ret);
        return false;
    }
    buf->stride = creq.pitch;
    buf->size = creq.size;
    buf->handle = creq.handle;

    struct drm_mode_fb_cmd fbcmd;
    mem_set(&fbcmd, 0, sizeof(fbcmd));
    fbcmd.width = buf->width;
    fbcmd.height = buf->height;
    fbcmd.depth = 24;
    fbcmd.bpp = 32;
    fbcmd.handle = buf->handle;
    ret = libdrm_ioctl(fd, DRM_IOCTL_MODE_ADDFB, &fbcmd);
    if (ret) {
        printf("Failed to add FB: %d\n", ret);
        return false;
    }
    buf->fb = fbcmd.fb_id;

    mem_set(&mreq, 0, sizeof(mreq));
    mreq.handle = buf->handle;
    ret = libdrm_ioctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (ret) {
        printf("Failed to map FB: %d\n", ret);
        return false;
    }
    buf->map = mem_mmap(0, buf->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
    if (((int64_t)buf->map) == -1) {
        printf("Failed to map FB!\n");
        return false;
    }

    mem_set(buf->map, 0, buf->size);
    return true;
}

int cursor_size = 20;
uint32_t bg_color = 0xFF999999;
uint32_t cursor_color = 0xFF0000FF;

/* Mouse buttons */
bool left_down = false;
bool right_down = false;

static void draw_data (int fd, struct libdrm_data *data) {
    struct libdrm_buf *buf = &data->framebuffer[data->front_buf ^ 1];
    int start_x = mouse_pos.x;
    int start_y = mouse_pos.y;

    uint32_t *p = (uint32_t*)buf->map;
    mem_set(p, bg_color, buf->size); /* clears bg */
    for (int x=0; x < cursor_size; x++) {
        for (int y=0; y < cursor_size; y++) {
            int pos = (start_x+x) + ((start_y+y) * data->width);
            if (pos * 4 >= buf->size) break;

            uint32_t color = cursor_color;
            if (left_down) color |= 0x00FF0000; /* red */
            if (right_down) color |= 0x0000FF00; /* green */

            p[pos] = color;
        }
    }

    struct drm_mode_crtc_page_flip flip;
    flip.fb_id = buf->fb;
    flip.crtc_id = data->crt_id;
    flip.user_data = (uint64_t)data;
    flip.flags = DRM_MODE_PAGE_FLIP_EVENT;
    flip.reserved = 0;

    int ret = libdrm_ioctl(fd, DRM_IOCTL_MODE_PAGE_FLIP, &flip);
    if (!ret) {
        data->pflip_pending = true;
        data->front_buf ^= 1;
    } else printf("Failed to flip: %d\n", ret);
}

static void page_flip_event (int fd, uint32_t frame, uint32_t sec, uint32_t usec, void *data) {
    struct libdrm_data *dev = data;
    dev->pflip_pending = false;
    if (!dev->cleanup) draw_data(fd, dev);
}

int set_mode (struct libdrm_data *data, struct drm_mode_get_connector conn, struct drm_mode_modeinfo mode) {
    if (!conn.encoder_id) {
        printf("No encoder found!\n");
        return -1;
    }

    struct drm_mode_get_encoder enc;
    if (libdrm_get_encoder(data->fd, conn.encoder_id, &enc)) {
        printf("Encoder load failed\n");
        return -1;
    }
    if (!enc.crtc_id) {
        printf("No CRT Controller!\n");
        return -1;
    }

    data->framebuffer[0].width = mode.hdisplay;
    data->framebuffer[0].height = mode.vdisplay;
    data->framebuffer[1].width = mode.hdisplay;
    data->framebuffer[1].height = mode.vdisplay;

    mouse_pos.x = 0;
    mouse_pos.y = 0;
    mouse_pos.max_x = mode.hdisplay;
    mouse_pos.max_y = mode.vdisplay;
    
    if (!create_framebuffer(data->fd, &data->framebuffer[0])) {
        printf("Failed to create framebuffer 1!\n");
        return -1;
    }
    if (!create_framebuffer(data->fd, &data->framebuffer[1])) {
        printf("Failed to create framebuffer 2!\n");
        return -1;
    }
    printf("Buffer created with size: %d\n", data->framebuffer[0].size);

    struct drm_mode_crtc crtc;
    mem_set(&crtc, 0, sizeof(crtc));
    crtc.crtc_id = enc.crtc_id;
    data->crt_id = enc.crtc_id;

    /* get current crtc, should be fb controlled */
    int ret = libdrm_ioctl(data->fd, DRM_IOCTL_MODE_GETCRTC, &crtc);
    saved_crtc = crtc;
    printf("Get CRTC: %d = %d (%d, %d, %x, %s)\n", crtc.crtc_id, ret, crtc.fb_id,
            crtc.count_connectors, crtc.set_connectors_ptr, crtc.mode.name);
    mem_set(&crtc, 0, sizeof(crtc));
    crtc.crtc_id = enc.crtc_id;

    mem_cpy(&crtc.mode, &mode, sizeof(mode));
    crtc.x = 0;
    crtc.y = 0;
    crtc.fb_id = data->framebuffer[0].fb;
    crtc.count_connectors = 1;
    crtc.set_connectors_ptr = (uint64_t)&conn.connector_id;
    crtc.mode_valid = 1;

    int mouse_fd = file_open("/dev/input/mice", O_RDONLY);
    printf("CRTC Res: %d/%d...\n", crtc.mode.hdisplay, crtc.mode.vdisplay);
    sleep_sec(4);
    
    ret = libdrm_ioctl(data->fd, DRM_IOCTL_MODE_SETCRTC, &crtc);
    if (ret) {
        printf("Failed to set CRTC: %d\n", ret);
        return ret;
    }

    sleep_sec(1);
    draw_data (data->fd, data);

    fd_set fds;
    FD_ZERO(&fds);
    struct libdrm_event_context ev;
    mem_set(&ev, 9, sizeof(ev));
    ev.version = 2;
    ev.page_flip_handler = page_flip_event;

    while (true) {
        FD_SET(0, &fds);
        FD_SET(data->fd, &fds);
        FD_SET(mouse_fd, &fds);
        ret = sys_select(mouse_fd+1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            printf("Select failed: %d\n", ret);
            break;
        }
        if (FD_ISSET(0, &fds)) break; /* user pressed key */
        if (FD_ISSET(data->fd, &fds))
            /* drawing happend on buffer */
            libdrm_handle_event(data->fd, &ev);
        if (FD_ISSET(mouse_fd, &fds)) {
            /* mouse event */
            char buf[3];
            int r = file_read(mouse_fd, buf, 3);
            mouse_pos.x += buf[1];
            mouse_pos.y -= buf[2];
            left_down = buf[0] & 1;
            right_down = buf[0] & 2;

            if (mouse_pos.x > mouse_pos.max_x - cursor_size)
                mouse_pos.x = mouse_pos.max_x - cursor_size;
            if (mouse_pos.y > mouse_pos.max_y - cursor_size)
                mouse_pos.y = mouse_pos.max_y - cursor_size;
            printf("MOUSE: %d, %d, %d\n", buf[0], buf[1], buf[2]);
        }
    }
    /* after loop, restore origin CRTC */
    saved_crtc.count_connectors = 1;
    saved_crtc.mode_valid = 1;
    saved_crtc.set_connectors_ptr = (uint64_t)&conn.connector_id;

    ret = libdrm_ioctl(data->fd, DRM_IOCTL_MODE_SETCRTC, &saved_crtc);
    file_ioctl(data->fd, DRM_IOCTL_DROP_MASTER, 0);
    file_close(data->fd);
    printf("Done\n");

    return 0;
}
