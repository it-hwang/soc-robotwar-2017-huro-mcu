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
#include "robot_protocol.h"
#include "image_filter.h"

ObstacleId_t* _obstacleSequence;

Screen_t* _pDefaultScreen;


void _improveSomeObstacle(void);

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
    finalizeColor();
}

int runProcessor(void) {
    int i;
    for (i = 0; i < 100; ++i) {
        _improveSomeObstacle();
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
    Matrix8_t* pColorMatrix = createColorMatrix(_pDefaultScreen, 
                                    pColorTables[COLOR_YELLOW]);


    //applyDilationToMatrix8(pColorMatrix, 1);
    //applyErosionToMatrix8(pColorMatrix, 2);
    //applyDilationToMatrix8(pColorMatrix, 1);


    //sendDataToRobot(command);
    //printf("send command to robot: %d\n", command);
    //waitDataFromRobot();

    _applyColorMatrix(_pDefaultScreen, pColorMatrix);
    destroyMatrix8(pColorMatrix);
    _convertScreenToDisplay(_pDefaultScreen);
    displayScreen(_pDefaultScreen);
}
