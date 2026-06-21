#include "include/doc_management.h"

void MoveActivePageUp(Document *doc){
    if(doc->activePage > 0){
        Page temp = doc->pages[doc->activePage];
        doc->pages[doc->activePage] = doc->pages[doc->activePage - 1];
        doc->pages[doc->activePage - 1] = temp;
        doc->activePage--;
    }
}

void MoveActivePageDown(Document *doc){
    if(doc->activePage < doc->pageCount - 1){
        Page temp = doc->pages[doc->activePage];
        doc->pages[doc->activePage] = doc->pages[doc->activePage + 1];
        doc->pages[doc->activePage + 1] = temp;
        doc->activePage++;
    }
}

void UndoLastStrokes(Layer *layer, float renderScale, bool pressureEnabled){
    if(layer->strokeCount > 0 ){
        layer->strokeCount--;
        free(layer->strokes[layer->strokeCount].points);
        layer->strokes[layer->strokeCount].points = NULL;
        layer->strokes[layer->strokeCount].pointCount = 0;
        layer->strokes[layer->strokeCount].capacity = 0;

        BeginTextureMode(layer->texture);
        ClearBackground(BLANK);
        Camera2D bakeCam = {0};
        bakeCam.zoom = renderScale;
        BeginMode2D(bakeCam);
        for(int i = 0; i < layer->strokeCount; i++)
            RenderStroke(&layer->strokes[i], 0, pressureEnabled);
        EndMode2D();
        EndTextureMode();
        

    }
}
void FinishStroke(Stroke *currentStroke, Document *doc){
    if((currentStroke->type <= BRUSH_PENCIL && currentStroke->pointCount > 1) ||
       (currentStroke->type >= BRUSH_LINE && currentStroke->pointCount == 2)){
        Layer *activeLayer = &doc->pages[doc->activePage].layers[doc->pages[doc->activePage].activeLayer];

        AddStrokeToLayer(activeLayer, *currentStroke);
        if(doc->useBakedRendering){
            BeginTextureMode(activeLayer->texture);
            ClearBackground(BLANK);
            Camera2D bakeCam = {0};
            bakeCam.zoom = doc->renderScale;
            BeginMode2D(bakeCam);
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            for(int i = 0; i < activeLayer->strokeCount; i++){
                RenderStroke(&activeLayer->strokes[i], 0, doc->pressureEnabled);
            }
            EndBlendMode();
            EndMode2D();
            EndTextureMode();
        }
    } else{
        
        free(currentStroke->points);
    }
    *currentStroke = (Stroke){0};
}

void MovePageToIndex(Document *doc, int fromIndex, int toIndex){
    if(fromIndex == toIndex) return;
    if(toIndex < 0) toIndex = 0;
    if(toIndex >= doc->pageCount) toIndex = doc->pageCount - 1;

    Page temp = doc->pages[fromIndex];
    if(fromIndex < toIndex){
        for(int i = fromIndex; i <toIndex;i++){
            doc->pages[i] = doc->pages[i + 1];
        }
    }else {
        for(int i = fromIndex; i > toIndex; i--){
            doc->pages[i] = doc->pages[i - 1];
        }
    }
    doc->pages[toIndex] = temp;
    doc->activePage = toIndex;
}