#ifndef __VERTICAL_BARRICADE_H__
#define __VERTICAL_BARRICADE_H__

#include <stdbool.h>

bool verticalBarricadeMain(void);

// 장애물과의 거리를 밀리미터 단위로 측정한다.
int measureVerticalBarricadeDistance(void);
// 장애물을 해결한다.
bool solveVerticalBarricade(void);

#endif // __VERTICAL_BARRICADE_H__
