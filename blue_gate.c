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


static Object_t* _searchBlueGate(Screen_t* pScreen);
static Matrix8_t* _createBlueMatrix(Screen_t* pScreen);
static void _setHeadRight(void);
static void _setHeadLeft(void);
static void _setHeadForward(void);
static void _setStandardStand(void);

bool blueGateMain(void) {
    for (int i = 0; i < 100; ++i) {
        int millimeters = measureRightBlueGateDistance();

        char input;
        input = getchar();
        while (input != '\n')
            input = getchar();
    }
    return true;
}

static bool _approchBlueGate(void) {
    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 150;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 50;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {    
        // 앞뒤 정렬
        int distance = measureRightBlueGateDistance();
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

    int millimeters = 0;

    Object_t* pObject = _searchBlueGate(pScreen);
    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    if (pObject != NULL) {
        printDebug("minX: %d, centerX: %f, maxX: %d\n",
                 pObject->minX, pObject->centerX, pObject->maxX);
        printDebug("minY: %d, centerY: %f, maxY: %d\n",
                 pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pObject->minY;
        
        millimeters = 655.21 * exp(-0.016 * distance);
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
    
    int millimeters = 0;

    Object_t* pObject = _searchBlueGate(pScreen);
    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    if (pObject != NULL) {
        printDebug("minX: %d, centerX: %f, maxX: %d\n",
                 pObject->minX, pObject->centerX, pObject->maxX);
        printDebug("minY: %d, centerY: %f, maxY: %d\n",
                 pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pObject->minY;
        
        millimeters = 655.21 * exp(-0.016 * distance);
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
