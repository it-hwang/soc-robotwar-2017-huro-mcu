#include <string.h>
#include <stdio.h>

#include "image_filter.h"
#include "color_model.h"


// n x n 크기의 평균 필터를 씌웁니다.
void applyMeanFilter(Screen_t* pScreen, PixelCoordinate_t n) {
    // n은 홀수
    if (n % 2 == 0)
        n += 1;
    int n2 = n / 2;

    int width = pScreen->width;
    int height = pScreen->height;
    Screen_t* pBuffer = createScreen(width, height);
    int sumR;
    int sumG;
    int sumB;
    int x;
    int y;
    int i;
    Rgab5515_t* pCurrentPixel;
    int backwardPosition;
    int forwardPosition;
    Rgab5515_t* pBackwardPixel;
    Rgab5515_t* pForwardPixel;
    
    // 가로 방향 연산 수행
    for (y = 0; y < height; ++y) {
        // 행 초기화
        backwardPosition = -n2 - 1;
        forwardPosition = n2 - 1;
        pBackwardPixel = (Rgab5515_t*)(pScreen->elements + y * width);
        pForwardPixel = (Rgab5515_t*)(pScreen->elements + y * width);
        pCurrentPixel = (Rgab5515_t*)(pBuffer->elements + y * width);
        sumR = (n2 + 1) * pBackwardPixel->r;
        sumG = (n2 + 1) * pBackwardPixel->g;
        sumB = (n2 + 1) * pBackwardPixel->b;
        for (i = 0; i < n2; ++i) {
            if (i < width)
                pForwardPixel++;
            sumR += pForwardPixel->r;
            sumG += pForwardPixel->g;
            sumB += pForwardPixel->b;
        }

        // 픽셀마다 평균 값 구하기
        for (x = 0; x < width; ++x) {
            backwardPosition++;
            forwardPosition++;
            if (backwardPosition > 0)
                pBackwardPixel++;
            if (forwardPosition < width)
                pForwardPixel++;

            sumR += (int)(pForwardPixel->r) - (int)(pBackwardPixel->r);
            sumG += (int)(pForwardPixel->g) - (int)(pBackwardPixel->g);
            sumB += (int)(pForwardPixel->b) - (int)(pBackwardPixel->b);

            pCurrentPixel->r = sumR / n;
            pCurrentPixel->g = sumG / n;
            pCurrentPixel->b = sumB / n;
            pCurrentPixel++;
        }
    }
    
    // 세로 방향 연산 수행
    for (x = 0; x < width; ++x) {
        // 열 초기화
        backwardPosition = -n2 - 1;
        forwardPosition = n2 - 1;
        pBackwardPixel = (Rgab5515_t*)(pBuffer->elements + x);
        pForwardPixel = (Rgab5515_t*)(pBuffer->elements + x);
        pCurrentPixel = (Rgab5515_t*)(pScreen->elements + x);
        sumR = (n2 + 1) * pBackwardPixel->r;
        sumG = (n2 + 1) * pBackwardPixel->g;
        sumB = (n2 + 1) * pBackwardPixel->b;
        for (i = 0; i < n2; ++i) {
            if (i < height)
                pForwardPixel += width;
            sumR += pForwardPixel->r;
            sumG += pForwardPixel->g;
            sumB += pForwardPixel->b;
        }

        // 픽셀마다 평균 값 구하기
        for (y = 0; y < height; ++y) {
            backwardPosition++;
            forwardPosition++;
            if (backwardPosition > 0)
                pBackwardPixel += width;
            if (forwardPosition < width)
                pForwardPixel += width;

            sumR += (int)(pForwardPixel->r) - (int)(pBackwardPixel->r);
            sumG += (int)(pForwardPixel->g) - (int)(pBackwardPixel->g);
            sumB += (int)(pForwardPixel->b) - (int)(pBackwardPixel->b);

            pCurrentPixel->r = sumR / n;
            pCurrentPixel->g = sumG / n;
            pCurrentPixel->b = sumB / n;
            pCurrentPixel += width;
        }
    }

    destroyScreen(pBuffer);
}

void applyErosionToMatrix8(Matrix8_t* pMatrix, uint8_t n) {
    Matrix8_t* pComparator;

    PixelCoordinate_t width = pMatrix->width;
    PixelCoordinate_t height = pMatrix->height;
    int i;
    int x;
    int y;

    // 가로 방향 침식
    pComparator = cloneMatrix8(pMatrix);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint8_t* pMatrixPixel = &(pMatrix->elements[y * width + x]);
            if (x >= n && x < width - n) {
                if (*pMatrixPixel) {
                    for (i = -n; i <= n; ++i) {
                        *pMatrixPixel &= 
                                pComparator->elements[y * width + x + i];
                    }
                }
            }
            else {
                *pMatrixPixel = 0;
            }
        }
    }
    destroyMatrix8(pComparator);

    // 세로 방향 침식
    pComparator = cloneMatrix8(pMatrix);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint8_t* pMatrixPixel = &(pMatrix->elements[y * width + x]);
            if (y >= n && y < height - n) {
                if (*pMatrixPixel) {
                    for (i = -n; i <= n; ++i) {
                        *pMatrixPixel &= 
                                pComparator->elements[(y + i) * width + x];
                    }
                }
            }
            else {
                *pMatrixPixel = 0;
            }
        }
    }
    destroyMatrix8(pComparator);
}

void applyDilationToMatrix8(Matrix8_t* pMatrix, uint8_t n) {
    Matrix8_t* pComparator;

    PixelCoordinate_t width = pMatrix->width;
    PixelCoordinate_t height = pMatrix->height;
    int i;
    int x;
    int y;

    // 가로 방향 팽창
    pComparator = cloneMatrix8(pMatrix);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint8_t* pMatrixPixel = &(pMatrix->elements[y * width + x]);
            for (i = -n; i <= n; ++i) {
                if (x + i >= 0 && x + i < width) {
                    *pMatrixPixel |= 
                            pComparator->elements[y * width + x + i];
                }
            }
        }
    }
    destroyMatrix8(pComparator);

    // 세로 방향 침식
    pComparator = cloneMatrix8(pMatrix);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            uint8_t* pMatrixPixel = &(pMatrix->elements[y * width + x]);
            for (i = -n; i <= n; ++i) {
                if (y + i >= 0 && y + i < height) {
                    *pMatrixPixel |= 
                            pComparator->elements[(y + i) * width + x];
                }
            }
        }
    }
    destroyMatrix8(pComparator);
}
