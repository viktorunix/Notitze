#ifndef RENDERER_H
#define RENDERER_H

#include "raylib.h"
#include "document.h"
#include "settings.h"



void InitRenderer(Document* doc);

void BeginPageClipping(float pageWidth, float pageHeight, float pageYOffset, Camera2D camera);

void DrawPageBackground(Document *doc, BgPattern pattern, float pageYOffset);
void GUILayerPanel(Document *doc, Stroke currentStroke);
void RebakeAllLayers(Document *doc);
void GUIPage(Document *doc, Stroke *currentStroke, int p, int pageYOffset, Camera2D camera);

void RenderWorkspace(Document* doc, Settings* settings, Camera2D camera,
                     int draggedPage, float dragOffsetY, Vector2 mouseWorldPos,
                     bool guiClicked, bool isMouseInsideCanvas);

void RenderUI(Document* doc, Settings* settings, Vector2 mousePos,
              BindState* listeningForBind, AppState *appState);
void ExportPageToPNG(Document *doc, int pageIndex, const char *filepath);
#endif
