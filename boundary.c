
#include "matrix.h"
#include "object_detection.h"
#include "log.h"

Matrix8_t* establishBoundary(Matrix8_t* pColorMatrix) {
    static const char* LOG_FUNCTION_NAME = "establishBoundary()";

    Matrix16_t* pLabelMatrix = createMatrix16(pColorMatrix->width, pColorMatrix->height);
    memset(pLabelMatrix->elements, 0, (pLabelMatrix->height * pLabelMatrix->width) * sizeof(uint16_t));

    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    Object_t* pObject = _getBoundaryObject(pObjectList, pLabelMatrix);

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
    static const char* LOG_FUNCTION_NAME = "applyBoundary()";
    
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

