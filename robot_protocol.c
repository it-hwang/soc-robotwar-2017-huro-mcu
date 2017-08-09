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
	return true;
}


// BUG: 현재 머리 각도가 최초 한번밖에 조절이 안되는 문제가 있다.
// RoboBasic 코드의 SERVO 명령어가 제대로 작동하는지가 가장 의심된다.
void setHeadVertical(int degrees) {
	if (degrees < -90 || degrees > 90)
		return;

	int offset = degrees + 100;

	sendDataToRobot(1);
	receiveDataFromRobot();
	sendDataToRobot(17);
	receiveDataFromRobot();
	sendDataToRobot(offset);
	waitMotion();
}

void setHeadHorizontal(int degrees) {
	if (degrees < -90 || degrees > 90)
		return;

	int offset = degrees + 100;

	sendDataToRobot(1);
	receiveDataFromRobot();
	sendDataToRobot(11);
	receiveDataFromRobot();
	sendDataToRobot(offset);
	waitMotion();
}

void setHead(int horizontalDegrees, int verticalDegrees) {
	setHeadHorizontal(horizontalDegrees);
	setHeadVertical(verticalDegrees);
}


// TODO: 함수 내용을 작성해야한다.
// 실험용이므로 거리가 정확하지 않아도 된다.
// 일단은 milliMeters 신경쓰지 않고 전진보행_1만 써서 구현해도 된다.
//  - 전진보행_1: 두 걸음 전진 (7cm)
//  - 전진보행_2: 네 걸음 전진 (16cm)
//  - 짧은전진보행: 한 걸음 전진 (3cm)
//  - 짧은전진보행2: 한 걸음 전진 (2.5cm)
//  - 짧은전진보행3: 한 걸음 전진 (2cm)
//  - 짧은전진보행4: 한 걸음 전진 (1.5cm)
//  - 빠른전진보행_1: 두 걸음 전진 (7cm)
//  - 전진보행_빠른_바리케이드: 빠른전진보행으로 길게 전진 (40cm)
//  - 전진보행_빠른_바리케이드_멀리: 빠른전진보행으로 길게 전진 (56cm)
bool walkForward(int milliMeters) {

}

bool walkBackward(int milliMeters) {

}

bool walkLeft(int milliMeters) {

}

bool walkRight(int milliMeters) {

}

bool turnLeft(int degrees) {

}

bool turnRight(int degrees) {

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
