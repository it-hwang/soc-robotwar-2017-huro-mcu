#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "object_detection.h"
#include "graphic_interface.h"
#include "matrix.h"

#define _LABEL_SIZE 1001

void _sortArray(uint16_t* array, int size);
uint8_t _searchAdjacencyLabel(uint16_t* array, int size);

ObjectList_t* _detectObjectsLocation(Matrix8_t* pMatrix,
                                     Matrix16_t* pOutputLabelMatrix) {
    static uint16_t equalLabelList[_LABEL_SIZE];
    static int labelCntList[_LABEL_SIZE];

    int x;
    int y;
    int width = pMatrix->width;
    int height = pMatrix->height;
    uint8_t* pixels = pMatrix->elements;

    Matrix16_t* pLabelMatrix = createMatrix16(width, height);
    memset(pLabelMatrix->elements, 0, (height * width) * sizeof(uint16_t));
    memset(equalLabelList, 0, sizeof(equalLabelList));
    memset(labelCntList, 0, sizeof(labelCntList));
    int lastLabel = 0;

    Object_t* labelLocationInfo = (Object_t*)malloc(_LABEL_SIZE * sizeof(Object_t));

    for(y = 0; y < height; ++y) {
        for(x = 0; x < width; ++x) {
            int index = y * width + x;
            uint8_t pixelData = pixels[index];

            if(pixelData != 0) {
                uint16_t adjacencyLabels[4];
                uint8_t listSize;
                
                if (y > 0)
                    adjacencyLabels[0] = pLabelMatrix->elements[(y-1)*width + x];
                else
                    adjacencyLabels[0] = 0;
                if (x > 0)
                    adjacencyLabels[1] = pLabelMatrix->elements[y*width + x-1];
                else
                    adjacencyLabels[1] = 0;
                if (y > 0 && x > 0)
                    adjacencyLabels[2] = pLabelMatrix->elements[(y-1)*width + x-1];
                else
                    adjacencyLabels[2] = 0;
                if (y > 0 && x < width - 1)
                    adjacencyLabels[3] = pLabelMatrix->elements[(y-1)*width + x+1];
                else
                    adjacencyLabels[3] = 0;
                
                _sortArray(adjacencyLabels, 4);
                
                listSize = _searchAdjacencyLabel(adjacencyLabels, 4);

                if(listSize == 0) {
                    ++lastLabel;

                    if(lastLabel > _LABEL_SIZE) {
                        destroyMatrix16(pLabelMatrix);
                        free(labelLocationInfo);
                        return NULL;
                    }

                    pLabelMatrix->elements[y*width + x] = lastLabel;
                    ++labelCntList[lastLabel];

                    labelLocationInfo[lastLabel].minX = x;
                    labelLocationInfo[lastLabel].minY = y;
                    labelLocationInfo[lastLabel].maxX = x;
                    labelLocationInfo[lastLabel].maxY = y;

                    labelLocationInfo[lastLabel].centerX = x;
                    labelLocationInfo[lastLabel].centerY = y;

                } else {
                    pLabelMatrix->elements[y*width + x] = adjacencyLabels[0];
                    int currentCnt = ++labelCntList[adjacencyLabels[0]];

                    Object_t* label = &labelLocationInfo[adjacencyLabels[0]];

                    if(label->minX > x)
                        label->minX = x;
                    
                    if(label->minY > y)
                        label->minY = y;
                    
                    if(label->maxX < x)
                        label->maxX = x;
                    
                    if(label->maxY < y)
                        label->maxY = y;
                    
                    label->centerX = ((label->centerX * (currentCnt-1))
                                            + x) / currentCnt;
                    label->centerY = ((label->centerY * (currentCnt-1))
                                            + y) / currentCnt;
                    
                    uint16_t finalLabel = adjacencyLabels[0];
                    while(equalLabelList[finalLabel] != 0) {
                        finalLabel = equalLabelList[finalLabel];
                    }

                    int i;
                    for(i = 1; i < listSize; ++i) {
                        int listIndex = adjacencyLabels[i];
                        while(equalLabelList[listIndex] != 0) {
                            listIndex = equalLabelList[listIndex];
                        }

                        if(listIndex != finalLabel) {
                            if(listIndex > finalLabel) {
                                equalLabelList[listIndex] = finalLabel;
                            } else {
                                equalLabelList[finalLabel] = listIndex;
                                finalLabel = listIndex;
                            }
                        }
                    }
                }
            }
        }
    }
    
    uint16_t i;
    int realLabels = 0;
    for(i = 1; i < _LABEL_SIZE; ++i) {
        uint16_t listIndex = i;
        
        while(equalLabelList[listIndex] != 0) {
            listIndex = equalLabelList[listIndex];
        }

        if(listIndex != i) {
            Object_t* targetLabel = &labelLocationInfo[listIndex];
            Object_t* sourceLabel = &labelLocationInfo[i];
            
            int targetCnt = labelCntList[listIndex];
            int sourceCnt = labelCntList[i];
            
            labelCntList[listIndex] += labelCntList[i];
            
            targetLabel->centerX = (targetCnt * targetLabel->centerX 
                                    + (sourceLabel->centerX)* sourceCnt)
                                    / labelCntList[listIndex];
        
            targetLabel->centerY = (targetCnt * targetLabel->centerY 
                                    + (sourceLabel->centerY) * sourceCnt)
                                    / labelCntList[listIndex];

            if(targetLabel->minX > sourceLabel->minX)
                targetLabel->minX = sourceLabel->minX;
            
            if(targetLabel->minY > sourceLabel->minY)
                targetLabel->minY = sourceLabel->minY;
            
            if(targetLabel->maxX < sourceLabel->maxX)
                targetLabel->maxX = sourceLabel->maxX;
            
            if(targetLabel->maxY < sourceLabel->maxY)
                targetLabel->maxY = sourceLabel->maxY; 

        } else if (labelCntList[i] > 0) {
            realLabels++;
        }
    }

    Object_t* resultObjects = (Object_t*)malloc(realLabels * sizeof(Object_t));

    int resultIndex = 0;
    for(i = 1; i < _LABEL_SIZE; ++i) {
        bool hasParentLabel = (equalLabelList[i] != 0);
        bool isEmptyLabel = (labelCntList[i] == 0);

        if(!hasParentLabel && !isEmptyLabel) {
            Object_t* targetObject = &resultObjects[resultIndex];
            Object_t* sourceObject = &labelLocationInfo[i];
            targetObject->minX = sourceObject->minX;
            targetObject->minY = sourceObject->minY;
            targetObject->maxX = sourceObject->maxX;
            targetObject->maxY = sourceObject->maxY;
            targetObject->cnt = labelCntList[i];
            targetObject->centerX = sourceObject->centerX;
            targetObject->centerY = sourceObject->centerY;
            resultIndex++;
        }
    }

    ObjectList_t* resultObjectList = (ObjectList_t*)malloc(sizeof(ObjectList_t));
    resultObjectList->size = realLabels;
    resultObjectList->list = resultObjects;
    /*********************************/

    bool hasOutputLabelMatrix = false;
    if(pOutputLabelMatrix != NULL) {
        hasOutputLabelMatrix = ( pOutputLabelMatrix->width == width &&
                                 pOutputLabelMatrix->height == height );
    }

    if(hasOutputLabelMatrix) {
        int cmpArray[_LABEL_SIZE] = {0,};
        int latestLabelId = 0;
        for(y=0; y<height; ++y) {
            for(x=0; x<width; ++x) {
                int labelId = pLabelMatrix->elements[y*width+x];
                
                if(cmpArray[labelId] == 0) {
                    int rootLabelId = labelId;
                    while(equalLabelList[rootLabelId] != 0) 
                        rootLabelId = equalLabelList[rootLabelId];
                    
                    if(cmpArray[rootLabelId] == 0) cmpArray[rootLabelId] = ++latestLabelId;
                    cmpArray[labelId] = cmpArray[rootLabelId];
                }

                pOutputLabelMatrix->elements[y*width+x] = cmpArray[labelId];
            }
        }
    }

    destroyMatrix16(pLabelMatrix);
    free(labelLocationInfo);

    return resultObjectList;
}

ObjectList_t* detectObjectsLocation(Matrix8_t* pMatrix) {
    return _detectObjectsLocation(pMatrix, NULL);
}

ObjectList_t* detectObjectsLocationWithLabeling(Matrix8_t* pMatrix,
                                                Matrix16_t* pLabelMatrix) {
    return _detectObjectsLocation(pMatrix, pLabelMatrix);
}


uint8_t _searchAdjacencyLabel(uint16_t* array, int size) {
    int i = 0;
    int labelSize = 0;

    for(i = 0; i < size; ++i) {
        if(array[i] != 0) {
            array[labelSize] = array[i];
            ++labelSize;
        }    
    }

    for(i = labelSize; i < size; ++i) {
        array[i] = 0;
    }

    return labelSize; 
}

void _sortArray(uint16_t* array, int size) {
    int i;
    int j;
    int temp;

    for(i = 1; i < size; ++i) {
        temp = array[i];
        for(j = i; j > 0; --j) {
            if(array[j-1] > temp) {
                array[j] = array[j-1];
                array[j-1] = temp;
            }
        }
    }
}
