#define DEBUG

#include <stdlib.h>
#include <string.h>

#include "golf.h"
#include "vector3.h"
#include "graphic_interface.h"
#include "white_balance.h"
#include "color.h"
#include "camera.h"
#include "robot_protocol.h"
#include "object_detection.h"
#include "image_filter.h"
#include "boundary.h"
#include "log.h"
#include "debug.h"


#define _GOLF_BALL_RADIUS   0.020

static bool _findObjects(Vector3_t* pBallLoc, Vector3_t* pHoleLoc);
static bool _searchGolfBall(Screen_t* pInputScreen, Object_t* pReturnedObject);
static bool _searchGolfHole(Screen_t* pInputScreen, Object_t* pReturnedObject);
static void _establishBoundary(Screen_t* pScreen);
static double _getGolfBallCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject);
static double _getGolfHoleCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject);
static void _setHead(int horizontalDegrees, int verticalDegrees);
static void _drawColorMatrixAdditive(Screen_t* pScreen, Matrix8_t* pColorMatrix);


bool golfMain(void) {
    solveGolf();
    return false;
}

int measureGolfDistance(void) {
    return 0;
}

bool solveGolf(void) {
    Vector3_t ballLoc;
    Vector3_t holeLoc;

    bool hasFound = _findObjects(&ballLoc, &holeLoc);
    if (hasFound) {
        printDebug("ballLoc: {x: %.0f, y: %.0f, z: %.0f}\n", ballLoc.x * 1000, ballLoc.y * 1000, ballLoc.z * 1000);
        printDebug("holeLoc: {x: %.0f, y: %.0f, z: %.0f}\n", holeLoc.x * 1000, holeLoc.y * 1000, holeLoc.z * 1000);
    }
    else {
        printDebug("Failed.\n");
    }

    return false;
}

static bool _findObjects(Vector3_t* pBallLoc, Vector3_t* pHoleLoc) {    
    const int HEAD_HORIZONTAL_DEGREES[] = {   0,   0, -45,  45 };
    const int HEAD_VERTICAL_DEGREES[]   = { -80, -40, -40, -40 };
    const Vector3_t HEAD_OFFSET = { 0.000, -0.020, 0.296 };

    setServoSpeed(10);
    runMotion(MOTION_BASIC_STANCE);
    resetServoSpeed();

    bool isBallFound = (!pBallLoc);
    bool isHoleFound = (!pHoleLoc);

    int l = sizeof(HEAD_HORIZONTAL_DEGREES) / sizeof(HEAD_HORIZONTAL_DEGREES[0]);
    for (int i = 0; i < l; ++i) {
        if (isBallFound && isHoleFound) break;

        _setHead(HEAD_HORIZONTAL_DEGREES[i], HEAD_VERTICAL_DEGREES[i]);

        Screen_t* pScreen = createDefaultScreen();
        readFpgaVideoDataWithWhiteBalance(pScreen);
        if (!isBallFound) {
            Object_t object;
            bool hasFound = _searchGolfBall(pScreen, &object);
            if (hasFound) {
                CameraParameters_t camParams;
                readCameraParameters(&camParams, &HEAD_OFFSET);

                PixelLocation_t screenLoc = { (int)object.centerX, (int)object.centerY };
                convertScreenLocationToWorldLocation(&camParams, &screenLoc, _GOLF_BALL_RADIUS, pBallLoc);
                isBallFound = true;
            }
        }
        if (!isHoleFound) {
            Object_t object;
            bool hasFound = _searchGolfHole(pScreen, &object);
            if (hasFound) {
                CameraParameters_t camParams;
                readCameraParameters(&camParams, &HEAD_OFFSET);

                Vector3_t worldLoc;
                PixelLocation_t screenLoc = { (int)object.centerX, (int)object.centerY };
                convertScreenLocationToWorldLocation(&camParams, &screenLoc, 0., pHoleLoc);
                isHoleFound = true;
            }
        }

        destroyScreen(pScreen);
    }

    return (isBallFound && isHoleFound);
}

static bool _searchGolfBall(Screen_t* pInputScreen, Object_t* pReturnedObject) {
    const double MIN_CORRELATION = 0.00;

    if (!pInputScreen) return false;

    Screen_t* pScreen = cloneMatrix16(pInputScreen);
    _establishBoundary(pScreen);

    Matrix8_t* pColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_ORANGE]);
    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 3);
    applyFastErosionToMatrix8(pColorMatrix, 2);

    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);
    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    Object_t* pMostSimilarObject = NULL;
    if (pObjectList) {
        double maxCorrelation = 0.;
        for (int i = 0; i < pObjectList->size; ++i) {
            Object_t* pObject = &(pObjectList->list[i]);

            double correlation = _getGolfBallCorrelation(pLabelMatrix, pObject);
            if (correlation < MIN_CORRELATION) continue;

            if (correlation > maxCorrelation) {
                pMostSimilarObject = pObject;
                maxCorrelation = correlation;
            }
        }
    }
    bool hasFound = (pMostSimilarObject != NULL);

    if (pMostSimilarObject && pReturnedObject) {
        memcpy(pReturnedObject, pMostSimilarObject, sizeof(Object_t));
    }
    
    // Display debug screen
    _drawColorMatrixAdditive(pScreen, pColorMatrix);
    drawObjectEdge(pScreen, pMostSimilarObject, NULL);
    displayScreen(pScreen);
    sdelay(3);

    destroyObjectList(pObjectList);
    destroyMatrix16(pLabelMatrix);
    destroyMatrix8(pColorMatrix);
    destroyScreen(pScreen);
    return hasFound;
}

static bool _searchGolfHole(Screen_t* pInputScreen, Object_t* pReturnedObject) {
    const double MIN_CORRELATION = 0.00;

    if (!pInputScreen) return false;

    Screen_t* pScreen = cloneMatrix16(pInputScreen);
    _establishBoundary(pScreen);

    Matrix8_t* pColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);
    applyFastDilationToMatrix8(pColorMatrix, 1);
    applyFastErosionToMatrix8(pColorMatrix, 2);
    applyFastDilationToMatrix8(pColorMatrix, 3);
    applyFastErosionToMatrix8(pColorMatrix, 2);

    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);
    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    Object_t* pMostSimilarObject = NULL;
    if (pObjectList) {
        double maxCorrelation = 0.;
        for (int i = 0; i < pObjectList->size; ++i) {
            Object_t* pObject = &(pObjectList->list[i]);

            double correlation = _getGolfHoleCorrelation(pLabelMatrix, pObject);
            if (correlation < MIN_CORRELATION) continue;
            
            if (correlation > maxCorrelation) {
                pMostSimilarObject = pObject;
                maxCorrelation = correlation;
            }
        }
    }
    bool hasFound = (pMostSimilarObject != NULL);

    if (pMostSimilarObject && pReturnedObject) {
        memcpy(pReturnedObject, pMostSimilarObject, sizeof(Object_t));
    }
    
    // Display debug screen
    _drawColorMatrixAdditive(pScreen, pColorMatrix);
    drawObjectEdge(pScreen, pMostSimilarObject, NULL);
    displayScreen(pScreen);
    sdelay(3);

    destroyObjectList(pObjectList);
    destroyMatrix16(pLabelMatrix);
    destroyMatrix8(pColorMatrix);
    destroyScreen(pScreen);
    return false;
    return hasFound;
}

static void _establishBoundary(Screen_t* pScreen) {
    Matrix8_t* pWhiteColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);
    Matrix8_t* pOrangeColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_ORANGE]);
    Matrix8_t* pBlueColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);
    Matrix8_t* pMergedColorMatrix = overlapColorMatrix(pOrangeColorMatrix, pWhiteColorMatrix);
    Matrix8_t* pMergedColorMatrix2 = overlapColorMatrix(pBlueColorMatrix, pMergedColorMatrix);

    applyFastErosionToMatrix8(pMergedColorMatrix2, 1);
    applyFastDilationToMatrix8(pMergedColorMatrix2, 1);

    Matrix8_t* pBoundaryMatrix = establishBoundary(pMergedColorMatrix2);

    applyBoundary(pScreen, pBoundaryMatrix);

    destroyMatrix8(pWhiteColorMatrix);
    destroyMatrix8(pOrangeColorMatrix);
    destroyMatrix8(pBlueColorMatrix);
    destroyMatrix8(pMergedColorMatrix);
    destroyMatrix8(pMergedColorMatrix2);
    destroyMatrix8(pBoundaryMatrix);
}

// TODO: 함수를 제대로 구현해야한다.
static double _getGolfBallCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject) {
    int width = pLabelMatrix->width;
    int height = pLabelMatrix->height;
    int maxCnt = width * height;

    return (double)pObject->cnt / maxCnt;
}

// TODO: 함수를 제대로 구현해야한다.
static double _getGolfHoleCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject) {
    int width = pLabelMatrix->width;
    int height = pLabelMatrix->height;
    int maxCnt = width * height;

    return (double)pObject->cnt / maxCnt;
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

static void _drawColorMatrixAdditive(Screen_t* pScreen, Matrix8_t* pColorMatrix) {
    if (!pScreen) return;
    if (!pColorMatrix) return;

    int width = pScreen->width;
    int height = pScreen->height;
    int length = width * height;
    PixelData_t* pScreenPixel = pScreen->elements;
    Color_t* pColorPixel = pColorMatrix->elements;

    for (int i = 0; i < length; ++i) {
        if (*pColorPixel)
            *pScreenPixel = colorToRgab5515Data(*pColorPixel);
        pScreenPixel++;
        pColorPixel++;
    }
}
