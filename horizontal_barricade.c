// #define DEBUG

#include "horizontal_barricade.h"

#include "log.h"
#include "debug.h"


static bool _approachHorizontalBarricade(void);
static bool _waitHorizontalBarricadeUp(void);


int measureVerticalBarricadeDistance(void) {
    // 최대 거리
    static const int MAX_DISTANCE = 500;

    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    int millimeters = 0;
    Object_t* pObject = _searchVerticalBarricade(pScreen);
    if (pObject != NULL) {
        printDebug("minY: %d, centerY: %f, maxY: %d\n",
                 pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        // BUG: 올라가는 중이거나 내려가는 중인 바리케이드라면 측정이 제대로 안될 수 있다.
        bool tooFar = (pObject->minY == 0);
        bool tooClose = (pObject->maxY == pScreen->height - 1);
        if (tooFar && tooClose) {
            millimeters = 0;    // 바리케이드가 아니다.
        }
        else if (tooClose || (pObject->minY > pScreen->height / 2)) {
            millimeters = -0.9434 * pObject->minY + 156.6038;
        }
        else {
            millimeters = -139.4 * log(pObject->maxY) + 740.62;
        }

        if (millimeters > MAX_DISTANCE)
            millimeters = 0;
    }

    _drawColorScreen(pScreen);
    drawObjectEdge(pScreen, pObject, NULL);
    displayScreen(pScreen);

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}


bool verticalBarricadeMain(void) {
    if (measureVerticalBarricadeDistance() <= 0)
        return false;

    return solveVerticalBarricade();
}

bool solveHorizontalBarricade(void) {
    printDebug("수직 바리케이드에 접근한다.\n");
    if (!_approachHorizontalBarricade()) {
        printDebug("접근 실패: 시간 초과\n");
        return false;
    }

    printDebug("수직 바리케이드가 사라질 때까지 대기한다.\n");
    if (!_waitHorizontalBarricadeUp()) {
        printDebug("대기 실패: 시간 초과\n");
        return false;
    }

    printDebug("달린다.\n");
    walkForward(256);

    return true;
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


static bool _approachHorizontalBarricade(void) {
    // 제한 시간 (밀리초)
    static const int STAND_BY_TIMEOUT = 15000;
    // 반복문을 한번 도는데 걸리는 시간 (밀리초)
    static const int LOOP_DELAY = 360;

    // 바리케이드 인식 거리 허용 오차 (밀리미터)
    static const int MEASURING_ERROR = 10;
    // 바리케이드에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 80;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 35;

    int elapsedTime = 0;
    int distance = 0;
    int tempDistance1 = 0;
    int tempDistance2 = 0;
    while ((distance == 0) || (distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR)) {
        if (elapsedTime >= STAND_BY_TIMEOUT)
            return false;
        elapsedTime += LOOP_DELAY;

        tempDistance1 = tempDistance2;
        tempDistance2 = measureVerticalBarricadeDistance();

        if (tempDistance1 == 0 || tempDistance2 == 0)
            continue;

        int measuringError = abs(tempDistance2 - tempDistance1);
        if (measuringError > MEASURING_ERROR)
            continue;
        
        distance = tempDistance2;
        bool isFar = distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR;
        if (isFar) {
            int walkingDistance = distance - APPROACH_DISTANCE;
            walkForward(walkingDistance);

            elapsedTime = 0;
        }
    }

    return true;
}

static bool _waitHorizontalBarricadeUp(void) {
    // 제한 시간 (밀리초)
    static const int STAND_BY_TIMEOUT = 15000;
    // 반복문을 한번 도는데 걸리는 시간 (밀리초)
    static const int LOOP_DELAY = 360;

    // 사진을 MAX_COUNT회 찍어 바리케이드가 계속 없다면 사라졌다고 판단한다.
    static const int MAX_COUNT = 2;

    int elapsedTime = 0;
    int count = 0;
    while (count < MAX_COUNT) {
        if (elapsedTime >= STAND_BY_TIMEOUT)
            return false;
        elapsedTime += LOOP_DELAY;

        bool isExists = (measureVerticalBarricadeDistance() > 0);
        if (!isExists)
            count++;
        else
            count = 0;
    }

    return true;
}
