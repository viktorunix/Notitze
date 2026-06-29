#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "raylib.h"
#include "document.h"
#include "viewport.h"

void ProcessInputs(Document* doc, Viewport* vp, bool guiClicked, int* draggedPage, float* dragOffsetY, float* currentPressure);

#endif
