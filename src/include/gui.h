#ifndef GUI
#define GUI
#include "raylib.h"
#include "document.h"
#include "settings.h"
#define A4_WIDTH 842
#define A4_HEIGHT 1191
#define UI_HEIGHT 60
bool GUIButton (Rectangle bounds, const char *text, bool isActive);
void GUISlider(Rectangle bounds, float *value, float minValue, float maxValue);
void RenderStroke(Stroke *stroke, float pageYOffset);
void DrawPageBackground(BgPattern pattern, float pageYOffset);
void GUIHeaderBar(Document *doc, Settings *settings);
#endif
