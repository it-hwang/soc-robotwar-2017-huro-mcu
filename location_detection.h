#ifndef __LOCATION_DETECTION_H__
#define __LOCATION_DETECTION_H__

#include <stdbool.h>

#include "graphic_interface.h"

typedef struct {
    double height;  // 높이 (meters)
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

typedef struct {
    double distance;    // meters
    double angle;       // radians
} WorldLocation_t;


bool convertScreenLocationToWorldLocation(CameraParameters_t* pCamParams, PixelLocation_t* pScreenLoc, WorldLocation_t* pWorldLoc);


#endif // __LOCATION_DETECTION_H__
