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
    size_t size = height * width * sizeof(uint8_t);
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

Matrix8_t* cloneMatrix8(Matrix8_t pMatrix8) {
    uint16_t width = pMatrix8->width;
    uint16_t height = pMatrix8->height;
    Matrix8_t* pMatrix = createMatrix8(width, height);
    size_t size = height * width * sizeof(uint8_t);
    memcpy(pMatrix->elements, pMatrix8->elements, size);

    return pMatrix;
}

Matrix16_t* cloneMatrix16(Matrix16_t pMatrix16) {
    uint16_t width = pMatrix16->width;
    uint16_t height = pMatrix16->height;
    Matrix16_t* pMatrix = createMatrix16(width, height);
    size_t size = height * width * sizeof(uint16_t);
    memcpy(pMatrix->elements, pMatrix16->elements, size);

    return pMatrix;
}

Matrix32_t* cloneMatrix32(Matrix32_t pMatrix32) {
    uint16_t width = pMatrix32->width;
    uint16_t height = pMatrix32->height;
    Matrix32_t* pMatrix = createMatrix32(width, height);
    size_t size = height * width * sizeof(uint32_t);
    memcpy(pMatrix->elements, pMatrix32->elements, size);

    return pMatrix;
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
