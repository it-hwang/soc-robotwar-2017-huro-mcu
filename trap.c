
#include "trap.h"
#include "boundary.h"

static bool _approachTrap(void);

bool trapMain(void) {
    _approachTrap();
}

static Object_t* _searchTrap(Screen_t* pScreen) {

    const int MAX_TRIES = 10;

    for(int i = 0; i < MAX_TRIES; ++i) {
        Object_t* pObject = _candidateObjectForBoundary(pSceen);

        if(pObject == NULL)
            continue;

        _setBoundary(pScreen, pObject);
    
        free(pObject);

        Object_t* pTrapObject = NULL;
        bool isTrap = _isTrapObject(pScreen, &pTrapObject);
    
        if( !isTrap ) {
            
        }
    }
    
    return pObject;
}

static bool _approachTrap(void) {
    // 함정을 발견하지 못할 경우 다시 찍는 횟수
    const int MAX_TRIES = 10;
    
    // 장애물에 다가갈 거리 (밀리미터)
    const int APPROACH_DISTANCE = 20;
    // 거리 허용 오차 (밀리미터)
    const int APPROACH_DISTANCE_ERROR = 30;

    const int ALIGN_OFFSET_X = 4;
    const double MILLIMETERS_PER_PIXELS = 2.;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        bool hasFound;
        
        // 앞뒤 정렬
        int distance = measureTrapDistance();
        hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printDebug("전진보행으로 이동하자. (거리: %d)\n", distance);
            walkForward(distance - APPROACH_DISTANCE);
            mdelay(300);
            nTries = 0;
            continue;
        }
        
        printDebug("접근 완료.\n");
        break;
    }
    if (nTries >= MAX_TRIES) {
        printDebug("시간 초과!\n");
        return false;
    }
    
    // 달라붙어 비비기
    runWalk(ROBOT_WALK_FORWARD_QUICK, 12);
    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);

    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {
        bool hasFound;
        
        // 앞뒤 정렬
        int distance = measureTrapDistance();
        hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance > APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printDebug("종종걸음으로 이동하자. (거리: %d)\n", distance);
            runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
            mdelay(300);
            nTries = 0;
            continue;
        }
        
        // 좌우 정렬
        int offsetX = _measureGreenBridgeCenterOffsetX();
        if (offsetX < ALIGN_OFFSET_X * -1) {
            int millimeters = abs(offsetX) * MILLIMETERS_PER_PIXELS;
            printDebug("offsetX: %d, Go left.\n", offsetX);
            walkLeft(millimeters);
            mdelay(300);
            nTries = 0;
            continue;
        }
        else if (offsetX > ALIGN_OFFSET_X) {
            int millimeters = abs(offsetX) * MILLIMETERS_PER_PIXELS;
            printDebug("offsetX: %d, Go right.\n", offsetX);
            walkRight(millimeters);
            mdelay(300);
            nTries = 0;
            continue;
        }
        
        printDebug("접근 완료.\n");
        break;
    }
    if (nTries >= MAX_TRIES) {
        printDebug("시간 초과!\n");
        return false;
    }

    runWalk(ROBOT_WALK_FORWARD_QUICK_THRESHOLD, 4);
    
    return true;
}