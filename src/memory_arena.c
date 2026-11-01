#include "include/memory_arena.h"
#include <stdlib.h>

MemoryArena activeStrokeArena = {0};

void InitArena(MemoryArena *arena, size_t capacity){
    arena->buffer = (unsigned char *)malloc(capacity);
    arena->capacity = capacity;
    arena->offset = 0;
}

void* ArenaAlloc(MemoryArena *arena, size_t size){
    size_t aligned_size = (size + 7) & ~7;

    if(arena->offset + aligned_size > arena->capacity){
        return NULL; 
    }

    void *ptr = arena->buffer + arena->offset;
    arena->offset += aligned_size;

    return ptr;
}

void ResetArena(MemoryArena *arena){
    arena->offset = 0;
}

void FreeArena(MemoryArena *arena){
    if(arena->buffer){
        free(arena->buffer);
        arena->buffer = NULL;
    }
    arena->capacity = 0;
    arena->offset = 0;

}