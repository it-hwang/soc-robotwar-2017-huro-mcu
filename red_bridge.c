#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "graphic_interface.h"
#include "object_detection.h"
#include "robot_protocol.h"
#include "image_filter.h"
#include "red_bridge.h"

Screen_t* _pDefaultScreen;

bool redBridgeMain(void) {

    _pDefaultScreen = createDefaultScreen();

    ObjectList_t* objList = _captureObject(_pDefaultScreen, COLOR_RED, false);
}

ObjectList_t* _captureRedObject(Screen_t* pScreen, Color_t color, bool flg) {
        
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
            }
        }

        displayScreen(pScreen);

        return objList;
}