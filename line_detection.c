#include <stdlib.h>
#include <string.h>

#include "line_detection.h"

Line_t* lineDetection(Matrix8_t* pObjectLineMatrix , ObjectList_t* pSubObjectList) {
    int i;
    Line_t* result;
    for(i = 0; i < pSubObjectList->size; ++i) {
        int x;
        int y;
        Object_t object = pSubObjectList->list[i];
        PixelLocation_t* CenterPoint;
        CenterPoint->x = object->centerX;
        CenterPoint->y = object->centery;
        PixelLocation_t* CenterUpperPoint;
        PixelLocation_t* CenterBottomPoint;

        CenterUpperPoint->x CenterPoint->x;
        CenterUpperPoint->y CenterPoint->y;
        CenterBottomPoint->x CenterPoint->x;
        CenterBottomPoint->y CenterPoint->y;

        while(1) {  //함수로 따로 구현 필요!
            Color_t* output = &(pObjectLineMatrix->elements[CenterUpperPoint->y++ *  pColorMatrix->width + CenterUpperPoint->x++]);
            if(output == 0) {
                --CenterUpperPoint->x;
                --CenterUpperPoint->y;
            }
        }      
    }
    
    free(pSubObjectList);
    return result;
}



/*****************************************
주어진 point를 인자로 받아 Search를 진행한다.
*****************************************/
PixelLocation_t* _searchToTop(Matrix8_t* pObjectLineMatrix, PixelLocation_t* pPixel){    //위를 향한 Search
    PixelLocation_t* pResultPixel = (PixelLocation_t*)malloc(sizeof(PixelLocation_t*));
    int x = pPixel->x;
    pResultPixel->x = x;
    
    int y;
    int cnt = 0;
    for(y=pPixel->y; y>=0; --y) {
        Color_t* output = &(pObjectLineMatrix->elements[pPixel->y * pObjectLineMatrix->width + pPixel->x]);
        if(*output == COLOR_BLACK) {
            cnt++;
        }
        else if(cnt > 0) {
            pResultPixel->y = y;
            break;
        }
    }
    return pResultPixel;
}

PixelLocation_t* _searchToBottom(Matrix8_t* pObjectLineMatrix, PixelLocation_t* pPixel){    //아래를 향한 Search
    PixelLocation_t* pResultPixel = (PixelLocation_t*)malloc(sizeof(PixelLocation_t*));
    int x = pPixel->x;
    pResultPixel->x = x;
    
    int y;
    int cnt = 0;
    for(y=pPixel->y; y<= pObjectLineMatrix->height; ++y) {
        Color_t* output = &(pObjectLineMatrix->elements[pPixel->y * pObjectLineMatrix->width + pPixel->x]);
        if(*output == COLOR_BLACK) {
            cnt++;
        }
        else if(cnt > 0) {
            pResultPixel->y = y;
            break;
        }
    }
    return pResultPixel;
}

//두 점을 이용한 기울기 계산
double* _computeThetaNeg(PixelLocation_t* srcPixel, PixelLocation_t* dstPixel) {
    src
}

double GetAngleFromXY2Degree(double dDeltaX, double dDeltaY )
{
    double dAngle = atan2( dDeltaY , dDeltaX);
    
    /*
    radian -> degree
    */                      
    dAngle *= (180.0/M_PI);
    
        if( dAngle < 0.0 ) dAngle += 360.0;
    
    return dAngle;
}
[출처] atan(dy/dx);, atan2( double dy, double dx); , math.h 의 상수 매크로, 프로그램시 계산오류등 정리|작성자 DongBangBoolPae






