#ifndef __COLOR_MODEL_H__
#define __COLOR_MODEL_H__

#include <stdint.h>

#define COLOR_MODEL_USE_TYPE_CHECKING     true


typedef struct {
    union {
        struct {
            uint8_t a;
            uint8_t b;
            uint8_t g;
            uint8_t r;
        };
        uint32_t data;
    };
} Rgba_t;

typedef struct {
    union {
        struct {
            uint16_t b : 5;
            uint16_t g : 6;
            uint16_t r : 5;
        };
        uint16_t data;
    };
} Rgb565_t;

typedef struct {
    union {
        struct {
            uint16_t b : 5;
            uint16_t a : 1;
            uint16_t g : 5;
            uint16_t r : 5;
        };
        uint16_t data;
    };
} Rgab5515_t;


#if COLOR_USE_TYPE_CHECK
    static inline uint32_t rgb565ToRgbaData(Rgb565_t* pRgb565) {
        return ((uint32_t)pRgb565->r << 27) |
               ((uint32_t)pRgb565->g << 18) |
               ((uint32_t)pRgb565->b << 11);
    }

    static inline uint16_t rgb565ToRgab5515Data(Rgb565_t* pRgb565) {
        return ((uint16_t)pRgb565->data & 0xffdf)
    }

    static inline uint32_t rgab5515ToRgbaData(Rgab5515_t* pRgab5515) {
        return ((uint32_t)pRgab5515->r << 27) |
               ((uint32_t)pRgab5515->g << 19) |
               ((uint32_t)pRgab5515->b << 11) |
               ((uint32_t)pRgab5515->a);
    }

    static inline uint16_t rgab5515ToRgb565Data(Rgab5515_t* pRgab5515) {
        return ((uint16_t)pRgab5515->data & 0xffdf |
               (((uint16_t)pRgab5515->data >> 1) & 0x0020));
    }

    static inline uint8_t rgab5515ToGrayscale(Rgab5515_t* pRgab5515) {
        return (uint8_t)(((float)((uint8_t)pRgab5515->r << 3) * 0.2126) +
                         ((float)((uint8_t)pRgab5515->g << 3) * 0.7152) +
                         ((float)((uint8_t)pRgab5515->b << 3) * 0.0722));
    }

    static inline uint16_t rgbaToRgb565Data(Rgba_t* pRgba) {
        return (((uint16_t)pRgba->r & 0xf8) << 8) |
               (((uint16_t)pRgba->g & 0xfc) << 3) |
               (((uint16_t)pRgba->b) >> 3);
    }
    
    static inline uint16_t rgbaToRgab5515Data(Rgba_t* pRgba) {
        return (((uint16_t)pRgba->r & 0xf8) << 8) |
               (((uint16_t)pRgba->g & 0xf8) << 3) |
               (((uint16_t)pRgba->a & 0x01) << 5) |
               (((uint16_t)pRgba->b) >> 3);
    }
#else
    #define rgb565ToRgbaData(pRgb565) \
                ((uint32_t)((Rgb565_t*)pRgb565)->r << 27) |\
                ((uint32_t)((Rgb565_t*)pRgb565)->g << 18) |\
                ((uint32_t)((Rgb565_t*)pRgb565)->b << 11)

    #define rgb565ToRgab5515Data(pRgb565) \
                ((uint16_t)pRgb565->data & 0xffdf)

    #define rgab5515ToRgbaData(pRgab5515) \
                ((uint32_t)((Rgab5515_t*)pRgab5515)->r << 27) |\
                ((uint32_t)((Rgab5515_t*)pRgab5515)->g << 19) |\
                ((uint32_t)((Rgab5515_t*)pRgab5515)->b << 11) |\
                ((uint32_t)((Rgab5515_t*)pRgab5515)->a)

    #define rgab5515ToRgb565Data(pRgab5515) \
                ((uint16_t)pRgab5515->data & 0xffdf |\
                (((uint16_t)pRgab5515->data >> 1) & 0x0020))

    #define rgab5515ToGrayscale(pRgab5515) \
                ((uint8_t)(((float)((uint8_t)pRgab5515->r << 3) * 0.2126) +\
                           ((float)((uint8_t)pRgab5515->g << 3) * 0.7152) +\
                           ((float)((uint8_t)pRgab5515->b << 3) * 0.0722)))

    #define rgbaToRgb565Data(pRgba) \
                (((uint16_t)((Rgba_t*)pRgba)->r & 0xf8) << 8) |\
                (((uint16_t)((Rgba_t*)pRgba)->g & 0xfc) << 3) |\
                (((uint16_t)((Rgba_t*)pRgba)->b) >> 3)

    #define rgbaToRgab5515Data(pRgba) \
                (((uint16_t)((Rgba_t*)pRgba)->r & 0xf8) << 8) |\
                (((uint16_t)((Rgba_t*)pRgba)->g & 0xf8) << 3) |\
                (((uint16_t)((Rgba_t*)pRgba)->a & 0x01) << 5) |\
                (((uint16_t)((Rgba_t*)pRgba)->b) >> 3)
#endif

#endif // __COLOR_MODEL_H__
