#ifndef FILE_SAVING
#define FILE_SAVING
#include <stdio.h>
#include <string.h>
#include "memory.h"

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
const char *ShowSaveFileDialog();
const char *ShowOpenFileDialog();

#endif
