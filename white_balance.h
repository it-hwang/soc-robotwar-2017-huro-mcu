#ifndef __WHITE_BALANCE_H__
#define __WHITE_BALANCE_H__

#include <stdbool.h>
#include "lut.h"
#include "color_model.h"
#include "graphic_interface.h"

// pInputColor: 입력 색상
// pRealColor : 실제 색상(목표 색상)
LookUpTable16_t* createWhiteBalanceTable(Rgab5515_t* pInputColor, Rgab5515_t* pRealColor,
                                         const char* filePath, bool overwrite);

void applyWhiteBalance(Screen_t* pScreen, LookUpTable16_t* pWhiteBalanceTable);

void setDefaultWhiteBalanceTable(LookUpTable16_t* pWhiteBalanceTable);
void resetDefaultWhiteBalanceTable(void);
void applyDefaultWhiteBalance(Screen_t* pScreen);
void readFpgaVideoDataWithWhiteBalance(Screen_t* pDefaultScreen);


#endif //__WHITE_BALANCE_H__
