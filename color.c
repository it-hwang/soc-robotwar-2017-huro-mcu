#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "color.h"
#include "color_model.h"


#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))

ColorTable_t* pCommonColorTable;
ColorTable_t* pOrangeColorTable;
ColorTable_t* pBlackColorTable;
LookUpTable16_t* pRgab5515Table;
LookUpTable8_t* pGrayScaleTable;


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

	if (i < 0.2 )
		return COLOR_BLACK;
	else if (i > 0.15 && s < 0.15)
		return COLOR_WHITE;
	else if (h >= 282  ||h<10)
		return COLOR_RED;
	else if (h >= 100 && h < 200)
		return COLOR_GREEN;
	else if (h >= 200 && h < 282)
		return COLOR_BLUE;
    else if(h>=10 && h<100)
		return COLOR_YELLOW;
    
    return COLOR_NONE;
}

Color_t _convertOrangeColorV1(PixelData_t pixelData) {
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

	if (i < 0.2 )
		return COLOR_BLACK;
	else if (i > 0.15 && s < 0.15)
		return COLOR_WHITE;
	else if(h>=3 && h<58)
		 return COLOR_ORANGE;
    else if (h >= 200 && h < 282)
		return COLOR_BLUE;
    else
        return COLOR_WHITE;
}

Color_t _convertBlackColorV1(PixelData_t pixelData) {
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

	if (i < 0.2 )
		return COLOR_BLACK;
    
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
            break;
        case COLOR_BLACK:
            rgba.r = 0x00;
            rgba.g = 0x00;
            rgba.b = 0x00;
            break;
        case COLOR_WHITE:
            rgba.r = 0xff;
            rgba.g = 0xff;
            rgba.b = 0xff;
            break;
        case COLOR_RED:
            rgba.r = 0xff;
            rgba.g = 0x00;
            rgba.b = 0x00;
            break;
        case COLOR_GREEN:
            rgba.r = 0x00;
            rgba.g = 0xff;
            rgba.b = 0x00;
            break;
        case COLOR_BLUE:
            rgba.r = 0x00;
            rgba.g = 0x00;
            rgba.b = 0xff;
            break;
        case COLOR_YELLOW:
            rgba.r = 0xff;
            rgba.g = 0xff;
            rgba.b = 0x00;
            break;
        case COLOR_ORANGE:
            rgba.r = 0xff;
            rgba.g = 0x7f;
            rgba.b = 0x27;
            break;
        default:
            rgba.r = 0x7f;
            rgba.g = 0x7f;
            rgba.b = 0x7f;
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
    static bool hasInitialized = false;
    if (hasInitialized)
        return;

    mkdir("./data", 0755);
    pCommonColorTable = _createColorTable("./data/common_v1.lut",
                            _convertCommonColorV1, false);
    pOrangeColorTable = _createColorTable("./data/orange_v1.lut",
                            _convertOrangeColorV1, false);
    pBlackColorTable = _createColorTable("./data/black_v1.lut",
                            _convertBlackColorV1, false);

    uint32_t length;
    length = pow(2, sizeof(PixelData_t) * 8);
    pRgab5515Table = createLookUpTable16("./data/rgab5515.lut",
                            _convertColorToRgb5515, MAX_COLOR, false);
    pGrayScaleTable = createLookUpTable8("./data/grayscale.lut",
                            _convertPixelDataToGrayColor, length, false);

    hasInitialized = true;
}

void finalizeColor(void) {
    _destroyColorTable(pCommonColorTable);
    _destroyColorTable(pOrangeColorTable);
    destroyLookUpTable8(pGrayScaleTable);
}

