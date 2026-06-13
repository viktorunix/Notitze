#ifndef DOCUMENT
#define DOCUMENT
#include "raylib.h"
#include "raymath.h"
#include "brush.h"

typedef enum {
    BG_BLANK = 0,
    BG_LINED,
    BG_GRID,
    BG_DOTS
} BgPattern;
typedef struct {
    Vector2 *points;
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
} Page;
typedef struct {
    Page *pages;
    int pageCount;
    int pageCapacity;
    int activePage;
    BgPattern pattern;

    bool isDrawing;
} Document;
#endif
