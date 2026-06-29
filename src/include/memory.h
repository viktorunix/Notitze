#ifndef MEMORY
#define MEMORY
#include <stdlib.h>
#include "raylib.h"
#include "document.h"
void AddPointToStroke(Stroke *stroke, Vector2 point, float pressure);
void AddStrokeToLayer(Layer *layer, Stroke stroke);
void AddLayerToPage(Page *page, float width, float height, float renderScale);
void AddPageToDocument(Document *doc);
void FreeLayer(Layer *layer);
void FreePage(Page *page);
void FreeDocument(Document *doc);
void DeleteActivePage(Document *doc);
void DeleteActiveLayer(Page *page);
void RemoveStrokeFromLayer(Layer *layer, int index);
#endif
