#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "boundary.h"
#include "matrix.h"
#include "object_detection.h"
#include "log.h"

#define LABEL_SIZE 1001

#define BOUNDARY_LINE   0x01
#define BOUNDARY_LEFT   0x02
#define BOUNDARY_RIGHT  0x04

static Object_t* _getBoundaryObject(ObjectList_t* pObjectList, Matrix16_t* pLabelMatrix);
static int _getObjectLabel(Matrix16_t* pLabelMatrix);
static PixelLocation_t _getStartPointForTraceLine(Object_t* pObject, Matrix16_t* pLabelMatrix);
static PixelLocation_t _directionToPoint(PixelLocation_t curPoint, int direction);
static int _notAllDirection(Matrix16_t* pLabelMatrix, Object_t* pObject, PixelLocation_t nextPoint, int* direction, int* checkAllDirection);
static int _findRightX(Matrix8_t* pBoundaryMatrix, int y);

Matrix8_t* establishBoundary(Matrix8_t* pColorMatrix) {

    Matrix16_t* pLabelMatrix = createMatrix16(pColorMatrix->width, pColorMatrix->height);
    memset(pLabelMatrix->elements, 0, (pLabelMatrix->height * pLabelMatrix->width) * sizeof(uint16_t));
    
    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    if(pObjectList == NULL || pObjectList->size == 0){
        destroyMatrix16(pLabelMatrix);
        destroyObjectList(pObjectList);
        return NULL;
    }

    Object_t* pObject = _getBoundaryObject(pObjectList, pLabelMatrix);

    if(pObject == NULL){
        destroyMatrix16(pLabelMatrix);
        destroyObjectList(pObjectList);
        return NULL;
    }

    Matrix8_t* pReturnMatrix = traceBoundaryLine(pObject, pLabelMatrix);

    fillBoundary(pReturnMatrix);

    destroyObjectList(pObjectList);
    
    destroyMatrix16(pLabelMatrix);

    return pReturnMatrix;
}

void applyBoundary(Screen_t* pScreen, Matrix8_t* pBoundaryMatrix) {
    
    if(pBoundaryMatrix == NULL)
        return;

    if(pScreen->width != pBoundaryMatrix->width)
        return;
    
    if(pScreen->height != pBoundaryMatrix->height)
        return;

    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    uint8_t* pBoundaryPixels = pBoundaryMatrix->elements;
    PixelData_t* pScreenPixels = pScreen->elements;

    for(int i = 0; i < length; ++i) {
        if(*pBoundaryPixels != 0xff)
            *pScreenPixels = 0x7bcf; //NONE_COLOR
        pBoundaryPixels++;
        pScreenPixels++;
    }
}

static Object_t* _getBoundaryObject(ObjectList_t* pObjectList, Matrix16_t* pLabelMatrix) {
    
    int objectLabel = _getObjectLabel(pLabelMatrix);

    if(objectLabel == 0){
        return NULL;
    }
    
    Object_t* pObject = NULL;
    for(int i = 0; i < pObjectList->size; ++i) {
        pObject = &(pObjectList->list[i]);
        
        if(pObject->id == objectLabel)
            break;
    }

    return pObject;
}

static int _getObjectLabel(Matrix16_t* pLabelMatrix) {

    int widthToExplore = pLabelMatrix->width;
    int heightToExplore = 5;
    int startHeight = pLabelMatrix->height - heightToExplore;
    int endHeight = pLabelMatrix->height;

    int* labelList = malloc(LABEL_SIZE * sizeof(int));
    memset(labelList, 0, LABEL_SIZE * sizeof(int));

    int maxLabel = 0;

    for(int y = startHeight; y < endHeight; ++y) {
        for(int x = 0; x < widthToExplore; ++x) {
            int index = y * widthToExplore + x;
            int listIndex = pLabelMatrix->elements[index];
            
            if(listIndex != 0){
                labelList[listIndex]++;

                if(labelList[listIndex] > labelList[maxLabel])
                    maxLabel = listIndex;
            }
        }
    }

    free(labelList);

    return maxLabel;
}

Matrix8_t* traceBoundaryLine(Object_t* pObject, Matrix16_t* pLabelMatrix) {
    
    PixelLocation_t startPoint = _getStartPointForTraceLine(pObject, pLabelMatrix);

    PixelLocation_t curPoint = startPoint;

    int direction = 0;
    int checkAllDirection = 0;

    Matrix8_t* pBoundaryMatrix = createMatrix8(pLabelMatrix->width, pLabelMatrix->height);
    memset(pBoundaryMatrix->elements, 0, (pBoundaryMatrix->height * pBoundaryMatrix->width) * sizeof(uint8_t));

    while(true) {
        PixelLocation_t nextPoint = _directionToPoint(curPoint, direction);
        int notAllDirection = _notAllDirection(pLabelMatrix, pObject, nextPoint, &direction, &checkAllDirection);
        
        if(notAllDirection < 0) {
            int index = curPoint.y * pBoundaryMatrix->width + curPoint.x;
            pBoundaryMatrix->elements[index] |= BOUNDARY_LINE;
            break;
        }else if(notAllDirection > 0) {
            int index = curPoint.y * pBoundaryMatrix->width + curPoint.x;
            pBoundaryMatrix->elements[index] |= BOUNDARY_LINE;

            // 선에 방향 성분을 추가한다.
            if (direction >= 5 && direction <= 7) {
                index = nextPoint.y * pBoundaryMatrix->width + nextPoint.x;
                pBoundaryMatrix->elements[index] |= BOUNDARY_LEFT;
            }
            else if (direction >= 1 && direction <= 3) {
                pBoundaryMatrix->elements[index] |= BOUNDARY_RIGHT;
            }

            // 두 방향을 모두 가진 선은 방향 성분을 제거한다.
            // 왼쪽 성분을 두번 가지면 문제가 생기기 때문이다.
            if ((pBoundaryMatrix->elements[index] & BOUNDARY_LEFT) && (pBoundaryMatrix->elements[index] & BOUNDARY_RIGHT))
                pBoundaryMatrix->elements[index] = BOUNDARY_LINE;
    
            curPoint = nextPoint;
    
            checkAllDirection = 0;
            direction = (direction + 6) % 8; // direction = direction - 2;    
        }

        if(curPoint.x == startPoint.x && curPoint.y == startPoint.y && direction == 0) {
            break;
        }
    }

    return pBoundaryMatrix;
}

static PixelLocation_t _getStartPointForTraceLine(Object_t* pObject, Matrix16_t* pLabelMatrix) {

    int labelNum = pObject->id;

    PixelLocation_t returnLocation;
    
    int x = pObject->minX - 1;
    int y = pObject->minY;
    int index = 0;

    do {
        x++;
        index = y * pLabelMatrix->width + x;
    }while(pLabelMatrix->elements[index] != labelNum);

    returnLocation.x = x;
    returnLocation.y = y;

    return returnLocation;
}

static PixelLocation_t _directionToPoint(PixelLocation_t curPoint, int direction) {

    PixelLocation_t returnPixel;

    if(direction == 0) {
        returnPixel.x = curPoint.x + 1;
        returnPixel.y = curPoint.y;
    } else if(direction == 1) {
        returnPixel.x = curPoint.x + 1;
        returnPixel.y = curPoint.y + 1;
    } else if(direction == 2) {
        returnPixel.x = curPoint.x;
        returnPixel.y = curPoint.y + 1;
    } else if(direction == 3) {
        returnPixel.x = curPoint.x - 1;
        returnPixel.y = curPoint.y + 1;
    } else if(direction == 4) {
        returnPixel.x = curPoint.x - 1;
        returnPixel.y = curPoint.y;
    } else if(direction == 5) {
        returnPixel.x = curPoint.x - 1;
        returnPixel.y = curPoint.y - 1;
    } else if(direction == 6) {
        returnPixel.x = curPoint.x;
        returnPixel.y = curPoint.y - 1;
    } else if(direction == 7) {
        returnPixel.x = curPoint.x + 1;
        returnPixel.y = curPoint.y - 1;
    } else {
        returnPixel.x = curPoint.x;
        returnPixel.y = curPoint.y;
    }

    return returnPixel;
}

static int _notAllDirection(Matrix16_t* pLabelMatrix, Object_t* pObject, PixelLocation_t nextPoint, int* direction, int* checkAllDirection) {
    int width = pLabelMatrix->width;
    int height = pLabelMatrix->height;

    int labelNum = pObject->id;
    
    int index = nextPoint.y * width + nextPoint.x;
    if(nextPoint.x < 0 || nextPoint.x >= width || nextPoint.y < 0 || nextPoint.y >= height) {
        if(++(*direction) > 7)
            *direction = 0;

        (*checkAllDirection)++;

        if(*checkAllDirection >= 8) {
            return -1;
        }

        return 0;
    } else if(pLabelMatrix->elements[index] != labelNum) {
        if(++(*direction) > 7)
            *direction = 0;

        (*checkAllDirection)++;

        if(*checkAllDirection >= 8) {
            return -1;
        }

        return 0;
    }else 
        return 1;
}

void fillBoundary(Matrix8_t* pBoundaryMatrix) {

    int height = pBoundaryMatrix->height;

    for(int y = 0; y < height; ++y) {
        
        int rightX = _findRightX(pBoundaryMatrix, y);
        if(rightX == 0)
            continue;
        
        // 외곽선 안쪽만 채우기
        uint8_t* elements = pBoundaryMatrix->elements + (y * pBoundaryMatrix->width);
        int status = 0;
        for (int x = 0; x <= rightX; ++x) {
            int element = elements[x];
            
            if (element & BOUNDARY_LINE)
                elements[x] = 0xff;

            if (status == 0) {  // Find border
                if (element & BOUNDARY_LEFT) status = 1;
            }
            else if (status == 1) { // Start filling
                if (element & BOUNDARY_RIGHT) status = 0;
                else elements[x] = 0xff;
            }
        }
    }
}

static int _findRightX(Matrix8_t* pBoundaryMatrix, int y) {

    int width = pBoundaryMatrix->width;

    int resultX = 0;

    for(int x = width-1; x > 0; --x) {
        int index = y * width + x;
        if(pBoundaryMatrix->elements[index]) {
            resultX = x;
            break;
        }
    }
    
    return resultX;
}
