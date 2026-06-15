#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "include/raylib.h"
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/document.h"
#include "include/memory.h"
#include "include/file_saving.h"
#include "include/doc_management.h"
#include "include/gui.h"
#include "include/settings.h"

#define SAVE_FILE "test.ntz"
#define PAGE_GAP 60


int main(void){
    const int screenWidth = 1400;
    const int screenHeight = 900;
   
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Notitze");
    SetExitKey(KEY_NULL);
    SetWindowMinSize(800, 600);
    Document doc = {0};
    AddPageToDocument(&doc);
    doc.enableLayers = false;
    doc.pattern = BG_BLANK;
    doc.activeBrush = BRUSH_PEN;
    Stroke currentStroke = {0};

    doc.isDrawing = false;
    bool isPanning = false;
    int draggedPage = -1;
    float dragOffsetY = 0.0f;
    Color pallete[] = {BLACK, RED, DARKBLUE, DARKGREEN, PURPLE};

    Color currentBrushColor = BLACK;


    Settings settings = {0};
    settings.showSettings = false;
    settings.currentBrushThickness = 3.0f;
    settings.selectedColorIndex = 0;
    settings.pallete = (Color*)malloc(5 * sizeof(Color));
    Color p[] = {BLACK, RED, DARKBLUE, DARKGREEN, PURPLE};
    memcpy(settings.pallete, p, 5 * sizeof(Color));
    settings.binds = (Keybinds){KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_S, KEY_L, KEY_U, KEY_DELETE};
    BindState listeningForBind = BIND_NONE;
    printf("DAAA\n");
    Camera2D camera = {0};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){ (GetScreenWidth() - A4_WIDTH) / 2.0f, 50.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    SetTargetFPS(120);


    int barWidth = 1000;
    int barHeight = 140;
    int barY = 20;
    printf("DAAA\n");
    //LoadSettings(&settings);
    while(!WindowShouldClose()){
        Vector2 mousePos = GetMousePosition();
        Vector2 mouseWorldPos = GetScreenToWorld2D(mousePos, camera);

        int barX = (GetScreenWidth() - barWidth / 2);
        Rectangle uiBounds = {(float)barX, (float)barY, (float)barWidth, (float)barHeight};
        bool guiClicked = settings.showSettings;
        bool layerHovered = false;
        if(doc.enableLayers && !settings.showSettings){
            int pH = 60 + (doc.pages[doc.activePage].layerCount * 40) + (doc.pages[doc.activePage].layerCount > 1 ? 50 : 0);
            Rectangle layerBounds = { (float)(GetScreenWidth() - 240), (float)(barY + barHeight + 20), 220, (float)pH};
            layerHovered = CheckCollisionPointRec(mousePos, layerBounds);
        }
        bool uiHovered = CheckCollisionPointRec(mousePos, uiBounds) || layerHovered;
        guiClicked = settings.showSettings || uiHovered;
        SettingsBinds(&listeningForBind, &settings);
        //InputHandler(&doc);
        InputHandler(&doc, &settings, &listeningForBind);

        // ui bar
        if(mousePos.y < UI_HEIGHT && !settings.showSettings){
            guiClicked = true;
            /*if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
                for(int i = 0; i < 5; i++){
                    int swatchX = GetScreenWidth() - 250 + (i * 45);
                    if(GetMouseX() >= swatchX && GetMouseX() <= swatchX + 30){
                        settings.selectedColorIndex = i;
                    }
                }
            }
                */
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
            if(doc.activeBrush == BRUSH_HIGHLIGHTER)
                pallete[settings.selectedColorIndex].a = 150;
            else
                pallete[settings.selectedColorIndex].a = 255;
            if(hoveredPage >= 0 && hoveredPage < doc.pageCount && mouseWorldPos.x >= 0 && mouseWorldPos.x <= A4_WIDTH){
                doc.activePage = hoveredPage;

                if(localMouseY <= 40){
                    draggedPage = hoveredPage;
                    dragOffsetY = mouseWorldPos.y - (hoveredPage * (A4_HEIGHT + PAGE_GAP));
                } else{
                    Layer *activeLayer = &doc.pages[hoveredPage].layers[doc.pages[hoveredPage].activeLayer];
                    if(activeLayer->isVisible){
                        doc.isDrawing = true;

                        currentStroke = (Stroke){0};
                        currentStroke.type = doc.activeBrush;
                        currentStroke.color = pallete[settings.selectedColorIndex];
                        currentStroke.thickness = settings.currentBrushThickness;
                        AddPointToStroke(&currentStroke, (Vector2){mouseWorldPos.x, localMouseY});
                    }
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

        } else if(doc.isDrawing && IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            if(doc.activeBrush == BRUSH_PEN || doc.activeBrush == BRUSH_HIGHLIGHTER || doc.activeBrush == BRUSH_PENCIL){
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
            if(doc.isDrawing){
                doc.isDrawing = false;
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
            //for(int i = 0; i < page->strokeCount; i++) 
            //    RenderStroke(&page->strokes[i], pageYOffset);
            for(int l = 0; l < page->layerCount; l++){
                Layer *layer = &page->layers[l];
                if(!layer->isVisible) continue;
                for(int i = 0; i < layer->strokeCount; i++)
                    RenderStroke(&layer->strokes[i], pageYOffset);

                if(doc.isDrawing && p == doc.activePage && l == page->activeLayer)
                    RenderStroke(&currentStroke, pageYOffset);
            }
        }
        // dragged page preview
        
        if(draggedPage != -1){
            float floatY = mouseWorldPos.y - dragOffsetY;
            DrawRectangle(15, floatY + 15, A4_WIDTH, A4_HEIGHT, (Color){0,0,0,100});
            DrawRectangle(0, floatY, A4_WIDTH, A4_HEIGHT, RAYWHITE);
            DrawPageBackground(doc.pattern, floatY);
            DrawRectangle(0, floatY, A4_WIDTH, 40, SKYBLUE);

            Page *page = &doc.pages[draggedPage];
            for(int l = 0; l < page->layerCount; l++){
                Layer *layer = &page->layers[l];
                if(!layer->isVisible) continue;
                for(int i = 0; i < layer->strokeCount; i++)
                    RenderStroke(&layer->strokes[i], floatY);
            }
        }

        EndMode2D();
        GUIHeaderDock(&doc, &settings, mousePos);
        // layer panel
        if(doc.enableLayers && !settings.showSettings){
            Page *aPage = &doc.pages[doc.activePage];
            int pW = 220;
            int pH = 60 + (aPage->layerCount * 45) + (aPage->layerCount > 1 ? 50 : 0);
            int pX = GetScreenWidth() - pW - 20;
            int pY = barY + barHeight + 20;

            DrawRectangleRounded((Rectangle){pX + 5, pY + 5, pW, pH}, 0.1f, 10, (Color){0,0,0,100});
            DrawRectangleRounded((Rectangle){pX, pY, pW, pH}, 0.1f, 10, (Color){35,35,40, 245});
            DrawRectangleRoundedLinesEx((Rectangle){pX, pY, pW, pH}, 0.1f, 10, 2.0f, (Color){60,60,65,255});

            DrawText("Layers", pX + 20, pY + 18, 20, WHITE);
            if(GUIButton((Rectangle){pX + pW - 50, pY + 10, 35,35}, "+",  false))AddLayerToPage(aPage);

            int lY = pY + 60;

            for(int l = aPage->layerCount - 1; l >=0; l--){
                Layer *layer = &aPage->layers[l];
                if(GUIButton((Rectangle){pX+10, lY, 40, 35}, layer->isVisible ? "O" : "-", layer->isVisible))
                    layer->isVisible = !layer->isVisible;
                if(GUIButton((Rectangle){pX+55, lY, pW- 65, 35}, TextFormat("Layer %d", l + 1), aPage->activeLayer == l))
                    aPage->activeLayer = l;
                lY+= 45;
            }
            if(aPage->layerCount > 1){
                if(GUIButton((Rectangle){pX + 10, lY + 5, pW - 20, 35}, "Delete Layer", false))
                    DeleteActiveLayer(aPage);
            }
        }
        // settings page
        if(settings.showSettings){
            SettingsPage(&doc, &settings, &listeningForBind);

        }
        EndDrawing();
    }

    SaveSettings(settings);
    FreeDocument(&doc);
    CloseWindow();

    return 0;
}
