#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

#define MOTION_HEAD_CENTER          73
#define MOTION_HEAD_BOTTOM          74


int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(uint8_t data);
uint8_t receiveDataFromRobot(void);

bool runMotion(uint8_t motionId, bool wait);
bool waitMotion(void);
void setSpeed(uint8_t speed);

void setHeadVertical(int degrees);
void setHeadHorizontal(int degrees);
void setHead(int horizontalDegrees, int verticalDegrees);

bool walkForward(int millimeters);
bool walkBackward(int millimeters);
bool walkLeft(int millimeters);
bool walkRight(int millimeters);
bool turnLeft(int degrees);
bool turnRight(int degrees);

void udelay(uint64_t microseconds);
void mdelay(uint64_t milliseconds);
void sdelay(uint32_t seconds);

// 삭제 예정
void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata);

#endif // __ROBOT_PROTOCOL_H__


