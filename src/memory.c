#include "include/memory.h"
void AddPointToStroke(Stroke *stroke, Vector2 point){
    if(stroke->pointCount >= stroke->capacity){
        stroke->capacity = stroke->capacity == 0 ? 128 : stroke->capacity * 2;
        stroke->points = (Vector2 *)realloc(stroke->points, stroke->capacity * sizeof(Vector2));

    }
    stroke->points[stroke->pointCount++] = point;
}
/*
void AddStrokeToPage(Page *page, Stroke stroke){
    if(page->strokeCount >= page->capacity){
        page->capacity = page->capacity == 0 ? 32 : page->capacity * 2;
        page->strokes = (Stroke *)realloc(page->strokes, page->capacity * sizeof(Stroke));

    }
    page->strokes[page->strokeCount++] = stroke;
}
*/
void AddStrokeToLayer(Layer *layer, Stroke stroke){
    if(layer->strokeCount >= layer->capacity){
        layer->capacity = layer->capacity == 0 ? 32 : layer->capacity * 2;
        layer->strokes = (Stroke *)realloc(layer->strokes, layer->capacity * sizeof(Stroke));
    }
    layer->strokes[layer->strokeCount++] = stroke;
}

void AddLayerToPage(Page *page){
    if(page->layerCount >= page->layerCapacity){
        page->layerCapacity = page->layerCapacity == 0 ? 4 : page->layerCapacity * 2;
        page->layers = (Layer *)realloc(page->layers, page->layerCapacity * sizeof(Layer));
    }
    page->layers[page->layerCount] = (Layer){0};
    page->layers[page->layerCount].isVisible = true;
    page->activeLayer = page->layerCount;
    page->layerCount++;
}

void AddPageToDocument(Document *doc){
    if(doc->pageCount >= doc->pageCapacity){
        doc->pageCapacity = doc->pageCapacity == 0 ? 4 : doc->pageCapacity * 2;
        doc->pages = (Page *)realloc(doc->pages, doc->pageCapacity  * sizeof(Page));
    }
    doc->pages[doc->pageCount] = (Page){0};
    AddLayerToPage(&doc->pages[doc->pageCount]);
    doc->activePage = doc->pageCount;
    doc->pageCount++;
}
/*
void AddPageToDocument(Document *doc){
    if(doc->pageCount >= doc->pageCapacity){
        doc->pageCapacity = doc->pageCapacity == 0 ? 4 : doc->pageCapacity * 2;
        doc->pages = (Page*)realloc(doc->pages, doc->pageCapacity * sizeof(Page));
    }
    doc->pages[doc->pageCount] = (Page){0};
    doc->activePage = doc->pageCount;
    doc->pageCount++;
}*/

void FreeLayer(Layer *layer){
    for(int s = 0; s < layer->strokeCount;s++)
        free(layer->strokes[s].points);
    free(layer->strokes);
}

void FreePage(Page *page){
    for(int l = 0; l < page->layerCount; l++)
        FreeLayer(&page->layers[l]);
    free(page->layers);
}
void DeleteActivePage(Document *doc){
    if(doc->pageCount <= 1) return;
    int p = doc->activePage;
    FreePage(&doc->pages[p]);
    for(int i = p; i < doc->pageCount - 1; i++)
        doc->pages[i] = doc->pages[i + 1];
    doc->pageCount--;
    if(doc->activePage >= doc->pageCount) 
        doc->activePage = doc->pageCount - 1;

}

void DeleteActiveLayer(Page *page){
    if(page->layerCount <= 1) return;
    int al = page->activeLayer;
    FreeLayer(&page->layers[al]);
    for(int i = al; i < page->layerCount - 1; i++)
        page->layers[i] = page->layers[i + 1];
    page->layerCount--;
    if(page->activeLayer >= page->layerCount)
        page->activeLayer = page->layerCount - 1;
}
/*
void FreeDocument(Document *doc){
    for(int p = 0; p < doc->pageCount;p++){
        for(int s = 0; s < doc->pages[p].strokeCount; s++){
            free(doc->pages[p].strokes[s].points);
        }
        free(doc->pages[p].strokes);
    }
    free(doc->pages);
    *doc = (Document){0};
}
    */

void FreeDocument(Document *doc){
    for(int p = 0; p < doc->pageCount; p++)
        FreePage(&doc->pages[p]);
    free(doc->pages);
    *doc = (Document){0};
}
/*
void DeleteActivePage(Document *doc){
    if (doc->pageCount <= 1) return;
    int p = doc->activePage;
    for(int s = 0; s < doc->pages[p].strokeCount; s++){
        free(doc->pages[p].strokes[s].points);
    }
    free(doc->pages[p].strokes);
    for (int i = p; i < doc->pageCount - 1; i++){
        doc->pages[i] = doc->pages[i + 1];
    }
    doc->pageCount--;

    if(doc->activePage >= doc->pageCount){
        doc->activePage = doc->pageCount - 1;
    }
}
*/
