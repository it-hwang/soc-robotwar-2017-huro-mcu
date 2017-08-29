#ifndef __BOUNDARY_H__
#define __BOUNDARY_H__

#include "matrix.h"
#include "graphic_interface.h"

Matrix8_t* establishBoundary(Matrix8_t* pColorMatrix);
void applyBoundary(Screen_t* pScreen, Matrix8_t* pBoundaryMatrix);
Matrix8_t* traceBoundaryLine(Object_t* pObject, Matrix16_t* pLabelMatrix);
void fillBoundary(Matrix8_t* pBoundaryMatrix);

#endif // __BOUNDARY_H__
