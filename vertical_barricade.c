#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "vertical_barricade.h"

ObjectList_t* _captureObject(Screen_t* pScreen, Color_t color, bool flg);

Screen_t* _pDefaultScreen;

bool verticalBarricadeMain(void) {
    
    _pDefaultScreen = createDefaultScreen();
    
    ObjectList_t* objList;

    bool isBarricade = false;
    int distanceCnt = 0;
    do{
        Send_Command(0xff);
        waitMotion();
        Send_Command(0x80);
        waitMotion();
        Send_Command(0xff);
        waitMotion();
        Send_Command(0x5c);
        waitMotion();
        
        objList = _captureObject(_pDefaultScreen, COLOR_YELLOW, false);

        distanceCnt = 0;
        if(objList != NULL){
            int i;
            for(i = 0; i < objList->size; ++i) {
                if(objList->list[i].cnt >= 400) {
                    if(distanceCnt < objList->list[i].cnt){
                        distanceCnt = objList->list[i].cnt;
                    }
                }
            }
        }
        printf("첫단계 크기 %d\n", distanceCnt);
        if(distanceCnt < 2000 && distanceCnt > 400) {
            Send_Command(0x03);
            waitMotion();
        }else if(distanceCnt >= 2000) {
            isBarricade = true;
        }

        if(objList != NULL) {
            free(objList->list);
            free(objList);
        }
    }
    while(!isBarricade); 
    
    do{
        objList = _captureObject(_pDefaultScreen, COLOR_YELLOW, false);

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

    }while(distanceCnt > 1000);

    Send_Command(0x03);
    waitMotion();
    /*readFpgaVideoData(_pDefaultScreen);
    Matrix8_t* pYellowMatrix = createColorMatrix(_pDefaultScreen, 
                                    pColorTables[COLOR_YELLOW]);

    //applyDilationToMatrix8(pBlackMatrix, 1);
    //applyErosionToMatrix8(pBlackMatrix, 2);
    //applyDilationToMatrix8(pBlackMatrix, 1);

    ObjectList_t* objList = detectObjectsLocation(pYellowMatrix);
*/
    if (objList){
        free(objList->list);
        free(objList);
    }

    //destroyMatrix8(pYellowMatrix);
    //sendDataToRobot(command);
    //printf("send command to robot: %d\n", command);
    //waitDataFromRobot();

    //_applyColorMatrix(_pDefaultScreen, pColorMatrix);
    destroyScreen(_pDefaultScreen);

    return false;
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

        int x;
        int y;
        if (objList) {
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

                //printf("%d cnt %d ( %d, %d )\n", i, object.cnt, (int)object.centerX, (int)object.centerY);

                if(object.cnt > 500) {
                    printf("%d cnt %d ( %d, %d )\n", i, object.cnt, (int)object.centerX, (int)object.centerY);
                }
            }
        }

        _convertScreenToDisplay(pScreen);
        displayScreen(pScreen);
        return objList;
}