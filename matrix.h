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
Matrix8_t* cloneMatrix8(Matrix8_t pMatrix8);
Matrix16_t* cloneMatrix16(Matrix16_t pMatrix16);
Matrix32_t* cloneMatrix32(Matrix32_t pMatrix32);
void destroyMatrix8(Matrix8_t* pMatrix8);
void destroyMatrix16(Matrix16_t* pMatrix16);
void destroyMatrix32(Matrix32_t* pMatrix32);

#endif // __MATRIX_H__
