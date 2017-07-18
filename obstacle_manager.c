#include <stdio.h>
#include <stdlib.h>
#include "obstacle_manager.h"

OBSTACLE _definedObstacleList[OBSTACLE_SIZE];

LPOBSTACLEID loadObstaclesFile(const char* fileName) {
    FILE* inputFile;
	inputFile = fopen(fileName, "r");
	if (inputFile == NULL)
		return NULL;

    uint32_t fileSize = 0;
    uint32_t fileData[50];
    
    while(fscanf(inputFile, "%d", &fileData[fileSize]) != EOF)
        ++fileSize; 

    fclose(inputFile);

    LPOBSTACLEID cache = (LPOBSTACLEID)malloc(fileSize * sizeof(OBSTACLEID));
    
    uint32_t i;
    for(i = 0; i < fileSize; ++i) {
        cache[i] = (OBSTACLEID)fileData[i];
    }

    return cache;
}

bool runSolveObstacle(OBSTACLEID obstacleId) {
    return _definedObstacleList[obstacleId].pObstacleFunc();
}

void registerObstacle(OBSTACLEID obstacleId, bool (*pFunc)(void)) {
    _definedObstacleList[obstacleId].pObstacleFunc = pFunc;
}

