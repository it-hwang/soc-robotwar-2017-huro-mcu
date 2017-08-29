// #define DEBUG
#include <math.h>

#include "vector3.h"
#include "debug.h"


const Vector3_t VECTOR3_AXIS_X = {1.0, 0.0, 0.0};
const Vector3_t VECTOR3_AXIS_Y = {0.0, 1.0, 0.0};
const Vector3_t VECTOR3_AXIS_Z = {0.0, 0.0, 1.0};

double getLengthVector3(Vector3_t* pVector) {
    if (!pVector) {
        printDebug("pVector is NULL.\n");
        return 0.0;
    }

    double x = pVector->x;
    double y = pVector->y;
    double z = pVector->z;

    return sqrt(x*x + y*y + z*z);
}

void setLengthVector3(Vector3_t* pVector, double length) {
    if (!pVector) {
        printDebug("pVector is NULL.\n");
        return;
    }

    double l = getLengthVector3(pVector);
    if (l == 0.0) {
        pVector->x = length;
        printDebug("pVector's length is 0.\n");
        return;
    }

    l = length / l;
    pVector->x *= l;
    pVector->y *= l;
    pVector->z *= l;
}

void addVector3(Vector3_t* pAugend, Vector3_t* pAddend) {
    if (!pAugend) {
        printDebug("pAugend is NULL.\n");
        return;
    }
    if (!pAddend) {
        printDebug("pAddend is NULL.\n");
        return;
    }

    pAugend->x += pAddend->x;
    pAugend->y += pAddend->y;
    pAugend->z += pAddend->z;
}

void subtractVector3(Vector3_t* pMinuend, Vector3_t* pSubtrahend) {
    if (!pMinuend) {
        printDebug("pMinuend is NULL.\n");
        return;
    }
    if (!pSubtrahend) {
        printDebug("pSubtrahend is NULL.\n");
        return;
    }

    pMinuend->x -= pSubtrahend->x;
    pMinuend->y -= pSubtrahend->y;
    pMinuend->z -= pSubtrahend->z;
}

double dotProductVector3(Vector3_t* pVectorA, Vector3_t* pVectorB) {
    if (!pVectorA) {
        printDebug("pVectorA is NULL.\n");
        return 0.0;
    }
    if (!pVectorB) {
        printDebug("pVectorB is NULL.\n");
        return 0.0;
    }

    return (pVectorA->x * pVectorB->x +
            pVectorA->y * pVectorB->y +
            pVectorA->z * pVectorB->z);
}

Vector3_t crossProductVector3(Vector3_t* pVectorA, Vector3_t* pVectorB) {
    Vector3_t result = {0.0, 0.0, 0.0};

    if (!pVectorA) {
        printDebug("pVectorA is NULL.\n");
        return result;
    }
    if (!pVectorB) {
        printDebug("pVectorB is NULL.\n");
        return result;
    }

    result.x = pVectorA->y * pVectorB->z - pVectorA->z * pVectorB->y;
    result.y = pVectorA->z * pVectorB->x - pVectorA->x * pVectorB->z;
    result.z = pVectorA->x * pVectorB->y - pVectorA->y * pVectorB->x;
    return result;
}

Vector3_t projectVector3(Vector3_t* pProjected, Vector3_t* pDirection) {
    Vector3_t result = {0.0, 0.0, 0.0};

    if (!pProjected) {
        printDebug("pProjected is NULL.\n");
        return result;
    }
    if (!pDirection) {
        printDebug("pDirection is NULL.\n");
        return result;
    }

    double l = getLengthVector3(pDirection);
    if (l == 0.0) {
        printDebug("pDirection's length is 0.\n");
        return result;
    }

    l = dotProductVector3(pProjected, pDirection) / l;
    result.x = pDirection->x * l;
    result.y = pDirection->y * l;
    result.z = pDirection->z * l;
    return result;
}

Vector3_t projectPlaneVector3(Vector3_t* pProjected, Vector3_t* pNormal) {
    Vector3_t result = {0.0, 0.0, 0.0};

    if (!pProjected) {
        printDebug("pProjected is NULL.\n");
        return result;
    }
    if (!pNormal) {
        printDebug("pNormal is NULL.\n");
        return result;
    }

    double l = getLengthVector3(pNormal);
    if (l == 0.0) {
        printDebug("pNormal's length is 0.\n");
        return result;
    }

    l = dotProductVector3(pProjected, pNormal) / l;
    result.x = pProjected->x - pNormal->x * l;
    result.y = pProjected->y - pNormal->y * l;
    result.z = pProjected->z - pNormal->z * l;
    return result;
}

double getAngleVector3(Vector3_t* pVectorA, Vector3_t* pVectorB) {
    if (!pVectorA) {
        printDebug("pVectorA is NULL.\n");
        return 0.0;
    }
    if (!pVectorB) {
        printDebug("pVectorB is NULL.\n");
        return 0.0;
    }

    double l = getLengthVector3(pVectorA) * getLengthVector3(pVectorB);
    if (l == 0.0) {
        printDebug("The length of at least one of the vectors is 0.\n");
        return 0.0;
    }

    return acos(dotProductVector3(pVectorA, pVectorB) / l);
}

void rotateVector3(Vector3_t* pVector, Vector3_t* pAxis, double angle) {
    if (!pVector) {
        printDebug("pVector is NULL.\n");
        return;
    }
    if (!pAxis) {
        printDebug("pAxis is NULL.\n");
        return;
    }

    double al = dotProductVector3(pAxis, pAxis); // (pAxis's length)^2
    if (al == 0.0) {
        printDebug("pAxis's length is 0.\n");
        return;
    }
    
    double c = cos(angle);
    double s = sin(angle);
    double f = dotProductVector3(pVector, pAxis) / al;
    double zx = pAxis->x * f;
    double zy = pAxis->y * f;
    double zz = pAxis->z * f;  // axis component of rotated vector
    double xx = pVector->x - zx;
    double xy = pVector->y - zy;
    double xz = pVector->z - zz;  // component of vector perpendicular to axis
    al = sqrt(al);
    double yx = (pAxis->y * xz - pAxis->z * xy) / al;
    double yy = (pAxis->z * xx - pAxis->x * xz) / al;  // y same length as x by using cross product and dividing with axis length
    double yz = (pAxis->x * xy - pAxis->y * xx) / al;  // x,y - coordinate system in which we rotate
    pVector->x = xx * c + yx * s + zx;
    pVector->y = xy * c + yy * s + zy;
    pVector->z = xz * c + yz * s + zz;
}
