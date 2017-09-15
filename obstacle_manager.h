#ifndef __OBSTACLE_MANAGER_H__
#define __OBSTACLE_MANAGER_H__

#include <stdbool.h>
#include <stdint.h>

//장애물 번호 정의
enum {
    OBSTACLE_VERTICAL_BARRICADE,
    OBSTACLE_RED_BRIDGE,
    OBSTACLE_MINE,
    OBSTACLE_HURDLE,
    OBSTACLE_CORNER,
    OBSTACLE_BLUE_GATE,
    OBSTACLE_GREEN_BRIDGE,
    OBSTACLE_GOLF,
    OBSTACLE_TRAP,
    OBSTACLE_HORIZONTAL_BARRICADE,
    OBSTACLE_SIZE
};
typedef uint8_t ObstacleId_t;

typedef struct {
    bool (*pObstacleFunc)(void);
} Obstacle_t;

typedef struct {
    ObstacleId_t* elements;
    int size;
} ObstacleSequence_t;

ObstacleSequence_t* loadObstaclesFile(const char* fileName);
void destroyObstacleSequence(ObstacleSequence_t* pObstacleSequence);

inline bool runSolveObstacle(ObstacleId_t obstacleId);
inline void registerObstacle(ObstacleId_t obstacleId, bool (*pFunc)(void));

#endif //__OBSTACLE_MANAGER_H__
