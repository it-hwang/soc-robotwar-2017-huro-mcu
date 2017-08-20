#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "vertical_barricade.h"
#include "color.h"
#include "color_model.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "white_balance.h"
#include "log.h"

#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))


static void _drawColorMatrixBY(Screen_t* pScreen, Matrix8_t* pBlackMatrix, Matrix8_t* pYellowMatrix) {
    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    int i;
    PixelData_t* pScreenPixel = pScreen->elements;
    Color_t* pBlackPixel = pBlackMatrix->elements;
    Color_t* pYellowPixel = pYellowMatrix->elements;

    for (i = 0; i < length; ++i) {
        if (*pBlackPixel && *pYellowPixel)
            *pScreenPixel = colorToRgab5515Data(COLOR_BLUE);
        else if (*pBlackPixel)
            *pScreenPixel = colorToRgab5515Data(*pBlackPixel);
        else
            *pScreenPixel = colorToRgab5515Data(*pYellowPixel);
        pScreenPixel++;
        pBlackPixel++;
        pYellowPixel++;
    }
}

static void _removeNoiseOfColorMatrix(Matrix8_t* pColorMatrix) {
    applyFastErosionToMatrix8(pColorMatrix, 1);
    applyFastDilationToMatrix8(pColorMatrix, 2);
    applyFastErosionToMatrix8(pColorMatrix, 1);
}

static void _drawColorScreen(Screen_t* pScreen) {
    Matrix8_t* pBlackMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLACK]);
    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);

    _removeNoiseOfColorMatrix(pBlackMatrix);
    _removeNoiseOfColorMatrix(pYellowMatrix);

    _drawColorMatrixBY(pScreen, pBlackMatrix, pYellowMatrix);
    destroyMatrix8(pBlackMatrix);
    destroyMatrix8(pYellowMatrix);
}


// 가장 직각 사각형에 가까운 객체를 찾는다.
// 만일, 유사도가 같다면, 더 큰 물체를 선택한다.
static Object_t* _findMostRectangleObject(Matrix8_t* pMatrix, ObjectList_t* pObjectList) {
    if (pObjectList == NULL)
        return NULL;

    float maxCorrelation = 0.;
    Object_t* pMostRectangleObject = NULL;

    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);
        float correlation = getRectangleCorrelation(pMatrix, pObject);

        if (correlation > maxCorrelation) {
            pMostRectangleObject = pObject;
            maxCorrelation = correlation;
        }
        else if (correlation == maxCorrelation) {
            if (pMostRectangleObject == NULL || pObject->cnt > pMostRectangleObject->cnt)
                pMostRectangleObject = pObject;
        }
    }

    return pMostRectangleObject;
}

// pObjectList에서 pObject와 인접한 왼쪽 객체를 찾는다.
static Object_t* _findLeftNeighborObject(Object_t* pObject, ObjectList_t* pObjectList) {
    static const int MARGIN_X = 3;

    if (pObject == NULL)
        return NULL;
    if (pObjectList == NULL)
        return NULL;

    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pCurrentObject = &(pObjectList->list[i]);
        bool isSimilarY = ((pCurrentObject->minY <= pObject->centerY) &&
                           (pCurrentObject->maxY >= pObject->centerY));
        bool isLeft = ((pCurrentObject->maxX >= pObject->minX - MARGIN_X) &&
                       (pCurrentObject->maxX <= pObject->maxX) &&
                       (pCurrentObject->minX < pObject->minX));

        if (isSimilarY && isLeft) {
            return pCurrentObject;
        }
    }

    return NULL;
}

// pObjectList에서 pObject와 인접한 오른쪽 객체를 찾는다.
static Object_t* _findRightNeighborObject(Object_t* pObject, ObjectList_t* pObjectList) {
    static const int MARGIN_X = 3;

    if (pObject == NULL)
        return NULL;
    if (pObjectList == NULL)
        return NULL;

    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pCurrentObject = &(pObjectList->list[i]);
        bool isSimilarY = ((pCurrentObject->minY <= pObject->centerY) &&
                           (pCurrentObject->maxY >= pObject->centerY));
        bool isRightPosition = ((pCurrentObject->minX <= pObject->maxX + MARGIN_X) &&
                                (pCurrentObject->minX >= pObject->minX) &&
                                (pCurrentObject->maxX > pObject->maxX));

        if (isSimilarY && isRightPosition) {
            return pCurrentObject;
        }
    }

    return NULL;
}

static void _mergeObject(Object_t* pSrcObject, Object_t* pDstObject) {
    if (pSrcObject == NULL)
        return;
    if (pDstObject == NULL)
        return;

    pDstObject->minX = _MIN(pDstObject->minX, pSrcObject->minX);
    pDstObject->maxX = _MAX(pDstObject->maxX, pSrcObject->maxX);
    pDstObject->cnt = pDstObject->cnt + pSrcObject->cnt;
}

// BUG: 간혹 잡음을 바리케이드로 인식하는 경우가 있다.
static Object_t* _searchVerticalBarricade(Screen_t* pScreen) {
    static const char* LOG_FUNCTION_NAME = "_searchVerticalBarricade()";
    // 하단 판정 Y값
    static const int BOTTOM_Y = 60;
    // 직사각형의 형태와 유사해야한다.
    static const float MIN_RECTANGLE_CORRELATION = 0.75;
    // 유사도를 후하게 쳐줄 경우
    static const float MIN_RECTANGLE_CORRELATION2 = 0.45;
    // 가장 큰 물체와 크기 차이가 적어야한다.
    static const float LARGEST_OBJECT_RELATIVE_RATIO = 0.35;

    Matrix8_t* pBlackMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLACK]);
    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);

    _removeNoiseOfColorMatrix(pBlackMatrix);
    _removeNoiseOfColorMatrix(pYellowMatrix);

    Object_t* pVerticalBarricadeObject = NULL;

    ObjectList_t* pYellowObjectList = detectObjectsLocation(pYellowMatrix);
    ObjectList_t* pBlackObjectList = detectObjectsLocation(pBlackMatrix);
    if (pYellowObjectList != NULL && pYellowObjectList->size > 0) {
        Object_t* pLargestObject = findLargestObject(pYellowObjectList);
        int minimumCnt = pLargestObject->cnt * LARGEST_OBJECT_RELATIVE_RATIO;
        removeSmallObjects(pYellowObjectList, minimumCnt);
        removeSmallObjects(pBlackObjectList, minimumCnt);
        printLog("[%s] minimumCnt: %d\n", LOG_FUNCTION_NAME, minimumCnt);

        while (pYellowObjectList->size > 0) {
            Object_t* pMostRectangleObject = _findMostRectangleObject(pYellowMatrix, pYellowObjectList);
            float correlation = getRectangleCorrelation(pYellowMatrix, pMostRectangleObject);
            bool onBottom = (pMostRectangleObject->minY >= BOTTOM_Y);
            printLog("[%s] correlation: %f\n", LOG_FUNCTION_NAME, correlation);

            // 바리케이드가 평소에는 직사각형으로 보인다.
            if (!onBottom && correlation < MIN_RECTANGLE_CORRELATION) {
                removeObjectFromList(pYellowObjectList, pMostRectangleObject);
                continue;
            }
            // 바리케이드가 하단에 걸치면 왜곡이 심해져서 직사각형으로 보이지 않는다.
            // 하단에 걸친경우 유사도를 후하게 쳐준다.
            if (onBottom && correlation < MIN_RECTANGLE_CORRELATION2) {
                removeObjectFromList(pYellowObjectList, pMostRectangleObject);
                continue;
            }

            bool tooClose = (pMostRectangleObject->maxY == pScreen->height - 1);
            bool tooFar = (pMostRectangleObject->minY == 0);
            // 너무 가까우면서 너무 큰 경우는 없다. (바리케이드가 아닌 다른 물체이다.)
            if (tooClose && tooFar) {
                removeObjectFromList(pYellowObjectList, pMostRectangleObject);
                continue;
            }

            Object_t* pLeftObject = _findLeftNeighborObject(pMostRectangleObject, pBlackObjectList);
            Object_t* pRightObject = _findRightNeighborObject(pMostRectangleObject, pBlackObjectList);
            bool hasNeighbor = (pLeftObject != NULL || pRightObject != NULL);
            // 가깝지 않다면 이웃을 식별할 수 있다.
            // 가깝지 않은데도 이웃이 없다면 바리케이드가 아니다.
            if (!tooClose && !hasNeighbor) {
                removeObjectFromList(pYellowObjectList, pMostRectangleObject);
                continue;
            }

            pVerticalBarricadeObject = malloc(sizeof(Object_t));
            memcpy(pVerticalBarricadeObject, pMostRectangleObject, sizeof(Object_t));
            pVerticalBarricadeObject->color = COLOR_NONE;

            if (pLeftObject != NULL)
                _mergeObject(pLeftObject, pVerticalBarricadeObject);
            if (pRightObject != NULL)
                _mergeObject(pRightObject, pVerticalBarricadeObject);

            break;
        }
    }

    if (pVerticalBarricadeObject != NULL)
        printLog("[%s] 객체를 찾았습니다.\n", LOG_FUNCTION_NAME);
    else
        printLog("[%s] 객체를 찾을 수 없습니다.\n", LOG_FUNCTION_NAME);

    destroyObjectList(pYellowObjectList);
    destroyObjectList(pBlackObjectList);
    destroyMatrix8(pBlackMatrix);
    destroyMatrix8(pYellowMatrix);

    return pVerticalBarricadeObject;
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
    // runMotion(MOTION_HEAD_FRONT);
    setHead(horizontalDegrees, verticalDegrees);
    resetServoSpeed();
    // mdelay(500);
    mdelay(800);
}

int measureVerticalBarricadeDistance(void) {
    static const char* LOG_FUNCTION_NAME = "measureVerticalBarricadeDistance()";

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
        printLog("[%s] minY: %d, centerY: %f, maxY: %d\n", LOG_FUNCTION_NAME,
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

    printLog("[%s] millimeters: %d\n", LOG_FUNCTION_NAME, millimeters);
    return millimeters;
}


bool verticalBarricadeMain(void) {
    if (measureVerticalBarricadeDistance() <= 0)
        return false;

    return solveVerticalBarricade();
}


static bool _waitVerticalBarricadeUp(void) {
    static const char* LOG_FUNCTION_NAME = "_waitVerticalBarricadeUp()";

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

static bool _approachVerticalBarricade(void) {
    static const char* LOG_FUNCTION_NAME = "_approachVerticalBarricade()";

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

bool solveVerticalBarricade(void) {
    static const char* LOG_FUNCTION_NAME = "solveVerticalBarricade()";

    printLog("[%s] 수직 바리케이드에 접근한다.\n", LOG_FUNCTION_NAME);
    if (!_approachVerticalBarricade()) {
        printLog("[%s] 접근 실패: 시간 초과\n", LOG_FUNCTION_NAME);
        return false;
    }

    printLog("[%s] 수직 바리케이드가 사라질 때까지 대기한다.\n", LOG_FUNCTION_NAME);
    if (!_waitVerticalBarricadeUp()) {
        printLog("[%s] 대기 실패: 시간 초과\n", LOG_FUNCTION_NAME);
        return false;
    }

    printLog("[%s] 달린다.\n", LOG_FUNCTION_NAME);
    runWalk(ROBOT_WALK_FORWARD_FAST, 8);

    return true;
}
