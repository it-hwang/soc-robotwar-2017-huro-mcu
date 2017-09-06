#define DEBUG

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "trap.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "white_balance.h"
#include "image_filter.h"
#include "matrix.h"
#include "object_detection.h"
#include "boundary.h"
#include "color.h"
#include "camera.h"
#include "log.h"
#include "debug.h"

static bool _searchTrap(void);
static Object_t* _candidateObjectForBoundary(Screen_t* pScreen, Matrix16_t* pLabelMatrix);
static Object_t* _getCandidateObjectForBoundary(Screen_t* pScreen, Matrix16_t* pLabelMatrix);
static void _establishBoundary(Screen_t* pScreen);
static bool _setBoundary(Screen_t* pScreen);
static bool _isTrapObject(Screen_t* pScreen,  Object_t* pTrapObject);
static Matrix8_t* _createYellowMatrix(Screen_t* pScreen);
static Matrix8_t* _createBlackMatrix(Screen_t* pScreen);
static double _measureObjectDistance(Object_t* pTrapObject, Vector3_t* pWorldLoc, double height);
static void _approachObject(Vector3_t* pVector, int approachDistance);
static bool _approachTrap(Object_t* pObject);
static bool _climbUpTrap(void);
static bool _approachBlackLine(void);
static bool _forwardRoll(void);

bool trapMain(void) {

    if( !_searchTrap() )
        return false;

    //_climbUpTrap();
    //_approachBlackLine();
    //_forwardRoll();

    return true;
}

static bool _searchTrap(void) {

    const int MAX_TRIES = 10;
    
    const int APPROACH_DISTANCE = 80;
    
    const int APPROACH_DISTANCE_ERROR = 30;

    const double OBJECT_HEIGHT = 0.2;

    Screen_t* pScreen = createDefaultScreen();

    for(int nTries = 0; nTries < MAX_TRIES; ++nTries) {
        
        if( !_setBoundary(pScreen) )
            continue;
            
        Object_t* pTrapObject = (Object_t*)malloc(sizeof(Object_t));
        bool isTrap = _isTrapObject(pScreen, pTrapObject);
        
        drawObjectEdge(pScreen, pTrapObject, NULL);
        displayScreen(pScreen);

        if( pTrapObject == NULL){
            printDebug("노란 물체가 없습니다.\n");
            continue;
        }
            
        if( isTrap ) {
            destroyScreen(pScreen);
            printDebug("함정을 찾았습니다.\n");
            nTries = 0;
            if( _approachTrap(pTrapObject) ) {
                free(pTrapObject);
                return true;
            }
            free(pTrapObject);
            continue;
        }

        printDebug("노란 물체를 찾았지만, 함정인지는...\n");
        Vector3_t trapVector;
        double distance =  _measureObjectDistance(&pTrapObject, &trapVector, OBJECT_HEIGHT);

        free(pTrapObject);
        printDebug("노란 물체의 거리 %f\n", distance);
        if(distance < APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR){
            printDebug("노란 물체에 더 이상 다가갈 수 없습니다. %f\n", distance);
            break;
        }
            
        nTries = 0;

        _approachObject(&trapVector, APPROACH_DISTANCE);
    }

    destroyScreen(pScreen);
    
    return false;
}

static bool _setBoundary(Screen_t* pScreen) {
    Matrix16_t* pLabelMatrix = createMatrix16(pScreen->width, pScreen->height);
    memset(pLabelMatrix->elements, 0, (pLabelMatrix->width * pLabelMatrix->height) * sizeof(uint16_t));
    
    Object_t* pObject = _candidateObjectForBoundary(pScreen, pLabelMatrix);
    
    if(pObject == NULL){
        destroyMatrix16(pLabelMatrix);
        return false;
    }
    
    Matrix8_t* pMatrix8 = traceBoundaryLine(pObject, pLabelMatrix);
    
    fillBoundary(pMatrix8);
    applyBoundary(pScreen, pMatrix8);
    
    destroyMatrix8(pMatrix8);
    destroyMatrix16(pLabelMatrix);
    free(pObject);

    return true;
}

static Object_t* _candidateObjectForBoundary(Screen_t* pScreen, Matrix16_t* pLabelMatrix) {

    readFpgaVideoDataWithWhiteBalance(pScreen);

    _establishBoundary(pScreen);
    // displayScreen(pScreen);
    Object_t* pObject = _getCandidateObjectForBoundary(pScreen, pLabelMatrix);
    
    return pObject;
}

static Object_t* _getCandidateObjectForBoundary(Screen_t* pScreen, Matrix16_t* pLabelMatrix) {
    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);

    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pYellowMatrix, pLabelMatrix);

    if(pObjectList == NULL || pObjectList->size == 0) {
        destroyObjectList(pObjectList);
        destroyMatrix8(pYellowMatrix);
        return NULL;
    }

    Object_t* pMaxObject = NULL;
    for(int i = 0; i < pObjectList->size; ++i) {
        Object_t* pObject = &pObjectList->list[i];

        if(pMaxObject == NULL || pMaxObject->cnt < pObject->cnt)
            pMaxObject = pObject;
    }

    if(pMaxObject == NULL) {
        destroyObjectList(pObjectList);
        destroyMatrix8(pYellowMatrix);
        return NULL;
    }

    Object_t* pReturnObject = (Object_t*)malloc(sizeof(Object_t));
    memcpy(pReturnObject, pMaxObject, sizeof(Object_t));

    destroyObjectList(pObjectList);
    destroyMatrix8(pYellowMatrix);

    return pReturnObject;
}

static void _establishBoundary(Screen_t* pScreen) {

    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);
    Matrix8_t* pWhiteMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);

    applyFastErosionToMatrix8(pYellowMatrix, 1);
    applyFastDilationToMatrix8(pYellowMatrix, 2);
    applyFastErosionToMatrix8(pYellowMatrix, 1);

    applyFastHeightDilationToMatrix8(pYellowMatrix, 2);

    Matrix8_t* pMergedColorMatrix = 
    overlapColorMatrix(pYellowMatrix, pWhiteMatrix);

    applyFastErosionToMatrix8(pMergedColorMatrix, 1); 
    applyFastDilationToMatrix8(pMergedColorMatrix, 2);
    applyFastErosionToMatrix8(pMergedColorMatrix, 1);
    
    Matrix8_t* pBoundaryMatrix = establishBoundary(pMergedColorMatrix);

    applyBoundary(pScreen, pBoundaryMatrix);
    //drawColorMatrix(pScreen, pMergedColorMatrix);

    destroyMatrix8(pWhiteMatrix);
    destroyMatrix8(pYellowMatrix);
    destroyMatrix8(pMergedColorMatrix);
    destroyMatrix8(pBoundaryMatrix);
}

static bool _isTrapObject(Screen_t* pScreen,  Object_t* pTrapObject) {
    Matrix8_t* pYellowMatrix = _createYellowMatrix(pScreen);

    ObjectList_t* pYellowObjectList = detectObjectsLocation(pYellowMatrix);
    
    // drawColorMatrix(pScreen, pYellowMatrix);
    // displayScreen(pScreen);

    if(pYellowObjectList == NULL) {
        destroyMatrix8(pYellowMatrix);
        return false;
    }
    
    Object_t* pMaxYellowObject = NULL;
    for(int i = 0; i < pYellowObjectList->size; ++i) {
        Object_t* pObject = &pYellowObjectList->list[i];

        if(pMaxYellowObject == NULL || pMaxYellowObject->cnt < pObject->cnt)
            pMaxYellowObject = pObject;
    }

    if(pMaxYellowObject == NULL) {
        destroyObjectList(pYellowObjectList);
        destroyMatrix8(pYellowMatrix);
        return false;
    }
    
    Matrix8_t* pBlackMatrix = _createBlackMatrix(pScreen);

    // drawColorMatrix(pScreen, pBlackMatrix);
    // displayScreen(pScreen);
    
    ObjectList_t* pBlackObjectList = detectObjectsLocation(pBlackMatrix);
    
    Object_t* pMaxBlackObject = NULL;
    for(int i = 0; i < pBlackObjectList->size; ++i) {
        Object_t* pObject = &pBlackObjectList->list[i];

        if(pMaxBlackObject == NULL || pMaxBlackObject->cnt < pObject->cnt)
            pMaxBlackObject = pObject;
    }
 
    memcpy(pTrapObject, pMaxYellowObject, sizeof(Object_t));

    bool isBlackXInsideYellowX = false;
    bool isBlackYInsideYellowY = false;

    if(pMaxBlackObject != NULL) {
        isBlackXInsideYellowX = (pMaxBlackObject->minX > pMaxYellowObject->minX && pMaxBlackObject->maxX < pMaxYellowObject->maxX);
        isBlackYInsideYellowY = (pMaxBlackObject->minY > pMaxYellowObject->minY && pMaxBlackObject->maxY < pMaxYellowObject->maxY);
    }
    
    destroyObjectList(pYellowObjectList);
    destroyMatrix8(pYellowMatrix);
    destroyObjectList(pBlackObjectList);
    destroyMatrix8(pBlackMatrix);

    if(isBlackXInsideYellowX && isBlackYInsideYellowY)
        return true;

    return false;
}

static Matrix8_t* _createYellowMatrix(Screen_t* pScreen) {
    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);
    
    applyFastErosionToMatrix8(pYellowMatrix, 1);
    applyFastDilationToMatrix8(pYellowMatrix, 2);
    applyFastErosionToMatrix8(pYellowMatrix, 1);

    return pYellowMatrix;
}

static Matrix8_t* _createBlackMatrix(Screen_t* pScreen) {
    Matrix8_t* pBlackMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLACK]);
    
    applyFastDilationToMatrix8(pBlackMatrix, 1);
    applyFastErosionToMatrix8(pBlackMatrix, 2);
    applyFastDilationToMatrix8(pBlackMatrix, 1);

    return pBlackMatrix;
}

static double _measureObjectDistance(Object_t* pTrapObject, Vector3_t* pWorldLoc, double height) {
    const Vector3_t HEAD_OFFSET = { 0.000, -0.020, 0.295 };

    CameraParameters_t camParameter;
    readCameraParameters(&camParameter, &HEAD_OFFSET);

    PixelLocation_t trapLoc = {pTrapObject->centerX, pTrapObject->maxY};
    convertScreenLocationToWorldLocation(&camParameter, &trapLoc, height, pWorldLoc);

    double distance = sqrt(dotProductVector3(pWorldLoc, pWorldLoc));

    return distance * 1000;
}

static void _approachObject(Vector3_t* pVector, int approachDistance) {
    // const double X_ERROR = 
    
    if(pVector->x < 0)
        walkLeft(pVector->x * -1000);
    else
        walkRight(pVector->x * 1000);

    printDebug("y값 %f\n", pVector->y);
    walkForward(pVector->y * 1000 - approachDistance);
}

static bool _approachTrap(Object_t* pTrap) {
    const int MAX_TRIES = 10;
    
    const int APPROACH_DISTANCE = 20;
    
    const int APPROACH_DISTANCE_ERROR = 30;

    const double TRAP_HEIGHT = 0.0;
    
    // for(int nTries = 0; nTries < MAX_TRIES; ++nTries) {
        Vector3_t trapVector;
        double distance =  _measureObjectDistance(pTrap, &trapVector, TRAP_HEIGHT);
        printDebug("함정과의 거리 %f\n", distance);

        if(distance < APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR){
            printDebug("함정과 가깝습니다.\n");
            return true;
        }

        _approachObject(&trapVector, APPROACH_DISTANCE);
    // }
    return false;
}

static bool _climbUpTrap(void) {
    return runMotion(MOTION_CLIMB_UP_STAIR);
}

static bool _approachBlackLine(void) {
    
    return false;
}

static bool _forwardRoll(void) {
    return false;
}
