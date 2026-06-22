#include "include/command_system.h"
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
    }
    if(doc->useBakedRendering){
        BeginTextureMode(layer->texture);
        ClearBackground(BLANK);
        Camera2D bakeCam = {0};
        bakeCam.zoom = doc->renderScale;
        BeginMode2D(bakeCam);
        BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
        for(int s = 0; s < layer->strokeCount; s++) {
            RenderStroke(*doc, &layer->strokes[s], 0);
        }
        EndBlendMode();
        EndMode2D();
        EndTextureMode();
    }
}