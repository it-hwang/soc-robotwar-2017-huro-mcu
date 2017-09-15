// #define DEBUG

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
#include "check_center.h"
#include "camera.h"
#include "math.h"
#include "log.h"
#include "debug.h"


// 녹색 다리의 최소 면적
static const int   _MIN_AREA = 300;
// 녹색 다리의 최소 유사도
static const float _MIN_CORRELATION = 0.1;

static const int   _LINE_DETECTION_WIDTH = 80;
static const int   _LINE_DETECTION_HEIGHT = 86;


#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))

static bool _approachUpStair(void);
static bool _climbUpStair(void);
static bool _crossGreenBridge(void);
static bool _approachDownStair(void);
static bool _climbDownStair(void);
static double _calculateBridgeAngle(Matrix16_t* pLabelMatrix, Object_t* pBridgeObject);
static double _calculateStairAngle(Matrix16_t* pLabelMatrix, Object_t* pStairObject);
static int _measureGreenBridgeCenterOffsetX(void);
static bool _searchGreenBridge(Screen_t* pScreen, Object_t* pOutputObject, Matrix16_t* pLabelMatrix);
static bool _searchBlackLine(Screen_t* pScreen, Object_t* pReturnedObject, Matrix16_t* pLabelMatrix);
static float _getGreenBridgeCorrelation(Matrix8_t* pGreenMatrix, Object_t* pObject);
static Matrix8_t* _createGreenMatrix(Screen_t* pScreen);
static Matrix8_t* _createBlackMatrix(Screen_t* pScreen);
static void _setHead(int horizontalDegrees, int verticalDegrees);
static void _testMeasurement(void);


bool greenBridgeMain(void) {
    //_testMeasurement();
    solveGreenBridge();

    return true;
}


int measureGreenBridgeDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);
    _setHead(0, 0);
    

    Object_t object;
    bool hasFound = _searchGreenBridge(pScreen, &object, NULL);

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
    _approachUpStair();
    _climbUpStair();
    _crossGreenBridge();
    _approachDownStair();
    _climbDownStair();

    return true;
}

static bool _approachUpStair(void) {
    // 녹색 다리를 발견하지 못할 경우 다시 찍는 횟수
    const int MAX_TRIES = 10;

    // 장애물에 다가갈 거리 (밀리미터)
    const int APPROACH_DISTANCE = 20;
    // 거리 허용 오차 (밀리미터)
    const int APPROACH_DISTANCE_ERROR = 30;

    const int ALIGN_OFFSET_X = 8;
    const double MILLIMETERS_PER_PIXELS = 2.5;

    const int APPROACH_MAX_DISTANCE = 300;
    
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
            int walkDistance = distance - APPROACH_DISTANCE;
            walkDistance = MIN(walkDistance, APPROACH_MAX_DISTANCE);
            walkForwardQuickly(walkDistance);
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
    walkForwardQuickly(80);
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
    
    _setHead(0, 0);
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 2);
    
    return true;
}


static bool _climbUpStair(void) {
    _setHead(0, 0);
    bool isSuccess = runMotion(MOTION_CLIMB_UP_STAIR);
    runWalk(ROBOT_WALK_FORWARD, 2);
    return isSuccess;
}


static bool _crossGreenBridge(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -70;

    const double MILLIMETERS_PER_PIXELS = 3.;
    // 각도 허용 오차 (도)
    const double ALIGN_FACING_ERROR = 5.;
    // 좌우 정렬 허용 오차 (밀리미터)
    const double ALIGN_CENTER_X_ERROR = 15.;
    // 각도 정렬 제한 회전 각도 (도)
    const double ALIGN_TURN_DEGREES_LIMIT = 20.;
    // 좌우 정렬 제한 이동 거리 (밀리미터)
    const double ALIGN_WALK_DISTANCE_LIMIT = 30.;
    const Vector3_t HEAD_OFFSET = { 0.000, -0.020, 0.295 };


    Screen_t* pScreen = createDefaultScreen();
    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);

    while (true) {
        Object_t bridge;
        
        _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);
        readFpgaVideoDataWithWhiteBalance(pScreen);
        _setHead(0, 0);

        bool isOnBridge = _searchGreenBridge(pScreen, &bridge, pLabelMatrix);
        if (!isOnBridge)
            break;
        printDebug("minX: %d, centerX: %f, maxX: %d, minY: %d, centerY: %f, maxY: %d\n", bridge.minX, bridge.centerX, bridge.maxX, bridge.minY, bridge.centerY, bridge.maxY);
            

        bool isEndOfBridge = (bridge.minY > pScreen->height * 0.3);
        if (isEndOfBridge)
            break;

        CameraParameters_t camParams;
        readCameraParameters(&camParams, &HEAD_OFFSET);

        // int bridgeCenterX = (bridge.minX + bridge.maxX) / 2;
        // PixelLocation_t screenLoc = { bridgeCenterX, (int)bridge.maxY };
        // Vector3_t worldLoc;
        // convertScreenLocationToWorldLocation(&camParams, &screenLoc, 0., &worldLoc);
        // double dx = worldLoc.x * 1000;

        double screenCenterX = camParams.cx;
        double bridgeAngle = _calculateBridgeAngle(pLabelMatrix, &bridge);
        double dx = (bridge.centerX - screenCenterX) * MILLIMETERS_PER_PIXELS;

        if (fabs(bridgeAngle) > ALIGN_FACING_ERROR) {
            if (bridgeAngle < 0)
                turnLeft(_MIN(fabs(bridgeAngle), ALIGN_TURN_DEGREES_LIMIT));
            else
                turnRight(_MIN(fabs(bridgeAngle), ALIGN_TURN_DEGREES_LIMIT));
            mdelay(200);
            continue;
        }

        if (fabs(dx) > ALIGN_CENTER_X_ERROR) {
            if (dx < 0)
                walkLeft(_MIN(fabs(dx), ALIGN_WALK_DISTANCE_LIMIT));
            else
                walkRight(_MIN(fabs(dx), ALIGN_WALK_DISTANCE_LIMIT));
            mdelay(200);
            continue;
        }

        printDebug("walk.\n");
        walkForward(34 * 6);
        mdelay(200);
    }
    
    destroyScreen(pScreen);
    destroyMatrix16(pLabelMatrix);

    return true;
}


static bool _approachDownStair(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -80;

    const double MILLIMETERS_PER_PIXELS = 2.25;
    // 각도 허용 오차 (도)
    const double ALIGN_FACING_ERROR = 10.;
    // 최대 회전 각도 (도)
    const double ALIGN_TURN_DEGREES_LIMIT = 20.;
    // 전진보행 접근 거리 (밀리미터)
    const double APPROACH_DISTANCE = 5.;
    // 전진보행 허용 오차 (밀리미터)
    const double APPROACH_DISTANCE_ERROR = 0.;
    // 전진보행으로 갈 수 있는 최대 제한 거리 (밀리미터)
    const double APPROACH_WALK_DISTANCE_LIMIT = 34 * 4;
    // 브라켓이 가려서 영상에서 최대한 달라붙어도 10mm 오차가 생긴다. 때문에 접근할 때 10mm 더 간다.
    const double APPROACH_ADD_WALK_DISTANCE = 10.;

    Screen_t* pScreen = createDefaultScreen();
    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);

    while (true) {
        Object_t blackLine;
        
        _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);
        mdelay(500);
        readFpgaVideoDataWithWhiteBalance(pScreen);

        bool isOnStair = _searchBlackLine(pScreen, &blackLine, pLabelMatrix);
        if (!isOnStair)
            break;

        double stairAngle = _calculateStairAngle(pLabelMatrix, &blackLine);
        double dy = (double)(_LINE_DETECTION_HEIGHT - blackLine.minY) * MILLIMETERS_PER_PIXELS;
        printDebug("stairAngle: %f, dy: %f\n", stairAngle, dy);

        if (fabs(stairAngle) > ALIGN_FACING_ERROR) {
            if (stairAngle < 0)
                turnLeft(_MIN(fabs(stairAngle), ALIGN_TURN_DEGREES_LIMIT));
            else
                turnRight(_MIN(fabs(stairAngle), ALIGN_TURN_DEGREES_LIMIT));
            mdelay(200);
            continue;
        }

        if (dy > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            walkForward(_MIN(dy - APPROACH_DISTANCE + APPROACH_ADD_WALK_DISTANCE, APPROACH_WALK_DISTANCE_LIMIT));
            mdelay(200);
            continue;
        }

        break;
    }
    
    destroyScreen(pScreen);
    destroyMatrix16(pLabelMatrix);

    return true;
}


static bool _climbDownStair(void) {
    _setHead(0, 0);
    runMotion(MOTION_CLIMB_DOWN_STAIR);

    checkCenterMain();
    runWalk(ROBOT_WALK_FORWARD, 4);

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

static double _calculateStairAngle(Matrix16_t* pLabelMatrix, Object_t* pStairObject) {
    Polygon_t* pPolygon = createPolygon(pLabelMatrix, pStairObject, 3);
    Line_t* pLine = NULL;

    if (pStairObject->maxY == _LINE_DETECTION_HEIGHT - 1)
        pLine = findTopLine(pPolygon);
    else
        pLine = findBottomLine(pPolygon);

    double degrees = pLine->theta;

    destroyPolygon(pPolygon);
    free(pLine);

    return degrees;
}


static int _measureGreenBridgeCenterOffsetX(void) {
    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES = 0;
    const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);
    _setHead(0, 0);

    Object_t object;
    bool hasFound = _searchGreenBridge(pScreen, &object, NULL);

    int offsetX = 0;
    if (hasFound) {
        printDebug("minX: %d, centerX: %f, maxX: %d, minY: %d, centerY: %f, maxY: %d\n", object.minX, object.centerX, object.maxX, object.minY, object.centerY, object.maxY);

        offsetX = (int)(object.centerX) - (pScreen->width / 2);
    }

    destroyScreen(pScreen);

    return offsetX;
}


// pScreen에서 녹색 다리를 찾는다.
static bool _searchGreenBridge(Screen_t* pScreen, Object_t* pReturnedObject, Matrix16_t* pLabelMatrix) {
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
    
    if (hasFound && pReturnedObject)
        memcpy(pReturnedObject, pGreenBridge, sizeof(Object_t));
    
    Screen_t* pDebugScreen = cloneMatrix16(pScreen);
    drawColorMatrix(pDebugScreen, pGreenMatrix);
    drawObjectEdge(pDebugScreen, pGreenBridge, NULL);
    drawObjectCenter(pDebugScreen, pGreenBridge, NULL);
    displayScreen(pDebugScreen);
    destroyScreen(pDebugScreen);

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


static bool _searchBlackLine(Screen_t* pScreen, Object_t* pReturnedObject, Matrix16_t* pLabelMatrix) {
    if (!pScreen) return false;

    Matrix8_t* pBlackMatrix = _createBlackMatrix(pScreen);

    ObjectList_t* pObjectList = NULL;
    if (pLabelMatrix)
        pObjectList = detectObjectsLocationWithLabeling(pBlackMatrix, pLabelMatrix);
    else
        pObjectList = detectObjectsLocation(pBlackMatrix);

    // 가장 먼 수평선을 찾는다.
    Object_t* pMostNearestObject = NULL;
    float minCenterY = 0;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        int width = pObject->maxX - pObject->minX + 1;
        bool isLine = (width == _LINE_DETECTION_WIDTH);
        if (!isLine)
            continue;

        if (!pMostNearestObject || pObject->centerY < minCenterY) {
            pMostNearestObject = pObject;
            minCenterY = pObject->centerY;
        }
    }

    Object_t* pBlackLine = NULL;
    bool hasFound = false;
    if (pMostNearestObject) {
        pBlackLine = pMostNearestObject;
        hasFound = true;
    }
    
    if (hasFound && pReturnedObject)
        memcpy(pReturnedObject, pBlackLine, sizeof(Object_t));

    Screen_t* pDebugScreen = cloneMatrix16(pScreen);
    drawColorMatrix(pDebugScreen, pBlackMatrix);
    drawObjectEdge(pDebugScreen, pBlackLine, NULL);
    drawObjectCenter(pDebugScreen, pBlackLine, NULL);
    displayScreen(pDebugScreen);
    destroyScreen(pDebugScreen);

    destroyMatrix8(pBlackMatrix);
    destroyObjectList(pObjectList);

    return hasFound;
}


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
    int minX = centerX - (_LINE_DETECTION_WIDTH / 2);
    int maxX = minX + _LINE_DETECTION_WIDTH - 1;
    int minY = 0;
    int maxY = minY + _LINE_DETECTION_HEIGHT - 1;
    
    Matrix8_t* pSubMatrix = createSubMatrix8(pMatrix, minX, minY, maxX, maxY);
    memset(pMatrix->elements, 0, width * height * sizeof(uint8_t));
    overlapMatrix8(pSubMatrix, pMatrix, minX, minY);
    destroyMatrix8(pSubMatrix);

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

    setServoSpeed(45);
    setHead(horizontalDegrees, verticalDegrees);
    resetServoSpeed();
    mdelay(200);
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
