#include <stdio.h>
#include <string.h>
#include "object_detection.h"

#define _WIDTH   180
#define _HEIGHT  120

typedef struct {
    int x;
    int y;
} Location_t;


ObjectList_t* detectObjectsLocation(uint16_t* pixels, ColorTable_t colorTable,
                                    Color_t flagColor) {
    int x;
    int y;

    uint16_t labeledPixels[_HEIGHT][_WIDTH];
    uint16_t equalLabelList[1001] = {0,};
    int labelCntList[1001] = {0,};
    int lastLabel = 0;

    for(y = 0; y < _HEIGHT; ++y) {
        for(x = 0; x < _WIDTH; ++x) {
            int index = y * _WIDTH + x;
            uint16_t pixelData = pixels[index];
            Color_t color = getColorFromTable(colorTable, pixelData);

            if(color == flagColor) {
                if(x > 0 && y > 0){
                    uint16_t up = labeledPixels[y-1][x];
                    uint16_t left = labeledPixels[y][x-1];
                    uint16_t upLeft = labeledPixels[y-1][x-1];
                    uint16_t upRight = labeledPixels[y-1][x+1];

                    
                }
            }
        }
    }
    
    return NULL;
}

/*
ObjectList_t* detectObjectsLocation(uint16_t* pixels, ColorTable_t colorTable,
                                    Color_t flagColor) {
    int x;
    int y;
    //Color_t coloredPixels[_HEIGHT][_WIDTH];
    uint16_t labeledPixels[_HEIGHT][_WIDTH];
    uint16_t equalLabelList[101] = {0,};
    int labelCntList[101] = {0,};
    //Location_t labelCenterList[1001];
    int lastLabel = 0;

    memset(labeledPixels, 0, (_HEIGHT * _WIDTH) * sizeof(uint16_t));
    
    for (y = 0; y < _HEIGHT; ++y) {
        for (x = 0; x < _WIDTH; ++x) {
            int index = y * _WIDTH + x;
            uint16_t pixelData = pixels[index];
            Color_t color = getColorFromTable(colorTable, pixelData);            

            if(color == flagColor) {
                //coloredPixels[y][x] = color;
                if(x > 0 && y > 0) {
                    uint16_t leftLabel = labeledPixels[y][x-1];
                    uint16_t upLabel = labeledPixels[y-1][x];

                    if(leftLabel == 0 && upLabel == 0) {
                        ++lastLabel;
                        labeledPixels[y][x] = lastLabel;
                        ++labelCntList[lastLabel];

                    } else if(leftLabel != 0 && upLabel != 0) {
                        uint16_t selectedLabel = 0;

                        if (leftLabel > upLabel) {
                            selectedLabel = leftLabel;
                            labeledPixels[y][x] = upLabel;
                            ++labelCntList[upLabel];
                            equalLabelList[leftLabel] = upLabel;
                        } else if (leftLabel < upLabel) {
                            selectedLabel = upLabel;
                            labeledPixels[y][x] = leftLabel;
                            ++labelCntList[leftLabel];
                            equalLabelList[upLabel] = leftLabel;
                        } else {
                            selectedLabel = 0;
                            labeledPixels[y][x] = leftLabel;
                            ++labelCntList[leftLabel];
                        }

                        if (selectedLabel > 0) {
                            uint16_t equalIndex = equalLabelList[selectedLabel];
                            while(equalLabelList[equalIndex] != 0) {
                                equalIndex = equalLabelList[equalIndex];
                            }

                            equalLabelList[selectedLabel] = equalIndex;
                        }

                    } else {
                        uint16_t anyLabel = leftLabel + upLabel;
                        labeledPixels[y][x] = anyLabel;
                        ++labelCntList[anyLabel];
                    }
                }
            }
            else {
                //coloredPixels[y][x] = COLOR_WHITE;
            }

            uint16_t outputLabel = labeledPixels[y][x];
            Rgb565_t* pOutput = (Rgb565_t*)&pixels[index];
            if(outputLabel != 0) {
                int outputColor = (equalLabelList[outputLabel] % 3) + 3;
                pOutput->data = colorToRgb565Data((Color_t)outputColor);
            }else {
                pOutput->data = colorToRgb565Data(COLOR_WHITE);
            }
        }
    }
    
    return NULL;
}*/
