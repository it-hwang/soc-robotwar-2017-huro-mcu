#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "check_center.h"
#include "hurdle.h"

#define MIN_CNT 400
#define HURDLE_CNT 1500
#define DIFFRENCE_X 80
#define LIMIT_TRY 10

ObjectList_t* _captureBlueObject(Screen_t* pScreen, Color_t color, bool flg);

Screen_t* _pDefaultScreen;

bool hurdleMain(void) {

    _pDefaultScreen = createDefaultScreen();

    int maxBlueObjectCnt = 0;
    int gapObjectX = 0;
    int falseCounter = 0;

    while(true) {
        ObjectList_t* objList = _captureBlueObject(_pDefaultScreen, COLOR_BLUE, false);
        
        if(objList != NULL) {
            int i;
            for(i = 0; i < objList->size; ++i) {
                if(maxBlueObjectCnt < objList->list[i].cnt) {
                    maxBlueObjectCnt = objList->list[i].cnt;
                    gapObjectX = (int)(objList->list[i].maxX - objList->list[i].minX);
                }
            }
            free(objList->list);
            free(objList);
        }

        if(objList == NULL || maxBlueObjectCnt < MIN_CNT) {
            falseCounter++;
            if(falseCounter > LIMIT_TRY) {
                destroyScreen(_pDefaultScreen);
                return false;
            }
        }else if(maxBlueObjectCnt > HURDLE_CNT) {
            falseCounter = 0;
            if(gapObjectX > DIFFRENCE_X) {
                //Send_Command(); //장애물 통과 모션
                //waitMotion();
                checkCenter();
                destroyScreen(_pDefaultScreen);
                return true;
            }else {
                //Send_Command(); //앞으로 이동
                //waitMotion();
                checkCenter();
            }
        }
    }
}

ObjectList_t* _captureBlueObject(Screen_t* pScreen, Color_t color, bool flg) {
        
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
