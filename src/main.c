#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "include/raylib.h"
#include "include/raylib.h"
#include "include/document.h"
#include "include/memory.h"

#include "include/gui.h"
#include "include/settings.h"
#include "include/brush_system.h"
#include "include/command_system.h"
#include "include/renderer.h"
#include "include/viewport.h"
#include "include/input_manager.h"
#include "include/tablet_support.h"
#include "include/doc_management.h"
#include "include/menu.h"

#include "include/memory_arena.h"
#define SAVE_FILE "test.ntz"
#define PAGE_GAP 60



Stroke currentStroke = {0};
Color currentBrushColor = BLACK;
float currentBrushThickness = 3.0f;


bool UpdateEditorState(Document *doc, Settings *settings, Viewport *vp,
int *draggedPage, float *dragOffsetY, float *currentPressure, BindState *listeningForBind, Color *pallete){
    Vector2 mousePos = GetMousePosition();

    int barWidth = 1000;
    int barHeight = 140;
    int barY = 20;
    int barX = (GetScreenWidth() - barWidth) / 2;

    Rectangle uiBounds = {(float)barX, (float)barY, (float)barWidth, (float)barHeight};
    bool guiClicked = settings->showSettings;
    bool layerHovered = false;

    //verifying if the layer panel was clicked
    if(doc->enableLayers && !settings->showSettings){
        int pH = 60 + (doc->pages[doc->activePage].layerCount * 40) + (doc->pages[doc->activePage].layerCount > 1 ? 50 : 0);
        Rectangle layerBounds = {(float)(GetScreenWidth() - 240), (float)(barY + barHeight + 20), 220, (float)pH};
        layerHovered = CheckCollisionPointRec(mousePos, layerBounds);
    }
    bool uiHovered = CheckCollisionPointRec(mousePos, uiBounds) || layerHovered;
    guiClicked = settings->showSettings || uiHovered;

    //updating shortcuts and input
    SettingsBinds(listeningForBind, settings);
    InputHandler(doc, settings, listeningForBind);

    //preventing accidental drawing uner the tool docker
    if(mousePos.y < (barY + barHeight) && !settings->showSettings){
        guiClicked = true;
    }
    //updating the active brush
    currentBrushColor = pallete[settings->selectedColorIndex];
    SetActiveBrush(doc->activeBrush);
    settings->currentBrushThicknessIndex = doc->activeBrush;
    settings->currentBrushColorsIndex = doc->activeBrush;
    currentBrushThickness = settings->currentBrushThicknesses[settings->currentBrushThicknessIndex];
    currentBrushColor = settings->pallete[settings->currentBrushColors[settings->currentBrushColorsIndex]];

    //sending the state of viewport and the drawing process
    UpdateViewportMath(vp, doc, mousePos, guiClicked);
    InputState state = {0};
    PollInputState(&state, settings, vp->camera, guiClicked);
    ProcessInputs(doc, vp, &state, draggedPage, dragOffsetY);
    return guiClicked;
}
int main(void){

    InitArena(&activeStrokeArena, 2 * 1024 * 1024);
    InitCommandSystem();
    InitBrushSystem();
    RegisterBrushes();

    const int screenWidth = 1400;
    const int screenHeight = 900;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Notitze");
    InitTabletSupport();
    SetExitKey(KEY_NULL);
    SetWindowMinSize(800, 600);
    Document *doc = CreateEmptyDocument();
    int draggedPage=  -1;
    float dragOffsetY= 0.0f;
    Viewport vp = {0};
    InitViewport(&vp, doc->pageWidth);

    Color pallete[] = {BLACK, RED, DARKBLUE, DARKGREEN, PURPLE};



    Settings settings = {0};
    settings.showSettings = false;
    settings.currentBrushThickness = 3.0f;
    settings.selectedColorIndex = 0;
    settings.pallete = (Color*)malloc(5 * sizeof(Color));
    Color p[] = {BLACK, RED, DARKBLUE, DARKGREEN, PURPLE};
    memcpy(settings.pallete, p, 5 * sizeof(Color));

    settings.currentBrushThicknesses = (float*)malloc(BRUSH_NUMBER * sizeof(float));
    settings.currentBrushColors = (int*)malloc(BRUSH_NUMBER * sizeof(int));

    for(int i = 0; i < BRUSH_NUMBER; i++){
        settings.currentBrushThicknesses[i] = 5.0f;
        settings.currentBrushColors[i] = 0;
    }
    settings.currentBrushThicknessIndex = 0;
    settings.currentBrushColorsIndex = 0;


    settings.binds = (Keybinds){KEY_ONE, KEY_TWO, KEY_THREE, KEY_FOUR, KEY_FIVE, KEY_S, KEY_L, KEY_U, KEY_DELETE};
    BindState listeningForBind = BIND_NONE;

    Camera2D camera = {0};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){ (GetScreenWidth() - doc->pageWidth) / 2.0f, 50.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    int frameSinceLastActivity = 0;
    const int idleFPS = 15;
    const int activeFPS = 120;
    //SetTargetFPS(120);


    int barWidth = 1000;
    int barHeight = 140;
    int barY = 20;
    LoadSettings(&settings);


    float currentPressure = 1.0f;
    bool isStartup = true;

    InitRenderer(doc);

    AppState appState = STATE_MENU;
    InitMainMenu();

    while(!WindowShouldClose()){
        bool hardwareActivity = (
            GetMouseDelta().x != 0.0f || GetMouseDelta().y !=0.0f ||
            GetMouseWheelMove() != 0.0f ||
            GetKeyPressed() !=0 || GetCharPressed() !=0 ||
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) ||
            IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE) ||
            doc->isDrawing || draggedPage != -1 || settings.showSettings
        );

        bool cursorBlinking = (doc->activeBrush == BRUSH_TEXT && doc->isDrawing);

        if(hardwareActivity || cursorBlinking || isStartup){
            isStartup = false;
            frameSinceLastActivity = 0;
            SetTargetFPS(activeFPS);
        } else{
            frameSinceLastActivity++;
            if(frameSinceLastActivity > activeFPS)
                SetTargetFPS(idleFPS);
        }
        BeginDrawing();
        ClearBackground(DARKGRAY);
        if(appState == STATE_MENU){
            if(UpdateDrawMainMenu(doc)){
                appState = STATE_EDITOR;
                InitViewport(&vp, doc->pageWidth);
            }
        }
        else{
            bool guiClicked = UpdateEditorState(doc, &settings, &vp, &draggedPage, &dragOffsetY, &currentPressure, &listeningForBind, pallete);
        
            RenderWorkspace(doc, &settings, vp.camera, draggedPage, dragOffsetY,
            vp.mouseWorldPos, guiClicked, vp.isMouseInsideCanvas);
            RenderUI(doc, &settings, GetMousePosition(), &listeningForBind, &appState);
            if(appState == STATE_MENU){
                InitMainMenu();
            }
        }
        EndDrawing();
    }
    SaveSettings(settings);
    FreeDocument(doc);
    FreeArena(&activeStrokeArena);
    CloseWindow();

    return 0;
}
