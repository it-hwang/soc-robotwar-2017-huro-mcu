
#include "matrix.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "log.h"

static Object_t* _getBoundaryObject(ObjectList_t* pObjectList, Matrix16_t* pLabelMatrix);

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

    Object_t* pObject = NULL;
    for(int i = 0; i < pObjectList->size; ++i) {
        pObject = &pObjectList->list[i];
        
        int index = pObject.centerY * pLabelMatrix->width + pObject.centerX;
        if(pLabelMatrix->elements[index] == objectLabel)
            break;
    }

    return pObject;
}