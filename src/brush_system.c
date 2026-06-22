#include "include/brush_system.h"
#include <stddef.h>

#define MAX_TOOLS 20

static Brush g_brushes[MAX_TOOLS];
static int g_brushCount = 0;
static Brush* g_activeBrush = NULL;

static void EmptyAction(Document *doc, Vector2 pos, float pressure){}
static void EmptyRelease(Document *doc, Vector2 pos){}
static void EmptyPreview(Document *doc, Vector2 pos, float thickness){}

void InitBrushSystem(void){
    g_brushCount = 0;
    g_activeBrush = NULL;
}
void RegisterBrush(Brush brush){
    if(g_brushCount >= MAX_TOOLS) return;

    if(!brush.OnPress) brush.OnPress = EmptyAction;
    if(!brush.OnDrag) brush.OnDrag = EmptyAction;
    if(!brush.OnRelease) brush.OnRelease = EmptyRelease;
    if(!brush.RenderPreview) brush.RenderPreview = EmptyPreview;

    g_brushes[g_brushCount] = brush;

    if(g_brushCount == 0)
        g_activeBrush = &g_brushes[0];
    
    g_brushCount++;
}

void SetActiveBrush(int id){
    for(int i = 0; i < g_brushCount; i++){
        if(g_brushes[i].id == id){
            g_activeBrush = &g_brushes[i];
            return;
        }
    }
}

Brush* GetActiveBrush(void){
    return g_activeBrush;
}
