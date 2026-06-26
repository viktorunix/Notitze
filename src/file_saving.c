#include "include/file_saving.h"
#include <stdint.h>
#define MAX_PATH 1024
static void WriteChunk(FILE *file, const char tag[4], void *data, uint32_t size){
    fwrite(tag,1,4,file);
    fwrite(&size, sizeof(uint32_t),1,file);
    if(size > 0 && data != NULL)
        fwrite(data,1,size,file);
}
void SaveDocumentBinary(const char *filename, Document *doc){

    FILE *file = fopen(filename, "wb");
    if(!file) return;

    char magic[4] = "NTZ3";
    fwrite(magic, sizeof(char), 4, file);

    struct{
        float w, h;
        int ppi, count, pattern;
        bool layers, baked, pressure;
        float scale;
    } docMeta = {
        doc->pageWidth, doc->pageHeight, doc->ppi, doc->pageCount, doc->pattern,
        doc->enableLayers, doc->useBakedRendering, doc->pressureEnabled, doc->renderScale
    };
    WriteChunk(file, "DOC ", &docMeta, sizeof(docMeta));
    struct{
        Color c;
        float s;
    } bkgdMeta = {doc->patternColor, doc->patternSpacing};
    WriteChunk(file, "BKGD", &bkgdMeta, sizeof(bkgdMeta));
    for(int p = 0; p < doc->pageCount; p++){
        Page *page = &doc->pages[p];
        struct {
            int layerCount, activeLayer;
        } pageMeta = {page->layerCount, page->activeLayer};
        WriteChunk(file, "PAGE",&pageMeta, sizeof(pageMeta));

        for(int l = 0 ; l < page->layerCount; l++){
            Layer *layer = &page->layers[l];
            struct {
                bool isVisible;
                int strokeCount;
            } layerMeta = {layer->isVisible, layer->strokeCount};

            WriteChunk(file, "LAYR", &layerMeta, sizeof(layerMeta));
            for(int s = 0; s < layer->strokeCount; s++){
                Stroke *stroke = &layer->strokes[s];
                struct{
                    int type;
                    Color color;
                    float thickness;
                    int pointCount;
                } strokeMeta ={stroke->type, stroke->color, stroke->thickness, stroke->pointCount};

                WriteChunk(file, "STRM", &strokeMeta, sizeof(strokeMeta));

                uint32_t pointSize = stroke->pointCount * sizeof(StrokePoint);
                WriteChunk(file, "STRP", stroke->points, pointSize);
            }
        }
    }
    fclose(file);
}


bool LoadDocumentBinary(const char *filename, Document *doc){

    int totalLayers = 0;
    FILE *file = fopen(filename, "rb");
    if(!file) return false;
    char magic[5] = {0};
    fread(magic, 1, 4, file);
    if(strncmp(magic, "NTZ", 3) != 0){
        fclose(file);
        return false;
    }

    Texture2D tempBrush = doc->brushTex;
    Texture2D tempPencil = doc->pencilTex;

    FreeDocument(doc);
    doc->brushTex = tempBrush;
    doc->pencilTex = tempPencil;

    if(magic[3] == '2'){
        bool success = LoadLegacyNTZ2(file, doc);
        fclose(file);
        return success;
    }

    Page *currentPage = NULL;
    Layer *currentLayer = NULL;
    Stroke *currentStroke = NULL;

    while(!feof(file)){
        char tag[5] = {0};
        if(fread(tag, sizeof(char), 4, file) != 4) break;

        uint32_t chunkSize = 0;
        fread(&chunkSize, sizeof(uint32_t),1,file);
        long chunkEnd = ftell(file) + chunkSize;

        if(strcmp(tag, "DOC ") == 0){
            struct {
                float w, h;
                int ppi, count, pattern;
                bool layers, baked, pressure;
                float scale;
            } meta;
            fread(&meta, sizeof(meta),1,file);
            doc->pageWidth = meta.w;
            doc->pageHeight = meta.h;
            doc->ppi = meta.ppi;
            //doc->pageCount = meta.count;
            doc->pattern = meta.pattern;
            doc->enableLayers = meta.layers;
            doc->useBakedRendering = meta.baked;
            doc->pressureEnabled = meta.pressure;
            doc->renderScale = meta.scale;


        }
        else if(strcmp(tag, "BKGD") == 0){
            struct {Color c; float s;} meta;
            fread(&meta, sizeof(meta),1,file);
            doc->patternColor = meta.c;
            doc->patternSpacing = meta.s;
        }
        else if(strcmp(tag, "PAGE") == 0){
            AddPageToDocument(doc);
            currentPage = &doc->pages[doc->pageCount - 1];

            FreePage(currentPage);
            currentPage->layerCount = 0;
            currentPage->layerCapacity = 0;
            currentPage->layers = NULL;

            struct {int layerCount, activeLayer;}meta;
            fread(&meta, sizeof(meta),1,file);
            currentPage->activeLayer = meta.activeLayer;
        }
        else if(strcmp(tag, "LAYR") == 0){
            totalLayers++;
            if(!currentPage) continue;

            if(currentPage->layerCount >= currentPage->layerCapacity){
                currentPage->layerCapacity = currentPage->layerCapacity == 0 ? 4 : currentPage->layerCapacity * 2;
                currentPage->layers = (Layer *)realloc(currentPage->layers, currentPage->layerCapacity * sizeof(Layer));
            }

            currentLayer = &currentPage->layers[currentPage->layerCount];
            currentLayer->strokes = NULL;
            currentLayer->strokeCount = 0;
            currentLayer->capacity = 0;
            currentLayer->texture = (RenderTexture2D){0};
            struct {bool isVisible; int strokeCount;} meta;
            fread(&meta, sizeof(meta), 1, file);
            currentLayer->isVisible = meta.isVisible;

            currentPage->layerCount++;
        }
        else if(strcmp(tag, "STRM") == 0){
            if(!currentLayer) continue;
            if(currentLayer->strokeCount >= currentLayer->capacity){
                currentLayer->capacity = currentLayer->capacity == 0 ? 32 : currentLayer->capacity * 2;
                currentLayer->strokes = (Stroke *)realloc(currentLayer->strokes, currentLayer->capacity * sizeof(Stroke));
            }
            currentStroke = &currentLayer->strokes[currentLayer->strokeCount];
            struct {int type; Color color; float thickness; int pointCount;} meta;
            fread(&meta, sizeof(meta), 1, file);

            currentStroke->type = meta.type;
            currentStroke->color = meta.color;
            currentStroke->thickness = meta.thickness;
            currentStroke->pointCount = meta.pointCount;
            currentStroke->capacity = meta.pointCount;
            currentStroke->points = NULL;

            currentLayer->strokeCount++;
        }
        else if(strcmp(tag, "STRP") == 0){
            if(currentStroke && currentStroke->pointCount > 0){
                currentStroke->points = (StrokePoint *)malloc(chunkSize);
                fread(currentStroke->points, 1, chunkSize, file);
            }
        }

        fseek(file, chunkEnd, SEEK_SET);
    }

    int loadedLayers = 0;
    for(int p = 0; p < doc->pageCount; p++){
        Page *page = &doc->pages[p];
        for(int l = 0; l < page->layerCount; l++){
            Layer *layer = &page->layers[l];
            layer->texture = LoadRenderTexture((int)doc->pageWidth * doc->renderScale, (int)doc->pageHeight * doc->renderScale);
            SetTextureFilter(layer->texture.texture,TEXTURE_FILTER_BILINEAR);
            BeginTextureMode(layer->texture);
            ClearBackground(BLANK);
            Camera2D bakeCam = {0};
            bakeCam.zoom = doc->renderScale;
            BeginMode2D(bakeCam);
            for(int s = 0; s < layer->strokeCount; s++)
                RenderStroke(*doc, &layer->strokes[s],0);
            EndMode2D();
            EndTextureMode();
            loadedLayers++;
            GUILoading(loadedLayers, totalLayers);
        }
    }
    doc->activePage = 0;
    printf("Layers: %d\n", totalLayers);
    fclose(file);
    return true;
}
bool LoadLegacyNTZ2(FILE *file, Document *doc){


    fread(&doc->pageWidth, sizeof(float),1,file);
    fread(&doc->pageHeight, sizeof(float), 1, file);
    fread(&doc->ppi, sizeof(int), 1, file);

    int totalPages = 0;
    fread(&totalPages, sizeof(int), 1, file);
    fread(&doc->pattern, sizeof(int),1,file);
    fread(&doc->enableLayers, sizeof(bool),1,file);


    fread(&doc->useBakedRendering, sizeof(bool),1,file);
    fread(&doc->renderScale, sizeof(float), 1, file);




    fread(&doc->pressureEnabled, sizeof(bool),1,file);

    for(int p = 0; p < totalPages; p++){
        AddPageToDocument(doc);
        Page *page = &doc->pages[p];

        FreePage(page);
        page->layerCount = 0;
        page->layerCapacity = 0;
        page->layers = NULL;

        fread(&page->layerCount, sizeof(int),1,file);
        fread(&page->activeLayer, sizeof(int), 1, file);

        page->layerCapacity = page->layerCount;
        page->layers = (Layer *)malloc(page->layerCapacity * sizeof(Layer));

        for(int l = 0; l < page->layerCount; l++){
            Layer *layer = &page->layers[l];
            layer->capacity = 0;
            layer->strokes = NULL;
            fread(&layer->isVisible, sizeof(bool),1,file);
            fread(&layer->strokeCount, sizeof(int), 1, file);

            layer->capacity = layer->strokeCount;
            if(layer->capacity > 0)
                layer->strokes = (Stroke *)malloc(layer->capacity * sizeof(Stroke));

            for(int s = 0; s < layer->strokeCount; s++){
                Stroke stroke = {0};
                fread(&stroke.type, sizeof(int), 1, file);
                fread(&stroke.color, sizeof(Color),1,file);
                fread(&stroke.thickness, sizeof(float),1,file);
                fread(&stroke.pointCount, sizeof(int),1,file);
                stroke.capacity = stroke.pointCount;
                stroke.points = (StrokePoint *)malloc(stroke.capacity * sizeof(StrokePoint));
                fread(stroke.points, sizeof(StrokePoint), stroke.pointCount, file);
                layer->strokes[s] = stroke;
            }
            layer->texture = LoadRenderTexture((int)doc->pageWidth * doc->renderScale, (int)doc->pageHeight * doc->renderScale);
            SetTextureFilter(layer->texture.texture, TEXTURE_FILTER_BILINEAR);
            BeginTextureMode(layer->texture);
            ClearBackground(BLANK);
            Camera2D bakeCam = {0};
            bakeCam.zoom = doc->renderScale;
            BeginMode2D(bakeCam);
            for(int s = 0; s < layer->strokeCount; s++)
                RenderStroke(*doc, &layer->strokes[s], 0);
            EndMode2D();
            EndTextureMode();
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
#elif defined(__linux__)
    static char filename[1024] = "";
    filename[0] = '\0'; // Wipe ghost paths

    // 1. Run zenity via system() which forces the program to completely FREEZE and wait.
    // We output the file selection to a temporary hidden file in /tmp/
    int status = system("zenity --file-selection --save --confirm-overwrite --title=\"Save Document\" --file-filter=\"*.ntz\" > /tmp/ntz_save_path.tmp 2>/dev/null");

    // WIFEXITED and WEXITSTATUS check if the user clicked "Cancel" or closed the window (returns non-zero status)
    if (status != 0) {
        remove("/tmp/ntz_save_path.tmp");
        return NULL;
    }

    // 2. Read the selected path back from our temporary file
    FILE *f = fopen("/tmp/ntz_save_path.tmp", "r");
    if (f) {
        if (fgets(filename, sizeof(filename), f) != NULL) {
            filename[strcspn(filename, "\n")] = '\0'; // Strip newline character
        }
        fclose(f);
        remove("/tmp/ntz_save_path.tmp"); // Clean up temporary file
    }

    if (strlen(filename) > 0) {
        // Auto-append .ntz if you forgot to type it
        size_t len = strlen(filename);
        if (len > 0 && (len < 4 || strcmp(filename + len - 4, ".ntz") != 0)) {
            if (len + 5 < sizeof(filename)) {
                strcat(filename, ".ntz");
            }
        }
        return filename;
    }
    return NULL;
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
#elif defined(__linux__)
    static char filename[1024] = "";
    filename[0] = '\0'; // Wipe ghost paths

    // Freeze application and wait for selection
    int status = system("zenity --file-selection --title=\"Open Document\" --file-filter=\"*.ntz\" > /tmp/ntz_open_path.tmp 2>/dev/null");

    if (status != 0) {
        remove("/tmp/ntz_open_path.tmp");
        return NULL;
    }

    FILE *f = fopen("/tmp/ntz_open_path.tmp", "r");
    if (f) {
        if (fgets(filename, sizeof(filename), f) != NULL) {
            filename[strcspn(filename, "\n")] = '\0'; // Strip newline
        }
        fclose(f);
        remove("/tmp/ntz_open_path.tmp");
    }

    if (strlen(filename) > 0) {
        return filename;
    }
    return NULL;

#endif
    return NULL;
}
