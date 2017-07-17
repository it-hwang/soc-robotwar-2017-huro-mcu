#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "processor.h"
#include "graphic_api.h"
#include "robot_protocol.h"
#include "color.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define _SCREEN_WIDTH		180
#define _SCREEN_HEIGHT		120

U16* _pixels;
LPCOLOR _colorCache;
U16 _pixelCache[10];

inline void _readFpgaVideoData(U16* pBuffer);
inline void _drawFpgaVideoData(U16* pBuffer);

void _improveSomeObstacle(void);


COLOR getColorFunc(uint32_t pixel) {
	uint32_t rgbaData;
	LPRGBA rgba = &rgbaData;
	rgab5515ToRgba(&pixel, rgba);

	float r = rgba->r;
	float g = rgba->g;
	float b = rgba->b;
	float h;	// hue
	float s;	// saturation
	float i;	// intensity
	float max = MAX(MAX(r, g), b);
	float min = MIN(MIN(r, g), b);
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
	if (open_graphic() < 0) {
		closeProcessor();
		return PROCESSOR_GRAPHIC_ERROR;
	}
	if (openRobotPort() < 0) {
		closeProcessor();
		return PROCESSOR_ROBOT_PORT_ERROR;
	}
	_pixels = (U16*)malloc(_SCREEN_WIDTH * _SCREEN_HEIGHT * sizeof(U16));

	createColorTable("/mnt/f0/data/main.ctb", sizeof(U16), getColorFunc, false);
	_colorCache = loadColorTable("/mnt/f0/data/main.ctb", sizeof(U16));

	LPRGB565 pixel;

	pixel = &_pixelCache[COLOR_BLACK];
	pixel->r = 0x00;
	pixel->g = 0x00;
	pixel->b = 0x00;
	pixel = &_pixelCache[COLOR_BLUE];
	pixel->r = 0x00;
	pixel->g = 0x00;
	pixel->b = 0xff;
	pixel = &_pixelCache[COLOR_GREEN];
	pixel->r = 0x00;
	pixel->g = 0xff;
	pixel->b = 0x00;
	pixel = &_pixelCache[COLOR_RED];
	pixel->r = 0xff;
	pixel->g = 0x00;
	pixel->b = 0x00;
	pixel = &_pixelCache[COLOR_WHITE];
	pixel->r = 0xff;
	pixel->g = 0xff;
	pixel->b = 0xff;
	pixel = &_pixelCache[COLOR_YELLOW];
	pixel->r = 0xff;
	pixel->g = 0xff;
	pixel->b = 0x00;

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
			U16 pixel = _pixels[y * _SCREEN_WIDTH + x];
			//LPRGAB5515 input = &_pixels[y * _SCREEN_WIDTH + x];
			LPRGB565 output = &_pixels[y * _SCREEN_WIDTH + x];
			//U16 c = _pixels[y * _SCREEN_WIDTH + x];
			//U8 r = (c >> 8) & 0xf8;
			//U8 g = (c >> 3) & 0xfc;
			//U8 b = (c << 3) & 0xf8;
			//U8 r = input->r << 3;
			//U8 g = input->g << 3;
			//U8 b = input->b << 3;
			COLOR color = _colorCache[pixel];
			
			output->data16 = _pixelCache[color];
			
			/*
			switch (color) {
				case COLOR_BLACK:
					output->r = 0x00;
					output->g = 0x00;
					output->b = 0x00;
					break;
				case COLOR_BLUE:
					output->r = 0x00;
					output->g = 0x00;
					output->b = 0xff;
					break;
				case COLOR_GREEN:
					output->r = 0x00;
					output->g = 0xff;
					output->b = 0x00;
					break;
				case COLOR_RED:
					output->r = 0xff;
					output->g = 0x00;
					output->b = 0x00;
					break;
				case COLOR_WHITE:
					output->r = 0xff;
					output->g = 0xff;
					output->b = 0xff;
					break;
				case COLOR_YELLOW:
					output->r = 0xff;
					output->g = 0xff;
					output->b = 0x00;
					break;
				default:
					output->r = 0x80;
					output->g = 0x80;
					output->b = 0x80;
			}
			*/

			/*
			if ((x == _SCREEN_WIDTH / 2) ||
				(y == _SCREEN_HEIGHT / 2)) {
				output->r = 0xff;
				output->g = 0x00;
				output->b = 0x00;
			}
			*/

			//_pixels[y * _SCREEN_WIDTH + x] = MAKE_RGB565(r, g, b);
		}
	}
	
	//sendDataToRobot(command);
	//printf("send command to robot: %d\n", command);
	//waitDataFromRobot();

	_drawFpgaVideoData(_pixels);
}
