#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "detection_corner.h"

#define FIT_DISTANCE 80
#define LIMIT_TRY 10

Screen_t* _pCornerScreen;

bool detectionCornerMain(void) {

    _pCornerScreen = createDefaultScreen();

    int distanceLine = 0;
    int falseCounter = 0;
    
    while(true) {
        Line_t* pLine = captureLine(_pCornerScreen);

        if(pLine != NULL) {
            falseCounter = 0;
            distanceLine = (int)pLine->distancePoint.y;
            free(pLine);

            if(distanceLine > FIT_DISTANCE) {
                Send_Command(MOTION_MOVE_FORWARD); 
                waitMotion();
                checkCenter();
            }else {
                Send_Command(MOTION_TURN_CORNER); //90도 회전
                waitMotion();
                Send_Command(MOTION_TURN_CORNER); //90도 회전
                waitMotion();
                checkCenter();
                destroyScreen(_pCornerScreen);
                return true;
            }
        }else {
            falseCounter++;
            if(falseCounter > LIMIT_TRY) {
                destroyScreen(_pCornerScreen);
                return false;
            }
        }
    }
}
