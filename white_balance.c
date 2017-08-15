#include <math.h>
#include <stdlib.h>
#include "white_balance.h"

#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))

static LookUpTable16_t* _pDefaultWhiteBalanceTable = NULL;

static double _aR, _bR, _cR;
static double _aG, _bG, _cG;
static double _aB, _bB, _cB;

static uint16_t _convertRealColor(PixelData_t pixelData) {
    uint16_t rgab5515Data = pixelData;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)&rgab5515Data;
    Rgba_t rgba;
    rgba.data = rgab5515ToRgbaData(pRgab5515);

    int r = rgba.r;
    int g = rgba.g;
    int b = rgba.b;
    int r2 = _aR * r * r + _bR * r + _cR;
    int g2 = _aG * g * g + _bG * g + _cG;
    int b2 = _aB * b * b + _bB * b + _cB;

    rgba.r = _MIN(_MAX(r2, 0), 255);
    rgba.g = _MIN(_MAX(g2, 0), 255);
    rgba.b = _MIN(_MAX(b2, 0), 255);
    
    pRgab5515->data = rgbaToRgab5515Data(&rgba);
    return pRgab5515->data;
}

LookUpTable16_t* createWhiteBalanceTable(Rgba_t* pInputColor, Rgba_t* pRealColor,
                                         const char* filePath, bool overwrite) {
    Rgba_t grayColor;
    grayColor.r = 128;
    grayColor.g = 128;
    grayColor.b = 128;
    if (pInputColor == NULL)
        pInputColor = &grayColor;
    if (pRealColor == NULL)
        pRealColor = &grayColor;

    int inputR = pInputColor->r;
    int inputG = pInputColor->g;
    int inputB = pInputColor->b;
    int realR = pRealColor->r;
    int realG = pRealColor->g;
    int realB = pRealColor->b;

    if (inputR == 0 || inputR == 255)
        return NULL;
    if (inputG == 0 || inputG == 255)
        return NULL;
    if (inputB == 0 || inputB == 255)
        return NULL;
    
    _aR = (double)(realR - inputR) / (inputR * (inputR - 255));
    _bR = 1.0 - (_aR * 255);
    _cR = 0.;
    _aG = (double)(realG - inputG) / (inputG * (inputG - 255));
    _bG = 1.0 - (_aG * 255);
    _cG = 0.;
    _aB = (double)(realB - inputB) / (inputB * (inputB - 255));
    _bB = 1.0 - (_aB * 255);
    _cB = 0.;

    uint32_t length = pow(2, sizeof(PixelData_t) * 8);
    return createLookUpTable16(filePath, (LookUpTableFunc16_t*)_convertRealColor, length, overwrite);
}

void applyWhiteBalance(Screen_t* pScreen, WhiteBalanceTable_t* pWhiteBalanceTable) {
    if (pScreen == NULL)
        return;
    if (pWhiteBalanceTable == NULL)
        return;

    int length = pScreen->width * pScreen->height;

    PixelData_t* pPixelData = pScreen->elements;
    for (int i = 0; i < length; ++i) {
        *pPixelData = pWhiteBalanceTable->elements[*pPixelData];
        pPixelData++;
    }
}

void setDefaultWhiteBalanceTable(WhiteBalanceTable_t* pWhiteBalanceTable) {
    if (_pDefaultWhiteBalanceTable != NULL)
        destroyLookUpTable16(_pDefaultWhiteBalanceTable);

    _pDefaultWhiteBalanceTable = pWhiteBalanceTable;
}

void resetDefaultWhiteBalanceTable(void) {
    setDefaultWhiteBalanceTable(NULL);
}

void applyDefaultWhiteBalance(Screen_t* pScreen) {
    applyWhiteBalance(pScreen, _pDefaultWhiteBalanceTable);
}

void readFpgaVideoDataWithWhiteBalance(Screen_t* pDefaultScreen) {
    readFpgaVideoData(pDefaultScreen);
    applyDefaultWhiteBalance(pDefaultScreen);
}
