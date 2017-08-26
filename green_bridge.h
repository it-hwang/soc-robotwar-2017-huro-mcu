#ifndef __GREEN_BRIDEGE_H__
#define __GREEN_BRIDEGE_H__

#include <stdbool.h>

bool greenBridgeMain(void);

// 장애물과의 거리를 밀리미터 단위로 측정한다.
int measureGreenBridgeDistance(void);
// 장애물을 해결한다.
bool solveGreenBridge(void);


#endif // __GREEN_BRIDEGE_H__
