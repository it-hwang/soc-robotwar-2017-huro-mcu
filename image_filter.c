#include <string.h>
#include <stdio.h>
#include <stdbool.h>

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

void applyFastHeightErosionToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n) {
    PixelCoordinate_t width = pBinaryMatrix->width;
    PixelCoordinate_t height = pBinaryMatrix->height;
    int x;
    int y;
    int count;
    bool needErosion;

    // 세로 방향 침식
    // 위에서 아래로 진행하여 값이 0인 픽셀이 있다면 상하로 번진다.
    for (x = 0; x < width; ++x) {
        count = 0;
        for (y = 0; y < height; ++y) {
            needErosion = pBinaryMatrix->elements[y * width + x] == 0;

            if (count > 0) {
                if (y - n - 1 >= 0)
                    pBinaryMatrix->elements[(y - n - 1) * width + x] = 0;
                if (y < height)
                    pBinaryMatrix->elements[y * width + x] = 0;
                count--;
            }

            if (needErosion)
                count = n;
        }
        // 잔여물 제거
        while (count > 0) {
            pBinaryMatrix->elements[(y - n - 1) * width + x] = 0;
            count--;
            y++;
        }
    }
}

void applyFastWidthErosionToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n) {
    PixelCoordinate_t width = pBinaryMatrix->width;
    PixelCoordinate_t height = pBinaryMatrix->height;
    int x;
    int y;
    int count;
    bool needErosion;

    // 가로 방향 침식
    // 왼쪽에서 오른쪽으로 진행하여 값이 0인 픽셀이 있다면 좌우로 번진다.
    for (y = 0; y < height; ++y) {
        count = 0;
        for (x = 0; x < width; ++x) {
            needErosion = pBinaryMatrix->elements[y * width + x] == 0;

            if (count > 0) {
                if (x - n - 1 >= 0)
                    pBinaryMatrix->elements[y * width + x - n - 1] = 0;
                if (x < width)
                    pBinaryMatrix->elements[y * width + x] = 0;
                count--;
            }

            if (needErosion)
                count = n;
        }
        // 잔여물 제거
        while (count > 0) {
            pBinaryMatrix->elements[y * width + x - n - 1] = 0;
            count--;
            x++;
        }
    }
}

void applyFastErosionToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n) {
    applyFastWidthErosionToMatrix8(pBinaryMatrix, n);
    applyFastHeightErosionToMatrix8(pBinaryMatrix, n);
}

void applyFastWidthDilationToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n) {
    PixelCoordinate_t width = pBinaryMatrix->width;
    PixelCoordinate_t height = pBinaryMatrix->height;
    int x;
    int y;
    int count;
    uint8_t value = 0;
    uint8_t element;
    bool needErosion;

    // 가로 방향 팽창
    // 왼쪽에서 오른쪽으로 진행하여 값이 0이 아닌 픽셀이 있다면 좌우로 번진다.
    for (y = 0; y < height; ++y) {
        count = 0;
        for (x = 0; x < width; ++x) {
            element = pBinaryMatrix->elements[y * width + x];
            needErosion = element != 0;
            if (needErosion)
                value = element;

            if (count > 0) {
                if (x - n - 1 >= 0)
                    pBinaryMatrix->elements[y * width + x - n - 1] = value;
                if (x < width)
                    pBinaryMatrix->elements[y * width + x] = value;
                count--;
            }

            if (needErosion)
                count = n;
        }
        // 잔여물 제거
        while (count > 0) {
            pBinaryMatrix->elements[y * width + x - n - 1] = value;
            count--;
            x++;
        }
    }
}

void applyFastHeightDilationToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n) {
    PixelCoordinate_t width = pBinaryMatrix->width;
    PixelCoordinate_t height = pBinaryMatrix->height;
    int x;
    int y;
    int count;
    uint8_t value = 0;
    uint8_t element;
    bool needErosion;

    // 세로 방향 팽창
    // 위에서 아래로 진행하여 값이 0이 아닌 픽셀이 있다면 상하로 번진다.
    for (x = 0; x < width; ++x) {
        count = 0;
        for (y = 0; y < height; ++y) {
            element = pBinaryMatrix->elements[y * width + x];
            needErosion = element != 0;
            if (needErosion)
                value = element;

            if (count > 0) {
                if (y - n - 1 >= 0)
                    pBinaryMatrix->elements[(y - n - 1) * width + x] = value;
                if (y < height)
                    pBinaryMatrix->elements[y * width + x] = value;
                count--;
            }

            if (needErosion)
                count = n;
        }
        // 잔여물 팽창
        while (count > 0) {
            pBinaryMatrix->elements[(y - n - 1) * width + x] = value;
            count--;
            y++;
        }
    }
}

void applyFastDilationToMatrix8(Matrix8_t* pBinaryMatrix, uint8_t n) {
    applyFastWidthDilationToMatrix8(pBinaryMatrix, n);
    applyFastHeightDilationToMatrix8(pBinaryMatrix, n);
}
