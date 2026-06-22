#include "include/viewport.h"
#include "include/raymath.h"

#define PAGE_GAP 60

void InitViewport(Viewport* vp, float pageWidth) {
    vp->camera = (Camera2D){0};
    vp->camera.target = (Vector2){0.0f, 0.0f};
    vp->camera.offset = (Vector2){ (GetScreenWidth() - pageWidth) / 2.0f, 50.0f};
    vp->camera.rotation = 0.0f;
    vp->camera.zoom = 1.0f;
    vp->isPanning = false;
}

void UpdateViewportMath(Viewport* vp, Document* doc, Vector2 mousePos, bool guiClicked) {
    vp->mouseWorldPos = GetScreenToWorld2D(mousePos, vp->camera);

    // 1. Handle Zoom
    float wheel = GetMouseWheelMove();
    if(wheel != 0 && !guiClicked){
        vp->camera.offset = mousePos;
        vp->camera.target = vp->mouseWorldPos;
        vp->camera.zoom += (wheel * 0.125f);
        if(vp->camera.zoom < 0.25f) vp->camera.zoom = 0.25f;
        if(vp->camera.zoom > 5.0f) vp->camera.zoom = 5.0f;
        
        // Re-calculate world pos after zoom is applied
        vp->mouseWorldPos = GetScreenToWorld2D(mousePos, vp->camera);
    }

    // 2. Calculate Page Mapping (Panning Math)
    vp->hoveredPage = (int)(vp->mouseWorldPos.y / (doc->pageHeight + PAGE_GAP));
    if(vp->mouseWorldPos.y < 0) vp->hoveredPage = -1;
    
    float localMouseY = vp->mouseWorldPos.y - (vp->hoveredPage * (doc->pageHeight + PAGE_GAP));
    vp->localMousePos = (Vector2){vp->mouseWorldPos.x, localMouseY};

    // 3. Determine if cursor is hitting the active canvas area
    vp->isMouseInsideCanvas = (vp->hoveredPage >= 0 && vp->hoveredPage < doc->pageCount &&
                               vp->localMousePos.x >= 0 && vp->localMousePos.x <= doc->pageWidth &&
                               localMouseY >= 0 && localMouseY <= doc->pageHeight);
}

void ApplyPanning(Viewport* vp) {
    if(vp->isPanning){
        Vector2 delta = GetMouseDelta();
        vp->camera.offset.x += delta.x;
        vp->camera.offset.y += delta.y;
    }
}