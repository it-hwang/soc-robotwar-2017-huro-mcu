#include "color.h"


void rgb565ToRgba(COLOR_RGB565* source, COLOR_RGBA* target) {
    target->r = source->r << 3;
    target->g = source->g << 2;
    target->b = source->b << 3;
    target->a = 0x00;
}

void rgbaToRgb565(COLOR_RGBA* source, COLOR_RGB565* target) {
    target->r = source->r >> 3;
    target->g = source->g >> 2;
    target->b = source->b >> 3;
}

void rgba5515ToRgba(COLOR_RGAB5515* source, COLOR_RGBA* target) {
    target->r = source->r << 3;
    target->g = source->g << 3;
    target->b = source->b << 3;
    target->a = ((uint8_t)(source->a << 7)) >> 7;
}

void rgbaToRgba5515(COLOR_RGBA* source, COLOR_RGAB5515* target) {
    target->r = source->r >> 3;
    target->g = source->g >> 3;
    target->b = source->b >> 3;
    target->a = source->a;
}
