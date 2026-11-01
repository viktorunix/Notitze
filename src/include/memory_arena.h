#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

#include <stddef.h>
#include <stdbool.h>

typedef struct{
    unsigned char *buffer;
    size_t capacity;
    size_t offset;
} MemoryArena;

void InitArena(MemoryArena *arena, size_t capacity);
void* ArenaAlloc(MemoryArena *arena , size_t size);
void ResetArena(MemoryArena *arena);
void FreeArena(MemoryArena *arena);


extern MemoryArena activeStrokeArena;

#endif