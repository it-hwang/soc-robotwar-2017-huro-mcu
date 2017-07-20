#include <stdlib.h>

#include "graphic_interface.h"
#include "graphic_api.h"

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


Screen_t* createScreen(PixelCoordinate_t width, PixelCoordinate_t height) {
    int nPixels = height * width;
    Screen_t* pScreen = (Screen_t*)malloc(sizeof(Screen_t));
    pScreen->pixels = (PixelData_t*)malloc(nPixels * sizeof(PixelData_t));
    pScreen->height = height;
    pScreen->width = width;

    return pScreen;
}

Screen_t* createDefaultScreen(void) {
    return createScreen(DEFAULT_SCREEN_HEIGHT, DEFAULT_SCREEN_WIDTH);
}

void destroyScreen(Screen_t* pScreen) {
    free(pScreen->pixels);
    free(pScreen);
}


void readFpgaVideoData(Screen_t* pDefaultScreen) {
    read_fpga_video_data(pDefaultScreen->pixels);
}

void displayScreen(Screen_t* pDefaultScreen) {
    clear_screen();
    draw_fpga_video_data_full(pDefaultScreen->pixels);
    flip();
}
