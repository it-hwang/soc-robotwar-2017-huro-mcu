#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "detection_corner.h"

#define CAPTURE_ERROR -1
#define RIGHT_SIDE_CLEAR 0
#define LEFT_SIDE_CLEAR 1
#define NO_CLEAR_SIDE 2

#define FIT_DISTANCE 80
#define LIMIT_TRY 10

Screen_t* _pDefaultScreen;

bool cornerDetection(void) {
    static const char* LOG_FUNCTION_NAME = "cornerDetection()";

    int turnWhere = _lookAround();

    if(turnWhere < 0) {
        printLog("[%s] 라인을 찾을 수 없습니다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if(turnWhere == NO_CLEAR_SIDE) {
        printLog("[%s] 이건 코너가 아닙니다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if( !_moveUntilSeeLine() ) {
        printLog("[%s] 선에 접근할 수 없습니다.\n", LOG_FUNCTION_NAME);
        return false;
    } 
    
    checkCenterMain();

    _moveToDestination(turnWhere);

    return true;
}

bool cornerDetection(void) {

    _pDefaultScreen = createDefaultScreen();

    int distanceLine = 0;
    int falseCounter = 0;
    
    while(true) {
        Line_t* pLine = captureLine(_pDefaultScreen);

        if(pLine != NULL) {
            falseCounter = 0;
            distanceLine = (int)pLine->distancePoint.y;
            free(pLine);

            if(distanceLine > FIT_DISTANCE) {
                //Send_Command(); //앞으로 간다.
                //waitMotion();
                checkCenter();
            }else {
                //Send_Command(); //90도 회전
                //waitMotion();
                checkCenter();
                destroyScreen(_pDefaultScreen);
                return true;
            }
        }else {
            falseCounter++;
            if(falsesCounter > LIMIT_TRY) {
                destroyScreen(_pDefaultScreen);
                return false;
            }
        }
    }
}
