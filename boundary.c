
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