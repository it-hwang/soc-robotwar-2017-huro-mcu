#define DEBUG

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
#include "line_detection.h"
#include "polygon_detection.h"
#include "location_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "vertical_barricade.h"
#include "red_bridge.h"
#include "corner_detection.h"
#include "boundary.h"
#include "white_balance.h"
#include "mine.h"
#include "log.h"
#include "screenio.h"
#include "debug.h"

#define PI 3.141592
#define DEG_TO_RAD  (PI / 180)
#define RAD_TO_DEG  (180 / PI)

static const char* _WHITE_BALANCE_TABLE_PATH = "./data/white_balance.lut";
// static ObstacleId_t* _obstacleSequence;


static void _runHuroC(void);
static void _runAdjustWhiteBalance(void);
static void _runCaptureScreen(void);
static void _runTest(void);

static void _adjustWhiteBalance(Rgba_t* pInputColor, Rgba_t* pRealColor);
static void _adjustWhiteBalanceAuto(void);

static void _testBoundary(void);

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
    printDebug("inputColor: {r: %d, g: %d, b: %d}\n",
             pInputColor->r, pInputColor->g, pInputColor->b);
    printDebug("realColor: {r: %d, g: %d, b: %d}\n",
             pRealColor->r, pRealColor->g, pRealColor->b);

    LookUpTable16_t* pWhiteBalanceTable = createWhiteBalanceTable(pInputColor, pRealColor, _WHITE_BALANCE_TABLE_PATH, true);
    if (pWhiteBalanceTable != NULL) {
        printDebug("Adjustment success.\n");
        setDefaultWhiteBalanceTable(pWhiteBalanceTable);
    }
    else {
        printDebug("Adjustment failed.\n");
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
    //runMotion(ROBOT_RELEASE_ARM_SERVOS);
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
static void _hurdleGaeYangArch(void);
static void _testCenter(void);

static void _runTest(void) {
    printLog("Test\n");

    // 작동 알림
    setHead(0, -90);
    sdelay(1);
    setHead(0, 0);
    sdelay(1);

    // 바로 움직이면 위험하므로 잠시 대기한다.
    sdelay(3);


    _testCenter();



    /*
    solveVerticalBarricade();
    checkCenterMain();
    redBridgeMain();
    checkCenterMain();
    mineMain();
    checkCenterMain();
    _hurdleGaeYangArch();
    cornerDetectionMain();
    */
}

static void _hurdleGaeYangArch(void) {
    runWalk(ROBOT_WALK_FORWARD_QUICK, 30);
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
    mdelay(500);
    runMotion(MOTION_HURDLE);
}

static void _setHead(int horizontalDegrees, int verticalDegrees) {
    static const int ERROR_RANGE = 3;

    bool isAlreadySet = true;
    if (abs(getHeadHorizontal() - horizontalDegrees) > ERROR_RANGE)
        isAlreadySet = false;
    if (abs(getHeadVertical() - verticalDegrees) > ERROR_RANGE)
        isAlreadySet = false;

    if (isAlreadySet)
        return;

    setServoSpeed(30);
    setHead(horizontalDegrees, verticalDegrees);
    resetServoSpeed();
    mdelay(800);
}


static void _drawPolygon(Screen_t* pScreen, Polygon_t* pPolygon);
static void _drawLine(Screen_t* pScreen, Line_t* pLine);

static void _testCenter(void) {
    static const int HEAD_HORIZONTAL_DEGREES = 0;
    static const int HEAD_VERTICAL_DEGREES = -80;

    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("머리와 팔 모터의 토크가 해제되었습니다.\n");
    
    //_setHead(HEAD_HORIZONTAL_DEGREES, HEAD_VERTICAL_DEGREES);

    while (true) {
        Screen_t* pScreen = createDefaultScreen();
        readFpgaVideoDataWithWhiteBalance(pScreen);

        Matrix8_t* pColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);
        applyFastErosionToMatrix8(pColorMatrix, 1);
        applyFastDilationToMatrix8(pColorMatrix, 1);

        ObjectList_t* pObjectList = detectObjectsLocation(pColorMatrix);
        Object_t* pObject = findLargestObject(pObjectList);
       
        drawColorMatrix(pScreen, pColorMatrix);
        drawObjectEdge(pScreen, pObject, NULL);
        displayScreen(pScreen);

        if (pObject) {
            CameraParameters_t camParams;
            camParams.height = 0.330;
            camParams.yaw = (double)getHeadHorizontal() * -1 * DEG_TO_RAD;
            camParams.pitch = (double)getHeadVertical() * DEG_TO_RAD;
            camParams.fx = 146.181;
            camParams.fy = 132.462;
            camParams.cx = 94.868;
            camParams.cy = 63.161;
            camParams.k1 = -0.417426;
            camParams.k2 = 0.172889;
            camParams.p1 = -0.004961;
            camParams.p2 = -0.002298;

            PixelLocation_t screenLoc;
            screenLoc.x = (int)pObject->centerX;
            screenLoc.y = (int)pObject->maxY;

            WorldLocation_t worldLoc;
            convertScreenLocationToWorldLocation(&camParams, &screenLoc, &worldLoc);

            printDebug("distance: %f, angle: %f\n", worldLoc.distance * 1000, worldLoc.angle * RAD_TO_DEG);
        }

        destroyMatrix8(pColorMatrix);
        destroyObjectList(pObjectList);
        destroyScreen(pScreen);
    }
}

static void _drawPolygon(Screen_t* pScreen, Polygon_t* pPolygon) {
    if (!pScreen) return;
    if (!pPolygon) return;

    for (int i = 0; i < pPolygon->size; ++i) {
        PixelLocation_t* pVertexLoc = &(pPolygon->vertices[i]);
        int index = pVertexLoc->y * pScreen->width + pVertexLoc->x;
        pScreen->elements[index] = 0xffff;
    }
}

#define _MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define _MAX(X,Y) ((X) > (Y) ? (X) : (Y))
static void _drawLine(Screen_t* pScreen, Line_t* pLine) {
    if (!pScreen) return;
    if (!pLine) return;

    int x1 = pLine->leftPoint.x;
    int y1 = pLine->leftPoint.y;
    int x2 = pLine->rightPoint.x;
    int y2 = pLine->rightPoint.y;
    int minX = _MIN(x1, x2);
    int maxX = _MAX(x1, x2);
    int minY = _MIN(y1, y2);
    int maxY = _MAX(y1, y2);

    int width = pScreen->width;
    if (x1 == x2) {
        int x = x1;
        for (int y = minY; y <= maxY; ++y) {
            int index = y * width + x;
            pScreen->elements[index] = 0x07e0;
        }
    }
    else if (abs(x2 - x1) > abs(y2 - y1)) {
        double a = (double)(y2 - y1) / (x2 - x1);
        for (int x = minX; x <= maxX; ++x) {
            int y = a * (x - x1) + y1;
            int index = y * width + x;
            pScreen->elements[index] = 0x07e0;
        }
    }
    else {
        double a = (double)(x2 - x1) / (y2 - y1);
        for (int y = minY; y <= maxY; ++y) {
            int x = a * (y - y1) + x1;
            int index = y * width + x;
            pScreen->elements[index] = 0x07e0;
        }
    }
}