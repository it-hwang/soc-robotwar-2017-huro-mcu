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

Screen_t* _pDefaultScreen;

bool detectionCornerMain(void) {

    _pDefaultScreen = createDefaultScreen();

    int distanceLine = 0;
    int falseCounter = 0;
    
    while(true) {
        Line_t* pLine = captureLine(_pDefaultScreen);

        if(pLine != NULL) {
            falseCounter = 0;
            distanceLine = (int)pLine->centerPoint.y;
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
            if(falseCounter > LIMIT_TRY) {
                destroyScreen(_pDefaultScreen);
                return false;
            }
        }
    }
}
