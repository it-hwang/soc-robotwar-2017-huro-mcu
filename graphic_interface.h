#ifndef __GRAPIC_INTERFACE_H__
#define __GRAPIC_INTERFACE_H__

#include <stdint.h>
#include "matrix.h"

typedef uint16_t PixelData_t;

typedef int16_t PixelCoordinate_t;
typedef struct {
    PixelCoordinate_t x;
    PixelCoordinate_t y;
} PixelLocation_t;

typedef Matrix16_t Screen_t;

// 화면의 너비/높이
#define DEFAULT_SCREEN_WIDTH    180
#define DEFAULT_SCREEN_HEIGHT   120
// 화면의 중심 좌표
#define DEFAULT_SCREEN_CENTER_X     (DEFAULT_SCREEN_WIDTH / 2)
#define DEFAULT_SCREEN_CENTER_Y     (DEFAULT_SCREEN_HEIGHT / 2)
// 카메라의 가로/세로 화각
#define CAMERA_HORIZONTAL_ANGLE_OF_VIEW     69.984040
#define CAMERA_VERTICAL_ANGLE_OF_VIEW       49.641083

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
