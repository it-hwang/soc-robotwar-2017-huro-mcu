#include <stdio.h>
#include "object_detection.h"

#define _WIDTH   180
#define _HEIGHT  120

ObjectList_t* detectObjectsLocation(uint16_t* pixels, ColorTable_t colorTable,
                                    Color_t flagColor) {
    int x;
    int y;
    Color_t coloredPixels[_HEIGHT][_WIDTH];
    uint16_t labelPixels[_HEIGHT][_WIDTH];
    
    for (y = 0; y < _HEIGHT; ++y) {
        for (x = 0; x < _WIDTH; ++x) {
            int index = y * _WIDTH + x;
            uint16_t pixelData = pixels[index];
            Color_t color = getColorFromTable(colorTable, pixelData);            

            if(color == flagColor)
                coloredPixels[y][x] = color;
            else
                coloredPixels[y][x] = COLOR_WHITE;
        }
    }
}
