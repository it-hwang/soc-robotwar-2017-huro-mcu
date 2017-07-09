#ifndef __ROBOT_PROTOCOL_H__
#define __ROBOT_PROTOCOL_H__

int		openRobotPort(void);
void	closeRobotPort(void);
void	sendDataToRobot(short data);

#endif // __ROBOT_PROTOCOL_H__


