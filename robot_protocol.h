#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

#define MOTION_DEFAULT 0x08
#define MOTION_CHECK_RIGHT 0x03
#define MOTION_CHECK_LEFT 0x02
#define MOTION_MOVE_RIGHT 0x05
#define MOTION_MOVE_LEFT 0x04
#define MOTION_TURN_LEFT 0x06
#define MOTION_TURN_RIGHT 0x07
#define MOTION_MOVE_FORWARD 0x09
#define MOTION_MOVE_DOWN 0x48
#define MOTION_TURN_LEFT_BIG 0x3c
#define MOTION_TURN_RIGHT_BIG 0x3d
#define MOTION_HEAD_DOWN 0x22
#define MOTION_HEAD_FRONT 0x13
#define MOTION_HEAD_LEFT 0x14
#define MOTION_HEAD_RIGHT 0x15
#define MOTION_BARRICADE_RUN 0x0e
#define MOTION_BARRICADE_RUN_FAR 0x0f
#define MOTION_TURN_90 0x10
#define MOTION_BIBIGI 0x18
#define MOTION_BIBIGI2 0x19
#define MOTION_BIBIGI3 0x1a
#define MOTION_BIBIGI4 0x1c
#define MOTION_BIBIGI5 0x1e
#define MOTION_HEAD_FRONT_DOWN_MINE 0x2a
#define MOTION_HEAD_LEFT_DOWN_MINE 0x2b
#define MOTION_HEAD_RIGHT_DOWN_MINE 0x2c
#define MOTION_STEP_UP 0x46
#define MOTION_STEP_DOWN 0x47


int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(short data);
void DelayLoop(int delay_time);
void Send_Command(unsigned char Ldata);
void waitMotion(void);
#endif // __ROBOT_PROTOCOL_H__


