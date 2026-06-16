#include "include/windows.h"

void startUpWindow(){
    PaperFormat startFormat = FORMAT_A4;
    int startPPI = 96;
    BeginDrawing();
    ClearBackground((Color){20,20,25,255});

    int boxW = 800;
    int boxH = 500;
    
    int boxX = (GetScreenWidth() - boxW) / 2;
    int boxY = (GetScreenHeight() - boxH) / 2;

    DrawRectangleRounded((Rectangle){boxX, boxY, boxW, boxH}, 0.05f, 10, (Color){35,35,40,255});
    DrawRectangleRoundedLinesEx((Rectangle){boxX, boxY, boxW, boxH}, 0.05f, 10, 2.0f, (Color){60,60,65,255});

    DrawText("Notitze 0.3V", boxX + boxW / 2 - MeasureText("Notitze 0.3V", 30)/ 2, boxY + 40, 30, WHITE);

    //paper format
    DrawText("Paper Format", boxX + 60, boxY + 110,20, LIGHTGRAY);
    int pX = boxX + 60;
    if(GUIButton((Rectangle){pX, boxY + 140, 100,40}, "A3", startFormat == FORMAT_A3)) startFormat = FORMAT_A3;
    pX +=115;
    if(GUIButton((Rectangle){pX, boxY + 140, 100, 40}, "A4", startFormat == FORMAT_A4 )) startFormat = FORMAT_A4;
    pX +=115;
    if(GUIButton((Rectangle){pX, boxY+140, 100, 40}, "A5", startFormat == FORMAT_A5)) startFormat = FORMAT_A5;
    pX +=115;
    if(GUIButton((Rectangle){pX, boxY+140, 100, 40}, "Letter", startFormat == FORMAT_LETTER)) startFormat = FORMAT_LETTER;
    pX+= 115;
    if(GUIButton((Rectangle){pX, boxY+140, 120, 40}, "Custom", startFormat == FORMAT_CUSTOM)) startFormat = FORMAT_CUSTOM;

    //resolution
    DrawText("Resolution (PPI)", boxX + 60, boxY + 210, 20, LIGHTGRAY);
    int rX = boxX + 60;
    if(GUIButton((Rectangle){rX, boxY+240, 100, 40}, "72", startPPI == 72)) startPPI = 72;
    rX += 115;
    if(GUIButton((Rectangle){rX, boxY+240, 100, 40}, "96",startPPI == 96)) startPPI = 96;
    rX += 115;
    if(GUIButton((Rectangle){rX, boxY+240, 100, 40}, "150", startPPI == 150)) startPPI = 150;
    rX +=115;
    if(GUIButton((Rectangle){rX, boxY+240,100, 40}, "300", startPPI == 300)) startPPI = 300;
    
}