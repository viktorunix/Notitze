#ifndef DOCUMENT
#define DOCUMENT
#include "raylib.h"
#include "brush.h"
#include "paper.h"

#define TILE_SIZE 512

typedef struct Tile {
    RenderTexture2D texture;
    bool isAllocated;
} Tile;
typedef enum{
    STATE_MENU,
    STATE_EDITOR
} AppState;
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
    char text[128];

    StrokeAABB bounds;
    bool hasBounds;
} Stroke;

typedef struct {
    bool isVisible;

    Stroke *strokes;
    int strokeCount;
    int capacity;


    Tile *tiles;
    int gridCols;
    int gridRows;
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

    bool useBakedRendering;
    float renderScale;
    bool pressureEnabled;

    Texture2D brushTex;
    Texture2D pencilTex;

    Color patternColor;
    float patternSpacing;

    int notebookIndex;
    char documentTitle[64];
} Document;
#endif
