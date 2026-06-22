#include "include/settings.h"

const char* KeyToString(int key){
    if (key == 0) return "NONE";
    switch (key){
        case KEY_ENTER: return "ENTR";
        case KEY_SPACE: return "SPCE";
        case KEY_ESCAPE: return "ESC";
        case KEY_DELETE: return "DEL";
        case KEY_BACKSPACE: return "BKSP";
        case KEY_UP: return "UP";
        case KEY_DOWN: return "DOWN";
        case KEY_LEFT: return "LEFT";
        case KEY_RIGHT:return "RIGHT";
        case KEY_LEFT_SHIFT: return "LSHF";
        case KEY_LEFT_CONTROL: return "LCTRL";
        case KEY_LEFT_ALT: return "LALT";
        default:
            if(key >=32 && key <= 126)
                return TextFormat("%c", key);
                return "?";
    }
}
void SettingsBinds(BindState *listeningForBind, Settings *settings){
    if(*listeningForBind != BIND_NONE){
        int pressedKey = GetKeyPressed();
        if(pressedKey != 0){
            if(pressedKey == KEY_ESCAPE) pressedKey = 0;

            switch(*listeningForBind){
                case BIND_PEN: settings->binds.keyPen = pressedKey; break;
                case BIND_HIGH: settings->binds.keyHigh = pressedKey; break;
                case BIND_LINE: settings->binds.keyLine = pressedKey; break;
                case BIND_RECT: settings->binds.keyRect = pressedKey; break;
                case BIND_CIRCLE: settings->binds.keyCircle = pressedKey; break;
                case BIND_SAVE: settings->binds.keySave = pressedKey; break;
                case BIND_LOAD: settings->binds.keyLoad = pressedKey; break;
                case BIND_UNDO: settings->binds.keyUndo = pressedKey; break;
                case BIND_DEL: settings->binds.keyDel = pressedKey; break;
                default: break;
            }
            *listeningForBind = BIND_NONE;
        }
    }
}
void InputHandler(Document *doc, Settings *settings, BindState *listeningForBind){
    if(!settings->showSettings && *listeningForBind == BIND_NONE){
        if(IsKeyPressed(settings->binds.keyPen) && settings->binds.keyPen != 0) doc->activeBrush = BRUSH_PEN;
        if(IsKeyPressed(settings->binds.keyHigh) && settings->binds.keyHigh != 0) doc->activeBrush = BRUSH_HIGHLIGHTER;
        if(IsKeyPressed(settings->binds.keyLine) && settings->binds.keyLine != 0) doc->activeBrush = BRUSH_LINE;
        if(IsKeyPressed(settings->binds.keyRect) && settings->binds.keyRect != 0) doc->activeBrush = BRUSH_RECTANGLE;
        if(IsKeyPressed(settings->binds.keyCircle) && settings->binds.keyCircle != 0) doc->activeBrush = BRUSH_CIRCLE;

        if(IsKeyPressed(settings->binds.keySave) && settings->binds.keySave != 0){
            const char *path = ShowSaveFileDialog();
            if(path) SaveDocumentBinary(path, doc);
        }
        if(IsKeyPressed(settings->binds.keyLoad) && settings->binds.keyLoad != 0){
            const char *path = ShowOpenFileDialog();
            if(path) LoadDocumentBinary(path, doc);
        }
        if(IsKeyPressed(settings->binds.keyUndo) && settings->binds.keyUndo != 0) UndoLastStrokes(&doc->pages[doc->activePage].layers[doc->pages[doc->activePage].activeLayer], doc->renderScale, doc->pressureEnabled,*doc);
        if(IsKeyPressed(settings->binds.keyDel) && settings->binds.keyDel != 0) DeleteActivePage(doc);
    }
}
void SettingsPage(Document *doc,Settings *settings, BindState *listeningForBind){
    DrawRectangle(0,0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0,150});

    int boxW = 800;
    int boxH = 500;
    int boxX = (GetScreenWidth() - boxW)/2;
    int boxY =  (GetScreenHeight() - boxH)/2;

    
    DrawRectangleRounded((Rectangle){boxX, boxY, boxW, boxH}, 0.05f, 10, (Color){40, 40, 40, 255});
    DrawRectangleRoundedLinesEx((Rectangle){boxX, boxY, boxW, boxH}, 0.05f, 10, 2.0f, DARKGRAY);
    
    if(GUIButton((Rectangle){boxX + 5, boxY + 5, 30,30}, "X", settings->showSettings)) settings->showSettings = !settings->showSettings;
    DrawText("Background", boxX + 50, boxY + 30, 20, WHITE);
    int col1X = boxX + 30;
    if(GUIButton((Rectangle){col1X, boxY+80, 180, 40}, "Blank", doc->pattern == BG_BLANK)) doc->pattern = BG_BLANK;
    if(GUIButton((Rectangle){col1X, boxY+130, 180, 40}, "Lined", doc->pattern == BG_LINED)) doc->pattern = BG_LINED;
    if(GUIButton((Rectangle){col1X, boxY+180, 180, 40}, "Grid", doc->pattern == BG_GRID)) doc->pattern = BG_GRID;
    if(GUIButton((Rectangle){col1X, boxY+230, 180, 40}, "Dots", doc->pattern == BG_DOTS)) doc->pattern =BG_DOTS;

    DrawText("Engine", boxX + 50, boxY + 300, 25, WHITE);
    int engineY = boxY + 340;
    if(GUIButton((Rectangle){col1X, engineY, 180, 40}, doc->useBakedRendering ? "Mode: Baked" : "Mode: Live", doc->useBakedRendering)){
        doc->useBakedRendering = !doc->useBakedRendering;
        RebakeAllLayers(doc);
    }
    engineY +=50;
    if(GUIButton((Rectangle){col1X, engineY, 180, 40}, doc->pressureEnabled ? "Pressure: ON" : "Pressure: OFF", doc->pressureEnabled)){
        doc->pressureEnabled = !doc->pressureEnabled;
        if(doc->useBakedRendering) RebakeAllLayers(doc);
    }
    engineY +=50;
    DrawText("Bake Scale", col1X, engineY + 10, 20, LIGHTGRAY);
    if(GUIButton((Rectangle) {col1X + 110, engineY, 30, 40}, "-", false)){
        if(doc->renderScale > 1.0f){
            doc->renderScale -=1.0f;
            RebakeAllLayers(doc);
        }
    }
    DrawText(TextFormat("%.1f", doc->renderScale), col1X + 145, engineY + 10, 20, WHITE );
    if(GUIButton((Rectangle){col1X + 185, engineY, 30, 40}, "+", false)){
        if(doc->renderScale < 4.0f){
            doc->renderScale +=1.0f;
            RebakeAllLayers(doc);
        }
    }
    DrawText("KEY", boxX + 320, boxY + 30, 20, WHITE);
    int col2X = boxX + 250;
    int currentY = boxY + 80;

    #define DRAW_BIND_ROW(label, keyval, bindID, xPos, yPos) \
        DrawText(label, xPos, yPos + 10, 20, LIGHTGRAY); \
        const char * disp##bindID = (*listeningForBind == bindID) ? "PRESS KEY" : KeyToString(keyval); \
        if (GUIButton((Rectangle){xPos + 120, yPos, 110, 40}, disp##bindID, *listeningForBind == bindID)) *listeningForBind = bindID;

    
    DRAW_BIND_ROW("Pen Brush", settings->binds.keyPen, BIND_PEN,col2X, currentY);
    currentY+=50;
    DRAW_BIND_ROW("Highlighter", settings->binds.keyHigh, BIND_HIGH, col2X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Line Brush", settings->binds.keyLine,BIND_LINE, col2X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Rect Tool", settings->binds.keyRect, BIND_RECT, col2X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Circle Tool", settings->binds.keyCircle, BIND_CIRCLE, col2X, currentY);
    
    DrawText("System", boxX + 590, boxY + 30, 20, WHITE);
    int col3X = boxX + 520;
    currentY = boxY + 80;

    DRAW_BIND_ROW("Save File", settings->binds.keySave, BIND_SAVE, col3X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Load File", settings->binds.keyLoad, BIND_LOAD, col3X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Undo", settings->binds.keyUndo,BIND_UNDO, col3X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Delete Page", settings->binds.keyDel, BIND_DEL, col3X, currentY);
    currentY +=50;

    if(GUIButton((Rectangle){col3X, currentY, 180, 40}, doc->enableLayers ? "Layers: ON" : "Layers: OFF", doc->enableLayers))
        doc->enableLayers = !doc->enableLayers;
    #undef DRAW_BIND_ROW
}
void SaveSettings(Settings settings){
    FILE *file = fopen("settings.stz", "wb");
    if(!file) return;

    char magic[6] = "NTZSTZ";
    fwrite(magic, sizeof(char), 6, file);
    fwrite(&settings.binds, sizeof(Keybinds), 1, file);
    fwrite(&settings.showSettings, sizeof(bool), 1, file);
    fwrite(&settings.selectedColorIndex, sizeof(int), 1, file);
    fwrite(&settings.currentBrushThickness, sizeof(float), 1, file);
    fwrite(&settings.pallete,  sizeof(Color),5, file);

    fclose(file);
}
bool LoadSettings(Settings *settings){
    FILE *file = fopen("settings.stz", "rb");
    if(!file) return false;

    char magic[6];
    fread(magic, sizeof(char), 6, file);
    if(strstr(magic, "NTZSTZ" )== NULL){
        fclose(file);
        return false;
    }
    
    fread(&settings->binds, sizeof(Keybinds), 1, file);
    //printf("daaa\n");
    fread(&settings->showSettings, sizeof(bool), 1, file);
    fread(&settings->selectedColorIndex, sizeof(int), 1, file);
    fread(&settings->currentBrushThickness, sizeof(float),1, file);
    printf("daaa\n");
    fread(&settings->pallete, sizeof(Color), 5, file);
    printf("daaa\n");

    printf("%i\n", settings->pallete[2]);
    fclose(file);
    return true;
    
}
void GUIHeaderDock(Document *doc, Settings *settings, Vector2 mousePos){
    int barWidth = 1000;
    int barHeight = 140;
    int barY = 20;
    int barX = (GetScreenWidth() - barWidth) / 2;
    Rectangle uiBounds = { (float)barX, (float)barY, (float)barWidth, (float)barHeight};

    DrawRectangleRounded((Rectangle){barX + 5, barY + 5, barWidth, barHeight}, 0.2f, 16, (Color){0,0,0,100});

    DrawRectangleRounded(uiBounds, 0.2f, 16, (Color){35,35,40, 245});
    DrawRectangleRoundedLinesEx(uiBounds, 0.2f, 16, 2.0f, (Color){60,60,65,255});

    int btnH = 46;
    int gap = 15;

    //row 1
    int r1Width = 765;
    int curX = barX + (barWidth - r1Width) / 2;
    int curY = barY + 15;

    if(GUIButton((Rectangle){curX, curY, 70, btnH}, "Pen", doc->activeBrush == BRUSH_PEN)) doc->activeBrush = BRUSH_PEN;
    curX += 70 + gap;
    if(GUIButton((Rectangle){curX, curY, 90, btnH}, "Pencil", doc->activeBrush == BRUSH_PENCIL)) doc->activeBrush = BRUSH_PENCIL;
    curX += 90 + gap;
    if(GUIButton((Rectangle){curX, curY, 70, btnH}, "High", doc->activeBrush == BRUSH_HIGHLIGHTER)) doc->activeBrush = BRUSH_HIGHLIGHTER;
    curX += 70 + gap;
    if(GUIButton((Rectangle){curX, curY, 70, btnH}, "Line", doc->activeBrush == BRUSH_LINE)) doc->activeBrush = BRUSH_LINE;
    curX += 70 + gap;
    if(GUIButton((Rectangle){curX, curY, 70, btnH}, "Rect", doc->activeBrush == BRUSH_RECTANGLE)) doc->activeBrush = BRUSH_RECTANGLE;
    curX += 70 + gap;
    if(GUIButton((Rectangle){curX, curY, 80, btnH}, "Circle", doc->activeBrush == BRUSH_CIRCLE)) doc->activeBrush = BRUSH_CIRCLE;
    curX += 80 + gap;
    if(GUIButton((Rectangle){curX, curY, 80, btnH}, "Eraser", doc->activeBrush == BRUSH_ERASER)) doc->activeBrush = BRUSH_ERASER;
    curX += 80 + 30;

    for(int i = 0; i < 5; i++){
        Vector2 center = {curX + 20, curY + btnH / 2.0f};
        if(!settings->showSettings * IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            if(CheckCollisionPointCircle(mousePos, center, 20.0f)) settings->selectedColorIndex = i;

        }
        if(i == settings->selectedColorIndex) DrawCircleV(center, 24, WHITE);
        DrawCircleV(center, 20, settings->pallete[i]);
        DrawCircleLines(center.x, center.y, 20, (Color){0,0,0,100});
        curX+=45;
    }
        
    
    //row 2
    int r2Width = 955;
    curX = barX + (barWidth - r2Width) / 2;
    curY = barY + 15 + btnH + 15;

    DrawText("Brush Size", curX + 30, curY , 20, LIGHTGRAY);
    curX += 50 + gap;
    GUISlider((Rectangle){curX - 30, curY + 30, 120, 16}, &settings->currentBrushThickness, 1.0f, 99.0f);

    
    curX +=110 + gap;

    if(GUIButton((Rectangle){curX, curY, 70, btnH}, "New", false)) AddPageToDocument(doc);
    curX += 70 + gap;
    if(GUIButton((Rectangle){curX, curY, 80, btnH}, "Save", false)){
        const char *path = ShowSaveFileDialog();
        if (path) SaveDocumentBinary(path, doc);
    }
    curX += 80 + gap;
    if(GUIButton((Rectangle){curX, curY, 80, btnH}, "Load", false)){
        const char *path = ShowOpenFileDialog();
        if(path) LoadDocumentBinary(path, doc);
    }
    curX += 80 + gap;
    if(GUIButton((Rectangle){curX, curY, 80, btnH}, "Undo", false)) UndoLastStrokes(&doc->pages[doc->activePage].layers[doc->pages[doc->activePage].activeLayer], doc->renderScale,doc->pressureEnabled, *doc);
    curX +=80 + gap;
    if(GUIButton((Rectangle){curX, curY, 90, btnH}, "Delete", false)) DeleteActivePage(doc);
    curX += 80 + gap;
    if(GUIButton((Rectangle){curX, curY, 100, btnH}, "Settings", settings->showSettings)) settings->showSettings = !settings->showSettings;

}
