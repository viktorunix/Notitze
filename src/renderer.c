#include "include/renderer.h"
#include "include/document.h"
#include "include/file_saving.h"
#include "include/memory.h"
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/gui.h"
#include "include/brush_system.h"
#include <sys/stat.h>
#define PAGE_GAP 60

extern Stroke currentStroke;
Texture2D GenerateSmoothBrush(void){
    int size = 128;
    Image img = GenImageColor(size, size, BLANK);
    float center = size/ 2.0f;
    float radius = center - 2.0f;
    float feather = 20.0f;
    float solidRadius = center - feather;
    for(int y = 0; y < size; y++){
        for(int x = 0; x < size; x++){
            float dx = (float)x - center;
            float dy = (float)y - center;
            float dist = sqrtf(dx * dx + dy * dy);
            float alpha = 1.0f;
            if(dist > solidRadius){
                alpha = 1.0f - ((dist - solidRadius) / feather);
            }
            if(alpha > 1.0f) alpha = 1.0f;
            if(alpha < 0.005f) alpha = 0.0f;
            alpha = alpha * alpha * (3.0f - 2.0f * alpha);

            unsigned char a = (unsigned char)(alpha *255);
            Color c = {a,a,a,a};
            ImageDrawPixel(&img, x, y, c);
        }
    }
    Texture2D tex = LoadTextureFromImage(img);
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    UnloadImage(img);
    return tex;
}
void InitRenderer(Document* doc) {
    Texture2D softBrushTex = GenerateSmoothBrush();
    /*Image brushImage = GenImageColor(256, 256, BLANK);
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
    */
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

        // --- REPLACE THIS BLOCK ---
        if(doc->useBakedRendering && layer->tiles != NULL) {
            BeginScissorMode(thumbRec.x, thumbRec.y, thumbRec.width, thumbRec.height);
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

            float logicalTileSize = TILE_SIZE / doc->renderScale;
            float scaleX = thumbRec.width / doc->pageWidth;
            float scaleY = thumbRec.height / doc->pageHeight;

            for (int row = 0; row < layer->gridRows; row++) {
                for (int col = 0; col < layer->gridCols; col++) {
                    int tileIdx = (row * layer->gridCols) + col;
                    Tile *tile = &layer->tiles[tileIdx];

                    if (tile->isAllocated && tile->texture.id != 0) {
                        float destX = thumbRec.x + (col * logicalTileSize * scaleX);
                        float destY = thumbRec.y + (row * logicalTileSize * scaleY);
                        float destW = logicalTileSize * scaleX;
                        float destH = logicalTileSize * scaleY;

                        Rectangle dest = {destX, destY, destW, destH};
                        // Use the raw FBO size for the source rectangle
                        Rectangle src = {0, 0, (float)tile->texture.texture.width, -(float)tile->texture.texture.height};

                        DrawTexturePro(tile->texture.texture, src, dest, (Vector2){0,0}, 0.0f, WHITE);
                    }
                }
            }
            EndBlendMode();
            EndScissorMode();
        }

        // Leave the live-rendering fallback right below it untouched:

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
void RebakeAllLayers(Document *doc) {
    for(int p = 0; p < doc->pageCount; p++){
        Page *page = &doc->pages[p];
        for(int l = 0; l < page->layerCount; l++){
            Layer *layer = &page->layers[l];

            // 1. Destroy old grid
            if(layer->tiles){
                int oldTotal = layer->gridCols * layer->gridRows;
                for(int i = 0; i < oldTotal; i++){
                    if(layer->tiles[i].isAllocated) UnloadRenderTexture(layer->tiles[i].texture);
                }
                free(layer->tiles);
            }

            // 2. Calculate new grid based on updated doc->renderScale
            layer->gridCols = (int)ceil((doc->pageWidth * doc->renderScale) / TILE_SIZE);
            layer->gridRows = (int)ceil((doc->pageHeight * doc->renderScale) / TILE_SIZE);
            int totalTiles = layer->gridCols * layer->gridRows;
            layer->tiles = (Tile *)calloc(totalTiles, sizeof(Tile));

            // 3. Re-bake all strokes into the new chunk system
            for(int s = 0; s < layer->strokeCount; s++){
                Stroke *currentStroke = &layer->strokes[s];
                StrokeAABB bounds = CalculateStrokeAABB(currentStroke);

                int startCol = (int)floor((bounds.minX * doc->renderScale) / TILE_SIZE);
                int endCol   = (int)floor((bounds.maxX * doc->renderScale) / TILE_SIZE);
                int startRow = (int)floor((bounds.minY * doc->renderScale) / TILE_SIZE);
                int endRow   = (int)floor((bounds.maxY * doc->renderScale) / TILE_SIZE);

                if(startCol < 0) startCol = 0;
                if(startRow < 0) startRow = 0;
                if(endCol >= layer->gridCols) endCol = layer->gridCols - 1;
                if(endRow >= layer->gridRows) endRow = layer->gridRows - 1;

                for (int row = startRow; row <= endRow; row++) {
                    for (int col = startCol; col <= endCol; col++) {
                        int tileIndex = (row * layer->gridCols) + col;
                        Tile *tile = &layer->tiles[tileIndex];

                        AddStrokeToTile(tile, s);
                        if (!tile->isAllocated) {
                            tile->texture = LoadRenderTexture2DOnly(TILE_SIZE, TILE_SIZE);
                            SetTextureFilter(tile->texture.texture, TEXTURE_FILTER_TRILINEAR);
                            tile->isAllocated = true;

                            // Initialize with blank background
                            BeginTextureMode(tile->texture);
                            ClearBackground(BLANK);
                            EndTextureMode();
                        }

                        // Add stroke to tile
                        BeginTextureMode(tile->texture);
                        Camera2D tileCam = {0};
                        tileCam.offset = (Vector2){ -(float)(col * TILE_SIZE), -(float)(row * TILE_SIZE) };
                        tileCam.zoom = doc->renderScale;

                        BeginMode2D(tileCam);
                        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
                        RenderStroke(*doc, currentStroke, 0);
                        EndBlendMode();
                        EndMode2D();
                        EndTextureMode();
                        GenTextureMipmaps(&tile->texture.texture);
                    }
                }
            }
        }
    }
}
void GUIPage(Document *doc, Stroke *currentStroke, int p, int pageYOffset, Camera2D camera){    // 1. Draw Paper and Background
    DrawRectangle(8, pageYOffset + 8, doc->pageWidth, doc->pageHeight, BLACK);
    DrawRectangle(0, pageYOffset, doc->pageWidth, doc->pageHeight, RAYWHITE);
    DrawPageBackground(doc, doc->pattern, pageYOffset);


    Page *page = &doc->pages[p];

    // 2. Render Layers
    for(int l = 0; l < page->layerCount; l++) {
        Layer *layer = &page->layers[l];
        if(!layer->isVisible) continue;
        if(doc->useBakedRendering && layer->tiles != NULL && camera.zoom >=0.6f) {
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

            float logicalTileSize = TILE_SIZE / doc->renderScale;

            for (int row = 0; row < layer->gridRows; row++) {
                for (int col = 0; col < layer->gridCols; col++) {
                    int tileIdx = (row * layer->gridCols) + col;
                    Tile *tile = &layer->tiles[tileIdx];

                    if (tile->isAllocated && tile->texture.id != 0) {
                        float exactX = col * logicalTileSize;
                        float exactY = pageYOffset + (row * logicalTileSize);
                        float exactW = logicalTileSize;
                        float exactH = logicalTileSize;

                        // edge clipping
                        if (exactX + exactW > doc->pageWidth)
                            exactW = doc->pageWidth - exactX;
                        if ((exactY - pageYOffset) + exactH > doc->pageHeight)
                            exactH = doc->pageHeight - (exactY - pageYOffset);


                        //get exact world coordinates
                        Vector2 tlWorld = { exactX, exactY };
                        Vector2 brWorld = { exactX + exactW, exactY + exactH };

                        //project to screen space and lock to physical monitor pixels
                        Vector2 tlScreen = GetWorldToScreen2D(tlWorld, camera);
                        Vector2 brScreen = GetWorldToScreen2D(brWorld, camera);

                        tlScreen.x = roundf(tlScreen.x);
                        tlScreen.y = roundf(tlScreen.y);
                        brScreen.x = roundf(brScreen.x);
                        brScreen.y = roundf(brScreen.y);

                        //project back to world space.
                        Vector2 snappedTl = GetScreenToWorld2D(tlScreen, camera);
                        Vector2 snappedBr = GetScreenToWorld2D(brScreen, camera);

                        Rectangle destination = {
                            snappedTl.x,
                            snappedTl.y,
                            snappedBr.x - snappedTl.x,
                            snappedBr.y - snappedTl.y
                        };

                        //source mapping remains exactly the same
                        float srcW = exactW * doc->renderScale;
                        float srcH = exactH * doc->renderScale;
                        float srcY = tile->texture.texture.height - srcH;

                        Rectangle source = { 0, srcY, srcW, -srcH };

                        DrawTexturePro(tile->texture.texture, source, destination, (Vector2){0,0}, 0.0f, WHITE);
                    }
                }
            }
            EndBlendMode();
        } else {
            //live rendering fallback
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            for(int i = 0; i < layer->strokeCount; i++){
                RenderStroke(*doc, &layer->strokes[i], pageYOffset);
            }
            EndBlendMode();
        }

        //render the active stroke
        if(doc->isDrawing && p == doc->activePage && l == page->activeLayer){
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            RenderStroke(*doc, currentStroke, pageYOffset);
            EndBlendMode();
        }

    }

    DrawRectangle(0, pageYOffset, doc->pageWidth, 40, (Color){200,200,200,255});
    DrawText("|||", doc->pageWidth/2 - MeasureText("|||", 20)/ 2, pageYOffset + 10, 20, DARKGRAY);

    Color borderColor = (p == doc->activePage) ? SKYBLUE : LIGHTGRAY;
    int borderThickness = (p == doc->activePage) ? 4 : 1;
    DrawRectangleLinesEx((Rectangle){0, pageYOffset, doc->pageWidth, doc->pageHeight}, borderThickness, borderColor);

    Rectangle deletePageBounds = {doc->pageWidth + 10, pageYOffset + 10, 50, 50};
    printf("%b\n",GUIButton(deletePageBounds, "-", false));
}
void RenderApplication(Document* doc, Settings* settings, Camera2D camera,
                       int draggedPage, float dragOffsetY,
                       Vector2 mousePos, Vector2 mouseWorldPos, Vector2 localMousePos,
                       bool guiClicked, bool isMouseInsideCanvas,
                       BindState* listeningForBind, AppState *appState) {

    BeginDrawing();
    ClearBackground(DARKGRAY);


    BeginMode2D(camera);

    //valculate the visible world-space Y boundaries
    float screenTopWorldY = GetScreenToWorld2D((Vector2){0, 0}, camera).y;
    float screenBottomWorldY = GetScreenToWorld2D((Vector2){0, GetScreenHeight()}, camera).y;

    //map those Y coordinates to page indices
    float pageStride = doc->pageHeight + PAGE_GAP;
    int startPage = (int)floor(screenTopWorldY / pageStride);
    int endPage   = (int)floor(screenBottomWorldY / pageStride);

    //add a 2-page buffer to prevent popping/lag during fast scrolling
    startPage -= 2;
    endPage += 2;

    //clamp indices to ensure we don't read out of bounds
    if (startPage < 0) startPage = 0;
    if (endPage >= doc->pageCount) endPage = doc->pageCount - 1;

    //render only the visible and buffered pages
    for (int p = startPage; p <= endPage; p++) {
        if (p == draggedPage) continue;
        float pageYOffset = p * pageStride;
        GUIPage(doc, &currentStroke, p, pageYOffset, camera);
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

            if(!layer->isVisible || layer->tiles == NULL) continue;

            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            float logicalTileSize = TILE_SIZE / doc->renderScale;

            for (int row = 0; row < layer->gridRows; row++) {
                for (int col = 0; col < layer->gridCols; col++) {
                    int tileIdx = (row * layer->gridCols) + col;
                    Tile *tile = &layer->tiles[tileIdx];

                    if (tile->isAllocated && tile->texture.id != 0) {
                        float exactX = col * logicalTileSize;
                        float exactY = row * logicalTileSize;
                        float exactW = logicalTileSize;
                        float exactH = logicalTileSize;

                        if (exactX + exactW > doc->pageWidth) exactW = doc->pageWidth - exactX;
                        if (exactY + exactH > doc->pageHeight) exactH = doc->pageHeight - exactY;

                        Rectangle dest = { exactX, exactY, exactW + 0.5f, exactH + 0.5f };

                        float srcW = exactW * doc->renderScale;
                        float srcH = exactH * doc->renderScale;
                        float srcY = tile->texture.texture.height - srcH;

                        Rectangle src = { 0, srcY, srcW, -srcH };

                        DrawTexturePro(tile->texture.texture, src, dest, (Vector2){0,0}, 0.0f, WHITE);
                    }
                }
            }
            EndBlendMode();
        }
    }


    if(!guiClicked && isMouseInsideCanvas){
        GetActiveBrush()->RenderPreview(doc, mouseWorldPos, settings->currentBrushThickness);
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

        if(doc->useBakedRendering && layer->tiles != NULL){
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            float logicalTileSize = TILE_SIZE / doc->renderScale;

            for (int row = 0; row < layer->gridRows; row++) {
                for (int col = 0; col < layer->gridCols; col++) {
                    int tileIdx = (row * layer->gridCols) + col;
                    Tile *tile = &layer->tiles[tileIdx];

                   if (tile->isAllocated && tile->texture.id != 0) {
                        float exactX = col * logicalTileSize;
                        float exactY = row * logicalTileSize;
                        float exactW = logicalTileSize;
                        float exactH = logicalTileSize;

                        if (exactX + exactW > doc->pageWidth) exactW = doc->pageWidth - exactX;
                        if (exactY + exactH > doc->pageHeight) exactH = doc->pageHeight - exactY;

                        int snapX = (int)roundf(exactX);
                        int snapY = (int)roundf(exactY);
                        int snapNextX = (int)roundf(exactX + exactW);
                        int snapNextY = (int)roundf(exactY + exactH);
                        int snapW = snapNextX - snapX;
                        int snapH = snapNextY - snapY;

                        Rectangle dest = { (float)snapX, (float)snapY, (float)snapW, (float)snapH };

                        float srcW = exactW * doc->renderScale;
                        float srcH = exactH * doc->renderScale;
                        float srcY = tile->texture.texture.height - srcH;

                        Rectangle src = { 0, srcY, srcW, -srcH };

                        DrawTexturePro(tile->texture.texture, src, dest, (Vector2){0,0}, 0.0f, WHITE);
                    }
                }
            }
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
