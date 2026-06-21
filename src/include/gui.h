#ifndef GUI
#define GUI
#include "raylib.h"
#include "document.h"
#include "memory.h"

#define UI_HEIGHT 160
bool GUIButton (Rectangle bounds, const char *text, bool isActive);
void GUISlider(Rectangle bounds, float *value, float minValue, float maxValue);
//void newRenderStroke(Stroke *stroke, float pageYOffset);
void RenderStroke(Stroke *stroke, float pageYOffset);
void DrawPageBackground(Document *doc, BgPattern pattern, float pageYOffset);
void GUILayerPanel(Document *doc, Stroke currentStroke);
void RebakeAllLayers(Document *doc);
void GUIPage(Document *doc, Stroke *currentStroke, int p, int pageYOffset);
#endif
