#ifndef __WHITE_BALANCE_H__
#define __WHITE_BALANCE_H__

#include "lut.h"
#include "color_model.h"
#include "graphic_interface.h"

// pInputColor: �Է� ����
// pRealColor : ���� ����(��ǥ ����)
LookUpTable16_t* createWhiteBalanceTable(Rgab5515_t* pInputColor, Rgab5515_t* pRealColor);

void setDefaultWhiteBalanceTable(LookUpTable16_t* pWhiteBalanceTable);

void applyWhiteBalance(Screen_t* pScreen, LookUpTable16_t* pWhiteBalanceTable);
void applyDefaultWhiteBalance(Screen_t* pScreen);


#endif //__WHITE_BALANCE_H__
