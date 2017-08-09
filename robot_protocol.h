#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(uint8_t data);
uint8_t receiveDataFromRobot(void);

bool runMotion(uint8_t motionId, bool wait);
bool waitMotion(void);

void setHeadVertical(int degrees);
void setHeadHorizontal(int degrees);
void setHead(int horizontalDegrees, int verticalDegrees);

bool walkForward(int milliMeters);
bool walkBackward(int milliMeters);
bool walkLeft(int milliMeters);
bool walkRight(int milliMeters);
bool turnLeft(int degrees);
bool turnRight(int degrees);

// 삭제 예정
void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata);

#endif // __ROBOT_PROTOCOL_H__


