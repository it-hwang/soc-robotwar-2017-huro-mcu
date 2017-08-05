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
#include "screenio.h"

ObstacleId_t* _obstacleSequence;

Screen_t* _pDefaultScreen;


void _saveScreenshot(void);
void _loadScreenshot(const char* filePath);

void _defineObstacle(void) {
	//registerObstacle(OBSTACLE_ONE, helloWorld);
	//registerObstacle(OBSTACLE_TWO, goodbyeWorld);
}

void _convertScreenToDisplay(Screen_t* pScreen) {
    int length = pScreen->height * pScreen->width;
    int i;
    for (i = 0; i < length; ++i) {
        Rgab5515_t* pRgab5515 = (Rgab5515_t*)&(pScreen->elements[i]);
        pRgab5515->a = pRgab5515->g;
    }
}

void _applyColorMatrix(Screen_t* pScreen, Matrix8_t* pColorMatrix) {
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
}

int runProcessor(void) {
    char inputs[1024] = {0,};
    while (true) {
        printf("Input ('x' is exit): ");
        scanf("%s", inputs);
        if (inputs[0] == 'x' || inputs[0] == 'X')
            break;

        _saveScreenshot();
        // _loadScreenshot(inputs);
    }

    return 0;
}


void _saveScreenshot(void) {
    mkdir("./screenshots", 0755);

    // 적합한 파일 이름 찾기
    char filePath[1024];
    int i = 0;
    while (true) {
        sprintf(filePath, "./screenshots/sc%d", i);
        if (access(filePath, 0) != 0)
            break;
        i++;
    }

    readFpgaVideoData(_pDefaultScreen);

    writeScreen(_pDefaultScreen, filePath);

    _convertScreenToDisplay(_pDefaultScreen);
    displayScreen(_pDefaultScreen);
}

void _loadScreenshot(const char* filePath) {
    Screen_t* pScreen = scanScreen(filePath);
    if (pScreen == NULL)
        return;
    
    displayScreen(pScreen);
    destroyScreen(pScreen);
}
