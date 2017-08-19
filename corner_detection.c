#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "detection_corner.h"
#include "log.h"

#define CAPTURE_ERROR -1
#define RIGHT_SIDE_CLEAR 0
#define LEFT_SIDE_CLEAR 1
#define NO_CLEAR_SIDE 2

#define FIT_FRONT_DISTANCE 50

Screen_t* _pDefaultScreen;

bool cornerDetection(void) {
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

    _setHeadLeft();
    Line_t* leftLine = _captureLeftLine(pScreen);

    _setHeadRight();
    Line_t* rightLine = _captureRightLine(pScreen);

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

    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 45, 0, 64, 110);

    Matrix8_t* pColorMatrix = createColorMatrix(pSubMatrix, 
                                pColorTables[COLOR_BLACK]);

    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 1);

    Line_t* returnLine = lineDetection(pColorMatrix);

    drawColorMatrix(pSubMatrix, pColorMatrix);
    overlapMatrix16(pSubMatrix, pScreen, 45, 0);

    _drawLine(pScreen, returnLine, 45, 0);
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
    static const int RANGE_OF_DISTANCE = 5;
    static const int LIMIT_TRY_COUNT = 10;

    _setHeadForward();

    Screen_t* pScreen = createDefaultScreen();

    

}