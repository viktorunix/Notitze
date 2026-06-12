#ifndef GUI
#define GUI
#include "raylib.h"
#include "document.h"
bool GUIButton (Rectangle bounds, const char *text, bool isActive);
void RenderStroke(Stroke *stroke, float pageYOffset);
#endif
