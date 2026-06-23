#include "include/input_manager.h"
#include "include/brush_system.h"
#include "include/doc_management.h"
#include "include/raymath.h"
#include "include/tablet_support.h"
#define PAGE_GAP 60


extern Stroke currentStroke; 

void ProcessInputs(Document* doc, Viewport* vp, bool guiClicked, int* draggedPage, float* dragOffsetY, float* currentPressure) {
    

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !guiClicked) {
        if (vp->isMouseInsideCanvas) {
            doc->activePage = vp->hoveredPage;
            
            if (vp->localMousePos.y <= 40) { 
                *draggedPage = vp->hoveredPage;
                *dragOffsetY = vp->mouseWorldPos.y - (vp->hoveredPage * (doc->pageHeight + PAGE_GAP));
            } else {
                *currentPressure = GetTabletPressure();
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
        // Dragging a page header... (Handled visually by the renderer)
    } 
    else if (doc->isDrawing && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float clampedX = vp->mouseWorldPos.x;
        if (clampedX < 0.0f) clampedX = 0.0f;
        if (clampedX > doc->pageWidth) clampedX = doc->pageWidth;

        float clampedY = vp->localMousePos.y;
        if (clampedY < 40.0f) clampedY = 40.0f;
        if (clampedY > doc->pageHeight) clampedY = doc->pageHeight;
        Vector2 clampedPos = {clampedX, clampedY};
        if(IsUsingTablet()){
            *currentPressure = GetTabletPressure();
        } else{
            if (doc->activeBrush <= BRUSH_PENCIL && currentStroke.pointCount > 0) {
                float dist = Vector2Distance(vp->localMousePos, currentStroke.points[currentStroke.pointCount - 1].pos);
                float targetPressure = 1.0f - (dist / 30.0f);
                if (targetPressure < 0.1f) targetPressure = 0.1f;
                if (targetPressure > 1.0f) targetPressure = 1.0f;
                *currentPressure = (*currentPressure * 0.7f) + (targetPressure * 0.3f);
            }
        }
        GetActiveBrush()->OnDrag(doc, clampedPos, *currentPressure);

        if (!vp->isMouseInsideCanvas) {
            GetActiveBrush()->OnRelease(doc, clampedPos);
        }
    } 
    else if (vp->isPanning) {
        ApplyPanning(vp);
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
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