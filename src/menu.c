#include "include/menu.h"
#include "include/gui.h"
#include "include/raylib.h"
#include "include/file_saving.h"
#include "include/paper.h"
#include "include/doc_management.h"
#include <string.h>
#include <stdio.h>

static NotebookIndex libraryIdx = {0};

static bool showNewModal = false;
static PaperFormat selectedFormat = FORMAT_A4;
static float customW = 1240.0f;
static float customH = 1754.0f;

static char newDocName[64] = "New Note";
static bool isTypingName = false;
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
   if(!showNewModal && GUIButton((Rectangle){GetScreenWidth() - 180, 20, 140, 40}, "+ New Note", false)){
       showNewModal = true;

       snprintf(newDocName, sizeof(newDocName), "Note %d", libraryIdx.count + 1);
       isTypingName = false;
   }

   //notebook entries
   int startX = 60;
   int startY = 160;
   int x = startX, y = startY;
   int itemW = 140;
   int itemH = 180;

   for(int i = 0 ; i<libraryIdx.count; i++){
       Rectangle itemBounds = {x, y, itemW, itemH};
       bool isHovered = !showNewModal && CheckCollisionPointRec(GetMousePosition(), itemBounds);


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

       DrawText(displayName, x + (itemW - MeasureText(displayName, 15))/2, y + 140, 15, WHITE);

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
   if(showNewModal){
       DrawRectangle(0,0, GetScreenWidth(), GetScreenHeight(), (Color){0,0,0,150});

       int mWidth = 420;
       int mHeight = 350;
       int mX = (GetScreenWidth() - mWidth)/2;
       int mY = (GetScreenHeight() - mHeight) /2;

       //background
       DrawRectangleRounded((Rectangle){mX, mY, mWidth, mHeight}, 0.1f, 10,(Color){40,40,45,255});
       DrawRectangleRoundedLinesEx((Rectangle){mX, mY, mWidth, mHeight}, 0.1f, 19, 2.0f, (Color){80,80,85,255});
       DrawText("Create New Note", mX + 20, mY + 20, 20, WHITE);

       //texbox for document name
       DrawText("Name:", mX + 20, mY + 70, 20, LIGHTGRAY);
       GUITextBox((Rectangle){mX + 90, mY + 60, 310,40}, newDocName, 64, &isTypingName);
       // format selection
       if(GUIButton((Rectangle){mX + 20, mY + 120, 80,35}, "A4", selectedFormat == FORMAT_A4)) selectedFormat = FORMAT_A4;
       if(GUIButton((Rectangle){mX + 110, mY + 120, 80,35}, "A5", selectedFormat== FORMAT_A5)) selectedFormat = FORMAT_A5;
       if(GUIButton((Rectangle){mX + 200, mY + 120, 80,35}, "Letter", selectedFormat == FORMAT_LETTER)) selectedFormat = FORMAT_LETTER;
       if(GUIButton((Rectangle){mX+290, mY + 120, 90, 35}, "Custom", selectedFormat == FORMAT_CUSTOM)) selectedFormat = FORMAT_CUSTOM;

       // custom dimension UI
       if (selectedFormat == FORMAT_CUSTOM) {
            DrawText("Width:", mX + 20, mY + 180, 20, LIGHTGRAY);
            GUISlider((Rectangle){mX + 100, mY + 180, 200, 20}, &customW, 500.0f, 4000.0f);
            DrawText(TextFormat("%.0f", customW), mX + 320, mY + 220, 18, WHITE);

            DrawText("Height:", mX + 20, mY + 160, 20, LIGHTGRAY);
            GUISlider((Rectangle){mX + 100, mY + 160, 200, 20}, &customH, 500.0f, 4000.0f);
            DrawText(TextFormat("%.0f", customH), mX + 320, mY + 160, 18, WHITE);
       }else {
            // Display Standard Sizes
            float w = 0, h = 0;
            if (selectedFormat == FORMAT_A4) { w = 1240; h = 1754; }
            else if (selectedFormat == FORMAT_A5) { w = 874; h = 1240; }
            else if (selectedFormat == FORMAT_LETTER) { w = 1275; h = 1650; }
            DrawText(TextFormat("Canvas Size: %.0f x %.0f px", w, h), mX + 20, mY + 220, 20, LIGHTGRAY);
        }
       // buttons
       if (GUIButton((Rectangle){mX + 20, mY + mHeight - 60, 170, 40}, "Cancel", false)) {
            showNewModal = false; // Close without doing anything
        }
        if (GUIButton((Rectangle){mX + 210, mY + mHeight - 60, 170, 40}, "Create", false)) {
            showNewModal = false; // Close modal

            //extern void FreeDocument(Document *doc);
            //extern void AddPageToDocument(Document *doc);


            Texture2D tempBrush = doc->brushTex;
            Texture2D tempPencil = doc->pencilTex;

            FreeDocument(doc);
            Document *tempDoc = CreateEmptyDocument();
            *doc = *tempDoc;
            
            doc->brushTex = tempBrush;
            doc->pencilTex = tempPencil;


            if (selectedFormat == FORMAT_A4) { doc->pageWidth = 1240.0f; doc->pageHeight = 1754.0f; doc->pageFormat = FORMAT_A4; }
            else if (selectedFormat == FORMAT_A5) { doc->pageWidth = 874.0f; doc->pageHeight = 1240.0f; doc->pageFormat = FORMAT_A5; }
            else if (selectedFormat == FORMAT_LETTER) { doc->pageWidth = 1275.0f; doc->pageHeight = 1650.0f; doc->pageFormat = FORMAT_LETTER; }
            else { doc->pageWidth = customW; doc->pageHeight = customH; doc->pageFormat = FORMAT_CUSTOM; }

            //doc->ppi = 150;
            //doc->pattern = BG_BLANK;
            //doc->patternSpacing = 30.0f;
            //doc->patternColor = (Color){200, 215, 230, 255};
            //doc->renderScale = 2.0f;

            //doc->useBakedRendering = true;
            //doc->pressureEnabled = true;
            //doc->enableLayers = false;

            //doc->notebookIndex = -1;
            if(strlen(newDocName) > 0){
                snprintf(doc->documentTitle, sizeof(doc->documentTitle), "%s", newDocName);
            } else{
                strcpy(doc->documentTitle, "Untitled Note");
            }
            AddPageToDocument(doc);
            return true;
        }
   }
   return false;
}
