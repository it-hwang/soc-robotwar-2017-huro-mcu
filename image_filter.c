#include <string.h>
#include <stdio.h>

#include "image_filter.h"
#include "color.h"

// n x n 크기의 평균 필터를 씌웁니다.
void applyMeanFilter(Screen_t* pScreen, PixelCoordinate_t n) {
    // n은 홀수
    if (n % 2 == 0)
        n += 1;
    int n2 = n / 2;

    Screen_t* pBuffer = createScreen(pScreen->width, pScreen->height);
    int width = pScreen->width;
    int height = pScreen->height;
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
        pBackwardPixel = (Rgab5515_t*)(pScreen->pixels + y * width);
        pForwardPixel = (Rgab5515_t*)(pScreen->pixels + y * width);
        pCurrentPixel = (Rgab5515_t*)(pBuffer->pixels + y * width);
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
        pBackwardPixel = (Rgab5515_t*)(pBuffer->pixels + x);
        pForwardPixel = (Rgab5515_t*)(pBuffer->pixels + x);
        pCurrentPixel = (Rgab5515_t*)(pScreen->pixels + x);
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
