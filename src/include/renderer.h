#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "document.h"
#include "settings.h"


void InitRenderer(Document* doc);


void RenderApplication(Document* doc, Settings* settings, Camera2D camera,
                       int draggedPage, float dragOffsetY,
                       Vector2 mousePos, Vector2 mouseWorldPos, Vector2 localMousePos,
                       bool guiClicked, bool isMouseInsideCanvas,
                       BindState* listeningForBind, AppState *appState);

#endif
