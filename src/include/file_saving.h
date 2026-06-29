#ifndef FILE_SAVING
#define FILE_SAVING
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "gui.h"
// windows API
#ifdef _WIN32
    #define Rectangle WinRectangle
    #define CloseWindow WinCloseWindow
    #define ShowCursor WinShowCursor
    #define PlaySound WinPlaySound
    #define DrawText WinDrawText
    #define DrawTextEx WinDrawTextEx
    #define LoadImage WinLoadImage

    #include <windows.h>
    #include <commdlg.h>

    #undef Rectangle
    #undef CloseWindow
    #undef ShowCursor
    #undef PlaySound
    #undef DrawText
    #undef DrawTextEx
    #undef LoadImage

#endif
void SaveDocumentBinary(const char *filename, Document *doc);
bool LoadDocumentBinary(const char *filename, Document *doc);
bool LoadLegacyNTZ2(FILE *file, Document *doc);
const char *ShowSaveFileDialog();
const char *ShowOpenFileDialog();

typedef struct {
    char title[64];
    long fileOffset;
    unsigned int fileSize;
    float w, h;
    int pageCount;
} NotebookEntry;

typedef struct {
    int count;
    NotebookEntry entries[256];
} NotebookIndex;

NotebookIndex ScanNotebook(const char *filename);
void SaveToNotebook(const char *notebookPath, Document *doc);
void LoadFromNotebook(const char *notebookPath, int index, Document *doc);

void CacheTexture(const char* filename, Texture2D texture);
#endif
