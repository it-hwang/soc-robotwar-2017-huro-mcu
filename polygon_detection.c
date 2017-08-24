#include <stdlib.h>
#include <math.h>

#include "polygon_detection.h"

#define _CONTOURLIST_DEFAULT_SIZE   1024


typedef struct {
    PixelLocation_t* elements;
    int size;
} _ContourList_t;

typedef struct __PolygonVertex_t {
    PixelCoordinate_t x;
    PixelCoordinate_t y;
    struct __PolygonVertex_t* pNext;
    struct __PolygonVertex_t* pPrev;
    int contourIndex;
} _PolygonVertex_t;

static _ContourList_t* _findContours(Matrix16_t* pLabelMatrix, Object_t* pObject);
static _ContourList_t* _createContourList(int size);
static void _destroyContourList(_ContourList_t* pContourList);
static bool _insertContour(_ContourList_t* pContourList, int index, PixelLocation_t* pLocation);
static bool _moveToTopLeft(PixelLocation_t* pLocation, Matrix16_t* pLabelMatrix, Object_t* pObject);
static bool _moveToDirection(PixelLocation_t* pLocation, PixelLocation_t* pStartLoc, int direction);
static _PolygonVertex_t* _findFarthestVertex(_ContourList_t* pContourList, int x, int y);
static int _findMiddleVertices(_ContourList_t* pContourList, _PolygonVertex_t* pHeadVertex, _PolygonVertex_t* pTailVertex, int threshold);
static void _destroyVertices(_PolygonVertex_t* pHeadVertex);
static Polygon_t* _createPolygon(_PolygonVertex_t* pHeadVertex, int size);


Polygon_t* createPolygon(Matrix16_t* pLabelMatrix, Object_t* pObject, int threshold) {
    if (!pLabelMatrix) return NULL;
    if (!pObject) return NULL;

    _ContourList_t* pContourList = _findContours(pLabelMatrix, pObject);

    int nVertices = 0;
    // 최초의 점을 찾는다.
    _PolygonVertex_t* pHeadVertex = _findFarthestVertex(pContourList, (int)pObject->centerX, (int)pObject->centerY);
    pHeadVertex->pNext = pHeadVertex->pPrev = pHeadVertex;
    nVertices++;

    if (pContourList->size > 1) {
        _PolygonVertex_t* pTailVertex = _findFarthestVertex(pContourList, pHeadVertex->x, pHeadVertex->y);
        pHeadVertex->pNext = pHeadVertex->pPrev = pTailVertex;
        pTailVertex->pNext = pTailVertex->pPrev = pHeadVertex;
        nVertices++;

        nVertices += _findMiddleVertices(pContourList, pHeadVertex, pTailVertex, threshold);
        nVertices += _findMiddleVertices(pContourList, pTailVertex, pHeadVertex, threshold);
    }

    Polygon_t* pPolygon = _createPolygon(pHeadVertex, nVertices);

    _destroyContourList(pContourList);
    _destroyVertices(pHeadVertex);

    return pPolygon;
}


static _ContourList_t* _findContours(Matrix16_t* pLabelMatrix, Object_t* pObject) {
    _ContourList_t* pContourList = _createContourList(_CONTOURLIST_DEFAULT_SIZE);

    uint16_t* elements = pLabelMatrix->elements;
    int width = pLabelMatrix->width;
    int height = pLabelMatrix->height;
    int id = pObject->id;

    PixelLocation_t startLoc;
    PixelLocation_t currentLoc;
    PixelLocation_t nextLoc;
    int direction = 0;
    int cnt = 0;
    int nContours = 0;

    _moveToTopLeft(&startLoc, pLabelMatrix, pObject); // 시작 지점으로 이동한다.
    currentLoc = startLoc;

    while (true) {
        _moveToDirection(&nextLoc, &currentLoc, direction);

        int index = nextLoc.y * width + nextLoc.x;
        bool isOutOfRange = (nextLoc.x < 0 || nextLoc.x >= width || nextLoc.y < 0 || nextLoc.y >= height);
        if (isOutOfRange || elements[index] != id) {
            if (++direction > 7) direction = 0;
            cnt++;

            if (cnt >= 8) {
                _insertContour(pContourList, nContours, &currentLoc);
                nContours++;
                break;
            }
        }
        else {
            _insertContour(pContourList, nContours, &currentLoc);
            nContours++;
            currentLoc = nextLoc;
            direction = (direction + 6) % 8;    // direction = direction - 2
            cnt = 0;
        }

        if (currentLoc.x == startLoc.x && currentLoc.y == startLoc.y && direction == 0)
            break;
    }

    pContourList->elements = realloc(pContourList->elements, nContours * sizeof(PixelLocation_t));
    pContourList->size = nContours;
    return pContourList;
}

static _ContourList_t* _createContourList(int size) {
    _ContourList_t* pContourList = (_ContourList_t*)malloc(sizeof(_ContourList_t));
    pContourList->elements = (PixelLocation_t*)malloc(size * sizeof(PixelLocation_t));
    pContourList->size = size;
    return pContourList;
}

static bool _insertContour(_ContourList_t* pContourList, int index, PixelLocation_t* pLocation) {
    // 리스트의 크기가 작으면 확장시킨다.
    int size = pContourList->size;
    while (index >= size) {
        size *= 2;
        pContourList->elements = realloc(pContourList->elements, size * sizeof(PixelLocation_t));
        pContourList->size = size;
    }

    pContourList->elements[index] = *pLocation;
    return true;
}

static bool _moveToTopLeft(PixelLocation_t* pLocation, Matrix16_t* pLabelMatrix, Object_t* pObject) {
    uint16_t* elements = pLabelMatrix->elements;
    int width = pLabelMatrix->width;
    int minX = pObject->minX;
    int maxX = pObject->maxX;
    int minY = pObject->minY;
    int maxY = pObject->maxX;
    int id = pObject->id;

    for (int i = minY; i <= maxY; ++i) {
        for (int j = minX; j <= maxX; ++j) {
            int index = i * width + j;
            if (elements[index] == id) {
                pLocation->x = j;
                pLocation->y = i;
                return true;
            }
        }
    }

    return false;
}

static bool _moveToDirection(PixelLocation_t* pLocation, PixelLocation_t* pStartLoc, int direction) {
    switch (direction) {
    case 0:
        pLocation->x = pStartLoc->x + 1;
        pLocation->y = pStartLoc->y;
        return true;
    case 1:
        pLocation->x = pStartLoc->x + 1;
        pLocation->y = pStartLoc->y + 1;
        return true;
    case 2:
        pLocation->x = pStartLoc->x;
        pLocation->y = pStartLoc->y + 1;
        return true;
    case 3:
        pLocation->x = pStartLoc->x - 1;
        pLocation->y = pStartLoc->y + 1;
        return true;
    case 4:
        pLocation->x = pStartLoc->x - 1;
        pLocation->y = pStartLoc->y;
        return true;
    case 5:
        pLocation->x = pStartLoc->x - 1;
        pLocation->y = pStartLoc->y - 1;
        return true;
    case 6:
        pLocation->x = pStartLoc->x;
        pLocation->y = pStartLoc->y - 1;
        return true;
    case 7:
        pLocation->x = pStartLoc->x + 1;
        pLocation->y = pStartLoc->y - 1;
        return true;
    default:
        pLocation->x = pStartLoc->x;
        pLocation->y = pStartLoc->y;
        return false;
    }
}

static void _destroyContourList(_ContourList_t* pContourList) {
    if (!pContourList) return;

    free(pContourList->elements);
    free(pContourList);
}


static Polygon_t* _createPolygon(_PolygonVertex_t* pHeadVertex, int size) {
    Polygon_t* pPolygon = (Polygon_t*)malloc(sizeof(Polygon_t));
    pPolygon->vertices = (PixelLocation_t*)malloc(size * sizeof(PixelLocation_t));
    pPolygon->size = size;

    _PolygonVertex_t* pVertex = pHeadVertex;
    for (int i = 0; i < size; ++i) {
        pPolygon->vertices[i].x = pVertex->x;
        pPolygon->vertices[i].y = pVertex->y;
        pVertex = pVertex->pNext;
    }

    return pPolygon;
}


static _PolygonVertex_t* _findFarthestVertex(_ContourList_t* pContourList, int x, int y) {
    int length = pContourList->size;
    int maxDistance = -1;
    int contourIndex = -1;

    for (int i = 0; i < length; ++i) {
        PixelLocation_t* pContourLoc = &(pContourList->elements[i]);
        int dx = pContourLoc->x - x;
        int dy = pContourLoc->y - y;
        int distance = dx*dx + dy*dy;   // 속도를 위해 sqrt는 쓰지 않았다.

        if (distance > maxDistance) {
            maxDistance = distance;
            contourIndex = i;
        }
    }

    if (contourIndex == -1)
        return NULL;

    _PolygonVertex_t* pVertex = (_PolygonVertex_t*)malloc(sizeof(_PolygonVertex_t));
    PixelLocation_t* pContourLoc = &(pContourList->elements[contourIndex]);
    pVertex->x = pContourLoc->x;
    pVertex->y = pContourLoc->y;
    pVertex->contourIndex = contourIndex;
    pVertex->pNext = NULL;
    pVertex->pPrev = NULL;
    return pVertex;
}

static int _findMiddleVertices(_ContourList_t* pContourList, _PolygonVertex_t* pHeadVertex, _PolygonVertex_t* pTailVertex, int threshold) {
    int maxDistance = -1;
    int contourIndex = -1;

    int a, b, c;
    if (pHeadVertex->x == pTailVertex->x) {
        a = 1;
        b = 0;
        c = pHeadVertex->x * -1;
    }
    else {
        a = pTailVertex->y - pHeadVertex->y;
        b = pHeadVertex->x - pTailVertex->x;
        c = (pTailVertex->x * pHeadVertex->y) - (pHeadVertex->x * pTailVertex->y);
    }
    
    int size = pContourList->size;
    int beginIndex = (pHeadVertex->contourIndex + 1) % size;
    int endIndex = pTailVertex->contourIndex;
    int index = beginIndex;
    while (index != endIndex) {
        PixelLocation_t* pContourLoc = &(pContourList->elements[index]);
        int distance = abs((a * pContourLoc->x) + (b * pContourLoc->y) + c);    // 속도 향상을 위해 상수 값으로 나누지는 않는다.

        if (distance > maxDistance) {
            maxDistance = distance;
            contourIndex = index;
        }

        if (++index >= size) index = 0;
    }

    if (contourIndex == -1)
        return 0;

    double distance = (double)maxDistance / sqrt(a*a + b*b);    // 정확한 거리를 구한다.
    if (distance < threshold)
        return 0;

    _PolygonVertex_t* pMiddleVertex = (_PolygonVertex_t*)malloc(sizeof(_PolygonVertex_t));
    PixelLocation_t* pContourLoc = &(pContourList->elements[contourIndex]);
    pMiddleVertex->x = pContourLoc->x;
    pMiddleVertex->y = pContourLoc->y;
    pMiddleVertex->contourIndex = contourIndex;
    pMiddleVertex->pNext = pTailVertex;
    pMiddleVertex->pPrev = pHeadVertex;
    pHeadVertex->pNext = pMiddleVertex;
    pTailVertex->pPrev = pMiddleVertex;

    int nMiddleVertices = 1;
    nMiddleVertices += _findMiddleVertices(pContourList, pHeadVertex, pMiddleVertex, threshold);
    nMiddleVertices += _findMiddleVertices(pContourList, pMiddleVertex, pTailVertex, threshold);

    return nMiddleVertices;
}

static void _destroyVertices(_PolygonVertex_t* pHeadVertex) {
    if (!pHeadVertex) return;

    _PolygonVertex_t* pVertex = pHeadVertex;
    while (pVertex) {
        _PolygonVertex_t* pNext = pVertex->pNext;
        free(pVertex);
        pVertex = pNext;

        if (pVertex == pHeadVertex) break;
    }
}


void destroyPolygon(Polygon_t* pPolygon) {
    if (!pPolygon) return;

    free(pPolygon->vertices);
    free(pPolygon);
}



//if (x1 == x2) {
//    a = 1;
//    b = 0;
//    c = -x1;
//}
//else {
//    a = y2 - y1;
//    b = x1 - x2;
//    c = x2*y1 - x1*y2;
//}
//
//d = abs(a*x3 + b*y3 + c) / sqrt(a*a + b*b);
//d = abs(a*x3 + b*y3 + c);
