#include "include/gui.h"
#include "include/command_system.h"
#include "include/raylib.h"
#include <stdio.h>
#include <string.h>
bool GUIButton (Rectangle bounds, const char *text, bool isActive){
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    bool isClicked = isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    Color bgColor = isActive ? (Color){70, 130, 180,255} : (isHovered ? (Color){70, 70, 70, 255} : (Color){45, 45, 45, 255});
    Color borderColor = isActive ? SKYBLUE : (Color){80,80,80,255};
    Color textColor = isActive ? WHITE : LIGHTGRAY;


    DrawRectangleRounded(bounds, 0.4f, 10, bgColor);
    DrawRectangleRoundedLinesEx(bounds, 0.4f, 10, 1.05f, borderColor);

    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + (bounds.width - textWidth) / 2, bounds.y + (bounds.height - 20) / 2, 20, textColor);


    return isClicked;

}
void GUISlider(Rectangle bounds, float *value, float minValue, float maxValue){
    Vector2 mousePos = GetMousePosition();

    Rectangle hitBox = {bounds.x - 10, bounds.y - 10, bounds.width + 20, bounds.height + 20};

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, hitBox)){
        float normalized = (mousePos.x - bounds.x) / bounds.width;
        if(normalized < 0.0f) normalized = 0.0f;
        if(normalized >= 1.0f) normalized = 1.0f;
        *value = minValue + normalized * (maxValue - minValue);
    }

    //background
    DrawRectangleRounded(bounds, 1.0f, 10, (Color) {35,35,40, 255});
    DrawRectangleRoundedLinesEx(bounds, 1.0f, 10, 1.5f, (Color){90, 90, 95, 255});

    //filled portion
    float currentNorm = (*value - minValue) / (maxValue - minValue);
    if(currentNorm > 0.02f){
        Rectangle fillRec = {bounds.x, bounds.y, bounds.width * currentNorm, bounds.height};
        DrawRectangleRounded(fillRec, 1.0f, 10, (Color){70, 130, 180, 255});

    }

    //knob
    Vector2 knobCenter = {bounds.x + bounds.width * currentNorm, bounds.y + bounds.height / 2.0f};
    DrawCircleV(knobCenter, bounds.height * 0.8f, WHITE);
    DrawCircleLines(knobCenter.x, knobCenter.y, bounds.height * 0.8f, GRAY);
}

bool GUITextBox(Rectangle bounds, char *text, int maxChars, bool *isActive){
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);

    //toggling active state on click
    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
        *isActive = isHovered;
    }

    //handling keyboard input when active
    if(*isActive){
        int key = GetCharPressed();
        while(key > 0){
            if((key >=32) && (key <= 125) && (strlen(text) < maxChars)){
                int len = strlen(text);
                text[len] = (char)key;
                text[len+1] = '\0';
            }
            key = GetCharPressed();
        }

        // handling backspace
        if(IsKeyPressed(KEY_BACKSPACE)){
            int len = strlen(text);
            if (len > 0) text[len - 1] = '\0';
        }
    }

    //background
    Color bgColor = *isActive ? (Color){60, 60, 65, 255} : (isHovered ? (Color){55,55,60,255} : (Color){45,45,50,255});
    Color borderColor = *isActive ? SKYBLUE : (isHovered ? LIGHTGRAY : (Color){80,80,85,255});

    DrawRectangleRounded(bounds, 0.2f, 10, bgColor);
    DrawRectangleRoundedLinesEx(bounds,0.2f, 10, 1.5f, borderColor);

    DrawText(text, bounds.x + 10, bounds.y + (bounds.height - 20) / 2, 20, WHITE);

    //blinking cursor
    if(*isActive && ((int)(GetTime() * 2) % 2 == 0)){
        int textWidth = MeasureText(text, 20);
        DrawText("_", bounds.x + 10 + textWidth, bounds.y + (bounds.height - 20) / 2, 20, LIGHTGRAY);
    }
    return *isActive;
}

void GUILoading(int progress, int total){
    int boxW = 300;
    int boxH = 150;
    int barX = (GetScreenWidth() - boxW) /2;
    int barY = (GetScreenHeight() - boxH) / 2;

    int currentY = barY +50;
    BeginDrawing();
    DrawRectangleRounded((Rectangle){barX, barY, boxW, boxH},0.2f, 16, DARKGRAY);
    //DrawText(TextFormat("%d/%d",progress, total),GetScreenWidth()/2, GetScreenHeight()/2, 20, WHITE);
    DrawText("Loading Document",(GetScreenWidth() - MeasureText("LoadingDocument", 20))/2.0f, currentY, 20, WHITE);
    currentY += 50;

    DrawRectangleRounded((Rectangle){barX + 20, currentY, 260,16 },1.0f, 10, DARKGRAY);
    DrawRectangleRoundedLinesEx((Rectangle){barX + 20, currentY, 260,16 },1.0f, 10,1.5f, WHITE);

    float filledX = 260 * ((float)progress / total);
    DrawRectangleRounded((Rectangle){barX + 20, currentY, filledX,16 },1.0f, 10, GREEN);


    EndDrawing();
}
