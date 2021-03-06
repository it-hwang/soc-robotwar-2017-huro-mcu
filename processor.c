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
#include "timer.h"
#include "obstacle_manager.h"
#include "object_detection.h"
#include "line_detection.h"
#include "polygon_detection.h"
#include "camera.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "vertical_barricade.h"
#include "red_bridge.h"
#include "corner_detection.h"
#include "green_bridge.h"
#include "boundary.h"
#include "white_balance.h"
#include "mine.h"
#include "trap.h"
#include "hurdle.h"
#include "blue_gate.h"
#include "golf.h"
#include "horizontal_barricade.h"
#include "vector3.h"
#include "log.h"
#include "screenio.h"
#include "debug.h"

#define PI 3.141592
#define DEG_TO_RAD  (PI / 180)
#define RAD_TO_DEG  (180 / PI)

static const char* _WHITE_BALANCE_TABLE_PATH = "./data/white_balance.lut";
static ObstacleSequence_t* _pObstacleSequence;


static void _runHuroC(void);
static void _runAdjustWhiteBalance(void);
static void _runCaptureScreen(void);
static void _runTest(void);

static void _adjustWhiteBalance(Rgba_t* pInputColor, Rgba_t* pRealColor);
static void _adjustWhiteBalanceAuto(void);

static void _defineObstacle(void) {
	registerObstacle(OBSTACLE_VERTICAL_BARRICADE, verticalBarricadeMain);
	registerObstacle(OBSTACLE_RED_BRIDGE, redBridgeMain);
	registerObstacle(OBSTACLE_MINE, mineMain);
	registerObstacle(OBSTACLE_HURDLE, hurdleMain);
	registerObstacle(OBSTACLE_CORNER, cornerDetectionMain);
	registerObstacle(OBSTACLE_BLUE_GATE, blueGateMain);
	registerObstacle(OBSTACLE_GREEN_BRIDGE, greenBridgeMain);
	registerObstacle(OBSTACLE_GOLF, golfMain);
	registerObstacle(OBSTACLE_TRAP, trapMain);
	registerObstacle(OBSTACLE_HORIZONTAL_BARRICADE, horizontalBarricadeMain);
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
    if (openTimer() < 0) {
        closeProcessor();
        return PROCESSOR_TIMER_ERROR;
    }
    initializeColor();
    // Init white balance
    bool isWhiteBalanceTableExists = (access(_WHITE_BALANCE_TABLE_PATH, F_OK) == 0);
    if (isWhiteBalanceTableExists) {
        LookUpTable16_t* pWhiteBalanceTable = createWhiteBalanceTable(NULL, NULL, _WHITE_BALANCE_TABLE_PATH, false);
        setDefaultWhiteBalanceTable(pWhiteBalanceTable);
    }

	_defineObstacle();
	_pObstacleSequence = loadObstaclesFile("./data/obstacles.txt");

    return 0;
}

void closeProcessor(void) {
    closeGraphicInterface();
    closeRobotPort();
    closeTimer();
    finalizeColor();
    resetDefaultWhiteBalanceTable();
    
    destroyObstacleSequence(_pObstacleSequence);
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

    // ?????? ??????
    setHead(0, -90);
    setHead(0, 0);

    // ?????? ???????????? ??????????????? ?????? ????????????.
    sdelay(3);
    
    int index = 0;
    int lastIndex = 0;
    int walkCount = 0;
    while (true) {
        ObstacleId_t id = _pObstacleSequence->elements[index];
        if (runSolveObstacle(id)) {
            if (id != OBSTACLE_MINE && id != OBSTACLE_BLUE_GATE)
                checkCenterMain();
            lastIndex = index;
            walkCount = 0;
        }
    
        index++;
        if (index >= _pObstacleSequence->size)
            index = 0;
        
        if (index == lastIndex) {
            walkForward(128);
            walkCount++;

            if (walkCount % 4 == 0) 
                checkCenterMain();
        }
    }

}


///////////////////////////////////////////////////////////////////////////////
// Adjust White Balance
///////////////////////////////////////////////////////////////////////////////
static bool _getYN(void) {
    while (true) {
        printf(">> ");

        char input = '\0';
        scanf("%c", &input);
        if (input != '\n')
            while (input != '\n') scanf("%c", &input);

        if (input == 'y' || input == 'Y')
            return true;
        else if (input == 'n' || input == 'N')
            return false;
        else
            printf("????????? ???????????????.\n");
    };
}

static void _waitEnter(void) {
    printf(">> ");

    char input = '\0';
    while (input != '\n') scanf("%c", &input);
}

static void _adjustWhiteBalance(Rgba_t* pInputColor, Rgba_t* pRealColor) {
    printLog("inputColor: {r: %d, g: %d, b: %d}\n",
             pInputColor->r, pInputColor->g, pInputColor->b);
    printLog("realColor: {r: %d, g: %d, b: %d}\n",
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


// ??????????????? ?????? ????????? ????????????.
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

// pMatrix??? value??? ????????????.
static void _fillMatrix16(Matrix16_t* pMatrix, uint16_t value) {
    int length = pMatrix->width * pMatrix->height;
    uint16_t* pData = pMatrix->elements;
    for (int i = 0; i < length; ++i) {
        *pData = value;
        pData++;
    }
}

// ?????? ???????????? ??????/?????? 20% ????????? ?????? ????????? ?????????.
static Rgab5515_t _getSampleColor(void) {
    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoData(pScreen);

    int width = pScreen->width;
    int height = pScreen->height;
    int subWidth = width * 0.2;
    int subHeight = height * 0.2;
    int centerX = width * 0.5;
    int centerY = height * 0.5;
    int minX = centerX - subWidth * 0.5;
    int minY = centerY - subHeight * 0.5;
    int maxX = minX + subWidth - 1;
    int maxY = minY + subHeight - 1;
    Matrix16_t* pSubMatrix = createSubMatrix16(pScreen, minX, minY, maxX, maxY);
    Rgab5515_t meanRgab5515 = _getMeanRgab5515OfMatrix16(pSubMatrix);

    // Debug screen
    _fillMatrix16(pScreen, meanRgab5515.data);
    overlapMatrix16(pSubMatrix, pScreen, minX, minY);
    displayScreen(pScreen);

    destroyMatrix16(pSubMatrix);
    destroyScreen(pScreen);

    return meanRgab5515;
}

static void _adjustWhiteBalanceAuto(void) {
    setServoSpeed(30);
    setHead(0, -60);
    sdelay(1);
    resetServoSpeed();

    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("????????? ??? ????????? ????????? ?????????????????????.\n");

    printf("????????? ???????????? ???????????????.\n");
    printf("?????? ??????????????? ???????????????, ????????? ???????????? Enter ?????? ????????????.\n");

    printf("1. ?????? ????????? ????????? ????????? ?????????.\n");
    enableDirectCameraDisplay();
    _waitEnter();
    disableDirectCameraDisplay();

    printf("2. ???????????? ?????? ????????? ????????? ????????? '?????????' ????????????.\n");
    enableDirectCameraDisplay();
    _waitEnter();
    disableDirectCameraDisplay();
    Rgab5515_t realRgab5515 = _getSampleColor();
    sdelay(1);

    printf("3. ???????????? ?????? ????????? ????????? ????????? '????????????' ????????????.\n");
    enableDirectCameraDisplay();
    _waitEnter();
    disableDirectCameraDisplay();
    Rgab5515_t inputRgab5515 = _getSampleColor();
    sdelay(1);
    
    Rgba_t inputColor;
    Rgba_t realColor;
    inputColor.data = rgab5515ToRgbaData(&inputRgab5515);
    realColor.data = rgab5515ToRgbaData(&realRgab5515);
    _adjustWhiteBalance(&inputColor, &realColor);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);
    displayScreen(pScreen);
    destroyScreen(pScreen);

    setHead(0, 0);
    resetServoSpeed();
}

static void _adjustWhiteBalanceSemiAuto(void) {
    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("????????? ??? ????????? ????????? ?????????????????????.\n");

    printf("????????? ???????????? ???????????????.\n");
    printf("?????? ??????????????? ???????????????, ????????? ???????????? Enter ?????? ????????????.\n");

    printf("1. ?????? ????????? ????????? ????????? ?????????.\n");
    enableDirectCameraDisplay();
    _waitEnter();
    disableDirectCameraDisplay();

    printf("2. ???????????? ?????? ????????? ????????? ????????? '????????????' ????????????.\n");
    enableDirectCameraDisplay();
    _waitEnter();
    disableDirectCameraDisplay();
    Rgab5515_t inputRgab5515 = _getSampleColor();
    
    int r, g, b;
    printf("realColor: r, g, b??? ???????????? ??????????????????. (??????: 0~255)\n");
    printf(">> ");
    scanf("%d %d %d", &r, &g, &b);

    Rgba_t realColor;
    realColor.r = r;
    realColor.g = g;
    realColor.b = b;
    
    Rgba_t inputColor;
    inputColor.data = rgab5515ToRgbaData(&inputRgab5515);
    _adjustWhiteBalance(&inputColor, &realColor);

    Screen_t* pScreen = createDefaultScreen();
    readFpgaVideoDataWithWhiteBalance(pScreen);
    displayScreen(pScreen);
    destroyScreen(pScreen);
}

static void _adjustWhiteBalanceManual(void) {
    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("????????? ??? ????????? ????????? ?????????????????????.\n");

    int r, g, b;
    printf("inputColor: r, g, b??? ???????????? ??????????????????. (??????: 1~254)\n");
    printf(">> ");
    scanf("%d %d %d", &r, &g, &b);

    Rgba_t inputColor;
    inputColor.r = r;
    inputColor.g = g;
    inputColor.b = b;

    printf("realColor: r, g, b??? ???????????? ??????????????????. (??????: 0~255)\n");
    printf(">> ");
    scanf("%d %d %d", &r, &g, &b);

    Rgba_t realColor;
    realColor.r = r;
    realColor.g = g;
    realColor.b = b;
    
    _adjustWhiteBalance(&inputColor, &realColor);
}

static void _testWhiteBalance(void) {
    Screen_t* pDefaultScreen = createDefaultScreen();
    for (int i = 0; i < 100; ++i) {
        readFpgaVideoDataWithWhiteBalance(pDefaultScreen);
        displayScreen(pDefaultScreen);
    }
    destroyScreen(pDefaultScreen);
}

static void _testColorTable(void) {
    int color;
    printf("????????? ??????????????????.\n");
    printf("1. COLOR_BLACK\n");
    printf("2. COLOR_WHITE\n");
    printf("3. COLOR_RED\n");
    printf("4. COLOR_GREEN\n");
    printf("5. COLOR_BLUE\n");
    printf("6. COLOR_YELLOW\n");
    printf("7. COLOR_ORANGE\n");
    printf("8. COLOR_BLACK2\n");
    printf(">> ");
    scanf("%d", &color);

    if (color < 1 || color >= MAX_COLOR) {
        printf("????????? ???????????????.\n");
        return;
    }

    Screen_t* pDefaultScreen = createDefaultScreen();
    for (int i = 0; i < 100; ++i) {
        readFpgaVideoDataWithWhiteBalance(pDefaultScreen);
        Matrix8_t* pColorMatrix = createColorMatrix(pDefaultScreen, pColorTables[color]);
        drawColorMatrix(pDefaultScreen, pColorMatrix);
        displayScreen(pDefaultScreen);
        destroyMatrix8(pColorMatrix);
    }
    destroyScreen(pDefaultScreen);
}

static void _runAdjustWhiteBalance(void) {
    printLog("Adjust White Balance\n");

    while (true) {
        printf("\n");
        printf("[????????? ????????? ?????? ??????]\n");
        printf("1. ?????? ??????\n");
        printf("2. ????????? ??????\n");
        printf("3. ?????? ??????\n");
        printf("4. ????????? ????????? ??????\n");
        printf("5. ?????? ????????? ??????\n");
        printf("x. ??????\n");

        char input;
        printf(">> ");
        input = getchar();
        if (input != '\n')
            while (getchar() != '\n');

        if (input == '1') {
            printf("[?????? ??????]\n");
            _adjustWhiteBalanceAuto();
        }
        else if (input == '2') {
            printf("[????????? ??????]\n");
            _adjustWhiteBalanceSemiAuto();
        }
        else if (input == '3') {
            printf("[?????? ??????]\n");
            _adjustWhiteBalanceManual();
        }
        else if (input == '4') {
            printf("[????????? ????????? ??????]\n");
            _testWhiteBalance();
        }
        else if (input == '5') {
            printf("[?????? ????????? ??????]\n");
            _testColorTable();
        }
        else if (input == 'x' || input == 'X') {
            break;
        }
        else {
            printf("????????? ???????????????.\n");
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
// Capture Screen
///////////////////////////////////////////////////////////////////////////////
static void _autoSaveScreen(Screen_t* pScreen, char* savedFilePath) {
    mkdir("./screenshots", 0755);

    // ????????? ?????? ?????? ??????
    int i = 0;
    while (true) {
        sprintf(savedFilePath, "./screenshots/sc%d.bmp", i);
        bool isExists = (access(savedFilePath, F_OK) == 0);
        if (!isExists)
            break;
        i++;
    }

    saveScreen(pScreen, savedFilePath);
}


static void _runCaptureScreen(void) {
    printLog("Capture Screen\n");

    enableDirectCameraDisplay();
    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("????????? ??? ????????? ????????? ?????????????????????.\n");

    while (true) {
        printf("\n");
        printf("[?????? ??????]\n");
        printf("Enter. ??????\n");
        printf("x. ??????\n");

        char input;
        printf(">> ");
        input = getchar();
        if (input == '\n') {
            disableDirectCameraDisplay();

            Screen_t* pDefaultScreen = createDefaultScreen();
            readFpgaVideoDataWithWhiteBalance(pDefaultScreen);
            displayScreen(pDefaultScreen);

            while (true) {
                printf("[??????]\n");
                printf("Enter. ??????\n");
                printf("n. ?????? ??????\n");

                printf(">> ");
                input = getchar();
                if (input == '\n') {
                    char savedFilePath[1024];
                    _autoSaveScreen(pDefaultScreen, savedFilePath);
                    
                    printf("???????????? ?????????????????????: %s\n", savedFilePath);
                    break;
                }
                else if (input == 'n' || input == 'N') {
                    while (getchar() != '\n');
                    break;
                }
                else {
                    printf("????????? ???????????????.\n");
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
            printf("????????? ???????????????.\n");
            while (getchar() != '\n');
        }
    }

    disableDirectCameraDisplay();
}

///////////////////////////////////////////////////////////////////////////////
// Test
///////////////////////////////////////////////////////////////////////////////
static void _testWorldLoc(void);

static void _runTest(void) {
    printLog("Test\n");

    // ?????? ??????
    setHead(0, -90);
    sdelay(1);
    setHead(0, 0);
    sdelay(1);

    // ?????? ???????????? ??????????????? ?????? ????????????.
    sdelay(3);
    
    // for(int i = 0; i < 5000; ++i) {
        golfMain();
        cornerDetectionMain();
        trapMain();
        blueGateMain();
        horizontalBarricadeMain();
    // }
}

static void _testBoundary(void) {
    Screen_t* pScreen = createDefaultScreen();

    readFpgaVideoDataWithWhiteBalance(pScreen);

    Matrix8_t* pWhiteColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);
    Matrix8_t* pBlueColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);

    Matrix8_t* pMergedColorMatrix = 
             overlapColorMatrix(pBlueColorMatrix, pWhiteColorMatrix);

    trapMain();
}

static void _testWorldLoc(void) {
    runMotion(ROBOT_RELEASE_ARM_SERVOS);
    printf("????????? ??? ????????? ????????? ?????????????????????.\n");

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
            Vector3_t headOffset = {0.0, -0.020, 0.296};
            CameraParameters_t camParams;
            readCameraParameters(&camParams, &headOffset);

            PixelLocation_t screenLoc = {(int)pObject->centerX, pObject->maxY};
            Vector3_t worldLoc;
            convertScreenLocationToWorldLocation(&camParams, &screenLoc, 0.0, &worldLoc);

            printDebug("x: %.0f, y: %.0f, z: %.0f\n", worldLoc.x * 1000, worldLoc.y * 1000, worldLoc.z * 1000);
        }

        destroyMatrix8(pColorMatrix);
        destroyObjectList(pObjectList);
        destroyScreen(pScreen);
    }
}
