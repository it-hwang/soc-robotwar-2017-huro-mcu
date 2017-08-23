

bool blueGateMain() {

}

static bool _approchBlueGate(void) {
    // 빨간 다리를 발견하지 못할 경우 다시 찍는 횟수
    static const int MAX_TRIES = 10;
    // 장애물에 다가갈 거리 (밀리미터)
    static const int APPROACH_DISTANCE = 150;
    // 거리 허용 오차 (밀리미터)
    static const int APPROACH_DISTANCE_ERROR = 50;
    
    int nTries;
    for (nTries = 0; nTries < MAX_TRIES; ++nTries) {    
        // 앞뒤 정렬
        int distance = measureFrontLineDistance();
        bool hasFound = (distance != 0);
        if (!hasFound)
            continue;
        
        if (distance <= APPROACH_DISTANCE + APPROACH_DISTANCE_ERROR) {
            printDebug("접근 완료.\n");
            break;
        }
        else {
            printDebug("전진보행으로 이동하자. (거리: %d)\n", distance);
            walkForward(distance - APPROACH_DISTANCE);
            mdelay(500);
            nTries = 0;
        }
    }

    if (nTries >= MAX_TRIES) {
        printDebug("시간 초과!\n");
        return false;
    }
    
    return true;
}

int measureBlueGateDistance(void) {
    // 거리 측정에 사용되는 머리 각도
    //static const int HEAD_HORIZONTAL_DEGREES = 0;
    //static const int HEAD_VERTICAL_DEGREES = -50;

    _setHeadForward();

    Screen_t* pScreen = createDefaultScreen();

    int millimeters = 0;

    Line_t* pLine = _captureForwardLine(pScreen);

    if (pLine != NULL) {
        printDebug("leftPointY: %d, centerPointY: %d, rigthPointY: %d\n",
                 pLine->leftPoint.y, pLine->centerPoint.y, pLine->rightPoint.y);

        // 화면 상의 위치로 실제 거리를 추측한다.
        int distance = pLine->centerPoint.y;
        
        millimeters = 655.21 * exp(-0.016 * distance);
        // 0을 반환하면 장애물이 없다고 생각할 수도 있기 때문에 1mm로 반환한다. 
        if (millimeters <= 0)
            millimeters = 1;
    }

    if (pLine != NULL)
        free(pLine);
    destroyScreen(pScreen);

    printDebug("millimeters: %d\n", millimeters);
    return millimeters;
}

static void _setHeadRight(void) {
    setServoSpeed(30);
    //runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(35, -40);
    mdelay(1000);
    resetServoSpeed();
}

static void _setHeadLeft(void) {
    setServoSpeed(30);
    //runMotion(MOTION_CHECK_SIDELINE_STANCE);
    setHead(-35, -40);
    mdelay(1000);
    resetServoSpeed();
}

static void _setHeadForward(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    setHead(0, -40);
    mdelay(1000);
    resetServoSpeed();
}

static void _setStandardStand(void) {
    setServoSpeed(30);
    runMotion(MOTION_BASIC_STANCE);
    setHead(0, 0);
    mdelay(1000);
    resetServoSpeed();
}