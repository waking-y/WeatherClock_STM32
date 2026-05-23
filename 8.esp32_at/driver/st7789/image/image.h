#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>

typedef struct
{
	uint16_t width;
	uint16_t height;
	const unsigned char *data;
}ImageDef;

extern const unsigned char gImage_ikun[];

extern const ImageDef my_demo_ikun;

#endif /* __IMAGE_H__ */
