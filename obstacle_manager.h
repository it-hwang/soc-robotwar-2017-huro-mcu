#ifndef __OBSTACLE_MANAGER_H__
#define __OBSTACLE_MANAGER_H__

#include <stdbool.h>
#include <stdint.h>

//장애물 번호 정의
typedef enum {
    OBSTACLE_ONE,
    OBSTACLE_TWO,
    OBSTACLE_THREE,
    OBSTACLE_SIZE
} OBSTACLEID, *LPOBSTACLEID;

typedef struct {
    bool (*pObstacleFunc)(void);
}OBSTACLE,*LPOBSTACLE;

LPOBSTACLEID loadObstaclesFile(const char* fileName);

inline bool runSolveObstacle(OBSTACLEID obstacleId);
inline void registerObstacle(OBSTACLEID obstacleId, bool (*pFunc)(void));

#endif //__OBSTACLE_MANAGER_H__
