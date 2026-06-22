#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "raylib.h"
#include "document.h"

typedef struct {
    Camera2D camera;
    Vector2 mouseWorldPos;
    Vector2 localMousePos;
    int hoveredPage;
    bool isMouseInsideCanvas;
    bool isPanning;
} Viewport;

void InitViewport(Viewport* vp, float pageWidth);

// Handles Zooming and calculates screen-to-world / world-to-page coordinates
void UpdateViewportMath(Viewport* vp, Document* doc, Vector2 mousePos, bool guiClicked);

// Applies dragging math to the camera offset
void ApplyPanning(Viewport* vp);

#endif