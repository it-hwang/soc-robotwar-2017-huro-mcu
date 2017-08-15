#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "red_bridge.h"
#include "matrix.h"
#include "color.h"
#include "color_model.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "white_balance.h"
#include "log.h"


// 빨간 다리의 최소 면적
static const int   _MIN_AREA        = 600;
// 빨간 다리의 최소 유사도
static const float _MIN_CORRELATION = 0.1;


static Matrix8_t* _createRedMatrix(Screen_t* pScreen) {
    Matrix8_t* pRedMatrix = createColorMatrix(pScreen, pColorTables[COLOR_RED]);

    // 잡음을 제거한다.
    applyFastErosionToMatrix8(pRedMatrix, 4);
    applyFastDilationToMatrix8(pRedMatrix, 8);
    applyFastErosionToMatrix8(pRedMatrix, 4);

    return pRedMatrix;
}

static void _displayDebugScreen(Screen_t* pScreen, Object_t* pObject) {
    Screen_t* pDebugScreen = cloneMatrix16(pScreen);
    Matrix8_t* pRedMatrix = _createRedMatrix(pScreen);

    drawColorMatrix(pDebugScreen, pRedMatrix);

    drawObjectEdge(pDebugScreen, pObject, NULL);
    drawObjectCenter(pDebugScreen, pObject, NULL);

    displayScreen(pDebugScreen);

    destroyMatrix16(pDebugScreen);
    destroyMatrix8(pRedMatrix);
}

// 빨간 다리와의 유사한 정도를 반환한다. (범위: 0.0 ~ 1.0)
//  # 크기가 클 수록 유사도가 높다.
//  # 무게중심이 약간 하단에 존재할 수록 유사도가 높다. (사다리꼴) 
static float _getRedBridgeCorrelation(Matrix8_t* pRedMatrix, Object_t* pRedObject) {
    static const float AREA_CORRELATION_RATIO = 0.90;
    static const float CENTER_CORRELATION_RATIO = 0.10;

    static const float CENTER_X_RATIO = 0.50;
    static const float CENTER_Y_RATIO = 0.70;

    if (pRedMatrix == NULL)
        return 0.;
    if (pRedObject == NULL)
        return 0.;

    float areaCorrelation;
    int width = pRedMatrix->width;
    int height = pRedMatrix->height;
    int maxArea = width * height;
    int objectArea = pRedObject->cnt;
    areaCorrelation = (float)objectArea / maxArea;

    float centerXCorrelation;
    float objectCenterX = pRedObject->centerX;
    float idealCenterX = ((float)pRedObject->maxX * CENTER_X_RATIO) + ((float)pRedObject->minX * (1. - CENTER_X_RATIO));
    int   objectWidth = pRedObject->maxX - pRedObject->minX + 1;
    centerXCorrelation = 1.0 - (fabs(objectCenterX - idealCenterX) / objectWidth);
    
    float centerYCorrelation;
    float objectCenterY = pRedObject->centerY;
    float idealCenterY = ((float)pRedObject->maxY * CENTER_Y_RATIO) + ((float)pRedObject->minY * (1. - CENTER_Y_RATIO));
    int   objectHeight = pRedObject->maxY - pRedObject->minY + 1;
    centerYCorrelation = 1.0 - (fabs(objectCenterY - idealCenterY) / objectHeight);
    
    float centerCorrelation = (centerXCorrelation * 0.5) + (centerYCorrelation * 0.5);

    return (areaCorrelation * AREA_CORRELATION_RATIO) + (centerCorrelation * CENTER_CORRELATION_RATIO);
}

// pScreen에서 빨간 다리를 찾는다.
// 반환된 오브젝트는 free()를 통하여 해제시켜야한다.
static Object_t* _searchRedBridge(Screen_t* pScreen) {
    Matrix8_t* pRedMatrix = _createRedMatrix(pScreen);

    ObjectList_t* pObjectList = detectObjectsLocation(pRedMatrix);
    removeSmallObjects(pObjectList, _MIN_AREA);

    // 유사도가 가장 큰 오브젝트를 찾는다.
    Object_t* pMostSimilarObject = NULL;
    float maxCorrelation = 0.;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        float correlation = _getRedBridgeCorrelation(pRedMatrix, pObject);
        if (correlation > maxCorrelation) {
            pMostSimilarObject = pObject;
            maxCorrelation = correlation;
        }
    }

    Object_t* pRedBridge = NULL;
    if (maxCorrelation >= _MIN_CORRELATION) {
        pRedBridge = (Object_t*)malloc(sizeof(Object_t));
        memcpy(pRedBridge, pMostSimilarObject, sizeof(Object_t));
    }

    destroyMatrix8(pRedMatrix);
    destroyObjectList(pObjectList);

    return pRedBridge;
}

static void _setHead(int horizontalDegrees, int verticalDegrees) {
    static const int ERROR_RANGE = 3;

    bool isAlreadySet = true;
    if (abs(getHeadHorizontal() - horizontalDegrees) > ERROR_RANGE)
        isAlreadySet = false;
    if (abs(getHeadVertical() - verticalDegrees) > ERROR_RANGE)
        isAlreadySet = false;

    if (isAlreadySet)
        return;

    setServoSpeed(30);
    runMotion(MOTION_HEAD_FRONT);
    setHead(horizontalDegrees, verticalDegrees);
    resetServoSpeed();
    mdelay(500);
}

int measureRedBridgeDistance(void) {
    static const char* LOG_FUNCTION_NAME = "measureRedBridgeDistance()";

    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    int millimeters = 0;

    Object_t* pObject = _searchRedBridge(pScreen);
    if (pObject != NULL) {
        printLog("[%s] minY: %d, centerY: %f, maxY: %d\n", LOG_FUNCTION_NAME,
                 pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        
    }

    _displayDebugScreen(pScreen, pObject);

    free(pObject);
    destroyScreen(pScreen);

    return millimeters;
}


bool redBridgeMain(void) {
    for (int i = 0; i < 100; ++i) {
        measureRedBridgeDistance();
    }

    return true;
}


bool solveRedBridge(void) {
    return true;
}

