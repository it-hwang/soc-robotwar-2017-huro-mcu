#include "color.h"

void rgb565ToRgba(COLOR_RGB565* source, COLOR_RGBA* target) {
    //target->r = source->r << 3;
    //target->g = source->g << 2;
    //target->b = source->b << 3;
    //target->a = 0x00;
    target->data32 = ((uint32_t)source->r << 27) | ((uint32_t)source->g << 18) | ((uint32_t)source->b << 11);
}

void rgbaToRgb565(COLOR_RGBA* source, COLOR_RGB565* target) {
    //target->r = source->r >> 3;
    //target->g = source->g >> 2;
    //target->b = source->b >> 3;
    target->data16 = (((uint16_t)source->r & 0xf8) << 8) | (((uint16_t)source->g & 0xfc) << 3) | (((uint16_t)source->b) >> 8);
}

void rgab5515ToRgba(COLOR_RGAB5515* source, COLOR_RGBA* target) {
    //target->r = source->r << 3;
    //target->g = source->g << 3;
    //target->b = source->b << 3;
    //target->a = ((uint8_t)(source->a << 7)) >> 7;
    target->data32 = ((uint32_t)source->r << 27) | ((uint32_t)source->g << 18) | ((uint32_t)source->b << 11) | ((uint32_t)source->a);
}

void rgbaToRgab5515(COLOR_RGBA* source, COLOR_RGAB5515* target) {
    //target->r = source->r >> 3;
    //target->g = source->g >> 3;
    //target->b = source->b >> 3;
    //target->a = source->a;
    target->data16 = (((uint16_t)source->r & 0xf8) << 8) | (((uint16_t)source->g & 0xf8) << 4) | (((uint16_t)source->a) << 5) | (((uint16_t)source->b) >> 3);
}