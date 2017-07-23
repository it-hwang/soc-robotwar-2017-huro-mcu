#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lut.h"


LookUpTable8_t* _createLookUpTable8(uint32_t length) {
    size_t size = length * sizeof(uint8_t);
    LookUpTable8_t* pLut = (LookUpTable8_t*)malloc(sizeof(LookUpTable8_t));
    pLut->elements = (uint8_t*)malloc(size);
    pLut->length = length;
    return pLut;
}

LookUpTable16_t* _createLookUpTable16(uint32_t length) {
    size_t size = length * sizeof(uint16_t);
    LookUpTable16_t* pLut = (LookUpTable16_t*)malloc(sizeof(LookUpTable16_t));
    pLut->elements = (uint16_t*)malloc(size);
    pLut->length = length;
    return pLut;
}

LookUpTable32_t* _createLookUpTable32(uint32_t length) {
    size_t size = length * sizeof(uint32_t);
    LookUpTable32_t* pLut = (LookUpTable32_t*)malloc(sizeof(LookUpTable32_t));
    pLut->elements = (uint32_t*)malloc(size);
    pLut->length = length;
    return pLut;
}

void _fillLookUpTable8(LookUpTable8_t* pLut8, uint8_t (*pFunc)(uint32_t)) {
    uint32_t length = pLut8->length;
    if (pFunc) {
        uint32_t i;
        for (i = 0; i < length; ++i)
            pLut8->elements[i] = pFunc(i);
    }
    else {
        size_t size = length * sizeof(uint8_t);
        memset(pLut8->elements, 0, size);
    }
}

void _fillLookUpTable16(LookUpTable16_t* pLut16, uint16_t (*pFunc)(uint32_t)) {
    uint32_t length = pLut16->length;
    if (pFunc) {
        uint32_t i;
        for (i = 0; i < length; ++i)
            pLut16->elements[i] = pFunc(i);
    }
    else {
        size_t size = length * sizeof(uint16_t);
        memset(pLut16->elements, 0, size);
    }
}

void _fillLookUpTable32(LookUpTable32_t* pLut32, uint32_t (*pFunc)(uint32_t)) {
    uint32_t length = pLut32->length;
    if (pFunc) {
        uint32_t i;
        for (i = 0; i < length; ++i)
            pLut32->elements[i] = pFunc(i);
    }
    else {
        size_t size = length * sizeof(uint32_t);
        memset(pLut32->elements, 0, size);
    }
}

bool _writeLookUpTableFile8(LookUpTable8_t* pLut8, const char* filePath) {
    FILE* outputFile = fopen(filePath, "w");
    if (outputFile == NULL)
        return false;

    fwrite(pLut8->elements, sizeof(uint8_t), pLut8->length, outputFile);
    fclose(outputFile);
    return true;
}

bool _writeLookUpTableFile16(LookUpTable16_t* pLut16, const char* filePath) {
    FILE* outputFile = fopen(filePath, "w");
    if (outputFile == NULL)
        return false;

    fwrite(pLut16->elements, sizeof(uint16_t), pLut16->length, outputFile);
    fclose(outputFile);
    return true;
}

bool _writeLookUpTableFile32(LookUpTable32_t* pLut32, const char* filePath) {
    FILE* outputFile = fopen(filePath, "w");
    if (outputFile == NULL)
        return false;

    fwrite(pLut32->elements, sizeof(uint32_t), pLut32->length, outputFile);
    fclose(outputFile);
    return true;
}

bool _readLookUpTableFile8(LookUpTable8_t* pLut8, const char* filePath) {
    FILE* inputFile = fopen(filePath, "r");
    if (inputFile == NULL)
        return false;

    fread(pLut8->elements, sizeof(uint8_t), pLut8->length, inputFile);
    fclose(inputFile);
    return true;
}

bool _readLookUpTableFile16(LookUpTable16_t* pLut16, const char* filePath) {
    FILE* inputFile = fopen(filePath, "r");
    if (inputFile == NULL)
        return false;

    fread(pLut16->elements, sizeof(uint16_t), pLut16->length, inputFile);
    fclose(inputFile);
    return true;
}

bool _readLookUpTableFile32(LookUpTable32_t* pLut32, const char* filePath) {
    FILE* inputFile = fopen(filePath, "r");
    if (inputFile == NULL)
        return false;

    fread(pLut32->elements, sizeof(uint32_t), pLut32->length, inputFile);
    fclose(inputFile);
    return true;
}


LookUpTable8_t* createLookUpTable8(const char* filePath,
    uint8_t (*pFunc)(uint32_t), uint32_t length, bool overwrite) {
    LookUpTable8_t* pLut = _createLookUpTable8(length);

    if (filePath != NULL && !overwrite) {
        if (_readLookUpTableFile8(pLut, filePath))
            return pLut;
    }

    _fillLookUpTable8(pLut, pFunc);
    if (filePath != NULL)
        _writeLookUpTableFile8(pLut, filePath);

    return pLut;
}

LookUpTable16_t* createLookUpTable16(const char* filePath,
    uint16_t (*pFunc)(uint32_t), uint32_t length, bool overwrite) {
    LookUpTable16_t* pLut = _createLookUpTable16(length);

    if (filePath != NULL && !overwrite) {
        if (_readLookUpTableFile16(pLut, filePath))
            return pLut;
    }

    _fillLookUpTable16(pLut, pFunc);
    if (filePath != NULL)
        _writeLookUpTableFile16(pLut, filePath);

    return pLut;
}

LookUpTable32_t* createLookUpTable32(const char* filePath,
    uint32_t (*pFunc)(uint32_t), uint32_t length, bool overwrite) {
    LookUpTable32_t* pLut = _createLookUpTable32(length);

    if (filePath != NULL && !overwrite) {
        if (_readLookUpTableFile32(pLut, filePath))
            return pLut;
    }

    _fillLookUpTable32(pLut, pFunc);
    if (filePath != NULL)
        _writeLookUpTableFile32(pLut, filePath);

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
