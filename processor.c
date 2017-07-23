#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "processor.h"
#include "graphic_api.h"
#include "robot_protocol.h"
#include "color.h"
#include "obstacle_manager.h"
#include "object_detection.h"

#define _DATA_DIR_PATH          "data"
#define _COLOR_TABLE_FILE_PATH  "data/main.ctb"

#define _SCREEN_WIDTH       180
#define _SCREEN_HEIGHT      120

#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))

ObstacleId_t* _obstacleSequence;

uint16_t* _pBuffer;
ColorTable_t _colorTable;

inline void _readFpgaVideoData(uint16_t* pBuffer);
inline void _drawFpgaVideoData(uint16_t* pBuffer);

void _defineObstacle(void);
void _improveSomeObstacle(void);

Color_t convertPixelDataToColor(uint32_t pixelData) {
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

	if (i < 0.2 )
		return COLOR_BLACK;
	else if (i > 0.15 && s < 0.15)
		return COLOR_WHITE;
	else if (h >= 282  ||h<10)
		return COLOR_RED;
	else if (h >= 100 && h < 200)
		return COLOR_GREEN;
	else if (h >= 200 && h < 282)
		return COLOR_BLUE;
    else if(h>=10 && h<100)
		return COLOR_YELLOW;
    return COLOR_WHITE;
}

int openProcessor(void) {
    if (open_graphic() < 0) {
        closeProcessor();
        return PROCESSOR_GRAPHIC_ERROR;
    }
    if (openRobotPort() < 0) {
        closeProcessor();
        return PROCESSOR_ROBOT_PORT_ERROR;
    }
    _pBuffer = (uint16_t*)malloc(_SCREEN_WIDTH * _SCREEN_HEIGHT * sizeof(uint16_t));

    initColorLib();
    mkdir(_DATA_DIR_PATH, 0755);
    createColorTableFile(_COLOR_TABLE_FILE_PATH, sizeof(uint16_t), convertPixelDataToColor, false);
    _colorTable = loadColorTableFile(_COLOR_TABLE_FILE_PATH, sizeof(uint16_t));
    
	_defineObstacle();
	_obstacleSequence = loadObstaclesFile("/mnt/f0/obstacles.txt");

    return 0;
}

void closeProcessor(void) {
    close_graphic();
    closeRobotPort();
    free(_pBuffer);
}

int runProcessor(void) {
    direct_camera_display_off();
    
    int i;
    for (i = 0; i < 100; ++i) {
        _improveSomeObstacle();
    }

    direct_camera_display_on();
    return 0;
}

void _defineObstacle(void) {
	//registerObstacle(OBSTACLE_ONE, helloWorld);
	//registerObstacle(OBSTACLE_TWO, goodbyeWorld);
}

void _readFpgaVideoData(U16* pBuffer) {
	read_fpga_video_data(pBuffer);
}

void _drawFpgaVideoData(uint16_t* pBuffer) {
    clear_screen();
    draw_fpga_video_data_full(pBuffer);
    flip();
}

void _improveSomeObstacle(void) {
    ///////////////////////////////////////////////////////////////////////////
    /*
        이 부분에서 영상처리를 수행합니다.
    */
    ///////////////////////////////////////////////////////////////////////////
    _readFpgaVideoData(_pBuffer);

    detectObjectsLocation(_pBuffer, _colorTable, COLOR_BLUE);

    /*int x;
    int y;

    for (y = 0; y < _SCREEN_HEIGHT; ++y) {
        for (x = 0; x < _SCREEN_WIDTH; ++x) {
            int index = y * _SCREEN_WIDTH + x;
            uint16_t pixelData = _pBuffer[index];
            Color_t color = getColorFromTable(_colorTable, pixelData);

            Rgb565_t* pOutput = (Rgb565_t*)&_pBuffer[index];
            pOutput->data = colorToRgb565Data(color);
        }
    }*/
    
    //sendDataToRobot(command);
    //printf("send command to robot: %d\n", command);
    //waitDataFromRobot();

    _drawFpgaVideoData(_pBuffer);
}
