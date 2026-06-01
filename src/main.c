#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "document.h"

int main(void){
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1400, 900, "Notitze");
    SetWindowMinSize(1100, 600);
    SetTargetFPS(120);

    int barWidth = 1000;
    int barHeight = 140;
    int barY = 20;

    Camera2D camera = {0};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){0.0f, barY+barHeight + 30.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    Document doc = {0};
    doc.pages = malloc(1 * sizeof(Page));
    doc.pageCount = 1;
    doc.pageCapacity = 10;
    doc.activePage = 1;

    printf("%p\n", doc.pages);
    (doc.pages)->strokes = malloc(32 * sizeof(Stroke));
    (doc.pages)->strokeCount = 0;
    (doc.pages)->capacit = 32;

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground((Color){20, 20, 25, 255});


        BeginMode2D(camera);

        for(int p  = 0; p < doc.pageCount; p++){

        }
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
    return 0;

}
