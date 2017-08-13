#include <stdlib.h>
#include <stdio.h>

#include "robot_protocol.h"
#include "uart_api.h"

#define _UART_BAUD_RATE		9600
#define _UART_BITS			8
#define _UART_STOPS			1


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


bool runMotion(uint8_t motionId, bool wait) {
	sendDataToRobot(motionId);

	if (wait == true) {
		return waitMotion();
	}
	else {
		return true;
	}
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


void setHeadVertical(int degrees) {
	if (degrees < -90 || degrees > 90)
		return;

	int offset = degrees + 100;
	setServoOffset(17, offset);
}

void setHeadHorizontal(int degrees) {
	if (degrees < -90 || degrees > 90)
		return;

	int offset = degrees + 100;
	setServoOffset(11, offset);
}

void setHead(int horizontalDegrees, int verticalDegrees) {
	setHeadHorizontal(horizontalDegrees);
	setHeadVertical(verticalDegrees);
}


bool runWalk(uint8_t walkId, uint8_t steps) {
	sendDataToRobot(walkId);

	receiveDataFromRobot();
	sendDataToRobot(steps);

	return waitMotion();
}


// TODO: 실험용으로 작성했으므로 더 섬세하게 작성해야한다.
// 일단은 milliMeters 신경쓰지 않고 작성되었다.
//  - 전진보행_1: 두 걸음 전진 (7cm)
//  - 전진보행_2: 네 걸음 전진 (16cm)
//  - 짧은전진보행: 한 걸음 전진 (3cm)
//  - 짧은전진보행2: 한 걸음 전진 (2.5cm)
//  - 짧은전진보행3: 한 걸음 전진 (2cm)
//  - 짧은전진보행4: 한 걸음 전진 (1.5cm)
//  - 빠른전진보행_1: 두 걸음 전진 (7cm)
//  - 전진보행_빠른_바리케이드: 빠른전진보행으로 길게 전진 (40cm)
//  - 전진보행_빠른_바리케이드_멀리: 빠른전진보행으로 길게 전진 (56cm)
bool walkForward(int millimeters) {
	if (millimeters > 35)
		runWalk(ROBOT_WALK_FORWARD, millimeters / 35);
	else if (millimeters > 0)
		runWalk(ROBOT_WALK_FORWARD, 1);

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
	while (millimeters > 0) {
		if (!runMotion(MOTION_MOVE_LEFT_MIDDLE, true))
			return false;
		millimeters -= 20;
	}
	
	return true;
}

bool walkRight(int millimeters) {
	while (millimeters > 0) {
		if (!runMotion(MOTION_MOVE_RIGHT_MIDDLE, true))
			return false;
		millimeters -= 20;
	}
	
	return true;
}

bool turnLeft(int degrees) {
	while (degrees > 0) {
		if (!runMotion(MOTION_TURN_LEFT_MIDDLE, true))
			return false;
		degrees -= 20;
	}
	
	return true;
}

bool turnRight(int degrees) {
	while (degrees > 0) {
		if (!runMotion(MOTION_TURN_RIGHT_MIDDLE, true))
			return false;
		degrees -= 20;
	}
	
	return true;
}


void udelay(uint64_t microseconds) {
	uint64_t counter = 18.4162 * microseconds;
	while (counter) {
		counter--;
	}
}

void mdelay(uint64_t milliseconds) {
	// udelay의 인수가 너무 커지는것을 방지하기위해 나누어 처리한다.
	while (milliseconds > 1000) {
		udelay(1000000);
		milliseconds -= 1000;
	}
	udelay(milliseconds * 1000);
}

void sdelay(uint32_t seconds) {
	// mdelay의 인수가 너무 커지는것을 방지하기위해 나누어 처리한다.
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
