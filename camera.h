#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "vector3.h"
#include "graphic_interface.h"

typedef struct {
    Vector3_t worldLoc;  // 초점의 월드 좌표 (meters)
    double yaw;     // 머리 좌우 각도 (radians)
    double pitch;   // 머리 상하 각도 (radians)
    // 캘리브레이션 결과 (GML C++ Camera Calibration Toolbox)
    double fx;      // Focal length[0]
    double fy;      // Focal length[1]
    double cx;      // Principal point[0]
    double cy;      // Principal point[1]
    double k1;      // Distortion[0]: radial distortion 계수
    double k2;      // Distortion[1]: radial distortion 계수
    double p1;      // Distortion[2]: tangential distortion 계수
    double p2;      // Distortion[3]: tangential distortion 계수
} CameraParameters_t;

// pHeadOffset: 현재 로봇의 머리 위치 (meters)
void readCameraParameters(CameraParameters_t* pCamParams, const Vector3_t* pHeadOffset);
void convertScreenLocationToWorldLocation(const CameraParameters_t* pCamParams, const PixelLocation_t* pScreenLoc, double height, Vector3_t* pWorldLoc);

Screen_t* createUndistortedScreen(const Screen_t* pScreen, const CameraParameters_t* pCamParams);

#endif // __CAMERA_H__
