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

void FinishStroke(Stroke *currentStroke, Document *doc){
    if(currentStroke->pointCount > 1){
        AddStrokeToPage(&doc->pages[doc->activePage], *currentStroke);
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