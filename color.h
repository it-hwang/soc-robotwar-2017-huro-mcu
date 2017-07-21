#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdint.h>
#include <stdbool.h>

#include "graphic_interface.h"

#define COLOR_USE_TYPE_CHECKING     true

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

typedef union {
    struct {
        uint8_t a;
        uint8_t b;
        uint8_t g;
        uint8_t r;
    };
    uint32_t data;
} Rgba_t;

typedef union {
    struct {
        uint16_t b : 5;
        uint16_t g : 6;
        uint16_t r : 5;
    };
    uint16_t data;
} Rgb565_t;

typedef union {
    struct {
        uint16_t b : 5;
        uint16_t a : 1;
        uint16_t g : 5;
        uint16_t r : 5;
    };
    uint16_t data;
} Rgab5515_t;

typedef Color_t* ColorTable_t;

typedef struct {
    Color_t* pixels;
    PixelCoordinate_t width;
    PixelCoordinate_t height;
} ColorScreen_t;


uint16_t colorToRgb565DataTable[SIZE_OF_COLOR];

#if COLOR_USE_TYPE_CHECK
    static inline uint32_t rgb565ToRgbaData(Rgb565_t* pRgb565) {
        return ((uint32_t)pRgb565->r << 27) |
               ((uint32_t)pRgb565->g << 18) |
               ((uint32_t)pRgb565->b << 11);
    }

    static inline uint32_t rgab5515ToRgbaData(Rgab5515_t* pRgab5515) {
        return ((uint32_t)pRgab5515->r << 27) |
               ((uint32_t)pRgab5515->g << 19) |
               ((uint32_t)pRgab5515->b << 11) |
               ((uint32_t)pRgab5515->a);
    }

    static inline uint16_t rgb565ToRgab5515Data(Rgb565_t* pRgb565) {
        return ((uint16_t)pRgb565->data & 0xffdf)
    }

    static inline uint16_t rgab5515ToRgb565Data(Rgab5515_t* pRgab5515) {
        return ((uint16_t)pRgab5515->data & 0xffdf |
               (((uint16_t)pRgab5515->data >> 1) & 0x0020));
    }

    static inline uint16_t rgbaToRgb565Data(Rgba_t* pRgba) {
        return (((uint16_t)pRgba->r & 0xf8) << 8) |
               (((uint16_t)pRgba->g & 0xfc) << 3) |
               (((uint16_t)pRgba->b) >> 3);
    }
    
    static inline uint16_t rgbaToRgab5515Data(Rgba_t* pRgba) {
        return (((uint16_t)pRgba->r & 0xf8) << 8) |
               (((uint16_t)pRgba->g & 0xf8) << 4) |
               (((uint16_t)pRgba->a) << 5) |
               (((uint16_t)pRgba->b) >> 3);
    }

    static inline Color_t getColorFromTable(ColorTable_t colorTable,
                                            PixelData_t pixelData) {
        return colorTable[pixelData];
    }

    static inline uint16_t colorToRgb565Data(Color_t color) {
        return colorToRgb565DataTable[color];
    }
#else
    #define rgb565ToRgbaData(pRgb565) \
                ((uint32_t)((Rgb565_t*)pRgb565)->r << 27) |\
                ((uint32_t)((Rgb565_t*)pRgb565)->g << 18) |\
                ((uint32_t)((Rgb565_t*)pRgb565)->b << 11)

    #define rgab5515ToRgbaData(pRgab5515) \
                ((uint32_t)((Rgab5515_t*)pRgab5515)->r << 27) |\
                ((uint32_t)((Rgab5515_t*)pRgab5515)->g << 19) |\
                ((uint32_t)((Rgab5515_t*)pRgab5515)->b << 11) |\
                ((uint32_t)((Rgab5515_t*)pRgab5515)->a)

    #define rgb565ToRgab5515Data(pRgb565) \
                ((uint16_t)pRgb565->data & 0xffdf)

    #define rgab5515ToRgb565Data(pRgab5515) \
                ((uint16_t)pRgab5515->data & 0xffdf |\
                (((uint16_t)pRgab5515->data >> 1) & 0x0020))

    #define rgbaToRgb565Data(pRgba) \
                (((uint16_t)((Rgba_t*)pRgba)->r & 0xf8) << 8) |\
                (((uint16_t)((Rgba_t*)pRgba)->g & 0xfc) << 3) |\
                (((uint16_t)((Rgba_t*)pRgba)->b) >> 3)

    #define rgbaToRgab5515Data(pRgba) \
                (((uint16_t)((Rgba_t*)pRgba)->r & 0xf8) << 8) |\
                (((uint16_t)((Rgba_t*)pRgba)->g & 0xf8) << 4) |\
                (((uint16_t)((Rgba_t*)pRgba)->a) << 5) |\
                (((uint16_t)((Rgba_t*)pRgba)->b) >> 3)

    #define getColorFromTable(colorTable, pixelData)    colorTable[pixelData]
    #define colorToRgb565Data(color)    colorToRgb565DataTable[color]
#endif

void initColorLib(void);

bool createColorTableFile(const char* filePath, Color_t (*pFunc)(PixelData_t),
                          bool overwrite);
ColorTable_t loadColorTableFile(const char* filePath);

ColorScreen_t* createColorScreen(PixelCoordinate_t width,
                                 PixelCoordinate_t height);
void destroyColorScreen(ColorScreen_t* pScreen);
bool readColorFromScreen(ColorScreen_t* pScreen, Screen_t* pSource,
                         ColorTable_t colorTable);
bool writeColorToScreen(ColorScreen_t* pScreen, Screen_t* pTarget);

#endif // __COLOR_H__
