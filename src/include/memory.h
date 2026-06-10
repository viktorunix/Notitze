#ifndef MEMORY
#define MEMORY
#include <stdlib.h>
#include "raylib.h"
#include "document.h"
void AddPointToStroke(Stroke *stroke, Vector2 point);
void AddStrokeToPage(Page *page, Stroke stroke);
void AddPageToDocument(Document *doc);
void FreeDocument(Document *doc);
#endif
