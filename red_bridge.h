#ifndef __RED_BRIDEGE_H__
#define __RED_BRIDEGE_H__

#include <stdbool.h>

bool redBridgeMain(void);

// 장애물과의 거리를 밀리미터 단위로 측정한다.
int measureRedBridgeDistance(void);
// 장애물을 해결한다.
bool solveRedBridge(void);


#endif // __RED_BRIDEGE_H__
