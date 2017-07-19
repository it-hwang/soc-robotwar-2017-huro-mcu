#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	COLOR_BLACK,
	COLOR_WHITE,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_YELLOW,
	COLOR_ORANGE,
	SIZE_OF_COLOR
} Color_t;

typedef struct {
	union {
		struct {
			uint8_t a;
			uint8_t b;
			uint8_t g;
			uint8_t r;
		};
		uint32_t data;
	};
} Rgba_t;

typedef struct {
	union {
		struct {
			uint16_t b : 5;
			uint16_t g : 6;
			uint16_t r : 5;
		};
		uint16_t data;
	};
} Rgb565_t;

typedef struct {
	union {
		struct {
			uint16_t b : 5;
			uint16_t a : 1;
			uint16_t g : 5;
			uint16_t r : 5;
		};
		uint16_t data;
	};
} Rgab5515_t;

typedef Color_t* ColorTable_t;


uint16_t colorToRgb565DataTable[SIZE_OF_COLOR];

#define rgb565ToRgbaData(rgb565)		((uint32_t)(rgb565).r << 27) | \
										((uint32_t)(rgb565).g << 18) | \
										((uint32_t)(rgb565).b << 11)
#define rgab5515ToRgbaData(rgab5515)	((uint32_t)(rgab5515).r << 27) | \
										((uint32_t)(rgab5515).g << 19) | \
										((uint32_t)(rgab5515).b << 11) | \
										((uint32_t)(rgab5515).a)
#define rgbaToRgb565Data(rgba)			(((uint16_t)(rgba).r & 0xf8) << 8) |\
										(((uint16_t)(rgba).g & 0xfc) << 3) |\
										(((uint16_t)(rgba).b) >> 3)
#define rgbaToRgab5515Data(rgba)		(((uint16_t)(rgba).r & 0xf8) << 8) |\
										(((uint16_t)(rgba).g & 0xf8) << 4) |\
										(((uint16_t)(rgba).a) << 5) |\
										(((uint16_t)(rgba).b) >> 3)

bool createColorTableFile(const char* filePath, size_t pixelDataSize,
						  Color_t (*pFunc)(uint32_t), bool overwrite);
ColorTable_t loadColorTableFile(const char* filePath, size_t pixelDataSize);
void initColorLib(void);

#define getColorFromTable(colorTable, pixelData)	colorTable[pixelData]
#define colorToRgb565Data(color)		colorToRgb565DataTable[color]

#endif // __COLOR_H__
