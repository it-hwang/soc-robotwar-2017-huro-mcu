#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "color.h"

LPRGB565 _colorToRgb565Table;
void _registerColor(COLOR color, uint8_t r, uint8_t g, uint8_t b);

void rgb565ToRgba(LPRGB565 source, LPRGBA target) {
	//target->r = source->r << 3;
	//target->g = source->g << 2;
	//target->b = source->b << 3;
	//target->a = 0x00;

	// 위의 명령을 줄여서 다음과 같이 표현합니다.
	target->data32 = ((uint32_t)source->r << 27) | ((uint32_t)source->g << 18) | ((uint32_t)source->b << 11);
}

void rgbaToRgb565(LPRGBA source, LPRGB565 target) {
	//target->r = source->r >> 3;
	//target->g = source->g >> 2;
	//target->b = source->b >> 3;
	
	// 위의 명령을 줄여서 다음과 같이 표현합니다.
	target->data16 = (((uint16_t)source->r & 0xf8) << 8) | (((uint16_t)source->g & 0xfc) << 3) | (((uint16_t)source->b) >> 3);
}

void rgab5515ToRgba(LPRGAB5515 source, LPRGBA target) {
	//target->r = source->r << 3;
	//target->g = source->g << 3;
	//target->b = source->b << 3;
	//target->a = source->a * 0xff;
	
	// 위의 명령을 줄여서 다음과 같이 표현합니다.
	target->data32 = ((uint32_t)source->r << 27) | ((uint32_t)source->g << 19) | ((uint32_t)source->b << 11) | ((uint32_t)source->a * 0xff);
}

void rgbaToRgab5515(LPRGBA source, LPRGAB5515 target) {
	//target->r = source->r >> 3;
	//target->g = source->g >> 3;
	//target->b = source->b >> 3;
	//target->a = !!source->a;
	
	// 위의 명령을 줄여서 다음과 같이 표현합니다.
	target->data16 = (((uint16_t)source->r & 0xf8) << 8) | (((uint16_t)source->g & 0xf8) << 4) | (((uint16_t)source->a) << 5) | (((uint16_t)source->b) >> 3);
}

void colorToRgb565(COLOR source, LPRGB565 target) {

	*target = _colorToRgb565Table[source];
}

bool createColorTableFile(const char* filename, size_t pixelSize,
					  COLOR (*pFunc)(uint32_t), bool overwrite) {
	bool isExists = access(filename, F_OK) == 0;
	if (!overwrite && isExists)
		return false;
	
	uint32_t nPixels = pow(2, pixelSize * 8);
	uint32_t i;
	LPCOLOR cache = (LPCOLOR)malloc(nPixels * sizeof(COLOR));
	for (i = 0; i < nPixels; ++i) {
		cache[i] = (*pFunc)(i);
	}

	FILE* outputFile;
	outputFile = fopen(filename, "w");
	if (outputFile == NULL)
		return false;

	fwrite(cache, sizeof(COLOR), nPixels, outputFile);

	fclose(outputFile);
	free(cache);
	return true;
}

LPCOLOR loadColorTableFile(const char* filename, size_t pixelSize) {
	FILE* inputFile;
	inputFile = fopen(filename, "r");
	if (inputFile == NULL)
		return NULL;

	uint32_t nPixels = pow(2, pixelSize * 8);
	LPCOLOR cache = (LPCOLOR)malloc(nPixels * pixelSize);

	fread(cache, sizeof(COLOR), nPixels, inputFile);

	fclose(inputFile);
	return cache;
}

void initColorToRgb565Table(void) {
	LPRGB565 cache = (LPRGB565)malloc(SIZE_OF_COLOR * sizeof(uint16_t));
	_colorToRgb565Table = cache;

	_registerColor(COLOR_BLACK, 0x00, 0x00, 0x00);
	_registerColor(COLOR_WHITE, 0xff, 0xff, 0xff);
	_registerColor(COLOR_RED, 0xff, 0x00, 0x00);
	_registerColor(COLOR_GREEN, 0x00, 0xff, 0x00);
	_registerColor(COLOR_BLUE, 0x00, 0x00, 0xff);
	_registerColor(COLOR_YELLOW, 0xff, 0xff, 0x00);
}

COLOR getColorFromTable(LPCOLOR colorTable, uint16_t pixel) {
	return colorTable[pixel];
}


void _registerColor(COLOR color, uint8_t r, uint8_t g, uint8_t b) {
	RGBA rgba;
	rgba.r = r;
	rgba.g = g;
	rgba.b = b;
	rgbaToRgb565(&rgba, &_colorToRgb565Table[color]);
}
