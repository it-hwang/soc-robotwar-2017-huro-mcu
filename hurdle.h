#ifndef __HURDLE_H__
#define __HURDLE_H__

#include <stdbool.h>

bool hurdleMain(void);

// 장애물과의 거리를 밀리미터 단위로 측정한다.
int measureHurdleDistance(void);
// 장애물을 해결한다.
bool solveHurdle(void);

#endif // __HURDLE_H__
