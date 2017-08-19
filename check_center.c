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

#define CENTER 50
#define RIGHT_ZERO_GRADIENT 4
#define LEFT_ZERO_GRADIENT -9
#define HEAD_DIRECTION_ERROR -1
#define HEAD_DIRECTION_RIGHT 0
#define HEAD_DIRECTION_LEFT 1

static int _searchLine(void);
static void _setHeadRight(void);
static void _setHeadLeft(void);
static void _setStandardStand(void);
static void _setHead(int headDirection);
static Line_t* _captureRightLine(Screen_t* pScreen);
static Line_t* _captureLeftLine(Screen_t* pScreen);
static void _drawLine(Screen_t* pScreen, Line_t* pLine, int minX, int minY);
static Line_t* _captureLine(Screen_t* pScreen, int headDirection);
static bool _approachLine(int headDirection, bool doHeadSet);
static void _moveForSetDistance(int lineDistanceFromRobot, int headDirection);
static void _walkDifferentDirection(int headDirection);
static void _walkSameDirection(int headDirection);
static bool _arrangeAngle(int headDirection, bool doHeadSet);
static int _getZeroGradient(int headDirection);
static void _moveForSetGradient(int lineGradient);

bool checkCenterMain(void) {
    static const char* LOG_FUNCTION_NAME = "checkCenterMain()";

    int headDirection = _searchLine();
    bool doHeadSet = false;

    if( headDirection < 0 ) {
        printLog("[%s] 라인을 찾을 수 없다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if( !_arrangeAngle(headDirection, doHeadSet) ) {
        printLog("[%s] 각도를 정렬에 실패했다.\n", LOG_FUNCTION_NAME);
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

    printLog("[%s] 중앙 정렬 완료.\n", LOG_FUNCTION_NAME);

    return true;
}

static int _searchLine(void) {
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

        if(pLine == NULL){
            tryCount++;
            printLog("[%s] 선을 확인 하지 못하여 다시 찍는 중.\n", LOG_FUNCTION_NAME);
        }
            
    }

    if(tryCount >= LIMIT_TRY_COUNT) {
        printLog("[%s] 좌우 최대 촬영 횟수(%d) 초과!\n", LOG_FUNCTION_NAME, tryCount);
        resultDirection = HEAD_DIRECTION_ERROR;
    } else
        printLog("[%s] 선 확인 완료. 머리 방향(%d)\n", LOG_FUNCTION_NAME, resultDirection);

    if(pLine != NULL)
        free(pLine);

    destroyScreen(pScreen);

    return resultDirection;
}

static void _setHeadRight(void) {
    setServoSpeed(30);
    runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(85, -50);
    mdelay(1000);
    resetServoSpeed();
}

static void _setHeadLeft(void) {
    setServoSpeed(30);
    runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(-85, -50);
    mdelay(1000);
    resetServoSpeed();
}

static void _setStandardStand(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    setHead(0, 0);
    mdelay(1000);
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

    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 70, 0, 89, 110);

    Matrix8_t* pColorMatrix = createColorMatrix(pSubMatrix, 
                                pColorTables[COLOR_BLACK]);
    /*
    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);
    */
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

    /*applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);
    */
    
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
    static const int LIMIT_TRY_COUNT = 10;

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

            if(abs(lineDistanceFromRobot) < RANGE_OF_DISTANCE)
                break;

            printLog("[%s] 중앙으로 부터 거리차(%d) 머리 방향(%d)\n", LOG_FUNCTION_NAME, lineDistanceFromRobot, headDirection);
            _moveForSetDistance(lineDistanceFromRobot, headDirection);
            
            free(pLine);
        } else {
            printLog("[%s] 선을 확인 하지 못하여 다시 찍는 중.\n", LOG_FUNCTION_NAME);
            tryCount++;
            lineDistanceFromRobot = RANGE_OF_DISTANCE;
        }
    } while(tryCount < LIMIT_TRY_COUNT);

    if(pLine != NULL)
        free(pLine);

    destroyScreen(pScreen);

    if(tryCount >= LIMIT_TRY_COUNT){
        printLog("[%s] 최대 촬영 횟수(%d) 초과!\n", LOG_FUNCTION_NAME, tryCount);
        return false;
    } else {
        printLog("[%s] 거리 조절 완료. 머리 방향(%d)\n", LOG_FUNCTION_NAME, headDirection);
        return true;
    }
        
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
    static const int RANGE_OF_GRADIENT = 5;
    static const int LIMIT_TRY_COUNT = 10;

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

            if(abs(lineGradient) < RANGE_OF_GRADIENT)
                break;

            printLog("[%s] 중앙으로 부터 각도차(%d) 머리 방향(%d)\n", LOG_FUNCTION_NAME, lineGradient, headDirection);
            _moveForSetGradient(lineGradient);
            
            free(pLine);
        } else {
            printLog("[%s] 선을 확인 하지 못하여 다시 찍는 중.\n", LOG_FUNCTION_NAME);
            tryCount++;
            lineGradient = RANGE_OF_GRADIENT;
        }
    } while(tryCount < LIMIT_TRY_COUNT);

    if(pLine != NULL)
        free(pLine);

    destroyScreen(pScreen);

    if(tryCount >= LIMIT_TRY_COUNT) {
        printLog("[%s] 최대 촬영 횟수(%d) 초과!\n", LOG_FUNCTION_NAME, tryCount);
        return false;
    } else {
        printLog("[%s] 각도 조절 완료. 머리 방향(%d)\n", LOG_FUNCTION_NAME, headDirection);
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
        turnRight(lineGradient);
    }
}
