#ifndef __COLOR_H__
#define __COLOR_H__

#include <stdint.h>


typedef enum {
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_YELLOW
} COLOR;

typedef struct {
    union {
        struct {
            uint16_t b : 5;
            uint16_t g : 6;
            uint16_t r : 5;
        };
        uint16_t data16;
    };
} COLOR_RGB565;

typedef struct {
    union {
        struct {
            uint16_t b : 5;
            uint16_t a : 1;
            uint16_t g : 5;
            uint16_t r : 5;
        };
        uint16_t data16;
    };
} COLOR_RGAB5515;



#endif // __COLOR_H__
