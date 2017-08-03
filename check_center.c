#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "check_center.h"
#include "object_detection.h"
#include "graphic_interface.h"
#include "robot_protocol.h"
#include "image_filter.h"

#define CENTER 80
#define RIGHT_ZERO_DEGREE 2
#define LEFT_ZERO_DEGREE -5
#define BIG_DIVIDE 15
#define SMALL_DIVIDE 5
#define ANGLE_RANGE 6

Screen_t* _pDefaultScreen;

bool checkCenter() {

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

    ///함수 시작
    int distance = CENTER - pLine->distancePoint.y;
    int isException = false;
    Line_t* pDistaceLine = NULL;
    cnt = 0;

    while(abs(distance) > 5){
        if(isException) {
            isException = false;
        }else if(distance < 0) {
            //반대방향으로 움직인다.
            //printf("반대방향으로 움직인다.\n");
            if(isRight) {
                Send_Command(MOTION_MOVE_LEFT);
                waitMotion();
            }else {
                Send_Command(MOTION_MOVE_RIGHT);
                waitMotion();
            }
        }else {
            //같은방향으로 움직인다.
            //printf("같은방향으로 움직인다.\n");
            if(isRight) {
                Send_Command(MOTION_MOVE_RIGHT);
                waitMotion();
            }else {
                Send_Command(MOTION_MOVE_LEFT);
                waitMotion();
            }
        }    
    
        if(pDistaceLine != NULL){
            free(pDistaceLine);
        }

        pDistaceLine = captureLine(_pDefaultScreen);//free 필요
        //printf("찍었다.\n");
        
        displayScreen(_pDefaultScreen);
        cnt = 0;
        while(cnt < 5 && pDistaceLine == NULL) {
            pDistaceLine = captureLine(_pDefaultScreen);
            cnt++;
            //printf("다시찍는중\n");
           
            displayScreen(_pDefaultScreen);
        }

        if(pDistaceLine == NULL) {
            //반대방향으로 3걸음 움직인다.
            //printf("반대방향으로 3걸음 움직인다.\n");
            if(isRight) {
                Send_Command(MOTION_MOVE_LEFT);
                waitMotion();
                Send_Command(MOTION_MOVE_LEFT);
                waitMotion();
                Send_Command(MOTION_MOVE_LEFT);
                waitMotion();
            }else {
                Send_Command(MOTION_MOVE_RIGHT);
                waitMotion();
                Send_Command(MOTION_MOVE_RIGHT);
                waitMotion();
                Send_Command(MOTION_MOVE_RIGHT);
                waitMotion();
            }
            isException = true;
        } else {
            distance = CENTER - pDistaceLine->distancePoint.y;
        }
    }

    if(distance <= 5) {
        pDistaceLine = captureLine(_pDefaultScreen);
        
        displayScreen(_pDefaultScreen);

        cnt = 0;
        while(cnt < 5 && pDistaceLine == NULL) {
            pDistaceLine = captureLine(_pDefaultScreen);
            cnt++;
            
            displayScreen(_pDefaultScreen);
            //printf("거리 다시찍는중..\n");
        }
    }

    double distanceTheta = pDistaceLine->theta;
    //printf("distanceTheta %f\n", distanceTheta);
    
    if(pDistaceLine != NULL)
        free(pDistaceLine);
    
    bool isGood;

    int zeroDegree;
    if(isRight) {
        zeroDegree = RIGHT_ZERO_DEGREE;
    } else {
        zeroDegree = LEFT_ZERO_DEGREE;
    }

    
    if((distanceTheta >= zeroDegree - ANGLE_RANGE) &&
        (distanceTheta <= zeroDegree + ANGLE_RANGE)) {
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
            bigMotion = MOTION_TURN_LEFT_BIG;
            smallMotion = MOTION_TURN_LEFT;
        } else {
            bigMotion = MOTION_TURN_RIGHT_BIG;
            smallMotion = MOTION_TURN_RIGHT;
        }
        printf("theta %f\n", distanceTheta);
        printf("bigAngleSize %d\n", bigAngleSize);
        printf("bigAngleSizeReminder %d\n", bigAngleSize);
        printf("smallAngleSize %d\n", smallAngleSize);
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

            
            if((checkPosition->theta >= zeroDegree - ANGLE_RANGE) &&
                (checkPosition->theta <= zeroDegree + ANGLE_RANGE)) {
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

    Send_Command(0xfe);
    waitMotion();
    Send_Command(0x80);
    waitMotion();
    
    return true;
}

Line_t* captureLine(Screen_t* pScreen) {
        
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

    
    if((distanceTheta >= zeroDegree - ANGLE_RANGE) &&
        (distanceTheta <= zeroDegree + ANGLE_RANGE)) {
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
            bigMotion = MOTION_TURN_LEFT_BIG;
            smallMotion = MOTION_TURN_LEFT;
        } else {
            bigMotion = MOTION_TURN_RIGHT_BIG;
            smallMotion = MOTION_TURN_RIGHT;
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

            
            if((checkPosition->theta >= zeroDegree - ANGLE_RANGE) &&
                (checkPosition->theta <= zeroDegree + ANGLE_RANGE)) {
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

    Send_Command(0xfe);
    waitMotion();
    Send_Command(0x80);
    waitMotion();
    
    return true;
}
