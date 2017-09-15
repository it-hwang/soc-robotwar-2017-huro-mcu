#ifndef __LINE_DETECTION_H__
#define __LINE_DETECTION_H__

#include <stdint.h>
#include "graphic_interface.h"
#include "matrix.h"
#include "object_detection.h"
#include "polygon_detection.h"


typedef struct {
    double theta;     //기울기
    PixelLocation_t centerPoint;  //Line과의 거리를 확인할 point
    PixelLocation_t rightPoint;
    PixelLocation_t leftPoint;
} Line_t;

Line_t* lineDetection(Matrix8_t* pColorMatrix);

Line_t* findTopLine(Polygon_t* pPolygon);
Line_t* findBottomLine(Polygon_t* pPolygon);
Line_t* findLeftLine(Polygon_t* pPolygon);
Line_t* findRightLine(Polygon_t* pPolygon);

void drawLine(Screen_t* pScreen, Line_t* pLine, Rgab5515_t* pLineColor);

#endif // __LINE_DETECTION_H__
