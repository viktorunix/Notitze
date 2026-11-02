#include "include/memory.h"
#include "include/memory_arena.h"
#include "include/rlgl.h"
#include "include/raymath.h"
#include <math.h>

static float PointSegmentDistance(Vector2 p, Vector2 a, Vector2 b){
    float l2 = Vector2DistanceSqr(a, b);
    if(l2 == 0) return Vector2Distance(p, a); //a and b are same

    //t is the projection of the point P on AB and then normalised
    float t = ((p.x - a.x) * (b.x - a.x) + (p.y - a.y) * (b.y - a.y)) / l2;
    t = fmaxf(0.0f, fminf(1.0f, t));

    Vector2 projection = {a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)};
    return Vector2Distance(p, projection);
}

static void RDP_Rec(StrokePoint *points, int startIndex, int endIndex, float epsilon, bool *keepPoint){
    float maxDistance = 0.0f;
    int maxIndex = startIndex;

    //finding the point farthest from the segment
    for(int i = startIndex + 1; i < endIndex; i++){
        float dist = PointSegmentDistance(points[i].pos, points[startIndex].pos, points[endIndex].pos);
        if (dist > maxDistance){
            maxDistance = dist;
            maxIndex = i;
        }
    }
    
    //if its too far than epsilon we keep the point
    if(maxDistance > epsilon){
        keepPoint[maxIndex] = true;

        RDP_Rec(points, startIndex, maxIndex, epsilon, keepPoint);
        RDP_Rec(points, maxIndex, endIndex, epsilon, keepPoint);
    }
}

void SimplifyStroke(Stroke *stroke, float epsilon){
    if (stroke->pointCount <= 2) return;

    bool *keepPoint = (bool *)ArenaAlloc(&activeStrokeArena, stroke->pointCount * sizeof(bool));
    if(!keepPoint) return;

    keepPoint[0] = true;
    keepPoint[stroke->pointCount - 1] = true;

    RDP_Rec(stroke->points, 0, stroke->pointCount - 1, epsilon, keepPoint);

    int newCount = 0;
    for(int i = 0; i < stroke->pointCount; i++){
        if(keepPoint[i]){
            stroke->points[newCount++] = stroke->points[i];
        }
    }

    stroke->pointCount = newCount;
}
RenderTexture2D LoadRenderTexture2DOnly(int width, int height){
    RenderTexture2D target = {0};
    target.id = rlLoadFramebuffer();

    if(target.id > 0){
        rlEnableFramebuffer(target.id);

        //color texture
        target.texture.id = rlLoadTexture(0, width, height, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);

        if(rlFramebufferComplete(target.id))
            TraceLog(LOG_INFO, "FBO: [ID %i] 2D-Only Framebuffer created", target.id);
        rlDisableFramebuffer();
    }
    return target;
}

void AddPointToStroke(Stroke *stroke, Vector2 point, float pressure){
    //first alocation
    if(stroke->pointCount == 0 || stroke->points == NULL){
        stroke->capacity = 4096;
        stroke->points = (StrokePoint *)ArenaAlloc(&activeStrokeArena, stroke->capacity * sizeof(StrokePoint));

        if(stroke->points == NULL) return;
    }
    // extending the capacity
    if(stroke->pointCount >= stroke->capacity){
        int oldCapacity = stroke->capacity;
        stroke->capacity *= 2;

        StrokePoint *newPoints = (StrokePoint *)ArenaAlloc(&activeStrokeArena, stroke->capacity * sizeof(StrokePoint));
        if(newPoints == NULL){
            stroke->capacity = oldCapacity;
            return;
        }
        //copying the old points
        for(int i = 0; i < oldCapacity; i++){
            newPoints[i] = stroke->points[i];
        }
        stroke->points = newPoints;
    }
    //adding the new point
    stroke->points[stroke->pointCount].pos = point;
    stroke->points[stroke->pointCount].pressure = pressure;
    stroke->pointCount++;

}
void AddStrokeToLayer(Layer *layer, Stroke stroke){
    if(layer->strokeCount >= layer->capacity){
        layer->capacity = layer->capacity == 0 ? 32 : layer->capacity * 2;
        layer->strokes = (Stroke *)realloc(layer->strokes, layer->capacity * sizeof(Stroke));
    }
    layer->strokes[layer->strokeCount++] = stroke;
}
void AddLayerToPage(Page *page, float width, float height, float renderScale) {
    if(page->layerCount >= page->layerCapacity){
        page->layerCapacity = page->layerCapacity == 0 ? 4 : page->layerCapacity * 2;
        page->layers = (Layer *)realloc(page->layers, page->layerCapacity * sizeof(Layer));
    }

    Layer *newLayer = &page->layers[page->layerCount];
    *newLayer = (Layer){0}; // Zero out memory
    newLayer->isVisible = true;

    newLayer->gridCols = (int)ceil((width * renderScale) / TILE_SIZE);
    newLayer->gridRows = (int)ceil((height * renderScale) / TILE_SIZE);
    int totalTiles = newLayer->gridCols * newLayer->gridRows;
    newLayer->tiles = (Tile *)calloc(totalTiles, sizeof(Tile));

    page->activeLayer = page->layerCount;
    page->layerCount++;
}

void AddPageToDocument(Document *doc){
    if(doc->pageCount >= doc->pageCapacity){
        doc->pageCapacity = doc->pageCapacity == 0 ? 4 : doc->pageCapacity * 2;
        doc->pages = (Page *)realloc(doc->pages, doc->pageCapacity  * sizeof(Page));
    }
    doc->pages[doc->pageCount] = (Page){0};
    AddLayerToPage(&doc->pages[doc->pageCount], doc->pageWidth, doc->pageHeight, doc->renderScale);
    doc->activePage = doc->pageCount;
    doc->pageCount++;
}

void FreeLayer(Layer *layer) {
    for(int s = 0; s < layer->strokeCount; s++) {
        free(layer->strokes[s].points);
    }
    free(layer->strokes);

    if (layer->tiles) {
        int totalTiles = layer->gridCols * layer->gridRows;
        for (int i = 0; i < totalTiles; i++) {
            if (layer->tiles[i].isAllocated) {
                UnloadRenderTexture(layer->tiles[i].texture);
            }
            if(layer->tiles[i].strokeIndices)
                free(layer->tiles[i].strokeIndices);
        }
        free(layer->tiles);
        layer->tiles = NULL;
    }
}

void FreePage(Page *page){
    for(int l = 0; l < page->layerCount; l++)
        FreeLayer(&page->layers[l]);
    free(page->layers);
}
void DeleteActivePage(Document *doc){
    if(doc->pageCount <= 1) return;
    int p = doc->activePage;
    FreePage(&doc->pages[p]);
    for(int i = p; i < doc->pageCount - 1; i++)
        doc->pages[i] = doc->pages[i + 1];
    doc->pageCount--;
    if(doc->activePage >= doc->pageCount)
        doc->activePage = doc->pageCount - 1;

}

void DeleteActiveLayer(Page *page){
    if(page->layerCount <= 1) return;
    int al = page->activeLayer;
    FreeLayer(&page->layers[al]);
    for(int i = al; i < page->layerCount - 1; i++)
        page->layers[i] = page->layers[i + 1];
    page->layerCount--;
    if(page->activeLayer >= page->layerCount)
        page->activeLayer = page->layerCount - 1;
}
/*
void FreeDocument(Document *doc){
    for(int p = 0; p < doc->pageCount; p++)
        FreePage(&doc->pages[p]);
    free(doc->pages);
    *doc = (Document){0};

}
    */
void FreeDocument(Document *doc){
    if(!doc) return;
    for(int p = 0; p < doc->pageCount; p++)
        FreePage(&doc->pages[p]);
    if(doc->pages){
        free(doc->pages);
        doc->pages = NULL;
    }
    //free(doc);
        //doc = NULL;
    doc->pageCount = 0;
    doc->pageCapacity = 0;
}
void RemoveStrokeFromLayer(Layer *layer, int index){
    if(index < 0 || index >= layer->strokeCount) return;


    for(int i = index; i < layer->strokeCount - 1; i++)
        layer->strokes[i] = layer->strokes[i + 1];
    layer->strokeCount--;

    if(layer->tiles){
        int totalTiles = layer->gridCols * layer->gridRows;
        for(int t = 0; t < totalTiles; t++){
            Tile *tile = &layer->tiles[t];
            for(int i = 0; i < tile->strokeCount; i++){
                if(tile->strokeIndices[i] == index){
                    for(int j = i; j < tile->strokeCount - 1; j++){
                        tile->strokeIndices[j] = tile->strokeIndices[j + 1];
                    }
                    tile->strokeCount--;
                    i--;
                }
                else if(tile->strokeIndices[i] > index){
                    tile->strokeIndices[i]--;
                }
            }
        }
    }

}
void AddStrokeToTile(Tile *tile, int strokeIndex){
    //prevent duplicate registrations
    for(int i = 0; i < tile->strokeCount; i++){
        if(tile->strokeIndices[i] == strokeIndex) return;
    }
    if(tile->strokeCount >= tile->capacity){
        tile->capacity = tile->capacity == 0 ? 8 : tile->capacity * 2;
        tile->strokeIndices = (int *)realloc(tile->strokeIndices, tile->capacity * sizeof(int));
    }
    tile->strokeIndices[tile->strokeCount++] = strokeIndex;
}
