#include <stdio.h>
#include <stdlib.h>
#include "obstacle_manager.h"

LPOBSTACLEID loadObstaclesFile(const char* fileName) {
    FILE* inputFile;
	inputFile = fopen(fileName, "r");
	if (inputFile == NULL)
		return NULL;

    uint32_t fileSize = 0;
    uint32_t fileData[50];
    
    while(fscanf(inputFile, "%d", &fileData[fileSize]) != EOF) ++fileSize; 

    fclose(inputFile);

    LPOBSTACLEID cache = (LPOBSTACLE)malloc(fileSize * sizeof(OBSTACLEID));
    
    uint32_t i;
    for(i = 0; i < fileSize; ++i) {
        cache[i] = fileData[i];
    }

    return cache;
}

bool runSolveObstacle(OBSTACLEID obstacleId) {

}

void registerObstacle(OBSTACLEID obstacleId, bool (*pObstacleFunc)(void)) {

}
