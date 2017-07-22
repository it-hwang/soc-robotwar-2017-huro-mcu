#ifndef __COLOR_RECOGNITION_H__
#define __COLOR_RECOGNITION_H__

#include "graphic_interface.h"
#include "lut.h"
#include "matrix.h"

#define COLOR_RECOGNITION_USE_TYPE_CHECKING     true


typedef enum {
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


extern ColorTable_t* pCommonColorTable;
extern ColorTable_t* pOrangeColorTable;
extern LookUpTable8_t* pGrayScaleTable;

void initializeColor(void);
void finalizeColor(void);

static inline Color_t getColorFromTable(ColorTable_t* pColorTable,
                                        PixelData_t pixelData) {
    return pColorTable->elements[pixelData];
}


#endif // __COLOR_RECOGNITION_H__
