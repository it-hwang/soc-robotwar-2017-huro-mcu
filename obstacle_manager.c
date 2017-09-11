#include <stdio.h>
#include <stdlib.h>
#include "obstacle_manager.h"

Obstacle_t _definedObstacleList[OBSTACLE_SIZE];

ObstacleSequence_t* loadObstaclesFile(const char* fileName) {
    FILE* inputFile;
    inputFile = fopen(fileName, "r");
    if (inputFile == NULL)
        return NULL;

    int fileSize = 0;
    int fileData[50];
    
    while(fscanf(inputFile, "%d", &fileData[fileSize]) != EOF)
        ++fileSize; 

    fclose(inputFile);

    ObstacleId_t* cache = (ObstacleId_t*)malloc(fileSize * sizeof(ObstacleId_t));
    
    for(int i = 0; i < fileSize; ++i) {
        cache[i] = (ObstacleId_t)fileData[i];
    }

    ObstacleSequence_t* pObstacleSequence = (ObstacleSequence_t*)malloc(sizeof(ObstacleSequence_t));
    pObstacleSequence->elements = cache;
    pObstacleSequence->size = fileSize;

    return pObstacleSequence;
}

void destroyObstacleSequence(ObstacleSequence_t* pObstacleSequence) {
    if (!pObstacleSequence)
        return;

    free(pObstacleSequence->elements);
    free(pObstacleSequence);
}

bool runSolveObstacle(ObstacleId_t obstacleId) {
    return _definedObstacleList[obstacleId].pObstacleFunc();
}

void registerObstacle(ObstacleId_t obstacleId, bool (*pFunc)(void)) {
    _definedObstacleList[obstacleId].pObstacleFunc = pFunc;
}

