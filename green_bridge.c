#define DEBUG

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "green_bridge.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "white_balance.h"
#include "image_filter.h"
#include "object_detection.h"
#include "polygon_detection.h"
#include "line_detection.h"
#include "log.h"
#include "debug.h"


// 녹색 다리의 최소 면적
static const int   _MIN_AREA = 300;
// 녹색 다리의 최소 유사도
static const float _MIN_CORRELATION = 0.1;

static const int   _LINE_DETECTION_WIDTH = 80;
static const int   _LINE_DETECTION_HEIGHT = 80;


#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))

static bool _approachGreenBridge(void);
static bool _climbUpStair(void);
static bool _crossGreenBridge(void);
static bool _climbDownStair(void);
static double _calculateBridgeAngle(Matrix16_t* pLabelMatrix, Object_t* pBridgeObject);
static int _measureGreenBridgeCenterOffsetX(void);
static void _displayDebugScreen(Screen_t* pScreen, Object_t* pObject);
static bool _searchGreenBridge(Screen_t* pScreen, Object_t* pObject, Matrix16_t* pLabelMatrix);
static bool _searchBlackLine(Screen_t* pScreen, Object_t* pObject, Matrix16_t* pLabelMatrix);
static float _getGreenBridgeCorrelation(Matrix8_t* pGreenMatrix, Object_t* pObject);
static Matrix8_t* _createGreenMatrix(Screen_t* pScreen);
static Matrix8_t* _createBlackMatrix(Screen_t* pScreen);
static void _setHead(int horizontalDegrees, int verticalDegrees);
static void _testMeasurement(void);


bool greenBridgeMain(void) {
    //_testMeasurement();
    solveGreenBridge();

    return false;
}


int measureGreenBridgeDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Object_t object;
    bool hasFound = _searchGreenBridge(pScreen, &object, NULL);
    _displayDebugScreen(pScreen, &object);

    int millimeters = 0;
    if (hasFound) {
        printDebug("minX: %d, centerX: %f, maxX: %d, minY: %d, centerY: %f, maxY: %d\n", object.minX, object.centerX, object.maxX, object.minY, object.centerY, object.maxY);

        if (object.maxY < pScreen->height - 1) {
            millimeters = -411.9 * log(object.maxY) + 1971.5; 
        }
        else {
            millimeters = -26.647 * object.minY + 468.74;
        }

        // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다.
        if (millimeters <= 0)
            millimeters = 1;
    }

    destroyScreen(pScreen);

    return millimeters;
}


bool solveGreenBridge(void) {
    //_approachGreenBridge();
    //_climbUpStair();
    _crossGreenBridge();
    _climbDownStair();

    return false;
}

static bool _approachGreenBridge(void) {
    // 녹색 다리를 발견하지 못할 경우 다시 찍는 횟수
    const int MAX_TRIES = 10;

    // 장애물에 다가갈 거리 (밀리미터)
    const int APPROACH_DISTANCE = 20;
    // 거리 허용 오차 (밀리미터)
    const int APPROACH_DISTANCE_ERROR = 30;

    const int ALIGN_OFFSET_X = 4;
    const double MILLIMETERS_PER_PIXELS = 2.;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        bool hasFound;
        
        // 앞뒤 정렬
        int distance = measureGreenBridgeDistance();
        hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printDebug("전진보행으로 이동하자. (거리: %d)\n", distance);
            walkForward(distance - APPROACH_DISTANCE);
            mdelay(300);
            nTries = 0;
            continue;
        }
        
        printDebug("접근 완료.\n");
        break;
    }
    if (nTries >= MAX_TRIES) {
        printDebug("시간 초과!\n");
        return false;
    }
    
    // 달라붙어 비비기
    runWalk(ROBOT_WALK_FORWARD_QUICK, 12);
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);

    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        bool hasFound;
        
        // 앞뒤 정렬
        int distance = measureGreenBridgeDistance();
        hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printDebug("종종걸음으로 이동하자. (거리: %d)\n", distance);
            runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
            mdelay(300);
            nTries = 0;
            continue;
        }
        
        // 좌우 정렬
        int offsetX = _measureGreenBridgeCenterOffsetX();
        if (offsetX < ALIGN_OFFSET_X * -1) {
            int millimeters = abs(offsetX) * MILLIMETERS_PER_PIXELS;
            printDebug("offsetX: %d, Go left.\n", offsetX);
            walkLeft(millimeters);
            mdelay(300);
            nTries = 0;
            continue;
        }
        else if (offsetX > ALIGN_OFFSET_X) {
            int millimeters = abs(offsetX) * MILLIMETERS_PER_PIXELS;
            printDebug("offsetX: %d, Go right.\n", offsetX);
            walkRight(millimeters);
            mdelay(300);
            nTries = 0;
            continue;
        }
        
        printDebug("접근 완료.\n");
        break;
    }
    if (nTries >= MAX_TRIES) {
        printDebug("시간 초과!\n");
        return false;
    }
    
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
    
    return true;
}


static bool _climbUpStair(void) {
    return runMotion(MOTION_CLIMB_UP_STAIR);
}


static bool _crossGreenBridge(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -80;

    const double MILLIMETERS_PER_PIXELS = 1.;
    // 각도 허용 오차 (도)
    const double ALIGN_FACING_ERROR = 5.;
    // 좌우 정렬 허용 오차 (밀리미터)
    const double ALIGN_CENTER_X_ERROR = 10.;
    const double ALIGN_MAX_TURN_DEGREES = 20.;
    const double ALIGN_MAX_WALK_DISTANCE = 30.;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);

    while (true) {
        Object_t bridge;
        
        readFpgaVideoDataWithWhiteBalance(pScreen);

        bool isOnBridge = _searchGreenBridge(pScreen, &bridge, pLabelMatrix);
        _displayDebugScreen(pScreen, &bridge);
        if (!isOnBridge)
            break;

        bool isEndOfBridge = (bridge.minY > pScreen->height * 0.3);
        if (isEndOfBridge)
            break;

        int screenCenterX = pScreen->width / 2;
        double bridgeAngle = _calculateBridgeAngle(pLabelMatrix, &bridge);
        double dx = (bridge.centerX - screenCenterX) * MILLIMETERS_PER_PIXELS;
        printDebug("angle: %f, dx: %f\n", bridgeAngle, dx);

        if (fabs(bridgeAngle) > ALIGN_FACING_ERROR) {
            if (bridgeAngle < 0)
                turnLeft(_MIN(fabs(bridgeAngle), ALIGN_MAX_TURN_DEGREES));
            else
                turnRight(_MIN(fabs(bridgeAngle), ALIGN_MAX_TURN_DEGREES));
            //mdelay(20);
            continue;
        }

        if (fabs(dx) > ALIGN_CENTER_X_ERROR) {
            if (dx < 0)
                walkLeft(_MIN(fabs(dx), ALIGN_MAX_WALK_DISTANCE));
            else
                walkRight(_MIN(fabs(dx), ALIGN_MAX_WALK_DISTANCE));
            //mdelay(20);
            continue;
        }

        printDebug("walk.\n");
        walkForward(128);
        //mdelay(40);
    }
    
    destroyScreen(pScreen);
    destroyMatrix16(pLabelMatrix);

    return true;
}


static bool _climbDownStair(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -80;

    // 각도 허용 오차 (도)
    const double ALIGN_FACING_ERROR = 5.;
/*
    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);

    while (true) {
        Object_t blackLine;

        bool isOnBridge = _searchBlackLine(pScreen, &blackLine, pLabelMatrix);
        if (!isOnBridge)
            break;

        double bridgeAngle = _calculateBridgeAngle(pLabelMatrix, &bridge);
        if (fabs(bridgeAngle) > ALIGN_FACING_ERROR) {
            if (bridgeAngle < 0)
                turnLeft(fabs(bridgeAngle));
            else
                turnRight(fabs(bridgeAngle));
            
            continue;
        }

        int screenCenterX = pScreen->width / 2;
        double dx = bridge.centerX - screenCenterX;
        if (fabs(dx) > ALIGN_CENTER_X_ERROR) {
            if (dx < 0)
                walkLeft(fabs(dx));
            else
                walkRight(fabs(dx));
                
            continue;
        }

        walkForward(128);
    }
    
    destroyScreen(pScreen);
    destroyMatrix16(pLabelMatrix);
*/
    return true;
}


static double _calculateBridgeAngle(Matrix16_t* pLabelMatrix, Object_t* pBridgeObject) {
    Polygon_t* pPolygon = createPolygon(pLabelMatrix, pBridgeObject, 5);
    Line_t* pLeftLine = findLeftLine(pPolygon);
    Line_t* pRightLine = findRightLine(pPolygon);

    double leftTheta = pLeftLine->theta;
    double rightTheta = pRightLine->theta;
    if (leftTheta < 0.) leftTheta += 180.;
    if (rightTheta < 0.) rightTheta += 180.;
    double degrees = ((leftTheta + rightTheta) / 2.) - 90.;

    destroyPolygon(pPolygon);
    free(pLeftLine);
    free(pRightLine);

    return degrees;
}


static int _measureGreenBridgeCenterOffsetX(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Object_t object;
    bool hasFound = _searchGreenBridge(pScreen, &object, NULL);
    _displayDebugScreen(pScreen, &object);

    int offsetX = 0;;
    if (hasFound) {
        printDebug("minX: %d, centerX: %f, maxX: %d, minY: %d, centerY: %f, maxY: %d\n", object.minX, object.centerX, object.maxX, object.minY, object.centerY, object.maxY);

        offsetX = (int)(object.centerX) - (pScreen->width / 2);
    }

    destroyScreen(pScreen);

    return offsetX;
}


static void _displayDebugScreen(Screen_t* pScreen, Object_t* pObject) {
    Screen_t* pDebugScreen = cloneMatrix16(pScreen);
    Matrix8_t* pGreenMatrix = _createGreenMatrix(pScreen);

    drawColorMatrix(pDebugScreen, pGreenMatrix);

    drawObjectEdge(pDebugScreen, pObject, NULL);
    drawObjectCenter(pDebugScreen, pObject, NULL);

    displayScreen(pDebugScreen);

    destroyMatrix16(pDebugScreen);
    destroyMatrix8(pGreenMatrix);
}

// pScreen에서 녹색 다리를 찾는다.
static bool _searchGreenBridge(Screen_t* pScreen, Object_t* pObject, Matrix16_t* pLabelMatrix) {
    if (!pScreen) return false;

    Matrix8_t* pGreenMatrix = _createGreenMatrix(pScreen);

    ObjectList_t* pObjectList = NULL;
    if (pLabelMatrix)
        pObjectList = detectObjectsLocationWithLabeling(pGreenMatrix, pLabelMatrix);
    else
        pObjectList = detectObjectsLocation(pGreenMatrix);

    removeSmallObjects(pObjectList, _MIN_AREA);

    // 유사도가 가장 큰 오브젝트를 찾는다.
    Object_t* pMostSimilarObject = NULL;
    float maxCorrelation = 0.;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        float correlation = _getGreenBridgeCorrelation(pGreenMatrix, pObject);
        if (correlation > maxCorrelation) {
            pMostSimilarObject = pObject;
            maxCorrelation = correlation;
        }
    }

    Object_t* pGreenBridge = NULL;
    bool hasFound = false;
    if (maxCorrelation >= _MIN_CORRELATION) {
        pGreenBridge = pMostSimilarObject;
        hasFound = true;
    }
    
    if (hasFound && pObject)
        memcpy(pObject, pGreenBridge, sizeof(Object_t));

    destroyMatrix8(pGreenMatrix);
    destroyObjectList(pObjectList);

    return hasFound;
}

// 빨간 다리와의 유사한 정도를 반환한다. (범위: 0.0 ~ 1.0)
//  # 크기가 클 수록 유사도가 높다.
//  # 무게중심이 약간 하단에 존재할 수록 유사도가 높다. (사다리꼴) 
static float _getGreenBridgeCorrelation(Matrix8_t* pGreenMatrix, Object_t* pObject) {
    const float AREA_CORRELATION_RATIO = 0.90;
    const float CENTER_CORRELATION_RATIO = 0.10;

    const float CENTER_X_RATIO = 0.50;
    const float CENTER_Y_RATIO = 0.58;

    if (!pGreenMatrix) return 0.;
    if (!pObject) return 0.;

    float areaCorrelation;
    int width = pGreenMatrix->width;
    int height = pGreenMatrix->height;
    int maxArea = width * height;
    int objectArea = pObject->cnt;
    areaCorrelation = (float)objectArea / maxArea;

    float centerXCorrelation;
    float objectCenterX = pObject->centerX;
    float idealCenterX = ((float)pObject->maxX * CENTER_X_RATIO) + ((float)pObject->minX * (1. - CENTER_X_RATIO));
    int   objectWidth = pObject->maxX - pObject->minX + 1;
    centerXCorrelation = 1.0 - (fabs(objectCenterX - idealCenterX) / objectWidth);

    float centerYCorrelation;
    float objectCenterY = pObject->centerY;
    float idealCenterY = ((float)pObject->maxY * CENTER_Y_RATIO) + ((float)pObject->minY * (1. - CENTER_Y_RATIO));
    int   objectHeight = pObject->maxY - pObject->minY + 1;
    centerYCorrelation = 1.0 - (fabs(objectCenterY - idealCenterY) / objectHeight);

    float centerCorrelation = (centerXCorrelation * 0.5) + (centerYCorrelation * 0.5);

    return (areaCorrelation * AREA_CORRELATION_RATIO) + (centerCorrelation * CENTER_CORRELATION_RATIO);
}

/*
static bool _searchBlackLine(Screen_t* pScreen, Object_t* pObject, Matrix16_t* pLabelMatrix) {
    if (!pScreen) return false;

    Matrix8_t* pBlackMatrix = _createBlackMatrix(pScreen);

    ObjectList_t* pObjectList = NULL;
    if (pLabelMatrix)
        pObjectList = detectObjectsLocationWithLabeling(pGreenMatrix, pLabelMatrix);
    else
        pObjectList = detectObjectsLocation(pGreenMatrix);

    // 가장 가까운 수평선을 찾는다.
    Object_t* pMostNearestObject = NULL;
    float maxCorrelation = 0.;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        float correlation = _getGreenBridgeCorrelation(pGreenMatrix, pObject);
        if (correlation > maxCorrelation) {
            pMostSimilarObject = pObject;
            maxCorrelation = correlation;
        }
    }

    Object_t* pGreenBridge = NULL;
    bool hasFound = false;
    if (maxCorrelation >= _MIN_CORRELATION) {
        pGreenBridge = pMostSimilarObject;
        hasFound = true;
    }
    
    if (hasFound && pObject)
        memcpy(pObject, pGreenBridge, sizeof(Object_t));

    destroyMatrix8(pGreenMatrix);
    destroyObjectList(pObjectList);

    return hasFound;
}
*/

static Matrix8_t* _createGreenMatrix(Screen_t* pScreen) {
    Matrix8_t* pMatrix = createColorMatrix(pScreen, pColorTables[COLOR_GREEN]);

    // 잡음을 제거한다.
    applyFastErosionToMatrix8(pMatrix, 2);
    applyFastDilationToMatrix8(pMatrix, 2);

    return pMatrix;
}

static Matrix8_t* _createBlackMatrix(Screen_t* pScreen) {
    Matrix8_t* pMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLACK]);

    // 잡음을 제거한다.
    applyFastErosionToMatrix8(pMatrix, 2);
    applyFastDilationToMatrix8(pMatrix, 4);
    applyFastErosionToMatrix8(pMatrix, 2);

    int width = pScreen->width;
    int height = pScreen->height;
    int centerX = width / 2;
    int minX = centerX - _LINE_DETECTION_WIDTH;
    int maxX = minX + _LINE_DETECTION_WIDTH - 1;

    return pMatrix;
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

static void _testMeasurement(void) {
    for (int i = 0; i < 100; ++i) {
        measureGreenBridgeDistance();

        char input;
        input = getchar();
        while (input != '\n')
            input = getchar();
    }
}
