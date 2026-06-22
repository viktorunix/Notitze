#include "../include/brush_system.h"
#include "../include/memory.h"
#include "../include/doc_management.h"

extern Stroke currentStroke;
extern Color currentBrushColor;
extern float currentBrushThickness;

static void Shape_OnPress(Document *doc, Vector2 pos, float pressure){
    Layer *activeLayer = &doc->pages[doc->activePage].layers[doc->pages[doc->activePage].activeLayer];

    if(activeLayer->isVisible){
        doc->isDrawing = true;
        currentStroke = (Stroke){0};
        currentStroke.type = GetActiveBrush()->id;
        currentStroke.color = currentBrushColor;
        currentStroke.color.a = 255;
        currentStroke.thickness = currentBrushThickness;

        AddPointToStroke(&currentStroke, pos, 1.0f);
    }
}

static void Shape_OnDrag(Document *doc, Vector2 pos, float pressure){
    if(currentStroke.pointCount == 1)
        AddPointToStroke(&currentStroke, pos, 1.0f);
    else if(currentStroke.pointCount == 2)
        currentStroke.points[1].pos = pos;
}

static void Shape_OnRelease(Document *doc, Vector2 pos){
    doc->isDrawing = false;
    FinishStroke(&currentStroke, doc);
}

Brush CreateLineBrush(void){
    Brush line = {0};
    line.id = BRUSH_LINE;
    line.name = "Line";
    line.OnPress = Shape_OnPress;
    line.OnDrag = Shape_OnDrag;
    line.OnRelease = Shape_OnRelease;
    return line;
}

Brush CreateRectangleBrush(void){
    Brush rectangle = {0};
    rectangle.id = BRUSH_RECTANGLE;
    rectangle.name = "Rectangle";
    rectangle.OnPress = Shape_OnPress;
    rectangle.OnDrag = Shape_OnDrag;
    rectangle.OnRelease = Shape_OnRelease;
    return rectangle;
}

Brush CreateCircleBrush(void) {
    Brush circle = {0};
    circle.id = BRUSH_CIRCLE;
    circle.name = "Circle";
    circle.OnPress = Shape_OnPress;
    circle.OnDrag = Shape_OnDrag;
    circle.OnRelease = Shape_OnRelease;
    return circle;
}