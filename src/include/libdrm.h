#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <libdrm/drm.h>
#include <libdrm/drm_mode.h>

struct libdrm_get_cap {
    uint64_t capability;
    uint64_t value;
};

enum libdrm_modes {
    DRM_MODE_CONNECTED = 1,
    DRM_MODE_DISCONNECTED = 2,
    DRM_MODE_UNKNOWN = 3,
};

struct libdrm_event_context {
    int version;
    void (*vblank_handler)(int fd, uint32_t sequence, uint32_t tv_sec, uint32_t tv_usec, void *user_data);
    void (*page_flip_handler)(int fd, uint32_t sequence, uint32_t tv_sec, uint32_t tv_usec, void *user_data);
};

extern int libdrm_ioctl (int fd, unsigned long request, void *arg);
extern int libdrm_open (const char *device_node);
extern int libdrm_get_resources (int fd, struct drm_mode_card_res *res);
extern int libdrm_get_connector (int fd, int id, struct drm_mode_get_connector *conn);

extern int libdrm_get_encoder(int fd, int id, struct drm_mode_get_encoder *enc);
extern int libdrm_handle_event(int fd, struct libdrm_event_context *context);
