#include <cstdio>
#include <cstring>

#include <EGL/egl.h>

#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-egl.h>
#include <linux/input.h>

#include "xdg-shell-client-protocol.h"

bool gShouldStop = false;

wl_display* wlDisplay = nullptr;
wl_registry* wlRegistry = nullptr;
wl_compositor* wlCompositor = nullptr;
xdg_wm_base* xdgWmBase = nullptr;
xdg_surface* xdgSurface = nullptr;
wl_surface* wlSurface = nullptr;
xdg_toplevel* xdgToplevel = nullptr;
bool wait_for_configure = true;

int windowWidth = 0;
int windowHeight = 0;

void xdg_surface_handle_configure(void * /*data*/, struct xdg_surface *xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);
    wait_for_configure = false;
}

const struct xdg_surface_listener xdg_surface_listener = {
    xdg_surface_handle_configure,
};

void xdg_wm_base_handle_ping(void * /*data*/, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
    xdg_wm_base_pong(xdg_wm_base, serial);
}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    xdg_wm_base_handle_ping,
};

void xdg_toplevel_handle_configure(void * /*data*/, struct xdg_toplevel * /*xdg_toplevel*/, int32_t width, int32_t height, struct wl_array * /*states*/)
{
    fprintf(stderr, "configure event: width %d, height %d\n", width, height);
    if (width != 0 && height !=0)
    {
        windowWidth = width;
        windowHeight = height;
    }
}

const struct xdg_toplevel_listener xdg_toplevel_listener = {
    xdg_toplevel_handle_configure,
    nullptr,
    nullptr,
    nullptr,
};

static void registerGlobalCallback(void* /*data*/, wl_registry* registry, uint32_t name, const char* interface, uint32_t /*version*/)
{
    if (strcmp(interface, "wl_compositor") == 0)
    {
        wlCompositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        xdgWmBase = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(xdgWmBase, &xdg_wm_base_listener, nullptr);
    }
}

static void globalObjectRemove(void* /*data*/, struct wl_registry* /*wl_registry*/, uint32_t /*name*/) { }

static const wl_registry_listener registryListener =
{
    registerGlobalCallback,
    globalObjectRemove
};

bool initWaylandConnection()
{
    if ((wlDisplay = wl_display_connect(nullptr)) == nullptr)
    {
        printf("Failed to connect to Wayland display!\n");
        return false;
    }

    if ((wlRegistry = wl_display_get_registry(wlDisplay)) == nullptr)
    {
        printf("Faield to get Wayland registry!\n");
        return false;
    }

    wl_registry_add_listener(wlRegistry, &registryListener, nullptr);
    wl_display_roundtrip(wlDisplay);
    if (!wlCompositor)
    {
        printf("Could not bind Wayland protocols!\n");
        return false;
    }

    return true;
}


bool nativeInit(int w, int h, EGLNativeDisplayType *eglNativeDisplay, EGLNativeWindowType *eglNativeWindow, bool fullscreen)
{
    if (!initWaylandConnection())
        return false;
    *eglNativeDisplay = (EGLNativeDisplayType)wlDisplay;

    wlSurface = wl_compositor_create_surface(wlCompositor);
    if (wlSurface == nullptr)
    {
        printf("Failed to create Wayland surface\n");
        return false;
    }

    xdgSurface = xdg_wm_base_get_xdg_surface(xdgWmBase, wlSurface);
    if (xdgSurface == nullptr)
    {
        printf("Failed to get Wayland shell surface\n");
        return false;
    }
    xdg_surface_add_listener(xdgSurface, &xdg_surface_listener, nullptr);

    xdgToplevel = xdg_surface_get_toplevel(xdgSurface);
    xdg_toplevel_set_title(xdgToplevel, "simple-egl");
    xdg_toplevel_add_listener(xdgToplevel, &xdg_toplevel_listener, nullptr);

    if (fullscreen)
        xdg_toplevel_set_fullscreen(xdgToplevel, nullptr);
    wl_surface_commit(wlSurface);

    while (wait_for_configure)
        wl_display_roundtrip(wlDisplay);

    if (windowWidth != 0 && windowHeight != 0)
    {
        w = windowWidth;
        h = windowHeight;
    }

    *eglNativeWindow = (EGLNativeWindowType)wl_egl_window_create(wlSurface, w, h);
    if (*eglNativeWindow == (EGLNativeWindowType)EGL_NO_SURFACE) {
        printf("Can't create egl window\n");
        return false;
    }

    return true;
}

void nativeDestroy()
{
    if (xdgSurface) xdg_surface_destroy(xdgSurface);
    if (wlSurface) wl_surface_destroy(wlSurface);
    if (xdgWmBase) xdg_wm_base_destroy(xdgWmBase);
    if (wlCompositor) wl_compositor_destroy(wlCompositor);
    if (wlRegistry) wl_registry_destroy(wlRegistry);
    if (wlDisplay) wl_display_disconnect(wlDisplay);
}

void pollEvent()
{
    if (wl_display_dispatch_pending(wlDisplay) == -1)
        gShouldStop = true;
}

bool shouldStop()
{
    return gShouldStop;
}
