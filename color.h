#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdint.h>
#include <stdbool.h>

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
                                            uint32_t pixelData) {
        return colorTable[pixelData];
    }

    static inline uint16_t colorToRgb565Data(Color_t color) {
        return colorToRgb565DataTable[color];
    }
#else
    #define rgb565ToRgbaData(pRgb565)   ((uint32_t)pRgb565->r << 27) | \
                                        ((uint32_t)pRgb565->g << 18) | \
                                        ((uint32_t)pRgb565->b << 11)
    #define rgab5515ToRgbaData(pRgab5515)   ((uint32_t)pRgab5515->r << 27) | \
                                            ((uint32_t)pRgab5515->g << 19) | \
                                            ((uint32_t)pRgab5515->b << 11) | \
                                            ((uint32_t)pRgab5515->a)
    #define rgbaToRgb565Data(pRgba)        (((uint16_t)pRgba->r & 0xf8) << 8) |\
                                        (((uint16_t)pRgba->g & 0xfc) << 3) |\
                                        (((uint16_t)pRgba->b) >> 3)
    #define rgbaToRgab5515Data(pRgba)   (((uint16_t)pRgba->r & 0xf8) << 8) |\
                                        (((uint16_t)pRgba->g & 0xf8) << 4) |\
                                        (((uint16_t)pRgba->a) << 5) |\
                                        (((uint16_t)pRgba->b) >> 3)
    #define getColorFromTable(colorTable, pixelData)    colorTable[pixelData]
    #define colorToRgb565Data(color)    colorToRgb565DataTable[color]
#endif

void initColorLib(void);

bool createColorTableFile(const char* filePath, size_t pixelDataSize,
                          Color_t (*pFunc)(uint32_t), bool overwrite);
ColorTable_t loadColorTableFile(const char* filePath, size_t pixelDataSize);

#endif // __COLOR_H__
