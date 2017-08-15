#ifndef __OBJECT_DETECTION_H__
#define __OBJECT_DETECTION_H__

#include <stdint.h>
#include "color.h"

typedef struct{
    uint8_t minX;
    uint8_t minY;
    uint8_t maxX;
    uint8_t maxY;
    float centerX;
    float centerY;
    Color_t color;
    uint16_t id;
    int cnt;
} Object_t;

typedef struct{
    Object_t* list;
    uint8_t size;
} ObjectList_t;

ObjectList_t* detectObjectsLocation(Matrix8_t* pMatrix);
ObjectList_t* detectObjectsLocationWithLabeling(Matrix8_t* pMatrix,
                                                Matrix16_t* pLabelMatrix);

// pObject가 직사각형과의 유사한 정도를 반환한다. (범위: 0.0 ~ 1.0)
float getRectangleCorrelation(Matrix8_t* pMatrix, Object_t* pObject);

#endif //__OBJECT_DETECTION_H__
