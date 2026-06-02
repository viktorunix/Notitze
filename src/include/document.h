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
} Stroke;


typedef struct {
    Stroke *strokes;
    int strokeCount;
    int capacity;
} Document;
#endif
