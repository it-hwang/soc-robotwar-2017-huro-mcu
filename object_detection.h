#ifndef __OBJECT_DETECTION_H__
#define __OBJECT_DETECTION_H__

#include "color.h"

typedef struct{
    uint8_t minX;
    uint8_t minY;
    uint8_t maxX;
    uint8_t maxY;
    uint8_t centerX;
    uint8_t centerY;
    Color_t color;
} Object_t;

typedef struct{
    Object_t* list;
    uint8_t size;
} ObjectList_t;

ObjectList_t* detectObjectsLocation(uint16_t* pixels, ColorTable_t colorTable, Color_t flagColor);

#endif //__OBJECT_DETECTION_H__