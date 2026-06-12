#ifndef GUI
#define GUI
#include "raylib.h"
#include "document.h"

#define A4_WIDTH 842
#define A4_HEIGHT 1191
bool GUIButton (Rectangle bounds, const char *text, bool isActive);
void RenderStroke(Stroke *stroke, float pageYOffset);
void DrawPageBackground(BgPattern pattern, float pageYOffset);
#endif
