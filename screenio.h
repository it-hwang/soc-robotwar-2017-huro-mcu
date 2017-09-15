#ifndef __SCREENIO_H__
#define __SCREENIO_H__

#include "graphic_interface.h"


void printScreen(Screen_t* pScreen);
bool writeScreen(Screen_t* pScreen, const char* filePath);
Screen_t* scanScreen(const char* filePath);
void saveScreen(Screen_t* pScreen, const char* filePath);
Screen_t* loadScreen(const char* filePath);

void printFpgaVideoData(void);

#endif // __SCREENIO_H__
