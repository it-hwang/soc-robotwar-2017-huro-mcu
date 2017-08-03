#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "detection_mine.h"

#define RANGE_CNT_MIN 20
#define RANGE_CNT_MAX 100
#define RANGE_DISTANCE_MIN 50
#define RANGE_DISTANCE_MAX 130
#define DISTANCE_MINE 100
#define STATUS_FRONT 0
#define STATUS_RIGHT 1
#define STATUS_LEFT 2

ObjectList_t* _captureBlackObject(Screen_t* pScreen, Color_t color, bool flg);
bool _isHurdleExist(void);

Screen_t* _pMineDefaultScreen;

bool detectionMineMain(void) {

    _pMineDefaultScreen = createDefaultScreen();

    int status = STATUS_FRONT;
    int rightY = 0;
    int leftY = 0;

    while(true) {
        
        ObjectList_t* objList = _captureBlackObject(_pMineDefaultScreen, COLOR_BLACK, false);
        
        int nearestMineY = 0;

        if(objList != NULL) {
            int i;
            for(i = 0; i < objList->size; ++i) {
                int centerX = (int)objList->list[i].centerX;
                int centerY = (int)objList->list[i].centerY;
                int cnt = (int)objList->list[i].cnt;
                if(centerY >= DISTANCE_MINE) {    
                    if(centerX >= RANGE_DISTANCE_MIN && centerX <= RANGE_DISTANCE_MAX) {
                        if(cnt >= RANGE_CNT_MIN && cnt <= RANGE_CNT_MAX) {
                            if(centerY > nearestMineY) {
                                nearestMineY = centerY;
                            }
                        }
                    }
                }
            }
            free(objList->list);
            free(objList);
        }

        if(objList == NULL || nearestMineY == 0) {
            switch(status) {
                case STATUS_FRONT :
                    Send_Command(MOTION_MOVE_FORWARD); //앞으로 이동
                    waitMotion();
                    checkAngle();
                    if(_isHurdleExist()) {
                        checkCenter();
                        return true;
                    }
                    break;
                
                case STATUS_RIGHT :
                    status = STATUS_FRONT;
                    //Send_Command(0xff);
                    //waitMotion();
                    //Send_Command(0x5c);
                    //waitMotion();
                    Send_Command(0xfe);
                    waitMotion();
                    Send_Command(0x80);
                    waitMotion();
                    Send_Command(MOTION_MOVE_RIGHT);
                    waitMotion();
                    break;

                case STATUS_LEFT :
                    status = STATUS_FRONT;
                    //Send_Command(0xff);
                    //waitMotion();
                    //Send_Command(0x5c);
                    //waitMotion();
                    Send_Command(0xfe);
                    waitMotion();
                    Send_Command(0x80);
                    waitMotion();
                    Send_Command(MOTION_MOVE_LEFT);
                    waitMotion();
                    break;
                default :
                    break;
            }
        } else {
            switch(status) {
                case STATUS_FRONT :
                    status = STATUS_RIGHT;
                    Send_Command(0xfe); //오른쪽으로 머리를 돌린다.
                    waitMotion();
                    Send_Command(0x3b); //오른쪽으로 머리를 돌린다.
                    waitMotion();
                    break;
                
                case STATUS_RIGHT :
                    status = STATUS_LEFT;
                    rightY = nearestMineY;
                    Send_Command(0xfe); //왼쪽으로 머리를 돌린다.
                    waitMotion();
                    Send_Command(0xc5); //왼쪽으로 머리를 돌린다.
                    waitMotion();
                    break;

                case STATUS_LEFT :
                    status = STATUS_FRONT;
                    leftY = nearestMineY;
                    Send_Command(0xfe);
                    waitMotion();
                    Send_Command(0x80);
                    waitMotion();
                    if(leftY > rightY) {
                        Send_Command(MOTION_MOVE_RIGHT); //오른쪽으로 이동
                        waitMotion();
                    }else {
                        Send_Command(MOTION_MOVE_LEFT); //왼쪽으로 이동
                        waitMotion();
                    }
                    leftY = 0;
                    rightY = 0;
                    break;
                default :
                    break;
            }
        }
        
    }
}

bool _isHurdleExist(void) {
    Screen_t* pScreen;

    pScreen = createDefaultScreen();

    ObjectList_t* objList = _captureBlackObject(pScreen, COLOR_BLUE, true);

    int i;
    for(i = 0; i < objList->size; ++i) {
        int diffrenceX = (int)objList->list[i].maxX - (int)objList->list[i].minX;
        int distanceY = (int)objList->list[i].maxY;
        if(diffrenceX > 70 && distanceY > 50) {
            return true;
        }
    }
    
    if(objList != NULL) {
        free(objList->list);
        free(objList);
    }

    destroyScreen(pScreen);

    return false;

}

ObjectList_t* _captureBlackObject(Screen_t* pScreen, Color_t color, bool flg) {
        
    readFpgaVideoData(pScreen);

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
        }
    }

    displayScreen(pScreen);

    return objList;
}
