#ifndef __WHITE_BALANCE_H__
#define __WHITE_BALANCE_H__

#include <stdbool.h>
#include "lut.h"
#include "color_model.h"
#include "graphic_interface.h"


typedef LookUpTable16_t WhiteBalanceTable_t;

// pInputColor: 입력 색상
// pRealColor : 실제 색상(목표 색상)
WhiteBalanceTable_t* createWhiteBalanceTable(Rgba_t* pInputColor, Rgba_t* pRealColor,
                                             const char* filePath, bool overwrite);

void applyWhiteBalance(Screen_t* pScreen, WhiteBalanceTable_t* pWhiteBalanceTable);

void setDefaultWhiteBalanceTable(WhiteBalanceTable_t* pWhiteBalanceTable);
void resetDefaultWhiteBalanceTable(void);
void applyDefaultWhiteBalance(Screen_t* pScreen);
void readFpgaVideoDataWithWhiteBalance(Screen_t* pDefaultScreen);


#endif //__WHITE_BALANCE_H__
