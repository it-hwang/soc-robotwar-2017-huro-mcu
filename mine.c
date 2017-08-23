// #define DEBUG

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mine.h"
#include "log.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "white_balance.h"
#include "image_filter.h"
#include "object_detection.h"
#include "color.h"
#include "debug.h"

// TODO: 알고리즘을 개선해야한다.
//       현재 지뢰 알고리즘은 예선전처럼 굉장히 친절하게 배치되어 있을 때에만
//       작동한다.
//       어떠한 형태로 지뢰가 배치되어도 완벽하게 수행할 수 있도록 알고리즘을
//       개선해야한다.

// 확인할 화면 크기 (단위: pixels)
#define _DETECTION_CENTER_X     96
#define _DETECTION_WIDTH        82
#define _DETECTION_HEIGHT       106
#define _DETECTION_MIN_X        (_DETECTION_CENTER_X - _DETECTION_WIDTH * 0.5)
#define _DETECTION_MAX_X        (_DETECTION_MIN_X + _DETECTION_WIDTH - 1)
#define _DETECTION_MIN_Y        0
#define _DETECTION_MAX_Y        (_DETECTION_MIN_Y + _DETECTION_HEIGHT - 1)


static void _captureScreen(Screen_t* pScreen);
static void _setHead(int horizontalDegrees, int verticalDegrees);
static int _calculateObjectDistance(Object_t* pObject);
static Object_t* _searchClosestMine(Screen_t* pScreen);
static Object_t* _searchOtherObstacle(Screen_t* pScreen);
static bool _actForMine(Object_t* pMine);
static void _displayDebugScreen(Screen_t* pScreen, Object_t* pMine, Object_t* pObstacle);
static Matrix8_t* _createBlackMatrix(Screen_t* pScreen);
static Matrix8_t* _createMineMatrix(Screen_t* pScreen);
static Matrix8_t* _createObstacleMatrix(Screen_t* pScreen);
static Matrix8_t* _createWhiteMatrix(Screen_t* pScreen);
static ObjectList_t* _detectMinesLocation(Matrix8_t* pMatrix);
static float _getMineCorrelation(Object_t* pObject);
static bool _isObjectInDetectionArea(Object_t* pObject);
static void _notMatrix8(Matrix8_t* pMatrix);
static void _overlapColorMatrix(Matrix8_t* pSourceMatrix, Matrix8_t* pTargetMatrix);


bool mineMain(void) {
    solveMine();
    return true;
}


int measureMineDistance(void) {
    return 0;
}

// 다른 장애물이 나올 때 까지 지뢰를 검사하며 앞으로 나아간다.
// 지뢰가 보이면, 지뢰를 해결한다.
// 지뢰가 보이지 않으면, 앞으로 조금 나아가 다시 검사한다.
bool solveMine(void) {
    Screen_t* pScreen = createDefaultScreen();

    bool endOfMine = false;
    while (!endOfMine) {
        _captureScreen(pScreen);

        Object_t* pMine = _searchClosestMine(pScreen);
        Object_t* pObstacle = _searchOtherObstacle(pScreen);

        int mineDistance = _calculateObjectDistance(pMine);
        int obstacleDistance = _calculateObjectDistance(pObstacle);

        // For debug
        if (pMine) {
            printDebug("<지뢰>", __func__);
            printDebug(" minX: %d", pMine->minX);
            printDebug(" centerX: %f", pMine->centerX);
            printDebug(" maxX: %d", pMine->maxX);
            printDebug(" minY: %d", pMine->minY);
            printDebug(" centerY: %f", pMine->centerY);
            printDebug(" maxY: %d", pMine->maxY);
            printDebug(" distance: %d", mineDistance);
            printDebug("\n");
        }
        if (pObstacle) {
            printDebug("<장애물>", __func__);
            printDebug(" minX: %d", pObstacle->minX);
            printDebug(" centerX: %f", pObstacle->centerX);
            printDebug(" maxX: %d", pObstacle->maxX);
            printDebug(" minY: %d", pObstacle->minY);
            printDebug(" centerY: %f", pObstacle->centerY);
            printDebug(" maxY: %d", pObstacle->maxY);
            printDebug(" distance: %d", obstacleDistance);
            printDebug("\n");
        }
        _displayDebugScreen(pScreen, pMine, pObstacle);

        if (pObstacle && (!pMine || obstacleDistance < mineDistance)) {
            printDebug("지뢰밭을 탈출했어.\n", __func__);
            endOfMine = true;
        }
        else if (pMine) {
            printDebug("앞에 있는 지뢰를 해결하자.\n", __func__);
            _actForMine(pMine);
        }
        else {
            printDebug("아무 것도 안보여. 직진해보자.\n", __func__);
            walkForward(128);
        }

        if (pMine) free(pMine);
        if (pObstacle) free(pObstacle);
    }

    destroyScreen(pScreen);

    return true;
}


static void _captureScreen(Screen_t* pScreen) {
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -70;

    if (pScreen == NULL)
        return;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);
    readFpgaVideoDataWithWhiteBalance(pScreen);
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


static int _calculateObjectDistance(Object_t* pObject) {
    if (pObject == NULL)
        return -1;

    int maxY = pObject->maxY;
    int millimeters = -2.6954 * maxY + 292.29;
    if (millimeters < 0)
        millimeters = 0;

    return millimeters;
}


static Object_t* _searchClosestMine(Screen_t* pScreen) {
    Matrix8_t* pMineMatrix = _createMineMatrix(pScreen);
    ObjectList_t* pMineList = _detectMinesLocation(pMineMatrix);
    if (pMineList == NULL) {
        destroyMatrix8(pMineMatrix);
        return NULL;
    }

    Object_t* pClosestObject = NULL;
    int minDistance = 0;
    for (int i = 0; i < pMineList->size; ++i) {
        Object_t* pObject = &(pMineList->list[i]);

        int distance = _calculateObjectDistance(pObject);
        if (pClosestObject == NULL || distance < minDistance) {
            pClosestObject = pObject;
            minDistance = distance;
        }
    }

    Object_t* pClonedObject = (Object_t*)malloc(sizeof(Object_t));
    memcpy(pClonedObject, pClosestObject, sizeof(Object_t));

    destroyMatrix8(pMineMatrix);
    destroyObjectList(pMineList);

    return pClonedObject;
}


static ObjectList_t* _detectMinesLocation(Matrix8_t* pMatrix) {
    static const float MIN_CORRELATION = 0.3;

    ObjectList_t* pMineList = detectObjectsLocation(pMatrix);
    if (pMineList == NULL)
        return NULL;

    // 지뢰가 아닌 객체를 모두 제거한다.
    // 역순으로 순회하여 리스트 꼬임을 방지한다.
    for (int i = pMineList->size - 1; i >= 0; --i) {
        Object_t* pObject = &(pMineList->list[i]);
        if (!_isObjectInDetectionArea(pObject)) {
            removeObjectFromList(pMineList, pObject);
            continue;
        }

        bool isMine = (_getMineCorrelation(pObject) > MIN_CORRELATION);
        if (!isMine) {
            removeObjectFromList(pMineList, pObject);
            continue;
        }
    }

    printDebug("size: %d\n", __func__, pMineList->size);
    if (pMineList->size == 0) {
        destroyObjectList(pMineList);
        return NULL;
    }

    return pMineList;
}


static bool _isObjectInDetectionArea(Object_t* pObject) {
    if (pObject == NULL)
        return false;
    
    if (pObject->minX > _DETECTION_MAX_X) return false;
    if (pObject->maxX < _DETECTION_MIN_X) return false;
    if (pObject->minY > _DETECTION_MAX_Y) return false;
    if (pObject->maxY < _DETECTION_MIN_Y) return false;
    return true;
}

// 지뢰와의 유사한 정도를 반환한다. (범위: 0.0 ~ 1.0)
//  # 일정 크기보다 작아야한다.
//  # 가로 세로 길이가 비슷할 수록 유사도가 높다.
//  # 무게중심이 중앙에 위치할 수록 유사도가 높다.
static float _getMineCorrelation(Object_t* pObject) {
    static const float RATIO_CORRELATION_RATIO  = 0.50;
    static const float CENTER_CORRELATION_RATIO = 0.50;

    static const float CENTER_X_RATIO = 0.50;
    static const float CENTER_Y_RATIO = 0.50;

    static const int MAX_WIDTH  = 30;
    static const int MAX_HEIGHT = 30;

    if (pObject == NULL)
        return 0.;
    
    int objectWidth = pObject->maxX - pObject->minX + 1;
    int objectHeight = pObject->maxY - pObject->minY + 1;
    if (objectWidth > MAX_WIDTH)
        return 0.;
    if (objectHeight > MAX_HEIGHT)
        return 0.;

    float ratioCorrelation;
    if (objectWidth < objectHeight)
        ratioCorrelation = objectWidth / objectHeight;
    else
        ratioCorrelation = objectHeight / objectWidth;

    float centerXCorrelation;
    float objectCenterX = pObject->centerX;
    float idealCenterX = ((float)pObject->maxX * CENTER_X_RATIO) + ((float)pObject->minX * (1. - CENTER_X_RATIO));
    centerXCorrelation = 1.0 - (fabs(objectCenterX - idealCenterX) / objectWidth);
    
    float centerYCorrelation;
    float objectCenterY = pObject->centerY;
    float idealCenterY = ((float)pObject->maxY * CENTER_Y_RATIO) + ((float)pObject->minY * (1. - CENTER_Y_RATIO));
    centerYCorrelation = 1.0 - (fabs(objectCenterY - idealCenterY) / objectHeight);
    
    float centerCorrelation = (centerXCorrelation * 0.5) + (centerYCorrelation * 0.5);

    float correlation = 0.;
    correlation += ratioCorrelation * RATIO_CORRELATION_RATIO;
    correlation += centerCorrelation * CENTER_CORRELATION_RATIO;
    return correlation;
}


static Object_t* _searchOtherObstacle(Screen_t* pScreen) {
    Matrix8_t* pObstacleMatrix = _createObstacleMatrix(pScreen);
    ObjectList_t* pObstacleList = detectObjectsLocation(pObstacleMatrix);
    if (pObstacleList == NULL) {
        destroyMatrix8(pObstacleMatrix);
        return NULL;
    }

    Object_t* closestObstacle = NULL;
    int minDistance = 0;
    for (int i = 0; i < pObstacleList->size; ++i) {
        Object_t* pObject = &(pObstacleList->list[i]);

        bool isObstacle = ((pObject->minX <= _DETECTION_MIN_X) && (pObject->maxX >= _DETECTION_MAX_X));
        if (!isObstacle)
            continue;

        int distance = _calculateObjectDistance(pObject);
        if (closestObstacle == NULL || distance < minDistance) {
            closestObstacle = pObject;
            minDistance = distance;
        }
    }

    if (closestObstacle == NULL) {
        destroyMatrix8(pObstacleMatrix);
        destroyObjectList(pObstacleList);
        return NULL;
    }

    Object_t* pClonedObject = (Object_t*)malloc(sizeof(Object_t));
    memcpy(pClonedObject, closestObstacle, sizeof(Object_t));

    destroyMatrix8(pObstacleMatrix);
    destroyObjectList(pObstacleList);

    return pClonedObject;
}


// pMine의 위치에 따라 행해야 할 행동이다.
static bool _actForMine(Object_t* pMine) {
    // 최대 전진보행 거리
    static const int MAX_WALK_FORWARD_DISTANCE = 128;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 30;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 30;
    
    static const int ALIGN_STANDARD_LEFT_X  = 78;
    static const int ALIGN_STANDARD_RIGHT_X = 114;

    static const int ALIGN_ROBOT_CENTER_X   = 96;
    static const int ALIGN_ROBOT_LEFT_X     = 54;
    static const int ALIGN_ROBOT_RIGHT_X    = 136;
    static const int ALIGN_ROBOT_ERROR      = 3;
    
    static const float _MILLIMETERS_PER_PIXEL = 2.;
    
    int distanceY = _calculateObjectDistance(pMine);
    int centerX = pMine->centerX;
    int minX = pMine->minX;
    int maxX = pMine->maxX;

    bool isFar = (distanceY > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR);
    if (isFar) {
        int walkDistance = distanceY - APPROACH_DISTANCE;
        if (walkDistance > MAX_WALK_FORWARD_DISTANCE)
            walkDistance = MAX_WALK_FORWARD_DISTANCE;
        printDebug("지뢰가 멀리 있다. 접근하자. (distanceY: %d, walkDistance: %d)\n", __func__, distanceY, walkDistance);
        walkForward(walkDistance);
        return true;
    }


    bool isCenterAligned = (abs(centerX - ALIGN_ROBOT_CENTER_X) <= ALIGN_ROBOT_ERROR);
    if (isCenterAligned) {
        printDebug("지뢰 중앙 정렬 완료. 달린다. (centerX: %d)\n", __func__, centerX);
        runMotion(MOTION_MINE_WALK);
        return true;
    }
    bool isLeftAligned = (maxX <= ALIGN_ROBOT_LEFT_X + ALIGN_ROBOT_ERROR);
    if (isLeftAligned) {
        printDebug("지뢰 왼쪽 정렬 완료. 달린다. (maxX: %d)\n", __func__, maxX);
        walkForward(64);
        return true;
    }
    bool isRightAligned = (minX >= ALIGN_ROBOT_RIGHT_X - ALIGN_ROBOT_ERROR);
    if (isRightAligned) {
        printDebug("지뢰 오른쪽 정렬 완료. 달린다. (minX: %d)\n", __func__, minX);
        walkForward(64);
        return true;
    }

    if (centerX < ALIGN_STANDARD_LEFT_X) {
        int walkDistance = fabs(maxX - ALIGN_ROBOT_LEFT_X) * _MILLIMETERS_PER_PIXEL;
        printDebug("지뢰가 왼쪽에 있다. 오른쪽으로 피하자. (centerX: %d, walkDistance: %d)\n", __func__, centerX, walkDistance);
        walkRight(walkDistance);
        return true;
    }
    else if (centerX > ALIGN_STANDARD_RIGHT_X) {
        int walkDistance = fabs(minX - ALIGN_ROBOT_RIGHT_X) * _MILLIMETERS_PER_PIXEL;
        printDebug("지뢰가 오른쪽에 있다. 왼쪽으로 피하자. (centerX: %d, walkDistance: %d)\n", __func__, centerX, walkDistance);
        walkLeft(walkDistance);
        return true;
    }
    else {
        int walkDistance = (float)(centerX - ALIGN_ROBOT_CENTER_X) * _MILLIMETERS_PER_PIXEL;
        printDebug("지뢰가 가운데에 있다. 중앙으로 정렬하자. (centerX: %d, walkDistance: %d)\n", __func__, centerX, walkDistance);
        if (walkDistance < 0)
            walkLeft(walkDistance * -1);
        else
            walkRight(walkDistance);
        return true;
    }
}


static void _displayDebugScreen(Screen_t* pScreen, Object_t* pMine, Object_t* pObstacle) {
    if (pScreen == NULL)
        return;
    
    Screen_t* pDebugScreen = cloneMatrix16(pScreen);
    Matrix8_t* pBlackMatrix = _createBlackMatrix(pScreen);
    Matrix8_t* pWhiteMatrix = _createWhiteMatrix(pScreen);

    Screen_t* pColorScreen = cloneMatrix16(pScreen);
    _overlapColorMatrix(pBlackMatrix, pWhiteMatrix);
    drawColorMatrix(pColorScreen, pWhiteMatrix);
    Screen_t* pSubColorScreen = createSubMatrix16(pColorScreen, _DETECTION_MIN_X, _DETECTION_MIN_Y, _DETECTION_MAX_X, _DETECTION_MAX_Y);
    overlapMatrix16(pSubColorScreen, pDebugScreen, _DETECTION_MIN_X, _DETECTION_MIN_Y);

    Rgab5515_t redColor = {{{0, }}}; redColor.r = 0x1f;
    Rgab5515_t blueColor = {{{0, }}}; blueColor.b = 0x1f;
    drawObjectEdge(pDebugScreen, pMine, &redColor);
    drawObjectEdge(pDebugScreen, pObstacle, &blueColor);

    displayScreen(pDebugScreen);

    destroyScreen(pDebugScreen);
    destroyMatrix8(pBlackMatrix);
    destroyMatrix8(pWhiteMatrix);
    destroyScreen(pColorScreen);
    destroyScreen(pSubColorScreen);
}


/******************************************************************************
 *  Matrix functions
 *****************************************************************************/
static Matrix8_t* _createMineMatrix(Screen_t* pScreen) {
    Matrix8_t* pMineMatrix = _createBlackMatrix(pScreen);

    Matrix8_t* pSubMatrix = createSubMatrix8(pMineMatrix, _DETECTION_MIN_X, _DETECTION_MIN_Y, _DETECTION_MAX_X, _DETECTION_MAX_Y);
    size_t length = pMineMatrix->width * pMineMatrix->height * sizeof(*(pMineMatrix->elements));
    memset(pMineMatrix->elements, 0, length);
    overlapMatrix8(pSubMatrix, pMineMatrix, _DETECTION_MIN_X, _DETECTION_MIN_Y);
    
    destroyMatrix8(pSubMatrix);

    return pMineMatrix;
}

static Matrix8_t* _createObstacleMatrix(Screen_t* pScreen) {
    Matrix8_t* pObstacleMatrix = _createWhiteMatrix(pScreen);
    _notMatrix8(pObstacleMatrix);

    Matrix8_t* pSubMatrix = createSubMatrix8(pObstacleMatrix, _DETECTION_MIN_X, _DETECTION_MIN_Y, _DETECTION_MAX_X, _DETECTION_MAX_Y);
    size_t length = pObstacleMatrix->width * pObstacleMatrix->height * sizeof(uint8_t);
    memset(pObstacleMatrix->elements, 0, length);
    overlapMatrix8(pSubMatrix, pObstacleMatrix, _DETECTION_MIN_X, _DETECTION_MIN_Y);

    destroyMatrix8(pSubMatrix);
    
    return pObstacleMatrix;
}


static Matrix8_t* _createBlackMatrix(Screen_t* pScreen) {
    Matrix8_t* pBlackMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLACK]);
    
    // 잡음을 제거한다.
    applyFastErosionToMatrix8(pBlackMatrix, 1);
    applyFastDilationToMatrix8(pBlackMatrix, 2);
    applyFastErosionToMatrix8(pBlackMatrix, 3);
    applyFastDilationToMatrix8(pBlackMatrix, 2);

    return pBlackMatrix;
}

static Matrix8_t* _createWhiteMatrix(Screen_t* pScreen) {
    Matrix8_t* pWhiteMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);

    // 잡음을 제거한다.
    applyFastDilationToMatrix8(pWhiteMatrix, 1);
    applyFastErosionToMatrix8(pWhiteMatrix, 2);
    applyFastDilationToMatrix8(pWhiteMatrix, 1);

    return pWhiteMatrix;
}

static void _notMatrix8(Matrix8_t* pMatrix) {
    int length = (pMatrix->width) * (pMatrix->height);

    uint8_t* pElement = pMatrix->elements;
    for (int i = 0; i < length; ++i) {
        *pElement = !(*pElement);
        pElement++;
    }
}

static void _overlapColorMatrix(Matrix8_t* pSourceMatrix, Matrix8_t* pTargetMatrix) {
    int width = pTargetMatrix->width;
    int height = pTargetMatrix->height;
    int length = width * height;
    uint8_t* pSourceElement = pSourceMatrix->elements;
    uint8_t* pTargetElement = pTargetMatrix->elements;

    for (int i = 0; i < length; ++i) {
        if (*pSourceElement != COLOR_NONE)
            *pTargetElement = *pSourceElement;
        pSourceElement++;
        pTargetElement++;
    }
}
