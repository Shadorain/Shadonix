#include "../include/libshado.h"
#include "../include/libdrm.h"
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>

#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

int libdrm_ioctl (int fd, unsigned long request, void *arg) {
    int ret;
    do {
        ret = file_ioctl(fd, request, arg);
    } while (ret == -EINTR || ret == -EAGAIN);

    return ret;
}

/* Opening device e.g: /dev/dri/by-path/pci-0000:00:02.0-card */
int libdrm_open (const char *device_node) {
    int fd = file_open((char*)device_node, O_RDWR | O_CLOEXEC);
    if (fd < 0) return fd;

    struct libdrm_get_cap get_cap = {
        .capability = DRM_CAP_DUMB_BUFFER,
        .value = 0,
    };
    if (libdrm_ioctl(fd, DRM_IOCTL_GET_CAP, &get_cap) < 0 || !get_cap.value)
        return -EOPNOTSUPP;
    return fd;
}

int libdrm_get_resources (int fd, struct drm_mode_card_res *res) {
    mem_set(res, 0, sizeof(struct drm_mode_card_res));
    int ior = 0;
    if (libdrm_ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, res))
        return -1;

    if (res->count_fbs) {
        res->fb_id_ptr = (uint64_t)malloc(res->count_fbs * sizeof(uint32_t));
        mem_set((void*)res->fb_id_ptr, 0, res->count_fbs * sizeof(uint32_t));
    }
    if (res->count_crtcs) {
        res->crtc_id_ptr = (uint64_t)malloc(res->count_crtcs * sizeof(uint32_t));
        mem_set((void*)res->crtc_id_ptr, 0, res->count_crtcs * sizeof(uint32_t));
    }
    if (res->count_connectors) {
        res->connector_id_ptr = (uint64_t)malloc(res->count_connectors * sizeof(uint32_t));
        mem_set((void*)res->connector_id_ptr, 0, res->count_connectors * sizeof(uint32_t));
    }
    if (res->count_encoders) {
        res->encoder_id_ptr = (uint64_t)malloc(res->count_encoders * sizeof(uint32_t));
        mem_set((void*)res->encoder_id_ptr, 0, res->count_encoders * sizeof(uint32_t));
    }

    ior = libdrm_ioctl(fd, DRM_IOCTL_MODE_GETRESOURCES, res);
    if (ior) return -1;

    return 0;
}

int libdrm_get_connector (int fd, int id, struct drm_mode_get_connector *conn) {
    mem_set(conn, 0, sizeof(struct drm_mode_get_connector));
    conn->connector_id = id;
    if (libdrm_ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, conn))
        return -1;

    if (conn->count_props) {
        conn->props_ptr = (uint64_t)malloc(conn->count_props * sizeof(uint32_t));
        conn->prop_values_ptr = (uint64_t)malloc(conn->count_props * sizeof(uint64_t));
    }

    if (conn->count_modes)
        conn->modes_ptr = (uint64_t)malloc(conn->count_modes * sizeof(struct drm_mode_modeinfo));
    if (conn->count_encoders)
        conn->encoders_ptr = (uint64_t)malloc(conn->count_encoders * sizeof(uint32_t));

    if (libdrm_ioctl(fd, DRM_IOCTL_MODE_GETCONNECTOR, conn))
        return -1;
    return 0;
}

int libdrm_get_encoder(int fd, int id, struct drm_mode_get_encoder *enc) {
    mem_set(enc, 0, sizeof(struct drm_mode_get_encoder));
    enc->encoder_id = id;
    if (libdrm_ioctl(fd, DRM_IOCTL_MODE_GETENCODER, enc))
        return -1;
    return 0;
}

int libdrm_handle_event(int fd, struct libdrm_event_context *context) {
    char buf[1024];
    struct drm_event *e;

    int len = file_read(fd, buf, sizeof(buf));
    if (!len) return 0;
    if (len < sizeof(struct drm_event)) return -1;

    int i=0;
    while (i < len) {
        e = (struct drm_event*)&buf[i];
        i += e->length;

        switch (e->type) {
            case DRM_EVENT_FLIP_COMPLETE: {
                struct drm_event_vblank *vb = (struct drm_event_vblank*)e;
                context->page_flip_handler(fd, vb->sequence, vb->tv_sec, vb->tv_usec, (void*)vb->user_data);
            } break;
        }
    }
    return 0;
}
