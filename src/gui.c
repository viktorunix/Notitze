#include "include/gui.h"

bool GUI_Button (Rectangle bounds, const char *text){
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    bool isClicked = isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    
    DrawRectangleRec(bounds, isHovered ? LIGHTGRAY : GRAY);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);

    int textWidth = MeasureText(text, 20);
    DrawText(text, bounds.x + (bounds.width - textWidth) / 2, bounds.y + (bounds.height - 20) / 2, 20, BLACK);

    return isClicked;
    
}