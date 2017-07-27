#ifndef __OBJECT_DETECTION_H__
#define __OBJECT_DETECTION_H__

#include "color.h"

typedef struct{
    uint8_t minX;
    uint8_t minY;
    uint8_t maxX;
    uint8_t maxY;
    float centerX;
    float centerY;
    Color_t color;
    int cnt;
} Object_t;

typedef struct{
    Object_t* list;
    uint8_t size;
} ObjectList_t;

ObjectList_t* detectObjectsLocation(Matrix8_t* matrix);

#endif //__OBJECT_DETECTION_H__
