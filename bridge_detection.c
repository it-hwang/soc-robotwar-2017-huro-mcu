#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bridge_detection.h"

#define PI 3.141592

bool _labelToBridge(Matrix16_t* pObjectLineMatrix, Object_t* object, Bridge_t* candidate, int labelNum);
PixelLocation_t _searchToRight(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToLeft(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToLeftCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
PixelLocation_t _searchToRightCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum);
double _getAngleV2(PixelLocation_t src, PixelLocation_t dst);


Bridge_t* bridgeDetection(Matrix8_t* pColorMatrix) {
    Matrix8_t* pSubMatrix;
    pSubMatrix = createSubMatrix8(pColorMatrix, 0, 80, 179, 119);
    
    Matrix16_t* pLabelMatrix = createMatrix16(pSubMatrix->width, pSubMatrix->height);
    memset(pLabelMatrix->elements, 0, (pSubMatrix->height * pSubMatrix->width) * sizeof(uint16_t));

    ObjectList_t* pObjectList;
    pObjectList = detectObjectsLocationWithLabeling(pSubMatrix, pLabelMatrix);

    int i;
    Bridge_t* resultBridge = (Bridge_t*)malloc(sizeof(Bridge_t));
    resultBridge->cnt = 0;

    Bridge_t* bridge = (Bridge_t*)malloc(sizeof(Bridge_t));
    bool emptyBridge = true;
    
    for(i = 0; i < pObjectList->size; i++) {
        Object_t* object = &(pObjectList->list[i]);
        if(object->maxX - object->minX >= 30) {
            uint16_t labelNum = pLabelMatrix->elements[(int)object->centerY * pLabelMatrix->width + (int)object->centerX];
            bool isBridge = _labelToBridge(pLabelMatrix, object, bridge, labelNum);
            if(isBridge) {
                if(resultBridge->cnt <= bridge->cnt) {
                    resultBridge->isRight = bridge->isRight;
                    resultBridge->theta = bridge->theta;
                    resultBridge->centerPoint.x = bridge->centerPoint.x;
                    resultBridge->centerPoint.y = bridge->centerPoint.y;
                    resultBridge->cnt = bridge->cnt;
                    emptyBridge = false;
                }
            }
        }  
    }

    if(emptyBridge) {
        free(resultBridge);
        resultBridge = NULL;
    }

    free(bridge);
    destroyMatrix8(pSubMatrix);
    destroyMatrix16(pLabelMatrix);
    
    if (pObjectList){
        free(pObjectList->list);
        free(pObjectList);
    }

    return resultBridge;
}

bool _labelToBridge(Matrix16_t* pObjectLineMatrix, Object_t* object, Bridge_t* candidate, int labelNum){
    
    PixelLocation_t centerLeftPoint;
    PixelLocation_t centerRightPoint;
    PixelLocation_t leftUpPoint;
    PixelLocation_t leftDownPoint;
    PixelLocation_t rightUpPoint;
    PixelLocation_t rightDownPoint;

    PixelLocation_t* pPixel = (PixelLocation_t*)malloc(sizeof(PixelLocation_t));

    pPixel->x = (uint8_t)object->centerX;
    pPixel->y = (uint8_t)object->centerY;
    
    candidate->centerPoint.x = (uint8_t)object->centerX;
    candidate->centerPoint.y = (uint8_t)object->centerY;
    
    centerLeftPoint = _searchToLeftCenter(pObjectLineMatrix, pPixel, labelNum);
    centerRightPoint = _searchToRightCenter(pObjectLineMatrix, pPixel, labelNum);


    pPixel->x = object->minX;
    pPixel->y = object->minY;
    leftUpPoint = _searchToRight(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->minX;
    pPixel->y = object->maxY;
    leftDownPoint = _searchToRight(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->maxX;
    pPixel->y = object->minY;
    rightUpPoint = _searchToLeft(pObjectLineMatrix, pPixel, labelNum);

    pPixel->x = object->maxX;
    pPixel->y = object->maxY;
    rightDownPoint = _searchToLeft(pObjectLineMatrix, pPixel, labelNum);

    free(pPixel);

    double angleRight1 = -(_getAngleV2(rightUpPoint, centerRightPoint));
    double angleRight2 = -(_getAngleV2(centerRightPoint, rightDownPoint));
    double angleLeft1 = _getAngleV2(leftUpPoint, centerLeftPoint);
    double angleLeft2 = _getAngleV2(centerLeftPoint, leftDownPoint);

    double dAngleRight = fabs(angleRight1 - angleRight2);
    double dAngleLeft = fabs(angleLeft1 - angleLeft2);

    double rightTheta = (angleRight1 + angleRight2)/2;
    double leftTheta = (angleLeft1 + angleLeft2)/2;


    if(dAngleRight>=10 || dAngleLeft>=10) { //둘 중 하나라도 일정하지 않다면 그건 다리가 아니다
        return false;
    }else { //둘 다 일정한 기울기를 가지는 경우
            //그 중 큰 각도를 가지는 기울기와 방향을 받는다
            //왜냐하면 작은 각도보다 큰 각도의 값의 변화가 훨씬 정확할 것이라고 판단.
            //capture결과 로봇의 방향(틀려진 경우)이 향한 쪽의 기울기가 상대적으로 크게 증가함.
            //큰 값을 보고 반대방향모션으로 조금씩 각도를 조절하여 맞춰나가면 될 것.
            //ex) isRight가 true라면 해결을 위해 왼쪽방향모션을 사용.
            //그 반대의 경우는 반대가 커보일 것이기 때문에 알아서 반대방향모션을 사용하게 함.
        if(rightTheta - leftTheta>=0) { 
            candidate->theta = rightTheta;
            candidate->cnt = object->cnt;
            candidate->isRight = true;
            return true;
        } 
        else {
            candidate->theta = leftTheta;
            candidate->cnt = object->cnt;
            candidate->isRight = false;
            return true;
        }   
    }
}

//두 점을 이용한 기울기 계산
double _getAngleV2(PixelLocation_t src, PixelLocation_t dst) {
    int dDeltaX;
    int dDeltaY;

    dDeltaX = dst.y - src.y;
    dDeltaY = dst.x - src.x;
    
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
PixelLocation_t _searchToRight(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //위를 향한 Search
    PixelLocation_t resultPixel;
    int y = pPixel->y;
    resultPixel.y = y;

    int x;
    int cnt = 0;
    for(x=pPixel->x; x<pObjectLineMatrix->width; x++) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        if((int)*output != labelNum) {
            cnt++;
        }
        else {
            resultPixel.x = x;
            break;
        }
    }
    return resultPixel;
    
    
    /*
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
    */
}

PixelLocation_t _searchToLeft(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //아래를 향한 Search
    PixelLocation_t resultPixel;
    int y = pPixel->y;
    resultPixel.y = y;
    
    int x;
    int cnt = 0;
    for(x=pPixel->x; x>=0; x--) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        if((int)*output != labelNum) {
            cnt++;
        }
        else {
            resultPixel.x = x;
            break;
        }
    }
    return resultPixel;
}

PixelLocation_t _searchToRightCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //위를 향한 Search
    PixelLocation_t resultPixel; 
    int y = pPixel->y;
    resultPixel.y = y;
    int x;
    int cnt = 0;
    for(x=pPixel->x; x<pObjectLineMatrix->width; x++) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        
        if((int)*output == labelNum) {
            cnt++;
        }
        else {
            resultPixel.x = x;
            break;
        }
    }
    return resultPixel;
}

PixelLocation_t _searchToLeftCenter(Matrix16_t* pObjectLineMatrix, PixelLocation_t* pPixel, int labelNum){    //아래를 향한 Search
    PixelLocation_t resultPixel;
    int y = pPixel->y;
    resultPixel.y = y;
    
    int x;
    int cnt = 0;
    for(x=pPixel->x; x<pObjectLineMatrix->width; x--) {
        uint16_t* output = &(pObjectLineMatrix->elements[y * pObjectLineMatrix->width + x]);
        if((int)*output == labelNum) {
            cnt++;
        }
        else {
            resultPixel.x = x;
            break;
        }
    }

    return resultPixel;
}


/*
//SubMatrix와 해당 SubMatrix의 LabelList를 인자로 받아 LineDetection을 진행한다.
Line_t* lineDetection(Matrix8_t* pColorMatrix) {
    
    Matrix8_t* pSubMatrix;
    pSubMatrix = createSubMatrix8(pColorMatrix, 60, 0, 119, 119);

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
    destroyMatrix8(pSubMatrix);
    destroyMatrix16(pLabelMatrix);
    
    if (pObjectList){
        free(pObjectList->list);
        free(pObjectList);
    }

    return resultLine;
}
*/