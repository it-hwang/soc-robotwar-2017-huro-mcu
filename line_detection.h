#ifndef __LINE_DETECTION_H__
#define __LINE_DETECTION_H__

#include <stdint.h>
#include "object_detection.h"
#include "graphic_interface.h"
#include "matrix.h"


typedef struct {
    double theta;     //기울기
    PixelLocation_t distancePoint;  //Line과의 거리를 확인할 point
} Line_t;

Line_t* lineDetection(Matrix8_t* pObjectLineMatrix , ObjectList_t* pSubObject);
PixelLocation_t* _searchToTop(Matrix8_t* pObjectLineMatrix, PixelLocation_t* pPixel);
PixelLocation_t* _searchToBottom(Matrix8_t* pObjectLineMatrix, PixelLocation_t* pPixel);
Line_t* _computeTheta()

#endif // __LINE_DETECTION_H__


