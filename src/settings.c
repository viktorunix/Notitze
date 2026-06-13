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
void SettingsPage(Document *doc,Settings *settings, BindState *listeningForBind){
    DrawRectangle(0,0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0,150});

    int boxW = 800;
    int boxH = 400;
    int boxX = (GetScreenWidth() - boxW)/2;
    int boxY = (GetScreenHeight() - boxH)/2;

    DrawRectangleRounded((Rectangle){boxX, boxY, boxW, boxH}, 0.05f, 10, (Color){40, 40, 40, 255});
    DrawRectangleRoundedLinesEx((Rectangle){boxX, boxY, boxW, boxH}, 0.05f, 10, 2.0f, DARKGRAY);
    
    DrawText("Document Background", boxX + 50, boxY + 30, 25, WHITE);
    int col1X = boxX + 30;
    if(GUIButton((Rectangle){col1X, boxY+80, 180, 40}, "Blank", doc->pattern == BG_BLANK)) doc->pattern = BG_BLANK;
    if(GUIButton((Rectangle){col1X, boxY+130, 180, 40}, "Lined", doc->pattern == BG_LINED)) doc->pattern = BG_LINED;
    if(GUIButton((Rectangle){col1X, boxY+180, 180, 40}, "Grid", doc->pattern == BG_GRID)) doc->pattern = BG_GRID;
    if(GUIButton((Rectangle){col1X, boxY+230, 180, 40}, "Dots", doc->pattern == BG_DOTS)) doc->pattern =BG_DOTS;

    DrawText("KEY", boxX + 320, boxY + 30, 25, WHITE);
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
    
    DrawText("System", boxX + 590, boxY + 30, 25, WHITE);
    int col3X = boxX + 520;
    currentY = boxY + 80;

    DRAW_BIND_ROW("Save File", settings->binds.keySave, BIND_SAVE, col3X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Load File", settings->binds.keyLoad, BIND_LOAD, col3X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Undo", settings->binds.keyUndo,BIND_UNDO, col3X, currentY);
    currentY +=50;
    DRAW_BIND_ROW("Delete Page", settings->binds.keyDel, BIND_DEL, col3X, currentY);


    #undef DRAW_BIND_ROW
}