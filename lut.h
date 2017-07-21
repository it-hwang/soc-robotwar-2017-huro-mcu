#ifndef __LUT_H__
#define __LUT_H__

#include <stdint.h>

typedef struct {
    uint8_t* elements;
    uint32_t length;
} LookUpTable8_t;

typedef struct {
    uint16_t* elements;
    uint32_t length;
} LookUpTable16_t;

typedef struct {
    uint32_t* elements;
    uint32_t length;
} LookUpTable32_t;


LookUpTable8_t* createLookUpTable8(uint8_t (*pFunc)(uint32_t),
                                   uint32_t length);
LookUpTable16_t* createLookUpTable16(uint16_t (*pFunc)(uint32_t),
                                     uint32_t length);
LookUpTable32_t* createLookUpTable32(uint32_t (*pFunc)(uint32_t),
                                     uint32_t length);
void destroyLookUpTable8(LookUpTable8_t* pLut8);
void destroyLookUpTable16(LookUpTable16_t* pLut16);
void destroyLookUpTable32(LookUpTable32_t* pLut32);

#endif // __LUT_H__
