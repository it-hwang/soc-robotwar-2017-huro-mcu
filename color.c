#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "color.h"
#include "color_model.h"


#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))


ColorTable_t* pColorTables[MAX_COLOR];
ColorTable_t* pCommonColorTable;
LookUpTable16_t* pRgab5515Table;
LookUpTable8_t* pGrayScaleTable;

bool _hasInitialized = false;


void _calculateHSI(PixelData_t pixelData, float* pH, float* pS, float* pI) {
    uint16_t rgab5515Data = pixelData;
    uint32_t rgbaData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    Rgba_t* pRgba = (Rgba_t*)&rgbaData;
    pRgba->data = rgab5515ToRgbaData(pRgab5515);

    float r = (float)pRgba->r / 255;
    float g = (float)pRgba->g / 255;
    float b = (float)pRgba->b / 255;
    float h;    // hue
    float s;    // saturation
    float i;    // intensity
    float max = _MAX(_MAX(r, g), b);
    float min = _MIN(_MIN(r, g), b);
    float c = max - min;

    i = (r + g + b) / 3;
    if (c == 0) h = 0;
    else if (max == r) h = 60 * fmodf(((g - b) / c), 6);
    else if (max == g) h = 60 * (((b - r) / c) + 2);
    else if (max == b) h = 60 * (((r - g) / c) + 4);
    else h = 0;
    if (c == 0) s = 0;
    else s = 1 - min / i;

    *pH = h;
    *pS = s;
    *pI = i;
}


void _calculateHSV(PixelData_t pixelData, float* pH, float* pS, float* pV) {
    uint16_t rgab5515Data = pixelData;
    uint32_t rgbaData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    Rgba_t* pRgba = (Rgba_t*)&rgbaData;
    pRgba->data = rgab5515ToRgbaData(pRgab5515);

    float r = (float)pRgba->r / 255;
    float g = (float)pRgba->g / 255;
    float b = (float)pRgba->b / 255;
    float h;    // hue
    float s;    // saturation
    float v;    // value
    float max = _MAX(_MAX(r, g), b);
    float min = _MIN(_MIN(r, g), b);
    float c = max - min;

    if (c == 0) h = 0;
    else if (max == r) h = 60 * fmodf(((g - b) / c), 6);
    else if (max == g) h = 60 * (((b - r) / c) + 2);
    else if (max == b) h = 60 * (((r - g) / c) + 4);
    else h = 0;

    if (max == 0) s = 0;
    else s = c / max;

    v = max;

    *pH = h;
    *pS = s;
    *pV = v;
}


Color_t _convertCommonColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    uint32_t rgbaData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    Rgba_t* pRgba = (Rgba_t*)&rgbaData;
    pRgba->data = rgab5515ToRgbaData(pRgab5515);

    float r = (float)pRgba->r / 255;
    float g = (float)pRgba->g / 255;
    float b = (float)pRgba->b / 255;
    float h;    // hue
    float s;    // saturation
    float i;    // intensity
    float max = _MAX(_MAX(r, g), b);
    float min = _MIN(_MIN(r, g), b);
    float c = max - min;

    i = (r + g + b) / 3;
    if (c == 0) h = 0;
    else if (max == r) h = 60 * fmodf(((g - b) / c), 6);
    else if (max == g) h = 60 * (((b - r) / c) + 2);
    else if (max == b) h = 60 * (((r - g) / c) + 4);
    else h = 0;
    if (c == 0) s = 0;
    else s = 1 - min / i;

    if (i < 0.2)
        return COLOR_BLACK;
    else if (i > 0.15 && s < 0.15)
        return COLOR_WHITE;
    else if (h >= 282 || h<10)
        return COLOR_RED;
    else if (h >= 100 && h < 200)
        return COLOR_GREEN;
    else if (h >= 200 && h < 282)
        return COLOR_BLUE;
    else if (h >= 10 && h<100)
        return COLOR_YELLOW;

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertBlackColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    //float a = 0.04;
    //float b = 0.18;
    //float c = 0.14;
    float a = 0.02;
    float b = 0.28;
    float c = 0.18;

    bool isGray = false;
    if (s <= b)
        isGray = true;
    else if (v <= (a / (s - b)) + c)
        isGray = true;

    if (isGray && v < 0.40)
        return COLOR_BLACK;

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertBlackColorV2(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    //float a = 0.04;
    //float b = 0.18;
    //float c = 0.14;
    float a = 0.02;
    float b = 0.28;
    float c = 0.18;

    bool isGray = false;
    if (s <= b)
        isGray = true;
    else if (v <= (a / (s - b)) + c)
        isGray = true;

    if (isGray && v < 0.30)
        return COLOR_BLACK;

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertBlack2ColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    //float a = 0.04;
    //float b = 0.18;
    //float c = 0.14;
    float a = 0.02;
    float b = -0.86;
    float c = 0.42;

    bool isGray = false;
    if (s <= b)
        isGray = true;
    else if (v <= (a / (s - b)) + c)
        isGray = true;

    if (isGray && v < 0.40)
        return COLOR_BLACK;

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertWhiteColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.04;
    float b = 0.28;
    float c = 0.14;

    bool isGray = false;
    if (s <= b)
        isGray = true;
    else if (v <= (a / (s - b)) + c)
        isGray = true;

    if (isGray && v >= 0.50)
        return COLOR_WHITE;

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertWhiteColorV2(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.04;
    float b = 0.38;
    float c = -0.20;

    bool isGray = false;
    if (s <= b)
        isGray = true;
    else if (v <= (a / (s - b)) + c)
        isGray = true;

    if (isGray && v >= 0.40)
        return COLOR_WHITE;

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertRedColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.02;
    float b = 0.28;
    float c = 0.12;

    if (h < 20 || h >= 330) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_RED;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertGreenColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.10;
    float b = 0.08;
    float c = -0.04;

    if (h >= 90 && h < 160) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_GREEN;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertBlueColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.16;
    float b = 0.00;
    float c = -0.04;

    if (h >= 180 && h < 275) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_BLUE;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertBlueColorV2(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.06;
    float b = 0.24;
    float c = -0.01;

    if (h >= 180 && h < 275) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_BLUE;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertYellowColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.12;
    float b = 0.12;
    float c = -0.06;

    if (h >= 26 && h < 70) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_YELLOW;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertYellowColorV2(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.12;
    float b = 0.20;
    float c = 0.00;

    if (h >= 34 && h < 65) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_YELLOW;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertOrangeColorV1(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.20;
    float b = 0.10;
    float c = 0.10;

    if (h >= 350 || h < 30) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_ORANGE;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertOrangeColorV2(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.20;
    float b = 0.25;
    float c = 0.10;

    if (h >= 350 || h < 30) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_ORANGE;
    }

    return COLOR_NONE;
}

// y=1/x ?????? ??????
Color_t _convertOrangeColorV3(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    if (pRgab5515->a == 0) return COLOR_NONE;

    float h;    // hue
    float s;    // saturation
    float v;    // value
    _calculateHSV(pixelData, &h, &s, &v);

    // i > ((a / (s - b)) + c)
    float a = 0.06;
    float b = 0.3;
    float c = 0.12;

    if (h >= 350 || h < 43) {
        if (s <= b)
            return COLOR_NONE;
        else if (v >(a / (s - b)) + c)
            return COLOR_ORANGE;
    }

    return COLOR_NONE;
}


uint8_t _convertPixelDataToGrayColor(uint32_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    return rgab5515ToGrayscale(pRgab5515);
}

uint16_t _convertColorToRgb5515(uint32_t colorData) {
    Color_t color = colorData;
    Rgba_t rgba;

    switch (color) {
    case COLOR_NONE:
        rgba.r = 0x7f;
        rgba.g = 0x7f;
        rgba.b = 0x7f;
        rgba.a = 0x00;
        break;
    case COLOR_BLACK:
        rgba.r = 0x00;
        rgba.g = 0x00;
        rgba.b = 0x00;
        rgba.a = 0xff;
        break;
    case COLOR_WHITE:
        rgba.r = 0xff;
        rgba.g = 0xff;
        rgba.b = 0xff;
        rgba.a = 0xff;
        break;
    case COLOR_RED:
        rgba.r = 0xff;
        rgba.g = 0x00;
        rgba.b = 0x00;
        rgba.a = 0xff;
        break;
    case COLOR_GREEN:
        rgba.r = 0x00;
        rgba.g = 0xff;
        rgba.b = 0x00;
        rgba.a = 0xff;
        break;
    case COLOR_BLUE:
        rgba.r = 0x00;
        rgba.g = 0x00;
        rgba.b = 0xff;
        rgba.a = 0xff;
        break;
    case COLOR_YELLOW:
        rgba.r = 0xff;
        rgba.g = 0xff;
        rgba.b = 0x00;
        rgba.a = 0xff;
        break;
    case COLOR_ORANGE:
        rgba.r = 0xff;
        rgba.g = 0x7f;
        rgba.b = 0x27;
        rgba.a = 0xff;
        break;
    default:
        rgba.r = 0x7f;
        rgba.g = 0x7f;
        rgba.b = 0x7f;
        rgba.a = 0x00;
    }

    return rgbaToRgab5515Data(&rgba);
}


ColorTable_t* _createColorTable(const char* filePath,
    ColorTableFunc_t* pFunc, bool overwrite) {
    uint32_t length = pow(2, sizeof(PixelData_t) * 8);
    return createLookUpTable8(filePath, (LookUpTableFunc8_t*)pFunc, length, overwrite);
}

void _destroyColorTable(ColorTable_t* pColorTable) {
    destroyLookUpTable8(pColorTable);
}


void initializeColor(void) {
    if (_hasInitialized)
        return;

    mkdir("./data", 0755);
    pCommonColorTable = _createColorTable("./data/common_v1.lut",
        _convertCommonColorV1, false);
    // pColorTables[COLOR_BLACK] = _createColorTable("./data/black_v1.lut",
    //                                             _convertBlackColorV1, false);
    pColorTables[COLOR_BLACK] = _createColorTable("./data/black_v2.lut",
        _convertBlackColorV2, false);
    // pColorTables[COLOR_WHITE] = _createColorTable("./data/white_v2.lut",
    //                                             _convertWhiteColorV2, false);
    pColorTables[COLOR_WHITE] = _createColorTable("./data/white_v2.lut",
        _convertWhiteColorV2, false);
    pColorTables[COLOR_RED] = _createColorTable("./data/red_v1.lut",
        _convertRedColorV1, false);
    pColorTables[COLOR_GREEN] = _createColorTable("./data/green_v1.lut",
        _convertGreenColorV1, false);
    // pColorTables[COLOR_BLUE] = _createColorTable("./data/blue_v1.lut",
    //     _convertBlueColorV1, false);
    pColorTables[COLOR_BLUE] = _createColorTable("./data/blue_v2.lut",
    _convertBlueColorV2, false);
    // pColorTables[COLOR_YELLOW] = _createColorTable("./data/yellow_v1.lut",
    //                                             _convertYellowColorV1, false);
    pColorTables[COLOR_YELLOW] = _createColorTable("./data/yellow_v2.lut",
        _convertYellowColorV2, false);
    // pColorTables[COLOR_ORANGE] = _createColorTable("./data/orange_v1.lut",
    //                                             _convertOrangeColorV1, false);
    // pColorTables[COLOR_ORANGE] = _createColorTable("./data/orange_v2.lut",
    //     _convertOrangeColorV2, false);
    pColorTables[COLOR_ORANGE] = _createColorTable("./data/orange_v3.lut",
        _convertOrangeColorV3, false);
    pColorTables[COLOR_BLACK2] = _createColorTable("./data/black2_v1.lut",
        _convertBlack2ColorV1, false);

    uint32_t length;
    length = pow(2, sizeof(PixelData_t) * 8);
    pRgab5515Table = createLookUpTable16("./data/rgab5515.lut",
        _convertColorToRgb5515, MAX_COLOR, false);
    pGrayScaleTable = createLookUpTable8("./data/grayscale.lut",
        _convertPixelDataToGrayColor, length, false);

    _hasInitialized = true;
}

void finalizeColor(void) {
    if (!_hasInitialized)
        return;

    _destroyColorTable(pCommonColorTable);
    _destroyColorTable(pColorTables[COLOR_BLACK]);
    _destroyColorTable(pColorTables[COLOR_WHITE]);
    _destroyColorTable(pColorTables[COLOR_RED]);
    _destroyColorTable(pColorTables[COLOR_GREEN]);
    _destroyColorTable(pColorTables[COLOR_BLUE]);
    _destroyColorTable(pColorTables[COLOR_YELLOW]);
    _destroyColorTable(pColorTables[COLOR_ORANGE]);
    _destroyColorTable(pColorTables[COLOR_BLACK2]);
    destroyLookUpTable16(pRgab5515Table);
    destroyLookUpTable8(pGrayScaleTable);
}


Matrix8_t* createColorMatrix(Screen_t* pScreen, ColorTable_t* pColorTable) {
    PixelCoordinate_t width = pScreen->width;
    PixelCoordinate_t height = pScreen->height;
    Matrix8_t* pColorMatrix = createMatrix8(width, height);
    int length = width * height;
    int i;
    Color_t* pColorData = pColorMatrix->elements;
    PixelData_t* pPixelData = pScreen->elements;

    for (i = 0; i < length; ++i) {
        *pColorData = getColorFromTable(pColorTable, *pPixelData);
        pColorData++;
        pPixelData++;
    }

    return pColorMatrix;
}


void drawColorMatrix(Screen_t* pScreen, Matrix8_t* pColorMatrix) {
    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    PixelData_t* pScreenPixel = pScreen->elements;
    Color_t* pColorPixel = pColorMatrix->elements;

    for (int i = 0; i < length; ++i) {
        *pScreenPixel = colorToRgab5515Data(*pColorPixel);
        pScreenPixel++;
        pColorPixel++;
    }
}

Matrix8_t* overlapColorMatrix(Matrix8_t* pSourceColorMatrix, Matrix8_t* pTargetColorMatrix) {
    if (pSourceColorMatrix->width != pTargetColorMatrix->width)
        return NULL;

    if (pSourceColorMatrix->height != pTargetColorMatrix->height)
        return NULL;

    int width = pSourceColorMatrix->width;
    int height = pSourceColorMatrix->height;
    int length = width * height;

    Matrix8_t* returnMatrix = createMatrix8(width, height);
    memset(returnMatrix->elements, 0, (returnMatrix->height * returnMatrix->width) * sizeof(uint8_t));

    for (int i = 0; i < length; ++i) {
        returnMatrix->elements[i] = pSourceColorMatrix->elements[i];
        if (returnMatrix->elements[i] == 0)
            returnMatrix->elements[i] = pTargetColorMatrix->elements[i];
    }

    return returnMatrix;
}
