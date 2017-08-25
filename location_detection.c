#include <math.h>

#include "location_detection.h"

#define PI 3.141592


typedef struct {
    double x;
    double y;
} _NormalizedLoc_t;


static void _undistortPixel(CameraParameters_t* pCamParams, int* pX, int* pY);

bool convertScreenLocationToWorldLocation(CameraParameters_t* pCamParams, PixelLocation_t* pScreenLoc, WorldLocation_t* pWorldLoc) {
    if (!pCamParams) return false;
    if (!pScreenLoc) return false;
    if (!pWorldLoc) return false;

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

    double c1c2 = pCamParams->height;
    double c2p2 = c1c2 * tan((PI / 2) + pCamParams->pitch - atan(v));
    double c1p2 = sqrt(c1c2*c1c2 + c2p2*c2p2);
    double c1p3 = sqrt(1 + v*v);
    double p1p2 = u * c1p2 / c1p3;
    double d = sqrt(c2p2*c2p2 + p1p2*p1p2);
    double theta = -atan2(p1p2, c2p2);

    pWorldLoc->distance = d;
    pWorldLoc->angle = pCamParams->yaw + theta;

    return true;
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

static Screen_t* _createUndistortedScreen(Screen_t* pScreen, CameraParameters_t* pCamParams) {
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

/*
static double _coordinateToAngle(double angleOfView, int length, int coordinate);

bool convertScreenLocationToWorldLocation(CameraParameters_t* pCamParams, PixelLocation_t* pScreenLoc, WorldLocation_t* pWorldLoc) {
    if (!pCamParams) return false;
    if (!pScreenLoc) return false;
    if (!pWorldLoc) return false;

    int x = pScreenLoc->x - pCamParams->centerX;
    int y = pScreenLoc->y - pCamParams->centerY;
    double angleX = _coordinateToAngle(pCamParams->horizontalAngleOfView, pCamParams->screenWidth, x);
    double angleY = _coordinateToAngle(pCamParams->verticalAngleOfView, pCamParams->screenHeight, y);

    double theta = (PI / 2) + pCamParams->pitch + angleY;
    double distanceY = pCamParams->height * tan(theta);
    double distance = distanceY / cos(angleX);

    pWorldLoc->distance = distance;
    pWorldLoc->angle = pCamParams->yaw + angleX;

    return true;
}

static double _coordinateToAngle(double angleOfView, int length, int coordinate) {
    double d = ((double)length / 2) / (tan(angleOfView / 2));
    return atan((double)coordinate / d);
}
*/
