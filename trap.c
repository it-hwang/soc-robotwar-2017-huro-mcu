//defien DEBUG

#include <stdlib.h>

#include "trap.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "white_balance.h"
#include "image_filter.h"
#include "matrix.h"
#include "object_detection.h"
#include "boundary.h"
#include "color.h"
#include "log.h"
#include "debug.h"

static Object_t* _searchTrap(void);
static Object_t* _candidateObjectForBoundary(Screen_t* pScreen);
static void _establishBoundary(Screen_t* pScreen);
static void _setBoundary(Screen_t* pScreen);
static bool _isTrapObject(Screen_t* pScreen,  Object_t* pTrapObject);
static int _measureObjectDistance(Object_t* pTrapObject);
static void _approachObject(int distance);
static bool _approachTrap(Object_t* pObject);
static bool _climbUpTrap(void);
static bool _approachBlackLine(void);
static bool _forwardRoll(void);

bool trapMain(void) {

    Object_t* pTrap = _searchTrap();

    if(pTrap == NULL)
        return false;

    _approachTrap(pTrap);
    _climbUpTrap();
    _approachBlackLine();
    _forwardRoll();
    
    free(pTrap);

    return true;
}

static Object_t* _searchTrap(void) {

    const int MAX_TRIES = 10;
    
    const int APPROACH_DISTANCE = 20;
    
    const int APPROACH_DISTANCE_ERROR = 30;

    Screen_t* pScreen = createDefaultScreen();

    for(int nTries = 0; nTries < MAX_TRIES; ++nTries) {
        
        if( !_setBoundary(pScreen) )
            continue;

        Object_t* pTrapObject = NULL;
        bool isTrap = _isTrapObject(pScreen, pTrapObject);
    
        if( isTrap ) {
            destroyScreen(pScreen);
            return pTrapObject;
        }

        int distance = _measureObjectDistance(pTrapObject);

        free(pTrapObject);

        if(distance < APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR)
            break;

        nTries = 0;

        _approachObject(distance);
    }

    destroyScreen(pScreen);
    
    return NULL;
}

static bool _setBoundary(Screen_t* pScreen) {
    Matrix16_t* pLabelMatrix = NULL;
    
    Object_t* pObject = _candidateObjectForBoundary(pScreen, pLabelMatrix);

    displayScreen(pScreen);

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
    mamcpy(pReturnObject, pMaxObject, sizeof(Object_t));

    destroyObjectList(pObjectList);
    destroyMatrix8(pYellowMatrix);

    return pReturnObject;
}

static void _establishBoundary(Screen_t* pScreen) {

    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);
    Matrix8_t* pWhiteMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);

    applyFastDilationToMatrix8(pYellowMatrix, 1);
    applyFastErosionToMatrix8(pYellowMatrix, 3);
    applyFastDilationToMatrix8(pYellowMatrix, 5);

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

    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);

    ObjectList_t* pYellowObjectList = detectObjectsLocation(pYellowMatrix);
    
    if(pYellowObjectList == NULL || pYellowObjectList->size == 0) {
        destroyObjectList(pYellowObjectList);
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

    Matrix8_t* pBlackMatrix = createColorMatrix(pScreen, pColorTables[COLOR_BLACK]);
    
    ObjectList_t* pBlackObjectList = detectObjectsLocation(pBlackMatrix);
    
    if(pBlackObjectList == NULL || pBlackObjectList->size == 0) {
        destroyObjectList(pBlackObjectList);
        destroyMatrix8(pBlackMatrix);
        return false;
    }

    Object_t* pMaxBlackObject = NULL;
    for(int i = 0; i < pBlackObjectList->size; ++i) {
        Object_t* pObject = &pBlackObjectList->list[i];

        if(pMaxBlackObject == NULL || pMaxBlackObject->cnt < pObject->cnt)
            pMaxBlackObject = pObject;
    }

    if(pMaxBlackObject == NULL) {
        destroyObjectList(pBlackObjectList);
        destroyMatrix8(pBlackMatrix);
        return false;
    }

    bool isBlackXInsideYellowX = (pMaxBlackObject->minX > pMaxYellowObject->minX && pMaxBlackObject->maxX < pMaxYellowObject->maxX);
    bool isBlackYInsideYellowY = (pMaxBlackObject->minY > pMaxYellowObject->minY && pMaxBlackObject->maxY < pMaxYellowObject->maxY);

    free(pMaxBlackObject);
    destroyObjectList(pYellowObjectList);
    destroyMatrix8(pYellowMatrix);
    destroyObjectList(pBlackObjectList);
    destroyMatrix8(pBlackMatrix);

    if(isBlackXInsideYellowX && isBlackYInsideYellowY){
        pTrapObject = (Object_t*)malloc(sizeof(Object_t));
        memcpy(pTrapObject, pMaxYellowObject, sizeof(Object_t));
        free(pMaxYellowObject);
        return true;
    }

    free(pMaxYellowObject);

    return false;
}

static int _measureObjectDistance(Object_t* pTrapObject) {
    return 0;
}

static void _approachObject(int distance) {

}
