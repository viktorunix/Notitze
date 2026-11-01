#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "raylib.h"
#include "document.h"
#include "settings.h"
#include "viewport.h"

typedef enum {
    ACTION_DRAW,
    ACTION_PAN,
    ACTION_ZOOM_IN,
    ACTION_ZOOM_OUT,
    ACTION_UNDO,
    ACTION_REDO,
    ACTION_DRAG_PAGE,
    ACTION_COUNT
} InputAction;

typedef struct {
    bool active[ACTION_COUNT];
    bool pressed[ACTION_COUNT];
    bool released[ACTION_COUNT];

    Vector2 mouseScreenPos;
    Vector2 mouseWorldPos;
    Vector2 mouseDelta;

    float scrollDelta;
    float pressure;

    bool isHardwareActive;
} InputState;

void PollInputState(InputState *state, Settings *settings, Camera2D camerra, bool guiHovered);
void ProcessInputs(Document *doc, Viewport* vp, InputState* state, int* draggedPage, float* dragOffsetY);


#endif
