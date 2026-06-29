#include "../include/brush_system.h"
#include "../include/memory.h"
#include "../include/doc_management.h"
#include "../include/command_system.h"

extern float currentBrushThickness;
float DistancePointSegment(Vector2 p, Vector2 a, Vector2 b){
    Vector2 ab = Vector2Subtract(b, a);
    Vector2 ap = Vector2Subtract(p, a);

    float dotAB = Vector2DotProduct(ab, ab);
    if(dotAB == 0.0f) return Vector2Distance(p, a);

    float t = Vector2DotProduct(ap, ab) / dotAB;
    t = fmaxf(0.0f, fminf(1.0f, t));

    Vector2 proj = Vector2Add(a, Vector2Scale(ab, t));
    return Vector2Distance(p, proj);
}

static void Eraser_OnDrag(Document *doc, Vector2 pos, float pressure){
    float eraserRadius = currentBrushThickness * 2.0f;
    Layer *activeLayer = &doc->pages[doc->activePage].layers[doc->pages[doc->activePage].activeLayer];
    bool layerNeedsRebake = false;

    for(int i = activeLayer->strokeCount - 1; i >=0; i--){
        Stroke *s = &activeLayer->strokes[i];
        bool hit = false;
        float hitThreshold = (s->thickness / 2.0f) + eraserRadius;

        if(s->type == BRUSH_CIRCLE){
            float radius = Vector2Distance(s->points[0].pos, s->points[1].pos);
            float distToCenter = Vector2Distance(pos, s->points[0].pos);
            if(fabsf(distToCenter - radius) <= hitThreshold) hit = true;
        }
        else if (s->type == BRUSH_RECTANGLE) {
            Vector2 p1 = s->points[0].pos;
            Vector2 p2 = s->points[1].pos;
            Vector2 tr = {p2.x, p1.y};
            Vector2 bl = {p1.x, p2.y};
            if (DistancePointSegment(pos, p1, tr) <= hitThreshold ||
                DistancePointSegment(pos, tr, p2) <= hitThreshold ||
                DistancePointSegment(pos, p2, bl) <= hitThreshold ||
                DistancePointSegment(pos, bl, p1) <= hitThreshold) hit = true;
            }
            else if(s->type == BRUSH_TEXT && s->pointCount > 0){
                int fontSize = (int)(s->thickness * 8.0f);
                if(fontSize < 10) fontSize = 10;
                int textWidth = MeasureText(s->text, fontSize);
                Rectangle textRec = {s->points[0].pos.x, s->points[0].pos.y, (float)textWidth, (float)fontSize};
                if(CheckCollisionCircleRec(pos, eraserRadius, textRec)) hit = true;
            } 
            else if (s->pointCount >= 2) {
                for (int j = 0; j < s->pointCount - 1; j++) {
                    if (DistancePointSegment(pos, s->points[j].pos, s->points[j+1].pos) <= hitThreshold) {
                        hit = true;
                        break;
                    }
                }
            }
            if (hit) {
                PushEraseCommand(doc->activePage, doc->pages[doc->activePage].activeLayer, s, i);
                RemoveStrokeFromLayer(activeLayer, i);
                layerNeedsRebake = true;
            }

        }
        if (layerNeedsRebake && doc->useBakedRendering) {
            BeginTextureMode(activeLayer->texture);
            ClearBackground(BLANK);
            Camera2D bakeCam = {0};
            bakeCam.zoom = doc->renderScale;
            BeginMode2D(bakeCam);
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            for(int s = 0; s < activeLayer->strokeCount; s++) {
                RenderStroke(*doc, &activeLayer->strokes[s], 0);
            }
            EndBlendMode();
            EndMode2D();
            EndTextureMode();
        }
}
static void Eraser_RenderPreview(Document *doc, Vector2 pos, float thickness){
    float eraserRadius = thickness *2.0f;
    DrawCircleLines(pos.x, pos.y, eraserRadius, RED);
}
static void Eraser_OnPress(Document *doc, Vector2 pos, float pressure) {
    
    doc->isDrawing = true;
}

static void Eraser_OnRelease(Document *doc, Vector2 pos) {
    doc->isDrawing = false;
}
Brush CreateEraserBrush(void){
    Brush eraser = {0};
    eraser.id = BRUSH_ERASER;
    eraser.name = "Eraser";
    eraser.OnPress = Eraser_OnPress;
    eraser.OnDrag = Eraser_OnDrag;
    eraser.OnRelease = Eraser_OnRelease;
    eraser.RenderPreview = Eraser_RenderPreview;

    return eraser;
}