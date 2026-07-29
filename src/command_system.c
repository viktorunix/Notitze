#include "include/command_system.h"
#include "include/document.h"
#include "include/memory.h"
#include "include/doc_management.h"
#include "include/brush_engine.h"
#include <stdlib.h>
#include <string.h>

#define MAX_UNDO 50

static Command undoStack[MAX_UNDO];
static int undoTop = 0;

void InitCommandSystem(void){
    undoTop= 0;
}

static void DeepCopyStroke(Stroke *dest, Stroke *src){
    *dest = *src;
    if(src->pointCount > 0){
        dest->points = (StrokePoint*)malloc(src->pointCount * sizeof(StrokePoint));
        memcpy(dest->points, src->points, src->pointCount * sizeof(StrokePoint));
        dest->capacity = src->capacity;
    } else{
        dest->points = NULL;
    }
}

void PushDrawCommand(int page, int layer, Stroke *stroke){
    if(undoTop >= MAX_UNDO){
        if(undoStack[0].strokeData.points)
            free(undoStack[0].strokeData.points);
        for(int i = 0; i <MAX_UNDO - 1; i++)
            undoStack[i] = undoStack[i+1];
        undoTop--;
    }

    Command cmd = {0};
    cmd.type = CMD_DRAW_STROKE;
    cmd.pageIndex = page;
    cmd.layerIndex = layer;
    DeepCopyStroke(&cmd.strokeData, stroke);

    undoStack[undoTop++] = cmd;

}

void PushEraseCommand(int page, int layer, Stroke *stroke, int index){
    if (undoTop >= MAX_UNDO) {
        if (undoStack[0].strokeData.points)
            free(undoStack[0].strokeData.points);
        for(int i = 0; i < MAX_UNDO - 1; i++)
            undoStack[i] = undoStack[i+1];
        undoTop--;

    }
    Command cmd = {0};
    cmd.type = CMD_ERASE_STROKE;
    cmd.pageIndex = page;
    cmd.layerIndex = layer;
    cmd.strokeIndex = index;
    DeepCopyStroke(&cmd.strokeData, stroke);

    undoStack[undoTop++] = cmd;
}

void UndoCommand(Document *doc){
    if(undoTop <= 0 ) return;

    undoTop--;
    Command cmd = undoStack[undoTop];

    if(cmd.pageIndex >= doc->pageCount) return;
    Page *page = &doc->pages[cmd.pageIndex];
    if(cmd.layerIndex >= page->layerCount) return;
    Layer *layer = &page->layers[cmd.layerIndex];

    if(cmd.type == CMD_DRAW_STROKE){
        RemoveStrokeFromLayer(layer, layer->strokeCount - 1);
        if(cmd.strokeData.points)
            free(cmd.strokeData.points);
    }
    else if(cmd.type == CMD_ERASE_STROKE){
        if(layer->strokeCount >= layer->capacity){
            layer->capacity = layer->capacity == 0 ? 32 : layer->capacity * 2;
            layer->strokes = (Stroke*)realloc(layer->strokes, layer->capacity * sizeof(Stroke));

        }

        for(int i = layer->strokeCount; i > cmd.strokeIndex; i--){
            layer->strokes[i] = layer->strokes[i - 1];
        }
        layer->strokes[cmd.strokeIndex] = cmd.strokeData;
        layer->strokeCount++;

        if(doc->useBakedRendering && layer->tiles != NULL){
            int totalTiles = layer->gridCols * layer->gridRows;

            for(int t = 0; t < totalTiles; t++){
                Tile *tile = &layer->tiles[t];
                for(int i = 0; i < tile->strokeCount; i++){
                    if(tile->strokeIndices[i] >= cmd.strokeIndex)
                        tile->strokeIndices[i]++;
                }
            }
        }
        StrokeAABB bounds = CalculateStrokeAABB(&layer->strokes[cmd.strokeIndex]);
        int startCol = (int)floor((bounds.minX * doc->renderScale) / TILE_SIZE);
        int endCol = (int)floor((bounds.maxX * doc->renderScale) / TILE_SIZE);
        int startRow = (int)floor((bounds.minY * doc->renderScale) / TILE_SIZE);
        int endRow = (int)floor((bounds.maxY * doc->renderScale) / TILE_SIZE);

        if(startCol < 0) startCol = 0;
        if(startRow < 0) startRow = 0;
        if(endCol >=layer->gridCols) endCol = layer->gridCols - 1;
        if(endRow >= layer->gridRows) endRow = layer->gridRows - 1;

        for(int row = startRow; row <=endRow; row++){
            for( int col = startCol; col <=endCol; col++){
                int tileIndex = (row * layer->gridCols) + col;
                AddStrokeToTile(&layer->tiles[tileIndex], cmd.strokeIndex);
            }
        }
    }
    if(doc->useBakedRendering){
        if(doc->useBakedRendering && layer->tiles != NULL){

            StrokeAABB bounds = CalculateStrokeAABB(&cmd.strokeData);

            int startCol = (int)floor((bounds.minX * doc->renderScale) / TILE_SIZE);
            int endCol = (int)floor((bounds.maxX * doc->renderScale) / TILE_SIZE);
            int startRow = (int)floor((bounds.minY * doc->renderScale) / TILE_SIZE);
            int endRow = (int)floor((bounds.maxY * doc->renderScale) / TILE_SIZE);

            if (startCol < 0) startCol = 0;
            if (startRow < 0) startRow = 0;
            if (endCol >= layer->gridCols) endCol = layer->gridCols - 1;
            if (endRow >= layer->gridRows) endRow = layer->gridRows - 1;

            for (int row = startRow; row <= endRow; row++) {
                for (int col = startCol; col <= endCol; col++) {
                    int tileIndex = (row * layer->gridCols) + col;
                    Tile *tile = &layer->tiles[tileIndex];

                    if (!tile->isAllocated) {
                        tile->texture = LoadRenderTexture2DOnly(TILE_SIZE, TILE_SIZE);
                        SetTextureFilter(tile->texture.texture, TEXTURE_FILTER_BILINEAR);
                        tile->isAllocated = true;
                    }

                    BeginTextureMode(tile->texture);
                    ClearBackground(BLANK);

                    Camera2D tileCam = {0};
                    tileCam.offset = (Vector2){ -(float)(col * TILE_SIZE), -(float)(row * TILE_SIZE) };
                    tileCam.zoom = doc->renderScale;

                    BeginMode2D(tileCam);
                    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);

                    for(int s = 0; s < layer->strokeCount; s++) {
                        RenderStroke(*doc, &layer->strokes[s], 0);
                    }

                    EndBlendMode();
                    EndMode2D();
                    EndTextureMode();
                }
            }
        }
    }
}
