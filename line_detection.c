#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "line_detection.h"

#define PI 3.141592
#define DIFFERENCE_OF_ANGLE 20

bool _isClosestLine(Line_t* currentLine, Line_t* prevLine);
Line_t* _labelToLine(Matrix16_t* pLabelMatrix, Object_t* pObject);
PixelLocation_t _searchCenterPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
PixelLocation_t _searchRightPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
PixelLocation_t _searchLeftPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum);
double _getAngle(PixelLocation_t src, PixelLocation_t dst);
bool _isFitRatio(double leftToCenterAngle, double centerToRightAngle, double leftToRightAngle);


Line_t* lineDetection(Matrix8_t* pColorMatrix) {
    
    Matrix16_t* pLabelMatrix = createMatrix16(pColorMatix->width, pColorMatrix->height);
    memset(pLabelMatrix->elements, 0, (pColorMatix->height * pColorMatix->width) * sizeof(uint16_t));

    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    Line_t* pResultLine = NULL;

    for(int i = 0; i < pObjectList->size; ++i) {
        Line_t* pLine = _labelToLine(pColorMatrix, &pObjectList->list[i]);

        bool isClosestLine = false;
        if(pLine != NULL) {
            if(pResultLine == NULL) {
                pResultLine = pLine;
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
            free(pLine);
        }
    }

    if(pObjectList != NULL) {
        free(pObjectList->list);
        free(pObjectList);
    }

    destroyMatrix16(pLabelMatrix);

    return pResultLine;

}

bool _isClosestLine(Line_t* currentLine, Line_t* prevLine) {
    
    int resultDistance = prevLine->centerPoint.y;
    int currentDistance = currentLine->centerPoint.y;
    if(resultDistance < currentDistance) { // 현재 라인의 거리가 더 가까운 경우
        return true;
    } else {
        return false;
    }
}

Line_t* _labelToLine(Matrix16_t* pLabelMatrix, Object_t* pObject) {

    int objectWidth = pObject->maxX - pObject->minX;
    
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

    bool isLine = _isFitRatio(leftToCenterAngle, centerToLeftAngle, leftToRightAngle);

    if(isLine) {
        returnLine = (Line_t*)malloc(sizeof(Line_t));

        returnLine->centerPoint = centerPoint;
        returnLine->rightPoint = rightPoint;
        returnLine->leftPoint = leftPoint;
        returnLine->theta = leftToRightAngle;
    }

    return returnLine;
}

PixelLocation_t _searchCenterPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

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

PixelLocation_t _searchRightPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

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

PixelLocation_t _searchLeftPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

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
double _getAngle(PixelLocation_t src, PixelLocation_t dst) {
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

bool _isFitRatio(double leftToCenterAngle, double centerToRightAngle, double leftToRightAngle) {
    
    if(fabs(leftToCenterAngle - leftToRightAngle) > DIFFERENCE_OF_ANGLE)
        return false;

    if(fabs(centerToRightAngle - leftToRightAngle) > DIFFERENCE_OF_ANGLE)
        return false;

    return true;
}
