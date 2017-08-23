#ifndef __MINE_H__
#define __MINE_DE__MINE_H__TECTION_H__

#include <stdbool.h>

bool mineMain(void);

// 장애물과의 거리를 밀리미터 단위로 측정한다.
int measureMineDistance(void);
// 장애물을 해결한다.
bool solveMine(void);

#endif // __MINE_H__
