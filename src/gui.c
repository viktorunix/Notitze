#include "include/gui.h"
#include <stdio.h>
bool GUIButton (Rectangle bounds, const char *text, bool isActive){
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    bool isClicked = isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    
    Color bgColor = isActive ? (Color){70, 130, 180,255} : (isHovered ? (Color){70, 70, 70, 255} : (Color){45, 45, 45, 255});
    Color borderColor = isActive ? SKYBLUE : (Color){80,80,80,255};
    Color textColor = isActive ? WHITE : LIGHTGRAY;


    DrawRectangleRounded(bounds, 0.4f, 10, bgColor);
    DrawRectangleRoundedLinesEx(bounds, 0.4f, 10, 1.05f, borderColor);

    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + (bounds.width - textWidth) / 2, bounds.y + (bounds.height - 20) / 2, 20, textColor);


    return isClicked;
    
}
void GUISlider(Rectangle bounds, float *value, float minValue, float maxValue){
    Vector2 mousePos = GetMousePosition();

    Rectangle hitBox = {bounds.x - 10, bounds.y - 10, bounds.width + 20, bounds.height + 20};

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, hitBox)){
        float normalized = (mousePos.x - bounds.x) / bounds.width;
        if(normalized < 0.0f) normalized = 0.0f;
        if(normalized >= 1.0f) normalized = 1.0f;
        *value = minValue + normalized * (maxValue - minValue);
    }

    //background
    DrawRectangleRounded(bounds, 1.0f, 10, (Color) {35,35,40, 255});
    DrawRectangleRoundedLinesEx(bounds, 1.0f, 10, 1.5f, (Color){90, 90, 95, 255});

    //filled portion
    float currentNorm = (*value - minValue) / (maxValue - minValue);
    if(currentNorm > 0.02f){
        Rectangle fillRec = {bounds.x, bounds.y, bounds.width * currentNorm, bounds.height};
        DrawRectangleRounded(fillRec, 1.0f, 10, (Color){70, 130, 180, 255});

    }

    //knob
    Vector2 knobCenter = {bounds.x + bounds.width * currentNorm, bounds.y + bounds.height / 2.0f};
    DrawCircleV(knobCenter, bounds.height * 0.8f, WHITE);
    DrawCircleLines(knobCenter.x, knobCenter.y, bounds.height * 0.8f, GRAY);
}
Vector2 CalculateSplinePoint(Vector2 p0, Vector2 p1, Vector2 p2, Vector2 p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return (Vector2){
        0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3),
        0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3)
    };
}
void newRenderStroke(Stroke *stroke, float pageYOffset){
    if(stroke->pointCount == 0) return;
    if(stroke->type == BRUSH_PEN || stroke->type == BRUSH_HIGHLIGHTER){
        if(stroke->pointCount < 3){
            for(int j = 0; j < stroke->pointCount - 1; j++){
                Vector2 p1 = {stroke->points[j].pos.x, stroke->points[j].pos.y + pageYOffset};
                Vector2 p2 = {stroke->points[j+1].pos.x, stroke->points[j+1].pos.y + pageYOffset};
                float thick = stroke->thickness * stroke->points[j].pressure;
                DrawLineEx(p1, p2, thick, stroke->color);
                DrawCircleV(p1, thick / 2.0f, stroke->color);
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

                int segments = (int)(Vector2Distance(p1.pos,p2.pos)/2.0f);
                if(segments < 2) segments = 2;

                Vector2 lastP = p1.pos;
                DrawCircleV(p1.pos,(stroke->thickness * p1.pressure) / 2.0f, stroke->color);
                for(int i = 1; i <=segments; i++){
                    float t = (float)i / (float)segments;
                    Vector2 nextP = CalculateSplinePoint(p0.pos,p1.pos,p2.pos,p3.pos, t);
                    
                    float currentPres = Lerp(p1.pressure, p2.pressure, t);
                    float currentThick = stroke->thickness * currentPres;
                    
                    DrawLineEx(lastP, nextP, currentThick, stroke->color);
                    DrawCircleV(nextP, currentThick / 2.0f, stroke->color);
                    lastP = nextP;
                }
            }
        }
    }
    else if (stroke->type == BRUSH_PENCIL){
        Color graphite = stroke->color;
        graphite.a = 100;
        if(stroke->pointCount < 3){
            for(int j = 0; j< stroke->pointCount - 1; j++){
                Vector2 p1 = {stroke->points[j].pos.x, stroke->points[j].pos.y + pageYOffset};
                Vector2 p2 = {stroke->points[j+1].pos.x, stroke->points[j+1].pos.y + pageYOffset};

                SetRandomSeed((unsigned int)(p1.x * 1337 + p1.y * 9999));

                float dist = Vector2Distance(p1, p2);
                Vector2 dir = Vector2Normalize(Vector2Subtract(p2, p1));

                float currentThick = stroke->thickness * stroke->points[j].pressure;
                for(float d = 0; d < dist; d+= 1.0f){
                    Vector2 basePos = Vector2Add(p1, Vector2Scale(dir, d));
                    int dustCount = (int)currentThick;
                    if(dustCount < 3) dustCount = 3;
                    for(int k = 0; k <dustCount; k++){
                        float randX = ((float)GetRandomValue(-100, 100) / 100.0f) * (currentThick / 2.0f);
                        float randY = ((float)GetRandomValue(-100, 100) / 100.0f) * (currentThick / 2.0f);

                        Vector2 speck = {basePos.x + randX, basePos.y + randY};
                        DrawCircleV(speck, 0.5f, graphite);
                    }
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

                SetRandomSeed((unsigned int)(p1.pos.x * 1337 + p1.pos.y * 9999));
                float segDist = Vector2Distance(p1.pos, p2.pos);
                for (float d = 0; d <segDist; d+=1.0f){
                    float t = d / segDist;
                    Vector2 basePos = CalculateSplinePoint(p0.pos,p1.pos,p2.pos,p3.pos,t);
                    
                    float currentPres = Lerp(p1.pressure, p2.pressure, t);
                    float currentThick = stroke->thickness * currentPres;

                    int dustCount=(int)(currentPres);
                    if(dustCount <3) dustCount = 3;
                    for (int k = 0; k < dustCount; k++) {
                        float randX = ((float)GetRandomValue(-100, 100) / 100.0f) * (currentThick / 2.0f);
                        float randY = ((float)GetRandomValue(-100, 100) / 100.0f) * (currentThick / 2.0f);
                        DrawCircleV((Vector2){basePos.x + randX, basePos.y + randY}, 0.5f, graphite);
                    }
                }
                
            }

        }
    } else if (stroke->pointCount >= 2){
        Vector2 p1 = {stroke->points[0].pos.x, stroke->points[0].pos.y + pageYOffset};
        Vector2 p2 = {stroke->points[1].pos.x, stroke->points[1].pos.y + pageYOffset};

        if(stroke->type == BRUSH_LINE){
            DrawLineEx(p1, p2, stroke->thickness, stroke->color);
            DrawCircleV(p1, stroke->thickness / 2.0f, stroke->color);
            DrawCircleV(p2, stroke->thickness / 2.0f, stroke->color);
        }
        else if(stroke->type == BRUSH_RECTANGLE){
            float rx = fminf(p1.x, p2.x);
            float ry = fminf(p1.y, p2.y);
            float rw = fabsf(p1.x - p2.x);
            float rh = fabsf(p1.y - p2.y);
            DrawRectangleLinesEx((Rectangle){rx, ry, rw, rh}, stroke->thickness, stroke->color);
        }
        else if(stroke->type == BRUSH_CIRCLE){
            float radius = Vector2Distance(p1, p2);
            DrawRing(p1, radius - (stroke->thickness / 2.0f),radius + (stroke->thickness / 2.0f), 0,360, 64,stroke->color);
        }
    }
}
/*
void RenderStroke(Stroke *stroke, float pageYOffset){
    if(stroke->pointCount == 0) return;
    if(stroke->type == BRUSH_PEN || stroke->type == BRUSH_HIGHLIGHTER){
        for(int j = 0;j < stroke->pointCount - 1; j++){
            Vector2 p1 = {stroke->points[j].x, stroke->points[j].y + pageYOffset};
            Vector2 p2 = {stroke->points[j+1].x, stroke->points[j+1].y + pageYOffset};
            DrawLineEx(p1, p2, stroke->thickness, stroke->color);
            DrawCircleV(p1, stroke->thickness / 2.0f, stroke->color);
        }
        if(stroke->pointCount > 0){
            Vector2 last = {stroke->points[stroke->pointCount - 1].x, stroke->points[stroke->pointCount - 1].y + pageYOffset};
            DrawCircleV(last, stroke->thickness/2.0f, stroke->color);
        }
    }
    else if (stroke->type == BRUSH_PENCIL){
        Color graphite = stroke->color;
        graphite.a = 100;

        for(int j = 0; j< stroke->pointCount - 1; j++){
            Vector2 p1 = {stroke->points[j].x, stroke->points[j].y + pageYOffset};
            Vector2 p2 = {stroke->points[j+1].x, stroke->points[j+1].y + pageYOffset};

            SetRandomSeed((unsigned int)(stroke->points[j].x * 1337 + stroke->points[j].y * 9999));

            float dist = Vector2Distance(p1, p2);
            Vector2 dir = Vector2Normalize(Vector2Subtract(p2, p1));

            for(float d = 0; d < dist; d+= 1.0f){
                Vector2 basePos = Vector2Add(p1, Vector2Scale(dir, d));

                for(int k = 0; k <2 * stroke->thickness; k++){
                    float randX = ((float)GetRandomValue(-100, 100) / 100.0f) * (stroke->thickness / 2.0f);
                    float randY = ((float)GetRandomValue(-100, 100) / 100.0f) * (stroke->thickness / 2.0f);

                    Vector2 speck = {basePos.x + randX, basePos.y + randY};
                    DrawCircleV(speck, 0.5f, graphite);
                }
            }
        }
    }
    else if (stroke->pointCount >= 2){
        Vector2 p1 = {stroke->points[0].x, stroke->points[0].y + pageYOffset};
        Vector2 p2 = {stroke->points[1].x, stroke->points[1].y + pageYOffset};

        if(stroke->type == BRUSH_LINE){
            DrawLineEx(p1, p2, stroke->thickness, stroke->color);
            DrawCircleV(p1, stroke->thickness / 2.0f, stroke->color);
            DrawCircleV(p2, stroke->thickness / 2.0f, stroke->color);
        }
        else if(stroke->type == BRUSH_RECTANGLE){
            float rx = fminf(p1.x, p2.x);
            float ry = fminf(p1.y, p2.y);
            float rw = fabsf(p1.x - p2.x);
            float rh = fabsf(p1.y - p2.y);
            DrawRectangleLinesEx((Rectangle){rx, ry, rw, rh}, stroke->thickness, stroke->color);
        }
        else if(stroke->type == BRUSH_CIRCLE){
            float radius = Vector2Distance(p1, p2);
            DrawRing(p1, radius - (stroke->thickness / 2.0f),radius + (stroke->thickness / 2.0f), 0,360, 64,stroke->color);
        }
    }
}
*/
void DrawPageBackground(Document *doc, BgPattern pattern, float pageYOffset){
    Color lineColor = (Color){200, 215, 230, 255};
    Color marginColor = (Color){255, 150, 150, 180};

    if (pattern == BG_LINED){
        DrawLine(80, pageYOffset, 80, pageYOffset + doc->pageHeight, marginColor);
        for(int y = 80; y < doc->pageHeight; y+=30){
            DrawLine(0, pageYOffset + y, doc->pageWidth, pageYOffset + y, lineColor);
        }
    }
    else if(pattern == BG_GRID){
        for(int x = 30; x < doc->pageWidth; x+=30)
            DrawLine(x, pageYOffset, x, pageYOffset + doc->pageHeight, lineColor);
        for(int y = 30; y < doc->pageHeight; y+=30)
            DrawLine(0, pageYOffset + y, doc->pageWidth, pageYOffset + y, lineColor);


    }
    else if (pattern == BG_DOTS){
        for(int x = 30; x < doc->pageWidth; x+=30){
            for(int y = 30; y < doc->pageHeight; y+=30)
                DrawCircle(x, pageYOffset + y, 2.0f, lineColor);
        }
    }
}

void GUILayerPanel(Document *doc){
    int barY = 20;
    int barHeight = 140;
    Page *aPage = &doc->pages[doc->activePage];
    int pW = 300;
    int pH = 60 + (aPage->layerCount * 125) + (aPage->layerCount > 1 ? 50 : 0);
    int pX = GetScreenWidth() - pW - 20;
    int pY = barY + barHeight + 20;

    DrawRectangleRounded((Rectangle){pX + 5, pY + 5, pW, pH}, 0.1f, 10, (Color){0,0,0,100});
    DrawRectangleRounded((Rectangle){pX, pY, pW, pH}, 0.1f, 10, (Color){35,35,40, 245});
    DrawRectangleRoundedLinesEx((Rectangle){pX, pY, pW, pH}, 0.1f, 10, 2.0f, (Color){60,60,65,255});

    DrawText("Layers", pX + 20, pY + 18, 20, WHITE);
    if(GUIButton((Rectangle){pX + pW -50, pY + 10, 35, 35}, "+", false)) AddLayerToPage(aPage);

    int lY = pY + 60;
    for(int l = aPage->layerCount - 1; l >=0; l--){
        Layer *layer = &aPage->layers[l];
        if(GUIButton((Rectangle){pX+10, lY + 45,35, 35}, layer->isVisible ? "O" : "-", layer->isVisible))
            layer->isVisible= !layer->isVisible;

        Rectangle thumbRec = {pX + 60, lY + 6, 80, 113};
        DrawRectangleRec(thumbRec, RAYWHITE);

        BeginScissorMode(thumbRec.x, thumbRec.y, thumbRec.width, thumbRec.height);

        Camera2D thumbCam = {0};
        thumbCam.target = (Vector2){0,0};
        thumbCam.offset = (Vector2){thumbRec.x, thumbRec.y};
        thumbCam.zoom = thumbRec.width / (float)doc->pageWidth;

        BeginMode2D(thumbCam);
        for(int i = 0; i < layer->strokeCount;i++)
            RenderStroke(&layer->strokes[i], 0);

        EndMode2D();
        EndScissorMode();
        DrawRectangleLinesEx(thumbRec, 1.0f, LIGHTGRAY);

        if(GUIButton((Rectangle){pX+155,lY+ 42, pW - 170,40}, TextFormat("Layer %d", l+1), aPage->activeLayer == l))
            aPage->activeLayer = l;
        lY+=125;
    }
    if(aPage->layerCount > 1){
        if(GUIButton((Rectangle){pX + 10, lY + 5, pW - 20, 35}, "Delete Layer", false))
            DeleteActiveLayer(aPage);
    }
}

void GUIPage(Document *doc, Stroke *currentStroke, int p, int pageYOffset){
    //paper
    DrawRectangle(8, pageYOffset + 8, doc->pageWidth, doc->pageHeight, BLACK);
    DrawRectangle(0, pageYOffset, doc->pageWidth, doc->pageHeight, RAYWHITE);
    DrawPageBackground(doc, doc->pattern, pageYOffset);
    //paper drag bar
    DrawRectangle(0, pageYOffset, doc->pageWidth, 40, (Color){200,200,200,255});
    DrawText("|||", doc->pageWidth/2 - MeasureText("|||", 20)/ 2, pageYOffset + 10, 20, DARKGRAY);
    // active page highlight
    Color borderColor = (p == doc->activePage) ? SKYBLUE : LIGHTGRAY;
    int borderThickness = (p == doc->activePage) ? 4 : 1;
    DrawRectangleLinesEx((Rectangle){0, pageYOffset, doc->pageWidth, doc->pageHeight}, borderThickness, borderColor);
    //strokes
    Page *page = &doc->pages[p];

    for(int l = 0; l < page->layerCount; l++){
        Layer *layer = &page->layers[l];
        if(!layer->isVisible) continue;
        for(int i = 0; i < layer->strokeCount; i++)
            RenderStroke(&layer->strokes[i], pageYOffset);
        if(doc->isDrawing && p == doc->activePage && l == page->activeLayer)
            RenderStroke(currentStroke, pageYOffset);
    }

}