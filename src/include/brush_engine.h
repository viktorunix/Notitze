#ifndef BRUSH_ENGINE
#define BRUSH_ENGINE
#include "raylib.h"
#include "document.h"
#include "gui.h"
Color Premultiply(Color c);
void RenderStroke(Document doc, Stroke *stroke, float pageYOffset);
#endif