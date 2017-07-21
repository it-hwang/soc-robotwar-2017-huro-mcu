#include <stdlib.h>

#include "lut.h"


LookUpTable8_t* createLookUpTable8(uint8_t (*pFunc)(uint32_t),
                                   uint32_t length) {
    size_t size = length * sizeof(uint8_t);
    LookUpTable8_t* pLut = (LookUpTable8_t*)malloc(sizeof(LookUpTable8_t));
    pLut->elements = (uint8_t*)malloc(size);
    pLut->length = length;
    if (pFunc) {
        uint32_t i;
        for (i = 0; i < length; ++i)
            pLut->elements[i] = pFunc(i);
    }
    return pLut;
}

LookUpTable16_t* createLookUpTable16(uint16_t (*pFunc)(uint32_t),
                                     uint32_t length) {
    size_t size = length * sizeof(uint16_t);
    LookUpTable16_t* pLut = (LookUpTable16_t*)malloc(sizeof(LookUpTable16_t));
    pLut->elements = (uint16_t*)malloc(size);
    pLut->length = length;
    if (pFunc) {
        uint32_t i;
        for (i = 0; i < length; ++i)
            pLut->elements[i] = pFunc(i);
    }
    return pLut;
}

LookUpTable32_t* createLookUpTable32(uint32_t (*pFunc)(uint32_t),
                                     uint32_t length) {
    size_t size = length * sizeof(uint32_t);
    LookUpTable32_t* pLut = (LookUpTable32_t*)malloc(sizeof(LookUpTable32_t));
    pLut->elements = (uint32_t*)malloc(size);
    pLut->length = length;
    if (pFunc) {
        uint32_t i;
        for (i = 0; i < length; ++i)
            pLut->elements[i] = pFunc(i);
    }
    return pLut;
}

void destroyLookUpTable8(LookUpTable8_t* pLut8) {
    free(pLut8->elements);
    free(pLut8);
}

void destroyLookUpTable16(LookUpTable16_t* pLut16) {
    free(pLut16->elements);
    free(pLut16);
}

void destroyLookUpTable32(LookUpTable32_t* pLut32) {
    free(pLut32->elements);
    free(pLut32);
}
