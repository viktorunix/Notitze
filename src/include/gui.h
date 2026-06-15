#ifndef GUI
#define GUI
#include "raylib.h"
#include "document.h"
#include "memory.h"
#define A4_WIDTH 842
#define A4_HEIGHT 1191
#define UI_HEIGHT 160
bool GUIButton (Rectangle bounds, const char *text, bool isActive);
void GUISlider(Rectangle bounds, float *value, float minValue, float maxValue);
void RenderStroke(Stroke *stroke, float pageYOffset);
void DrawPageBackground(BgPattern pattern, float pageYOffset);
void GUILayerPanel(Document *doc);
void GUIPage(Document *doc, Stroke *currentStroke, int p, int pageYOffset);
#endif
