#ifndef __COLOR_RECOGNITION_H__
#define __COLOR_RECOGNITION_H__

#include "graphic_interface.h"
#include "lut.h"
#include "matrix.h"

#define COLOR_RECOGNITION_USE_TYPE_CHECKING     true


typedef enum {
    COLOR_NONE = 0,
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW,
    COLOR_ORANGE,
    MAX_COLOR
} Color_t;

typedef LookUpTable8_t ColorTable_t;
typedef Color_t (ColorTableFunc_t)(PixelData_t);


extern ColorTable_t* pColorTables[MAX_COLOR];
extern ColorTable_t* pCommonColorTable;
extern LookUpTable16_t* pRgab5515Table;
extern LookUpTable8_t* pGrayScaleTable;

void initializeColor(void);
void finalizeColor(void);

static inline Color_t getColorFromTable(ColorTable_t* pColorTable,
                                        PixelData_t pixelData) {
    return pColorTable->elements[pixelData];
}

static inline PixelData_t colorToRgab5515Data(Color_t color) {
    return pRgab5515Table->elements[color];
}

Matrix8_t* createColorMatrix(Screen_t* pScreen, ColorTable_t* pColorTable);


#endif // __COLOR_RECOGNITION_H__
