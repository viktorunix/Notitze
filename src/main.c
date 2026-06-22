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
#include "include/brush_system.h"
#include "include/command_system.h"
#include "include/renderer.h"
#include "include/viewport.h"
//#include "include/windows.h"
#define SAVE_FILE "test.ntz"
#define PAGE_GAP 60

Stroke currentStroke = {0};
Color currentBrushColor = BLACK;
float currentBrushThickness = 3.0f;

int main(void){
    InitCommandSystem();
    InitBrushSystem();
    RegisterBrush(CreatePenBrush());
    RegisterBrush(CreatePencilBrush());
    RegisterBrush(CreateHighlighterBrush());
    RegisterBrush(CreateLineBrush());
    RegisterBrush(CreateRectangleBrush());
    RegisterBrush(CreateCircleBrush());
    RegisterBrush(CreateEraserBrush());

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
    doc.isDrawing = false;

    int draggedPage=  -1;
    float dragOffsetY= 0.0f;
    Viewport vp = {0};
    InitViewport(&vp, doc.pageWidth);

    Color pallete[] = {BLACK, RED, DARKBLUE, DARKGREEN, PURPLE};



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

    InitRenderer(&doc);
    while(!WindowShouldClose()){

        
        if(isStartup){
            isStartup = startUpWindow(&doc,&vp.camera);
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

        currentBrushColor = pallete[settings.selectedColorIndex];
        currentBrushThickness = settings.currentBrushThickness;
        SetActiveBrush(doc.activeBrush);
        
        UpdateViewportMath(&vp, &doc, mousePos, guiClicked);
        
        if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !guiClicked){
            if(vp.isMouseInsideCanvas){
                doc.activePage = vp.hoveredPage;
                if(vp.localMousePos.y <= 40){
                    draggedPage = vp.hoveredPage;
                    dragOffsetY = vp.mouseWorldPos.y - (vp.hoveredPage * (doc.pageHeight + PAGE_GAP));
                } else{
                    currentPressure = 1.0f;
                    GetActiveBrush()->OnPress(&doc, vp.localMousePos, currentPressure);
                }
            } else{
                vp.isPanning = true;
            }
            
        }
        
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) || IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)){
            vp.isPanning = true;
        }
        

       
        if(draggedPage != -1){

        } else if(doc.isDrawing && IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            float clampedX = vp.mouseWorldPos.x;
            if(clampedX < 0.0f) clampedX = 0.0f;
            if(clampedX > doc.pageWidth) clampedX = doc.pageWidth;

            float clampedY = vp.localMousePos.y;
            if(clampedY < 40.0f) clampedY = 40.0f;
            if(clampedY > doc.pageHeight) clampedY = doc.pageHeight;
            Vector2 clampedPos = {clampedX, clampedY};

            if(doc.activeBrush <=BRUSH_PENCIL && currentStroke.pointCount > 0){
                float dist = Vector2Distance(vp.mouseWorldPos, currentStroke.points[currentStroke.pointCount - 1].pos);
                float targetPressure = 1.0f -(dist / 30.0f);
                if(targetPressure < 0.1f) targetPressure = 0.1f;
                if(targetPressure > 1.0f) targetPressure = 1.0f;
                currentPressure = (currentPressure * 0.7f) + (targetPressure * 0.3f);
            }
            GetActiveBrush()->OnDrag(&doc, clampedPos, currentPressure);

            if(!vp.isMouseInsideCanvas){
                GetActiveBrush()->OnRelease(&doc, clampedPos);
            }
        } else if(vp.isPanning){
            ApplyPanning(&vp);
        }

        
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
            if(draggedPage != -1){
               int dropIndex = (int)(vp.mouseWorldPos.y / (doc.pageHeight + PAGE_GAP));
               MovePageToIndex(&doc, draggedPage, dropIndex);
               draggedPage = -1;
            }
            if(doc.isDrawing){
                GetActiveBrush()->OnRelease(&doc, vp.localMousePos);
            }
            vp.isPanning = false;
        }
        

 


    RenderApplication(&doc, &settings, vp.camera, draggedPage, dragOffsetY, 
                      mousePos, vp.mouseWorldPos, vp.localMousePos, 
                      guiClicked, vp.isMouseInsideCanvas, &listeningForBind);
    }

    SaveSettings(settings);
    FreeDocument(&doc);
    CloseWindow();

    return 0;
}
