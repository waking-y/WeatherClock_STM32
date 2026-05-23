#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>

typedef struct
{
    uint16_t height;
    const uint8_t *model;
} font_t;

extern const font_t font16;
extern const font_t font32;
extern const font_t font48;

#endif /* __FONT_H__ */
