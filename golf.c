#define DEBUG

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "golf.h"
#include "vector3.h"
#include "math.h"
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

static bool _approachGolfBall(void);
static bool _aimToHole(void);
static void _shootGolfBall(void);
static void _walkToLocation(Vector3_t* pTargetLoc, double errorX, double errorY);
static bool _findObjects(Vector3_t* pBallLoc, Vector3_t* pHoleLoc, bool precise);
static bool _searchGolfBall(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation);
static bool _searchGolfHole(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation);
static void _establishBoundary(Screen_t* pScreen);
static double _getGolfBallCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject);
static double _getGolfHoleCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject);
static void _setHead(int horizontalDegrees, int verticalDegrees);
static bool _turnLeft(int degrees);
static bool _turnRight(int degrees);
static void _drawColorMatrixAdditive(Screen_t* pScreen, Matrix8_t* pColorMatrix);


bool golfMain(void) {
    solveGolf();
    return true;
}

int measureGolfDistance(void) {
    return 0;
}

bool solveGolf(void) {
    _approachGolfBall();
    _aimToHole();
    _shootGolfBall();

    return true;
}

static bool _approachGolfBall(void) {
    const int MAX_TRIES = 10;
    const double APPROACH_ERROR_X = 0.050;
    const double APPROACH_ERROR_Y = 0.050;
    const double APPROACH_MAX_DISTANCE = 0.200;

    Vector3_t ballLoc;

    for (int i = 0; i < MAX_TRIES; ++i) {
        bool hasFound = _findObjects(&ballLoc, NULL, false);
        if (!hasFound) {
            printLog("[%s] Failed.\n", __func__);
            return false;
        }
        ballLoc.z = 0.;
        
        Vector3_t targetLoc = { 0., -0.08 - _GOLF_BALL_RADIUS, 0. };
        addVector3(&targetLoc, &ballLoc);
        if (getLengthVector3(&targetLoc) > APPROACH_MAX_DISTANCE)
            setLengthVector3(&targetLoc, APPROACH_MAX_DISTANCE);

        printLog("[%s] ballLoc: {x: %.0f, y: %.0f, z: %.0f}\n", __func__, ballLoc.x * 1000, ballLoc.y * 1000, ballLoc.z * 1000);
        printLog("[%s] targetLoc: {x: %.0f, y: %.0f, z: %.0f}\n", __func__, targetLoc.x * 1000, targetLoc.y * 1000, targetLoc.z * 1000);
    
        if (fabs(targetLoc.x) <= APPROACH_ERROR_X && fabs(targetLoc.y) <= APPROACH_ERROR_Y)
            return true;

        _walkToLocation(&targetLoc, APPROACH_ERROR_X, APPROACH_ERROR_Y);
    }
    printLog("[%s] timeout.\n", __func__);

    return false;
}

static bool _aimToHole(void) {
    const int MAX_TRIES = 10;
    const double APPROACH_ERROR_X = 0.010;
    const double APPROACH_ERROR_Y = 0.010;
    const double APPROACH_ERROR_ANGLE = 5.;    // degrees
    const double APPROACH_MAX_DISTANCE = 0.050;

    Vector3_t ballLoc;
    Vector3_t holeLoc;

    for (int i = 0; i < MAX_TRIES; ++i) {
        mdelay(500);
        bool hasFound = _findObjects(&ballLoc, &holeLoc, false);
        if (!hasFound) {
            printLog("[%s] Failed.\n", __func__);
            return false;
        }
        ballLoc.z = 0.;
            
        Vector3_t ballToHole = holeLoc;
        subtractVector3(&ballToHole, &ballLoc);

        Vector3_t targetLoc = ballToHole;
        setLengthVector3(&targetLoc, -0.040 - _GOLF_BALL_RADIUS);
        addVector3(&targetLoc, &ballLoc);
        if (getLengthVector3(&targetLoc) > APPROACH_MAX_DISTANCE)
            setLengthVector3(&targetLoc, APPROACH_MAX_DISTANCE);

        double facing = atan2(ballToHole.y, ballToHole.x) * RAD_TO_DEG - 90.;
        if (facing < -180.) facing += 360.;

        printLog("[%s] ballLoc: {x: %.0f, y: %.0f, z: %.0f}\n", "golf", ballLoc.x * 1000, ballLoc.y * 1000, ballLoc.z * 1000);
        printLog("[%s] holeLoc: {x: %.0f, y: %.0f, z: %.0f}\n", "golf", holeLoc.x * 1000, holeLoc.y * 1000, holeLoc.z * 1000);
        printLog("[%s] targetLoc: {x: %.0f, y: %.0f, z: %.0f}\n", __func__, targetLoc.x * 1000, targetLoc.y * 1000, targetLoc.z * 1000);
        printLog("[%s] facing: %.0f\n", __func__, facing);
    
        if (fabs(targetLoc.x) <= APPROACH_ERROR_X && fabs(targetLoc.y) <= APPROACH_ERROR_Y && fabs(facing) <= APPROACH_ERROR_ANGLE)
            return true;

        _walkToLocation(&targetLoc, APPROACH_ERROR_X, APPROACH_ERROR_Y);

        if (fabs(facing) > APPROACH_ERROR_ANGLE) {
            if (facing > 0) _turnLeft(fabs(facing));
            else _turnRight(fabs(facing));
        }
    }
    printLog("[%s] timeout.\n", __func__);


    return false;
}

static void _shootGolfBall(void) {
    runMotion(MOTION_KICK);
}

static void _walkToLocation(Vector3_t* pTargetLoc, double errorX, double errorY) {
    int dx = pTargetLoc->x * 1000;
    int dy = pTargetLoc->y * 1000;

    if (dy < 0 - errorY) walkBackward(abs(dy));

    if (dx < 0 - errorX) walkLeft(abs(dx));
    else if (dx > 0 + errorX) walkRight(abs(dx));

    if (dy > 0 + errorY) walkForward(abs(dy));
}

static bool _findObjects(Vector3_t* pBallLoc, Vector3_t* pHoleLoc, bool precise) {    
    const int HEAD_HORIZONTAL_DEGREES[] = {   0,   0, -40,  40 };
    const int HEAD_VERTICAL_DEGREES[]   = { -65, -40, -40, -40 };
    const Vector3_t HEAD_OFFSET = { -0.040, -0.020, 0.295 };

    setServoSpeed(10);
    runMotion(MOTION_BASIC_STANCE);
    resetServoSpeed();

    bool isBallFound = (!pBallLoc);
    bool isHoleFound = (!pHoleLoc);
    double ballCorrelation = 0.;
    double holeCorrelation = 0.;

    int l = sizeof(HEAD_HORIZONTAL_DEGREES) / sizeof(HEAD_HORIZONTAL_DEGREES[0]);
    for (int i = 0; i < l; ++i) {
        if (!precise && isBallFound && isHoleFound) break;

        _setHead(HEAD_HORIZONTAL_DEGREES[i], HEAD_VERTICAL_DEGREES[i]);

        Screen_t* pScreen = createDefaultScreen();
        readFpgaVideoDataWithWhiteBalance(pScreen);
        _establishBoundary(pScreen);

        if (precise || !isBallFound) {
            Object_t object;
            double correlation;
            bool hasFound = _searchGolfBall(pScreen, &object, &correlation);
            if (hasFound && correlation > ballCorrelation) {
                CameraParameters_t camParams;
                readCameraParameters(&camParams, &HEAD_OFFSET);

                PixelLocation_t screenLoc = { (int)object.centerX, (int)object.centerY };
                convertScreenLocationToWorldLocation(&camParams, &screenLoc, _GOLF_BALL_RADIUS, pBallLoc);
                ballCorrelation = correlation;
                isBallFound = true;
            }
        }
        if (precise || !isHoleFound) {
            Object_t object;
            double correlation;
            bool hasFound = _searchGolfHole(pScreen, &object, &correlation);
            if (hasFound && correlation > holeCorrelation) {
                bool isOnBoundary = (object.minX <= 1 || object.maxX >= pScreen->width - 2 || object.minY <= 1);
                if (!isOnBoundary) {
                    CameraParameters_t camParams;
                    readCameraParameters(&camParams, &HEAD_OFFSET);

                    PixelLocation_t screenLoc = { (int)object.centerX, (int)object.centerY };
                    convertScreenLocationToWorldLocation(&camParams, &screenLoc, 0., pHoleLoc);
                    holeCorrelation = correlation;
                    isHoleFound = true;
                }
            }
        }

        destroyScreen(pScreen);
    }
    
    setServoSpeed(30);
    setHead(0, 0);
    mdelay(500);
    resetServoSpeed();
    
    return (isBallFound && isHoleFound);
}

static bool _searchGolfBall(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation) {
    const double MIN_CORRELATION = 0.00;

    if (!pInputScreen) return false;

    Screen_t* pScreen = cloneMatrix16(pInputScreen);

    Matrix8_t* pColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_ORANGE]);
    applyFastDilationToMatrix8(pColorMatrix, 2);
    applyFastErosionToMatrix8(pColorMatrix, 2);

    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);
    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    Object_t* pMostSimilarObject = NULL;
    double maxCorrelation = 0.;
    if (pObjectList) {
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

    if (pMostSimilarObject && pReturnedObject)
        memcpy(pReturnedObject, pMostSimilarObject, sizeof(Object_t));
    if (pMostSimilarObject && pCorrelation)
        *pCorrelation = maxCorrelation;
    
    // Display debug screen
    Matrix8_t* pWhiteMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);
    drawColorMatrix(pScreen, pWhiteMatrix);
    _drawColorMatrixAdditive(pScreen, pColorMatrix);
    drawObjectEdge(pScreen, pMostSimilarObject, NULL);
    displayScreen(pScreen);
    destroyMatrix8(pWhiteMatrix);

    destroyObjectList(pObjectList);
    destroyMatrix16(pLabelMatrix);
    destroyMatrix8(pColorMatrix);
    destroyScreen(pScreen);
    return hasFound;
}

static bool _searchGolfHole(Screen_t* pInputScreen, Object_t* pReturnedObject, double* pCorrelation) {
    const double MIN_CORRELATION = 0.00;

    if (!pInputScreen) return false;

    Screen_t* pScreen = cloneMatrix16(pInputScreen);

    Matrix8_t* pColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);
    applyFastDilationToMatrix8(pColorMatrix, 1);

    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);
    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pColorMatrix, pLabelMatrix);

    Object_t* pMostSimilarObject = NULL;
    double maxCorrelation = 0.;
    if (pObjectList) {
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

    if (pMostSimilarObject && pReturnedObject)
        memcpy(pReturnedObject, pMostSimilarObject, sizeof(Object_t));
    if (pMostSimilarObject && pCorrelation)
        *pCorrelation = maxCorrelation;
    
    // Display debug screen
    Matrix8_t* pWhiteMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);
    drawColorMatrix(pScreen, pWhiteMatrix);
    _drawColorMatrixAdditive(pScreen, pColorMatrix);
    drawObjectEdge(pScreen, pMostSimilarObject, NULL);
    displayScreen(pScreen);
    destroyMatrix8(pWhiteMatrix);

    destroyObjectList(pObjectList);
    destroyMatrix16(pLabelMatrix);
    destroyMatrix8(pColorMatrix);
    destroyScreen(pScreen);
    return hasFound;
}

static void _establishBoundary(Screen_t* pScreen) {
    int width = pScreen->width;
    int height = pScreen->height;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < 3; ++j) {
            int index = i * width + j;
            pScreen->elements[index] = 0x7bcf; //NONE_COLOR
        }
    }

    Matrix8_t* pWhiteColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);
    Matrix8_t* pOrangeColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_ORANGE]);
    Matrix8_t* pBlueColorMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLUE]);
    Matrix8_t* pMergedColorMatrix = overlapColorMatrix(pOrangeColorMatrix, pWhiteColorMatrix);
    Matrix8_t* pMergedColorMatrix2 = overlapColorMatrix(pBlueColorMatrix, pMergedColorMatrix);
    destroyMatrix8(pWhiteColorMatrix);
    destroyMatrix8(pOrangeColorMatrix);
    destroyMatrix8(pBlueColorMatrix);
    destroyMatrix8(pMergedColorMatrix);

    applyFastDilationToMatrix8(pMergedColorMatrix2, 1);
    applyFastErosionToMatrix8(pMergedColorMatrix2, 2);

    Matrix8_t* pBoundaryMatrix = establishBoundary(pMergedColorMatrix2);
    
    applyBoundary(pScreen, pBoundaryMatrix);

    destroyMatrix8(pBoundaryMatrix);
    destroyMatrix8(pMergedColorMatrix2);
}

// TODO: 함수를 제대로 구현해야한다.
static double _getGolfBallCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject) {
    const int MIN_COUNT = 20;

    if (pObject->cnt < MIN_COUNT) return 0.;

    int width = pLabelMatrix->width;
    int height = pLabelMatrix->height;
    int maxCnt = width * height;

    return (double)pObject->cnt / maxCnt;
}

// TODO: 함수를 제대로 구현해야한다.
static double _getGolfHoleCorrelation(Matrix16_t* pLabelMatrix, Object_t* pObject) {
    const double MAX_DENSITY    = 0.50;
    const int    MIN_COUNT      = 20;

    if (pObject->cnt < MIN_COUNT) return 0.;
    
    int objectWidth = pObject->maxX - pObject->minX + 1;
    int objectHeight = pObject->maxY - pObject->minY + 1;
    double density = (double)pObject->cnt / (objectWidth * objectHeight);
    if (density > MAX_DENSITY) return 0.;

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
    mdelay(200);
}

static bool _turnLeft(int degrees) {
    if (degrees < 5) {
        degrees = 5;
    }

    float remainingDegrees = degrees;

    int nSteps;
    nSteps = remainingDegrees / 4.1;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_LEFT_4DEG);
        remainingDegrees -= 4.1;
    }

    return true;
}

static bool _turnRight(int degrees) {
    if (degrees < 5) {
        degrees = 5;
    }

    float remainingDegrees = degrees;

    int nSteps;
    nSteps = remainingDegrees / 4.1;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_RIGHT_4DEG);
        remainingDegrees -= 4.1;
    }
    
    return true;
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
