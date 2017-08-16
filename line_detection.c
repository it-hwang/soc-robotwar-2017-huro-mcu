#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "line_detection.h"

#define PI 3.141592

bool _isClosestLine(Line_t* currentLine, Line_t* prevLine);
bool _labelToLine(Matrix16_t* pLabelMatrix, Object_t* pObject);
PixelLocation_t _searchToTop(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToBottom(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToTopCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToBottomCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
double _getAngle(PixelLocation_t src, PixelLocation_t dst);


Line_t* lineDetection(Matrix8_t* pColorMatrix) {
    
    Matrix16_t* pLabelMatrix = createMatrix16(pColorMatix->width, pColorMatrix->height);
    memset(pLabelMatrix->elements, 0, (pSubMatrix->height * pSubMatrix->width) * sizeof(uint16_t));

    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatix, pLabelMatrix);

    Line_t* pResultLine = NULL;

    for(int i = 0; i < pObjectList->size; ++i) {
        Line_t* pLine = _labelToLine(pColorMatrix, &pObjectList->list[i]);

        bool isClosestLine = false;
        if(pLine != NULL) {
            if(pResultLline == NULL) {
                pResultLine = pLine;
            } else {
                isClosestLine = _isClosestLine(pLine, pResultLine);
            }
        }

        if(isClosestLine) {
            free(pResultLine);
            pResultLine = pLine;
        } else {
            free(pLine);
        }
    }

    if(pLine != NULL) {
        free(pLine);
    }

    if(pObjectList != NULL) {
        free(pObjectList->list);
        free(pObjectList);
    }

    destroyMatrix16(pLabelMatrix);

    return pResultLine;

}

bool _isClosestLine(Line_t* currentLine, Line_t* prevLine) {
    
    int resultDistance = pResultLine->centerPoint.y;
    int currentDistance = pLine->centerPoint.y;
    if(resultDistance < currentDistance) { // 현재 라인의 거리가 더 가까운 경우
        return true;
    } else {
        return false;
    } 
}

Line_t* _labelToLine(Matrix16_t* pLabelMatrix, Object_t* pObject) {

    int index = pObject->centerY * pLabelMatrix->width + pObject->centerY;
    int labelNum = pLabelMatrix->elements[index];

    Line_t* returnLine = NULL;

    PixelLocation_t centerPoint = _searchCenterPoint(pLabelMatrix, pObject, labelNum);

    PixelLocation_t rightPoint = _searchRightPoint(pLabelMatrix, pObject, labelNum);

    PixelLocation_t leftpoint = _serchLeftPoint(pLabelMatrix, pObject, labelNum);

    double leftToCenterAngle = _getAngle(leftPoint, centerPoint);

    double centerToLeftAngle = _getAngle(centerPoint, leftPoint);

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
    int y = pObject->centerY+1;

    do {
        y--;
        int index = y * pLabelMatrix->width + x;
    }while(pLabelMatrix->elements[index] != labelNum);

    returnLocation.x = x;
    returnLocation.y = y;

    return returnLocation;
}

PixelLocation_t _searchRightPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

    PixelLocation_t returnLocation;

    int x = pObject->minX;
    int y = pObject->minY+1;

    do {
        y--;
        int index = y * pLabelMatrix->width + x;
    }while(pLabelMatrix->elements[index] != labelNum);

    returnLocation.x = x;
    returnLocation.y = y;

    return returnLocation;
}

PixelLocation_t _serchLeftPoint(Matrix16_t* pLabelMatrix, Object_t* pObject, int labelNum) {

    PixelLocation_t returnLocation;

    int x = pObject->maxX;
    int y = pObject->maxY+1;

    do {
        y--;
        int index = y * pLabelMatrix->width + x;
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

/*****************************************
주어진 point를 인자로 받아 Search를 진행한다.
*****************************************/
PixelLocation_t _searchToTop(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //위를 향한 Search
    PixelLocation_t resultPixel; 
    int x = pPixel->x;
    resultPixel.x = x;
    
    int y;
    int cnt = 0;
    for(y=pPixel->y; y>=0; --y) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        if((int)*output != labelNum) {
            cnt++;
        }
        else {
            resultPixel.y = y;
            break;
        }
    }
    return resultPixel;
}

PixelLocation_t _searchToBottom(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //아래를 향한 Search
    PixelLocation_t resultPixel;
    int x = pPixel->x;
    resultPixel.x = x;
    
    int y;
    int cnt = 0;
    for(y=pPixel->y; y<= pObjectLineMatrix->height; ++y) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        if((int)*output != labelNum) {
            cnt++;
        }
        else {
            resultPixel.y = y;
            break;
        }
    }
    return resultPixel;
}

PixelLocation_t _searchToTopCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //위를 향한 Search
    PixelLocation_t resultPixel; 
    int x = pPixel->x;
    resultPixel.x = x;
    int y;
    int cnt = 0;
    for(y=pPixel->y; y>=0; y--) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        
        if((int)*output == labelNum) {
            cnt++;
        }
        else {
            resultPixel.y = y;
            break;
        }
    }
    return resultPixel;
}

PixelLocation_t _searchToBottomCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //아래를 향한 Search
    PixelLocation_t resultPixel;
    int x = pPixel->x;
    resultPixel.x = x;
    
    int y;
    int cnt = 0;
    for(y=pPixel->y; y<= pObjectLineMatrix->height; y++) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        if((int)*output == labelNum) {
            cnt++;
        }
        else {
            resultPixel.y = y;
            break;
        }
    }

    return resultPixel;
}
