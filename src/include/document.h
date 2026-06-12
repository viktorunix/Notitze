#ifndef DOCUMENT
#define DOCUMENT
#include "raylib.h"
#include "raymath.h"
#include "brush.h"


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
} Document;
#endif
