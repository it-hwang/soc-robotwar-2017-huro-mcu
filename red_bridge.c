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
    setHead(horizontalDegrees, verticalDegrees);
    resetServoSpeed();
    mdelay(800);
}

// 다리가 가까이 있는 경우에 사용하는 측정 함수
static int _measureRedBridgeDistance2(void) {
    static const char* LOG_FUNCTION_NAME = "_measureRedBridgeDistance2()";

    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -70;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    int millimeters = 0;

    Object_t* pObject = _searchRedBridge(pScreen);
    _displayDebugScreen(pScreen, pObject);
    if (pObject != NULL) {
        printLog("[%s] minX: %d, centerX: %f, maxX: %d\n", LOG_FUNCTION_NAME,
                 pObject->minX, pObject->centerX, pObject->maxX);
        printLog("[%s] minY: %d, centerY: %f, maxY: %d\n", LOG_FUNCTION_NAME,
                 pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int minY = pObject->minY;
        int maxY = pObject->maxY;

        if (minY > 1) {
            millimeters = 0;
        }
        else if (maxY >= pScreen->height - 1) {
            millimeters = 0;
        }
        else {
            millimeters = -2.5667 * maxY + 279.27;
            // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
            if (millimeters <= 0)
                millimeters = 1;    
        }
    }

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    printLog("[%s] millimeters: %d\n", LOG_FUNCTION_NAME, millimeters);
    return millimeters;
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
    _displayDebugScreen(pScreen, pObject);
    if (pObject != NULL) {
        printLog("[%s] minX: %d, centerX: %f, maxX: %d\n", LOG_FUNCTION_NAME,
                 pObject->minX, pObject->centerX, pObject->maxX);
        printLog("[%s] minY: %d, centerY: %f, maxY: %d\n", LOG_FUNCTION_NAME,
                 pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int maxY = pObject->maxY;

        if (maxY < pScreen->height - 1) {
            millimeters = -396.1 * log(maxY) + 2063;
            // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
            if (millimeters <= 0)
                millimeters = 1;
        }
        else {
            millimeters = _measureRedBridgeDistance2();
        }
    }

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    printLog("[%s] millimeters: %d\n", LOG_FUNCTION_NAME, millimeters);
    return millimeters;
}

static bool _measureRedBridgeOffsetX(int* pOffsetX) {
    static const char* LOG_FUNCTION_NAME = "_measureRedBridgeOffsetX()";

    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Object_t* pObject = _searchRedBridge(pScreen);
    _displayDebugScreen(pScreen, pObject);
    bool hasFound = (pObject != NULL);

    int offsetX = 0;
    if (pObject != NULL) {
        printLog("[%s] minX: %d, centerX: %f, maxX: %d\n", LOG_FUNCTION_NAME,
                 pObject->minX, pObject->centerX, pObject->maxX);
        printLog("[%s] minY: %d, centerY: %f, maxY: %d\n", LOG_FUNCTION_NAME,
                 pObject->minY, pObject->centerY, pObject->maxY);

        float screenCenterX = (float)(pScreen->width - 1) / 2;
        float objectCenterX = pObject->centerX;
        offsetX = objectCenterX - screenCenterX;
    }

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);
    
    if (pOffsetX != NULL)
        *pOffsetX = offsetX;

    printLog("[%s] offsetX: %d\n", LOG_FUNCTION_NAME, *pOffsetX);
    return hasFound;
}


// 빨간 다리 위에서 빨간다리를 찾는다.
static Object_t* _searchRedBridge2(Screen_t* pScreen) {
    Matrix8_t* pRedMatrix = _createRedMatrix(pScreen);

    ObjectList_t* pObjectList = detectObjectsLocation(pRedMatrix);

    // 유사도가 가장 큰 오브젝트를 찾는다.
    Object_t* pMostSimilarObject = NULL;
    float maxCorrelation = 0.;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        if (pObject->maxY < pScreen->height * 0.67)
            continue;

        float correlation = _getRedBridgeCorrelation(pRedMatrix, pObject);
        if (correlation > maxCorrelation) {
            pMostSimilarObject = pObject;
            maxCorrelation = correlation;
        }
    }

    Object_t* pRedBridge = NULL;
    if (pMostSimilarObject != NULL) {
        pRedBridge = (Object_t*)malloc(sizeof(Object_t));
        memcpy(pRedBridge, pMostSimilarObject, sizeof(Object_t));
    }

    destroyMatrix8(pRedMatrix);
    destroyObjectList(pObjectList);

    return pRedBridge;
}

// 빨간 다리 위에서 빨간 다리가 끝나는 지점의 거리를 구한다.
static int _measureRedBridgeEndDistance(void) {
    static const char* LOG_FUNCTION_NAME = "_measureRedBridgeEndDistance()";
    
    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -70;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);
    
    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);
    
    Object_t* pObject = _searchRedBridge2(pScreen);
    _displayDebugScreen(pScreen, pObject);

    int millimeters = 0;
    if (pObject != NULL) {
        printLog("[%s] minX: %d, centerX: %f, maxX: %d\n", LOG_FUNCTION_NAME,
                 pObject->minX, pObject->centerX, pObject->maxX);
        printLog("[%s] minY: %d, centerY: %f, maxY: %d\n", LOG_FUNCTION_NAME,
                 pObject->minY, pObject->centerY, pObject->maxY);

        int minY = pObject->minY;
        millimeters = -3.0973 * minY + 309.33;
    }
    
    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    return millimeters;
}


bool redBridgeMain(void) {
    // for (int i = 0; i < 100; ++i) {
    //     int millimeters = _measureRedBridgeEndDistance();

    //     char input;
    //     input = getchar();
    //     while (input != '\n')
    //         input = getchar();
    // }
    // return true;

    bool hasFound = (measureRedBridgeDistance() > 0);
    if (!hasFound)
        return false;

    return solveRedBridge();
}


static bool _approachRedBridge(void) {
    static const char* LOG_FUNCTION_NAME = "_approachRedBridge()";

    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;

    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 10;
    // 장애물에 전진보행으로 다가갈 거리 (밀리미터)
    static const int APPROACH_WALK_DISTANCE = 300;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_WALK_DISTANCE_ERROR = 50;
    // 종종걸음의 보폭 (밀리미터)
    static const int WALK_FORWARD_QUICK_DISTANCE_PER_STEP = 10;
    // 좌측 이동 보폭 (픽셀)
    static const int MOVE_LEFT_DISTANCE_PER_STEP = 4;
    // 우측 이동 보폭 (픽셀)
    static const int MOVE_RIGHT_DISTANCE_PER_STEP = 4;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        bool hasFound;
        
        // 좌우 정렬
        int offsetX = 0;
        hasFound = _measureRedBridgeOffsetX(&offsetX);
        if (!hasFound)
            continue;
        
        if (offsetX < MOVE_LEFT_DISTANCE_PER_STEP * -1) {
            int nSteps = abs(offsetX) / MOVE_LEFT_DISTANCE_PER_STEP;
            printLog("[%s] 왼쪽으로 %d 걸음 가자.\n", LOG_FUNCTION_NAME, nSteps);
            for (int i = 0; i < nSteps; ++i)
                runMotion(MOTION_MOVE_LEFT_MIDDLE);
            mdelay(500);
            nTries = 0;
            continue;
        }
        else if (offsetX > MOVE_RIGHT_DISTANCE_PER_STEP) {
            int nSteps = abs(offsetX) / MOVE_RIGHT_DISTANCE_PER_STEP;
            printLog("[%s] 오른쪽으로 %d 걸음 가자.\n", LOG_FUNCTION_NAME, nSteps);
            for (int i = 0; i < nSteps; ++i)
                runMotion(MOTION_MOVE_RIGHT_MIDDLE);
            mdelay(500);
            nTries = 0;
            continue;
        }
        
        // 앞뒤 정렬
        int distance = measureRedBridgeDistance();
        hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance <= APPROACH_DISTANCE) {
            printLog("[%s] 접근 완료.\n", LOG_FUNCTION_NAME);
            break;
        }
        else if (distance > APPROACH_WALK_DISTANCE + APPROACH_WALK_DISTANCE_ERROR) {
            printLog("[%s] 전진보행으로 이동하자. (거리: %d)\n", LOG_FUNCTION_NAME, distance);
            walkForward(distance - APPROACH_DISTANCE);
            mdelay(500);
            nTries = 0;
        }
        else {
            printLog("[%s] 종종걸음으로 이동하자. (거리: %d)\n", LOG_FUNCTION_NAME, distance);
            int nSteps = distance / WALK_FORWARD_QUICK_DISTANCE_PER_STEP;
            runWalk(ROBOT_WALK_FORWARD_QUICK, nSteps);
            mdelay(500);
            nTries = 0;
        }
    }
    if (nTries >= MAX_TRIES) {
        printLog("[%s] 시간 초과!\n", LOG_FUNCTION_NAME);
        return false;
    }

    // 달라붙어 비비기
    runWalk(ROBOT_WALK_FORWARD_QUICK, 4);
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
    
    return true;
}

static bool _climbUp(void) {
    runMotion(MOTION_CLIMB_UP_RED_BRIDGE);
    return true;
}

static bool _alignToCenter(void) {
    // TODO: 로봇을 중앙 정렬 시킨다.
    //       일단은 check_center 알고리즘을 사용하지만,
    //       시간을 단축하기 위해 더 빠른 방법을 고안해야한다.

    return true;
}

static bool _climbDown(void) {
    static const char* LOG_FUNCTION_NAME = "_climbDown()";

    // 다가간 장애물과의 최소 거리 (밀리미터)
    static const int MIN_APPROACH_DISTANCE = 10;
    // 다가간 장애물과의 최대 거리 (밀리미터)
    static const int MAX_APPROACH_DISTANCE = 40;

    while (true) {
        int distance = _measureRedBridgeEndDistance();
        if (distance <= MAX_APPROACH_DISTANCE)
            break;
        
        printLog("[%s] 전진보행으로 이동하자. (거리: %d)\n", LOG_FUNCTION_NAME, distance);
        walkForward(distance - MIN_APPROACH_DISTANCE);
    }

    runMotion(MOTION_CLIMB_DOWN_RED_BRIDGE);
    return true;
}

bool solveRedBridge(void) {
    _approachRedBridge();
    _climbUp();
    _alignToCenter();
    _climbDown();

    return true;
}

