#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "processor.h"
#include "robot_protocol.h"
#include "graphic_interface.h"
#include "color.h"
#include "obstacle_manager.h"

#define _DATA_DIR_PATH          "data"
#define _COLOR_TABLE_FILE_PATH  "data/main.ctb"

#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))

ObstacleId_t* _obstacleSequence;

Screen_t* _pDefaultScreen;
ColorScreen_t* _pDefaultColorScreen;
ColorTable_t _colorTable;

void _defineObstacle(void);
void _improveSomeObstacle(void);

Color_t convertPixelDataToColor(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    uint32_t rgbaData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    Rgba_t* pRgba = (Rgba_t*)&rgbaData;
    pRgba->data = rgab5515ToRgbaData(pRgab5515);

    float r = (float)pRgba->r / 255;
    float g = (float)pRgba->g / 255;
    float b = (float)pRgba->b / 255;
    float h;    // hue
    float s;    // saturation
    float i;    // intensity
    float max = _MAX(_MAX(r, g), b);
    float min = _MIN(_MIN(r, g), b);
    float c = max - min;

    i = (r + g + b) / 3;
    if (c == 0) h = 0;
    else if (max == r) h = 60 * fmodf(((g - b) / c), 6);
    else if (max == g) h = 60 * (((b - r) / c) + 2);
    else if (max == b) h = 60 * (((r - g) / c) + 4);
    else h = 0;
    if (c == 0) s = 0;
    else s = 1 - min / i;

    if (i < 0.2)
        return COLOR_BLACK;
    else if (i > 0.8 && s < 0.2)
        return COLOR_WHITE;
    else if (h >= 300 || h < 60)
        return COLOR_RED;
    else if (h >= 60 && h < 180)
        return COLOR_GREEN;
    else if (h >= 180 && h < 300)
        return COLOR_BLUE;

    return COLOR_YELLOW;
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
    initColorLib();

    mkdir(_DATA_DIR_PATH, 0755);
    createColorTableFile(_COLOR_TABLE_FILE_PATH, convertPixelDataToColor, false);
    _colorTable = loadColorTableFile(_COLOR_TABLE_FILE_PATH);
    
	_defineObstacle();
	_obstacleSequence = loadObstaclesFile("/mnt/f0/obstacles.txt");

    _pDefaultScreen = createDefaultScreen();
    _pDefaultColorScreen = createColorScreen(_pDefaultScreen->width,
                                             _pDefaultScreen->height);

    return 0;
}

void closeProcessor(void) {
    closeGraphicInterface();
    closeRobotPort();
    destroyScreen(_pDefaultScreen);
    destroyColorScreen(_pDefaultColorScreen);
}

int runProcessor(void) {
    int i;
    for (i = 0; i < 100; ++i) {
        _improveSomeObstacle();
    }

    return 0;
}

void _defineObstacle(void) {
	//registerObstacle(OBSTACLE_ONE, helloWorld);
	//registerObstacle(OBSTACLE_TWO, goodbyeWorld);
}

void _improveSomeObstacle(void) {
    ///////////////////////////////////////////////////////////////////////////
    /*
        이 부분에서 영상처리를 수행합니다.
    */
    ///////////////////////////////////////////////////////////////////////////
    readFpgaVideoData(_pDefaultScreen);
    readColorFromScreen(_pDefaultColorScreen, _pDefaultScreen, _colorTable);

    //sendDataToRobot(command);
    //printf("send command to robot: %d\n", command);
    //waitDataFromRobot();

    writeColorToScreen(_pDefaultColorScreen, _pDefaultScreen);
    displayScreen(_pDefaultScreen);
}
