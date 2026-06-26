#include "../include/brush_system.h"
#include "../include/memory.h"
#include "../include/doc_management.h"

extern Stroke currentStroke;
extern Color currentBrushColor;
extern float currentBrushThickness;

static void FreeHand_OnPress(Document *doc, Vector2 pos, float pressure){
    Layer *activeLayer = &doc->pages[doc->activePage]. layers[doc->pages[doc->activePage].activeLayer];
    if(activeLayer->isVisible){
        doc->isDrawing = true;
        currentStroke = (Stroke){0};

        int brushId = GetActiveBrush()->id;

        currentStroke.type = brushId;
        currentStroke.color = currentBrushColor;
        currentStroke.thickness = currentBrushThickness;
        currentStroke.color.a = brushId == BRUSH_HIGHLIGHTER ? 150 : 255;
        AddPointToStroke(&currentStroke, pos, pressure);
    }

}

static void FreeHand_OnDrag(Document *doc, Vector2 pos, float pressure){
    #ifdef __WIN32
    if(currentStroke.pointCount > 0){
        Vector2 lastPoint = currentStroke.points[currentStroke.pointCount - 1].pos;
        float dist = Vector2Distance(pos, lastPoint);

        if((dist * dist) > 4.0f){
            Vector2 smoothPoint = {
                lastPoint.x + (pos.x - lastPoint.x) * 0.75f,
                lastPoint.y + (pos.y - lastPoint.y) * 0.75f
            };
            AddPointToStroke(&currentStroke, smoothPoint, pressure);
        }
    }
    #elif defined(__linux__)

    if(currentStroke.pointCount > 0){
        Vector2 lastPoint = currentStroke.points[currentStroke.pointCount - 1].pos;
        float dist = Vector2Distance(pos,lastPoint);

        float stepSize = 2.0f;

        if(dist >= stepSize){
            int steps = (int)(dist/stepSize);

            float lastPressure = currentStroke.points[currentStroke.pointCount - 1].pressure;
            for(int i = 1; i <= steps; i++){
                float t = (float)i / (float)steps;

                Vector2 lerpPos = {
                    lastPoint.x + (pos.x - lastPoint.x) * t,
                    lastPoint.y + (pos.y - lastPoint.y) * t
                };

                float lerpPressure = lastPressure + (pressure - lastPressure) * t;
                AddPointToStroke(&currentStroke, lerpPos, lerpPressure);
            }
        }
    }

    #endif
}

static void FreeHand_OnRelease(Document *doc, Vector2 pos){
    doc->isDrawing = false;
    FinishStroke(&currentStroke, doc);
}

Brush CreatePenBrush(void){
    Brush pen = {0};
    pen.id = BRUSH_PEN;
    pen.name = "Pen";
    pen.OnPress = FreeHand_OnPress;
    pen.OnDrag = FreeHand_OnDrag;
    pen.OnRelease = FreeHand_OnRelease;
    return pen;
}

Brush CreatePencilBrush(void){
    Brush pencil = {0};
    pencil.id = BRUSH_PENCIL;
    pencil.name = "Pencil";
    pencil.OnPress = FreeHand_OnPress;
    pencil.OnDrag = FreeHand_OnDrag;
    pencil.OnRelease = FreeHand_OnRelease;
    return pencil;
}

Brush CreateHighlighterBrush(void){
    Brush highlighter = {0};
    highlighter.id = BRUSH_HIGHLIGHTER;
    highlighter.name = "Highlighter";
    highlighter.OnPress = FreeHand_OnPress;
    highlighter.OnDrag = FreeHand_OnDrag;
    highlighter.OnRelease = FreeHand_OnRelease;
    return highlighter;
}
