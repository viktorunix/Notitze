#ifndef BRUSH_ENGINE
#define BRUSH_ENGINE
#include "raylib.h"
#include "document.h"
#include "raymath.h"
#include "gui.h"

StrokeAABB CalculateStrokeAABB(Stroke *stroke);
Color Premultiply(Color c);
void RenderStroke(Document doc, Stroke *stroke, float pageYOffset);
#endif
