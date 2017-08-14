#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>

enum {
    ROBOT_SET_SERVO_OFFSET = 1,
    ROBOT_SET_SERVO_SPEED,
    ROBOT_GET_SERVO_OFFSET,
    ROBOT_RELEASE_ARM_SERVOS,
    MOTION_BASIC_STANCE,
    MOTION_BOARD_CONNECTING_STANCE,
    MOTION_CHECK_SIDELINE_STANCE,
    MOTION_HEAD_FRONT,
    MOTION_HEAD_BOTTOM,
    ROBOT_WALK_FORWARD,
    ROBOT_WALK_FORWARD_FAST,
    ROBOT_WALK_FORWARD_SHORT1,
    ROBOT_WALK_FORWARD_SHORT2,
    ROBOT_WALK_FORWARD_SHORT3,
    ROBOT_WALK_BACKWARD,
    ROBOT_WALK_FORWARD_QUICK_LOW,
    ROBOT_WALK_FORWARD_QUICK,
    ROBOT_WALK_FORWARD_QUICK_THRESHOLD,
    ROBOT_WALK_BACKWARD_QUICK,
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

enum {
    SERVO_HEAD_HORIZONTAL = 11,
    SERVO_HEAD_VERTICAL = 17
};

int openRobotPort(void);
void closeRobotPort(void);
void sendDataToRobot(uint8_t data);
uint8_t receiveDataFromRobot(void);

bool runMotion(uint8_t motionId);
bool waitMotion(void);
void setServoSpeed(uint8_t speed);
void resetServoSpeed(void);
void setServoOffset(uint8_t servoId, uint8_t offset);
uint8_t getServoOffset(uint8_t servoId);

void setHeadVertical(int degrees);
void setHeadHorizontal(int degrees);
void setHead(int horizontalDegrees, int verticalDegrees);

bool runWalk(uint8_t walkId, uint8_t steps);
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


