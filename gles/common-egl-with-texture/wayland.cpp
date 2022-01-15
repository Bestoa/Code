#include <cstdio>
#include <cstring>

#include <EGL/egl.h>

#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-egl.h>
#include <linux/input.h>

bool gShouldStop = false;

wl_display* wlDisplay = NULL;
wl_registry* wlRegistry = NULL;
wl_compositor* wlCompositor = NULL;
wl_shell* wlShell = NULL;
wl_surface* wlSurface = NULL;
wl_shell_surface* wlShellSurface = NULL;

static void registerGlobalCallback(void* /*data*/, wl_registry* registry, uint32_t name, const char* interface, uint32_t /*version*/)
{
    if (strcmp(interface, "wl_compositor") == 0)
    {
        wlCompositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
    else if (strcmp(interface, "wl_shell") == 0)
    {
        wlShell = (wl_shell*)wl_registry_bind(registry, name, &wl_shell_interface, 1);
    }
}

static void globalObjectRemove(void* /*data*/, struct wl_registry* /*wl_registry*/, uint32_t /*name*/) { }

static const wl_registry_listener registryListener =
{
    registerGlobalCallback,
    globalObjectRemove
};

static void ping_cb(void* /*data*/, struct wl_shell_surface* shell_surface, uint32_t serial)
{
    wl_shell_surface_pong(shell_surface, serial);
}

static void configure_cb(void* /*data*/, struct wl_shell_surface* /*shell_surface*/, uint32_t /*edges*/, int32_t /*width*/, int32_t /*height*/) { }

static void popupDone_cb(void* /*data*/, struct wl_shell_surface* /*shell_surface*/) { }

static const struct wl_shell_surface_listener shellSurfaceListeners =
{
    ping_cb,
    configure_cb,
    popupDone_cb
};

bool initWaylandConnection()
{
    if ((wlDisplay = wl_display_connect(NULL)) == NULL)
    {
        printf("Failed to connect to Wayland display!\n");
        return false;
    }

    if ((wlRegistry = wl_display_get_registry(wlDisplay)) == NULL)
    {
        printf("Faield to get Wayland registry!\n");
        return false;
    }

    wl_registry_add_listener(wlRegistry, &registryListener, NULL);
    wl_display_dispatch(wlDisplay);
    if (!wlCompositor)
    {
        printf("Could not bind Wayland protocols!\n");
        return false;
    }

    return true;
}


bool nativeInit(int w, int h, EGLNativeDisplayType *eglNativeDisplay, EGLNativeWindowType *eglNativeWindow)
{
    if (!initWaylandConnection())
        return false;
    *eglNativeDisplay = (EGLNativeDisplayType)wlDisplay;

    wlSurface = wl_compositor_create_surface(wlCompositor);
    if (wlSurface == NULL)
    {
        printf("Failed to create Wayland surface\n");
        return false;
    }

    wlShellSurface = wl_shell_get_shell_surface(wlShell, wlSurface);
    if (wlShellSurface == NULL)
    {
        printf("Failed to get Wayland shell surface\n");
        return false;
    }

    wl_shell_surface_add_listener(wlShellSurface, &shellSurfaceListeners, NULL);
    wl_shell_surface_set_toplevel(wlShellSurface);

    *eglNativeWindow = (EGLNativeWindowType)wl_egl_window_create(wlSurface, w, h);
    if (*eglNativeWindow == (EGLNativeWindowType)EGL_NO_SURFACE) {
        printf("Can't create egl window\n");
        return false;
    }

    return true;
}

void nativeDestroy()
{
    if (wlShellSurface) wl_shell_surface_destroy(wlShellSurface);
    if (wlSurface) wl_surface_destroy(wlSurface);
    if (wlShell) wl_shell_destroy(wlShell);
    if (wlCompositor) wl_compositor_destroy(wlCompositor);
    if (wlRegistry) wl_registry_destroy(wlRegistry);
    if (wlDisplay) wl_display_disconnect(wlDisplay);
}

void pollEvent()
{
    wl_display_dispatch_pending(wlDisplay);
}

bool shouldStop()
{
    return gShouldStop;
}
