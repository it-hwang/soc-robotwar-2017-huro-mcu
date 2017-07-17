#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "processor.h"
#include "graphic_api.h"
#include "robot_protocol.h"
#include "color.h"

#define _SCREEN_WIDTH		180
#define _SCREEN_HEIGHT		120

U16* _pixels;

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
	_pixels = (U16*)malloc(_SCREEN_WIDTH * _SCREEN_HEIGHT * sizeof(U16));

	return 0;
}

void closeProcessor(void) {
	close_graphic();
	closeRobotPort();
	free(_pixels);
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

void _improveSomeObstacle(void) {
	///////////////////////////////////////////////////////////////////////////
	/*
		이 부분에서 영상처리를 수행합니다.
	*/
	///////////////////////////////////////////////////////////////////////////
	_readFpgaVideoData(_pixels);

	int x;
	int y;

	for (y = 0; y < _SCREEN_HEIGHT; ++y) {
		for (x = 0; x < _SCREEN_WIDTH; ++x) {
			U16 c = _pixels[y * _SCREEN_WIDTH + x];
			U8 r = (c >> 8) & 0xf8;
			U8 g = (c >> 3) & 0xfc;
			U8 b = (c << 3) & 0xf8;
			
			if ((x == _SCREEN_WIDTH / 2) ||
				(y == _SCREEN_HEIGHT / 2))
				r = 0xff;

			_pixels[y * _SCREEN_WIDTH + x] = MAKE_RGB565(r, g, b);
		}
	}
	
	//sendDataToRobot(command);
	//printf("send command to robot: %d\n", command);
	//waitDataFromRobot();

	_drawFpgaVideoData(_pixels);
}
