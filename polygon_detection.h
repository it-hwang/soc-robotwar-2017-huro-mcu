#ifndef __POLYGON_DETECTION_H__
#define __POLYGON_DETECTION_H__

#include "graphic_interface.h"
#include "object_detection.h"


typedef struct {
    PixelLocation_t* vertices;
    int size;
} Polygon_t;

Polygon_t* createPolygon(Matrix16_t* pLabelMatrix, Object_t* pObject, int threshold);
void destroyPolygon(Polygon_t* pPolygon);

void drawPolygon(Screen_t* pScreen, Polygon_t* pPolygon, Rgab5515_t* pPointColor);

#endif //__POLYGON_DETECTION_H__
