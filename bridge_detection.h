#ifndef __BRIDGE_DETECTIONV2_H__
#define __BRIDGE_DETECTIONV2_H__

#include <stdint.h>
#include "object_detection.h"
#include "graphic_interface.h"
#include "matrix.h"

typedef struct {
    bool isRight;
    double theta;
    PixelLocation_t centerPoint;
    int cnt;
} Bridge_t;

Bridge_t* bridgeDetection(Matrix8_t* pColorMatrix);

#endif // __LINE_DETECTIONV2_H__
