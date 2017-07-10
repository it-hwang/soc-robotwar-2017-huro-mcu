#include <stdio.h>

#include "processor.h"
#include "graphic_api.h"
#include "robot_protocol.h"

#define _SCREEN_WIDTH		180
#define _SCREEN_HEIGHT		120

U16 _pixels[_SCREEN_HEIGHT][_SCREEN_WIDTH];

void _loop(void);


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
		clear_screen();
		read_fpga_video_data((U16*)&_pixels);
		_loop();
		draw_fpga_video_data_full((U16*)&_pixels);
		flip();
	}

	direct_camera_display_on();
	return 0;
}

void _loop(void) {
	///////////////////////////////////////////////////////////////////////////
	/*
		이 부분에서 영상처리를 수행합니다.
	*/
	///////////////////////////////////////////////////////////////////////////
	int x;
	int y;
        
	for (y = 0; y < _SCREEN_HEIGHT; ++y) {
		for (x = 0; x < _SCREEN_WIDTH; ++x) {
			U16 c = _pixels[y][x];
			U8 r = (c >> 8) & 0xf8;
			U8 g = (c >> 3) & 0xfc;
			U8 b = (c << 3) & 0xf8;
			
			if ((x == _SCREEN_WIDTH / 2) ||
				(y == _SCREEN_HEIGHT / 2))
				r = 0xff;

			_pixels[y][x] = MAKE_RGB565(r, g, b);
		}
	}
	
	//sendDataToRobot(command);
	//printf("send command to robot: %d\n", command);
	//waitDataFromRobot();
}
