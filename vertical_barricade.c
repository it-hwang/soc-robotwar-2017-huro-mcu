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

static void _drawObjectEdge(Screen_t* pScreen, Object_t* pObject) {
    if (pObject == NULL)
        return;

    int width = pScreen->width;
    int minX = pObject->minX;
    int minY = pObject->minY;
    int maxX = pObject->maxX;
    int maxY = pObject->maxY;

    for (int i = minX; i < maxX; ++i) {
        int topIndex = minY * width + i;
        int bottomIndex = maxY * width + i;
        Rgab5515_t* pTopPixel = &(pScreen->elements[topIndex]);
        Rgab5515_t* pBottomPixel = &(pScreen->elements[bottomIndex]);
        pTopPixel->r = 0x1f;
        pTopPixel->g = 0x00;
        pTopPixel->b = 0x00;
        pBottomPixel->r = 0x1f;
        pBottomPixel->g = 0x00;
        pBottomPixel->b = 0x00;
    }

    for (int i = minY; i < maxY; ++i) {
        int leftIndex = i * width + minX;
        int rightIndex = i * width + maxX;
        Rgab5515_t* pLeftPixel = &(pScreen->elements[leftIndex]);
        Rgab5515_t* pRightPixel = &(pScreen->elements[rightIndex]);
        pLeftPixel->r = 0x1f;
        pLeftPixel->g = 0x00;
        pLeftPixel->b = 0x00;
        pRightPixel->r = 0x1f;
        pRightPixel->g = 0x00;
        pRightPixel->b = 0x00;
    }
}


static int _getIndexOfObject(ObjectList_t* pObjectList, Object_t* pObject) {
    if (pObjectList == NULL)
        return -1;
    if (pObject == NULL)
        return -1;

    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pCurrentObject = &(pObjectList->list[i]);
        if (pCurrentObject == pObject)
            return i;
    }
    return -1;
}

static void _removeObjectFromList(ObjectList_t* pObjectList, Object_t* pObject) {
    if (pObjectList == NULL)
        return;
    if (pObject == NULL)
        return;

    int lastIndex = pObjectList->size - 1;
    Object_t* pLastObject = &(pObjectList->list[lastIndex]);
    Object_t tempObject = *pLastObject;

    memcpy(pLastObject, pObject, sizeof(Object_t));
    memcpy(pObject, &tempObject, sizeof(Object_t));

    pObjectList->size--;
}

static void _destroyObjectList(ObjectList_t* pObjectList) {
    if (pObjectList == NULL)
        return;

    free(pObjectList->list);
    free(pObjectList);
}

// pObjectList에서 minCnt보다 작은 값을 가진 객체들을 제거한다.
static void _filterObjectsByCnt(ObjectList_t* pObjectList, int minCnt) {
    if (pObjectList == NULL)
        return;

    // 도중에 remove하여 리스트의 크기가 변해도 문제가 생기지 않도록
    // 역순으로 리스트를 순회한다.
    int lastIndex = pObjectList->size - 1;
    for (int i = lastIndex; i >= 0; --i) {
        Object_t* pObject = &(pObjectList->list[i]);
        if (pObject->cnt < minCnt)
            _removeObjectFromList(pObjectList, pObject);
    }
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

// 가장 큰 객체를 찾는다.
static Object_t* _findLargestObject(ObjectList_t* pObjectList) {
    if (pObjectList == NULL)
        return NULL;

    int maxArea = 0;
    Object_t* pLargestObject = NULL;

    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);
        int area = pObject->cnt;

        if (area > maxArea) {
            pLargestObject = pObject;
            maxArea = area;
        }
    }

    return pLargestObject;
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
        Object_t* pLargestObject = _findLargestObject(pYellowObjectList);
        int minCnt = pLargestObject->cnt * LARGEST_OBJECT_RELATIVE_RATIO;
        _filterObjectsByCnt(pYellowObjectList, minCnt);
        _filterObjectsByCnt(pBlackObjectList, minCnt);
        printLog("[%s] minCnt: %d\n", LOG_FUNCTION_NAME, minCnt);

        while (pYellowObjectList->size > 0) {
            Object_t* pMostRectangleObject = _findMostRectangleObject(pYellowMatrix, pYellowObjectList);
            float correlation = getRectangleCorrelation(pYellowMatrix, pMostRectangleObject);
            bool onBottom = (pMostRectangleObject->minY >= BOTTOM_Y);
            printLog("[%s] correlation: %f\n", LOG_FUNCTION_NAME, correlation);

            // 바리케이드가 평소에는 직사각형으로 보인다.
            if (!onBottom && correlation < MIN_RECTANGLE_CORRELATION) {
                _removeObjectFromList(pYellowObjectList, pMostRectangleObject);
                continue;
            }
            // 바리케이드가 하단에 걸치면 왜곡이 심해져서 직사각형으로 보이지 않는다.
            // 하단에 걸친경우 유사도를 후하게 쳐준다.
            if (onBottom && correlation < MIN_RECTANGLE_CORRELATION2) {
                _removeObjectFromList(pYellowObjectList, pMostRectangleObject);
                continue;
            }

            bool tooClose = (pMostRectangleObject->maxY == pScreen->height - 1);
            bool tooFar = (pMostRectangleObject->minY == 0);
            // 너무 가까우면서 너무 큰 경우는 없다. (바리케이드가 아닌 다른 물체이다.)
            if (tooClose && tooFar) {
                _removeObjectFromList(pYellowObjectList, pMostRectangleObject);
                continue;
            }

            Object_t* pLeftObject = _findLeftNeighborObject(pMostRectangleObject, pBlackObjectList);
            Object_t* pRightObject = _findRightNeighborObject(pMostRectangleObject, pBlackObjectList);
            bool hasNeighbor = (pLeftObject != NULL || pRightObject != NULL);
            // 가깝지 않다면 이웃을 식별할 수 있다.
            // 가깝지 않은데도 이웃이 없다면 바리케이드가 아니다.
            if (!tooClose && !hasNeighbor) {
                _removeObjectFromList(pYellowObjectList, pMostRectangleObject);
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

    _destroyObjectList(pYellowObjectList);
    _destroyObjectList(pBlackObjectList);
    destroyMatrix8(pBlackMatrix);
    destroyMatrix8(pYellowMatrix);

    return pVerticalBarricadeObject;
}

static void _setHeadToCheck(void) {
    setSpeed(15);
    runMotion(MOTION_HEAD_BOTTOM, true);
    setSpeed(45);
    setHead(0, -35);
    mdelay(280);
    setSpeed(5);
}


bool verticalBarricadeMain(void) {
    for (int i = 0; i < 10; ++i) {
        int distance = measureVerticalBarricadeDistance();
        printf("distance: %d\n", distance);

        char c;
        scanf("%c", &c);
    }

    return true;
}

int measureVerticalBarricadeDistance(void) {
    static const char* LOG_FUNCTION_NAME = "measureVerticalBarricadeDistance()";

    _setHeadToCheck();

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoData(pScreen);

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
    }

    _drawColorScreen(pScreen);
    _drawObjectEdge(pScreen, pObject);
    displayScreen(pScreen);

    free(pObject);
    destroyScreen(pScreen);

    return millimeters;
}
