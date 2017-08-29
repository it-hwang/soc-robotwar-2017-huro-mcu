#ifndef __VECTOR3_H__
#define __VECTOR3_H__

typedef struct {
    double x;
    double y;
    double z;
} Vector3_t;

extern const Vector3_t VECTOR3_AXIS_X;
extern const Vector3_t VECTOR3_AXIS_Y;
extern const Vector3_t VECTOR3_AXIS_Z;

double getLengthVector3(Vector3_t* pVector);
void setLengthVector3(Vector3_t* pVector, double length);
void addVector3(Vector3_t* pAugend, Vector3_t* pAddend);
void subtractVector3(Vector3_t* pMinuend, Vector3_t* pSubtrahend);
double dotProductVector3(Vector3_t* pVectorA, Vector3_t* pVectorB);
Vector3_t crossProductVector3(Vector3_t* pVectorA, Vector3_t* pVectorB);
Vector3_t projectVector3(Vector3_t* pProjected, Vector3_t* pDirection);
Vector3_t projectPlaneVector3(Vector3_t* pProjected, Vector3_t* pNormal);
double getAngleVector3(Vector3_t* pVectorA, Vector3_t* pVectorB);
void rotateVector3(Vector3_t* pVector, Vector3_t* pAxis, double radians);

#endif // __VECTOR3_H__
