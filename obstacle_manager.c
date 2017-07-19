#include <stdio.h>
#include <stdlib.h>
#include "obstacle_manager.h"

Obstacle_t _definedObstacleList[OBSTACLE_SIZE];

ObstacleId_t* loadObstaclesFile(const char* fileName) {
    FILE* inputFile;
    inputFile = fopen(fileName, "r");
    if (inputFile == NULL)
        return NULL;

    uint32_t fileSize = 0;
    uint32_t fileData[50];
    
    while(fscanf(inputFile, "%d", &fileData[fileSize]) != EOF)
        ++fileSize; 

    fclose(inputFile);

    ObstacleId_t* cache = (ObstacleId_t*)malloc(fileSize * sizeof(ObstacleId_t));
    
    uint32_t i;
    for(i = 0; i < fileSize; ++i) {
        cache[i] = (ObstacleId_t)fileData[i];
    }

    return cache;
}

bool runSolveObstacle(ObstacleId_t obstacleId) {
    return _definedObstacleList[obstacleId].pObstacleFunc();
}

void registerObstacle(ObstacleId_t obstacleId, bool (*pFunc)(void)) {
    _definedObstacleList[obstacleId].pObstacleFunc = pFunc;
}

