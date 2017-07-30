#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "check_center.h"
#include "line_detection.h"
#include "object_detection.h"
#include "graphic_interface.h"

#include "uart_api.h"

#define CENTER 52
#define RIGHT_ZERO_DEGREE -10
#define LEFT_ZERO_DEGREE 4
#define BIG_DIVIDE 15
#define SMALL_DIVIDE 5

Line_t* _captureLine(Screen_t* pScreen);

Screen_t* _pDefaultScreen;

bool checkCenter() {

    sleep(1);

    _pDefaultScreen = createDefaultScreen();

    bool isRight = false;
    Line_t* pLine;
    int cnt = 0;

    do {
        if(cnt > 5) {
            cnt = 0;
            //머리 상하각도를 조절한다.
            printf("cnt 5이상! 머리각도 조절\n");
        }

        if(!isRight) {
            //오른쪽으로 머리를 돌린다.
            isRight = true;
            Send_Command(0x46);
            waitMotion();
            printf("오른쪽\n");
        }
        else {   
            //왼쪽으로 머리를 돌린다.
            isRight = false;
            Send_Command(0x47);
            waitMotion();
            printf("왼쪽\n");
        }

        pLine = _captureLine(_pDefaultScreen);

        cnt++;

        _convertScreenToDisplay(_pDefaultScreen);
        displayScreen(_pDefaultScreen);
        
    } while(pLine == NULL);

   if(isRight) {
       printf("오른쪽에서 찾음\n");
    } else {
       printf("오른쪽에서 찾음\n");
    }

    int distance = CENTER - pLine->distancePoint.y;
    int isException = false;
    Line_t* pDistaceLine = NULL;

    while(abs(distance) > 5){
        if(isException) {
            isException = false;
        }else if(distance < 0) {
            //반대방향으로 움직인다.
            printf("반대방향으로 움직인다.\n");
        }else {
            //같은방향으로 움직인다.
            printf("같은방향으로 움직인다.\n");
        }    
    
        if(pDistaceLine != NULL){
            free(pDistaceLine);
        }

        pDistaceLine = _captureLine(_pDefaultScreen);//free 필요
        
        _convertScreenToDisplay(_pDefaultScreen);
        displayScreen(_pDefaultScreen);

        int cnt = 0;
        while(cnt < 5 && pDistaceLine == NULL) {
            pDistaceLine = _captureLine(_pDefaultScreen);
            cnt++;
            printf("다시찍는중\n");
            _convertScreenToDisplay(_pDefaultScreen);
            displayScreen(_pDefaultScreen);
        }

        if(pDistaceLine == NULL) {
            //반대방향으로 3걸음 움직인다.
            printf("반대방향으로 3걸음 움직인다.\n");
            isException = true;
        } else {
            distance = CENTER - pDistaceLine->distancePoint.y;
        }
    }

    if(isRight) {
        double distanceTheta = pDistaceLine->theta;
        distanceTheta += RIGHT_ZERO_DEGREE;
        if(distanceTheta < 0){
            int bigAngleSize = (int)(distanceTheta / (double)BIG_DIVIDE);
            int bigAngleSizeRemainer = (int)(distanceTheta % (double)BIG_DIVIDE);
            int smallAngleSize = (int)(bigAngleSizeRemainer / (double)SMALL_DIVIDE);

            int i;
            for(i = 0; i < bigAngleSize; ++i) {
                //빅 모션 한다.
            }

            for(i = 0; i < smallAngleSize; ++i) {
                //스몰 모션 한다.
            }

            

        }else {

        }

    }else {

    }

    if(pLine != NULL) {
        free(pLine);
    }

    destroyScreen(_pDefaultScreen);

    return false;
}

Line_t* _captureLine(Screen_t* pScreen) {
        
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