#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "include/raylib.h"
#include "raylib.h"
#include "raymath.h"
#include "include/document.h"

#define A4_WIDTH 842
#define A4_HEIGHT 1191
#define SAVE_FILE "test.ntz"
#define PAGE_GAP 60
void AddPointToStroke(Stroke *stroke, Vector2 point){
    if(stroke->pointCount >= stroke->capacity){
        stroke->capacity = stroke->capacity == 0 ? 128 : stroke->capacity * 2;
        stroke->points = (Vector2 *)realloc(stroke->points, stroke->capacity * sizeof(Vector2));

    }
    stroke->points[stroke->pointCount++] = point;
}

void AddStrokeToPage(Page *page, Stroke stroke){
    if(page->strokeCount >= page->capacity){
        page->capacity = page->capacity == 0 ? 32 : page->capacity * 2;
        page->strokes = (Stroke *)realloc(page->strokes, page->capacity * sizeof(Stroke));

    }
    page->strokes[page->strokeCount++] = stroke;
}

void AddPageToDocument(Document *doc){
    if(doc->pageCount >= doc->pageCapacity){
        doc->pageCapacity = doc->pageCapacity == 0 ? 4 : doc->pageCapacity * 2;
        doc->pages = (Page*)realloc(doc->pages, doc->pageCapacity * sizeof(Page));
    }
    doc->pages[doc->pageCount] = (Page){0};
    doc->activePage = doc->pageCount;
    doc->pageCount++;
}
void FreeDocument(Document *doc){
    for(int p = 0; p < doc->pageCount;p++){
        for(int s = 0; s < doc->pages[p].strokeCount; s++){
            free(doc->pages[p].strokes[s].points);
        }
        free(doc->pages[p].strokes);
    }
    free(doc->pages);
    *doc = (Document){0};
}
void SaveDocumentBinary(const char *filename, Document *doc){
    FILE *file = fopen(filename, "wb");
    if (!file) return;

    char magic[3] = "NTZ";
    fwrite(magic, sizeof(char),3, file);
    fwrite(&doc->pageCount, sizeof(int),1,file);

    for(int p = 0; p < doc->pageCount; p++){
        Page *page = &doc->pages[p];
        fwrite(&page->strokeCount, sizeof(int),1,file);

        for(int s = 0; s< page->strokeCount;s++){
            Stroke *stroke = &page->strokes[s];
            fwrite(&stroke->color, sizeof(Color), 1, file);
            fwrite(&stroke->thickness, sizeof(float), 1, file);
            fwrite(&stroke->pointCount, sizeof(int), 1, file);

            fwrite(stroke->points, sizeof(Vector2), stroke->pointCount, file);
        }
    }
    fclose(file);
}

bool LoadDocumentBinary(const char *filename, Document *doc){
    FILE *file = fopen(filename, "rb");
    if (!file) return false;
    char magic[3];
    fread(magic, sizeof(char), 3, file);
    if(strstr(magic, "NTZ") ==NULL){
        fclose(file);
        return false;
    }

    FreeDocument(doc);
    int totalPages = 0;
    fread(&totalPages, sizeof(int), 1, file);

    for(int p = 0; p < totalPages; p++){
        AddPageToDocument(doc);
        Page *page = &doc->pages[p];

        int totalStrokes = 0;
        fread(&totalStrokes, sizeof(int), 1, file);

        for(int s = 0; s < totalStrokes; s++){
            Stroke stroke = {0};
            fread(&stroke.color, sizeof(Color), 1, file);
            fread(&stroke.thickness, sizeof(float), 1, file);
            fread(&stroke.pointCount, sizeof(int), 1, file);

            stroke.capacity = stroke.pointCount;
            stroke.points = (Vector2 *)malloc(stroke.capacity *sizeof(Vector2));
            fread(stroke.points,sizeof(Vector2), stroke.pointCount, file);

            AddStrokeToPage(page, stroke);
        }
    }

    doc->activePage = 0;
    fclose(file);
    return true;
}
void UndoLastStrokes(Page *activePage){
    if(activePage->strokeCount > 0){
        activePage->strokeCount--;

        free(activePage->strokes[activePage->strokeCount].points);
        activePage->strokes[activePage->strokeCount].points = NULL;
        activePage->strokes[activePage->strokeCount].pointCount = 0;
        activePage->strokes[activePage->strokeCount].capacity = 0;
    }
}
int main(void){
    const int screenWidth = 1400;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Notitze");
    SetWindowMinSize(800, 600);
    Document doc = {0};
    AddPageToDocument(&doc);

    Stroke currentStroke = {0};
    bool isDrawing = false;

    Color currentBrushColor = BLACK;
    float currentBrushThickness = 3.0f;

    //Vector2 canvasOffset = { (screenWidth - A4_WIDTH) / 2.0f, 20.0f};
    Camera2D camera = {0};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){ (GetScreenWidth() - A4_WIDTH) / 2.0f, 50.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    SetTargetFPS(120);

    while(!WindowShouldClose()){
        if(IsKeyPressed(KEY_N))
            AddPageToDocument(&doc);
        if(IsKeyPressed(KEY_RIGHT) && doc.activePage < doc.pageCount - 1)
            doc.activePage ++;
        if(IsKeyPressed(KEY_LEFT) && doc.activePage  > 0)
            doc.activePage --;
        if(IsKeyPressed(KEY_S))
            SaveDocumentBinary(SAVE_FILE, &doc);
        if(IsKeyPressed(KEY_L))
            LoadDocumentBinary(SAVE_FILE, &doc);
        if(IsKeyPressed(KEY_ONE)){
            currentBrushColor = BLACK;
            currentBrushThickness = 3.0f;
        }
        if(IsKeyPressed(KEY_TWO)){
            currentBrushColor = (Color){255,255,0,100};;
            currentBrushThickness = 20.0f;
        }
        if(IsKeyPressed(KEY_U)){
            UndoLastStrokes(&(doc.pages[doc.activePage]));
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            Vector2 delta = GetMouseDelta();
            camera.offset.x +=delta.x;
            camera.offset.y +=delta.y;
        }
        float wheel = GetMouseWheelMove();
        if(wheel != 0){
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;

            const float zoomIncrement = 0.125f;
            camera.zoom += (wheel * zoomIncrement);

            if (camera.zoom < 0.25f) camera.zoom = 0.25;
            if (camera.zoom > 5.0f) camera.zoom = 5.0f;
        }
        Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        int hoveredPage = (int)(mouseWorldPos.y / (A4_HEIGHT + PAGE_GAP));
        if(mouseWorldPos.y < 0) hoveredPage = -1;
        float localMouseY = mouseWorldPos.y - (hoveredPage *(A4_HEIGHT + PAGE_GAP));
        Vector2 localMousePos = {mouseWorldPos.x, localMouseY};

        bool isMouseInsideCanvas = (hoveredPage >= 0 && hoveredPage < doc.pageCount &&
                                    localMousePos.x >= 0 && localMousePos.x <= A4_WIDTH &&
                                    localMouseY >= 0 && localMouseY <= A4_HEIGHT);

        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && isMouseInsideCanvas && GetMouseY() > 40){
            isDrawing = true;
            doc.activePage = hoveredPage;
            currentStroke = (Stroke){0};
            currentStroke.color = currentBrushColor;
            currentStroke.thickness = currentBrushThickness;
            AddPointToStroke(&currentStroke,localMousePos);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isDrawing){
            if(currentStroke.pointCount > 0){
                Vector2 lastPoint = currentStroke.points[currentStroke.pointCount - 1];
                float distSq = (mouseWorldPos.x - lastPoint.x)*( mouseWorldPos.x - lastPoint.x) + (mouseWorldPos.y  - lastPoint.y)*(mouseWorldPos.y  - lastPoint.y);
                if(distSq > 9.0f && isMouseInsideCanvas){
                    AddPointToStroke(&currentStroke, localMousePos);
                }
            }
        }
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isDrawing){
            isDrawing = false;
            if(currentStroke.pointCount > 1){
                AddStrokeToPage(&doc.pages[doc.activePage], currentStroke);
            } else{
                free(currentStroke.points);
            }
            currentStroke = (Stroke){0};
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);
        for(int p = 0; p< doc.pageCount; p++){
            float pageYOffset = p * (A4_HEIGHT + PAGE_GAP);

            DrawRectangle(5, pageYOffset + 5, A4_WIDTH, A4_HEIGHT, BLACK);
            DrawRectangle(0, pageYOffset, A4_WIDTH, A4_HEIGHT, RAYWHITE);
            DrawRectangleLines(0, pageYOffset, A4_WIDTH, A4_HEIGHT, LIGHTGRAY);

            Page *page = &doc.pages[p];

            for(int i = 0; i< page->strokeCount; i++){
                Stroke *s = &page->strokes[i];
                for(int j = 0; j< s->pointCount - 1;j++){
                    Vector2 p1 = {s->points[j].x, s->points[j].y + pageYOffset};
                    Vector2 p2 = {s->points[j+1].x, s->points[j+1].y + pageYOffset};
                    DrawLineEx(p1, p2, s->thickness, s->color);
                    DrawCircleV(p1, s->thickness / 2.0f, s->color);
                }
                if(s->pointCount > 0){
                    Vector2 lastP = {s->points[s->pointCount - 1].x, s->points[s->pointCount - 1].y + pageYOffset};
                    DrawCircleV(lastP, s->thickness/2.0f, s->color);
                }
            }
            if(isDrawing && p == doc.activePage){
                for(int j = 0; j< currentStroke.pointCount - 1; j++){
                    Vector2 p1 = {currentStroke.points[j].x, currentStroke.points[j].y + pageYOffset};
                    Vector2 p2 = {currentStroke.points[j+1].x, currentStroke.points[j+1].y + pageYOffset};
                    DrawLineEx(p1, p2, currentStroke.thickness, currentStroke.color);
                    DrawCircleV(p1, currentStroke.thickness / 2.0f, currentStroke.color);
                }
            }
        }

        EndMode2D();
        DrawRectangle(0,0, GetScreenWidth(), 40, BLACK);
        DrawText(TextFormat("Page: %d/%d | [N] New | [< >] Switch| [S] Save | [L] Load | Right-Click: Pan Canvas", doc.activePage + 1, doc.pageCount), 20, 10, 20, WHITE);
        EndDrawing();
    }

    FreeDocument(&doc);
    CloseWindow();

    return 0;
}
