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
	COLOR_YELLOW
} COLOR;

typedef struct {
	union {
		struct {
			uint8_t a;
			uint8_t b;
			uint8_t g;
			uint8_t r;
		};
		uint32_t data32;
	};
} COLOR_RGBA;

typedef struct {
	union {
		struct {
			uint16_t b : 5;
			uint16_t g : 6;
			uint16_t r : 5;
		};
		uint16_t data16;
	};
} COLOR_RGB565;

typedef struct {
	union {
		struct {
			uint16_t b : 5;
			uint16_t a : 1;
			uint16_t g : 5;
			uint16_t r : 5;
		};
		uint16_t data16;
	};
} COLOR_RGAB5515;

inline void rgb565ToRgba(COLOR_RGB565* source, COLOR_RGBA* target);
inline void rgbaToRgb565(COLOR_RGBA* source, COLOR_RGB565* target);
inline void rgab5515ToRgba(COLOR_RGAB5515* source, COLOR_RGBA* target);
inline void rgbaToRgab5515(COLOR_RGBA* source, COLOR_RGAB5515* target);

/*
#define rgb565ToRgba(source, target) target.data32 = ((uint32_t)source.r << 27) | ((uint32_t)source.g << 18) | ((uint32_t)source.b << 11)
#define rgbaToRgb565(source, target) target.data16 = (((uint16_t)source.r & 0xf8) << 8) | (((uint16_t)source.g & 0xfc) << 3) | (((uint16_t)source.b) >> 8)
#define rgab5515ToRgba(source, target) target.data32 = ((uint32_t)source.r << 27) | ((uint32_t)source.g << 18) | ((uint32_t)source.b << 11) | ((uint32_t)source.a)
#define rgbaToRgab5515(source, target) target.data16 = (((uint16_t)source.r & 0xf8) << 8) | (((uint16_t)source.g & 0xf8) << 4) | (((uint16_t)source.a) << 5) | (((uint16_t)source.b) >> 3)
*/

bool createColorTable(const char* filename, uint8_t nBytesPerPixel,
					  COLOR (*pFunc)
void* loadColorTable(const char* filename);

#endif // __COLOR_H__
