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

void UndoLastStrokes(Page *activePage){
    if(activePage->strokeCount > 0){
        activePage->strokeCount--;

        free(activePage->strokes[activePage->strokeCount].points);
        activePage->strokes[activePage->strokeCount].points = NULL;
        activePage->strokes[activePage->strokeCount].pointCount = 0;
        activePage->strokes[activePage->strokeCount].capacity = 0;
    }
}
