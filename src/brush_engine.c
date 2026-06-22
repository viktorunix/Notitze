#include "include/brush_engine.h"
Vector2 CalculateSplinePoint(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return (Vector2){
        0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3),
        0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3)
    };
}

Color Premultiply(Color c){
    Color result = {
        (unsigned char)((c.r * c.a)/ 255),
        (unsigned char)((c.g * c.a) / 255),
        (unsigned char)((c.b * c.a) / 255),
        c.a
    };
    return result;
}
void RenderBrushStroke(Document doc, Stroke *stroke, float pageYOffset, Color pColor){
    if(stroke->pointCount < 3){
        for(int j = 0; j < stroke->pointCount - 1; j++){
            Vector2 p1 = {stroke->points[j].pos.x, stroke->points[j].pos.y + pageYOffset};
            Vector2 p2 = {stroke->points[j+1].pos.x, stroke->points[j+1].pos.y + pageYOffset};
            float pressure = doc.pressureEnabled ? stroke->points[j].pressure : 1.0f;
            float thick = stroke->thickness * pressure;
            thick = fmaxf(thick * 0.05f, 0.5f);

            float spacing = fmaxf(thick * 0.1f, 1.0f);
            float dist = Vector2Distance(p1,p2);
            Vector2 dir = Vector2Normalize(Vector2Subtract(p2, p1));
            for(float d = 0; d < dist; d += spacing){
                Vector2 pos = Vector2Add(p1, Vector2Scale(dir, d));
                Rectangle source = {0, 0, (float)doc.brushTex.width, (float)doc.brushTex.height};
                Rectangle destination = {pos.x, pos.y, thick, thick};
                Vector2 origin = {thick / 2.0f, thick / 2.0f};

                DrawTexturePro(doc.brushTex, source, destination, origin, 0.0f, pColor);
            }
        }
    } else {
        //catmull-rom spline
        for(int j = 0; j < stroke->pointCount - 1; j++){
            StrokePoint p0 = (j == 0) ? stroke->points[0] : stroke->points[j-1];
            StrokePoint p1 = stroke->points[j];
            StrokePoint p2 = stroke->points[j + 1];
            StrokePoint p3 = (j + 2 < stroke->pointCount) ? stroke->points[j + 2] : p2;

            p0.pos.y += pageYOffset;
            p1.pos.y += pageYOffset;
            p2.pos.y += pageYOffset;
            p3.pos.y += pageYOffset;

            float segDist = Vector2Distance(p1.pos, p2.pos);

            float startPressure = doc.pressureEnabled ? p1.pressure : 1.0f;
            float baseThickness = fmaxf(stroke->thickness * startPressure, 0.5f);
            float spacing = fmaxf(baseThickness * 0.05f, 0.5f);

            for(float d = 0; d < segDist; d += spacing){
                float t = d/ segDist;
                Vector2 nextP = CalculateSplinePoint(p0.pos, p1.pos, p2.pos,p3.pos, t);

                float currentPressure = Lerp(p1.pressure, p2.pressure, t);
                float finalPressure = doc.pressureEnabled? currentPressure : 1.0f;
                float currentThickness = fmaxf(stroke->thickness * finalPressure, 0.5f);

                Rectangle source = {0, 0, (float)doc.brushTex.width, (float)doc.brushTex.height};
                Rectangle destination = {nextP.x, nextP.y, currentThickness, currentThickness};
                Vector2 origin = {currentThickness/2.0f, currentThickness/2.0f};

                DrawTexturePro(doc.brushTex, source, destination, origin, 0.0f, pColor);
            }
        }
    }
    if(stroke->pointCount > 0){
        Vector2 last = {stroke->points[stroke->pointCount - 1].pos.x, stroke->points[stroke->pointCount-1].pos.y + pageYOffset};
        float endPressure = doc.pressureEnabled ? stroke->points[stroke->pointCount - 1].pressure : 1.0f;
        float endThick = stroke->thickness * endPressure;
        endThick = fmaxf(endThick, 0.5f);
        //DrawCircleV(last, endThick/ 2.0f, pColor);

        Rectangle source = {0,0, (float)doc.brushTex.width, (float)doc.brushTex.height};
        Rectangle destination = {last.x, last.y, endThick, endThick};
        Vector2 origin = {endThick / 2.0f, endThick / 2.0f};
        DrawTexturePro(doc.brushTex, source, destination, origin, 0.0f, pColor);
    }
}
void RenderHighlighterStroke(Document doc, Stroke *stroke, float pageYOffset){
    Color hColor = stroke->color;
    hColor.a = 100;
    Color mColor = Premultiply(hColor);

    float dx = cosf(PI / 4.0f) * (stroke->thickness / 2.0f);
    float dy = sinf(PI / 4.0f) * (stroke->thickness / 2.0f);

    if(stroke->pointCount  < 3){
        for(int j = 0; j < stroke->pointCount - 1; j++){
            Vector2 p1 = {stroke->points[j].pos.x, stroke->points[j].pos.y + pageYOffset};
            Vector2 p2 = {stroke->points[j + 1].pos.x, stroke->points[j+1].pos.y + pageYOffset};

            Vector2 o1 = {p1.x - dx, p1.y + dy};
            Vector2 o2 = {p1.x + dx, p1.y - dy};

            Vector2 n1 = {p2.x - dx, p2.y + dy};
            Vector2 n2 = {p2.x + dx, p2.y - dy};

            DrawTriangle(o1, o2, n1, mColor);
            DrawTriangle(o2, n2, n1, mColor);
            DrawTriangle(o1, n1, o2, mColor);
            DrawTriangle(o2, n1, n2, mColor);
        }
    } else {
        for( int j = 0; j < stroke->pointCount - 1; j++){
            StrokePoint p0 = (j == 0) ? stroke->points[0] : stroke->points[j - 1];
            StrokePoint p1 = stroke->points[j];
            StrokePoint p2 = stroke->points[j + 1];
            StrokePoint p3 = (j + 2 < stroke->pointCount) ? stroke->points[ j + 2] : p2;

            p0.pos.y += pageYOffset;
            p1.pos.y += pageYOffset;
            p2.pos.y += pageYOffset;
            p3.pos.y += pageYOffset;

            int segments = (int)(Vector2Distance(p1.pos, p2.pos) / 2.0f);
            if(segments < 2) segments = 2;

            Vector2 lastP = p1.pos;

            for(int i = 1; i <= segments; i++){
                float t = (float) i / (float) segments;
                Vector2 nextP = CalculateSplinePoint(p0.pos, p1.pos, p2.pos, p3.pos, t);
                Vector2 o1 = {lastP.x - dx, lastP.y + dy};
                Vector2 o2 = {lastP.x + dx, lastP.y - dy};

                Vector2 n1 = {nextP.x - dx, nextP.y + dy};
                Vector2 n2 = {nextP.x + dx, nextP.y - dy};

                DrawTriangle(o1, o2, n1, mColor);
                DrawTriangle(o2, n2, n1, mColor);

                DrawTriangle(o1,n1,o2, mColor);
                DrawTriangle(o2, n1, n2, mColor);

                lastP = nextP;

            }
        }
    }
}
void RenderPencilStroke(Document doc, Stroke *stroke, float pageYOffset){
    Color graphite = stroke->color;
    graphite.a = 30;
    graphite = Premultiply(graphite);
    if(stroke->pointCount < 3){
        for(int j = 0; j< stroke->pointCount - 1; j++){
            Vector2 p1 = {stroke->points[j].pos.x, stroke->points[j].pos.y + pageYOffset};
            Vector2 p2 = {stroke->points[j+1].pos.x, stroke->points[j+1].pos.y + pageYOffset};

            float pressure = doc.pressureEnabled ? stroke->points[j].pressure : 1.0f;
            float currentThick = stroke->thickness * pressure;
            currentThick = fmaxf(currentThick, 0.5f);

            float spacing = fmaxf(currentThick * 0.10f, 1.0f);
            float dist = Vector2Distance(p1, p2);
            Vector2 dir = Vector2Normalize(Vector2Subtract(p2, p1));


            for(float d = 0; d < dist; d+= 1.0f){
                Vector2 basePos = Vector2Add(p1, Vector2Scale(dir, d));
                Rectangle source = {0, 0, (float)doc.pencilTex.width, (float)doc.pencilTex.height};
                Rectangle dest = {basePos.x, basePos.y, currentThick, currentThick};
                Vector2 origin = {currentThick / 2.0f, currentThick / 2.0f};
                DrawTexturePro(doc.pencilTex, source, dest, origin, 0.0f, graphite); 
            }
        }
    }
    else{
        for(int j = 0; j< stroke->pointCount - 1; j++){
            StrokePoint p0 = (j == 0) ? stroke->points[0] : stroke->points[j-1];
            StrokePoint p1 = stroke->points[j];
            StrokePoint p2 = stroke->points[j + 1];
            StrokePoint p3 = (j + 2 < stroke->pointCount) ? stroke->points[j + 2] : p2;

            p0.pos.y += pageYOffset;
            p1.pos.y += pageYOffset;
            p2.pos.y += pageYOffset;
            p3.pos.y += pageYOffset;


            float segDist = Vector2Distance(p1.pos, p2.pos);
            float startPressure = doc.pressureEnabled ? p1.pressure : 1.0f;
            float baseThickness = stroke->thickness * startPressure;
            baseThickness = fmaxf(baseThickness, 0.5f);

            float spacing = fmaxf(baseThickness * 0.10f, 1.0f);

            for (float d = 0; d <segDist; d+=1.0f){
                float t = d / segDist;
                Vector2 basePos = CalculateSplinePoint(p0.pos,p1.pos,p2.pos,p3.pos,t);
                    
                float currentPres = Lerp(p1.pressure, p2.pressure, t);
                float finalPressure = doc.pressureEnabled ? currentPres : 1.0f;
                float currentThick = stroke->thickness * finalPressure;
                currentThick=fmaxf(currentThick, 0.5f);

                Rectangle source = {0,0, (float)doc.pencilTex.width, (float)doc.pencilTex.height};
                Rectangle dest = {basePos.x, basePos.y, currentThick, currentThick};
                Vector2 origin = {currentThick/ 2.0f, currentThick/ 2.0f};
                DrawTexturePro(doc.pencilTex, source, dest, origin, 0.0f, graphite);
            }    
        }
    }
}
void RenderLineShape(Stroke *stroke, Vector2 p1, Vector2 p2){
    DrawLineEx(p1, p2, stroke->thickness, stroke->color);
    DrawCircleV(p1, stroke->thickness / 2.0f, stroke->color);
    DrawCircleV(p2, stroke->thickness / 2.0f, stroke->color);
}
void RenderRectangleShape(Stroke *stroke, Vector2 p1, Vector2 p2){
    float rx = fminf(p1.x, p2.x);
    float ry = fminf(p1.y, p2.y);
    float rw = fabsf(p1.x - p2.x);
    float rh = fabsf(p1.y - p2.y);
    DrawRectangleLinesEx((Rectangle){rx, ry, rw, rh}, stroke->thickness, stroke->color);

}
void RenderCircleShape(Stroke *stroke, Vector2 p1, Vector2 p2){
    float radius = Vector2Distance(p1, p2);
    DrawRing(p1, radius - (stroke->thickness / 2.0f),radius + (stroke->thickness / 2.0f), 0,360, 64,stroke->color);

}
void RenderShape(Stroke *stroke, float pageYOffset){
    Vector2 p1 = {stroke->points[0].pos.x, stroke->points[0].pos.y + pageYOffset};
    Vector2 p2 = {stroke->points[1].pos.x, stroke->points[1].pos.y + pageYOffset};
    if(stroke->type == BRUSH_LINE)
        RenderLineShape(stroke, p1, p2);
    else if(stroke->type == BRUSH_RECTANGLE)
        RenderRectangleShape(stroke, p1, p2);
    else if (stroke->type == BRUSH_CIRCLE)
        RenderCircleShape(stroke, p1, p2);
}
void RenderStroke(Document doc, Stroke *stroke, float pageYOffset){
    if(stroke->pointCount == 0) return;
    Color pColor = Premultiply(stroke->color);
    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
    if(stroke->type == BRUSH_PEN)
        RenderBrushStroke(doc, stroke, pageYOffset, pColor);
    else if(stroke->type == BRUSH_HIGHLIGHTER)
        RenderHighlighterStroke(doc, stroke, pageYOffset);
    else if(stroke->type == BRUSH_PENCIL)
        RenderPencilStroke(doc, stroke, pageYOffset);
    else if (stroke->pointCount >= 2)
        RenderShape(stroke, pageYOffset);
    EndBlendMode();
}
