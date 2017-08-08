#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <stdint.h>

typedef struct {
    uint8_t* elements;
    uint16_t width;
    uint16_t height;
} Matrix8_t;

typedef struct {
    uint16_t* elements;
    uint16_t width;
    uint16_t height;
} Matrix16_t;

typedef struct {
    uint32_t* elements;
    uint16_t width;
    uint16_t height;
} Matrix32_t;


Matrix8_t* createMatrix8(uint16_t width, uint16_t height);
Matrix16_t* createMatrix16(uint16_t width, uint16_t height);
Matrix32_t* createMatrix32(uint16_t width, uint16_t height);
Matrix8_t* cloneMatrix8(Matrix8_t* pMatrix8);
Matrix16_t* cloneMatrix16(Matrix16_t* pMatrix16);
Matrix32_t* cloneMatrix32(Matrix32_t* pMatrix32);
Matrix8_t* createSubMatrix8(Matrix8_t* pMatrix8, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY);
Matrix16_t* createSubMatrix16(Matrix16_t* pMatrix16, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY);
Matrix32_t* createSubMatrix32(Matrix32_t* pMatrix32, uint16_t minX, uint16_t minY, uint16_t maxX, uint16_t maxY);
void overlapMatrix8(Matrix8_t* pSourceMatrix8, Matrix8_t* pTargetMatrix8, uint16_t x, uint16_t y);
void overlapMatrix16(Matrix16_t* pSourceMatrix16, Matrix16_t* pTargetMatrix16, uint16_t x, uint16_t y);
void overlapMatrix32(Matrix32_t* pSourceMatrix32, Matrix32_t* pTargetMatrix32, uint16_t x, uint16_t y);
void destroyMatrix8(Matrix8_t* pMatrix8);
void destroyMatrix16(Matrix16_t* pMatrix16);
void destroyMatrix32(Matrix32_t* pMatrix32);

#endif // __MATRIX_H__


