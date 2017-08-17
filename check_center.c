#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "check_center.h"
#include "object_detection.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "log.h"

#define CENTER 80
#define RIGHT_ZERO_DEGREE 2
#define LEFT_ZERO_DEGREE -5
#define BIG_DIVIDE 15
#define SMALL_DIVIDE 5

bool checkCenterMain(void) {
    static const char* LOG_FUNCTION_NAME = "checkCenterMain()";

    int headDirection = _searchLine();

    if( headDirection < 0 ) {
        printLog("[%s] 라인을 찾을 수 없다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if( !_approachLine(headDirection) ) {
        printLog("[%s] 선에 접근 할 수 없다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    if( !_arrangeAngle(headDirection) ) {
        printLog("[%s] 각도를 정렬에 실패했다.\n", LOG_FUNCTION_NAME);
        return false;
    }

    return true;
}

static Line_t* _captureLine(Screen_t* pScreen) {
        
        readFpgaVideoData(pScreen);     

        Matrix8_t* pColorMatrix = createColorMatrix(pScreen, 
                                    pColorTables[COLOR_BLACK]);

        applyDilationToMatrix8(pColorMatrix, 1);
        applyErosionToMatrix8(pColorMatrix, 2);
        applyDilationToMatrix8(pColorMatrix, 1);
        
        Line_t* returnLine = lineDetection(pColorMatrix);
        
        destroyMatrix8(pColorMatrix);

        return returnLine;
}

bool checkAngle(void) {

    _pDefaultScreen = createDefaultScreen();

    bool isRight = false;
    Line_t* pLine;
    int cnt = 0;

    do {
        if(cnt > 5) {
            cnt = 0;
            //머리 상하각도를 조절한다.
            //printf("cnt 5이상! 머리각도 조절\n");
        }

        if(!isRight) {
            //오른쪽으로 머리를 돌린다.
            isRight = true;
            //printf("오른쪽\n");
            Send_Command(0xfe);
            waitMotion();
            Send_Command(0x3a);
            waitMotion();
        }else {   
            //왼쪽으로 머리를 돌린다.
            isRight = false;
            //printf("왼쪽\n");
            Send_Command(0xfe);
            waitMotion();
            Send_Command(0xc5);
            waitMotion();
        }

        pLine = captureLine(_pDefaultScreen);

        cnt++;

        displayScreen(_pDefaultScreen);
        
    } while(pLine == NULL);


    double distanceTheta = pLine->theta;
    //printf("distanceTheta %f\n", distanceTheta);
    
    bool isGood;

    int zeroDegree;
    if(isRight) {
        zeroDegree = RIGHT_ZERO_DEGREE;
    } else {
        zeroDegree = LEFT_ZERO_DEGREE;
    }

    
    if((distanceTheta >= zeroDegree - 4) &&
        (distanceTheta <= zeroDegree + 4)) {
        isGood = true;
    } else {
        isGood = false;
    }

    while(!isGood) {
        if(isRight) {
            distanceTheta -= RIGHT_ZERO_DEGREE;
        } else {
            distanceTheta -= LEFT_ZERO_DEGREE;
        }
        
        int bigAngleSize = (abs((int)distanceTheta) / BIG_DIVIDE);
        int bigAngleSizeRemainer = (abs((int)distanceTheta) % BIG_DIVIDE);
        int smallAngleSize = (abs((int)bigAngleSizeRemainer) / SMALL_DIVIDE);

        unsigned char bigMotion;
        unsigned char smallMotion;

        if(distanceTheta < 0) {
            bigMotion = 0x10;
            smallMotion = 0x13;
        } else {
            bigMotion = 0x0f;
            smallMotion = 0x12;
        }
        /*printf("theta %f\n", distanceTheta);
        printf("bigAngleSize %d\n", bigAngleSize);
        printf("bigAngleSizeReminder %d\n", bigAngleSize);
        printf("smallAngleSize %d\n", smallAngleSize);
        */
        int i;
        for(i = 0; i < bigAngleSize; ++i) {
            //빅 모션 한다.
            //printf("빅모션\n");
            Send_Command(bigMotion);
            waitMotion();
        }

        for(i = 0; i < smallAngleSize; ++i) {
            //스몰 모션 한다.
            //printf("스몰모션\n");
            Send_Command(smallMotion);
            waitMotion();
        }

        Line_t* checkPosition = captureLine(_pDefaultScreen);
        
        
        displayScreen(_pDefaultScreen);

        cnt = 0;
        while(checkPosition == NULL && cnt < 3) {
            checkPosition = captureLine(_pDefaultScreen);

            
            displayScreen(_pDefaultScreen);
        }

        if(checkPosition != NULL) {
            if(isRight) {
                zeroDegree = RIGHT_ZERO_DEGREE;
            } else {
                zeroDegree = LEFT_ZERO_DEGREE;
            }

            
            if((checkPosition->theta >= zeroDegree - 4) &&
                (checkPosition->theta <= zeroDegree + 4)) {
                isGood = true;
            } else {
                distanceTheta = checkPosition->theta;
            }

            free(checkPosition);
        } else {
            isGood = true;
        }
        
    }

    //////함수 끝


    if(pLine != NULL) {
        free(pLine);
    }

    destroyScreen(_pDefaultScreen);

    return true;
}
