#ifndef DOC_MANAGEMENT
#define DOC_MANAGEMENT
#include <stdlib.h>
#include "raylib.h"
#include "document.h"
#include "memory.h"

void MoveActivePageUp(Document *doc);
void MoveActivePageDown(Document *doc);
//void UndoLastStrokes(Page *page);
void UndoLastStrokes(Layer *layer);
void FinishStroke(Stroke *currentStroke, Document *doc);
void MovePageToIndex(Document *doc, int fromIndex, int toIndex);
#endif