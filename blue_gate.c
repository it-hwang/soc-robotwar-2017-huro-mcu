#define DEBUG

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "matrix.h"
#include "color.h"
#include "color_model.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "white_balance.h"
#include "log.h"
#include "debug.h"

#define LIMIT_TRY_COUNT 2

static bool _approchBlueGate(void);
static void _establishBoundary(Screen_t* pScreen);
static Object_t* _searchBlueGate(Screen_t* pScreen);
static Matrix8_t* _createBlueMatrix(Screen_t* pScreen);
static void _setHeadRight(void);
static void _setHeadLeft(void);
static void _setHeadForward(void);
static void _setStandardStand(void);
static bool _solveBluegate(void);
static bool _balanceToSolveBlueGate(void);
static bool _arrangeAngleBalance(void);
static bool _arrangeDistanceBalance(void);
static Object_t* _captureBlueGate(void);
static void _passThroughBlueGate(void);

bool blueGateMain(void) {
   /*  for (int i = 0; i < 100; ++i) {
        //int millimeters = measureRightBlueGateDistance();
        int millimeters = measureLeftBlueGateDistance();

        char input;
        input = getchar();
        while (input != '\n')
            input = getchar();
    }
    return true; */

    if( !_approchBlueGate() ) {
        return false;
    }

    return _solveBluegate();
}

static bool _approchBlueGate(void) {
    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 100;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 50;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {    
        // 앞뒤 정렬
        int rightDistance = measureRightBlueGateDistance();
        int leftDistance = measureLeftBlueGateDistance();

        int distance = (rightDistance + leftDistance) / 2;

        bool hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance <= APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printDebug("접근 완료.\n");
            break;
        }
        else {
            printDebug("전진보행으로 이동하자. (거리: %d)\n", distance);
            walkForward(distance - APPROACH_DISTANCE);
            mdelay(500);
            nTries = 0;
        }
    }

    if (nTries >= MAX_TRIES) {
        printDebug("시간 초과!\n");
        return false;
    }
    
    return true;
}

int measureRightBlueGateDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    //static const int HEAD_HORIZONTAL_DEGREES = 0;
    //static const int HEAD_VERTICAL_DEGREES = -50;

    _setHeadRight();

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    _establishBoundary(pScreen);

    int millimeters = 0;

    Object_t* pObject = _searchBlueGate(pScreen);
    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    if (pObject != NULL) {
        printDebug("minX: %d, centerX: %f, maxX: %d minY: %d, centerY: %f, maxY: %d\n",
        pObject->minX, pObject->centerX, pObject->maxX, pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pObject->maxY;
        
        millimeters = 0.0357*distance*distance - 11.558*distance + 849.66;
        // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
        if (millimeters <= 0)
            millimeters = 1;
    }

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    //printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}

int measureLeftBlueGateDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    //static const int HEAD_HORIZONTAL_DEGREES = 0;
    //static const int HEAD_VERTICAL_DEGREES = -50;

    _setHeadLeft();

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);
    
    _establishBoundary(pScreen);

    int millimeters = 0;

    Object_t* pObject = _searchBlueGate(pScreen);
    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    if (pObject != NULL) {
        printDebug("minX: %d, centerX: %f, maxX: %d minY: %d, centerY: %f, maxY: %d\n",
                 pObject->minX, pObject->centerX, pObject->maxX, pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pObject->maxY;
        
        millimeters = 0.0502*distance*distance - 12.867*distance + 842.13;
        // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
        if (millimeters <= 0)
            millimeters = 1;
    }

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    //printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}

static void _establishBoundary(Screen_t* pScreen) {
    
    Matrix8_t* pBlueMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);
    Matrix8_t* pWhiteMatix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);

    Matrix8_t* pMergedColorMatrix = 
    overlapColorMatrix(pBlueColorMatrix, pWhiteColorMatrix);

    applyFastErosionToMatrix8(pMergedColorMatrix, 1);
    applyFastDilationToMatrix8(pMergedColorMatrix, 1);

    Matrix8_t* pBoundaryMatrix = establishBoundary(pMergedColorMatrix);

    applyBoundary(pScreen, pBoundaryMatrix);

    destroyMatrix8(pWhiteColorMatrix);
    destroyMatrix8(pBlueColorMatrix);
    destroyMatrix8(pMergedColorMatrix);
    destroyMatrix8(pBoundaryMatrix);
}

static Object_t* _searchBlueGate(Screen_t* pScreen) {

    Matrix8_t* pBlueMatrix = _createBlueMatrix(pScreen);

    ObjectList_t* pObjectList = detectObjectsLocation(pBlueMatrix);

    if(pObjectList == NULL) {
        destroyMatrix8(pBlueMatrix);
        return NULL;
    }

    Object_t* pLargestObject = NULL;
    for(int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        if(pLargestObject == NULL)
            pLargestObject = pObject;
        else if(pObject->cnt >= pLargestObject->cnt){
            pLargestObject = pObject;
        }
    }

    Object_t* pBlueGate = NULL;
    if(pLargestObject != NULL) {
        pBlueGate = (Object_t*)malloc(sizeof(Object_t));
        memcpy(pBlueGate, pLargestObject, sizeof(Object_t));
    }

    destroyMatrix8(pBlueMatrix);
    destroyObjectList(pObjectList);

    return pBlueGate;
}

static Matrix8_t* _createBlueMatrix(Screen_t* pScreen) {

    Matrix8_t* pBlueMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);

     // 잡음을 제거한다.
    applyFastErosionToMatrix8(pBlueMatrix, 1);
    applyFastDilationToMatrix8(pBlueMatrix, 2);
    applyFastErosionToMatrix8(pBlueMatrix, 1);

    return pBlueMatrix;
}

static void _setHeadRight(void) {
    setServoSpeed(30);
    //runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(35, -40);
    mdelay(1000);
    resetServoSpeed();
}

static void _setHeadLeft(void) {
    setServoSpeed(30);
    //runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(-35, -40);
    mdelay(1000);
    resetServoSpeed();
}

static void _setHeadForward(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    setHead(0, -40);
    mdelay(1000);
    resetServoSpeed();
}

static void _setStandardStand(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    setHead(0, 0);
    mdelay(1000);
    resetServoSpeed();
}


static bool _solveBluegate(void) {

    if( _balanceToSolveBlueGate() ) {
        _passThroughBlueGate();
    }

    return true;
}

static bool _balanceToSolveBlueGate(void) {

    if( !_arrangeAngleBalance() ) {
        return false;
    }

    if( !_arrangeDistanceBalance() ) {
        return false;
    }

    _setStandardStand();

    return true;
}

static bool _arrangeAngleBalance(void) {
    
    int tryCount = 0;

    while( tryCount < LIMIT_TRY_COUNT ) {

        _setHeadRight();
        Object_t* rightBlueGate = _captureBlueGate();

        _setHeadLeft();
        Object_t* leftBlueGate = _captureBlueGate();

        if(rightBlueGate == NULL || leftBlueGate == NULL) {
            tryCount++;
            continue;
        }

        tryCount = 0;

        int differnceY = rightBlueGate->maxY - leftBlueGate->maxY;

        if( abs(differnceY) < 5 )
            break;

        if(differnceY < 0)
            turnLeft(20);
        else
            turnRight(20);
    }
}

static bool _arrangeDistanceBalance(void) {
    
    int tryCount = 0;

    while( tryCount < LIMIT_TRY_COUNT ) {

        _setHeadRight();
        Object_t* rightBlueGate = _captureBlueGate();

        _setHeadLeft();
        Object_t* leftBlueGate = _captureBlueGate();

        if(rightBlueGate == NULL || leftBlueGate == NULL) {
            tryCount++;
            continue;
        }

        tryCount = 0;

        int rightDistance = rightBlueGate->minX;
        int leftDistance = 179 - leftBlueGate->maxX;
        int differnceDistance = rightDistance - leftDistance;

        if( abs(differnceDistance) < 5 )
            break;

        if(differnceDistance < 0)
            walkLeft(5);
        else
            walkRight(5);
    }
}

static Object_t* _captureBlueGate(void) {

    Screen_t* pScreen = createDefaultScreen();

    readFpgaVideoDataWithWhiteBalance(pScreen);

    _establishBoundary(pScreen);

    Object_t* pObject = _searchBlueGate(pScreen);

    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    destroyScreen(pScreen);

    return pObject;
}

static void _passThroughBlueGate(void) {
    
    _setStandardStand();

    walkForward(100);
}
