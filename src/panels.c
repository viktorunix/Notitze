#include "include/panels.h"
bool startUpWindow(Document *doc, Camera2D *camera){
    PaperFormat startFormat = doc->pageFormat;
    int startPPI = doc->ppi;
    int customW = doc->pageWidth;
    int customH = doc->pageHeight;
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

    DrawText("Canvas Size", boxX + 60, boxY + 310, 20, LIGHTGRAY);

    float calcW = 0 ;
    float calcH = 0;

    if(startFormat == FORMAT_A4){
        calcW = 8.27f * startPPI;
        calcH = 11.69f * startPPI;
    }
    if(startFormat == FORMAT_A3){
        calcW = 11.69f * startPPI;
        calcH = 16.54f * startPPI;
    }
    if(startFormat == FORMAT_A5){
        calcW = 5.83f * startPPI;
        calcH = 8.27f * startPPI;
    }
    if(startFormat == FORMAT_LETTER){
        calcW = 8.5f * startPPI;
        calcH = 11.0f * startPPI;
    }

    if(startFormat == FORMAT_CUSTOM){
        calcW = customW;
        calcH = customH;

        int cX = boxX + 60;
        DrawText("Width:", cX, boxY + 350, 20, WHITE);
        cX += 70;
        if(GUIButton((Rectangle){cX,boxY+340,40,40}, "-", false)) customW -=100;
        cX +=50;
        DrawText(TextFormat("%d px", customW), cX, boxY + 350, 20, WHITE);
        cX+=90;
        if(GUIButton((Rectangle){cX, boxY + 340, 40,40}, "+", false)) customW +=100;
        cX += 80;

        DrawText("Height:", cX, boxY + 350, 20, WHITE);
        cX+=80;
        if(GUIButton((Rectangle){cX, boxY + 340, 40, 40}, "-", false)) customH -=100;
        cX += 50;
        DrawText(TextFormat("%d px", customH), cX, boxY + 350, 20, WHITE);
        cX+=90;
        if(GUIButton((Rectangle){cX, boxY+340, 40,40}, "+", false)) customH +=100;
        
        if(customW < 100) customW = 100;
        if(customH < 100) customH = 100;
        doc->pageWidth = customW;
        doc->pageHeight = customH;

    } else {
        DrawText(TextFormat("%d px x %d px", (int)calcW, (int)calcH), boxX + 60, boxY + 350, 25, WHITE);
        doc->pageWidth = calcW;
        doc->pageHeight = calcH;
    }

    // action buttons
    if(GUIButton((Rectangle){boxX + boxW - 400, boxY + 430, 180,50}, "Load Document", false)){
        const char *path = ShowOpenFileDialog();
        if(path && LoadDocumentBinary(path, doc)){
            camera->offset.x = (GetScreenWidth() - doc->pageWidth) / 2.0f;
            return false;
        }
    }
    if(GUIButton((Rectangle){boxX + boxW - 200, boxY + 430, 180,50}, "Create Document", true)){
        
        AddPageToDocument(doc);
        camera->offset.x = (GetScreenWidth() - doc->pageWidth) / 2.0f;
        return false;
    }
    EndDrawing();
    doc->pageFormat = startFormat;
    doc->ppi = startPPI;
    return true;
}