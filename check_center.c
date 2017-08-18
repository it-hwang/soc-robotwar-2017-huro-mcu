#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "check_center.h"
#include "object_detection.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "log.h"
#include "white_balance.h"

#define CENTER 80
#define RIGHT_ZERO_GRADIENT 2
#define LEFT_ZERO_GRADIENT -5
#define HEAD_DIRECTION_ERROR -1
#define HEAD_DIRECTION_RIGHT 0
#define HEAD_DIRECTION_LEFT 1

bool checkCenterMain(void) {
    static const char* LOG_FUNCTION_NAME = "checkCenterMain()";

    int headDirection = _searchLine();
    bool doHeadSet = false;

    if( headDirection < 0 ) {
        printLog("[%s] 라인을 찾을 수 없다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if( !_approachLine(headDirection, doHeadSet) ) {
        printLog("[%s] 선에 접근 할 수 없다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if( !_arrangeAngle(headDirection, doHeadSet) ) {
        printLog("[%s] 각도를 정렬에 실패했다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    _setStandardStand();

    return true;
}

static int _searchLine() {
    static const char* LOG_FUNCTION_NAME = "_searchLine()";
    static const int LIMIT_TRY_COUNT = 6;

    Screen_t* pScreen = createDefaultScreen();
    Line_t* pLine = NULL;

    int tryCount = 0;
    int resultDirection = HEAD_DIRECTION_ERROR;

    while( tryCount < LIMIT_TRY_COUNT && pLine == NULL) {
        
        if(resultDirection != HEAD_DIRECTION_RIGHT) {
            printLog("[%s] 오른쪽 선을 확인 중.\n", LOG_FUNCTION_NAME);
            resultDirection = HEAD_DIRECTION_RIGHT;
            _setHeadRight();
            pLine = _captureRightLine(pScreen);
        } else {
            printLog("[%s] 왼쪽 선을 확인 중.\n", LOG_FUNCTION_NAME);
            resultDirection = HEAD_DIRECTION_LEFT;
            _setHeadLeft();
            pLine = _captureLeftLine(pScreen);
        }

        if(pLine == NULL)
            tryCount++;
    }

    if(tryCount >= LIMIT_TRY_COUNT) {
        printLog("[%s] 좌우 최대 촬영 횟수(%d) 초과!\n", LOG_FUNCTION_NAME, tryCount);
        resultDirection = HEAD_DIRECTION_ERROR;
    }

    if(pLine != NULL)
        free(pLine);

    destroyScreen(pScreen);

    return resultDirection;
}

static void _setHeadRight() {
    setServoSpeed(30);
    runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(85, -50);
    mdelay(500);
    resetServoSpeed();
}

static void _setHeadLeft() {
    setServoSpeed(30);
    runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(-85, -50);
    mdelay(500);
    resetServoSpeed();
}

static void _setStandardStand() {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    setHead(0, 0);
    mdelay(500);
    resetServoSpeed();
}

static void _setHead(int headDirection) {
    static const char* LOG_FUNCTION_NAME = "_setHead()";

    if(headDirection == HEAD_DIRECTION_RIGHT)
        _setHeadRight();
    else if(headDirection == HEAD_DIRECTION_LEFT)
        _setHeadLeft();
    else
        printLog("[%s] 잘못된 매개 변수 값!(%d)\n", LOG_FUNCTION_NAME, headDirection);
}

static Line_t* _captureRightLine(Screen_t* pScreen) {
        
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 10, 0, 59, 95);

    Matrix8_t* pColorMatrix = createColorMatrix(pSubMatrix, 
                                pColorTables[COLOR_BLACK]);

    applyDilationToMatrix8(pColorMatrix, 1);
    applyErosionToMatrix8(pColorMatrix, 2);
    applyDilationToMatrix8(pColorMatrix, 1);
    
    Line_t* returnLine = lineDetection(pColorMatrix);
    
    drawColorMatrix(pSubMatrix, pColorMatrix);
    overlapMatrix16(pSubMatrix, pScreen, 10, 0);
    
    _drawLine(pScreen, returnLine, 10, 0);

    destroyMatrix8(pColorMatrix);
    destroyMatrix16(pSubMatrix);

    return returnLine;
}

static Line_t* _captureLeftLine(Screen_t* pScreen) {
    
    readFpgaVideoDataWithWhiteBalance(pScreen);
    
    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 120, 0, 169, 95);

    Matrix8_t* pColorMatrix = createColorMatrix(pSubMatrix, 
                                pColorTables[COLOR_BLACK]);

    applyDilationToMatrix8(pColorMatrix, 1);
    applyErosionToMatrix8(pColorMatrix, 2);
    applyDilationToMatrix8(pColorMatrix, 1);
    
    Line_t* returnLine = lineDetection(pColorMatrix);
    
    drawColorMatrix(pSubMatrix, pColorMatrix);
    overlapMatrix16(pSubMatrix, pScreen, 120, 0);
    
    _drawLine(pScreen, returnLine, 120, 0);

    destroyMatrix8(pColorMatrix);
    destroyMatrix16(pSubMatrix);
    
    return returnLine;
}

static void _drawLine(Screen_t* pScreen, Line_t* pLine, int minX, int minY) {
    static const int LINE_WIDTH = (int)pLine->leftPoint.x - pLine->rigthPoint.x + 1;

    PixelData_t* pixels = pScreen->elements;

    int centerX = (int)pLine->centerPoint.x + minX;
    int centerY = (int)pLine->centerPoint.y + minY;

    for(int x = minX; x <= centerX; ++x) {
        int y = (int)pLine->leftPoint.y;
        int index = y * pScreen->width + x;
        uint16_t* pOutput = (uint16_t*)&pixels[index];
        *pOutput = 0xF800;
    }

    for(int x = minX + LINE_WIDTH - 1; x >= centerX; --x) {
        int y = (int)pLine->rightPoint.y;
        int index = y * pScreen->width + x;
        uint16_t* pOutput = (uint16_t*)&pixels[index];
        *pOutput = 0xF800;
    }
}

static Line_t* _captureLine(Screen_t* pScreen, int headDirection) {
    static const char* LOG_FUNCTION_NAME = "_captureLine()";

    if(headDirection == HEAD_DIRECTION_RIGHT) 
        return _captureRightLine(pScreen);
    else if(headDirection == HEAD_DIRECTION_LEFT)
        return _captureLeftLine(pScreen);
    else {
        printLog("[%s] 잘못된 매개 변수 값!(%d)\n", LOG_FUNCTION_NAME, headDirection);
        return NULL;
    }
        
}

static bool _approachLine(int headDirection, bool doHeadSet) {
    static const char* LOG_FUNCTION_NAME = "_approachLine()";
    static const int RANGE_OF_DISTANCE = 5;
    static const int LIMIT_TRY_COUNT = 5;

    if(headDirection == HEAD_DIRECTION_ERROR)
        return false;

    if(doHeadSet)
        _setHead(headDirection);

    Screen_t* pScreen = createDefaultScreen();
    Line_t* pLine = NULL;

    int lineDistanceFromRobot = 0;
    int tryCount = 0;

    do {
        pLine = _captureLine(pScreen, headDirection);

        if(pLine != NULL) {
            lineDistanceFromRobot = CENTER - pLine->centerPoint.y;
            tryCount = 0;
            _moveForSetDistance(lineDistanceFromRobot, headDirection);
            printLog("[%s] 중앙으로 부터 거리차(%d) 머리 방향(%d)\n", LOG_FUNCTION_NAME, lineDistanceFromRobot, headDirection);
            free(pLine);
        } else {
            tryCount++;
            lineDistanceFromRobot = RANGE_OF_DISTANCE;
        }
    } while(abs(lineDistanceFromRobot) >= RANGE_OF_DISTANCE && tryCount < LIMIT_TRY_COUNT);

    if(pLine != NULL)
        free(pLine);

    destroyScreen(pScreen);

    if(tryCount >= LIMIT_TRY_COUNT){
        printLog("[%s] 최대 촬영 횟수(%d) 초과!\n", LOG_FUNCTION_NAME, tryCount);
        return false;
    } else
        return true;
}

static void _moveForSetDistance(int lineDistanceFromRobot, int headDirection) {
    static const char* LOG_FUNCTION_NAME = "_moveForSetDistance()";

    if( lineDistanceFromRobot < 0) {
        printLog("[%s] 다른 방향으로 이동. 머리방향(%d)\n", LOG_FUNCTION_NAME, headDirection);
        _walkDifferentDirection(headDirection);
    } else {
        printLog("[%s] 같은 방향으로 이동. 머리방향(%d)\n", LOG_FUNCTION_NAME, headDirection);
        _walkSameDirection(headDirection);
    }
}

static void _walkDifferentDirection(int headDirection) {
    static const char* LOG_FUNCTION_NAME = "_walkDifferentDirection()";

    if(headDirection == HEAD_DIRECTION_RIGHT)
        walkLeft(5);
    else if( headDirection == HEAD_DIRECTION_LEFT)
        walkRight(5);
    else
        printLog("[%s] 잘못된 매개 변수 값!(%d)\n", LOG_FUNCTION_NAME, headDirection);
}

static void _walkSameDirection(int headDirection) {
    static const char* LOG_FUNCTION_NAME = "_walkSameDirection()";

    if(headDirection == HEAD_DIRECTION_RIGHT)
        walkRight(5);
    else if( headDirection == HEAD_DIRECTION_LEFT)
        walkLeft(5);
    else 
        printLog("[%s] 잘못된 매개 변수 값!(%d)\n", LOG_FUNCTION_NAME, headDirection);
}

static bool _arrangeAngle(int headDirection, bool doHeadSet) {
    static const char* LOG_FUNCTION_NAME = "_arrangeAngle()";
    static const int RANGE_OF_GRADIENT = 10;
    static const int LIMIT_TRY_COUNT = 5;

    if(headDirection == HEAD_DIRECTION_ERROR)
        return false;

    if(doHeadSet)
        _setHead(headDirection);

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
            _moveForSetGradient(lineGradient);
            printLog("[%s] 중앙으로 부터 거리차(%d) 머리 방향(%d)\n", LOG_FUNCTION_NAME, lineGradient, headDirection);
            free(pLine);
        } else {
            tryCount++;
            lineGradient = RANGE_OF_GRADIENT;
        }
    } while(abs(lineGradient) >= RANGE_OF_GRADIENT && tryCount < LIMIT_TRY_COUNT);

    if(pLine != NULL)
        free(pLine);

    destroyScreen(pScreen);

    if(tryCount >= LIMIT_TRY_COUNT) {
        return false;
    } else {
        return true;
    }
    
}

static int _getZeroGradient(int headDirection) {
    if(headDirection == HEAD_DIRECTION_RIGHT)
        return RIGHT_ZERO_GRADIENT;
    else
        return LEFT_ZERO_GRADIENT;
}

static void _moveForSetGradient(int lineGradient) {
    static const char* LOG_FUNCTION_NAME = "_moveForSetGradient()";

    if(lineGradient < 0) {
        printLog("[%s] 왼쪽으로 회전. 기울기(%d)\n", LOG_FUNCTION_NAME, lineGradient);
        turnLeft(lineGradient);
    } else {
        printLog("[%s] 오른쪽으로 회전. 기울기(%d)\n", LOG_FUNCTION_NAME, lineGradient);
        turnRigth(lineGradient);
    }
}