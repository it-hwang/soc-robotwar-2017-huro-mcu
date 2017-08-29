#ifndef __GOLF_H__
#define __GOLF_H__

#include <stdbool.h>

bool golfMain(void);

// 장애물과의 거리를 밀리미터 단위로 측정한다.
int measureGolfDistance(void);
// 장애물을 해결한다.
bool solveGolf(void);

#endif //__GOLF_H__
