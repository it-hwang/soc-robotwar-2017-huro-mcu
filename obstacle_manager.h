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
} ObstacleId_t;

typedef struct {
    bool (*pObstacleFunc)(void);
}Obstacle_t;

ObstacleId_t* loadObstaclesFile(const char* fileName);

inline bool runSolveObstacle(ObstacleId_t obstacleId);
inline void registerObstacle(ObstacleId_t obstacleId, bool (*pFunc)(void));

#endif //__OBSTACLE_MANAGER_H__
