#include "include/renderer.h"
#include "include/document.h"
#include "include/file_saving.h"
#include "include/raymath.h"
#include "include/gui.h"
#include "include/brush_system.h"
#include <sys/stat.h>
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
    CacheTexture("penBrush", softBrushTex);
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
            layer->texture = LoadRenderTexture2DOnly((int)(doc->pageWidth * doc->renderScale), (int)(doc->pageHeight * doc->renderScale));
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
void ExportPageToPNG(Document *doc, int pageIndex, const char *filepath){
    if(pageIndex < 0 || pageIndex >= doc->pageCount) return;

    RenderTexture2D exportTex = LoadRenderTexture2DOnly(doc->pageWidth, doc->pageHeight);
    BeginTextureMode(exportTex);
    ClearBackground(RAYWHITE);

    DrawPageBackground(doc, doc->pattern, 0);

    Page *page = &doc->pages[pageIndex];
    for(int l = 0; l < page->layerCount; l++){
        Layer *layer = &page->layers[l];
        if(!layer->isVisible) continue;

        if(doc->useBakedRendering && layer->texture.id != 0){
            Rectangle source = {0, 0, (float)layer->texture.texture.width, -(float)layer->texture.texture.height};
            Rectangle destination = {0, 0, doc->pageWidth, doc->pageHeight};
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            DrawTexturePro(layer->texture.texture, source, destination, (Vector2){0,0}, 0.0f, WHITE);
            EndBlendMode();
        } else{
            for(int i = 0; i < layer->strokeCount; i++){
                RenderStroke(*doc, &layer->strokes[i], 0);
            }
        }
    }
    EndTextureMode();

    Image img = LoadImageFromTexture(exportTex.texture);
    ImageFlipVertical(&img);
    ExportImage(img, filepath);

    UnloadImage(img);
    UnloadRenderTexture(exportTex);
}