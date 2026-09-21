#include "include/input_manager.h"
#include "include/brush_system.h"
#include "include/doc_management.h"
#include "include/raymath.h"
#include "include/tablet_wayland.h"

#define PAGE_GAP 60

extern Stroke currentStroke;

// Track the previous frame state to detect exact press/release moments
static bool previousTabletDown = false;

void ProcessInputs(Document* doc, Viewport* vp, bool guiClicked, int* draggedPage, float* dragOffsetY, float* currentPressure) {

    // Unified Input States
    bool isPointerDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || tabletState.isDown;
    bool isPointerPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || (tabletState.isDown && !previousTabletDown);
    bool isPointerReleased = IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || (!tabletState.isDown && previousTabletDown);

    // Save state for the next frame
    previousTabletDown = tabletState.isDown;

    // CRITICAL: Use isPointerPressed here so OnPress is only called ONCE per stroke
    if (isPointerPressed && !guiClicked) {
        if (vp->isMouseInsideCanvas) {
            doc->activePage = vp->hoveredPage;

            if (vp->localMousePos.y <= 40) {
                *draggedPage = vp->hoveredPage;
                *dragOffsetY = vp->mouseWorldPos.y - (vp->hoveredPage * (doc->pageHeight + PAGE_GAP));
            } else {
                GetActiveBrush()->OnPress(doc, vp->localMousePos, *currentPressure);
            }
        } else {
            vp->isPanning = true;
        }
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
        vp->isPanning = true;
    }

    if (*draggedPage != -1) {
        // Dragging a page header...
    }
    else if (doc->isDrawing && isPointerDown) {
        float clampedX = vp->mouseWorldPos.x;
        if (clampedX < 0.0f) clampedX = 0.0f;
        if (clampedX > doc->pageWidth) clampedX = doc->pageWidth;

        float clampedY = vp->localMousePos.y;
        if (clampedY < 40.0f) clampedY = 40.0f;
        if (clampedY > doc->pageHeight) clampedY = doc->pageHeight;
        Vector2 clampedPos = {clampedX, clampedY};

        // Use the native Wayland pressure variable passed from main.c
        GetActiveBrush()->OnDrag(doc, clampedPos, *currentPressure);

        if (!vp->isMouseInsideCanvas) {
            GetActiveBrush()->OnRelease(doc, clampedPos);
        }
    }
    else if (vp->isPanning) {
        ApplyPanning(vp);
    }

    // Trigger release using the unified pointer release check
    if (isPointerReleased || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        if (*draggedPage != -1) {
            int dropIndex = (int)(vp->mouseWorldPos.y / (doc->pageHeight + PAGE_GAP));
            MovePageToIndex(doc, *draggedPage, dropIndex);
            *draggedPage = -1;
        }
        if (doc->isDrawing) {
            GetActiveBrush()->OnRelease(doc, vp->localMousePos);
        }
        vp->isPanning = false;
    }
}
