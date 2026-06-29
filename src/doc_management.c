#include "include/doc_management.h"
#include "include/command_system.h"
#include "include/paper.h"
#include <string.h>
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


void FinishStroke(Stroke *currentStroke, Document *doc){
    if((currentStroke->type <= BRUSH_PENCIL && currentStroke->pointCount > 1) ||
       (currentStroke->type >= BRUSH_LINE && currentStroke->pointCount == 2) ||
        (currentStroke->type == BRUSH_TEXT && currentStroke->pointCount == 1)){
        Layer *activeLayer = &doc->pages[doc->activePage].layers[doc->pages[doc->activePage].activeLayer];

        AddStrokeToLayer(activeLayer, *currentStroke);
        PushDrawCommand(doc->activePage, doc->pages[doc->activePage].activeLayer, currentStroke);
        if(doc->useBakedRendering){

            if(activeLayer->texture.id == 0){
                activeLayer->texture = LoadRenderTexture((int)(doc->pageWidth * doc->renderScale), (int)(doc->pageHeight * doc->renderScale));
                SetTextureFilter(activeLayer->texture.texture, TEXTURE_FILTER_BILINEAR);
            }
            BeginTextureMode(activeLayer->texture);
            ClearBackground(BLANK);
            Camera2D bakeCam = {0};
            bakeCam.zoom = doc->renderScale;
            BeginMode2D(bakeCam);
            BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
            for(int i = 0; i < activeLayer->strokeCount; i++){
                RenderStroke(*doc, &activeLayer->strokes[i], 0);
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

Document* CreateEmptyDocument(){
    Document *doc = (Document *)calloc(1, sizeof(Document));
    doc->ppi = 150;
    doc->pageFormat = FORMAT_A4;
    doc->pattern = BG_BLANK;
    doc->patternSpacing = 30.0f;
    doc->patternColor = (Color){200, 215,230, 255};
    doc->renderScale = 2.0f;

    doc->useBakedRendering = true;
    doc->pressureEnabled = true;
    doc->enableLayers = false;
    doc->notebookIndex = -1;

    strcpy(doc->documentTitle, "Untitled Note");

    return doc;
}
