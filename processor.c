#include <stdio.h>
#include <stdbool.h>

#include "processor.h"
#include "graphic_api.h"
#include "robot_protocol.h"
#include "color.h"

#define _SCREEN_WIDTH		180
#define _SCREEN_HEIGHT		120

U16 _pixels[_SCREEN_HEIGHT][_SCREEN_WIDTH];

inline void _readFpgaVideoData(U16* pBuffer);
inline void _drawFpgaVideoData(U16* pBuffer);

void _improveSomeObstacle(void);


int openProcessor(void) {
	if (open_graphic() < 0) {
		closeProcessor();
		return PROCESSOR_GRAPHIC_ERROR;
	}
	if (openRobotPort() < 0) {
		closeProcessor();
		return PROCESSOR_ROBOT_PORT_ERROR;
	}

	return 0;
}

void closeProcessor(void) {
	close_graphic();
	closeRobotPort();
}

int runProcessor(void) {
	direct_camera_display_off();
	
	int i;
	for (i = 0; i < 10000; ++i) {
		_improveSomeObstacle();
	}

	direct_camera_display_on();
	return 0;
}

void _readFpgaVideoData(U16* pBuffer) {
	read_fpga_video_data(pBuffer);
}

void _drawFpgaVideoData(U16* pBuffer) {
	clear_screen();
	draw_fpga_video_data_full(pBuffer);
	flip();
}

U16 _getPixel(U16* pBuffer, int width, int height, int x, int y) {
	return pBuffer[y * width + x];
}

void _setPixel(U16* pBuffer, int width, int height, int x, int y, U16 value) {
	pBuffer[y * width + x] = value;
}

void _displayKeypoints(void) {
	int x;
	int y;
	bool isKeypoint;

	//COLOR_RGAB5515 input;
	for (y = 0; y < _SCREEN_HEIGHT-4; ++y) {
		for (x = 0; x < _SCREEN_WIDTH-2; ++x) {
			//input.data16 = _pixels[y][x];
			// TODO : 임시 작성 코드이므로, 수정이 필요합니다.
			isKeypoint = _pixels[(y+4)][(x+2)] & 0x0020;

			if (isKeypoint) {
				//input.r = 0x00;
				//input.g = 0xff;
				//input.b = 0x00;
				_pixels[y][x] = 0x07e0;
			}

			//_pixels[y][x] = input.data16;
		}
	}
}

void _improveSomeObstacle(void) {
	///////////////////////////////////////////////////////////////////////////
	/*
		이 부분에서 영상처리를 수행합니다.
	*/
	///////////////////////////////////////////////////////////////////////////
	_readFpgaVideoData((U16*)_pixels);
	
	_displayKeypoints();
	
	//sendDataToRobot(command);
	//printf("send command to robot: %d\n", command);
	//waitDataFromRobot();

	_drawFpgaVideoData((U16*)_pixels);
}
