#ifndef SETTINGS
#define SETTINGS
#include "raylib.h"
#include "document.h"

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

    bool showSettings;
    int selectedColorIndex;
    float currentBrushThickness;

    Color *pallete;

} Settings;
const char* KeyToString(int key);
void SettingsBinds(BindState *listeningForBind, Settings *settings);
void InputHandler(Document *doc, Settings *settings, BindState *listeningForBind);
void SettingsPage(Document *doc, Settings *settings, BindState *listeningForBind);
void SaveSettings(Settings settings);
bool LoadSettings(Settings *settings);
bool GUIHeaderDock(Document *doc, Settings *settings,Vector2 mousePos);

#endif
