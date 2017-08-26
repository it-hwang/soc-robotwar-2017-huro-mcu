// #define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "line_detection.h"
#include "log.h"
#include "debug.h"

#define _PI 3.141592
#define _DEG_TO_RAD  (_PI / 180)
#define _RAD_TO_DEG  (180 / _PI)
#define _DIFFERENCE_OF_ANGLE 10

#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))

static bool _isClosestLine(Line_t* currentLine, Line_t* prevLine);
static Line_t* _labelToLine(Matrix16_t* pLabelMatrix, Object_t* pObject);
static PixelLocation_t _searchCenterPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
static PixelLocation_t _searchRightPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
static PixelLocation_t _searchLeftPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
static double _getAngle(PixelLocation_t src, PixelLocation_t dst);
static bool _isFitRatio(double leftToCenterAngle, double centerToRightAngle, double leftToRightAngle);


Line_t* lineDetection(Matrix8_t* pColorMatrix) {
    Matrix16_t* pLabelMatrix = createMatrix16(pColorMatrix->width, pColorMatrix->height);
    memset(pLabelMatrix->elements, 0, (pLabelMatrix->height * pLabelMatrix->width) * sizeof(uint16_t));

    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    Line_t* pResultLine = NULL;

    for(int i = 0; i < pObjectList->size; ++i) {
        Line_t* pLine = _labelToLine(pLabelMatrix, &pObjectList->list[i]);
        
        bool isClosestLine = false;
        if(pLine != NULL) {
            if(pResultLine == NULL) {
                pResultLine = pLine;
                pLine = NULL;
            } else {
                isClosestLine = _isClosestLine(pLine, pResultLine);
            }
        }

        //pLine이 기존 라인보다 가까운 경우
        //기존 라인을 free하고 pLine의 주소를 물려준다.
        if(isClosestLine) {
            free(pResultLine);
            pResultLine = pLine;
        } else {
            if(pLine != NULL)
                free(pLine);
        }
    }

    if(pObjectList != NULL) {
        free(pObjectList->list);
        free(pObjectList);
    }

    destroyMatrix16(pLabelMatrix);

    if(pResultLine != NULL) {
        printDebug("최종 라인 기울기(%f).\n", pResultLine->theta);
        printDebug("최종 라인 거리(%f).\n", pResultLine->centerPoint.y);
    }

    return pResultLine;
}

static bool _isClosestLine(Line_t* currentLine, Line_t* prevLine) {
    
    int resultDistance = prevLine->centerPoint.y;
    int currentDistance = currentLine->centerPoint.y;
    if(resultDistance < currentDistance) { // 현재 라인의 거리가 더 가까운 경우
        return true;
    } else {
        return false;
    }
}

static Line_t* _labelToLine(Matrix16_t* pLabelMatrix, Object_t* pObject) {
    int objectWidth = pObject->maxX - pObject->minX + 1;
    
    if(objectWidth < pLabelMatrix->width)
        return NULL;

    int labelNum = pObject->id;

    Line_t* returnLine = NULL;

    PixelLocation_t centerPoint = _searchCenterPoint(pLabelMatrix, pObject, labelNum);

    PixelLocation_t rightPoint = _searchRightPoint(pLabelMatrix, pObject, labelNum);

    PixelLocation_t leftPoint = _searchLeftPoint(pLabelMatrix, pObject, labelNum);

    double leftToCenterAngle = _getAngle(leftPoint, centerPoint);

    double centerToRightAngle = _getAngle(centerPoint, rightPoint);

    double leftToRightAngle = _getAngle(leftPoint, rightPoint);

    bool isLine = _isFitRatio(leftToCenterAngle, centerToRightAngle, leftToRightAngle);

    if(isLine) {
        printDebug("왼쪽 기울기(%f).\n", leftToCenterAngle);
        printDebug("오른쪽 기울기(%f).\n", centerToRightAngle);
        printDebug("전체 기울기(%f).\n", leftToRightAngle);
        printDebug("중앙 거리(%d).\n", centerPoint.y);

        returnLine = (Line_t*)malloc(sizeof(Line_t));

        returnLine->centerPoint = centerPoint;
        returnLine->rightPoint = rightPoint;
        returnLine->leftPoint = leftPoint;
        returnLine->theta = leftToRightAngle;
    }

    return returnLine;
}

static PixelLocation_t _searchCenterPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

    PixelLocation_t returnLocation;

    int x = pObject->centerX;
    int y = pObject->maxY+1;
    int index;

    do {
        y--;
        index = y * pLabelMatrix->width + x;
    }while(pLabelMatrix->elements[index] != labelNum);

    returnLocation.x = x;
    returnLocation.y = y;

    return returnLocation;
}

static PixelLocation_t _searchRightPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

    PixelLocation_t returnLocation;

    int x = pObject->maxX;
    int y = pObject->maxY+1;
    int index;

    do {
        y--;
        index = y * pLabelMatrix->width + x;
    }while(pLabelMatrix->elements[index] != labelNum);

    returnLocation.x = x;
    returnLocation.y = y;

    return returnLocation;
}

static PixelLocation_t _searchLeftPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

    PixelLocation_t returnLocation;

    int x = pObject->minX;
    int y = pObject->maxY+1;
    int index;

    do {
        y--;
        index = y * pLabelMatrix->width + x;
    }while(pLabelMatrix->elements[index] != labelNum);

    returnLocation.x = x;
    returnLocation.y = y;

    return returnLocation;
}

//두 점을 이용한 기울기 계산
static double _getAngle(PixelLocation_t src, PixelLocation_t dst) {
    int dDeltaX;
    int dDeltaY;

    dDeltaX = dst.x - src.x;
    dDeltaY = dst.y - src.y;
    
    double dAngle = atan2(dDeltaY , dDeltaX);
    /*
    radian -> degree
    */                      
    dAngle *= _RAD_TO_DEG;
    //if( dAngle < 0.0 ) dAngle += 360.0;
    return dAngle;
}

static bool _isFitRatio(double leftToCenterAngle, double centerToRightAngle, double leftToRightAngle) {
    
    if(fabs(leftToCenterAngle - leftToRightAngle) > _DIFFERENCE_OF_ANGLE)
        return false;

    if(fabs(centerToRightAngle - leftToRightAngle) > _DIFFERENCE_OF_ANGLE)
        return false;

    return true;
}


static Line_t* _createLine(PixelLocation_t* pLoc1, PixelLocation_t* pLoc2);
static int _getTopVertexIndex(Polygon_t* pPolygon);
static int _getBottomVertexIndex(Polygon_t* pPolygon);
static int _getLeftVertexIndex(Polygon_t* pPolygon);
static int _getRightVertexIndex(Polygon_t* pPolygon);

Line_t* findTopLine(Polygon_t* pPolygon) {
    if (!pPolygon) return NULL;

    int size = pPolygon->size;
    if (size < 2) return NULL;

    int mainIndex = _getTopVertexIndex(pPolygon);
    int prevIndex = (mainIndex - 1 + size) % size;
    int nextIndex = (mainIndex + 1) % size;
    PixelLocation_t mainLoc = pPolygon->vertices[mainIndex];
    PixelLocation_t prevLoc = pPolygon->vertices[prevIndex];
    PixelLocation_t nextLoc = pPolygon->vertices[nextIndex];

    double prevSlope = _getAngle(prevLoc, mainLoc);
    double nextSlope = _getAngle(mainLoc, nextLoc);

    Line_t* pLine = NULL;
    if (fabs(prevSlope) < fabs(nextSlope))
        pLine = _createLine(&prevLoc, &mainLoc);
    else
        pLine = _createLine(&mainLoc, &nextLoc);

    return pLine;
}

Line_t* findBottomLine(Polygon_t* pPolygon) {
    if (!pPolygon) return NULL;

    int size = pPolygon->size;
    if (size < 2) return NULL;

    int mainIndex = _getBottomVertexIndex(pPolygon);
    int prevIndex = (mainIndex - 1 + size) % size;
    int nextIndex = (mainIndex + 1) % size;
    PixelLocation_t mainLoc = pPolygon->vertices[mainIndex];
    PixelLocation_t prevLoc = pPolygon->vertices[prevIndex];
    PixelLocation_t nextLoc = pPolygon->vertices[nextIndex];

    double prevSlope = _getAngle(prevLoc, mainLoc);
    double nextSlope = _getAngle(mainLoc, nextLoc);

    Line_t* pLine = NULL;
    if (fabs(prevSlope) < fabs(nextSlope))
        pLine = _createLine(&prevLoc, &mainLoc);
    else
        pLine = _createLine(&mainLoc, &nextLoc);

    return pLine;
}

Line_t* findLeftLine(Polygon_t* pPolygon) {
    if (!pPolygon) return NULL;

    int size = pPolygon->size;
    if (size < 2) return NULL;

    int mainIndex = _getLeftVertexIndex(pPolygon);
    int prevIndex = (mainIndex - 1 + size) % size;
    int nextIndex = (mainIndex + 1) % size;
    PixelLocation_t mainLoc = pPolygon->vertices[mainIndex];
    PixelLocation_t prevLoc = pPolygon->vertices[prevIndex];
    PixelLocation_t nextLoc = pPolygon->vertices[nextIndex];

    double prevSlope = _getAngle(prevLoc, mainLoc);
    double nextSlope = _getAngle(mainLoc, nextLoc);

    Line_t* pLine = NULL;
    if (fabs(fabs(prevSlope) - 90) < fabs(fabs(nextSlope) - 90))
        pLine = _createLine(&prevLoc, &mainLoc);
    else
        pLine = _createLine(&mainLoc, &nextLoc);

    return pLine;
}

Line_t* findRightLine(Polygon_t* pPolygon) {
    if (!pPolygon) return NULL;

    int size = pPolygon->size;
    if (size < 2) return NULL;

    int mainIndex = _getRightVertexIndex(pPolygon);
    int prevIndex = (mainIndex - 1 + size) % size;
    int nextIndex = (mainIndex + 1) % size;
    PixelLocation_t mainLoc = pPolygon->vertices[mainIndex];
    PixelLocation_t prevLoc = pPolygon->vertices[prevIndex];
    PixelLocation_t nextLoc = pPolygon->vertices[nextIndex];

    double prevSlope = _getAngle(prevLoc, mainLoc);
    double nextSlope = _getAngle(mainLoc, nextLoc);

    Line_t* pLine = NULL;
    if (fabs(fabs(prevSlope) - 90) < fabs(fabs(nextSlope) - 90))
        pLine = _createLine(&prevLoc, &mainLoc);
    else
        pLine = _createLine(&mainLoc, &nextLoc);

    return pLine;
}


static Line_t* _createLine(PixelLocation_t* pLoc1, PixelLocation_t* pLoc2) {
    Line_t* pLine = (Line_t*)malloc(sizeof(Line_t));

    if (pLoc1->x < pLoc2->x) {
        pLine->leftPoint = *pLoc1;
        pLine->rightPoint = *pLoc2;
    }
    else if (pLoc1->x > pLoc2->x) {
        pLine->leftPoint = *pLoc2;
        pLine->rightPoint = *pLoc1;
    }
    else if (pLoc1->y < pLoc2->y) {
        pLine->leftPoint = *pLoc1;
        pLine->rightPoint = *pLoc2;
    }
    else {
        pLine->leftPoint = *pLoc2;
        pLine->rightPoint = *pLoc1;
    }
    pLine->centerPoint.x = (pLine->leftPoint.x + pLine->rightPoint.x) / 2;
    pLine->centerPoint.y = (pLine->leftPoint.y + pLine->rightPoint.y) / 2;
    pLine->theta = _getAngle(pLine->leftPoint, pLine->rightPoint);

    return pLine;
}

static int _getTopVertexIndex(Polygon_t* pPolygon) {
    if (!pPolygon) return -1;

    int minY = 0;
    int vertexIndex = -1;

    int length = pPolygon->size;
    for (int i = 0; i < length; ++i) {
        PixelLocation_t* pVertexLoc = &(pPolygon->vertices[i]);
        int y = pVertexLoc->y;
        if (vertexIndex == -1 || y < minY) {
            vertexIndex = i;
            minY = y;
        }
    }

    return vertexIndex;
}

static int _getBottomVertexIndex(Polygon_t* pPolygon) {
    if (!pPolygon) return -1;

    int maxY = 0;
    int vertexIndex = -1;

    int length = pPolygon->size;
    for (int i = 0; i < length; ++i) {
        PixelLocation_t* pVertexLoc = &(pPolygon->vertices[i]);
        int y = pVertexLoc->y;
        if (vertexIndex == -1 || y > maxY) {
            vertexIndex = i;
            maxY = y;
        }
    }

    return vertexIndex;
}

static int _getLeftVertexIndex(Polygon_t* pPolygon) {
    if (!pPolygon) return -1;

    int minX = 0;
    int vertexIndex = -1;

    int length = pPolygon->size;
    for (int i = 0; i < length; ++i) {
        PixelLocation_t* pVertexLoc = &(pPolygon->vertices[i]);
        int x = pVertexLoc->x;
        if (vertexIndex == -1 || x < minX) {
            vertexIndex = i;
            minX = x;
        }
    }

    return vertexIndex;
}

static int _getRightVertexIndex(Polygon_t* pPolygon) {
    if (!pPolygon) return -1;

    int maxX = 0;
    int vertexIndex = -1;

    int length = pPolygon->size;
    for (int i = 0; i < length; ++i) {
        PixelLocation_t* pVertexLoc = &(pPolygon->vertices[i]);
        int x = pVertexLoc->x;
        if (vertexIndex == -1 || x > maxX) {
            vertexIndex = i;
            maxX = x;
        }
    }

    return vertexIndex;
}


void drawLine(Screen_t* pScreen, Line_t* pLine, Rgab5515_t* pLineColor) {
    Rgab5515_t lineColor;
    lineColor.r = 0x10;
    lineColor.g = 0x00;
    lineColor.b = 0x10;

    if (!pScreen) return;
    if (!pLine) return;
    if (!pLineColor)
        pLineColor = &lineColor;

    int x1 = pLine->leftPoint.x;
    int y1 = pLine->leftPoint.y;
    int x2 = pLine->rightPoint.x;
    int y2 = pLine->rightPoint.y;
    int minX = _MIN(x1, x2);
    int maxX = _MAX(x1, x2);
    int minY = _MIN(y1, y2);
    int maxY = _MAX(y1, y2);

    int width = pScreen->width;
    if (x1 == x2) {
        int x = x1;
        for (int y = minY; y <= maxY; ++y) {
            int index = y * width + x;
            pScreen->elements[index] = pLineColor->data;
        }
    }
    else if (abs(x2 - x1) > abs(y2 - y1)) {
        double a = (double)(y2 - y1) / (x2 - x1);
        for (int x = minX; x <= maxX; ++x) {
            int y = a * (x - x1) + y1;
            int index = y * width + x;
            pScreen->elements[index] = pLineColor->data;
        }
    }
    else {
        double a = (double)(x2 - x1) / (y2 - y1);
        for (int y = minY; y <= maxY; ++y) {
            int x = a * (y - y1) + x1;
            int index = y * width + x;
            pScreen->elements[index] = pLineColor->data;
        }
    }
}
