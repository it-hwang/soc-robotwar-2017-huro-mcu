#ifndef __CHECK_CENTER_H__
#define __CHECK_CENTER_H__

#include <stdbool.h>

#include "line_detection.h"

bool checkCenter(void);
bool checkAngle(void);
Line_t* captureLine(Screen_t* pScreen);

#endif //__CHECK_CENTER_H__
