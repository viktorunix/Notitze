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