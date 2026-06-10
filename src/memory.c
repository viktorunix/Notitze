#include "include/memory.h"
void AddPointToStroke(Stroke *stroke, Vector2 point){
    if(stroke->pointCount >= stroke->capacity){
        stroke->capacity = stroke->capacity == 0 ? 128 : stroke->capacity * 2;
        stroke->points = (Vector2 *)realloc(stroke->points, stroke->capacity * sizeof(Vector2));

    }
    stroke->points[stroke->pointCount++] = point;
}

void AddStrokeToPage(Page *page, Stroke stroke){
    if(page->strokeCount >= page->capacity){
        page->capacity = page->capacity == 0 ? 32 : page->capacity * 2;
        page->strokes = (Stroke *)realloc(page->strokes, page->capacity * sizeof(Stroke));

    }
    page->strokes[page->strokeCount++] = stroke;
}

void AddPageToDocument(Document *doc){
    if(doc->pageCount >= doc->pageCapacity){
        doc->pageCapacity = doc->pageCapacity == 0 ? 4 : doc->pageCapacity * 2;
        doc->pages = (Page*)realloc(doc->pages, doc->pageCapacity * sizeof(Page));
    }
    doc->pages[doc->pageCount] = (Page){0};
    doc->activePage = doc->pageCount;
    doc->pageCount++;
}
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
