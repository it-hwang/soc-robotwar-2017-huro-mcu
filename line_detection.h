#ifndef __LINE_DETECTION_H__
#define __LINE_DETECTION_H__

#include <stdint.h>
#include "object_detection.h"
#include "graphic_interface.h"
#include "matrix.h"


typedef struct {
    double theta;     //기울기
    PixelLocation_t centerPoint;  //Line과의 거리를 확인할 point
    PixelLocation_t rightPoint;
    PixelLocation_t leftPoint;
} Line_t;

Line_t* lineDetection(Matrix8_t* pColorMatrix);

#endif // __LINE_DETECTION_H__
