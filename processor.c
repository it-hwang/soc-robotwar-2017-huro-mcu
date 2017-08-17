#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "processor.h"
#include "color.h"
#include "color_model.h"
#include "graphic_interface.h"
#include "obstacle_manager.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "line_detection.h"
#include "check_center.h"
#include "vertical_barricade.h"
#include "red_bridge.h"
#include "white_balance.h"
#include "log.h"
#include "screenio.h"

static const char* _WHITE_BALANCE_TABLE_PATH = "./data/white_balance.lut";
// static ObstacleId_t* _obstacleSequence;


static void _runHuroC(void);
static void _runAdjustWhiteBalance(void);
static void _runCaptureScreen(void);
static void _runTest(void);

static void _adjustWhiteBalance(Rgba_t* pInputColor, Rgba_t* pRealColor);
static void _adjustWhiteBalanceAuto(void);

static void _defineObstacle(void) {
	// registerObstacle(OBSTACLE_ONE, helloWorld);
	// registerObstacle(OBSTACLE_TWO, goodbyeWorld);
}


int openProcessor(void) {
    if (openGraphicInterface() < 0) {
        closeProcessor();
        return PROCESSOR_GRAPHIC_ERROR;
    }
    if (openRobotPort() < 0) {
        closeProcessor();
        return PROCESSOR_ROBOT_PORT_ERROR;
    }
    initializeColor();
    // Init white balance
    bool isWhiteBalanceTableExists = (access(_WHITE_BALANCE_TABLE_PATH, F_OK) == 0);
    if (isWhiteBalanceTableExists) {
        LookUpTable16_t* pWhiteBalanceTable = createWhiteBalanceTable(NULL, NULL, _WHITE_BALANCE_TABLE_PATH, false);
        setDefaultWhiteBalanceTable(pWhiteBalanceTable);
    }

	_defineObstacle();
	// _obstacleSequence = loadObstaclesFile("/mnt/f0/obstacles.txt");

    return 0;
}

void closeProcessor(void) {
    closeGraphicInterface();
    closeRobotPort();
    finalizeColor();
    resetDefaultWhiteBalanceTable();
    
    // free(_obstacleSequence);
}

int runProcessor(int command) {
    switch (command) {
        case 1:
            // HURO-C
            _runHuroC();
            break;
        
        case 2:
            // Adjust White Balance
            _runAdjustWhiteBalance();
            break;

        case 3:
            // Capture screen
            _runCaptureScreen();
            break;

        case 4:
            // Test
            _runTest();
            break;

        default:
            break;
    }

    return 0;
}


///////////////////////////////////////////////////////////////////////////////
// HURO-C
///////////////////////////////////////////////////////////////////////////////
static void _runHuroC(void) {
    printLog("Start HURO-C\n");
}


///////////////////////////////////////////////////////////////////////////////
// Adjust White Balance
///////////////////////////////////////////////////////////////////////////////
static bool _getYN(void) {
    while (true) {
        printf(">> ");
        char input = getchar();

        if (input != '\n')
            while (getchar() != '\n');

        if (input == 'y' || input == 'Y')
            return true;
        else if (input == 'n' || input == 'N')
            return false;
        else
            printf("잘못된 입력입니다.\n");
    };
}

static void _adjustWhiteBalance(Rgba_t* pInputColor, Rgba_t* pRealColor) {
    static const char* LOG_FUNCTION_NAME = "_adjustWhiteBalance()";

    printLog("[%s] inputColor: {r: %d, g: %d, b: %d}\n", LOG_FUNCTION_NAME,
             pInputColor->r, pInputColor->g, pInputColor->b);
    printLog("[%s] realColor: {r: %d, g: %d, b: %d}\n", LOG_FUNCTION_NAME,
             pRealColor->r, pRealColor->g, pRealColor->b);

    LookUpTable16_t* pWhiteBalanceTable = createWhiteBalanceTable(pInputColor, pRealColor, _WHITE_BALANCE_TABLE_PATH, true);
    if (pWhiteBalanceTable != NULL) {
        printLog("[%s] Adjustment success.\n", LOG_FUNCTION_NAME);
        setDefaultWhiteBalanceTable(pWhiteBalanceTable);
    }
    else {
        printLog("[%s] Adjustment failed.\n", LOG_FUNCTION_NAME);
    }
}


// 매트릭스의 평균 색상을 구합니다.
static Rgab5515_t _getMeanRgab5515OfMatrix16(Matrix16_t* pMatrix) {
    uint64_t totalR = 0;
    uint64_t totalG = 0;
    uint64_t totalB = 0;
    
    int length = pMatrix->width * pMatrix->height;
    Rgab5515_t* pRgab5515 = (Rgab5515_t*)pMatrix->elements;
    for (int i = 0; i < length; ++i) {
        totalR += pRgab5515->r;
        totalG += pRgab5515->g;
        totalB += pRgab5515->b;
        pRgab5515++;
    }

    Rgab5515_t meanRgab5515;
    meanRgab5515.r = totalR / length;
    meanRgab5515.g = totalG / length;
    meanRgab5515.b = totalB / length;
    return meanRgab5515;
}

// pMatrix를 value로 채웁니다.
static void _fillMatrix16(Matrix16_t* pMatrix, uint16_t value) {
    int length = pMatrix->width * pMatrix->height;
    uint16_t* pData = pMatrix->elements;
    for (int i = 0; i < length; ++i) {
        *pData = value;
        pData++;
    }
}

// 고개를 숙여 브라켓에 비치는 회색영역의 색을 기준으로 화이트밸런스 테이블을 생성합니다.
static void _adjustWhiteBalanceAuto(void) {
    setServoSpeed(30);
    setHead(0, -90);
    mdelay(1000);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoData(pScreen);
    displayScreen(pScreen);

    int width = pScreen->width;
    int height = pScreen->height;
    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, 10, height - 20, width - 10, height - 1);
    Rgab5515_t meanRgab5515 = _getMeanRgab5515OfMatrix16(pSubMatrix);
    
    Rgba_t inputColor;
    Rgba_t realColor;
    inputColor.data = rgab5515ToRgbaData(&meanRgab5515);
    realColor.r = 128;
    realColor.g = 128;
    realColor.b = 128;
    _adjustWhiteBalance(&inputColor, &realColor);

    // Debug screen
    _fillMatrix16(pScreen, meanRgab5515.data);
    overlapMatrix16(pSubMatrix, pScreen, 10, height - 20);
    displayScreen(pScreen);

    destroyMatrix16(pSubMatrix);
    destroyScreen(pScreen);

    setHead(0, 0);
    resetServoSpeed();
}

static void _adjustWhiteBalanceManual(void) {
    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("머리와 팔 모터의 토크가 해제되었습니다.\n");

    int r, g, b;
    printf("inputColor: r, g, b를 차례대로 입력하십시오. (범위: 1~254)\n");
    printf(">> ");
    scanf("%d %d %d", &r, &g, &b);

    Rgba_t inputColor;
    inputColor.r = r;
    inputColor.g = g;
    inputColor.b = b;

    printf("realColor: r, g, b를 차례대로 입력하십시오. (범위: 1~254)\n");
    printf(">> ");
    scanf("%d %d %d", &r, &g, &b);

    Rgba_t realColor;
    realColor.r = r;
    realColor.g = g;
    realColor.b = b;
    
    _adjustWhiteBalance(&inputColor, &realColor);

}

static void _runAdjustWhiteBalance(void) {
    printLog("Adjust White Balance\n");

    while (true) {
        printf("\n");
        printf("[화이트 밸런스 설정 메뉴]\n");
        printf("1. 자동 설정\n");
        printf("2. 수동 설정\n");
        printf("3. 화이트 밸런스 시험\n");
        printf("x. 종료\n");

        char input;
        printf(">> ");
        input = getchar();
        if (input != '\n')
            while (getchar() != '\n');

        if (input == '1') {
            printf("[자동 설정]\n");

            _adjustWhiteBalanceAuto();
        }
        else if (input == '2') {
            printf("[수동 설정]\n");

            _adjustWhiteBalanceManual();
        }
        else if (input == '3') {
            printf("[화이트 밸런스 시험]\n");

            Screen_t* pDefaultScreen = createDefaultScreen();
            for (int i = 0; i < 100; ++i) {
                readFpgaVideoDataWithWhiteBalance(pDefaultScreen);
                displayScreen(pDefaultScreen);
            }
            destroyScreen(pDefaultScreen);
        }
        else if (input == 'x' || input == 'X') {
            break;
        }
        else {
            printf("잘못된 입력입니다.\n");
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Capture Screen
///////////////////////////////////////////////////////////////////////////////
static void _autoSaveScreen(Screen_t* pScreen, char* savedFilePath) {
    mkdir("./screenshots", 0755);

    // 적합한 파일 이름 찾기
    int i = 0;
    while (true) {
        sprintf(savedFilePath, "./screenshots/sc%d", i);
        bool isExists = (access(savedFilePath, F_OK) == 0);
        if (!isExists)
            break;
        i++;
    }

    writeScreen(pScreen, savedFilePath);
}


static void _runCaptureScreen(void) {
    printLog("Capture Screen\n");

    enableDirectCameraDisplay();
    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("머리와 팔 모터의 토크가 해제되었습니다.\n");

    while (true) {
        printf("\n");
        printf("[촬영 모드]\n");
        printf("Enter. 촬영\n");
        printf("x. 종료\n");

        char input;
        printf(">> ");
        input = getchar();
        if (input == '\n') {
            disableDirectCameraDisplay();

            Screen_t* pDefaultScreen = createDefaultScreen();
            readFpgaVideoDataWithWhiteBalance(pDefaultScreen);
            displayScreen(pDefaultScreen);

            while (true) {
                printf("[촬영]\n");
                printf("Enter. 저장\n");
                printf("n. 저장 안함\n");

                printf(">> ");
                input = getchar();
                if (input == '\n') {
                    char savedFilePath[1024];
                    _autoSaveScreen(pDefaultScreen, savedFilePath);
                    
                    printf("이미지가 저장되었습니다: %s\n", savedFilePath);
                    break;
                }
                else if (input == 'n' || input == 'N') {
                    while (getchar() != '\n');
                    break;
                }
                else {
                    printf("잘못된 입력입니다.\n");
                    while (getchar() != '\n');
                }
            }

            destroyScreen(pDefaultScreen);
            enableDirectCameraDisplay();
        }
        else if (input == 'x' || input == 'X') {
            break;
        }
        else {
            printf("잘못된 입력입니다.\n");
            while (getchar() != '\n');
        }
    }

    disableDirectCameraDisplay();
}


///////////////////////////////////////////////////////////////////////////////
// Test
///////////////////////////////////////////////////////////////////////////////
static void _runTest(void) {
    printLog("Test\n");

    // 작동 알림
    setHead(0, -90);
    sdelay(1);
    setHead(0, 0);
    sdelay(1);

    // 바로 움직이면 위험하므로 잠시 대기한다.
    sdelay(3);
    
    verticalBarricadeMain();
}

