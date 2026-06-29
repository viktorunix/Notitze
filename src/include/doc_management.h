#ifndef DOC_MANAGEMENT
#define DOC_MANAGEMENT
#include <stdlib.h>
#include "raylib.h"
#include "document.h"
#include "file_saving.h"

void MoveActivePageUp(Document *doc);
void MoveActivePageDown(Document *doc);
void UndoLastStrokes(Layer *layer, float renderScale, bool pressureEnabled, Document doc);
void FinishStroke(Stroke *currentStroke, Document *doc);
void MovePageToIndex(Document *doc, int fromIndex, int toIndex);
Document* CreateEmptyDocument();
#endif
