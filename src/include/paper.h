#ifndef PAPER
#define PAPER
typedef enum{
    FORMAT_A4 = 0,
    FORMAT_A3,
    FORMAT_A5,
    FORMAT_LETTER,
    FORMAT_CUSTOM
} PaperFormat;
#define START_PPI 96
#define CUSTOM_W 1920
#define CUSTOM_H 1080
#endif