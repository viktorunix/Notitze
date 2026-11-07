#include "include/input_manager.h"
#include "include/brush_system.h"
#include "include/doc_management.h"
#include "include/raymath.h"
#include "include/tablet_support.h"
#include "include/command_system.h"
#define PAGE_GAP 60


extern Stroke currentStroke;
void PollInputState(InputState *state, Settings *settings, Camera2D camera, bool guiHovered){
    static Vector2 smoothedPos = {0};
    static float smoothedPressure = 0.0f;

    state->mouseScreenPos = GetMousePosition();
    state->mouseWorldPos = GetScreenToWorld2D(state->mouseScreenPos, camera);
    state->mouseDelta = GetMouseDelta();
    state->scrollDelta = GetMouseWheelMove();
    float rawPressure = GetTabletPressure();
    smoothedPressure += (rawPressure - smoothedPressure) * 0.15f;

    state->pressure = smoothedPressure;
    state->smoothingFactor = settings->smoothingFactor;

    state->isHardwareActive = (
        state->mouseDelta.x != 0.0f || state->mouseDelta.y != 0.0f ||
        state->scrollDelta != 0.0f || 
        GetKeyPressed() != 0 || GetCharPressed() != 0 ||
        IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT) ||
        IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)
    );

    //cleaning last frame action
    for(int i = 0; i < ACTION_COUNT; i++){
        state->active[i] = false;
        state->pressed[i] = false;
        state->released[i] = false;
    }

    if(!guiHovered){
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) state->active[ACTION_DRAW] = true;
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) state->pressed[ACTION_DRAW] = true;
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) state->released[ACTION_DRAW] = true;

        if(IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) state->active[ACTION_PAN] = true;

        if(state->scrollDelta > 0) state->pressed[ACTION_ZOOM_IN] = true;
        if(state->scrollDelta < 0) state->pressed[ACTION_ZOOM_OUT] = true;
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_Z)) {
        state->pressed[ACTION_UNDO] = true;
    }
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_Z)) {
        state->pressed[ACTION_REDO] = true;
    }

}
void ProcessInputs(Document *doc, Viewport* vp, InputState* state, int* draggedPage, float* dragOffsetY){
    static Vector2 smoothedPos = {0};
    static Vector2 snapStartPos = {0};
    static bool isSnapped = false;
    float smoothingFactor = fmax(1.0f -state->smoothingFactor, 0.01f);

    if(state->pressed[ACTION_DRAW]){
        if(vp->isMouseInsideCanvas){
            isSnapped = false;
            doc->activePage = vp->hoveredPage;

            if(vp->localMousePos.y <=40){
                *draggedPage = vp->hoveredPage;
                *dragOffsetY = state->mouseWorldPos.y - (vp->hoveredPage * (doc->pageHeight + PAGE_GAP));

            } else {
                smoothedPos = vp->localMousePos;
                snapStartPos = vp->localMousePos;
                GetActiveBrush()->OnPress(doc, vp->localMousePos, state->pressure);

            }
        } else {
            vp->isPanning = true;
        }
    }

    if(state->pressed[ACTION_PAN]){
        vp->isPanning = true;

    }
    if(state->pressed[ACTION_UNDO]){
        UndoCommand(doc);
    }

    if(*draggedPage != -1){

    }
    else  if (doc->isDrawing && state->active[ACTION_DRAW]){
        float clampedX = state->mouseWorldPos.x;
        if (clampedX < 0.0f) clampedX = 0.0f;
        if(clampedX > doc->pageWidth) clampedX = doc->pageWidth;

        float clampedY = vp->localMousePos.y;
        if (clampedY < 40.0f) clampedY = 40.0f;
        if (clampedY > doc->pageHeight) clampedY = doc->pageHeight;
        Vector2 realMousePos = {clampedX, clampedY};
        if(IsKeyDown(KEY_LEFT_SHIFT)){
            isSnapped = true;
        }
        if(isSnapped){
            float dx = realMousePos.x - snapStartPos.x;
            float dy = realMousePos.y - snapStartPos.y;

            if(fabsf(dx) > fabsf(dy)) {
                realMousePos.y = snapStartPos.y;
            } else {
                realMousePos.x = snapStartPos.x;
            }
        }
        smoothedPos.x += (realMousePos.x - smoothedPos.x) * smoothingFactor;
        smoothedPos.y += (realMousePos.y - smoothedPos.y) * smoothingFactor;

        GetActiveBrush()->OnDrag(doc, smoothedPos, state->pressure);

        if(!vp->isMouseInsideCanvas){
            GetActiveBrush()->OnRelease(doc, smoothedPos);
        }
    }
    else if (vp->isPanning || state->active[ACTION_PAN]){
        ApplyPanning(vp);
    }

    if(state->released[ACTION_DRAW] || state->released[ACTION_PAN]){
        if(*draggedPage != -1){
            int dropIndex = (int)(state->mouseWorldPos.y / (doc->pageHeight + PAGE_GAP));
            MovePageToIndex(doc, *draggedPage, dropIndex);
            *draggedPage = -1;
        }
        if(doc->isDrawing){
            GetActiveBrush()->OnRelease(doc, vp->localMousePos);
        }
        vp->isPanning = false;
    }
}
