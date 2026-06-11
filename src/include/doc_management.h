#ifndef DOC_MANAGEMENT
#define DOC_MANAGEMENT
#include <stdlib.h>
#include "raylib.h"
#include "document.h"

void MoveActivePageUp(Document *doc);
void MoveActivePageDown(Document *doc);
void UndoLastStrokes(Page *page);

#endif