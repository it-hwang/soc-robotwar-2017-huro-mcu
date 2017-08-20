
#include "matrix.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "log.h"

#define LABEL_SIZE 1000

static Object_t* _getBoundaryObject(ObjectList_t* pObjectList, Matrix16_t* pLabelMatrix);
static int _getObjectLabel(Matrix16_t* pLabelMatrix);

Matrix8_t* establishBoundary(Matrix8_t* pColorMatrix) {

    Matrix16_t* pLabelMatrix = createMatrix16(pColorMatrix->width, pColorMatrix->height);
    memset(pLabelMatrix->elements, 0, (pLabelMatrix->height * pLabelMatrix->width) * sizeof(uint16_t));

    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    if(pObjectList == NULL || pObjectList->size == 0)
        return NULL;

    Object_t* pObject = _getBoundaryObject(pObjectList, pLabelMatrix);

    if(pObject == NULL)
        return NULL;
        
    Matrix8_t* pReturnMatrix = _traceBoundaryLine(pObject, pLabelMatrix);

    _fillBoundary(pReturnMatrix);

    if(pObjectList != NULL){
        free(pObjectList->list);
        free(pObjectList);
    }
    
    destroyMatrix16(pLabelMatrix);

    return pReturnMatrix;
}

void applyBoundary(Screen_t* pScreen, Matrix8_t* pBoundaryMatrix) {
    
    if(pScreen->width != pBoundaryMatrix->width)
        return;
    
    if(pScreen->height != pBoundaryMatrix->height)
        return;

    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;

    for(int i = 0; i < length; ++i) {
        if(pBoundaryMatrix->elements[i] != 0xFF)
            pScreen->elements[i] = 0x7bcf; //NONE_COLOR
    }
}

static Object_t* _getBoundaryObject(ObjectList_t* pObjectList, Matrix16_t* pLabelMatrix) {
    
    int objectLabel = _getObjectLabel(pLabelMatrix);

    if(objectLabel == 0)
        return NULL;

    Object_t* pObject = NULL;
    for(int i = 0; i < pObjectList->size; ++i) {
        pObject = &pObjectList->list[i];
        
        int index = pObject.centerY * pLabelMatrix->width + pObject.centerX;
        if(pLabelMatrix->elements[index] == objectLabel)
            break;
    }

    return pObject;
}

static int _getObjectLabel(Matrix16_t* pLabelMatrix) {

    int widthToExplore = pLabelMatrix->width;
    int heightToExplore = 5;
    int startHeight = pLabelMatrix->width - heightToExplore;
    int endHeight = startHeight + heightToExplore - 1;

    int labelList[LABEL_SIZE] = {0, };
    int maxLabel = 0;

    for(int y = startHeight; y < endHeight; ++y) {
        for(int x = 0; x < widthToExplore; ++x) {
            int index = y * widthToExplore + x;
            int listIndex = pLabelMatrix->elements[index];
            
            if(labelList[listIndex] != 0){
                labelList[listIndex]++;

                if(labelList[listIndex] > labelList[maxLabel])
                    maxLabel = listIndex;
            }
        }
    }

    return maxLabel;
}

static Matrix8_t* _traceBoundaryLine(Object_t* pObject, Matrix16_t* pLabelMatrix) {
    
    PixelLocation_t startPoint = _getStartPointForTraceLine(pObject, pLabelMatrix);

    PixelLocation_t curPoint = startPoint;
    int direction = 0;
    int checkAllDirection = 0;

    Matrix8_t* pBoundaryMatrix = createMatrix8(pLabelMatrix->width, pLabelMatrix->height);
    memset(pBoundaryMatrix->elements, 0, (pBoundaryMatrix->height * pBoundaryMatrix->width) * sizeof(uint8_t));

    while(true) {
        PixelLocation_t nextPoint = _directionToPoint(curPoint, direction);

        int notAllDirection = _notAllDirection(pBoundaryMatrix, curPoint, nextPoint, &direction, &checkAllDirection)
        if(notAllDirection < 0) {
            break;
        }else if(notAllDirection > 0) {
            int index = curPoint.y * pBoundaryMatrix->width + curPoint.x;
            pBoundaryMatrix->elements[index] = 0xff;
    
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

    int index = pObject->centerY * pLabelMatrix->width + pObject->centerX;
    int labelNum = pLabelMatrix->elements[index];

    PixelLocation_t returnLocation;
    
    int x = pObject->minX - 1;
    int y = pObject->minY;

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

static int _notAllDirection(Matrix8_t* pBoundaryMatrix, PixelLocation_t curPoint, PixelLocation_t nextPoint, int* direction, int* checkAllDirection) {
 
    int width = pBoundaryMatrix->width;
    int height = pBoundaryMatrix->height;

    int index = nextPoint.y * width + nextPoint.x;
    if(nextPoint.x < 0 || nextPoint.x >= width || nextPoint.y < 0 || nextPoint.y >= height) {
        if(++(*direction) > 7)
            *direction = 0;

        *checkAllDirection++;

        if(*checkAllDirection >= 8) {
            index = curPoint.y * width + curPoint.x;
            pBoundaryMatrix->elements[index] = 0xff;
            return -1;
        }

        return 0;
    } else if(pBoundaryMatrix->elements[index] != 0xff) {
        if(++(*direction) > 7)
            *direction = 0;

        *checkAllDirection++;

        if(*checkAllDirection >= 8) {
            index = curPoint.y * width + curPoint.x;
            pBoundaryMatrix->elements[index] = 0xff;
            return -1;
        }

        return 0;
    }else 
        return 1;
}

static void _fillBoundary(Matrix8_t* pBoundaryMatrix) {

    int height = pBoundaryMatrix->height;

    for(int y = 0; y < height; ++y) {
        
        int rightX = _findRightX(pBoundaryMatrix, y);
        if(rightX == 0)
            continue;
        
        int leftX = _findLeftX(pBoundaryMatrix, y);

        _fillLeftToRight(pBoundaryMatrix, leftX, rightX);
    }
}

static int _findRightX(Matrix8_t* pBoundaryMatrix, int y) {

    int width = pBoundaryMatrix->width;

    int resultX = 0;

    for(int x = width-1; x > 0; --x) {
        int index = y * width + x;
        if(pBoundaryMatrix->elements[index] == 0xff) {
            resultX = x;
            break;
        }
    }
    
    return resultX;
}
static void _fillLeftToRight(Matrix8_t* pBoundaryMatrix, int leftX, int rightX) {

}
