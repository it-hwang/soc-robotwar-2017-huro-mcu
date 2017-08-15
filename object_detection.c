#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "object_detection.h"
#include "graphic_interface.h"
#include "matrix.h"
#include "log.h"

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
            targetObject->id = resultIndex + 1;
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


void destroyObjectList(ObjectList_t* pObjectList) {
    if (pObjectList == NULL)
        return;

    free(pObjectList->list);
    free(pObjectList);
}


void removeObjectFromList(ObjectList_t* pObjectList, Object_t* pObject) {
    if (pObjectList == NULL)
        return;
    if (pObject == NULL)
        return;

    // Swap two objects
    int lastIndex = pObjectList->size - 1;
    Object_t* pLastObject = &(pObjectList->list[lastIndex]);
    Object_t tempObject = *pLastObject;
    memcpy(pLastObject, pObject, sizeof(Object_t));
    memcpy(pObject, &tempObject, sizeof(Object_t));

    pObjectList->size--;
}


void drawObjectEdge(Screen_t* pScreen, Object_t* pObject, Rgab5515_t* pBorderColor) {
    Rgab5515_t purpleColor;
    purpleColor.r = 0x1f;
    purpleColor.g = 0x00;
    purpleColor.b = 0x1f;

    if (pScreen == NULL)
        return;
    if (pObject == NULL)
        return;
    if (pBorderColor == NULL)
        pBorderColor = &purpleColor;

    int width = pScreen->width;
    int minX = pObject->minX;
    int minY = pObject->minY;
    int maxX = pObject->maxX;
    int maxY = pObject->maxY;

    for (int i = minX; i < maxX; ++i) {
        int topIndex = minY * width + i;
        int bottomIndex = maxY * width + i;
        Rgab5515_t* pTopPixel = (Rgab5515_t*)&(pScreen->elements[topIndex]);
        Rgab5515_t* pBottomPixel = (Rgab5515_t*)&(pScreen->elements[bottomIndex]);
        pTopPixel->data = pBorderColor->data;
        pBottomPixel->data = pBorderColor->data;
    }

    for (int i = minY; i < maxY; ++i) {
        int leftIndex = i * width + minX;
        int rightIndex = i * width + maxX;
        Rgab5515_t* pLeftPixel = (Rgab5515_t*)&(pScreen->elements[leftIndex]);
        Rgab5515_t* pRightPixel = (Rgab5515_t*)&(pScreen->elements[rightIndex]);
        pLeftPixel->data = pBorderColor->data;
        pRightPixel->data = pBorderColor->data;
    }
}


void drawObjectCenter(Screen_t* pScreen, Object_t* pObject, Rgab5515_t* pPointColor) {
    Rgab5515_t cyanColor;
    cyanColor.r = 0x00;
    cyanColor.g = 0x1f;
    cyanColor.b = 0x1f;

    if (pScreen == NULL)
        return;
    if (pObject == NULL)
        return;
    if (pPointColor == NULL)
        pPointColor = &cyanColor;

    int width = pScreen->width;
    int x = pObject->centerX;
    int y = pObject->centerY;
    int index = (y * width) + x;
    Rgab5515_t* pPixel = (Rgab5515_t*)&(pScreen->elements[index]);
    pPixel->data = pPointColor->data;
}


void removeSmallObjects(ObjectList_t* pObjectList, int minimumCnt) {
    if (pObjectList == NULL)
        return;

    // 도중에 remove하여 리스트의 크기가 변해도 문제가 생기지 않도록
    // 역순으로 리스트를 순회한다.
    int lastIndex = pObjectList->size - 1;
    for (int i = lastIndex; i >= 0; --i) {
        Object_t* pObject = &(pObjectList->list[i]);
        if (pObject->cnt < minimumCnt)
            removeObjectFromList(pObjectList, pObject);
    }
}


Object_t* findLargestObject(ObjectList_t* pObjectList) {
    if (pObjectList == NULL)
        return NULL;

    int maxArea = 0;
    Object_t* pLargestObject = NULL;

    for (int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &(pObjectList->list[i]);
        int area = pObject->cnt;

        if (area > maxArea) {
            pLargestObject = pObject;
            maxArea = area;
        }
    }

    return pLargestObject;
}


void destroyObject(Object_t* pObject) {
    if (pObject == NULL)
        return;

    free(pObject);
}


// pObject가 직사각형과의 유사한 정도를 반환한다. (범위: 0.0 ~ 1.0)
float getRectangleCorrelation(Matrix8_t* pMatrix, Object_t* pObject) {
    if (pObject == NULL)
        return 0.;

    static const float AREA_CORRELATION_RATIO = 0.8;
    static const float CENTER_CORRELATION_RATIO = 0.2;

    int width = pObject->maxX - pObject->minX + 1;
    int height = pObject->maxY - pObject->minY + 1;
    int objectArea = pObject->cnt;
    int rectangleArea = width * height;
    float areaCorrelation = (float)objectArea / rectangleArea;

    float objectCenterX = pObject->centerX;
    float objectCenterY = pObject->centerY;
    float rectangleCenterX = (float)(pObject->maxX + pObject->minX) / 2;
    float rectangleCenterY = (float)(pObject->maxY + pObject->minY) / 2;
    float deltaCenterX = rectangleCenterX - objectCenterX;
    float deltaCenterY = rectangleCenterY - objectCenterY;
    float centerDistance = sqrtf(deltaCenterX*deltaCenterX + deltaCenterY*deltaCenterY);
    float radius = sqrtf(width*width + height*height) / 2;
    float centerCorrelation = 1.0 - (centerDistance / radius);

    return (areaCorrelation   * AREA_CORRELATION_RATIO) +
           (centerCorrelation * CENTER_CORRELATION_RATIO);
}
