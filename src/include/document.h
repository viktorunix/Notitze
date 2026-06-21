#ifndef DOCUMENT
#define DOCUMENT
#include "raylib.h"
#include "raymath.h"
#include "brush.h"
#include "paper.h"

typedef enum {
    BG_BLANK = 0,
    BG_LINED,
    BG_GRID,
    BG_DOTS
} BgPattern;

typedef struct {
    Vector2 pos;
    float pressure;

} StrokePoint;
typedef struct {
    StrokePoint *points;
    int pointCount;
    int capacity;
    Color color;
    float thickness;
    BrushType type;
} Stroke;

typedef struct {
    Stroke *strokes;
    int strokeCount;
    int capacity;
    bool isVisible;
    RenderTexture2D texture;
} Layer;

typedef struct {
    Layer *layers;
    int layerCount;
    int layerCapacity;
    int activeLayer;
} Page;
typedef struct {
    Page *pages;
    int pageCount;
    int pageCapacity;
    int activePage;
    BgPattern pattern;

    bool isDrawing;
    BrushType activeBrush;

    bool enableLayers;

    PaperFormat pageFormat;
    float pageWidth;
    float pageHeight;
    int ppi;
} Document;
#endif
