#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "line_detection.h"

Line_t* _labelToLine(Matrix16_t* pObjectLineMatrix, Object_t* object, Line_t* candidate, int labelNum);
PixelLocation_t _searchToTop(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToBottom(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);


//SubMatrix와 해당 SubMatrix의 LabelList를 인자로 받아 LineDetection을 진행한다.
Line_t* lineDetection(Matrix8_t* pColorMatrix) {
    
    Matrix8_t* pSubMatrix;
    pSubMatrix = createSubMatrix8(pColorMatrix, 60, 0, 120, 120);

    Matrix16_t* pLabelMatrix = createMatrix16(pSubMatrix->width, pSubMatrix->height);   
    
    ObjectList_t* pMatrixObjectListWithLabeling;
    
    pMatrixObjectListWithLabeling = detectObjectsLocationWithLabeling(pSubMatrix, pLabelMatrix);
    
    printf("A");
    return NULL;
    
    Line_t* lineA = (Line_t*)malloc(sizeof(Line_t));
    
    /*
    int i;
    int maxList;
    Line_t* resultLine = (Line_t*)malloc(sizeof(Line_t));
    for(i = 0; i < pSubObjectList->size; ++i) {
        Object_t* object = &(pSubObjectList->list[i]);
        if(object->minX < 5 && object->maxX )


        int labelNum = i+1;
        resultLine = _labelToLine(pLabelMatrix, object, resultLine, labelNum);  
        //6개의 포인트를 통해 각도를 구하고
    }

    
    destroyMatrix16(pLabelMatrix);
    if (pMatrixObjectListWithLabeling)
        free(pMatrixObjectListWithLabeling);

    return resultLine;
    */
}

Line_t* _labelToLine(Matrix16_t* pObjectLineMatrix, Object_t* object, Line_t* candidate, int labelNum){
    PixelLocation_t minMinPoint;
    PixelLocation_t minMaxPoint;
    PixelLocation_t maxMinPoint;
    PixelLocation_t maxMaxPoint;

    minMinPoint.x = object->minX;
    minMinPoint.y = object->minY;
    
    minMaxPoint.x = object->minX;
    minMaxPoint.y = object->maxY;
    
    maxMinPoint.x = object->maxX;
    maxMinPoint.y = object->minY;

    maxMaxPoint.x = object->maxX;
    maxMaxPoint.y = object->maxY;

    candidate->centerPoint.x = (uint8_t)object->centerX;
    candidate->centerPoint.y = (uint8_t)object->centerY;

    PixelLocation_t* pPixel = (PixelLocation_t*)malloc(sizeof(PixelLocation_t));
    
    pPixel->x = candidate->centerPoint.x;
    pPixel->y = candidate->centerPoint.y;
    candidate->centerUpperPoint = _searchToTop(pObjectLineMatrix, pPixel, labelNum);
    candidate->centerLowerPoint = _searchToBottom(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->minX;
    pPixel->y = object->maxY;
    //candidate->minMaxUpperPoint = _searchToTop(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->maxX;
    pPixel->y = object->minY;
    //candidate->maxMinLowerPoint = _searchToBottom(pObjectLineMatrix, pPixel, labelNum);
    
    free(pPixel);
    return candidate;
}

//두 점을 이용한 기울기 계산
double _computeTheta(PixelLocation_t* srcPixel, PixelLocation_t* dstPixel) {
    double theta;
    return theta; 
}

double GetAngleFromXY2Degree(double dDeltaX, double dDeltaY) {
    double dAngle = atan2( dDeltaY , dDeltaX);
    
    /*
    radian -> degree
    */                      
    dAngle *= (180.0/M_PI);
    
        if( dAngle < 0.0 ) dAngle += 360.0;
    
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
        Color_t* output = &(pObjectLineMatrix->elements[pPixel->y * pObjectLineMatrix->width + pPixel->x]);
        if(*output == COLOR_BLACK) {
            cnt++;
        }
        else if(cnt > 0) {
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
        Color_t* output = &(pObjectLineMatrix->elements[pPixel->y * pObjectLineMatrix->width + pPixel->x]);
        if(*output == COLOR_BLACK) {
            cnt++;
        }
        else if(cnt > 0) {
            resultPixel.y = y;
            break;
        }
    }
    return resultPixel;
}










