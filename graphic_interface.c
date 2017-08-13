#include <stdlib.h>

#include "graphic_interface.h"
#include "graphic_api.h"
#include "color_model.h"

const PixelCoordinate_t DEFAULT_SCREEN_WIDTH = 180;
const PixelCoordinate_t DEFAULT_SCREEN_HEIGHT = 120;


int openGraphicInterface(void) {
    int status = open_graphic();
    if (status == 0)
        direct_camera_display_off();
    
    return status;
}

void closeGraphicInterface(void) {
    direct_camera_display_on();
    close_graphic();
}

void enableDirectCameraDisplay(void) {
    direct_camera_display_on();
}

void disableDirectCameraDisplay(void) {
    direct_camera_display_off();
}


Screen_t* createScreen(PixelCoordinate_t width, PixelCoordinate_t height) {
    return createMatrix16(width, height);
}

Screen_t* createDefaultScreen(void) {
    return createScreen(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT);
}

void destroyScreen(Screen_t* pScreen) {
    destroyMatrix16(pScreen);
}


void readFpgaVideoData(Screen_t* pDefaultScreen) {
    read_fpga_video_data(pDefaultScreen->elements);
}

static void _convertRgab5515MatrixToRgb565Matrix(Screen_t* pScreen) {
    int length = pScreen->height * pScreen->width;
    
    Rgab5515_t* pRgab5515 = pScreen->elements;
    for (int i = 0; i < length; ++i) {
        pRgab5515->a = pRgab5515->g;
        pRgab5515++;
    }
}

void displayScreen(Screen_t* pDefaultScreen) {
    Screen_t* pScreen = cloneMatrix16(pDefaultScreen);
    _convertRgab5515MatrixToRgb565Matrix(pScreen);

    clear_screen();
    draw_fpga_video_data_full(pScreen->elements);
    flip();

    destroyMatrix16(pScreen);
}
