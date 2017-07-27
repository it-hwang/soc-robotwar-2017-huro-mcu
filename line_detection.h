#ifndef __LINE_DETECTION_H__
#define __LINE_DETECTION_H__

#include <stdint.h>
#include "object_detection.h"
#include "graphic_interface.h"
#include "matrix.h"


typedef struct {
    uint16_t theta;     //기울기
    uint8_t linePointX;
    uint8_t linePointY;
} Line_t;

Line_t* lineDetection(Matrix8_t* pObjectLineMatrix , ObjectList_t* pSubObject);

#endif // __LINE_DETECTION_H__


