#ifndef DOCUMENT
#define DOCUMENT
#include "raylib.h"
#include "raymath.h"
#include "brush.h"
typedef struct {
    Vector2 pos;
    float pressure;
} StrokePoint;

typedef struct{
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
    RenderTexture2D texture;
} Page;

typedef struct{
    Page *pages;
    int pageCount;
    int pageCapacity;
    int activePage;
} Document;
#endif
