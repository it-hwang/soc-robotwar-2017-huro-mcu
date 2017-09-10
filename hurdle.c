#define DEBUG

#include <stdlib.h>
#include <string.h>

#include "hurdle.h"
#include "white_balance.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "camera.h"
#include "math.h"
#include "timer.h"
#include "check_center.h"
#include "log.h"
#include "debug.h"

static bool _approachHurdle(void);
static bool _crossHurdle(void);
static bool _searchHurdle(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation);
static double _getHurdleCorrelation(Matrix8_t* pColorMatrix, Object_t* pObject);
static void _setHead(int horizontalDegrees, int verticalDegrees);

bool hurdleMain(void) {
    //if (measureHurdleDistance() <= 0)
    //    return false;

    return solveHurdle();
}

int measureHurdleDistance(void) {
    // 최대 거리
    const int MAX_DISTANCE = 5000;

    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES[] = { 0, 0 };
    const int HEAD_VERTICAL_DEGREES[] = { -35, -65 };
    const int NUMBER_OF_HEAD_DEGREES = (sizeof(HEAD_HORIZONTAL_DEGREES) / sizeof(HEAD_HORIZONTAL_DEGREES[0]));

    const Vector3_t HEAD_OFFSET = { -0.040, -0.020, 0.295 };

    Screen_t* pScreen = createDefaultScreen();

    Object_t obstacle;
    double correlation;
    bool hasFound = false;
    for (int i = 0; i < NUMBER_OF_HEAD_DEGREES; ++i) {
        _setHead(HEAD_HORIZONTAL_DEGREES[i], HEAD_VERTICAL_DEGREES[i]);
        readFpgaVideoDataWithWhiteBalance(pScreen);
        
        hasFound = _searchHurdle(pScreen, &obstacle, &correlation);
        if (hasFound) break;
    }

    int millimeters = 0;
    if (hasFound) {
        printDebug("minY: %d, centerY: %f, maxY: %d\n",
            obstacle.minY, obstacle.centerY, obstacle.maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        bool tooFar = (obstacle.minY == 0);
        bool tooClose = (obstacle.maxY == pScreen->height - 1);
        if (tooFar && tooClose) {
            millimeters = 0;    // 허들이 아니다.
        }
        else {
            CameraParameters_t camParams;
            readCameraParameters(&camParams, &HEAD_OFFSET);

            Vector3_t obstacleLoc;
            PixelLocation_t screenLoc = { (int)obstacle.centerX, (int)obstacle.maxY };
            convertScreenLocationToWorldLocation(&camParams, &screenLoc, 0.090, &obstacleLoc);
            millimeters = obstacleLoc.y * 1000;
        }

        if (millimeters > MAX_DISTANCE)
            millimeters = 0;
    }

    destroyScreen(pScreen);

    printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}

bool solveHurdle(void) {
    _approachHurdle();
    runWalk(ROBOT_WALK_FORWARD_QUICK, 20);
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
    mdelay(500);

    checkCenterMain();
    
    runWalk(ROBOT_WALK_FORWARD_QUICK, 10);
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);

    _crossHurdle();
}


static bool _approachHurdle(void) {
    // 허들에 다가갈 거리 (밀리미터)
    const int APPROACH_DISTANCE = 50;
    // 거리 허용 오차 (밀리미터)
    const int APPROACH_DISTANCE_ERROR = 20;
    
    const int APPROACH_MAX_DISTANCE = 300;

    int distance = measureHurdleDistance();
    while (distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
        int walkDistance = distance - APPROACH_DISTANCE;
        walkDistance = MIN(walkDistance, APPROACH_MAX_DISTANCE);
        walkForward(walkDistance);

        distance = measureHurdleDistance();
    }

    return true;
}

static bool _crossHurdle(void) {
    runMotion(MOTION_HURDLE);

}

static bool _searchHurdle(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation) {
    const double MIN_CORRELATION = 0.00;

    if (!pInputScreen) return false;

    Screen_t* pScreen = cloneMatrix16(pInputScreen);
    Matrix8_t* pColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);

    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastWidthErosionToMatrix8(pColorMatrix, 3);
    applyFastWidthDilationToMatrix8(pColorMatrix, 3);

    ObjectList_t* pObjectList = detectObjectsLocation(pColorMatrix);
    Object_t* pMostSimilarObject = NULL;
    double maxCorrelation = 0.;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);
        double correlation = _getHurdleCorrelation(pColorMatrix, pObject);
        bool isHorizontalBarricade = (correlation >= MIN_CORRELATION);

        if (isHorizontalBarricade && correlation > maxCorrelation) {
            pMostSimilarObject = pObject;
            maxCorrelation = correlation;
        }
    }
    bool hasFound = (pMostSimilarObject != NULL);

    if (pReturnedObject && pMostSimilarObject)
        memcpy(pReturnedObject, pMostSimilarObject, sizeof(Object_t));
    if (pCorrelation && pMostSimilarObject)
        *pCorrelation = maxCorrelation;

    drawColorMatrix(pScreen, pColorMatrix);
    drawObjectEdge(pScreen, pMostSimilarObject, NULL);
    displayScreen(pScreen);

    destroyObjectList(pObjectList);
    destroyMatrix8(pColorMatrix);
    destroyScreen(pScreen);

    return hasFound;
}

static double _getHurdleCorrelation(Matrix8_t* pColorMatrix, Object_t* pObject) {
    const double RECTANGLE_CORRELATION_RATIO = 0.10;
    const double AREA_CORRELATION_RATIO = 0.90;

    int objectWidth = pObject->maxX - pObject->minX + 1;
    int objectHeight = pObject->maxY - pObject->minY + 1;
    bool isWide = (((double)objectWidth / objectHeight) >= 2.5);

    if (isWide) {
        double rectangleCorrelation = getRectangleCorrelation(pColorMatrix, pObject);
        
        int maxCnt = pColorMatrix->width * pColorMatrix->height;
        double areaCorrelation = (double)pObject->cnt / maxCnt;

        return (rectangleCorrelation * RECTANGLE_CORRELATION_RATIO +
                areaCorrelation * AREA_CORRELATION_RATIO);
    }

    return 0.;
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
