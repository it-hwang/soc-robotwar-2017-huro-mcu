#include <stdlib.h>
#include <stdio.h>

#include "robot_protocol.h"
#include "uart_api.h"
#include "timer.h"

#define _UART_BAUD_RATE		9600
#define _UART_BITS			8
#define _UART_STOPS			1


static int _headVertical = 0;
static int _headHorizontal = 0;

static void _setHeadVertical(int degrees);
static void _setHeadHorizontal(int degrees);


int openRobotPort(void) {
    int status;

    status = uart_open();
    if (status < 0)
        return status;
    
    uart_config(UART1, _UART_BAUD_RATE, _UART_BITS, UART_PARNONE, _UART_STOPS);

    return 0;
}

void closeRobotPort(void) {
    uart_close();
}

void sendDataToRobot(uint8_t data) {
    unsigned char buffer[1];
    buffer[0] = data;

    uart1_buffer_write(buffer, 1);
}

uint8_t receiveDataFromRobot(void) {
    unsigned char buffer[1] = {0};
    uart1_buffer_read(buffer, 1);

    return buffer[0];
}


bool runMotion(uint8_t motionId) {
    sendDataToRobot(motionId);
    return waitMotion();
}

bool waitMotion(void) {
    uint8_t receivedData = receiveDataFromRobot();
    bool isSuccess = (receivedData == 0);
    return isSuccess;
}

void setServoSpeed(uint8_t speed) {
    sendDataToRobot(ROBOT_SET_SERVO_SPEED);

    receiveDataFromRobot();
    sendDataToRobot(speed);

    waitMotion();
}

void resetServoSpeed(void) {
    setServoSpeed(0);
}

void setServoOffset(uint8_t servoId, uint8_t offset) {
    sendDataToRobot(ROBOT_SET_SERVO_OFFSET);

    receiveDataFromRobot();
    sendDataToRobot(servoId);

    receiveDataFromRobot();
    sendDataToRobot(offset);

    waitMotion();
}

uint8_t getServoOffset(uint8_t servoId) {
    sendDataToRobot(ROBOT_GET_SERVO_OFFSET);

    receiveDataFromRobot();
    sendDataToRobot(servoId);

    uint8_t offset = receiveDataFromRobot();
    return offset;
}


int getHeadHorizontal(void) {
    // int offset = getServoOffset(SERVO_HEAD_HORIZONTAL);
    // int degrees = offset - 100;
    // return degrees;
    return _headHorizontal;
}

int getHeadVertical(void) {
    // int offset = getServoOffset(SERVO_HEAD_VERTICAL);
    // int degrees = offset - 100;
    // return degrees;
    return _headVertical;
}

void setHeadVertical(int degrees) {
    _setHeadVertical(degrees);
    runMotion(ROBOT_WAIT_SERVO_OFFSET);
}

void setHeadHorizontal(int degrees) {
    _setHeadHorizontal(degrees);
    runMotion(ROBOT_WAIT_SERVO_OFFSET);
}

void setHead(int horizontalDegrees, int verticalDegrees) {
    _setHeadHorizontal(horizontalDegrees);
    _setHeadVertical(verticalDegrees);
    runMotion(ROBOT_WAIT_SERVO_OFFSET);
}

static void _setHeadVertical(int degrees) {
    if (degrees < -90 || degrees > 90)
        return;

    int offset = degrees + 100;
    setServoOffset(SERVO_HEAD_VERTICAL, offset);
    _headVertical = degrees;
}

static void _setHeadHorizontal(int degrees) {
    if (degrees < -90 || degrees > 90)
        return;

    int offset = degrees + 100;
    setServoOffset(SERVO_HEAD_HORIZONTAL, offset);
    _headHorizontal = degrees;
}


bool runWalk(uint8_t walkId, uint8_t steps) {
    sendDataToRobot(walkId);

    receiveDataFromRobot();
    sendDataToRobot(steps);

    return waitMotion();
}


// TODO: ??????????????? ?????????????????? ??? ???????????? ??????????????????.
// ????????? milliMeters ???????????? ?????? ???????????????.
//  - ????????????_1: ??? ?????? ?????? (7cm)
//  - ????????????_2: ??? ?????? ?????? (16cm)
//  - ??????????????????: ??? ?????? ?????? (3cm)
//  - ??????????????????2: ??? ?????? ?????? (2.5cm)
//  - ??????????????????3: ??? ?????? ?????? (2cm)
//  - ??????????????????4: ??? ?????? ?????? (1.5cm)
//  - ??????????????????_1: ??? ?????? ?????? (7cm)
//  - ????????????_??????_???????????????: ???????????????????????? ?????? ?????? (40cm)
//  - ????????????_??????_???????????????_??????: ???????????????????????? ?????? ?????? (56cm)
bool walkForward(int millimeters) {
    setServoSpeed(30);
    setHead(0, 0);

    if (millimeters < 6) {
        millimeters = 6;
    }

    float remainingDistance = millimeters;

    int nSteps;	
    nSteps = remainingDistance / 34.;
    if (nSteps >= 2) {
        runWalk(ROBOT_WALK_RUN_FORWARD_32MM, nSteps);
        remainingDistance -= (float)nSteps * 34.;
    }

    nSteps = remainingDistance / 3.;
    if (nSteps >= 2) {
        runWalk(ROBOT_WALK_RUN_FORWARD_3MM, nSteps);
        remainingDistance -= (float)nSteps * 3.;
    }

    return true;
}

bool walkBackward(int millimeters) {
    if (millimeters > 35)
        runWalk(ROBOT_WALK_BACKWARD, millimeters / 35);
    else if (millimeters > 0)
        runWalk(ROBOT_WALK_BACKWARD, 1);
    
    return true;
}

bool walkLeft(int millimeters) {
    if (millimeters < 10) millimeters = 10;

    while (millimeters >= 30) {
        if (!runMotion(MOTION_MOVE_LEFT_30MM))
            return false;
        millimeters -= 30;
    }
    
    while (millimeters >= 20) {
        if (!runMotion(MOTION_MOVE_LEFT_20MM))
            return false;
        millimeters -= 20;
    }
    while (millimeters >= 10) {
        if (!runMotion(MOTION_MOVE_LEFT_10MM))
            return false;
        millimeters -= 10;
    }
    
    return true;
}

bool walkRight(int millimeters) {
    if (millimeters < 10) millimeters = 10;
    
    while (millimeters >= 30) {
        if (!runMotion(MOTION_MOVE_RIGHT_30MM))
            return false;
        millimeters -= 30;
    }
    
    while (millimeters >= 20) {
        if (!runMotion(MOTION_MOVE_RIGHT_20MM))
            return false;
        millimeters -= 20;
    }
    
    while (millimeters >= 10) {
        if (!runMotion(MOTION_MOVE_RIGHT_10MM))
            return false;
        millimeters -= 10;
    }
    
    return true;
}

bool turnLeft(int degrees) {
    if (degrees < 5) {
        degrees = 5;
    }

    float remainingDegrees = degrees;

    int nSteps;	
    nSteps = remainingDegrees / 27.5;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_LEFT_27DEG);
        remainingDegrees -= 27.5;
    }

    nSteps = remainingDegrees / 6.4;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_LEFT_6DEG);
        remainingDegrees -= 6.4;
    }

    nSteps = remainingDegrees / 4.5;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_LEFT_4DEG);
        remainingDegrees -= 4.5;
    }

    return true;
}

bool turnRight(int degrees) {
    if (degrees < 5) {
        degrees = 5;
    }

    float remainingDegrees = degrees;

    int nSteps;	
    nSteps = remainingDegrees / 27.5;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_RIGHT_27DEG);
        remainingDegrees -= 27.5;
    }

    nSteps = remainingDegrees / 6.4;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_RIGHT_6DEG);
        remainingDegrees -= 6.4;
    }

    nSteps = remainingDegrees / 4.5;
    for (int i = 0; i < nSteps; ++i) {
        runMotion(MOTION_TURN_RIGHT_4DEG);
        remainingDegrees -= 4.5;
    }
    
    return true;
}


void udelay(uint64_t microseconds) {
    if (isTimerOpened()) {
        uint64_t endTime = getTime() + microseconds;
        while (getTime() < endTime);
    }
    else {
        uint64_t counter = 18.4162 * microseconds;
        while (counter) {
            counter--;
        }
    }
}

void mdelay(uint64_t milliseconds) {
    // udelay??? ????????? ?????? ??????????????? ?????????????????? ????????? ????????????.
    while (milliseconds > 1000) {
        udelay(1000000);
        milliseconds -= 1000;
    }
    udelay(milliseconds * 1000);
}

void sdelay(uint32_t seconds) {
    // mdelay??? ????????? ?????? ??????????????? ?????????????????? ????????? ????????????.
    while (seconds > 1000) {
        mdelay(1000000);
        seconds -= 1000;
    }
    mdelay(seconds * 1000);
}


void DelayLoop(int delay_time)
{
    while(delay_time)
        delay_time--;
}

void Send_Command(unsigned char Ldata)
{
    unsigned char Command_Buffer[1] = {0,};
    
    Command_Buffer[0] = Ldata;

    uart1_buffer_write(Command_Buffer, 1);
}
