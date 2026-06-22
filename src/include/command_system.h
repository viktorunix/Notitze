#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include "document.h"

typedef enum{
    CMD_DRAW_STROKE = 0,
    CMD_ERASE_STROKE
} CommandType;

typedef struct{
    CommandType type;
    int pageIndex;
    int layerIndex;

    Stroke strokeData;
    int strokeIndex;
} Command;

void InitCommandSystem(void);
void PushDrawCommand(int page, int layer, Stroke *stroke);
void PushEraseCommand(int page, int layer, Stroke *stroke, int index);
void UndoCommand(Document *doc);

#endif