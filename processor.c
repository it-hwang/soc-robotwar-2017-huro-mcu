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
        //_improveSomeObstacle();
        Send_Command(0x01, 0xfe);
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
                                    pColorTables[COLOR_BLACK]);
    
    // 깁기
    applyDilationToMatrix8(pColorMatrix, 1);
    applyErosionToMatrix8(pColorMatrix, 2);
    applyDilationToMatrix8(pColorMatrix, 1);
  
  
    ObjectList_t* pMatrixObjectList;
    pMatrixObjectList = detectObjectsLocation(pColorMatrix);
    //printf("list size 1 %d\n", pMatrixObjectList->size);
    if (pMatrixObjectList) {
        int i;
        for(i = 0; i < pMatrixObjectList->size; ++i) {
            int x;
            int y;
            Object_t object = pMatrixObjectList->list[i];
            PixelData_t* pixels = _pDefaultScreen->elements;

            for(y = object.minY; y <= object.maxY; ++y) {
                for(x = object.minX; x <= object.maxX; ++x) {
                    int index = y * _pDefaultScreen->width + x;
                    uint16_t* pOutput = (uint16_t*)&pixels[index];

                    if(y == object.minY || y == object.maxY) {
                        *pOutput = 0xF800;
                    } else if(x == object.minX || x == object.maxX) {
                        *pOutput = 0xF800;
                    }
                }
            }

            int index = (int)object.centerY * _pDefaultScreen->width + (int)object.centerX;
            uint16_t* pOutput = (uint16_t*)&pixels[index];
            *pOutput = 0x1F;
        }
    }

    if (pMatrixObjectList)
        free(pMatrixObjectList);
    
    
    /*pMatrixObjectList = detectObjectsLocation(pColorMatrix);
    if (pMatrixObjectList)
        free(pMatrixObjectList);
    */

    //line-detection process    
    
    Line_t* line = lineDetection(pColorMatrix);
    if(line == NULL) {
        printf("None line!!!\n");
    }
    else {
        printf("Yes line!!!\n");
        printf("line THETA = %f\n", line->theta);
        printf("line DistancePixel = (%d, %d)\n", line->distancePoint.x, line->distancePoint.y);
        free(line);
    }
    
    /***********************************************************************************************/
    //sendDataToRobot(command);
    //printf("send command to robot: %d\n", command);
    //waitDataFromRobot();
    
    //free(A);
    //_applyColorMatrix(_pDefaultScreen, pColorMatrix);
    destroyMatrix8(pColorMatrix);
    _convertScreenToDisplay(_pDefaultScreen);
    displayScreen(_pDefaultScreen);
}
