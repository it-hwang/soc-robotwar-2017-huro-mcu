#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "vertical_barricade.h"

Screen_t* _pDefaultScreen;

bool verticalBarricadeMain(void) {
    
    readFpgaVideoData(_pDefaultScreen);
    Matrix8_t* pYellowMatrix = createColorMatrix(_pDefaultScreen, 
                                    pColorTables[COLOR_YELLOW]);

    /*Matrix8_t* pBlackMatrix = createColorMatrix(_pDefaultScreen,
                                    pColorTables[COLOR_BLACK]);*/

    //applyDilationToMatrix8(pBlackMatrix, 1);
    //applyErosionToMatrix8(pBlackMatrix, 2);
    //applyDilationToMatrix8(pBlackMatrix, 1);

    ObjectList_t* yellowObjectList = detectObjectsLocation(pYellowMatrix);
    //ObjectList_t* blackObjectList = detectObjectsLocation(pBlackMatrix);

    int x;
    int y;
    if (yellowObjectList) {
        int i;
        for(i = 0; i < yellowObjectList->size; ++i) {
            int x;
            int y;
            Object_t object = yellowObjectList->list[i];
            PixelData_t* pixels = _pDefaultScreen->elements;

            for(y = object.minY; y <= object.maxY; ++y) {
                for(x = object.minX; x <= object.maxX; ++x) {
                    int index = y * _pDefaultScreen->width + x;
                    uint16_t* pOutput = (uint16_t*)&pixels[index];

                    if(y == object.minY || y == object.maxY) {
                        *pOutput = 0xF800;
                    } else if(x == object.minX || x == object.maxX) {
                        *pOutput = 0xF800;
                    }
                }
            }

            int index = (int)object.centerY * _pDefaultScreen->width + (int)object.centerX;
            uint16_t* pOutput = (uint16_t*)&pixels[index];
            *pOutput = 0x1F;

            //printf("%d cnt %d ( %d, %d )\n", i, object.cnt, (int)object.centerX, (int)object.centerY);

            if(object.cnt > 500) {
                printf("%d cnt %d ( %d, %d )\n", i, object.cnt, (int)object.centerX, (int)object.centerY);
            }
        }
    }

    if (yellowObjectList){
        free(yellowObjectList->list);
        free(yellowObjectList);
    }

    //sendDataToRobot(command);
    //printf("send command to robot: %d\n", command);
    //waitDataFromRobot();

    //_applyColorMatrix(_pDefaultScreen, pColorMatrix);
    destroyMatrix8(pYellowMatrix);
    _convertScreenToDisplay(_pDefaultScreen);
    displayScreen(_pDefaultScreen);
    return false;
}
