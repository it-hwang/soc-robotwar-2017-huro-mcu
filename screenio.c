#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screenio.h"

const int _VERSION = 1;

Screen_t* _loadScreen(const char* filePath);
void _saveScreen(Screen_t* pScreen, const char* filePath);


void printScreen(Screen_t* pScreen) {
    int length = pScreen->width * pScreen->height;
    int i;

    printf("========= Print Screen =========\n");
    printf("PrintScreen\n");
    printf("Version   : %d\n", _VERSION);
    printf("Pixel Size: %d\n", sizeof(pScreen->elements[0]));
    printf("Width     : %d\n", pScreen->width);
    printf("Height    : %d\n", pScreen->height);

    for (i = 0; i < length; ++i) {
        printf("%x ", pScreen->elements[i]);
    }
    printf("\n");
    printf("================================\n");
}

bool writeScreen(Screen_t* pScreen, const char* filePath) {
    FILE* fd = fopen(filePath, "w");
    if (fd == NULL)
        return false;

    int length = pScreen->width * pScreen->height;
    int i;

    fprintf(fd, "========= Print Screen =========\n");
    fprintf(fd, "PrintScreen\n");
    fprintf(fd, "Version   : %d\n", _VERSION);
    fprintf(fd, "Pixel Size: %d\n", sizeof(pScreen->elements[0]));
    fprintf(fd, "Width     : %d\n", pScreen->width);
    fprintf(fd, "Height    : %d\n", pScreen->height);

    for (i = 0; i < length; ++i) {
        fprintf(fd, "%x ", pScreen->elements[i]);
    }
    fprintf(fd, "\n");
    fprintf(fd, "================================\n");

    fclose(fd);

    return true;
}

Screen_t* scanScreen(const char* filePath) {
    FILE* pFile = fopen(filePath, "r");
    if (pFile == NULL)
        return NULL;

    char buffer[1024];
    fscanf(pFile, "%s\n", buffer);
    bool isValid = (strcmp(buffer, "PrintScreen") == 0);
    if (!isValid)
        return NULL;

    int version;
    fscanf(pFile, "Version   : %d\n", &version);
    if (version != _VERSION)
        return NULL;

    int pixelSize;
    int width;
    int height;
    fscanf(pFile, "Pixel Size: %d\n", &pixelSize);
    fscanf(pFile, "Width     : %d\n", &width);
    fscanf(pFile, "Height    : %d\n", &height);

    Screen_t* pScreen = createScreen(width, height);
    if (pixelSize != sizeof(pScreen->elements[0])) {
        destroyScreen(pScreen);
        return NULL;
    }

    int length = width * height;
    int i;
    for (i = 0; i < length; ++i) {
        int data;
        fscanf(pFile, "%d", &data);
        pScreen->elements[i] = data;
    }

    fclose(pFile);

    return pScreen;
}

void saveScreen(Screen_t* pScreen, const char* filePath) {
    _saveScreen(pScreen, filePath);
}

Screen_t* loadScreen(const char* filePath)
{
    return _loadScreen(filePath);
}

void printFpgaVideoData(void) {
    Screen_t* pDefaultScreen = createDefaultScreen();

    readFpgaVideoData(pDefaultScreen);
    printScreen(pDefaultScreen);

    destroyScreen(pDefaultScreen);
}


// 다음은 비트맵 파일 입출력에 관한 코드이다.
// TODO: 이 곳에 다음 코드들이 오는 것이 적절한지 검토해야한다.
//       "screenio.c"에 위치하는게 좋을지 다시 생각해보아야 한다.
// 출처: C 언어 코딩 도장

#pragma pack(push, 1)                // 구조체를 1바이트 크기로 정렬

typedef struct _BITMAPFILEHEADER     // BMP 비트맵 파일 헤더 구조체
{
    unsigned short bfType;           // BMP 파일 매직 넘버
    unsigned int   bfSize;           // 파일 크기
    unsigned short bfReserved1;      // 예약
    unsigned short bfReserved2;      // 예약
    unsigned int   bfOffBits;        // 비트맵 데이터의 시작 위치
} BITMAPFILEHEADER;

typedef struct _BITMAPINFOHEADER     // BMP 비트맵 정보 헤더 구조체(DIB 헤더)
{
    unsigned int   biSize;           // 현재 구조체의 크기
    int            biWidth;          // 비트맵 이미지의 가로 크기
    int            biHeight;         // 비트맵 이미지의 세로 크기
    unsigned short biPlanes;         // 사용하는 색상판의 수
    unsigned short biBitCount;       // 픽셀 하나를 표현하는 비트 수
    unsigned int   biCompression;    // 압축 방식
    unsigned int   biSizeImage;      // 비트맵 이미지의 픽셀 데이터 크기
    int            biXPelsPerMeter;  // 그림의 가로 해상도(미터당 픽셀)
    int            biYPelsPerMeter;  // 그림의 세로 해상도(미터당 픽셀)
    unsigned int   biClrUsed;        // 색상 테이블에서 실제 사용되는 색상 수
    unsigned int   biClrImportant;   // 비트맵을 표현하기 위해 필요한 색상 인덱스 수
} BITMAPINFOHEADER;

typedef struct _RGBTRIPLE            // 24비트 비트맵 이미지의 픽셀 구조체
{
    unsigned char rgbtBlue;          // 파랑
    unsigned char rgbtGreen;         // 초록
    unsigned char rgbtRed;           // 빨강
} RGBTRIPLE;

#pragma pack(pop)

#define PIXEL_SIZE   3    // 픽셀 한 개의 크기 3바이트(24비트)
#define PIXEL_ALIGN  4    // 픽셀 데이터 가로 한 줄은 4의 배수 크기로 저장됨


Screen_t* _loadScreen(const char* filePath)
{
    FILE* fpBmp;                    // 비트맵 파일 포인터
    BITMAPFILEHEADER fileHeader;    // 비트맵 파일 헤더 구조체 변수
    BITMAPINFOHEADER infoHeader;    // 비트맵 정보 헤더 구조체 변수

    unsigned char* image;    // 픽셀 데이터 포인터
    int size;                // 픽셀 데이터 크기
    int width, height;       // 비트맵 이미지의 가로, 세로 크기
    int padding;             // 픽셀 데이터의 가로 크기가 4의 배수가 아닐 때 남는 공간의 크기
    bool isInversed = false;

    fpBmp = fopen(filePath, "rb");    // 비트맵 파일을 바이너리 모드로 열기
    if (fpBmp == NULL)    // 파일 열기에 실패하면
        return NULL;      // 프로그램 종료

                          // 비트맵 파일 헤더 읽기. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpBmp) < 1)
    {
        fclose(fpBmp);
        return NULL;
    }

    // 매직 넘버가 MB가 맞는지 확인(2바이트 크기의 BM을 리틀 엔디언으로 읽었으므로 MB가 됨)
    // 매직 넘버가 맞지 않으면 프로그램 종료
    if (fileHeader.bfType != 'MB')
    {
        fclose(fpBmp);
        return NULL;
    }

    // 비트맵 정보 헤더 읽기. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpBmp) < 1)
    {
        fclose(fpBmp);
        return NULL;
    }

    // 24비트 비트맵이 아니면 프로그램 종료
    if (infoHeader.biBitCount != 24)
    {
        fclose(fpBmp);
        return NULL;
    }

    size = infoHeader.biSizeImage;    // 픽셀 데이터 크기
    width = infoHeader.biWidth;       // 비트맵 이미지의 가로 크기
    height = infoHeader.biHeight;     // 비트맵 이미지의 세로 크기
    if (height < 0) {
        height = -height;
        isInversed = true;
    }

                                      // 이미지의 가로 크기에 픽셀 크기를 곱하여 가로 한 줄의 크기를 구하고 4로 나머지를 구함
                                      // 그리고 4에서 나머지를 빼주면 남는 공간을 구할 수 있음.
                                      // 만약 남는 공간이 0이라면 최종 결과가 4가 되므로 여기서 다시 4로 나머지를 구함
    padding = (PIXEL_ALIGN - ((width * PIXEL_SIZE) % PIXEL_ALIGN)) % PIXEL_ALIGN;

    if (size == 0)    // 픽셀 데이터 크기가 0이라면
    {
        // 이미지의 가로 크기 * 픽셀 크기에 남는 공간을 더해주면 완전한 가로 한 줄 크기가 나옴
        // 여기에 이미지의 세로 크기를 곱해주면 픽셀 데이터의 크기를 구할 수 있음
        size = (width * PIXEL_SIZE + padding) * height;
    }

    image = malloc(size);    // 픽셀 데이터의 크기만큼 동적 메모리 할당

                             // 파일 포인터를 픽셀 데이터의 시작 위치로 이동
    fseek(fpBmp, fileHeader.bfOffBits, SEEK_SET);

    // 파일에서 픽셀 데이터 크기만큼 읽음. 읽기에 실패하면 파일 포인터를 닫고 프로그램 종료
    if (fread(image, size, 1, fpBmp) < 1)
    {
        fclose(fpBmp);
        free(image);
        return NULL;
    }

    fclose(fpBmp);    // 비트맵 파일 닫기

    int widthBytes = width * PIXEL_SIZE + padding;

    // 픽셀 데이터를 스크린으로 옮김
    Screen_t* pScreen = createScreen(width, height);

    uint16_t* lpRgb565Buffer = (uint16_t*)(pScreen->elements);
    int i;
    int j;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            int dataIndex = i * widthBytes + j * PIXEL_SIZE;
            uint8_t r = image[dataIndex + 2];
            uint8_t g = image[dataIndex + 1];
            uint8_t b = image[dataIndex + 0];
            uint16_t rgb565Data = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);
            int y = (isInversed ? i : (height - 1) - i);
            int bufferIndex = (y * pScreen->width) + j;
            lpRgb565Buffer[bufferIndex] = rgb565Data;
        }
    }

    free(image);

    return pScreen;
}

// TODO: 구조체를 fwrite하는 것이 제대로 되는지 검증이 필요하다.
//       일단은 잘 작동되는 것 처럼 보이지만, 헥스에딧으로 열어보니
//       생각과는 다른 파일구조를 가진 것으로 보였다.
void _saveScreen(Screen_t* pScreen, const char* filePath) {
    unsigned char* image;    // 픽셀 데이터 포인터
    int size;                // 픽셀 데이터 크기
    int width, height;       // 비트맵 이미지의 가로, 세로 크기
    int padding;             // 픽셀 데이터의 가로 크기가 4의 배수가 아닐 때 남는 공간의 크기
    int widthBytes;          // 픽셀 데이터의 패딩을 포함한 가로 크기

    width = pScreen->width;       // 비트맵 이미지의 가로 크기
    height = pScreen->height;     // 비트맵 이미지의 세로 크기

    // 이미지의 가로 크기에 픽셀 크기를 곱하여 가로 한 줄의 크기를 구하고 4로 나머지를 구함
    // 그리고 4에서 나머지를 빼주면 남는 공간을 구할 수 있음.
    // 만약 남는 공간이 0이라면 최종 결과가 4가 되므로 여기서 다시 4로 나머지를 구함
    padding = (PIXEL_ALIGN - ((width * PIXEL_SIZE) % PIXEL_ALIGN)) % PIXEL_ALIGN;

    widthBytes = width * PIXEL_SIZE + padding;
    size = widthBytes * height;    // 픽셀 데이터 크기

    image = malloc(size);    // 픽셀 데이터의 크기만큼 동적 메모리 할당

    uint16_t* lpRgb565Buffer = (uint16_t*)(pScreen->elements);
    int i;
    int j;
    for (i = 0; i < height; ++i) {
        for (j = 0; j < width; ++j) {
            int bufferIndex = ((height - 1) - i) * pScreen->width + j;
            int dataIndex = i * widthBytes + j * PIXEL_SIZE;
            uint16_t rgb565 = lpRgb565Buffer[bufferIndex];
            uint8_t r = (rgb565 >> 8) & 0xf8;
            uint8_t g = (rgb565 >> 3) & 0xfc;
            uint8_t b = (rgb565 << 3) & 0xf8;
            image[dataIndex + 2] = r;
            image[dataIndex + 1] = g;
            image[dataIndex + 0] = b;
        }
    }

    FILE* fpBmp;                    // 비트맵 파일 포인터
    BITMAPFILEHEADER fileHeader;    // 비트맵 파일 헤더 구조체 변수
    BITMAPINFOHEADER infoHeader;    // 비트맵 정보 헤더 구조체 변수

    fpBmp = fopen(filePath, "wb");    // 비트맵 파일을 바이너리 모드로 열기
    if (fpBmp == NULL) {  // 파일 열기에 실패하면
        free(image);
        return;      // 프로그램 종료
    }


    // fileHeader 설정
    fileHeader.bfType = 'MB';
    fileHeader.bfSize = sizeof(fileHeader) + sizeof(infoHeader) + size;
    fileHeader.bfReserved1 = 0x00;
    fileHeader.bfReserved2 = 0x00;
    fileHeader.bfOffBits = fileHeader.bfSize - size;

    // infoHeader 설정
    infoHeader.biSize = sizeof(infoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = size;
    infoHeader.biXPelsPerMeter = 0;
    infoHeader.biYPelsPerMeter = 0;
    infoHeader.biClrUsed = 0;
    infoHeader.biClrImportant = 0;

    fwrite(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fpBmp);
    fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fpBmp);
    fwrite(image, size, 1, fpBmp);

    free(image);
}
