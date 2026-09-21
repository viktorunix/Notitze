#ifndef TABLET_WAYLAND_H
#define TABLET_WAYLAND_H

#include <stdbool.h>

typedef struct {
    float x, y;
    float pressure;
    float tiltX, tiltY;
    bool isDown;
    bool isHovering;
    bool isActive;
} WaylandTabletState;

extern WaylandTabletState tabletState;

void InitWaylandTablet(void);
void PollWaylandTablet(void);
void CloseWaylandTablet(void);
#endif
