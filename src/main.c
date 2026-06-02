#include <stdlib.h>
#include <stdio.h>
#include "include/raylib.h"
#include "raylib.h"
#include "raymath.h"
#include "include/document.h"

void AddPointToStroke(Stroke *stroke, Vector2 point){
    if(stroke->pointCount >= stroke->capacity){
        stroke->capacity = stroke->capacity == 0 ? 128 : stroke->capacity * 2;
        stroke->points = (Vector2 *)realloc(stroke->points, stroke->capacity * sizeof(Vector2));

    }
    stroke->points[stroke->pointCount++] = point;
}

void AddStrokeToDocument(Document *doc, Stroke stroke){
    if(doc->strokeCount >= doc->capacity){
        doc->capacity = doc->capacity == 0 ? 32 : doc->capacity * 2;
        doc->strokes = (Stroke *)realloc(doc->strokes, doc->capacity * sizeof(Stroke));

    }
    doc->strokes[doc->strokeCount++] = stroke;
}

void UndoLastStrokes(Document *doc){
    if(doc->strokeCount > 0){
        doc->strokeCount--;

        free(doc->strokes[doc->strokeCount].points);
        doc->strokes[doc->strokeCount].points = NULL;
        doc->strokes[doc->strokeCount].pointCount = 0;
        doc->strokes[doc->strokeCount].capacity = 0;
    }
}
int main(void){
    const int screenWidth = 1200;
    const int screenHeight = 800;

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Notitze");

    Document doc = {0};
    Stroke currentStroke = {0};
    bool isDrawing = false;

    Color currentBrushColor = BLACK;
    float currentBrushThickness = 3.0f;

    SetTargetFPS(120);

    while(!WindowShouldClose()){

        if(IsKeyPressed(KEY_ONE)){
            currentBrushColor = BLACK;
            currentBrushThickness = 3.0f;
        }
        if(IsKeyPressed(KEY_TWO)){
            currentBrushColor = (Color){255,255,0,100};;
            currentBrushThickness = 20.0f;
        }
        if(IsKeyPressed(KEY_U)){
            UndoLastStrokes(&doc);
        }

        Vector2 mousePos = GetMousePosition();

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            isDrawing = true;
            currentStroke = (Stroke){0};
            currentStroke.color = currentBrushColor;
            currentStroke.thickness = currentBrushThickness;
            AddPointToStroke(&currentStroke, mousePos);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isDrawing){
            if(currentStroke.pointCount > 0){
                Vector2 lastPoint = currentStroke.points[currentStroke.pointCount - 1];
                float distSq = (mousePos.x - lastPoint.x)*(mousePos.x - lastPoint.x) + (mousePos.y - lastPoint.y)*(mousePos.y - lastPoint.y);
                if(distSq > 9.0f){
                    AddPointToStroke(&currentStroke, mousePos);
                }
            }
        }
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isDrawing){
            isDrawing = false;
            if(currentStroke.pointCount > 1){
                AddStrokeToDocument(&doc, currentStroke);
            } else{
                free(currentStroke.points);
            }
            currentStroke = (Stroke){0};
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Controls [1] Pen | [2] Highlighter | [U] Undo", 20, 20 ,20, DARKGRAY);

        for(int i = 0; i <doc.strokeCount;i++){
            Stroke *s = &doc.strokes[i];
            for(int j = 0; j< s->pointCount - 1;j++){
                DrawLineEx(s->points[j], s->points[j+1], s->thickness, s->color);
                DrawCircleV(s->points[j], s->thickness/2.0f, s->color);
            }
            if(s->pointCount >0)
                DrawCircleV(s->points[s->pointCount - 1], s->thickness/2.0f, s->color);
        }

        if(isDrawing){
            for(int j = 0;j< currentStroke.pointCount - 1; j++){
                DrawLineEx(currentStroke.points[j], currentStroke.points[j+1], currentStroke.thickness, currentStroke.color);
                DrawCircleV(currentStroke.points[j], currentStroke.thickness/2.0f, currentStroke.color);

            }
            if (currentStroke.pointCount > 0)
                DrawCircleV(currentStroke.points[currentStroke.pointCount - 1], currentStroke.thickness/2.0f, currentStroke.color);

        }
        EndDrawing();
    }
    for (int i = 0; i < doc.strokeCount; i++) {
            free(doc.strokes[i].points);
        }
    free(doc.strokes);
    CloseWindow();

    return 0;
}
