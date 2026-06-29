#ifndef GUI
#define GUI
#include "raylib.h"
#include "document.h"
#include "memory.h"
#include "brush_engine.h"

#define UI_HEIGHT 160
bool GUIButton (Rectangle bounds, const char *text, bool isActive);
void GUISlider(Rectangle bounds, float *value, float minValue, float maxValue);
bool GUITextBox(Rectangle bounds, char *text, int maxChars, bool *isActive);
void GUILoading(int progress, int total);
#endif
