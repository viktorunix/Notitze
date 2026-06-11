#ifndef FILE_SAVING
#define FILE_SAVING
#include <stdio.h>
#include <string.h>
#include "memory.h"

void SaveDocumentBinary(const char *filename, Document *doc);
bool LoadDocumentBinary(const char *filename, Document *doc);
void DeleteActivePage(Document *doc);
#endif
