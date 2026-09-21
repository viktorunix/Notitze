#include "include/tablet_wayland.h"
#include "include/tablet-unstable-v2.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-util.h>

#include "include/raylib.h"

struct wl_display* glfwGetWaylandDisplay(void);

WaylandTabletState tabletState = {0};
static struct zwp_tablet_manager_v2 *tablet_manager = NULL;
static struct wl_display *display = NULL;
static struct wl_seat *wayland_seat = NULL;

static void tool_type(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t type) {}
static void tool_hardware_serial(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t hi, uint32_t lo) {}
static void tool_hardware_id_wacom(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t hi, uint32_t lo) {}
static void tool_capability(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t cap){}
static void tool_done(void *data, struct zwp_tablet_tool_v2 *tool){}
static void tool_removed(void *data, struct zwp_tablet_tool_v2 *tool) {}

static void tool_proximity_in(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t serial, struct zwp_tablet_v2 *tablet, struct wl_surface *surface){
    printf("DEBUG: Pen entered window proximity!\n");
    tabletState.isHovering = true;
    tabletState.isActive = true;
}

static void tool_motion(void *data, struct zwp_tablet_tool_v2 *tool, wl_fixed_t x, wl_fixed_t y){
    tabletState.x = wl_fixed_to_double(x);
    tabletState.y = wl_fixed_to_double(y);
    printf("Motion: %f, %f\n", tabletState.x, tabletState.y);
}

static void tool_proximity_out(void *data, struct zwp_tablet_tool_v2 *tool){
    tabletState.isHovering = false;
    tabletState.isDown = false;
}
static void tool_down(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t serial){
    tabletState.isDown = true;
}
static void tool_up(void *data, struct zwp_tablet_tool_v2 *tool){
    tabletState.isDown = false;
}

static void tool_pressure(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t pressure){
    tabletState.pressure = (float)pressure / 65535.0f;

}
static void tool_distance(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t distance) {}

static void tool_tilt(void *data, struct zwp_tablet_tool_v2 *tool, wl_fixed_t tilt_x, wl_fixed_t tilt_y){
    tabletState.tiltX = wl_fixed_to_double(tilt_x);
    tabletState.tiltY = wl_fixed_to_double(tilt_y);
}

static void tool_rotation(void *data, struct zwp_tablet_tool_v2 *tool, wl_fixed_t rotation) {}
static void tool_slider(void *data, struct zwp_tablet_tool_v2 *tool, int32_t position) {}
static void tool_wheel(void *data, struct zwp_tablet_tool_v2 *tool, wl_fixed_t degrees, int32_t clicks) {}
static void tool_button(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t serial, uint32_t button, uint32_t state) {}
static void tool_frame(void *data, struct zwp_tablet_tool_v2 *tool, uint32_t time) {}

static const struct zwp_tablet_tool_v2_listener tool_listener = {
    .type = tool_type,
    .hardware_serial = tool_hardware_serial,
    .hardware_id_wacom = tool_hardware_id_wacom,
    .capability = tool_capability,
    .done = tool_done,
    .removed = tool_removed,
    .proximity_in = tool_proximity_in,
    .proximity_out = tool_proximity_out,
    .down = tool_down,
    .up = tool_up,
    .motion = tool_motion,
    .pressure = tool_pressure,
    .distance = tool_distance,
    .tilt = tool_tilt,
    .rotation = tool_rotation,
    .slider = tool_slider,
    .wheel = tool_wheel,
    .button = tool_button,
    .frame = tool_frame
};

static void seat_tablet_added(void *data, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, struct zwp_tablet_v2 *id){}
static void seat_tool_added(void *data, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, struct zwp_tablet_tool_v2 *id){
    printf("DEBUG: Tool Added! Attaching listener...\n");
    zwp_tablet_tool_v2_add_listener(id, &tool_listener, NULL);
}
static void seat_pad_added(void *data, struct zwp_tablet_seat_v2 *zwp_tablet_seat_v2, struct zwp_tablet_pad_v2 *id){}

static const struct zwp_tablet_seat_v2_listener tablet_seat_listener = {
    .tablet_added = seat_tablet_added,
    .tool_added = seat_tool_added,
    .pad_added = seat_pad_added
};

static void registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version){
    if(strcmp(interface, "zwp_tablet_manager_v2") == 0){
        tablet_manager = wl_registry_bind(registry, id, &zwp_tablet_manager_v2_interface, 1);
    }
    else if(strcmp(interface, "wl_seat") == 0){
        wayland_seat = wl_registry_bind(registry, id, &wl_seat_interface, 1);
    }
}
static void registry_remover(void *data, struct wl_registry *registry, uint32_t id) {}
static const struct wl_registry_listener registry_listener = {registry_handler, registry_remover};


void InitWaylandTablet(void){
    display = glfwGetWaylandDisplay();
    if(!display){
        printf("NO NATIVE WAYLAND DISPLAY FOUND\n");
        return;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    // 1st Roundtrip: Get the globals (tablet_manager and wayland_seat)
    wl_display_roundtrip(display);

    if(tablet_manager && wayland_seat){
        printf("SUCCESS: Tablet Manager and Seat found!\n");
        struct zwp_tablet_seat_v2 *tablet_seat = zwp_tablet_manager_v2_get_tablet_seat(tablet_manager, wayland_seat);
        zwp_tablet_seat_v2_add_listener(tablet_seat, &tablet_seat_listener, NULL);

        // 2nd Roundtrip (CRITICAL): Force the compositor to send seat_tool_added events NOW
        wl_display_roundtrip(display);
    } else {
        printf("ERROR: zwp_tablet_manager_v2 or wl_seat NOT FOUND in registry!\n");
    }
}
void PollWaylandTablet(){
    if(display){
        wl_display_dispatch_pending(display);
    }
}
void CloseWaylandTablet(void) {
    if (tablet_manager) {
        zwp_tablet_manager_v2_destroy(tablet_manager);
        tablet_manager = NULL;
    }
}
