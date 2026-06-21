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
#include "include/panels.h"
//#include "include/windows.h"
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
    doc.renderScale = 2.0f;
    doc.useBakedRendering = true;
    //AddPageToDocument(&doc);
    doc.enableLayers = false;
    doc.pattern = BG_BLANK;
    doc.activeBrush = BRUSH_PEN;
    doc.pageFormat = FORMAT_A4;
    doc.ppi = START_PPI;
    doc.pageWidth = CUSTOM_W;
    doc.pageHeight = CUSTOM_H;
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
    camera.offset = (Vector2){ (GetScreenWidth() - doc.pageWidth) / 2.0f, 50.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    SetTargetFPS(120);


    int barWidth = 1000;
    int barHeight = 140;
    int barY = 20;
    //LoadSettings(&settings);


    float currentPressure = 1.0f;
    bool isStartup = true;

    //brush circle texture
    //Image brushImage = GenImageGradientRadial(64,64,0.0f, WHITE, BLANK);
    Image brushImage = GenImageColor(256,256, BLANK);
    for(int y = 0; y <256; y++){
        for(int x = 0; x < 256; x++){
            float dist = Vector2Distance((Vector2){x + 0.5f, y+0.5f}, (Vector2){128.0f, 128.0f});

            if(dist <=126.0f){
                ImageDrawPixel(&brushImage, x, y, WHITE);

            }else if(dist <= 128.0f){
                float alpha = (128.0f - dist) / 2.0f;
                Color c = WHITE;
                c.a = (unsigned char)(alpha * 255.0f);
                ImageDrawPixel(&brushImage, x, y, c);
            }
        }
    }
    Texture2D softBrushTex = LoadTextureFromImage(brushImage);
    UnloadImage(brushImage);
    SetTextureFilter(softBrushTex, TEXTURE_FILTER_BILINEAR);
    doc.brushTex = softBrushTex;
    while(!WindowShouldClose()){

        
        if(isStartup){
            isStartup = startUpWindow(&doc,&camera);
            continue;
        }
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
        InputHandler(&doc, &settings, &listeningForBind);

        // ui bar
        if(mousePos.y < UI_HEIGHT && !settings.showSettings){
            guiClicked = true;
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
        int hoveredPage = (int)(mouseWorldPos.y / (doc.pageHeight + PAGE_GAP));
        if(mouseWorldPos.y < 0) hoveredPage = -1;
        float localMouseY = mouseWorldPos.y - (hoveredPage * (doc.pageHeight + PAGE_GAP));


        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !guiClicked){
            if(doc.activeBrush == BRUSH_HIGHLIGHTER)
                pallete[settings.selectedColorIndex].a = 150;
            else
                pallete[settings.selectedColorIndex].a = 255;
            if(hoveredPage >= 0 && hoveredPage < doc.pageCount && mouseWorldPos.x >= 0 && mouseWorldPos.x <= doc.pageWidth){
                doc.activePage = hoveredPage;

                if(localMouseY <= 40){
                    draggedPage = hoveredPage;
                    dragOffsetY = mouseWorldPos.y - (hoveredPage * (doc.pageHeight + PAGE_GAP));
                } else{
                    Layer *activeLayer = &doc.pages[hoveredPage].layers[doc.pages[hoveredPage].activeLayer];
                    if(activeLayer->isVisible){
                        doc.isDrawing = true;
                        currentPressure = 1.0f;
                        currentStroke = (Stroke){0};
                        currentStroke.type = doc.activeBrush;
                        currentStroke.color = pallete[settings.selectedColorIndex];
                        currentStroke.thickness = settings.currentBrushThickness;
                        AddPointToStroke(&currentStroke, (Vector2){mouseWorldPos.x, localMouseY}, currentPressure);
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
                                    localMousePos.x >= 0 && localMousePos.x <= doc.pageWidth &&
                                    localMouseY >= 0 && localMouseY <= doc.pageHeight);

        if(draggedPage != -1){

        } else if(doc.isDrawing && IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            float clampedX = mouseWorldPos.x;
            if(clampedX < 0.0f) clampedX = 0.0f;
            if(clampedX > doc.pageWidth) clampedX = doc.pageWidth;

            float clampedY = localMouseY;
            if(clampedY < 40.0f) clampedY = 40.0f;
            if(clampedY > doc.pageHeight) clampedY = doc.pageHeight;
            if(doc.activeBrush == BRUSH_PEN || doc.activeBrush == BRUSH_HIGHLIGHTER || doc.activeBrush == BRUSH_PENCIL){
                if(currentStroke.pointCount > 0){
                    Vector2 lastPoint = currentStroke.points[currentStroke.pointCount - 1].pos;
                    //float distSq = (mouseWorldPos.x - lastPoint.x)*( mouseWorldPos.x - lastPoint.x) + (mouseWorldPos.y  - lastPoint.y)*(mouseWorldPos.y  - lastPoint.y);
                    float dist = Vector2Distance(mouseWorldPos, lastPoint);
                    

                    //if moving >30px/frame, pressure drops to 0.1f
                    float targetPressure = 1.0f - (dist / 30.0f);
                    if(targetPressure < 0.1f) targetPressure = 0.1f;
                    if(targetPressure > 1.0f) targetPressure = 1.0f;

                    currentPressure = (currentPressure * 0.7f) + (targetPressure * 0.3f);

                    if((dist * dist) > 4.0f && hoveredPage == doc.activePage){
                        Vector2 smoothPoint = {
                            lastPoint.x + (mouseWorldPos.x - lastPoint.x) * 0.75f,
                            lastPoint.y + (localMouseY - lastPoint.y) * 0.75f
                        };
                        AddPointToStroke(&currentStroke, smoothPoint, currentPressure);
                    }
                }
                if(!isMouseInsideCanvas){
                    FinishStroke(&currentStroke, &doc);
                }
            } else {
                //shapes
                if(currentStroke.pointCount == 1){
                    AddPointToStroke(&currentStroke, (Vector2){clampedX, clampedY},1.0f);
                } else if (currentStroke.pointCount == 2){
                    currentStroke.points[1].pos = (Vector2){clampedX, clampedY};
                }
            }
        } else if(isPanning){
            Vector2 delta = GetMouseDelta();
            camera.offset.x += delta.x;
            camera.offset.y += delta.y;
        }
        
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
            if(draggedPage != -1){
               int dropIndex = (int)(mouseWorldPos.y / (doc.pageHeight + PAGE_GAP));
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
            float pageYOffset = p * (doc.pageHeight + PAGE_GAP);
            GUIPage(&doc, &currentStroke, p, pageYOffset);
            
        }
        // dragged page preview
        
        if(draggedPage != -1){
            float floatY = mouseWorldPos.y - dragOffsetY;
            DrawRectangle(15, floatY + 15, doc.pageWidth, doc.pageHeight, (Color){0,0,0,100});
            DrawRectangle(0, floatY, doc.pageWidth, doc.pageHeight, RAYWHITE);
            DrawPageBackground(&doc, doc.pattern, floatY);
            DrawRectangle(0, floatY, doc.pageWidth, 40, SKYBLUE);

            Page *page = &doc.pages[draggedPage];
            for(int l = 0; l < page->layerCount; l++){
                Layer *layer = &page->layers[l];
                if(!layer->isVisible) continue;

                Rectangle source = {0, 0, (float) layer->texture.texture.width, -(float)layer->texture.texture.height};
                Rectangle destination = {0, floatY, doc.pageWidth, doc.pageHeight};
                BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
                DrawTexturePro(layer->texture.texture, source, destination, (Vector2){0,0}, 0.0f, WHITE);
                EndBlendMode();
            }
        }

        EndMode2D();
        GUIHeaderDock(&doc, &settings, mousePos);
        // layer panel
        if(doc.enableLayers && !settings.showSettings){
            GUILayerPanel(&doc, currentStroke);
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
