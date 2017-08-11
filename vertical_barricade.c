#include <stdlib.h>
#include "vertical_barricade.h"
#include "color.h"
#include "color_model.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"


// pScreen에 pColorMatrix를 표시한다.
static void _drawColorMatrix(Screen_t* pScreen, Matrix8_t* pColorMatrix) {
    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    int i;
    PixelData_t* pScreenPixel = pScreen->elements;
    Color_t* pColorPixel = pColorMatrix->elements;

    for (i = 0; i < length; ++i) {
        *pScreenPixel = colorToRgab5515Data(*pColorPixel);
        pScreenPixel++;
        pColorPixel++;
    }
}

// pScreen에 pColorMatrix에 있는 색 중, COLOR_NONE이 아닌 것들만 표시한다.
static void _drawColorMatrixAdditive(Screen_t* pScreen, Matrix8_t* pColorMatrix) {
    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    int i;
    PixelData_t* pScreenPixel = pScreen->elements;
    Color_t* pColorPixel = pColorMatrix->elements;

    for (i = 0; i < length; ++i) {
        if (*pColorPixel != COLOR_NONE)
            *pScreenPixel = colorToRgab5515Data(*pColorPixel);
        pScreenPixel++;
        pColorPixel++;
    }
}


bool verticalBarricadeMain(void) {
    for (int i = 0; i < 10; ++i) {
        measureVerticalBarricadeDistance();
    }

    return true;
}

static Object_t* _searchVerticalBarricade(void) {
    static const float MIN_RECTANGLE_CORRELATION = 0.75;

    Object_t* pObject = NULL;

    return pObject;
}

int measureVerticalBarricadeDistance(void) {
    setHead(0, -35);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoData(pScreen);

    Matrix8_t* pBlackMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLACK]);
    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);
    /*
    applyFastErosionToMatrix8(pBlackMatrix, 1);
    applyFastDilationToMatrix8(pBlackMatrix, 2);
    applyFastErosionToMatrix8(pBlackMatrix, 1);

    applyFastErosionToMatrix8(pYellowMatrix, 1);
    applyFastDilationToMatrix8(pYellowMatrix, 2);
    applyFastErosionToMatrix8(pYellowMatrix, 1);
    */
    _drawColorMatrix(pScreen, pBlackMatrix);
    _drawColorMatrixAdditive(pScreen, pYellowMatrix);
    destroyMatrix8(pBlackMatrix);
    destroyMatrix8(pYellowMatrix);

    displayScreen(pScreen);
    destroyScreen(pScreen);

    sdelay(5);

    return 0;
}
