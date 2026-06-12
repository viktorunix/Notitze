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

#define SAVE_FILE "test.ntz"
#define PAGE_GAP 60
#define UI_HEIGHT 60

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
    doc.pattern = BG_BLANK;
    BrushType activeBrush = BRUSH_PEN;
    Stroke currentStroke = {0};
    bool isDrawing = false;
    bool isPanning = false;
    bool showSettings = false;
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

        bool guiClicked = showSettings;

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
        if(mousePos.y < UI_HEIGHT && !showSettings){
            guiClicked = true;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                for(int i = 0; i < 5; i++){
                    int swatchX = GetScreenWidth() - 250 + (i * 45);
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
                    currentStroke.type = activeBrush;
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
            if(activeBrush == BRUSH_PEN || activeBrush == BRUSH_HIGHLIGHTER || activeBrush == BRUSH_PENCIL){
                if(currentStroke.pointCount > 0){
                    Vector2 lastPoint = currentStroke.points[currentStroke.pointCount - 1];
                    float distSq = (mouseWorldPos.x - lastPoint.x)*( mouseWorldPos.x - lastPoint.x) + (mouseWorldPos.y  - lastPoint.y)*(mouseWorldPos.y  - lastPoint.y);
                    if(distSq > 9.0f && hoveredPage == doc.activePage){
                        AddPointToStroke(&currentStroke, (Vector2){mouseWorldPos.x, localMouseY});
                    }
                }
                if(!isMouseInsideCanvas){
                    FinishStroke(&currentStroke, &doc);
                    //isDrawing = false;
                }
            } else {
                //shapes
                if(currentStroke.pointCount == 1){
                    AddPointToStroke(&currentStroke, (Vector2){mouseWorldPos.x, localMouseY});
                } else if (currentStroke.pointCount == 2){
                    currentStroke.points[1] = (Vector2){mouseWorldPos.x, localMouseY};
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
            DrawPageBackground(doc.pattern, pageYOffset);

            //paper drag bar
            DrawRectangle(0, pageYOffset, A4_WIDTH, 40, (Color){200,200,200,255});
            DrawText("|||", A4_WIDTH/2 - MeasureText("|||", 20)/2, pageYOffset + 10, 20, DARKGRAY);

            //active page highlight
            Color borderColor = (p == doc.activePage) ? SKYBLUE: LIGHTGRAY;
            int borderThickness = (p == doc.activePage) ? 4 : 1;
            DrawRectangleLinesEx((Rectangle){0, pageYOffset, A4_WIDTH, A4_HEIGHT}, borderThickness, borderColor);


            //strokes
            Page *page = &doc.pages[p];
            for(int i = 0; i < page->strokeCount; i++) 
                RenderStroke(&page->strokes[i], pageYOffset);
            // real-time stroke
            if(isDrawing && p == doc.activePage)
                RenderStroke(&currentStroke, pageYOffset);
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

        int toolX = 20;
        if(GUIButton((Rectangle){toolX, 10,60, 40}, "Pen", activeBrush == BRUSH_PEN)) activeBrush = BRUSH_PEN;
        if(GUIButton((Rectangle){toolX +=60, 10, 75, 40}, "Pencil", activeBrush == BRUSH_PENCIL)) activeBrush = BRUSH_PENCIL;
        if(GUIButton((Rectangle){toolX += 85, 10, 100, 40}, "Highlighter", activeBrush == BRUSH_HIGHLIGHTER)) activeBrush = BRUSH_HIGHLIGHTER;
        if(GUIButton((Rectangle){toolX += 110, 10, 60, 40}, "Line", activeBrush == BRUSH_LINE)) activeBrush = BRUSH_LINE;
        if(GUIButton((Rectangle){toolX += 70, 10, 70, 40}, "Rect", activeBrush == BRUSH_RECTANGLE)) activeBrush = BRUSH_RECTANGLE;
        if(GUIButton((Rectangle){toolX += 70, 10, 70, 40}, "Circle", activeBrush == BRUSH_CIRCLE)) activeBrush = BRUSH_CIRCLE;

        int centerX = GetScreenWidth() / 2;
        DrawText(TextFormat("Page %d/%d", doc.activePage + 1, doc.pageCount), centerX - 250, 20, 20, WHITE);

        if(GUIButton((Rectangle){centerX - 130, 10, 70, 40}, "New", false)) AddPageToDocument(&doc);
        if(GUIButton((Rectangle){centerX - 50, 10, 70, 40}, "Save", false)) {
            const char *savePath = ShowSaveFileDialog();
            if(savePath) SaveDocumentBinary(savePath, &doc);
        }
        if(GUIButton((Rectangle){centerX + 30, 10, 70, 40}, "Load", false)) {
            const char *openPath = ShowOpenFileDialog();
            if(openPath) LoadDocumentBinary(openPath, &doc);
        }
        if(GUIButton((Rectangle){centerX + 110, 10, 70, 40}, "Undo", false)) UndoLastStrokes(&doc.pages[doc.activePage]);
        if(GUIButton((Rectangle){centerX + 190, 10, 80, 40}, "Delete", false)) DeleteActivePage(&doc);

        if(GUIButton((Rectangle){centerX + 250, 10, 90, 40}, "Settings", showSettings)) showSettings = !showSettings;
        for(int i = 0; i<5;i++){
            int swatchX = GetScreenWidth() - 250 + (i * 45);
            Vector2 center = {swatchX + 20, UI_HEIGHT / 2.0f};


            if(i == selectedColorIndex){
                DrawCircleV(center, 20, WHITE);

            }
            DrawCircleV(center, 16, pallete[i]);
            DrawCircleLines(center.x, center.y, 16, (Color){0,0,0,100});
        }
        // settings page
        if(showSettings){
            DrawRectangle(0,0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0,150});

            int boxW = 400;
            int boxH = 300;
            int boxX = (GetScreenWidth() - boxW)/2;
            int boxY = (GetScreenHeight() - boxH)/2;

            DrawRectangleRounded((Rectangle){boxX, boxY, boxW, boxH}, 0.1f, 10, (Color){40, 40, 40, 255});
            DrawRectangleRoundedLinesEx((Rectangle){boxX, boxY, boxW, boxH}, 0.1f, 10, 2.0f, DARKGRAY);
            DrawText("Document Background", boxX + 60, boxY + 30, 25, WHITE);

            int btnX = boxX + 110;
            if(GUIButton((Rectangle){btnX, boxY+80, 180, 40}, "Blank", doc.pattern == BG_BLANK)) doc.pattern = BG_BLANK;
            if(GUIButton((Rectangle){btnX, boxY+130, 180, 40}, "Lined", doc.pattern == BG_LINED)) doc.pattern = BG_LINED;
            if(GUIButton((Rectangle){btnX, boxY+180, 180, 40}, "Grid", doc.pattern == BG_GRID)) doc.pattern = BG_GRID;
            if(GUIButton((Rectangle){btnX, boxY+230, 180, 40}, "Dots", doc.pattern == BG_DOTS)) doc.pattern =BG_DOTS;


        }
        EndDrawing();
    }

    FreeDocument(&doc);
    CloseWindow();

    return 0;
}
