#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "include/raylib.h"
#include "raylib.h"
#include "raymath.h"
#include "include/document.h"
#include "include/memory.h"
#include "include/file_saving.h"
#include "include/doc_management.h"
#include "include/gui.h"
#define A4_WIDTH 842
#define A4_HEIGHT 1191
#define SAVE_FILE "test.ntz"
#define PAGE_GAP 60


void InputHandler(Document *doc){
    if(IsKeyPressed(KEY_N))
        AddPageToDocument(doc);
    if(IsKeyPressed(KEY_RIGHT) && doc->activePage < doc->pageCount - 1)
        doc->activePage ++;
    if(IsKeyPressed(KEY_LEFT) && doc->activePage  > 0)
        doc->activePage --;
    if(IsKeyPressed(KEY_S))
        SaveDocumentBinary(SAVE_FILE, doc);
    if(IsKeyPressed(KEY_L))
        LoadDocumentBinary(SAVE_FILE, doc);
    if(IsKeyPressed(KEY_U))
        UndoLastStrokes(&(doc->pages[doc->activePage]));
    if(IsKeyPressed(KEY_DELETE))
        DeleteActivePage(doc);
    if(IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_UP))
        MoveActivePageUp(doc);
    if(IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_DOWN))
        MoveActivePageDown(doc);
    
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
    bool isPanning = false;
    int draggedPage = -1;
    float dragOffsetY = 0.0f;
    Color pallete[] = {BLACK, RED, DARKBLUE, DARKGREEN, PURPLE};
    int selectedColorIndex = 0;
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
        Vector2 mousePos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);

        bool guiClicked = false;

        InputHandler(&doc);

        if(IsKeyPressed(KEY_ONE)){
            currentBrushColor = BLACK;
            currentBrushThickness = 3.0f;
        }
        if(IsKeyPressed(KEY_TWO)){
            currentBrushColor = (Color){255,255,0,100};;
            currentBrushThickness = 20.0f;
        }
        // ui bar
        if(mousePos.y < 50){
            guiClicked = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                for(int i = 0; i < 5; i++){
                    int swatchX = 600 + (i * 40);
                    if(GetMouseX() >= swatchX && GetMouseX() <= swatchX + 30){
                        selectedColorIndex = i;
                    }
                }
            }
        }
        
        // camera zoom
        float wheel = GetMouseWheelMove();
        if(wheel != 0 && !guiClicked){
            camera.offset = mousePos;
            camera.target = mouseWorldPos;
            camera.zoom += (wheel * 0.125f);
            if(camera.zoom < 0.25f) camera.zoom = 0.25f;
            if(camera.zoom > 5.0f) camera.zoom = 5.0f;
        }

        //drag, drom and drawing
        int hoveredPage = (int)(mouseWorldPos.y / (A4_HEIGHT + PAGE_GAP));
        if(mouseWorldPos.y < 0) hoveredPage = -1;
        float localMouseY = mouseWorldPos.y - (hoveredPage * (A4_HEIGHT + PAGE_GAP));


        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !guiClicked){
            if(hoveredPage >= 0 && hoveredPage < doc.pageCount && mouseWorldPos.x >= 0 && mouseWorldPos.x <= A4_WIDTH){
                doc.activePage = hoveredPage;

                if(localMouseY <= 40){
                    draggedPage = hoveredPage;
                    dragOffsetY = mouseWorldPos.y - (hoveredPage * (A4_HEIGHT + PAGE_GAP));
                } else{
                    isDrawing = true;
                    currentStroke = (Stroke){0};
                    currentStroke.color = pallete[selectedColorIndex];
                    currentStroke.thickness = currentBrushThickness;
                    AddPointToStroke(&currentStroke, (Vector2){mouseWorldPos.x, localMouseY});
                }
            } else{
                isPanning = true;
            }
        }
        
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)){
            isPanning = true;
        }
        

        Vector2 localMousePos = {mouseWorldPos.x, localMouseY};

        bool isMouseInsideCanvas = (hoveredPage >= 0 && hoveredPage < doc.pageCount &&
                                    localMousePos.x >= 0 && localMousePos.x <= A4_WIDTH &&
                                    localMouseY >= 0 && localMouseY <= A4_HEIGHT);

        if(draggedPage != -1){

        } else if(isDrawing && IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            if(currentStroke.pointCount > 0){
                Vector2 lastPoint = currentStroke.points[currentStroke.pointCount - 1];
                float distSq = (mouseWorldPos.x - lastPoint.x)*( mouseWorldPos.x - lastPoint.x) + (mouseWorldPos.y  - lastPoint.y)*(mouseWorldPos.y  - lastPoint.y);
                if(distSq > 9.0f && hoveredPage == doc.activePage){
                    AddPointToStroke(&currentStroke, (Vector2){mouseWorldPos.x, localMouseY});
                }
                if(!isMouseInsideCanvas){
                    FinishStroke(&currentStroke, &doc);
                    //isDrawing = false;
                }
            }
        } else if(isPanning){
            Vector2 delta = GetMouseDelta();
            camera.offset.x += delta.x;
            camera.offset.y += delta.y;
        }
        
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
            if(draggedPage != -1){
               int dropIndex = (int)(mouseWorldPos.y / (A4_HEIGHT + PAGE_GAP));
               MovePageToIndex(&doc, draggedPage, dropIndex);
               draggedPage = -1;
            }
            if(isDrawing){
                isDrawing = false;
                FinishStroke(&currentStroke, &doc);
            }
            isPanning = false;
        }
        

 

        BeginDrawing();
        ClearBackground(DARKGRAY);

        BeginMode2D(camera);
        for(int p = 0; p< doc.pageCount; p++){

            if(p == draggedPage) continue;
            float pageYOffset = p * (A4_HEIGHT + PAGE_GAP);

            //paper
            DrawRectangle(5, pageYOffset + 5, A4_WIDTH, A4_HEIGHT, BLACK);
            DrawRectangle(0, pageYOffset, A4_WIDTH, A4_HEIGHT, RAYWHITE);

            //paper drag bar
            DrawRectangle(0, pageYOffset, A4_WIDTH, 40, (Color){200,200,200,255});
            DrawText("|||", A4_WIDTH/2 - MeasureText("|||", 20)/2, pageYOffset + 10, 20, DARKGRAY);

            //active page highlight
            Color borderColor = (p == doc.activePage) ? SKYBLUE: LIGHTGRAY;
            int borderThickness = (p == doc.activePage) ? 4 : 1;
            DrawRectangleLinesEx((Rectangle){0, pageYOffset, A4_WIDTH, A4_HEIGHT}, borderThickness, borderColor);


            //strokes
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
            // real-time stroke
            if(isDrawing && p == doc.activePage){
                for(int j = 0; j< currentStroke.pointCount - 1; j++){
                    Vector2 p1 = {currentStroke.points[j].x, currentStroke.points[j].y + pageYOffset};
                    Vector2 p2 = {currentStroke.points[j+1].x, currentStroke.points[j+1].y + pageYOffset};
                    DrawLineEx(p1, p2, currentStroke.thickness, currentStroke.color);
                    DrawCircleV(p1, currentStroke.thickness / 2.0f, currentStroke.color);
                }
            }
        }
        // dragged page preview
        if(draggedPage != -1){
            float floatY = mouseWorldPos.y - dragOffsetY;
            DrawRectangle(15, floatY + 15, A4_WIDTH, A4_HEIGHT, (Color){0,0,0,100});
            DrawRectangle(0, floatY, A4_WIDTH, A4_HEIGHT, RAYWHITE);
            DrawRectangle(0, floatY, A4_WIDTH, 40, SKYBLUE);
            DrawRectangleLinesEx((Rectangle){0, floatY, A4_WIDTH, A4_HEIGHT}, 4, SKYBLUE);

            Page *page = &doc.pages[draggedPage];
            for (int i = 0; i < page->strokeCount; i++){
                Stroke *s = &page->strokes[i];
                for(int j = 0; j < s->pointCount - 1; j++){
                    Vector2 p1 = {s->points[j].x, s->points[j].y + floatY};
                    Vector2 p2 = {s->points[j+1].x, s->points[j+1].y + floatY};
                    DrawLineEx(p1, p2, s->thickness, s->color);
                    DrawCircleV(p1, s->thickness / 2.0f, s->color);
                }
            }
        }

        EndMode2D();
        DrawRectangle(0,0, GetScreenWidth(), 50, (Color){50,50,50,255});
        DrawText(TextFormat("Page %d/%d", doc.activePage + 1, doc.pageCount),10, 15, 20, WHITE);

        if(GUIButton((Rectangle){120, 10, 100, 30}, "New Page"))
            AddPageToDocument(&doc);
        if(GUIButton((Rectangle){230, 10, 80, 30}, "Save"))
            SaveDocumentBinary(SAVE_FILE, &doc);
        if(GUIButton((Rectangle){320, 10, 80, 30}, "Load"))
            LoadDocumentBinary(SAVE_FILE, &doc);
        if(GUIButton((Rectangle){410, 10, 80, 30}, "Undo"))
            UndoLastStrokes(&doc.pages[doc.activePage]);
        if(GUIButton((Rectangle){500, 10, 80, 30}, "Delete"))
            DeleteActivePage(&doc);

        for(int i = 0; i<5;i++){
            int swatchX = GetScreenWidth() - 200 + (i * 40);


            if(i == selectedColorIndex){
                DrawRectangle(swatchX - 2, 8,34,34, WHITE);

            }
            DrawRectangle(swatchX, 10, 30, 30, pallete[i]);
        }
        EndDrawing();
    }

    FreeDocument(&doc);
    CloseWindow();

    return 0;
}
