#include "include/menu.h"
#include "include/gui.h"
#include "include/raylib.h"
#include "include/file_saving.h"
#include <string.h>
#include <stdio.h>

static NotebookIndex libraryIdx = {0};

void InitMainMenu(void){
    libraryIdx = ScanNotebook("Library.ntzbook");
}

bool UpdateDrawMainMenu(Document *doc){
    ClearBackground((Color){30,30,35,255});

    //top bar
    DrawRectangle(0,0, GetScreenWidth(), 80, (Color){40,40,45,255});
    DrawText("Notitze - Library", 40, 25, 30, WHITE);

    //path breadcrumbs
    DrawText("Library.ntzbook", 40, 100, 20, LIGHTGRAY);

   //new document
   if(GUIButton((Rectangle){GetScreenWidth() - 180, 20, 140, 40}, "+ New Note", false)){
       extern void FreeDocument(Document *doc);
       extern void AddPageToDocument(Document *doc);
       Texture2D tempBrush = doc->brushTex;
              Texture2D tempPencil = doc->pencilTex;

       FreeDocument(doc);
       doc->brushTex = tempBrush;
       doc->pencilTex = tempPencil;
       doc->pageWidth = 1000.0f;
       doc->pageHeight = 1400.0f;
       doc->ppi = 150;
       doc->pattern = BG_BLANK;
       doc->patternSpacing = 30.0f;
       doc->patternColor = (Color){200, 215, 230, 255};
       doc->renderScale = 2.0f;
       doc->useBakedRendering = true;
        doc->pressureEnabled = true;
        doc->enableLayers = false;
       doc->notebookIndex = -1;
       snprintf(doc->documentTitle, sizeof(doc->documentTitle), "Note %d", libraryIdx.count + 1);

       AddPageToDocument(doc);
       return true;
   }

   //notebook entries
   int startX = 60;
   int startY = 160;
   int x = startX, y = startY;
   int itemW = 140;
   int itemH = 180;

   for(int i = 0 ; i<libraryIdx.count; i++){
       Rectangle itemBounds = {x, y, itemW, itemH};
       bool isHovered = CheckCollisionPointRec(GetMousePosition(), itemBounds);

       DrawRectangleRounded(itemBounds, 0.1f, 8, isHovered ? (Color){60,60,65,255}:(Color){45,45,50,255});

       //Document preview
       int iconX = x + 30;
       int iconY = y + 20;
       DrawRectangle(iconX, iconY, 80, 100, RAYWHITE);
       DrawRectangleLines(iconX, iconY, 80, 100, LIGHTGRAY);
       DrawText(TextFormat("%d Pg", libraryIdx.entries[i].pageCount), iconX + 25, iconY + 40, 15, GRAY);

       // name
       char displayName[32];
       strncpy(displayName, libraryIdx.entries[i].title, 14);
       displayName[14] = '\0';
       if(strlen(libraryIdx.entries[i].title) > 14) strcat(displayName, "...");

       //click hangler
       if(isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
           LoadFromNotebook("Library.ntzbook", i, doc);
           return true;
       }

       // grid layout
       x += itemW + 30;
       if(x > GetScreenWidth() - itemW - 40){
           x = startX;
           y += itemH +30;
       }
   }
   return false;
}
