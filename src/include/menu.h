#ifndef MENU_H
#define MENU_H

#include "document.h"
#include <stdbool.h>

typedef enum{
    CTX_NONE = -1,
    CTX_RENAME = 0,
    CTX_DELETE,
    CTX_EXPORT,
    CTX_OPTION_COUNT
} ContexMenuOption;

typedef struct {
    bool isActive;
    Vector2 position;
    int targetDocIndex;
    int hoveredOption;
} ContextMenu;
static ContextMenu ctxMenu = {0};
static const char* ctxOptions[CTX_OPTION_COUNT] = {"Rename", "Delete", "Export"};
void InitMainMenu(void);
bool UpdateDrawMainMenu(Document *doc);

#endif
