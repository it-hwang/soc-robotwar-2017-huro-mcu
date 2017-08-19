#include <math.h>
#include <stdlib.h>

#include "mine.h"
#include "white_balance.h"
#include "log.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "color.h"
#include "image_filter.h"
#include "object_detection.h"


// 지뢰를 확인할 화면 크기
static const int _SUB_MATRIX_WIDTH  = 100;
static const int _SUB_MATRIX_HEIGHT = 106;


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

static Screen_t* _createMineScreen(Screen_t* pScreen) {
    static const char* LOG_FUNCTION_NAME = "_createMineScreen()";
    
    int centerX = pScreen->width * 0.5;
    int minX = centerX - _SUB_MATRIX_WIDTH * 0.5;
    int maxX = centerX + _SUB_MATRIX_WIDTH * 0.5 - 1;
    int minY = 0;
    int maxY = _SUB_MATRIX_HEIGHT;
    Screen_t* pSubScreen = createSubMatrix16(pScreen, minX, minY, maxX, maxY);
    
    printLog("[%s] minX: %d, minY: %d, maxX: %d, maxY: %d, width: %d, height: %d\n",
             LOG_FUNCTION_NAME, minX, minY, maxX, maxY, pSubScreen->width, pSubScreen->height);

    return pSubScreen;
}

static void _drawMineScreen(Screen_t* pScreen, Screen_t* pSubScreen) {
    int centerX = pScreen->width * 0.5;
    int minX = centerX - _SUB_MATRIX_WIDTH * 0.5;
    int minY = 0;

    overlapMatrix16(pSubScreen, pScreen, minX, minY);
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

static void _addColorMatrix(Screen_t* pScreen, Matrix8_t* pColorMatrix) {
    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    PixelData_t* pScreenPixel = pScreen->elements;
    Color_t* pColorPixel = pColorMatrix->elements;

    for (int i = 0; i < length; ++i) {
        if (*pColorPixel != COLOR_NONE)
            *pScreenPixel = colorToRgab5515Data(*pColorPixel);
        pScreenPixel++;
        pColorPixel++;
    }
}

static void _displayDebugScreen(Screen_t* pScreen, Object_t* pMine, Object_t* pOtherObstacle) {
    Screen_t* pDebugScreen = cloneMatrix16(pScreen);
    Screen_t* pSubScreen = _createMineScreen(pDebugScreen);
    
    Matrix8_t* pWhiteMatrix = _createWhiteMatrix(pSubScreen);
    Matrix8_t* pBlackMatrix = _createBlackMatrix(pSubScreen);
    
    drawColorMatrix(pSubScreen, pWhiteMatrix);
    _addColorMatrix(pSubScreen, pBlackMatrix);
    _drawMineScreen(pDebugScreen, pSubScreen);
    displayScreen(pDebugScreen);

    destroyMatrix8(pWhiteMatrix);
    destroyMatrix8(pBlackMatrix);
    destroyScreen(pSubScreen);
    destroyScreen(pDebugScreen);
}

static bool _isMine(Screen_t* pScreen, Object_t* pObject) {
    bool tooFar = (pObject->minY < 3);
    bool tooClose = (pObject->minY >= pScreen->height - 3);

    int objectWidth = pObject->maxX - pObject->minX + 1;
    bool tooWide = (objectWidth >= pScreen->width);

    return (!tooFar && !tooClose && !tooWide);
}

static Object_t* _searchClosestMine(Screen_t* pScreen) {
    static const char* LOG_FUNCTION_NAME = "_searchClosestMine()";

    Matrix16_t* pSubScreen = _createMineScreen(pScreen);
    Matrix8_t* pBlackMatrix = _createBlackMatrix(pSubScreen);
    ObjectList_t* pObjectList = detectObjectsLocation(pBlackMatrix);

    // 지뢰 오브젝트 중에 가장 가까이 있는 오브젝트를 찾는다.
    Object_t* pClosestObject = NULL;
    int closestMaxY = 0;
    if (pObjectList != NULL) {
        for (int i = 0; i < pObjectList->size; ++i) {
            Object_t* pObject = &(pObjectList->list[i]);

            if (!_isMine(pSubScreen, pObject))
                continue;
            
            if (pObject->maxY > closestMaxY) {
                pClosestObject = pObject;
                closestMaxY = pObject->maxY;
            }
        }
    }

    Object_t* pClosestMine = NULL;
    if (pClosestObject != NULL) {
        pClosestMine = (Object_t*)malloc(sizeof(Object_t));
        memcpy(pClosestMine, pClosestObject, sizeof(Object_t));
    }

    destroyObjectList(pObjectList);
    destroyMatrix8(pBlackMatrix);
    destroyMatrix16(pSubScreen);

    return pClosestMine;
}

static int _measureMineDistance(Screen_t* pScreen) {
    static const char* LOG_FUNCTION_NAME = "_measureMineDistance()";

    Object_t* pObject = _searchClosestMine(pScreen);
    
    int millimeters = 0;
    if (pObject != NULL) {
        printLog("[%s] minX: %d, centerX: %f, maxX: %d\n", LOG_FUNCTION_NAME,
                 pObject->minX, pObject->centerX, pObject->maxX);
        printLog("[%s] minY: %d, centerY: %f, maxY: %d\n", LOG_FUNCTION_NAME,
                pObject->minY, pObject->centerY, pObject->maxY);

        int maxY = pObject->maxY;
        millimeters = -2.6954 * maxY + 292.29;
        
        if (millimeters <= 0)
            millimeters = 1;
    }

    if (pObject != NULL)
        free(pObject);

    printLog("[%s] millimeters: %d\n", LOG_FUNCTION_NAME, millimeters);
    return millimeters;
}

static bool _measureMineOffsetX(Screen_t* pScreen, int* pOffsetX) {
    static const char* LOG_FUNCTION_NAME = "_measureMineOffsetX()";

    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -35;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Object_t* pObject = _searchClosestMine(pScreen);
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
    
    if (pOffsetX != NULL)
        *pOffsetX = offsetX;

    printLog("[%s] offsetX: %d\n", LOG_FUNCTION_NAME, offsetX);
    return hasFound;
}

int measureMineDistance(void) {
    static const char* LOG_FUNCTION_NAME = "measureMineDistance()";

    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -70;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    int distance = _measureMineDistance(pScreen);
    Object_t* pMineObject = _searchClosestMine(pScreen);
    _displayDebugScreen(pScreen, pMineObject, NULL);
    
    if (pMineObject != NULL)
        free (pMineObject);
    destroyScreen(pScreen);

    return distance;
}

static int _measureOtherObstacleDistance(void) {
    static const char* LOG_FUNCTION_NAME = "measureOtherObstacleDistance()";

    // 거리 측정에 사용되는 머리 각도
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -70;

    _setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Object_t* pMineObject = _searchClosestMine(pScreen);

    destroyScreen(pScreen);
    
    return 0;
}


bool mineMain(void) {
    // for (int i = 0; i < 100; ++i) {
    //     int millimeters = measureMineDistance();

    //     char input;
    //     input = getchar();
    //     while (input != '\n')
    //         input = getchar();
    // }
    // return true;
    solveMine();

    return false;
}


static bool _approachMine(void) {
    static const char* LOG_FUNCTION_NAME = "_approachMine()";

    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;

    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 20;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 30;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        // 앞뒤 정렬
        int distance = measureMineDistance();
        bool hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance <= APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printLog("[%s] 접근 완료.\n", LOG_FUNCTION_NAME);
            break;
        }
        else {
            printLog("[%s] 전진보행으로 이동하자. (거리: %d)\n", LOG_FUNCTION_NAME, distance);
            walkForward(distance - APPROACH_DISTANCE);
            mdelay(500);
            nTries = 0;
        }
    }
    if (nTries >= MAX_TRIES) {
        printLog("[%s] 시간 초과!\n", LOG_FUNCTION_NAME);
        return false;
    }
    
    return true;
}


bool solveMine(void) {
    _approachMine();

    return false;
}
