#define DEBUG

#include <stdlib.h>
#include <string.h>

#include "horizontal_barricade.h"
#include "white_balance.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "camera.h"
#include "math.h"
#include "timer.h"
#include "log.h"
#include "debug.h"
#include "screenio.h"


static bool _approachHorizontalBarricade(void);
static bool _waitHorizontalBarricadeUp(void);
static bool _searchHorizontalBarricade(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation);
static double _getHorizontalBarricadeCorrelation(Matrix8_t* pColorMatrix, Object_t* pObject);
static void _setHead(int horizontalDegrees, int verticalDegrees);


int measureHorizontalBarricadeDistance(void) {
    // 최대 거리
    const int MAX_DISTANCE = 5000;

    // 거리 측정에 사용되는 머리 각도
    const int HEAD_HORIZONTAL_DEGREES[] = { 0, 0 };
    const int HEAD_VERTICAL_DEGREES[] = { -35, -50 };
    const int NUMBER_OF_HEAD_DEGREES = (sizeof(HEAD_HORIZONTAL_DEGREES) / sizeof(HEAD_HORIZONTAL_DEGREES[0]));

    const Vector3_t HEAD_OFFSET = { -0.040, -0.020, 0.295 };

    Screen_t* pScreen = createDefaultScreen();

    Object_t obstacle;
    double correlation;
    bool hasFound = false;
    for (int i = 0; i < NUMBER_OF_HEAD_DEGREES; ++i) {
        _setHead(HEAD_HORIZONTAL_DEGREES[i], HEAD_VERTICAL_DEGREES[i]);
        readFpgaVideoDataWithWhiteBalance(pScreen);
        
        hasFound = _searchHorizontalBarricade(pScreen, &obstacle, &correlation);
        if (hasFound) break;
    }

    int millimeters = 0;
    if (hasFound) {
        printDebug("minY: %d, centerY: %f, maxY: %d\n",
            obstacle.minY, obstacle.centerY, obstacle.maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        // BUG: 올라가는 중이거나 내려가는 중인 바리케이드라면 측정이 제대로 안될 수 있다.
        bool tooFar = (obstacle.minY == 0);
        bool tooClose = (obstacle.maxY == pScreen->height - 1);
        if (tooFar && tooClose) {
            millimeters = 0;    // 바리케이드가 아니다.
        }
        else {
            CameraParameters_t camParams;
            readCameraParameters(&camParams, &HEAD_OFFSET);

            Vector3_t obstacleLoc;
            PixelLocation_t screenLoc = { (int)obstacle.centerX, (int)obstacle.maxY };
            convertScreenLocationToWorldLocation(&camParams, &screenLoc, 0.220, &obstacleLoc);
            millimeters = obstacleLoc.y * 1000;
        }

        if (millimeters > MAX_DISTANCE)
            millimeters = 0;
    }

    destroyScreen(pScreen);

    printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}


bool horizontalBarricadeMain(void) {
    //if (measureHorizontalBarricadeDistance() <= 0)
    //    return false;

    return solveHorizontalBarricade();
}

bool solveHorizontalBarricade(void) {
    printLog("[%s] 수평 바리케이드에 접근한다.\n", __func__);
    if (!_approachHorizontalBarricade()) {
        printLog("[%s] 접근 실패: 시간 초과\n", __func__);
        return false;
    }

    printLog("[%s] 수평 바리케이드가 사라질 때까지 대기한다.\n", __func__);
    if (!_waitHorizontalBarricadeUp()) {
        printLog("[%s] 대기 실패: 시간 초과\n", __func__);
        return false;
    }

    printLog("[%s] 달린다.\n", __func__);
    walkForward(256);

    return true;
}


static bool _approachHorizontalBarricade(void) {
    // 제한 시간 (밀리초)
    const int STAND_BY_TIMEOUT = 15000;

    // 바리케이드 인식 거리 허용 오차 (밀리미터)
    const int MEASURING_ERROR = 10;
    // 바리케이드에 다가갈 거리 (밀리미터)
    const int APPROACH_DISTANCE = 120;
    // 거리 허용 오차 (밀리미터)
    const int APPROACH_DISTANCE_ERROR = 20;
    
    const int APPROACH_MAX_DISTANCE = 300;

    uint64_t startTime = getTime();
    int distance = 0;
    int tempDistance1 = 0;
    int tempDistance2 = 0;
    while ((distance == 0) || (distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR)) {
        uint64_t elapsedTime = (getTime() - startTime) / 1000;
        if (elapsedTime >= STAND_BY_TIMEOUT)
            return false;

        tempDistance1 = tempDistance2;
        tempDistance2 = measureHorizontalBarricadeDistance();

        if (tempDistance1 == 0 || tempDistance2 == 0)
            continue;

        int measuringError = abs(tempDistance2 - tempDistance1);
        if (measuringError > MEASURING_ERROR)
            continue;
        
        distance = tempDistance2;
        bool isFar = distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR;
        if (isFar) {
            int walkingDistance = distance - APPROACH_DISTANCE;
            walkingDistance = MIN(walkingDistance, APPROACH_MAX_DISTANCE);
            walkForward(walkingDistance);

            startTime = getTime();
        }
    }

    return true;
}

static bool _waitHorizontalBarricadeUp(void) {
    // 제한 시간 (밀리초)
    const int STAND_BY_TIMEOUT = 15000;

    // 사진을 MAX_COUNT회 찍어 바리케이드가 계속 없다면 사라졌다고 판단한다.
    const int MAX_COUNT = 2;

    uint64_t startTime = getTime();
    int count = 0;
    while (count < MAX_COUNT) {
        uint64_t elapsedTime = (getTime() - startTime) / 1000;
        if (elapsedTime >= STAND_BY_TIMEOUT)
            return false;

        bool isExists = (measureHorizontalBarricadeDistance() > 0);
        if (!isExists)
            count++;
        else
            count = 0;
    }

    return true;
}

static bool _searchHorizontalBarricade(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation) {
    const double MIN_CORRELATION = 0.00;

    if (!pInputScreen) return false;

    Screen_t* pScreen = cloneMatrix16(pInputScreen);
    Matrix8_t* pColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);

    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastWidthErosionToMatrix8(pColorMatrix, 6);
    applyFastWidthDilationToMatrix8(pColorMatrix, 6);

    ObjectList_t* pObjectList = detectObjectsLocation(pColorMatrix);
    Object_t* pMostSimilarObject = NULL;
    double maxCorrelation = 0.;
    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);
        double correlation = _getHorizontalBarricadeCorrelation(pColorMatrix, pObject);
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

static double _getHorizontalBarricadeCorrelation(Matrix8_t* pColorMatrix, Object_t* pObject) {
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
