#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

#define MOTION_DEFAULT 0x08
#define MOTION_CHECK_RIGHT 0x03
#define MOTION_CHECK_LEFT 0x02
#define MOTION_MOVE_RIGHT 0x05
#define MOTION_MOVE_LEFT 0x04
#define MOTION_TURN_LEFT 0x06
#define MOTION_TURN_RIGHT 0x07
//#define MOTION_TURN_CORNER 0x3e
#define MOTION_MOVE_FORWARD 0x3e
#define MOTION_MOVE_DOWN 0x48
#define MOTION_TURN_LEFT_BIG 0x3c
#define MOTION_TURN_RIGHT_BIG 0x3d
#define MOTION_MOVE_HUDDLE 0x27
#define MOTION_STAND_HUDDLE 0x2a

int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(short data);
void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata);
void waitMotion(void);
#endif // __ROBOT_PROTOCOL_H__


