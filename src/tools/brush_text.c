#include "../include/brush_system.h"
#include "../include/memory.h"
#include "../include/doc_management.h"
#include <string.h>

extern Stroke currentStroke;
extern Color currentBrushColor;
extern float currentBrushThickness;

static bool isTyping = false;
static char textBuffer[128] = "";
static Vector2 textLocation = {0};

static void Text_OnPress(Document *doc, Vector2 pos, float pressure){
    if(isTyping){
        // if clicked elswere commit the current text
        if(strlen(textBuffer) > 0){
            currentStroke = (Stroke){0};
            currentStroke.type = BRUSH_TEXT;
            currentStroke.color = currentBrushColor;
            currentStroke.thickness = currentBrushThickness;
            strcpy(currentStroke.text, textBuffer);

            //single point for storing the position of the text
            AddPointToStroke(&currentStroke, textLocation, 1.0f);
            FinishStroke(&currentStroke, doc);
        }
        isTyping = false;
        doc->isDrawing = false;
        textBuffer[0] = '\0';
        return;
    }
    //start typing at new location
    isTyping = true;
    doc->isDrawing = true;
    textLocation = pos;
    textBuffer[0] = '\0';
}

static void Text_RenderPreview(Document *doc, Vector2 pos, float thickness){
    int fontSize = (int)(thickness * 8.0f);
    if(fontSize < 10) fontSize = 10;

    if(isTyping){
        // capture key input
        int key = GetCharPressed();
        while(key > 0){
            if((key >= 32) && (key <=125) && (strlen(textBuffer) < 127)){
                int len = strlen(textBuffer);
                textBuffer[len] = (char)key;
                textBuffer[len+1] = '\0';
            }
            key = GetCharPressed();
        }
        if(IsKeyPressed(KEY_BACKSPACE)){
            int len = strlen(textBuffer);
            if(len > 0) textBuffer[len - 1] = '\0';
        }
        if(IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)){
            isTyping = false;
            doc->isDrawing = false;
        }

        DrawText(textBuffer, textLocation.x, textLocation.y, fontSize, currentBrushColor);

        //blinking cursor
        if(((int)(GetTime() * 2) %2 == 0)){
            int w = MeasureText(textBuffer, fontSize);
            DrawText("_", textLocation.x + w + 2, textLocation.y, fontSize, LIGHTGRAY);
        }
    } else{
        //draw previw cursor to know how big is the font
        DrawText("A", pos.x, pos.y - fontSize, fontSize, LIGHTGRAY);
    }
}

Brush CreateTextBrush(void){
    Brush textBrush = {0};
    textBrush.id = BRUSH_TEXT;
    textBrush.name = "Text";
    textBrush.OnPress = Text_OnPress;
    textBrush.RenderPreview = Text_RenderPreview;
    return textBrush;
}