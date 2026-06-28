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

void DrawPageBackground(Document *doc, BgPattern pattern, float pageYOffset){
    Color lineColor = doc->patternColor.a == 0 ? (Color){200,215,230,255} : doc->patternColor;
    Color marginColor = (Color){255, 150, 150, 180};
    float spacing = doc->patternSpacing <= 5.0f ? 30.0f : doc->patternSpacing;


    if (pattern == BG_LINED){
        DrawLine(80, pageYOffset, 80, pageYOffset + doc->pageHeight, marginColor);
        for(int y = 80; y < doc->pageHeight; y+=spacing){
            DrawLine(0, pageYOffset + y, doc->pageWidth, pageYOffset + y, lineColor);
        }
    }
    else if(pattern == BG_GRID){
        for(int x = 30; x < doc->pageWidth; x+=spacing)
            DrawLine(x, pageYOffset, x, pageYOffset + doc->pageHeight, lineColor);
        for(int y = 30; y < doc->pageHeight; y+=spacing)
            DrawLine(0, pageYOffset + y, doc->pageWidth, pageYOffset + y, lineColor);


    }
    else if (pattern == BG_DOTS){
        for(int x = 30; x < doc->pageWidth; x+=spacing){
            for(int y = 30; y < doc->pageHeight; y+=spacing)
                DrawCircle(x, pageYOffset + y, 2.0f, lineColor);
        }
    }
}

void GUILayerPanel(Document *doc, Stroke currentStroke){
    int barY = 20;
    int barHeight = 140;
    Page *aPage = &doc->pages[doc->activePage];
    int pW = 300;
    int pH = 60 + (aPage->layerCount * 125) + (aPage->layerCount > 1 ? 50 : 0);
    int pX = GetScreenWidth() - pW - 20;
    int pY = barY + barHeight + 20;

    DrawRectangleRounded((Rectangle){pX + 5, pY + 5, pW, pH}, 0.1f, 10, (Color){0,0,0,100});
    DrawRectangleRounded((Rectangle){pX, pY, pW, pH}, 0.1f, 10, (Color){35,35,40, 245});
    DrawRectangleRoundedLinesEx((Rectangle){pX, pY, pW, pH}, 0.1f, 10, 2.0f, (Color){60,60,65,255});

    DrawText("Layers", pX + 20, pY + 18, 20, WHITE);
    if(GUIButton((Rectangle){pX + pW -50, pY + 10, 35, 35}, "+", false)) AddLayerToPage(aPage, doc->pageWidth, doc->pageHeight, doc->renderScale);

    int lY = pY + 60;
    for(int l = aPage->layerCount - 1; l >=0; l--){
        Layer *layer = &aPage->layers[l];
        if(GUIButton((Rectangle){pX+10, lY + 45,35, 35}, layer->isVisible ? "O" : "-", layer->isVisible))
            layer->isVisible= !layer->isVisible;

        Rectangle thumbRec = {pX + 60, lY + 6, 80, 113};
        DrawRectangleRec(thumbRec, RAYWHITE);

        if(doc->useBakedRendering && layer->texture.id != 0){
            Rectangle source = {0,0, (float)layer->texture.texture.width, -(float)layer->texture.texture.height};
            DrawTexturePro(layer->texture.texture, source, thumbRec, (Vector2){0,0}, 0.0f, WHITE);
        }
        if(!doc->useBakedRendering || (doc->isDrawing && doc->activePage >= 0 && l == aPage->activeLayer)){
            BeginScissorMode(thumbRec.x, thumbRec.y, thumbRec.width, thumbRec.height);
            Camera2D thumbCam = {0};
            thumbCam.target = (Vector2){0,0};
            thumbCam.offset = (Vector2){thumbRec.x, thumbRec.y};
            thumbCam.zoom = thumbRec.width / doc->pageWidth;
            BeginMode2D(thumbCam);
            if(!doc->useBakedRendering){
                for(int i = 0 ; i < layer->strokeCount; i++)
                    RenderStroke(*doc, &layer->strokes[i], 0);
            }
            if(doc->isDrawing && doc->activePage >= 0 && l == aPage->activeLayer)
                RenderStroke(*doc, &currentStroke, 0);
            EndMode2D();
            EndScissorMode();
        }

        DrawRectangleLinesEx(thumbRec, 1.0f, LIGHTGRAY);

        if(GUIButton((Rectangle){pX+155,lY+ 42, pW - 170,40}, TextFormat("Layer %d", l+1), aPage->activeLayer == l))
            aPage->activeLayer = l;
        lY+=125;
    }
    if(aPage->layerCount > 1){
        if(GUIButton((Rectangle){pX + 10, lY + 5, pW - 20, 35}, "Delete Layer", false))
            DeleteActiveLayer(aPage);
    }
}
void RebakeAllLayers(Document *doc){
    for(int p = 0; p < doc->pageCount; p++){
        Page *page = &doc->pages[p];
        for(int l = 0; l < page->layerCount; l++){
            Layer *layer = &page->layers[l];

            UnloadRenderTexture(layer->texture);
            layer->texture = LoadRenderTexture((int)(doc->pageWidth * doc->renderScale), (int)(doc->pageHeight * doc->renderScale));
            SetTextureFilter(layer->texture.texture, TEXTURE_FILTER_BILINEAR);

            BeginTextureMode(layer->texture);
            ClearBackground(BLANK);
            Camera2D bakeCam = {0};
            bakeCam.zoom = doc->renderScale;
            BeginMode2D(bakeCam);
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            for(int i = 0 ; i< layer->strokeCount; i++)
                RenderStroke(*doc, &layer->strokes[i], 0);
            EndBlendMode();
            EndMode2D();
            EndTextureMode();
        }
    }
}
void GUIPage(Document *doc, Stroke *currentStroke, int p, int pageYOffset){
    //paper
    DrawRectangle(8, pageYOffset + 8, doc->pageWidth, doc->pageHeight, BLACK);
    DrawRectangle(0, pageYOffset, doc->pageWidth, doc->pageHeight, RAYWHITE);
    DrawPageBackground(doc, doc->pattern, pageYOffset);
    //strokes
    Page *page = &doc->pages[p];

    for(int l = 0; l < page->layerCount; l++){
        Layer *layer = &page->layers[l];
        if(!layer->isVisible) continue;
        if(doc->useBakedRendering && layer->texture.id !=0){
            Rectangle source = {0,0, (float)layer->texture.texture.width, -(float)layer->texture.texture.height};
            Rectangle destination = {0, pageYOffset, doc->pageWidth, doc->pageHeight};
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            DrawTexturePro(layer->texture.texture, source, destination, (Vector2){0,0}, 0.0f, WHITE);
            EndBlendMode();
        }else{
            for(int i = 0; i < layer->strokeCount; i++){
                RenderStroke(*doc, &layer->strokes[i], pageYOffset);
            }
        }
        if(doc->isDrawing && p == doc->activePage && l == page->activeLayer){
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            RenderStroke(*doc, currentStroke, pageYOffset);
            EndBlendMode();

        }
    }
    //paper drag bar
    DrawRectangle(0, pageYOffset, doc->pageWidth, 40, (Color){200,200,200,255});
    DrawText("|||", doc->pageWidth/2 - MeasureText("|||", 20)/ 2, pageYOffset + 10, 20, DARKGRAY);
    // active page highlight
    Color borderColor = (p == doc->activePage) ? SKYBLUE : LIGHTGRAY;
    int borderThickness = (p == doc->activePage) ? 4 : 1;
    DrawRectangleLinesEx((Rectangle){0, pageYOffset, doc->pageWidth, doc->pageHeight}, borderThickness, borderColor);

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
