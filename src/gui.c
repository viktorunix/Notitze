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
        printf("DAAA %d\n", stroke->pointCount);
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

void DrawPageBackground(BgPattern pattern, float pageYOffset){
    Color lineColor = (Color){200, 215, 230, 255};
    Color marginColor = (Color){255, 150, 150, 180};

    if (pattern == BG_LINED){
        DrawLine(80, pageYOffset, 80, pageYOffset + A4_HEIGHT, marginColor);
        for(int y = 80; y < A4_HEIGHT; y+=30){
            DrawLine(0, pageYOffset + y, A4_WIDTH, pageYOffset + y, lineColor);
        }
    }
    else if(pattern == BG_GRID){
        for(int x = 30; x < A4_WIDTH; x+=30)
            DrawLine(x, pageYOffset, x, pageYOffset + A4_HEIGHT, lineColor);
        for(int y = 30; y < A4_HEIGHT; y+=30)
            DrawLine(0, pageYOffset + y, A4_WIDTH, pageYOffset + y, lineColor);


    }
    else if (pattern == BG_DOTS){
        for(int x = 30; x < A4_WIDTH; x+=30){
            for(int y = 30; y < A4_HEIGHT; y+=30)
                DrawCircle(x, pageYOffset + y, 2.0f, lineColor);
        }
    }
}

void GUIHeaderBar(Document *doc, Settings *settings){
    DrawRectangle(0,0, GetScreenWidth(), 50, (Color){50,50,50,255});
        DrawText(TextFormat("Page %d/%d", doc->activePage + 1, doc->pageCount),10, 15, 20, WHITE);

        int toolX = 20;
        if(GUIButton((Rectangle){toolX, 10,60, 40}, "Pen", doc->activeBrush == BRUSH_PEN)) doc->activeBrush = BRUSH_PEN;
        if(GUIButton((Rectangle){toolX +=60, 10, 75, 40}, "Pencil", doc->activeBrush == BRUSH_PENCIL)) doc->activeBrush = BRUSH_PENCIL;
        if(GUIButton((Rectangle){toolX += 85, 10, 100, 40}, "Highlighter", doc->activeBrush == BRUSH_HIGHLIGHTER)) doc->activeBrush = BRUSH_HIGHLIGHTER;
        if(GUIButton((Rectangle){toolX += 110, 10, 60, 40}, "Line", doc->activeBrush == BRUSH_LINE)) doc->activeBrush = BRUSH_LINE;
        if(GUIButton((Rectangle){toolX += 70, 10, 70, 40}, "Rect", doc->activeBrush == BRUSH_RECTANGLE)) doc->activeBrush = BRUSH_RECTANGLE;
        if(GUIButton((Rectangle){toolX += 70, 10, 70, 40}, "Circle", doc->activeBrush == BRUSH_CIRCLE)) doc->activeBrush = BRUSH_CIRCLE;

        
        int centerX = GetScreenWidth() / 2;
        DrawText(TextFormat("Page %d/%d", doc->activePage + 1, doc->pageCount), centerX - 250, 20, 20, WHITE);
        GUISlider((Rectangle){centerX - 200, 10, 120, 16}, &settings->currentBrushThickness, 1.0f, 99.0f);
        if(GUIButton((Rectangle){centerX - 130, 10, 70, 40}, "New", false)) AddPageToDocument(doc);
        if(GUIButton((Rectangle){centerX - 50, 10, 70, 40}, "Save", false)) {
            const char *savePath = ShowSaveFileDialog();
            if(savePath) SaveDocumentBinary(savePath, doc);
        }
        if(GUIButton((Rectangle){centerX + 30, 10, 70, 40}, "Load", false)) {
            const char *openPath = ShowOpenFileDialog();
            if(openPath) LoadDocumentBinary(openPath, doc);
        }
        if(GUIButton((Rectangle){centerX + 110, 10, 70, 40}, "Undo", false)) UndoLastStrokes(&doc->pages[doc->activePage]);
        if(GUIButton((Rectangle){centerX + 190, 10, 80, 40}, "Delete", false)) DeleteActivePage(doc);
        if(GUIButton((Rectangle){centerX + 250, 10, 90, 40}, "Settings", settings->showSettings)) settings->showSettings = !settings->showSettings;
        for(int i = 0; i<5;i++){
            int swatchX = GetScreenWidth() - 250 + (i * 45);
            Vector2 center = {swatchX + 20, UI_HEIGHT / 2.0f};


            if(i == settings->selectedColorIndex){
                DrawCircleV(center, 20, WHITE);

            }
            DrawCircleV(center, 16, settings->pallete[i]);
            DrawCircleLines(center.x, center.y, 16, (Color){0,0,0,100});
        }
}