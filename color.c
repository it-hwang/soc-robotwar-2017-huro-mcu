#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "color.h"

void _registerColor(Color_t color, uint8_t r, uint8_t g, uint8_t b);
ColorTable_t _createColorTable(uint64_t length, Color_t (*pFunc)(PixelData_t));


void initColorLib(void) {
    static bool hasInitialized = false;
    if (hasInitialized)
        return;
    
    _registerColor(COLOR_BLACK, 0x00, 0x00, 0x00);
    _registerColor(COLOR_WHITE, 0xff, 0xff, 0xff);
    _registerColor(COLOR_RED, 0xff, 0x00, 0x00);
    _registerColor(COLOR_GREEN, 0x00, 0xff, 0x00);
    _registerColor(COLOR_BLUE, 0x00, 0x00, 0xff);
    _registerColor(COLOR_YELLOW, 0xff, 0xff, 0x00);
    _registerColor(COLOR_ORANGE, 0xff, 0x80, 0x27);
    
    hasInitialized = true;
}

bool createColorTableFile(const char* filePath, Color_t (*pFunc)(PixelData_t),
                          bool overwrite) {
    bool isFileExists = access(filePath, F_OK) == 0;

    if (isFileExists && !overwrite) {
        return false;
    }

    uint64_t colorTableLength = pow(2, sizeof(PixelData_t) * 8);
    ColorTable_t colorTable = _createColorTable(colorTableLength, pFunc);

    FILE* outputFile = fopen(filePath, "w");
    if (outputFile == NULL) {
        free(colorTable);
        return false;
    }

    fwrite(colorTable, sizeof(Color_t), colorTableLength, outputFile);
    fclose(outputFile);
    free(colorTable);
    return true;
}

ColorTable_t loadColorTableFile(const char* filePath) {
    FILE* inputFile = fopen(filePath, "r");
    if (inputFile == NULL)
        return NULL;

    uint64_t colorTableLength = pow(2, sizeof(PixelData_t) * 8);
    ColorTable_t colorTable = _createColorTable(colorTableLength, NULL);

    fread(colorTable, sizeof(Color_t), colorTableLength, inputFile);

    fclose(inputFile);
    return colorTable;
}


void _registerColor(Color_t color, uint8_t r, uint8_t g, uint8_t b) {
    Rgba_t rgba;
    rgba.r = r;
    rgba.g = g;
    rgba.b = b;
    colorToRgb565DataTable[color] = rgbaToRgb565Data(&rgba);
}

ColorTable_t _createColorTable(uint64_t length, Color_t (*pFunc)(PixelData_t)) {
    ColorTable_t colorTable;
    uint64_t i = 0;

    colorTable = (ColorTable_t)malloc(length * sizeof(Color_t));
    if (pFunc)
        for (i = 0; i < length; ++i)
            colorTable[i] = (*pFunc)(i);

    return colorTable;
}

ColorScreen_t* createColorScreen(PixelCoordinate_t width, PixelCoordinate_t height) {
    int nPixels = height * width;
    ColorScreen_t* pScreen = (ColorScreen_t*)malloc(sizeof(ColorScreen_t));
    pScreen->pixels = (Color_t*)malloc(nPixels * sizeof(Color_t));
    pScreen->height = height;
    pScreen->width = width;

    return pScreen;
}

void destroyColorScreen(ColorScreen_t* pScreen) {
    free(pScreen->pixels);
    free(pScreen);
}

bool readColorFromScreen(ColorScreen_t* pScreen, Screen_t* pSource,
                         ColorTable_t colorTable) {
    if (pScreen->width != pSource->width || pScreen->height != pSource->height)
        return false;

    int nPixels = pScreen->height * pScreen->width;
    int i;
    Color_t* pColor = pScreen->pixels;
    PixelData_t* pPixelData = pSource->pixels;

    for (i = 0; i < nPixels; ++i) {
        *pColor = getColorFromTable(colorTable, *pPixelData);
        pColor++;
        pPixelData++;
    }

    return true;
}

bool writeColorToScreen(ColorScreen_t* pScreen, Screen_t* pTarget) {
    if (pScreen->width != pTarget->width || pScreen->height != pTarget->height)
        return false;

    int nPixels = pScreen->height * pScreen->width;
    int i;
    Color_t* pColor = pScreen->pixels;
    PixelData_t* pPixelData = pTarget->pixels;

    for (i = 0; i < nPixels; ++i) {
        *pPixelData = colorToRgb565Data(*pColor);
        pColor++;
        pPixelData++;
    }

    return true;
}
