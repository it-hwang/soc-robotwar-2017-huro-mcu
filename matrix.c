#include <stdlib.h>
#include <string.h>

#include "matrix.h"


Matrix8_t* createMatrix8(uint16_t width, uint16_t height) {
    Matrix8_t* pMatrix = (Matrix8_t*)malloc(sizeof(Matrix8_t));
    size_t size = height * width * sizeof(uint8_t);
    pMatrix->elements = (uint8_t*)malloc(size);
    pMatrix->width = width;
    pMatrix->height = height;

    return pMatrix;
}

Matrix16_t* createMatrix16(uint16_t width, uint16_t height) {
    Matrix16_t* pMatrix = (Matrix16_t*)malloc(sizeof(Matrix16_t));
    size_t size = height * width * sizeof(uint16_t);
    pMatrix->elements = (uint16_t*)malloc(size);
    pMatrix->width = width;
    pMatrix->height = height;

    return pMatrix;
}

Matrix32_t* createMatrix32(uint16_t width, uint16_t height) {
    Matrix32_t* pMatrix = (Matrix32_t*)malloc(sizeof(Matrix32_t));
    size_t size = height * width * sizeof(uint32_t);
    pMatrix->elements = (uint32_t*)malloc(size);
    pMatrix->width = width;
    pMatrix->height = height;

    return pMatrix;
}

Matrix8_t* cloneMatrix8(Matrix8_t* pMatrix8) {
    uint16_t width = pMatrix8->width;
    uint16_t height = pMatrix8->height;
    Matrix8_t* pMatrix = createMatrix8(width, height);
    size_t size = height * width * sizeof(uint8_t);
    memcpy(pMatrix->elements, pMatrix8->elements, size);

    return pMatrix;
}

Matrix16_t* cloneMatrix16(Matrix16_t* pMatrix16) {
    uint16_t width = pMatrix16->width;
    uint16_t height = pMatrix16->height;
    Matrix16_t* pMatrix = createMatrix16(width, height);
    size_t size = height * width * sizeof(uint16_t);
    memcpy(pMatrix->elements, pMatrix16->elements, size);

    return pMatrix;
}

Matrix32_t* cloneMatrix32(Matrix32_t* pMatrix32) {
    uint16_t width = pMatrix32->width;
    uint16_t height = pMatrix32->height;
    Matrix32_t* pMatrix = createMatrix32(width, height);
    size_t size = height * width * sizeof(uint32_t);
    memcpy(pMatrix->elements, pMatrix32->elements, size);

    return pMatrix;
}


Matrix8_t* createSubMatrix8(Matrix8_t* pMatrix8, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY) {
    if (minX > maxX || maxX >= pMatrix8->width)
        return NULL;
    if (minY > maxY || maxY >= pMatrix8->height)
        return NULL;

    uint16_t width = maxX-minX+1;
    uint16_t height = maxY-minY+1;
    Matrix8_t* pMatrix = createMatrix8(width, height);
    
    uint8_t* pSrcElements = pMatrix8->elements + minX;
    uint8_t* pDstElements = pMatrix->elements;
    size_t size_w = width * sizeof(uint8_t);
    int i;
    for(i=minY; i<=maxY; ++i) {
        memcpy(pDstElements, pSrcElements, size_w);
        pSrcElements += pMatrix8->width;
        pDstElements += width;
    }
    
    return pMatrix;
}

Matrix16_t* createSubMatrix16(Matrix16_t* pMatrix16, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY) {
    if (minX > maxX || maxX >= pMatrix16->width)
        return NULL;
    if (minY > maxY || maxY >= pMatrix16->height)
        return NULL;

    uint16_t width = maxX-minX+1;
    uint16_t height = maxY-minY+1;
    Matrix16_t* pMatrix = createMatrix16(width, height);

    uint16_t* pSrcElements = pMatrix16->elements + minX;
    uint16_t* pDstElements = pMatrix->elements;
    size_t size_w = width * sizeof(uint16_t);
    int i;
    for(i=minY; i<=maxY; ++i) {
        memcpy(pDstElements, pSrcElements, size_w);
        pSrcElements += pMatrix16->width;
        pDstElements += width;
    }
   
    return pMatrix;
}

Matrix32_t* createSubMatrix32(Matrix32_t* pMatrix32, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY) {
    if (minX > maxX || maxX >= pMatrix32->width)
        return NULL;
    if (minY > maxY || maxY >= pMatrix32->height)
        return NULL;

    uint16_t width = maxX-minX+1;
    uint16_t height = maxY-minY+1;
    Matrix32_t* pMatrix = createMatrix32(width, height);
    
    uint16_t* pSrcElements = (uint16_t*)(pMatrix32->elements + minX);
    uint16_t* pDstElements = (uint16_t*)pMatrix->elements;
    size_t size_w = width * sizeof(uint32_t);
    int i;
    for(i=minY; i<=maxY; ++i) {
        memcpy(pDstElements, pSrcElements, size_w);
        pSrcElements += pMatrix32->width;
        pDstElements += width;
    }
   
    return pMatrix;
}

void overlapMatrix8(Matrix8_t* pSourceMatrix8, Matrix8_t* pTargetMatrix8, uint16_t x, uint16_t y) {
    if (x < 0 || x + pSourceMatrix8->width > pTargetMatrix8->width)
        return;
    if (y < 0 || y + pSourceMatrix8->height > pTargetMatrix8->height)
        return;

    uint16_t sourceWidth = pSourceMatrix8->width;
    uint16_t sourceHeight = pSourceMatrix8->height;
    uint16_t targetWidth = pTargetMatrix8->width;

    int i, j;
    for(j = 0; j < sourceHeight; ++j) {
        for(i = 0; i < sourceWidth; ++i) {
            pTargetMatrix8->elements[(y + j) * targetWidth + (x + i)] 
                            = pSourceMatrix8->elements[j * sourceWidth + i];
        }
    }
}

void overlapMatrix16(Matrix16_t* pSourceMatrix16, Matrix16_t* pTargetMatrix16, uint16_t x, uint16_t y) {
    if (x < 0 || x + pSourceMatrix16->width > pTargetMatrix16->width)
        return;
    if (y < 0 || y + pSourceMatrix16->height > pTargetMatrix16->height)
        return;

    uint16_t sourceWidth = pSourceMatrix16->width;
    uint16_t sourceHeight = pSourceMatrix16->height;
    uint16_t targetWidth = pTargetMatrix16->width;

    int i, j;
    for(j = 0; j < sourceHeight; ++j) {
        for(i = 0; i < sourceWidth; ++i) {
            pTargetMatrix16->elements[(y + j) * targetWidth + (x + i)] 
                            = pSourceMatrix16->elements[j * sourceWidth + i];
        }
    }
}

void overlapMatrix32(Matrix32_t* pSourceMatrix32, Matrix32_t* pTargetMatrix32, uint16_t x, uint16_t y) {
    if (x < 0 || x + pSourceMatrix32->width > pTargetMatrix32->width)
        return;
    if (y < 0 || y + pSourceMatrix32->height > pTargetMatrix32->height)
        return;

    uint16_t sourceWidth = pSourceMatrix32->width;
    uint16_t sourceHeight = pSourceMatrix32->height;
    uint16_t targetWidth = pTargetMatrix32->width;

    int i, j;
    for(j = 0; j < sourceHeight; ++j) {
        for(i = 0; i < sourceWidth; ++i) {
            pTargetMatrix32->elements[(y + j) * targetWidth + (x + i)] 
                            = pSourceMatrix32->elements[j * sourceWidth + i];
        }
    }
}

void destroyMatrix8(Matrix8_t* pMatrix8) {
    free(pMatrix8->elements);
    free(pMatrix8);
}

void destroyMatrix16(Matrix16_t* pMatrix16) {
    free(pMatrix16->elements);
    free(pMatrix16);
}

void destroyMatrix32(Matrix32_t* pMatrix32) {
    free(pMatrix32->elements);
    free(pMatrix32);
}
