#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "line_detection.h"
#include "log.h"

#define PI 3.141592
#define DIFFERENCE_OF_ANGLE 10

static bool _isClosestLine(Line_t* currentLine, Line_t* prevLine);
static Line_t* _labelToLine(Matrix16_t* pLabelMatrix, Object_t* pObject);
static PixelLocation_t _searchCenterPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
static PixelLocation_t _searchRightPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
static PixelLocation_t _searchLeftPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
static double _getAngle(PixelLocation_t src, PixelLocation_t dst);
static bool _isFitRatio(double leftToCenterAngle, double centerToRightAngle, double leftToRightAngle);


Line_t* lineDetection(Matrix8_t* pColorMatrix) {
    static const char* LOG_FUNCTION_NAME = "lineDetection()";

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

    printLog("[%s] 최종 라인 기울기(%f).\n", LOG_FUNCTION_NAME, pResultLine->theta);
    printLog("[%s] 최종 라인 거리(%f).\n", LOG_FUNCTION_NAME, pResultLine->centerPoint.y);

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
    static const char* LOG_FUNCTION_NAME = "_labelToLine()";

    int objectWidth = pObject->maxX - pObject->minX + 1;
    
    if(objectWidth < pLabelMatrix->width)
        return NULL;

    int index = pObject->centerY * pLabelMatrix->width + pObject->centerY;
    int labelNum = pLabelMatrix->elements[index];

    Line_t* returnLine = NULL;

    PixelLocation_t centerPoint = _searchCenterPoint(pLabelMatrix, pObject, labelNum);

    PixelLocation_t rightPoint = _searchRightPoint(pLabelMatrix, pObject, labelNum);

    PixelLocation_t leftPoint = _searchLeftPoint(pLabelMatrix, pObject, labelNum);

    double leftToCenterAngle = _getAngle(leftPoint, centerPoint);

    double centerToRightAngle = _getAngle(centerPoint, rightPoint);

    double leftToRightAngle = _getAngle(leftPoint, rightPoint);

    bool isLine = _isFitRatio(leftToCenterAngle, centerToRightAngle, leftToRightAngle);

    if(isLine) {
        printLog("[%s] 왼쪽 기울기(%f).\n", LOG_FUNCTION_NAME, leftToCenterAngle);
        printLog("[%s] 오른쪽 기울기(%f).\n", LOG_FUNCTION_NAME, centerToRightAngle);
        printLog("[%s] 전체 기울기(%f).\n", LOG_FUNCTION_NAME, leftToRightAngle);
        printLog("[%s] 중앙 거리(%d).\n", LOG_FUNCTION_NAME, centerPoint.y);

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
    dAngle *= (180.0/PI);
    //if( dAngle < 0.0 ) dAngle += 360.0;
    return dAngle;
}

static bool _isFitRatio(double leftToCenterAngle, double centerToRightAngle, double leftToRightAngle) {
    
    if(fabs(leftToCenterAngle - leftToRightAngle) > DIFFERENCE_OF_ANGLE)
        return false;

    if(fabs(centerToRightAngle - leftToRightAngle) > DIFFERENCE_OF_ANGLE)
        return false;

    return true;
}
