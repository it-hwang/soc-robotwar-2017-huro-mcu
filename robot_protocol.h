#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

enum {
    ROBOT_SET_MOTOR,
    ROBOT_SET_SPEED,
    ROBOT_GET_MOTOR,
    ROBOT_TURN_OFF_ARM_MOTORS,
    MOTION_BASIC_STANCE,
    MOTION_BOARD_CONNECTING_STANCE,
    MOTION_CHECK_SIDELINE_STANCE,
    MOTION_HEAD_FRONT,
    MOTION_HEAD_BOTTOM,
    ROBOT_WALK_FRONT,
    ROBOT_WALK_FRONT_FAST,
    ROBOT_WALK_FRONT_SHORT1,
    ROBOT_WALK_FRONT_SHORT2,
    ROBOT_WALK_FRONT_SHORT3,
    ROBOT_WALK_BACK,
    ROBOT_WALK_FRONT_QUICK_LOW,
    ROBOT_WALK_FRONT_QUICK,
    ROBOT_WALK_FRONT_QUICK_THRESHOLD,
    ROBOT_WALK_BACK_QUICK,
    ROBOT_WALK_CRAWL,
    MOTION_MOVE_LEFT_LIGHT,
    MOTION_MOVE_LEFT_MIDDLE,
    MOTION_MOVE_LEFT_HEAVY,
    MOTION_MOVE_RIGHT_LIGHT,
    MOTION_MOVE_RIGHT_MIDDLE,
    MOTION_MOVE_RIGHT_HEAVY,
    MOTION_TURN_LEFT_LIGHT,
    MOTION_TURN_LEFT_MIDDLE,
    MOTION_TURN_LEFT_HEAVY,
    MOTION_TURN_RIGHT_LIGHT,
    MOTION_TURN_RIGHT_MIDDLE,
    MOTION_TURN_RIGHT_HEAVY,
    MOTION_CLIMB_UP_RED_BRIDGE,
    MOTION_CLIMB_DOWN_RED_BRIDGE,
    MOTION_MINE_WALK,
    MOTION_HURDLE,
    MOTION_CLIMB_UP_STAIR,
    MOTION_GREEN_BRIDGE,
    MOTION_CLIMB_DOWN_STAIR,
    MOTION_GOLF_BALL,
    MOTION_KICK,
    MOTION_GOLF_BALL_LAST,
    MOTION_CLIMB_UP_TRAP,
    MOTION_TRAP,
    MOTION_HORIZONTAL_BARRICADE
};

int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(uint8_t data);
uint8_t receiveDataFromRobot(void);

bool runMotion(uint8_t motionId, bool wait);
bool waitMotion(void);

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


