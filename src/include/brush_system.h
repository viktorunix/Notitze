#ifndef BRUSH_SYSTEM_H
#define BRUSH_SYSTEM_H

#include "raylib.h"
#include "document.h"

typedef struct Brush{
    int id;
    const char* name;

    void (*OnPress)(Document *doc, Vector2 pos, float pressure);
    void (*OnDrag)(Document *doc, Vector2 pos, float pressure);
    void (*OnRelease)(Document *doc, Vector2 pos);
    void (*RenderPreview)(Document *doc, Vector2 pos, float thickness);
} Brush;

void InitBrushSystem(void);
void RegisterBrush(Brush tool);
void SetActiveBrush(int id);
Brush* GetActiveBrush(void);

Brush CreatePenBrush(void);
Brush CreatePencilBrush(void);
Brush CreateHighlighterBrush(void);
Brush CreateLineBrush(void);
Brush CreateRectangleBrush(void);
Brush CreateCircleBrush(void);
Brush CreateEraserBrush(void);

#endif