#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "color.h"

void _registerColor(Color_t color, uint8_t r, uint8_t g, uint8_t b);
ColorTable_t _createColorTable(uint64_t length, Color_t (*pFunc)(uint32_t));


void initColorLib(void) {
	_registerColor(COLOR_BLACK, 0x00, 0x00, 0x00);
	_registerColor(COLOR_WHITE, 0xff, 0xff, 0xff);
	_registerColor(COLOR_RED, 0xff, 0x00, 0x00);
	_registerColor(COLOR_GREEN, 0x00, 0xff, 0x00);
	_registerColor(COLOR_BLUE, 0x00, 0x00, 0xff);
	_registerColor(COLOR_YELLOW, 0xff, 0xff, 0x00);
	_registerColor(COLOR_ORANGE, 0xff, 0x80, 0x27);
}

bool createColorTableFile(const char* filePath, size_t pixelDataSize,
						  Color_t (*pFunc)(uint32_t), bool overwrite) {
	bool isFileExists = access(filePath, F_OK) == 0;

	if (isFileExists && !overwrite) {
		return false;
	}

	uint64_t colorTableLength = pow(2, pixelDataSize * 8);
	ColorTable_t colorTable = _createColorTable(colorTableLength, pFunc);

	FILE* outputFile = fopen(filePath, "w");
	if (outputFile == NULL) {
		free(colorTable);
		return false;
	}

	fwrite(colorTable, sizeof(Color_t), colorTableLength, outputFile);
	fclose(outputFile);
	free(colorTable);
	return true;
}

ColorTable_t loadColorTableFile(const char* filePath, size_t pixelDataSize) {
	FILE* inputFile = fopen(filePath, "r");
	if (inputFile == NULL)
		return NULL;

	uint64_t colorTableLength = pow(2, pixelDataSize * 8);
	ColorTable_t colorTable = _createColorTable(colorTableLength, NULL);

	fread(colorTable, sizeof(Color_t), colorTableLength, inputFile);

	fclose(inputFile);
	return colorTable;
}


void _registerColor(Color_t color, uint8_t r, uint8_t g, uint8_t b) {
	Rgba_t rgba;
	rgba.r = r;
	rgba.g = g;
	rgba.b = b;
	colorToRgb565DataTable[color] = rgbaToRgb565Data(rgba);
}

ColorTable_t _createColorTable(uint64_t length, Color_t (*pFunc)(uint32_t)) {
	ColorTable_t colorTable;
	uint64_t i = 0;

	colorTable = (ColorTable_t)malloc(length * sizeof(Color_t));
	if (pFunc)
		for (i = 0; i < length; ++i)
			colorTable[i] = (*pFunc)(i);

	return colorTable;
}
