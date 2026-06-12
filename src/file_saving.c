#include "include/file_saving.h"
void SaveDocumentBinary(const char *filename, Document *doc){
    FILE *file = fopen(filename, "wb");
    if (!file) return;

    char magic[3] = "NTZ";
    fwrite(magic, sizeof(char),3, file);
    fwrite(&doc->pageCount, sizeof(int),1,file);
    fwrite(&doc->pattern, sizeof(int), 1, file);
    for(int p = 0; p < doc->pageCount; p++){
        Page *page = &doc->pages[p];
        fwrite(&page->strokeCount, sizeof(int),1,file);

        for(int s = 0; s< page->strokeCount;s++){
            Stroke *stroke = &page->strokes[s];
            fwrite(&stroke->color, sizeof(Color), 1, file);
            fwrite(&stroke->thickness, sizeof(float), 1, file);
            fwrite(&stroke->pointCount, sizeof(int), 1, file);

            fwrite(stroke->points, sizeof(Vector2), stroke->pointCount, file);
        }
    }
    fclose(file);
}

bool LoadDocumentBinary(const char *filename, Document *doc){
    FILE *file = fopen(filename, "rb");
    if (!file) return false;
    char magic[3];
    fread(magic, sizeof(char), 3, file);
    if(strstr(magic, "NTZ") ==NULL){
        fclose(file);
        return false;
    }

    FreeDocument(doc);
    int totalPages = 0;
    fread(&totalPages, sizeof(int), 1, file);

    fread(&doc->pattern, sizeof(int), 1, file);
    for(int p = 0; p < totalPages; p++){
        AddPageToDocument(doc);
        Page *page = &doc->pages[p];

        int totalStrokes = 0;
        fread(&totalStrokes, sizeof(int), 1, file);

        for(int s = 0; s < totalStrokes; s++){
            Stroke stroke = {0};
            fread(&stroke.color, sizeof(Color), 1, file);
            fread(&stroke.thickness, sizeof(float), 1, file);
            fread(&stroke.pointCount, sizeof(int), 1, file);

            stroke.capacity = stroke.pointCount;
            stroke.points = (Vector2 *)malloc(stroke.capacity *sizeof(Vector2));
            fread(stroke.points,sizeof(Vector2), stroke.pointCount, file);

            AddStrokeToPage(page, stroke);
        }
    }

    doc->activePage = 0;
    fclose(file);
    return true;
}


const char *ShowSaveFileDialog(){
#ifdef _WIN32
    static char filename[MAX_PATH] = "";
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Notitze Files (*.ntz)\0*.ntz\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "ntz";

    if(GetSaveFileName(&ofn)){
        return filename;
    }
#endif
    return NULL;
}

const char* ShowOpenFileDialog() {
#ifdef _WIN32
    static char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
ofn.lpstrFilter = "Notitze Files (*.ntz)\0*.ntz\0All Files (*.*)\0*.*\0";    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "ntz";

    if (GetOpenFileNameA(&ofn)) return filename;
#endif
    return NULL; 
}