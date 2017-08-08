#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "line_detection.h"

#define PI 3.141592

bool _labelToLine(Matrix16_t* pObjectLineMatrix, Object_t* object, Line_t* candidate, int labelNum);
PixelLocation_t _searchToTop(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToBottom(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToTopCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToBottomCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
double _getAngle(PixelLocation_t src, PixelLocation_t dst);


//SubMatrix와 해당 SubMatrix의 LabelList를 인자로 받아 LineDetection을 진행한다.
Line_t* lineDetection(Matrix8_t* pColorMatrix, Matrix8_t* pSubMatrix) {
    
    Matrix16_t* pLabelMatrix = createMatrix16(pSubMatrix->width, pSubMatrix->height);   
    memset(pLabelMatrix->elements, 0, (pSubMatrix->height * pSubMatrix->width) * sizeof(uint16_t));

    ObjectList_t* pObjectList;
    
    pObjectList = detectObjectsLocationWithLabeling(pSubMatrix, pLabelMatrix);
    //printf("list size 2 %d\n", pObjectList->size);
    int i;
    Line_t* resultLine = (Line_t*)malloc(sizeof(Line_t));
    resultLine->distancePoint.y = 0;

    Line_t* line = (Line_t*)malloc(sizeof(Line_t));
    bool emptyLine = true;
    
    for(i = 0; i < pObjectList->size; i++) {
        Object_t* object = &(pObjectList->list[i]);
        if(object->minX<5 && object->maxX>55) {
            uint16_t labelNum = pLabelMatrix->elements[(int)object->centerY * pLabelMatrix->width + (int)object->centerX];
            bool isLine = _labelToLine(pLabelMatrix, object, line, labelNum);
            if(isLine) {
                if(resultLine->distancePoint.y <= line->distancePoint.y) {
                    resultLine->theta = line->theta;
                    resultLine->distancePoint.x = line->distancePoint.x;
                    resultLine->distancePoint.y = line->distancePoint.y;
                    emptyLine = false;
                }
            }
        }  
    }

    if(emptyLine) {
        free(resultLine);
        resultLine = NULL;
    }

    free(line);
    //destroyMatrix8(pSubMatrix);
    destroyMatrix16(pLabelMatrix);
    
    if (pObjectList){
        free(pObjectList->list);
        free(pObjectList);
    }

    return resultLine;
}

bool _labelToLine(Matrix16_t* pObjectLineMatrix, Object_t* object, Line_t* candidate, int labelNum){
    
    PixelLocation_t centerUpPoint;
    PixelLocation_t centerDownPoint;
    PixelLocation_t leftUpPoint;
    PixelLocation_t leftDownPoint;
    PixelLocation_t rightUpPoint;
    PixelLocation_t rightDownPoint;

    PixelLocation_t* pPixel = (PixelLocation_t*)malloc(sizeof(PixelLocation_t));

    pPixel->x = (uint8_t)object->centerX;
    pPixel->y = (uint8_t)object->centerY;
    centerUpPoint = _searchToTopCenter(pObjectLineMatrix, pPixel, labelNum);
    centerDownPoint = _searchToBottomCenter(pObjectLineMatrix, pPixel, labelNum);
  
    candidate->distancePoint = centerDownPoint;

    pPixel->x = object->minX;
    pPixel->y = object->minY;
    leftUpPoint = _searchToBottom(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->minX;
    pPixel->y = object->maxY;
    leftDownPoint = _searchToTop(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->maxX;
    pPixel->y = object->minY;
    rightUpPoint = _searchToBottom(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->maxX;
    pPixel->y = object->maxY;
    rightDownPoint = _searchToTop(pObjectLineMatrix, pPixel, labelNum);

    free(pPixel);

    double angleDown1 = _getAngle(leftDownPoint, centerDownPoint);
    double angleDown2 = _getAngle(centerDownPoint, rightDownPoint);
    double angleUp1 = _getAngle(leftUpPoint, centerUpPoint);
    double angleUp2 = _getAngle(centerUpPoint, rightUpPoint);

    //printf("angelUp1 = %f\n", angleUp1);
    //printf("angelUp2 = %f\n", angleUp2);
    //printf("angelDown1 = %f\n", angleDown1);
    //printf("angelDown2 = %f\n", angleDown2);
    
    /*
    printf("centerPoint.x = %d\n", candidate->centerPoint.x);
    printf("centerPoint.y = %d\n", candidate->centerPoint.y);
    printf("minX = %d\n", object->minX);
    printf("minY = %d\n", object->minY);
    printf("maxX = %d\n", object->maxX);
    printf("maxY = %d\n", object->maxY);
   
    printf("centerUpPoint.x = %d\n", centerUpPoint.x);
    printf("centerUpPoint.y = %d\n", centerUpPoint.y);

    printf("centerDownPoint.x = %d\n", centerDownPoint.x);
    printf("centerDownPoint.y = %d\n", centerDownPoint.y);
    
    printf("leftUpPoint.x = %d\n", leftUpPoint.x);
    printf("leftUpPoint.y = %d\n", leftUpPoint.y);
    
    printf("leftDownPoint.x = %d\n", leftDownPoint.x);
    printf("leftDownPoint.y = %d\n", leftDownPoint.y);
    
    printf("rightUpPoint.x = %d\n", rightUpPoint.x);
    printf("rightUpPoint.y = %d\n", rightUpPoint.y);
    
    printf("rightDownPoint.x = %d\n", rightDownPoint.x);
    printf("rightDownPoint.y = %d\n", rightDownPoint.y);
    */

    if(fabs(angleDown1 - angleDown2) <= 10) {
        candidate->theta = (angleDown1+angleDown2)/2;
        return true;
    }
    else {
        return false;
    }
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
