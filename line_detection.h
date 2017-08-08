#ifndef __LINE_DETECTION_H__
#define __LINE_DETECTION_H__

#include <stdint.h>
#include "object_detection.h"
#include "graphic_interface.h"
#include "matrix.h"


typedef struct {
    double theta;     //기울기
    PixelLocation_t distancePoint;  //Line과의 거리를 확인할 point
    /*
    PixelLocation_t centerUpPoint;
    PixelLocation_t centerDownPoint;
    
    PixelLocation_t leftUpPoint;
    PixelLocation_t leftDownPoint;
    
    PixelLocation_t rightUpPoint;
    PixelLocation_t rightDownPoint;
    */
} Line_t;

Line_t* lineDetection(Matrix8_t* pColorMatrix, Matrix8_t* pSubMatrix);

#endif // __LINE_DETECTION_H__
