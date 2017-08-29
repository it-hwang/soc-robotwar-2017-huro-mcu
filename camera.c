#include <math.h>

#include "camera.h"
#include "robot_protocol.h"
#include "math.h"

typedef struct {
    double x;
    double y;
} _NormalizedLoc_t;

static void _undistortPixel(CameraParameters_t* pCamParams, int* pX, int* pY);


void readCameraParameters(CameraParameters_t* pCamParams, Vector3_t* pHeadOffset) {
    // 목에서 카메라 초점까지의 거리 (meters)
    double cameraOffsetX = 0.0000;
    double cameraOffsetY = 0.0095;
    double cameraOffsetZ = 0.0410;
    double yaw = (double)getHeadHorizontal() * -1 * DEG_TO_RAD;
    double pitch = (double)getHeadVertical() * DEG_TO_RAD;
    
    // double fx = 146.181;
    // double fy = 132.462;
    // double cx = 94.868;
    // double cy = 63.161;
    // double k1 = -0.417426;
    // double k2 = 0.172889;
    // double p1 = -0.004961;
    // double p2 = -0.002298;

    // double fx = 142.962;
    // double fy = 129.645;
    // double cx = 94.609;
    // double cy = 58.863;
    // double k1 = -0.397926;
    // double k2 = 0.152559;
    // double p1 = 0.001119;
    // double p2 = -0.001430;

    double fx = 142.991;
    double fy = 129.732;
    double cx = 94.003;
    double cy = 59.221;
    double k1 = -0.406361;
    double k2 = 0.168461;
    double p1 = 0.000926;
    double p2 = -0.000226;

    Vector3_t camOffset = {cameraOffsetX, cameraOffsetY, cameraOffsetZ};
    rotateVector3(&camOffset, &VECTOR3_AXIS_X, pitch);
    rotateVector3(&camOffset, &VECTOR3_AXIS_Z, yaw);

    pCamParams->worldLoc.x = pHeadOffset->x + camOffset.x;
    pCamParams->worldLoc.y = pHeadOffset->y + camOffset.y;
    pCamParams->worldLoc.z = pHeadOffset->z + camOffset.z;
    pCamParams->yaw = yaw;
    pCamParams->pitch = pitch;
    pCamParams->fx = fx;
    pCamParams->fy = fy;
    pCamParams->cx = cx;
    pCamParams->cy = cy;
    pCamParams->k1 = k1;
    pCamParams->k2 = k2;
    pCamParams->p1 = p1;
    pCamParams->p2 = p2;
}

void convertScreenLocationToWorldLocation(CameraParameters_t* pCamParams, PixelLocation_t* pScreenLoc, double height, Vector3_t* pWorldLoc) {
    if (!pCamParams) return;
    if (!pScreenLoc) return;
    if (!pWorldLoc) return;

    int ux = pScreenLoc->x;
    int uy = pScreenLoc->y;
    _undistortPixel(pCamParams, &ux, &uy);

    double x = ux;
    double y = uy;
    double cx = pCamParams->cx;
    double cy = pCamParams->cy;
    double fx = pCamParams->fx;
    double fy = pCamParams->fy;

    double u = (x - cx) / fx;
    double v = (y - cy) / fy;

    double c1c2 = pCamParams->worldLoc.z - height;
    double c2p2 = c1c2 * tan((M_PI / 2) + pCamParams->pitch - atan(v));
    double c1p2 = sqrt(c1c2*c1c2 + c2p2*c2p2);
    double c1p3 = sqrt(1 + v*v);
    double p1p2 = u * c1p2 / c1p3;
    double d = sqrt(c2p2*c2p2 + p1p2*p1p2);
    double angle = pCamParams->yaw - atan2(p1p2, c2p2);

    double wx = d * sin(angle * -1);
    double wy = d * cos(angle);
    
    pWorldLoc->x = wx + pCamParams->worldLoc.x;
    pWorldLoc->y = wy + pCamParams->worldLoc.y;
    pWorldLoc->z = height;
}

static void _normalize(CameraParameters_t* pCamParams,  double* pX, double* pY) {
    double x = *pX;
    double y = *pY;
    double y_n = (y - pCamParams->cy) / pCamParams->fy;
    double x_n = (x - pCamParams->cx) / pCamParams->fx;

    *pX = x_n;
    *pY = y_n;
}

static void _denormalize(CameraParameters_t* pCamParams, double* pX, double* pY) {
    double x = *pX;
    double y = *pY;
    double x_p = pCamParams->fx*x + pCamParams->cx;
    double y_p = pCamParams->fy*y + pCamParams->cy;

    *pX = x_p;
    *pY = y_p;
}

static void _distort(CameraParameters_t* pCamParams, double* pX, double* pY) {
    double x = *pX;
    double y = *pY;
    double r2 = x*x + y*y;
    double radial_d = 1 + pCamParams->k1*r2 + pCamParams->k2*r2*r2;
    double x_d = radial_d*x + 2 * pCamParams->p1*x*y + pCamParams->p2*(r2 + 2 * x*x);
    double y_d = radial_d*y + pCamParams->p1*(r2 + 2 * y*y) + 2 * pCamParams->p2*x*y;

    *pX = x_d;
    *pY = y_d;
}

static void _distortPixel(CameraParameters_t* pCamParams, int* pX, int* pY) {
    double x = *pX;
    double y = *pY;

    _normalize(pCamParams, &x, &y);
    _distort(pCamParams, &x, &y);
    _denormalize(pCamParams, &x, &y);

    *pX = (int)(x + 0.5);
    *pY = (int)(y + 0.5);
}

static void _undistortPixel(CameraParameters_t* pCamParams, int* pX, int* pY) {
    double xd = *pX;
    double yd = *pY;

    _normalize(pCamParams, &xd, &yd);
    double xu = xd;
    double yu = yd;

    double err_thr = (0.1 / pCamParams->fx)*(0.1 / pCamParams->fx) + (0.1 / pCamParams->fy)*(0.1 / pCamParams->fy);

    while (1)
    {
        double xud = xu;
        double yud = yu;
        _distort(pCamParams, &xud, &yud);

        double err_x = xud - xd;
        double err_y = yud - yd;
        double err = err_x*err_x + err_y*err_y;

        xu = xu - err_x;
        yu = yu - err_y;

        if (err<err_thr) break;
    }

    _denormalize(pCamParams, &xu, &yu);

    *pX = (int)(xu + 0.5);
    *pY = (int)(yu + 0.5);
}

Screen_t* createUndistortedScreen(Screen_t* pScreen, CameraParameters_t* pCamParams) {
    double fx = pCamParams->fx;
    double fy = pCamParams->fy;
    double cx = pCamParams->cx;
    double cy = pCamParams->cy;
    double k1 = pCamParams->k1;
    double k2 = pCamParams->k2;
    double p1 = pCamParams->p1;
    double p2 = pCamParams->p2;

    int width = pScreen->width;
    int height = pScreen->height;

    Screen_t* pUndistortedScreen = createScreen(width, height);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double y_nu = ((double)y - cy) / fy;
            double x_nu = ((double)x - cx) / fx;

            double ru2 = x_nu*x_nu + y_nu*y_nu;
            double radial_d = 1 + k1*ru2 + k2*ru2*ru2;

            double x_nd = radial_d*x_nu + 2 * p1*x_nu*y_nu + p2*(ru2 + 2 * x_nu*x_nu);
            double y_nd = radial_d*y_nu + p1 * (ru2 + 2 * y_nu*y_nu) + 2 * p2*x_nu*y_nu;

            double x_pd = fx*x_nd + cx;
            double y_pd = fy*y_nd + cy;

            int index1 = y * width + x;
            int index2 = (int)y_pd * width + (int)x_pd;
            pUndistortedScreen->elements[index1] = pScreen->elements[index2];

        }
    }

    return pUndistortedScreen;
}
