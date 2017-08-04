#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "vertical_barricade.h"
#include "check_center.h"

#define MIN_CNT 300
#define BARRICADE_CNT 1000
#define PROGRESS_CNT 700

ObjectList_t* _captureObject(Screen_t* pScreen, Color_t color, bool flg);

Screen_t* _pVerticalBarricadeScreen;

bool verticalBarricadeMain(void) {
    
    _pVerticalBarricadeScreen = createDefaultScreen();
    
    ObjectList_t* objList;

    bool isBarricade = false;
    int distanceCnt = 0;
    int distanceY;
    int walkCnt = 0;
    
    do{
    
        Send_Command(MOTION_HEAD_DOWN);
        waitMotion();
        Send_Command(MOTION_HEAD_FRONT);
        waitMotion();
        
        objList = _captureObject(_pVerticalBarricadeScreen, COLOR_YELLOW, true);
        distanceY = 0;
        distanceCnt = 0;
        if(objList != NULL){
            int i;
            for(i = 0; i < objList->size; ++i) {
                if(objList->list[i].cnt >= MIN_CNT) {
                    if(distanceCnt < objList->list[i].cnt){
                        distanceCnt = objList->list[i].cnt;
                        distanceY = objList->list[i].minY;
                    }
                }
            }
        }
        printf("첫단계 크기 %d\n", distanceCnt);
        if(distanceCnt < BARRICADE_CNT && distanceCnt > MIN_CNT) {
            Send_Command(MOTION_MOVE_FORWARD);
            waitMotion();
            walkCnt++;
            if(walkCnt > 2) {
                checkCenter();
                walkCnt = 0;
            }
            //checkCenter();
        }else if(distanceY >= 70) {
            isBarricade = true;
        }

        if(objList != NULL) {
            free(objList->list);
            free(objList);
        }
    }
    while(!isBarricade); 
    
    do{
        //까딱까딲
        Send_Command(MOTION_HEAD_DOWN);
        waitMotion();
        Send_Command(MOTION_HEAD_FRONT);
        waitMotion();

        objList = _captureObject(_pVerticalBarricadeScreen, COLOR_YELLOW, true);

        distanceCnt = 0;
        
        if(objList != NULL){
            int i;
            for(i = 0; i < objList->size; ++i) {
                if(distanceCnt < objList->list[i].cnt){
                    distanceCnt = objList->list[i].cnt;
                }
            }
        }
        printf("두번째 크기 %d\n", distanceCnt);
        if(objList != NULL) {
            free(objList->list);
            free(objList);
        }

    }while(distanceCnt > PROGRESS_CNT);

    sleep(1);
    Send_Command(MOTION_BARRICADE_RUN_FAR);
    waitMotion();
    //checkCenter();
   

    destroyScreen(_pVerticalBarricadeScreen);

    return true;
}

ObjectList_t* _captureObject(Screen_t* pScreen, Color_t color, bool flg) {
        
    readFpgaVideoData(pScreen);     

    //_convertScreenToDisplay(pScreen);
    //displayScreen(pScreen);

    Matrix8_t* pColorMatrix = createColorMatrix(pScreen, 
                                pColorTables[color]);
    if(flg){
        applyDilationToMatrix8(pColorMatrix, 1);
        applyErosionToMatrix8(pColorMatrix, 2);
        applyDilationToMatrix8(pColorMatrix, 1);
    }
    
    ObjectList_t* objList = detectObjectsLocation(pColorMatrix);
    
    destroyMatrix8(pColorMatrix);

    if (objList != NULL) {
        int i;
        for(i = 0; i < objList->size; ++i) {
            int x;
            int y;
            Object_t object = objList->list[i];
            PixelData_t* pixels = pScreen->elements;

            for(y = object.minY; y <= object.maxY; ++y) {
                for(x = object.minX; x <= object.maxX; ++x) {
                    int index = y * pScreen->width + x;
                    uint16_t* pOutput = (uint16_t*)&pixels[index];

                    if(y == object.minY || y == object.maxY) {
                        *pOutput = 0xF800;
                    } else if(x == object.minX || x == object.maxX) {
                        *pOutput = 0xF800;
                    }
                }
            }

            int index = (int)object.centerY * pScreen->width + (int)object.centerX;
            uint16_t* pOutput = (uint16_t*)&pixels[index];
            *pOutput = 0x1F;

            if(object.cnt > 500) {
                //printf("%d cnt %d ( %d, %d )\n", i, object.cnt, (int)object.centerX, (int)object.centerY);
            }
        }
    }

    displayScreen(pScreen);
    return objList;
}
