#include <stdlib.h>

#include "graphic_interface.h"
#include "graphic_api.h"
#include "color_model.h"


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
    // 모든 색상을 유효한 색상으로 변환
    int length = pDefaultScreen->height * pDefaultScreen->width;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)pDefaultScreen->elements;
    for (int i = 0; i < length; ++i) {
        pRgab5515->a = 1;
        pRgab5515++;
    }
    // 왼쪽 픽셀 제거
    for(int i = 0; i < pDefaultScreen->height; ++i){
        pDefaultScreen->elements[i * pDefaultScreen->width + 0] = 0x7bcf;
        pDefaultScreen->elements[i * pDefaultScreen->width + 1] = 0x7bcf;
    }
}

static void _convertRgab5515MatrixToRgb565Matrix(Screen_t* pScreen) {
    int length = pScreen->height * pScreen->width;
    
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)pScreen->elements;
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
