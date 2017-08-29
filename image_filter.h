#ifndef __IMAGE_FILTER_H__
#define __IMAGE_FILTER_H__

#include "graphic_interface.h"
#include "matrix.h"

void applyMeanFilter(Screen_t* pScreen, PixelCoordinate_t n);
void applyErosionToMatrix8(Matrix8_t* pMatrix, uint8_t n);
void applyDilationToMatrix8(Matrix8_t* pMatrix, uint8_t n);
void applyFastErosionToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n);
void applyFastDilationToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n);
void applyFastWidthErosionToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n);
void applyFastLengthErosionToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n);
void applyFastWidthDilationToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n);
void applyFastLengthDilationToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n);

#endif // __IMAGE_FILTER_H__
