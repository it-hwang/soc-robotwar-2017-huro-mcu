#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "corner_detection.h"
#include "log.h"

#define CAPTURE_ERROR -1
#define RIGHT_SIDE_CLEAR 0
#define LEFT_SIDE_CLEAR 1
#define NO_CLEAR_SIDE 2

static int _lookAround();
static int _captureBothSide(void);
static void _setHeadRight(void);
static void _setHeadLeft(void);
static void _setHeadForward(void);
static void _setStandardStand(void);
static Line_t* _captureRightLine(Screen_t* pScreen);
static Line_t* _captureLeftLine(Screen_t* pScreen);
static Line_t* _captureForwardLine(Screen_t* pScreen);
static void _drawLine(Screen_t* pScreen, Line_t* pLine, int minX, int minY);
static bool _moveUntilSeeLine();
static void _moveToDestination(int turnWhere);

bool cornerDetectionMain(void) {
    static const char* LOG_FUNCTION_NAME = "cornerDetection()";

    int turnWhere = _lookAround();

    _setHeadForward();

    if(turnWhere < 0) {
        printLog("[%s] 라인을 찾을 수 없습니다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if(turnWhere == NO_CLEAR_SIDE) {
        printLog("[%s] 이건 코너가 아닙니다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if( !_moveUntilSeeLine() ) {
        printLog("[%s] 선에 접근할 수 없습니다.\n", LOG_FUNCTION_NAME);
        return false;
    } 
    
    checkCenterMain();

    _moveToDestination(turnWhere);

    return true;
}

static int _lookAround() {
    static const int INIT_STATE = -1;
    static const int LIMIT_TRY_COUNT = 2;

    int resultState = INIT_STATE;
    int tryCount = 0;
    
    while(tryCount < LIMIT_TRY_COUNT) {
        resultState = _captureBothSide();

        if(resultState == CAPTURE_ERROR)
            tryCount++;
        else 
            break;
    }

    return resultState;
}

static int _captureBothSide(void) {
    static const char* LOG_FUNCTION_NAME = "_captureBothSide()";

    Screen_t* pScreen = createDefaultScreen();
    
    _setHeadRight();
    Line_t* rightLine = _captureRightLine(pScreen);

    _setHeadLeft();
    Line_t* leftLine = _captureLeftLine(pScreen);

    destroyScreen(pScreen);

    if(leftLine == NULL && rightLine != NULL) {
        free(rightLine);
        printLog("[%s] 오른쪽에서 선을 찾았습니다.\n", LOG_FUNCTION_NAME);
        return LEFT_SIDE_CLEAR;
    }

    if(leftLine != NULL && rightLine == NULL) {
        free(leftLine);
        printLog("[%s] 왼쪽에서 선을 찾았습니다.\n", LOG_FUNCTION_NAME);
        return RIGHT_SIDE_CLEAR;
    }

    if(leftLine != NULL && rightLine != NULL) {
        free(leftLine);
        free(rightLine);
        printLog("[%s] 양쪽에서 선을 찾았습니다.\n", LOG_FUNCTION_NAME);
        return NO_CLEAR_SIDE;
    }

    return CAPTURE_ERROR;  
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

static void _setHeadForward(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    setHead(0, -50);
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

static Line_t* _captureForwardLine(Screen_t* pScreen) {
    
    readFpgaVideoDataWithWhiteBalance(pScreen);

    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 75, 0, 104, 119);

    Matrix8_t* pColorMatrix = createColorMatrix(pSubMatrix, 
                                pColorTables[COLOR_BLACK]);

    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);

    Line_t* returnLine = lineDetection(pColorMatrix);

    drawColorMatrix(pSubMatrix, pColorMatrix);
    overlapMatrix16(pSubMatrix, pScreen, 75, 0);

    _drawLine(pScreen, returnLine, 75, 0);
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

static bool _moveUntilSeeLine() {
    static const char* LOG_FUNCTION_NAME = "_moveUntilSeeLine()";
    
    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 250;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 50;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {    
        // 앞뒤 정렬
        int distance = measureFrontLineDistance();
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

int measureFrontLineDistance(void) {
    static const char* LOG_FUNCTION_NAME = "measureFrontLineDistance()";

    // 거리 측정에 사용되는 머리 각도
    //static const int HEAD_HORIZONTAL_DEGREES = 0;
    //static const int HEAD_VERTICAL_DEGREES = -50;

    _setHeadForward();

    Screen_t* pScreen = createDefaultScreen();

    int millimeters = 0;

    Line_t* pLine = _captureForwardLine(pScreen);

    if (pLine != NULL) {
        printLog("[%s] leftPointY: %d, centerPointY: %d, rigthPointY: %d\n", LOG_FUNCTION_NAME,
                 pLine->leftPoint.y, pLine->centerPoint.y, pLine->rightPoint.y);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pLine->centerPoint.y;
        
        millimeters = 655.21 * exp(-0.016 * distance);
        // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
        if (millimeters <= 0)
            millimeters = 1;
    }

    if (pLine != NULL)
        free(pLine);
    destroyScreen(pScreen);

    printLog("[%s] millimeters: %d\n", LOG_FUNCTION_NAME, millimeters);
    return millimeters;
}

static void _moveToDestination(int turnWhere) {
    if(turnWhere == RIGHT_SIDE_CLEAR)
        turnRight(90);
    else if(turnWhere == LEFT_SIDE_CLEAR)
        turnLeft(90);
}
