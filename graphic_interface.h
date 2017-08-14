#ifndef __GRAPIC_INTERFACE_H__
#define __GRAPIC_INTERFACE_H__

#include <stdint.h>
#include "matrix.h"

typedef uint16_t PixelData_t;

typedef uint8_t PixelCoordinate_t;
typedef struct {
    PixelCoordinate_t x;
    PixelCoordinate_t y;
} PixelLocation_t;

typedef Matrix16_t Screen_t;

extern const PixelCoordinate_t DEFAULT_SCREEN_WIDTH;
extern const PixelCoordinate_t DEFAULT_SCREEN_HEIGHT;

int openGraphicInterface(void);
void closeGraphicInterface(void);

void enableDirectCameraDisplay(void);
void disableDirectCameraDisplay(void);

Screen_t* createScreen(PixelCoordinate_t width, PixelCoordinate_t height);
Screen_t* createDefaultScreen(void);
void destroyScreen(Screen_t* pScreen);

void readFpgaVideoData(Screen_t* pDefaultScreen);
void displayScreen(Screen_t* pDefaultScreen);

#endif // __GRAPIC_INTERFACE_H__
