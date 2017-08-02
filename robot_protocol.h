#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

#define MOTION_DEFAULT 0x34
#define MOTION_MOVE_RIGHT 0x36
#define MOTION_MOVE_LEFT 0x38
#define MOTION_TURN_LEFT 0x3a
#define MOTION_TURN_RIGHT 0x3c
#define MOTION_TURN_CORNER 0x3e
#define MOTION_MOVE_FORWARD 0x40
#define MOTION_MOVE_DOWN 0x48
#define MOTION_TURN_LEFT_BIG 0x53
#define MOTION_TURN_RIGHT_BIG 0x54

int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(short data);
void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata);
void waitMotion(void);
#endif // __ROBOT_PROTOCOL_H__


