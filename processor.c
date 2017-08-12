#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "processor.h"
#include "color.h"
#include "color_model.h"
#include "graphic_interface.h"
#include "obstacle_manager.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "line_detection.h"
#include "check_center.h"
#include "vertical_barricade.h"
#include "red_bridge.h"
#include "white_balance.h"
#include "log.h"

static ObstacleId_t* _obstacleSequence;

static Screen_t* _pDefaultScreen;


static void _improveSomeObstacle(void);

static void _adjustWhiteBalance(int r5, int g5, int b5);
static void _adjustWhiteBalanceAuto(void);

static void _defineObstacle(void) {
	//registerObstacle(OBSTACLE_ONE, helloWorld);
	//registerObstacle(OBSTACLE_TWO, goodbyeWorld);
}

static void _convertScreenToDisplay(Screen_t* pScreen) {
    int length = pScreen->height * pScreen->width;
    int i;
    for (i = 0; i < length; ++i) {
        Rgab5515_t* pRgab5515 = (Rgab5515_t*)&(pScreen->elements[i]);
        pRgab5515->a = pRgab5515->g;
    }
}

static void _applyColorMatrix(Screen_t* pScreen, Matrix8_t* pColorMatrix) {
    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    int i;
    PixelData_t* pScreenPixel = pScreen->elements;
    Color_t* pColorPixel = pColorMatrix->elements;

    for (i = 0; i < length; ++i) {
        *pScreenPixel = colorToRgab5515Data(*pColorPixel);
        //if (*pColorPixel)
        //    *pScreenPixel = 0x00ff;
        pScreenPixel++;
        pColorPixel++;
    }
}


int openProcessor(void) {
    if (openGraphicInterface() < 0) {
        closeProcessor();
        return PROCESSOR_GRAPHIC_ERROR;
    }
    if (openRobotPort() < 0) {
        closeProcessor();
        return PROCESSOR_ROBOT_PORT_ERROR;
    }
    initializeColor();
    
	_defineObstacle();
	_obstacleSequence = loadObstaclesFile("/mnt/f0/obstacles.txt");

    _pDefaultScreen = createDefaultScreen();

    return 0;
}

void closeProcessor(void) {
    closeGraphicInterface();
    closeRobotPort();
    destroyScreen(_pDefaultScreen);
    free(_obstacleSequence);
    finalizeColor();
    setDefaultWhiteBalanceTable(NULL);
}

int runProcessor(void) {
    _adjustWhiteBalanceAuto();

    for (int i = 0; i < 500; ++i) {
        readFpgaVideoData(_pDefaultScreen);
        applyDefaultWhiteBalance(_pDefaultScreen);
        displayScreen(_pDefaultScreen);
    }

    return 0;
}

void _improveSomeObstacle(void) {
    ///////////////////////////////////////////////////////////////////////////
    /*
        이 부분에서 영상처리를 수행합니다.
    */
    ///////////////////////////////////////////////////////////////////////////
    readFpgaVideoData(_pDefaultScreen);

    _convertScreenToDisplay(_pDefaultScreen);
    displayScreen(_pDefaultScreen);
}


static void _adjustWhiteBalance(int r5, int g5, int b5) {
    static const char* LOG_FUNCTION_NAME = "_adjustWhiteBalance()";

    Rgab5515_t inputColor;
    Rgab5515_t realColor;
    inputColor.r = r5;
    inputColor.g = g5;
    inputColor.b = b5;
    realColor.r = 16;
    realColor.g = 16;
    realColor.b = 16;

    LookUpTable16_t* pWhiteBalanceTable = createWhiteBalanceTable(&inputColor, &realColor);
    setDefaultWhiteBalanceTable(pWhiteBalanceTable);

    if (pWhiteBalanceTable != NULL) {
        printLog("[%s] inputColor: {r: %d, g: %d, b: %d}\n", LOG_FUNCTION_NAME,
                 inputColor.r, inputColor.g, inputColor.b);
        printLog("[%s] realColor: {r: %d, g: %d, b: %d}\n", LOG_FUNCTION_NAME,
                 realColor.r, realColor.g, realColor.b);
        printLog("[%s] Adjustment success.\n", LOG_FUNCTION_NAME);
    }
    else {
        printLog("[%s] inputColor: {r: %d, g: %d, b: %d}\n", LOG_FUNCTION_NAME,
                 inputColor.r, inputColor.g, inputColor.b);
        printLog("[%s] realColor: {r: %d, g: %d, b: %d}\n", LOG_FUNCTION_NAME,
                 realColor.r, realColor.g, realColor.b);
        printLog("[%s] Adjustment failed.\n", LOG_FUNCTION_NAME);
    }
}

static Rgab5515_t _getMeanRgab5515OfMatrix16(Matrix16_t* pMatrix) {
    uint64_t totalR = 0;
    uint64_t totalG = 0;
    uint64_t totalB = 0;
    
    int length = pMatrix->width * pMatrix->height;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)pMatrix->elements;
    for (int i = 0; i < length; ++i) {
        totalR += pRgab5515->r;
        totalG += pRgab5515->g;
        totalB += pRgab5515->b;
        pRgab5515++;
    }

    Rgab5515_t meanRgab5515;
    meanRgab5515.r = totalR / length;
    meanRgab5515.g = totalG / length;
    meanRgab5515.b = totalB / length;
    return meanRgab5515;
}

static void _fillMatrix16(Matrix16_t* pMatrix, uint16_t value) {
    int length = pMatrix->width * pMatrix->height;
    uint16_t* pData = pMatrix->elements;
    for (int i = 0; i < length; ++i) {
        *pData = value;
        pData++;
    }
}

static void _adjustWhiteBalanceAuto(void) {
    setHead(0, -90);
    mdelay(1500);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoData(pScreen);
    displayScreen(pScreen);

    int width = pScreen->width;
    int height = pScreen->height;
    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 10, height - 20, width - 10, height - 1);
    Rgab5515_t meanRgab5515 = _getMeanRgab5515OfMatrix16(pSubMatrix);
    _adjustWhiteBalance(meanRgab5515.r, meanRgab5515.g, meanRgab5515.b);

    // Debug screen
    _fillMatrix16(pScreen, meanRgab5515.data);
    overlapMatrix16(pSubMatrix, pScreen, 10, height - 20);
    displayScreen(pScreen);

    destroyMatrix16(pSubMatrix);
    destroyScreen(pScreen);

    setHead(0, 0);
    mdelay(1500);
}

