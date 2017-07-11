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
	for (i = 0; i < 100; ++i) {
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

// [임시 함수] 특징점 옮기기
void _moveKeypoints() {
	int x;
	int y;
	
	COLOR_RGAB5515 input;
	COLOR_RGAB5515 output;
	for (y = 0; y < _SCREEN_HEIGHT; ++y) {
		for (x = 0; x < _SCREEN_WIDTH; ++x) {
			input.data16 = _pixels[(y+4)%_SCREEN_HEIGHT][(x+2)%_SCREEN_WIDTH];
			output.data16 = _pixels[y][x];

			output.a = input.a;

			_pixels[y][x] = output.data16;
		}
	}
}

void _displayKeypoints(void) {
	int x;
	int y;

	COLOR_RGAB5515 input;
	COLOR_RGBA rgba;
	COLOR_RGB565 output;
	for (y = 0; y < _SCREEN_HEIGHT; ++y) {
		for (x = 0; x < _SCREEN_WIDTH; ++x) {
			input.data16 = _pixels[y][x];
			rgab5515ToRgba(&input, &rgba);

			if (rgba.a == 0xff) {
				rgba.r = 0x00;
				rgba.g = 0xff;
				rgba.b = 0x00;
			}

			rgbaToRgb565(&rgba, &output);

			_pixels[y][x] = output.data16;
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
	
	_moveKeypoints();
	_displayKeypoints();
	
	//sendDataToRobot(command);
	//printf("send command to robot: %d\n", command);
	//waitDataFromRobot();

	_drawFpgaVideoData((U16*)_pixels);
}
