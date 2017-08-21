#ifndef __BOUNDARY_H__
#define __BOUNDARY_H__

#include "matrix.h"
#include "graphic_interface.h"

Matrix8_t* establishBoundary(Screen_t* pScreen, Matrix8_t* pColorMatrix);
void applyBoundary(Screen_t* pScreen, Matrix8_t* pBoundaryMatrix);

#endif // __BOUNDARY_H__
