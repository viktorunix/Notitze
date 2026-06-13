#ifndef SETTINGS
#define SETTINGS
#include "raylib.h"
#include "document.h"
#include "gui.h"
typedef enum{
    BIND_NONE = 0,
    BIND_PEN,
    BIND_HIGH,
    BIND_LINE,
    BIND_RECT,
    BIND_CIRCLE,
    BIND_SAVE,
    BIND_LOAD,
    BIND_UNDO,
    BIND_DEL
} BindState;

typedef struct {
    int keyPen, keyHigh, keyLine, keyRect, keyCircle;
    int keySave, keyLoad, keyUndo, keyDel;
} Keybinds;

typedef struct {
    Keybinds binds;
} Settings;
const char* KeyToString(int key);
void SettingsBinds(BindState *listeningForBind, Settings *settings);
void SettingsPage(Document *doc, Settings *settings, BindState *listeningForBind);
#endif