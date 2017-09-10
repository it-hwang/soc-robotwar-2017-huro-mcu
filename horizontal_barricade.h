#ifndef __HORIZONTAL_BARRICADE_H__
#define __HORIZONTAL_BARRICADE_H__

#include <stdbool.h>

bool horizontalBarricadeMain(void);

// 장애물과의 거리를 밀리미터 단위로 측정한다.
int measureHorizontalBarricadeDistance(void);
// 장애물을 해결한다.
bool solveHorizontalBarricade(void);

#endif // __HORIZONTAL_BARRICADE_H__
