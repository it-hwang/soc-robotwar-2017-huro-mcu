//#define DEBUG

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "blue_gate.h"
#include "matrix.h"
#include "color.h"
#include "color_model.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "line_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "white_balance.h"
#include "boundary.h"
#include "log.h"
#include "debug.h"

#define LIMIT_TRY_COUNT 2
#define RIGHT_ZERO_GRADIENT 4
#define LEFT_ZERO_GRADIENT -8
#define HEAD_DIRECTION_ERROR -1
#define HEAD_DIRECTION_RIGHT 0
#define HEAD_DIRECTION_LEFT 1

static bool _approchBlueGate(void);
static void _establishBoundary(Screen_t* pScreen);
static Object_t* _searchBlueGate(Screen_t* pScreen);
static Matrix8_t* _createBlueMatrix(Screen_t* pScreen);
static void _setHeadRight(void);
static void _setHeadLeft(void);
static void _setHeadForward(void);
static void _setStandardStand(void);
static void _setHeadRightForGradient(void);
static void _setHeadLeftForGradient(void);
static void _setHeadForGradient(int headDirection);
static void _setHead(int horizontalDegrees, int verticalDegrees);
static bool _solveBluegate(void);
static bool _balanceToSolveBlueGate(void);
static bool _arrangeAngle(int headDirection, bool doHeadSet);
static Line_t* _captureLine(Screen_t* pScreen, int headDirection);
static Line_t* _captureRightLine(Screen_t* pScreen);
static Line_t* _captureLeftLine(Screen_t* pScreen);
static void _drawLine(Screen_t* pScreen, Line_t* pLine, int minX, int minY);
static int _getZeroGradient(int headDirection);
static void _moveForSetGradient(int lineGradient);
static bool _arrangeDistanceBalance(void);
static Object_t* _captureBlueGate(void);
static void _passThroughBlueGate(void);

bool blueGateMain(void) {
   /*  for (int i = 0; i < 100; ++i) {
        //int millimeters = measureRightBlueGateDistance();
        int millimeters = measureLeftBlueGateDistance();

        char input;
        input = getchar();
        while (input != '\n')
            input = getchar();
    }
    return true; */

    if( !_approchBlueGate() ) {
        return false;
    }

    return _solveBluegate();
}

static bool _approchBlueGate(void) {
    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 150;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 50;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {    
        // 앞뒤 정렬
        int rightDistance = measureRightBlueGateDistance();
        int leftDistance = measureLeftBlueGateDistance();

        int distance = (rightDistance + leftDistance) / 2;

        bool hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance <= APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printDebug("접근 완료.\n");
            break;
        }
        else {
            printDebug("전진보행으로 이동하자. (거리: %d)\n", distance);
            _setStandardStand();
            walkForward(distance - APPROACH_DISTANCE);
            mdelay(500);
            nTries = 0;
        }
    }

    if (nTries >= MAX_TRIES) {
        printDebug("시간 초과!\n");
        return false;
    }
    
    return true;
}

int measureRightBlueGateDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    //static const int HEAD_HORIZONTAL_DEGREES = 0;
    //static const int HEAD_VERTICAL_DEGREES = -50;

    _setHeadRight();

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);

    _establishBoundary(pScreen);

    int millimeters = 0;

    Object_t* pObject = _searchBlueGate(pScreen);
    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    if (pObject != NULL) {
        printDebug("minX: %d, centerX: %f, maxX: %d minY: %d, centerY: %f, maxY: %d\n",
        pObject->minX, pObject->centerX, pObject->maxX, pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pObject->maxY;
        
        millimeters = 0.0357*distance*distance - 11.558*distance + 849.66;
        // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
        if (millimeters <= 0)
            millimeters = 1;
    }

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    //printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}

int measureLeftBlueGateDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    //static const int HEAD_HORIZONTAL_DEGREES = 0;
    //static const int HEAD_VERTICAL_DEGREES = -50;

    _setHeadLeft();

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);
    
    _establishBoundary(pScreen);

    int millimeters = 0;

    Object_t* pObject = _searchBlueGate(pScreen);
    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    if (pObject != NULL) {
        printDebug("minX: %d, centerX: %f, maxX: %d minY: %d, centerY: %f, maxY: %d\n",
                 pObject->minX, pObject->centerX, pObject->maxX, pObject->minY, pObject->centerY, pObject->maxY);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pObject->maxY;
        
        millimeters = 0.0502*distance*distance - 12.867*distance + 842.13;
        // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
        if (millimeters <= 0)
            millimeters = 1;
    }

    if (pObject != NULL)
        free(pObject);
    destroyScreen(pScreen);

    //printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}

static void _establishBoundary(Screen_t* pScreen) {
    
    Matrix8_t* pBlueColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);
    Matrix8_t* pWhiteColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);

    Matrix8_t* pMergedColorMatrix = 
    overlapColorMatrix(pBlueColorMatrix, pWhiteColorMatrix);

    applyFastErosionToMatrix8(pMergedColorMatrix, 1);
    applyFastDilationToMatrix8(pMergedColorMatrix, 2);
    applyFastErosionToMatrix8(pMergedColorMatrix, 1);
    
    Matrix8_t* pBoundaryMatrix = establishBoundary(pMergedColorMatrix);

    applyBoundary(pScreen, pBoundaryMatrix);

    destroyMatrix8(pWhiteColorMatrix);
    destroyMatrix8(pBlueColorMatrix);
    destroyMatrix8(pMergedColorMatrix);
    destroyMatrix8(pBoundaryMatrix);
}

static Object_t* _searchBlueGate(Screen_t* pScreen) {

    Matrix8_t* pBlueMatrix = _createBlueMatrix(pScreen);

    ObjectList_t* pObjectList = detectObjectsLocation(pBlueMatrix);

    if(pObjectList == NULL) {
        destroyMatrix8(pBlueMatrix);
        return NULL;
    }

    Object_t* pLargestObject = NULL;
    for(int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);

        if(pLargestObject == NULL)
            pLargestObject = pObject;
        else if(pObject->cnt >= pLargestObject->cnt){
            pLargestObject = pObject;
        }
    }

    Object_t* pBlueGate = NULL;
    if(pLargestObject != NULL) {
        pBlueGate = (Object_t*)malloc(sizeof(Object_t));
        memcpy(pBlueGate, pLargestObject, sizeof(Object_t));
    }

    destroyMatrix8(pBlueMatrix);
    destroyObjectList(pObjectList);

    return pBlueGate;
}

static Matrix8_t* _createBlueMatrix(Screen_t* pScreen) {

    Matrix8_t* pBlueMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);

     // 잡음을 제거한다.
    applyFastErosionToMatrix8(pBlueMatrix, 1);
    applyFastDilationToMatrix8(pBlueMatrix, 2);
    applyFastErosionToMatrix8(pBlueMatrix, 1);

    return pBlueMatrix;
}

static void _setHeadRight(void) {
    _setHead(35, -40);
}

static void _setHeadLeft(void) {
    _setHead(-35, -40);
}

static void _setHeadForward(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    resetServoSpeed();
    _setHead(0, -40);
}

static void _setStandardStand(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    resetServoSpeed();
    _setHead(0, 0);
}

static void _setHeadRightForGradient(void) {
    setServoSpeed(30);
    runMotion(MOTION_CHECK_SIDELINE_STANCE);
    resetServoSpeed();
    _setHead(85, -50);
}

static void _setHeadLeftForGradient(void) {
    setServoSpeed(30);
    runMotion(MOTION_CHECK_SIDELINE_STANCE);
    resetServoSpeed();
    _setHead(-85, -50);
}

static void _setHeadForGradient(int headDirection) {
    if(headDirection == HEAD_DIRECTION_RIGHT)
        _setHeadRightForGradient();
    else if(headDirection == HEAD_DIRECTION_LEFT)
        _setHeadLeftForGradient();
    else
        printDebug("잘못된 매개 변수 값!(%d)\n", headDirection);
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
    mdelay(1000);
}

static bool _solveBluegate(void) {

    if( _balanceToSolveBlueGate() ) {
        _passThroughBlueGate();
    }

    return true;
}

static bool _balanceToSolveBlueGate(void) {

    if( !_arrangeAngle(HEAD_DIRECTION_RIGHT, true) )
        _arrangeAngle(HEAD_DIRECTION_LEFT, true);

    if( !_arrangeDistanceBalance() ) {
        return false;
    }

    _setStandardStand();

    return true;
}

static bool _arrangeAngle(int headDirection, bool doHeadSet) {
    static const int RANGE_OF_GRADIENT = 5;
    static const int LIMIT_TRY = 10;

    if(headDirection == HEAD_DIRECTION_ERROR)
        return false;

    if(doHeadSet)
        _setHeadForGradient(headDirection);

    Screen_t* pScreen = createDefaultScreen();
    Line_t* pLine = NULL;

    int zeroGradient = _getZeroGradient(headDirection);
    int lineGradient = 0;
    int tryCount = 0;

    do {
        pLine = _captureLine(pScreen, headDirection);

        if(pLine != NULL) {
            lineGradient = (int)pLine->theta - zeroGradient;
            tryCount = 0;

            if(abs(lineGradient) < RANGE_OF_GRADIENT)
                break;

            printDebug("중앙으로 부터 각도차(%d) 머리 방향(%d)\n", lineGradient, headDirection);
            _moveForSetGradient(lineGradient);
            
            free(pLine);
        } else {
            printDebug("선을 확인 하지 못하여 다시 찍는 중.\n");
            tryCount++;
            lineGradient = RANGE_OF_GRADIENT;
        }
    } while(tryCount < LIMIT_TRY);

    if(pLine != NULL)
        free(pLine);

    destroyScreen(pScreen);

    if(tryCount >= LIMIT_TRY) {
        printDebug("최대 촬영 횟수(%d) 초과!\n", tryCount);
        return false;
    } else {
        printDebug("각도 조절 완료. 머리 방향(%d)\n", headDirection);
        return true;
    }
    
}

static Line_t* _captureLine(Screen_t* pScreen, int headDirection) {
    if(headDirection == HEAD_DIRECTION_RIGHT) 
        return _captureRightLine(pScreen);
    else if(headDirection == HEAD_DIRECTION_LEFT)
        return _captureLeftLine(pScreen);
    else {
        printDebug("잘못된 매개 변수 값!(%d)\n", headDirection);
        return NULL;
    }
}

static Line_t* _captureRightLine(Screen_t* pScreen) {
    
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 70, 0, 89, 110);

    Matrix8_t* pColorMatrix = createColorMatrix(pSubMatrix, 
                                pColorTables[COLOR_BLACK]);

    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);

    Line_t* returnLine = lineDetection(pColorMatrix);

    drawColorMatrix(pSubMatrix, pColorMatrix);
    overlapMatrix16(pSubMatrix, pScreen, 70, 0);

    _drawLine(pScreen, returnLine, 70, 0);
    displayScreen(pScreen);

    destroyMatrix8(pColorMatrix);
    destroyMatrix16(pSubMatrix);

    return returnLine;
}

static Line_t* _captureLeftLine(Screen_t* pScreen) {

    readFpgaVideoDataWithWhiteBalance(pScreen);

    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 90, 0, 109, 110);

    Matrix8_t* pColorMatrix = createColorMatrix(pSubMatrix, 
                                pColorTables[COLOR_BLACK]);

    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);

    Line_t* returnLine = lineDetection(pColorMatrix);

    drawColorMatrix(pSubMatrix, pColorMatrix);
    overlapMatrix16(pSubMatrix, pScreen, 90, 0);

    _drawLine(pScreen, returnLine, 90, 0);
    displayScreen(pScreen);

    destroyMatrix8(pColorMatrix);
    destroyMatrix16(pSubMatrix);

    return returnLine;
}

static void _drawLine(Screen_t* pScreen, Line_t* pLine, int minX, int minY) {
    
    PixelData_t* pixels = pScreen->elements;

    
    int centerX = (int)pLine->centerPoint.x + minX;

    for(int x = minX; x <= centerX; ++x) {
        int y = (int)pLine->leftPoint.y + minY;
        int index = y * pScreen->width + x;
        uint16_t* pOutput = (uint16_t*)&pixels[index];
        *pOutput = 0xF800;
    }

    for(int x = minX + pLine->rightPoint.x; x >= centerX; --x) {
        int y = (int)pLine->rightPoint.y + minY;
        int index = y * pScreen->width + x;
        uint16_t* pOutput = (uint16_t*)&pixels[index];
        *pOutput = 0xF800;
    }
}

static int _getZeroGradient(int headDirection) {
    if(headDirection == HEAD_DIRECTION_RIGHT)
        return RIGHT_ZERO_GRADIENT;
    else
        return LEFT_ZERO_GRADIENT;
}

static void _moveForSetGradient(int lineGradient) {
    if(lineGradient < 0) {
        printDebug("왼쪽으로 회전. 기울기(%d)\n", lineGradient);
        turnLeft(lineGradient * -1);
    } else {
        printDebug("오른쪽으로 회전. 기울기(%d)\n", lineGradient);
        turnRight(lineGradient);
    }
}

static bool _arrangeDistanceBalance(void) {
    
    int rightTryCount = 0;
    int leftTryCount = 0;

    bool right = false;
    bool left = false;
    bool ing = false;

    while( rightTryCount < LIMIT_TRY_COUNT && leftTryCount < LIMIT_TRY_COUNT) {

        if(!ing) {
            _setHeadRight();
            Object_t* rightBlueGate = _captureBlueGate();
        
            if(rightBlueGate != NULL) {
                rightTryCount = 0;

                int rightDistance = rightBlueGate->minX;
                if(rightDistance >= 100)
                    right = true;
                else {
                    walkLeft(5);
                    right = false;
                    left = false;
                    continue;
                }
            } else 
                rightTryCount++;
        }
        
        _setHeadLeft();
        Object_t* leftBlueGate = _captureBlueGate();

        if(leftBlueGate != NULL) {
            leftTryCount = 0;
            int leftDistance = leftBlueGate->maxX;
            if(leftDistance <= 80) {
                left = true;
                ing = false;
            }
            else {
                walkRight(5);
                right = false;
                left = false;
                ing = true;
                continue;
            }
        } else {
            leftTryCount++;
            ing = false;
        }
            

        if(right && left)
            break;
    }
    
    if(rightTryCount >= LIMIT_TRY_COUNT || leftTryCount >= LIMIT_TRY_COUNT)
        return false;
    else
        return true;
}

static Object_t* _captureBlueGate(void) {

    Screen_t* pScreen = createDefaultScreen();

    readFpgaVideoDataWithWhiteBalance(pScreen);

    _establishBoundary(pScreen);

    Object_t* pObject = _searchBlueGate(pScreen);

    drawObjectEdge(pScreen, pObject, NULL);
    drawObjectCenter(pScreen, pObject, NULL);
    displayScreen(pScreen);

    destroyScreen(pScreen);

    return pObject;
}

static void _passThroughBlueGate(void) {
    
    _setStandardStand();

    walkForward(100);
}
