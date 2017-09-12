#define DEBUG

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
#include "polygon_detection.h"
#include "camera.h"
#include "line_detection.h"
#include "math.h"
#include "log.h"
#include "debug.h"


static bool _approachRedBridge(void);
static bool _approachRedBridgeUp(void);
static bool _climbUp(void);
static bool _approachRedBridgeDown(void);
static bool _climbDown(void);
static bool _findRedBridge(Object_t* pObject, Polygon_t* pPolygon);
static bool _searchRedBridge(Screen_t* pInputScreen, Object_t* pOutputObject, Matrix16_t* pOutputLabelMatrix);
static float _getRedBridgeCorrelation(Matrix8_t* pRedMatrix, Object_t* pRedObject);
static void _convertWorldLoc(const PixelLocation_t* pScreenLoc, double height, Vector3_t* pWorldLoc);
static void _setHead(int horizontalDegrees, int verticalDegrees);
static void _walkForwardQuickly(int distance);


bool redBridgeMain(void) {
    bool hasFound = (measureRedBridgeDistance() > 0);
    if (!hasFound)
        return false;

    return solveRedBridge();
}

int measureRedBridgeDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -35;

    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

        Object_t  object;
        bool hasFound = _findRedBridge(&object, NULL);
        if (!hasFound)
            continue;

        PixelLocation_t pixelLoc = { (int)object.centerX, object.maxY };
        Vector3_t worldLoc;
        _convertWorldLoc(&pixelLoc, 0.000, &worldLoc);
        int dy = worldLoc.y * 1000;

        if (dy <= 0) dy = 1;
        return dy;
    }
    
    printLog("시간 초과!\n");
    return 0;
}

bool solveRedBridge(void) {
    _approachRedBridge();
    _approachRedBridgeUp();
    _climbUp();
    _approachRedBridgeDown();
    _climbDown();

    return true;
}


static bool _approachRedBridge(void) {
    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -35;

    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;

    // 각도 정렬 허용 오차 (도)
    static const double ALIGN_ANGLE_ERROR = 5.;
    // 좌우 정렬 허용 오차 (밀리미터)
    static const int ALIGN_DISTANCE_ERROR = 100;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 30;
    // 장애물에 최대로 다가갈 거리 (밀리미터)
    static const int APPROACH_MAX_WALK_DISTANCE = 300;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 50;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

        Object_t  object;
        Polygon_t polygon;
        bool hasFound = _findRedBridge(&object, &polygon);
        if (!hasFound)
            continue;
        
        bool isClose = (object.maxY == DEFAULT_SCREEN_HEIGHT - 1);
        if (isClose)
            return true;

        PixelLocation_t pixelLoc = { (int)object.centerX, object.maxY };
        Vector3_t worldLoc;
        _convertWorldLoc(&pixelLoc, 0.000, &worldLoc);
        int dx = worldLoc.x * 1000;
        int dy = worldLoc.y * 1000;

        Line_t* pLine = findTopLine(&polygon);
        double angle = pLine->theta;
        free(pLine);
        
        if (fabs(angle) > ALIGN_ANGLE_ERROR) {
            if (angle < 0) turnLeft(fabs(angle));
            else turnRight(fabs(angle));
            nTries = 0;
            continue;
        }
        
        if (abs(dx) > ALIGN_DISTANCE_ERROR) {
            if (dx < 0) walkLeft(abs(dx));
            else walkRight(abs(dx));
            nTries = 0;
            continue;
        }
        
        if (dy > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            int walkDistance = dy - APPROACH_DISTANCE;
            walkDistance = MIN(walkDistance, APPROACH_MAX_WALK_DISTANCE);
            _setHead(0, 0);
            walkForward(walkDistance);
            mdelay(200);
            nTries = 0;
            continue;
        }

        return true;
    }
    
    printLog("시간 초과!\n");
    return false;
}

static bool _approachRedBridgeUp(void) {
    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -35;

    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;

    // 빨간 다리 길이 (밀리미터)
    static const int RED_BRIDGE_LENGTH = 800;
    // 각도 정렬 허용 오차 (도)
    static const double ALIGN_ANGLE_ERROR = 5.;
    // 좌우 정렬 허용 오차 (밀리미터)
    static const int ALIGN_DISTANCE_ERROR = 20;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = -10;
    // 장애물에 최대로 다가갈 거리 (밀리미터)
    static const int APPROACH_MAX_WALK_DISTANCE = 300;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 20;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

        Object_t  object;
        Polygon_t polygon;
        bool hasFound = _findRedBridge(&object, &polygon);
        if (!hasFound)
            continue;

        PixelLocation_t screenLoc = { (int)object.centerX, object.minY };
        Vector3_t worldLoc;
        _convertWorldLoc(&screenLoc, 0.040, &worldLoc);
        int dx = worldLoc.x * 1000;
        int dy = worldLoc.y * 1000 - RED_BRIDGE_LENGTH;

        Line_t* pLine = findTopLine(&polygon);
        double angle = pLine->theta;
        free(pLine);
        
        if (fabs(angle) > ALIGN_ANGLE_ERROR) {
            if (angle < 0) turnLeft(fabs(angle));
            else turnRight(fabs(angle));
            nTries = 0;
            continue;
        }
        
        if (abs(dx) > ALIGN_DISTANCE_ERROR) {
            if (dx < 0) walkLeft(abs(dx));
            else walkRight(abs(dx));
            nTries = 0;
            continue;
        }
        
        if (dy > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            int walkDistance = dy - APPROACH_DISTANCE;
            walkDistance = MIN(walkDistance, APPROACH_MAX_WALK_DISTANCE);
            _setHead(0, 0);
            _walkForwardQuickly(walkDistance);
            runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
            mdelay(200);
            nTries = 0;
            continue;
        }

        return true;
    }
    
    printLog("시간 초과!\n");
    return false;
}

static bool _climbUp(void) {
    runMotion(MOTION_CLIMB_UP_RED_BRIDGE);
    return true;
}

static bool _approachRedBridgeDown(void) {
    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES[] = { 0, 0 };
    static const int HEAD_VERTICAL_DEGREES[] = { -45, -70 };
    const int NUMBER_OF_HEAD_DEGREES = (sizeof(HEAD_HORIZONTAL_DEGREES) / sizeof(HEAD_HORIZONTAL_DEGREES[0]));

    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;

    // 각도 정렬 허용 오차 (도)
    static const double ALIGN_ANGLE_ERROR = 3.;
    // 좌우 정렬 허용 오차 (밀리미터)
    static const int ALIGN_DISTANCE_ERROR = 20;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 30;
    // 장애물에 최대로 다가갈 거리 (밀리미터)
    static const int APPROACH_MAX_WALK_DISTANCE = 300;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 10;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        Object_t  object;
        Polygon_t polygon;
        bool hasFound;
        // 고개를 숙여서 찾다가 물체가 화면을 가득 채우면 들어서 다시 찾는다.
        for (int i = 0; i < NUMBER_OF_HEAD_DEGREES; ++i) {
            _setHead(HEAD_HORIZONTAL_DEGREES[i], HEAD_VERTICAL_DEGREES[i]);

            mdelay(200);
            hasFound = _findRedBridge(&object, &polygon);
            if (!hasFound)
                break;
            else if (object.minY > 0)
                break; 
        }
        if (!hasFound)
            continue;

        Line_t* pLine = findTopLine(&polygon);
        PixelLocation_t screenLoc = { (int)object.centerX, pLine->centerPoint.y };
        Vector3_t worldLoc;
        _convertWorldLoc(&screenLoc, 0.000, &worldLoc);
        int dx = worldLoc.x * 1000;
        int dy = worldLoc.y * 1000;

        double angle = pLine->theta;
        free(pLine);

        
        bool isClose = (dy <= APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR);
        if (isClose)
            return true;
        
        if (fabs(angle) > ALIGN_ANGLE_ERROR) {
            if (angle < 0) turnLeft(fabs(angle));
            else turnRight(fabs(angle));
            nTries = 0;
            continue;
        }
        
        if (abs(dx) > ALIGN_DISTANCE_ERROR) {
            if (dx < 0) walkLeft(abs(dx));
            else walkRight(abs(dx));
            nTries = 0;
            continue;
        }
        
        if (!isClose) {
            int walkDistance = dy - APPROACH_DISTANCE;
            //walkDistance = MIN(walkDistance, APPROACH_MAX_WALK_DISTANCE);
            _setHead(0, 0);
            walkForward(walkDistance);
            mdelay(200);
            nTries = 0;
            return true;
            continue;
        }

        return true;
    }
    
    printLog("시간 초과!\n");
    return false;
}

static bool _climbDown(void) {
    runMotion(MOTION_CLIMB_DOWN_RED_BRIDGE);
    return true;
}

static bool _findRedBridge(Object_t* pOutputObject, Polygon_t* pOutputPolygon) {
    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Object_t object;
    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);
    bool hasFound = _searchRedBridge(pScreen, &object, pLabelMatrix);
    if (!hasFound) {
        destroyMatrix16(pLabelMatrix);
        destroyScreen(pScreen);
        return false;
    }

    Polygon_t* pPolygon = createPolygon(pLabelMatrix, &object, 8);
    if (pOutputObject)
        memcpy(pOutputObject, &object, sizeof(Object_t));
    if (pOutputPolygon)
        memcpy(pOutputPolygon, pPolygon, sizeof(Polygon_t));

    destroyPolygon(pPolygon);
    destroyMatrix16(pLabelMatrix);
    destroyScreen(pScreen);
    return true;
}

// pScreen에서 빨간 다리를 찾는다.
// 반환된 오브젝트는 free()를 통하여 해제시켜야한다.
static bool _searchRedBridge(Screen_t* pInputScreen, Object_t* pOutputObject, Matrix16_t* pOutputLabelMatrix) {
    const int    MIN_AREA        = 400;
    const double MIN_CORRELATION = 0.1;

    if (!pInputScreen) return false;

    Screen_t* pScreen = cloneMatrix16(pInputScreen);
    Matrix8_t* pRedMatrix = createColorMatrix(pScreen, pColorTables[COLOR_RED]);
    
    // 잡음을 제거한다.
    applyFastErosionToMatrix8(pRedMatrix, 1);
    applyFastDilationToMatrix8(pRedMatrix, 2);
    applyFastErosionToMatrix8(pRedMatrix, 1);

    ObjectList_t* pObjectList;
    if (pOutputLabelMatrix)
        pObjectList = detectObjectsLocationWithLabeling(pRedMatrix, pOutputLabelMatrix);
    else
        pObjectList = detectObjectsLocation(pRedMatrix);
    
    removeSmallObjects(pObjectList, MIN_AREA);

    // 유사도가 가장 큰 오브젝트를 찾는다.
    Object_t* pMostSimilarObject = NULL;
    float maxCorrelation = 0.;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        float correlation = _getRedBridgeCorrelation(pRedMatrix, pObject);
        if (correlation >= MIN_CORRELATION && correlation > maxCorrelation) {
            pMostSimilarObject = pObject;
            maxCorrelation = correlation;
        }
    }
    bool hasFound = (pMostSimilarObject != NULL);

    if (pMostSimilarObject && pOutputObject)
        memcpy(pOutputObject, pMostSimilarObject, sizeof(Object_t));

    // display debug screen
    drawColorMatrix(pScreen, pRedMatrix);
    drawObjectEdge(pScreen, pMostSimilarObject, NULL);
    displayScreen(pScreen);

    destroyMatrix8(pRedMatrix);
    destroyObjectList(pObjectList);
    destroyScreen(pScreen);

    return hasFound;
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

static void _convertWorldLoc(const PixelLocation_t* pScreenLoc, double height, Vector3_t* pWorldLoc) {
    const Vector3_t HEAD_OFFSET = { 0.000, -0.020, 0.295 };

    if (!pWorldLoc) return;

    CameraParameters_t camParams;
    readCameraParameters(&camParams, &HEAD_OFFSET);
    convertScreenLocationToWorldLocation(&camParams, pScreenLoc, height, pWorldLoc);

    printDebug("yaw: %f, y: %f\n", camParams.yaw * RAD_TO_DEG, camParams.pitch * RAD_TO_DEG);
    printDebug("x: %f, y: %f\n", pWorldLoc->x, pWorldLoc->y);
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
    mdelay(200);
}

static void _walkForwardQuickly(int distance) {
    // 종종걸음의 보폭 (밀리미터)
    static const int WALK_FORWARD_QUICK_DISTANCE_PER_STEP = 20;

    int nSteps = distance / WALK_FORWARD_QUICK_DISTANCE_PER_STEP;
    if (nSteps < 1) nSteps = 1;

    runWalk(ROBOT_WALK_FORWARD_QUICK, nSteps);
}
