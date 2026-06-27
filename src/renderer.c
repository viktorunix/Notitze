#include "include/renderer.h"
#include "include/document.h"
#include "include/raymath.h"
#include "include/gui.h"
#include "include/panels.h"
#include "include/brush_system.h"

#define PAGE_GAP 60

extern Stroke currentStroke;

void InitRenderer(Document* doc) {
    Image brushImage = GenImageColor(256, 256, BLANK);
    for(int y = 0; y < 256; y++){
        for(int x = 0; x < 256; x++){
            float dist = Vector2Distance((Vector2){x + 0.5f, y + 0.5f}, (Vector2){128.0f, 128.0f});
            if(dist <= 126.0f){
                ImageDrawPixel(&brushImage, x, y, WHITE);
            } else if(dist <= 128.0f){
                float alpha = (128.0f - dist) / 2.0f;
                unsigned char aVal = (unsigned char)(alpha * 255.0f);
                Color c = {aVal, aVal, aVal, aVal};
                ImageDrawPixel(&brushImage, x, y, c);
            }
        }
    }
    Texture2D softBrushTex = LoadTextureFromImage(brushImage);
    UnloadImage(brushImage);
    SetTextureFilter(softBrushTex, TEXTURE_FILTER_BILINEAR);
    doc->brushTex = softBrushTex;

    Image pencilImage = GenImageColor(256, 256, BLANK);
    for(int y = 0; y < 256; y += 2){
        for(int x = 0; x < 256; x += 2){
            float dist = Vector2Distance((Vector2){x + 0.5f, y + 0.5f}, (Vector2){128.0f, 128.0f});
            if(dist <= 128.0f){
                float randVal = (float)GetRandomValue(0,100) / 100.0f;
                float falloff = 1.0f - (dist / 128.0f);
                float density = falloff * falloff;
                if(randVal < density){
                    ImageDrawPixel(&pencilImage, x, y, WHITE);
                }
            }
        }
    }
    Texture2D pencilTexture = LoadTextureFromImage(pencilImage);
    UnloadImage(pencilImage);
    SetTextureFilter(pencilTexture, TEXTURE_FILTER_POINT);
    doc->pencilTex = pencilTexture;
}

void RenderApplication(Document* doc, Settings* settings, Camera2D camera,
                       int draggedPage, float dragOffsetY,
                       Vector2 mousePos, Vector2 mouseWorldPos, Vector2 localMousePos,
                       bool guiClicked, bool isMouseInsideCanvas,
                       BindState* listeningForBind, AppState *appState) {

    BeginDrawing();
    ClearBackground(DARKGRAY);


    BeginMode2D(camera);

    for(int p = 0; p < doc->pageCount; p++){
        if(p == draggedPage) continue;
        float pageYOffset = p * (doc->pageHeight + PAGE_GAP);
        GUIPage(doc, &currentStroke, p, pageYOffset);
    }


    if(draggedPage != -1){
        float floatY = mouseWorldPos.y - dragOffsetY;
        DrawRectangle(15, floatY + 15, doc->pageWidth, doc->pageHeight, (Color){0,0,0,100});
        DrawRectangle(0, floatY, doc->pageWidth, doc->pageHeight, RAYWHITE);
        DrawPageBackground(doc, doc->pattern, floatY);
        DrawRectangle(0, floatY, doc->pageWidth, 40, SKYBLUE);

        Page *page = &doc->pages[draggedPage];
        for(int l = 0; l < page->layerCount; l++){
            Layer *layer = &page->layers[l];
            if(!layer->isVisible || layer->texture.id == 0) continue;

            Rectangle source = {0, 0, (float) layer->texture.texture.width, -(float)layer->texture.texture.height};
            Rectangle destination = {0, floatY, doc->pageWidth, doc->pageHeight};
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            DrawTexturePro(layer->texture.texture, source, destination, (Vector2){0,0}, 0.0f, WHITE);
            EndBlendMode();
        }
    }


    if(!guiClicked && isMouseInsideCanvas){
        GetActiveBrush()->RenderPreview(doc, localMousePos, settings->currentBrushThickness);
    }
    EndMode2D();


    if(GUIHeaderDock(doc, settings, mousePos)){
        *appState = STATE_MENU;
    }

    if(doc->enableLayers && !settings->showSettings){
        GUILayerPanel(doc, currentStroke);
    }

    if(settings->showSettings){
        SettingsPage(doc, settings, listeningForBind);
    }

    EndDrawing();
}
