#include "include/doc_management.h"
#include "include/command_system.h"
#include "include/paper.h"
#include <string.h>
void MoveActivePageUp(Document *doc){
    if(doc->activePage > 0){
        Page temp = doc->pages[doc->activePage];
        doc->pages[doc->activePage] = doc->pages[doc->activePage - 1];
        doc->pages[doc->activePage - 1] = temp;
        doc->activePage--;
    }
}

void MoveActivePageDown(Document *doc){
    if(doc->activePage < doc->pageCount - 1){
        Page temp = doc->pages[doc->activePage];
        doc->pages[doc->activePage] = doc->pages[doc->activePage + 1];
        doc->pages[doc->activePage + 1] = temp;
        doc->activePage++;
    }
}


void FinishStroke(Stroke *currentStroke, Document *doc){
    if((currentStroke->type <= BRUSH_PENCIL && currentStroke->pointCount > 1) ||
       (currentStroke->type >= BRUSH_LINE && currentStroke->pointCount == 2) ||
        (currentStroke->type == BRUSH_TEXT && currentStroke->pointCount == 1)){
        Layer *activeLayer = &doc->pages[doc->activePage].layers[doc->pages[doc->activePage].activeLayer];

        AddStrokeToLayer(activeLayer, *currentStroke);
        PushDrawCommand(doc->activePage, doc->pages[doc->activePage].activeLayer, currentStroke);
        if(doc->useBakedRendering){

           StrokeAABB bounds = CalculateStrokeAABB(currentStroke);

            //map world coordinates to tile grid indices (factoring in renderScale)
            int startCol = (int)floor((bounds.minX * doc->renderScale) / TILE_SIZE);
            int endCol   = (int)floor((bounds.maxX * doc->renderScale) / TILE_SIZE);
            int startRow = (int)floor((bounds.minY * doc->renderScale) / TILE_SIZE);
            int endRow   = (int)floor((bounds.maxY * doc->renderScale) / TILE_SIZE);

            //clamp to grid boundaries to prevent segfaults
            if (startCol < 0) startCol = 0;
            if (startRow < 0) startRow = 0;
            if (endCol >= activeLayer->gridCols) endCol = activeLayer->gridCols - 1;
            if (endRow >= activeLayer->gridRows) endRow = activeLayer->gridRows - 1;

            //the Targeted Bake Loop
            for (int row = startRow; row <= endRow; row++) {
                for (int col = startCol; col <= endCol; col++) {
                    int tileIndex = (row * activeLayer->gridCols) + col;
                    Tile *tile = &activeLayer->tiles[tileIndex];

                    int newStrokeIndex = activeLayer->strokeCount - 1;
                    AddStrokeToTile(tile, newStrokeIndex);
                    if (!tile->isAllocated) {
                        tile->texture = LoadRenderTexture2DOnly(TILE_SIZE, TILE_SIZE);
                        SetTextureFilter(tile->texture.texture, TEXTURE_FILTER_POINT);
                        tile->isAllocated = true;
                    }
                    BeginTextureMode(tile->texture);
                    ClearBackground(BLANK);

                    Camera2D tileCam = {0};
                    //shift the camera so the tile's world-space coordinates map to (0,0) of the FBO
                    tileCam.offset = (Vector2){ -(float)(col * TILE_SIZE), -(float)(row * TILE_SIZE) };
                    tileCam.zoom = doc->renderScale;

                    BeginMode2D(tileCam);
                    BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);


                    //TODO: only draw strokes that intersect this tile's AABB)
                    for(int i = 0; i < activeLayer->strokeCount; i++){
                        RenderStroke(*doc, &activeLayer->strokes[i], 0);
                    }

                    EndBlendMode();
                    EndMode2D();
                    EndTextureMode();
                }
            }
        }
    } else{

        free(currentStroke->points);
    }
    *currentStroke = (Stroke){0};
}

void MovePageToIndex(Document *doc, int fromIndex, int toIndex){
    if(fromIndex == toIndex) return;
    if(toIndex < 0) toIndex = 0;
    if(toIndex >= doc->pageCount) toIndex = doc->pageCount - 1;

    Page temp = doc->pages[fromIndex];
    if(fromIndex < toIndex){
        for(int i = fromIndex; i <toIndex;i++){
            doc->pages[i] = doc->pages[i + 1];
        }
    }else {
        for(int i = fromIndex; i > toIndex; i--){
            doc->pages[i] = doc->pages[i - 1];
        }
    }
    doc->pages[toIndex] = temp;
    doc->activePage = toIndex;
}

Document* CreateEmptyDocument(){
    Document *doc = (Document *)calloc(1, sizeof(Document));
    doc->ppi = 150;
    doc->pageFormat = FORMAT_A4;
    doc->pattern = BG_BLANK;
    doc->patternSpacing = 30.0f;
    doc->patternColor = (Color){200, 215,230, 255};
    doc->renderScale = 2.0f;

    doc->useBakedRendering = true;
    doc->pressureEnabled = true;
    doc->enableLayers = false;
    doc->notebookIndex = -1;

    strcpy(doc->documentTitle, "Untitled Note");

    return doc;
}
