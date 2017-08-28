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
static void _setBoundary(Screen_t* pScreen, Object_t* pObject);
static bool _isTrapObject(Screen_t* pScreen,  Object_t* pTrapObject);
static int _measureObjectDistance(Object_t* pTrapsObject);
static void _approachObject(int distance);

bool trapMain(void) {

    _searchTrap();

    return false;
}

static Object_t* _searchTrap(void) {

    const int MAX_TRIES = 10;
    
    const int APPROACH_DISTANCE = 20;
    
    const int APPROACH_DISTANCE_ERROR = 30;

    Screen_t* pScreen = createDefaultScreen();
    
    for(int nTries = 0; nTries < MAX_TRIES; ++nTries) {
        Object_t* pObject = _candidateObjectForBoundary(pScreen);

        displayScreen(pScreen);

        if(pObject == NULL)
            continue;

        _setBoundary(pScreen, pObject);
    
        free(pObject);

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

static Object_t* _candidateObjectForBoundary(Screen_t* pScreen) {

    readFpgaVideoDataWithWhiteBalance(pScreen);

    _establishBoundary(pScreen);

    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);

    Matrix16_t* pLabelMatrix = NULL;

    ObjectList_t* pObjectList = detectObjectsLocationWithLabeling(pYellowMatrix, pLabelMatrix);

    return NULL;
}

static void _establishBoundary(Screen_t* pScreen) {

    Matrix8_t* pYellowMatrix = createColorMatrix(pScreen, pColorTables[COLOR_YELLOW]);
    Matrix8_t* pWhiteMatrix = createColorMatrix(pScreen, pColorTables[COLOR_WHITE]);

    /* applyFastErosionToMatrix8(pYellowMatrix, 1); 
    applyFastDilationToMatrix8(pYellowMatrix, 2); */

    Matrix8_t* pMergedColorMatrix = 
    overlapColorMatrix(pYellowMatrix, pWhiteMatrix);

    /* applyFastErosionToMatrix8(pMergedColorMatrix, 1); 
    applyFastDilationToMatrix8(pMergedColorMatrix, 2);
    applyFastErosionToMatrix8(pMergedColorMatrix, 1); */
    
    Matrix8_t* pBoundaryMatrix = establishBoundary(pMergedColorMatrix);

    //applyBoundary(pScreen, pBoundaryMatrix);
    drawColorMatrix(pScreen, pMergedColorMatrix);

    destroyMatrix8(pWhiteMatrix);
    destroyMatrix8(pYellowMatrix);
    destroyMatrix8(pMergedColorMatrix);
    destroyMatrix8(pBoundaryMatrix);
}

static void _setBoundary(Screen_t* pScreen, Object_t* pObject) {

}

static bool _isTrapObject(Screen_t* pScreen,  Object_t* pTrapObject) {
    return false;
}

static int _measureObjectDistance(Object_t* pTrapObject) {
    return 0;
}

static void _approachObject(int distance) {

}