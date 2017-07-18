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
	SIZE_OF_COLOR
} COLOR, *LPCOLOR;

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
} RGBA, *LPRGBA;

typedef struct {
	union {
		struct {
			uint16_t b : 5;
			uint16_t g : 6;
			uint16_t r : 5;
		};
		uint16_t data16;
	};
} *LPRGB565;

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
} *LPRGAB5515;

inline void rgb565ToRgba(LPRGB565 source, LPRGBA target);
inline void rgbaToRgb565(LPRGBA source, LPRGB565 target);
inline void rgab5515ToRgba(LPRGAB5515 source, LPRGBA target);
inline void rgbaToRgab5515(LPRGBA source, LPRGAB5515 target);
inline COLOR getColorFromTable(LPCOLOR colorTable, uint16_t pixel);
inline void colorToRgb565(COLOR source, LPRGB565 target);

inline uint32_t rgb565ToRgbaData(LPRGB565 pSource);
inline uint32_t rgab5515ToRgbaData(LPRGAB5515 pSource);
inline uint16_t rgbaToRgb565Data(LPRGBA pSource);
inline uint16_t rgbaToRgab5515Data(LPRGBA pSource);
inline uint16_t colorToRgb565Data(COLOR source);
/*
#define rgb565ToRgba(source, target) target.data32 = ((uint32_t)source.r << 27) | ((uint32_t)source.g << 18) | ((uint32_t)source.b << 11)
#define rgbaToRgb565(source, target) target.data16 = (((uint16_t)source.r & 0xf8) << 8) | (((uint16_t)source.g & 0xfc) << 3) | (((uint16_t)source.b) >> 8)
#define rgab5515ToRgba(source, target) target.data32 = ((uint32_t)source.r << 27) | ((uint32_t)source.g << 18) | ((uint32_t)source.b << 11) | ((uint32_t)source.a)
#define rgbaToRgab5515(source, target) target.data16 = (((uint16_t)source.r & 0xf8) << 8) | (((uint16_t)source.g & 0xf8) << 4) | (((uint16_t)source.a) << 5) | (((uint16_t)source.b) >> 3)
*/

bool createColorTableFile(const char* filename, size_t pixelSize,
					  COLOR (*pFunc)(uint32_t), bool overwrite);
LPCOLOR loadColorTableFile(const char* filename, size_t pixelSize);
void initColorToRgb565Table(void);

#endif // __COLOR_H__
