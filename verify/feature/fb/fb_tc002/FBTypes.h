#ifndef _FB_TYPES_H_

#define _FB_TYPES_H_



#include <stdio.h>

#include <stdint.h>



typedef struct

{

	uint16_t l, t;

	uint16_t w, h;

} FBRect;



typedef struct

{

	uint16_t x, y;

} FBPoint;



typedef struct
{

	uint32_t w, h; // pixel width/height

	uint8_t *p;    // pixels buffer

	int16_t f;     // pixel color format, according to FBColorFmt

} FBImage;

typedef  uint32_t FBCOLOR;



typedef enum
{

	E_FB_CLRFMT_RGB565 = 1,
	E_FB_CLRFMT_ARGB4444 = 2,
	E_FB_CLRFMT_ARGB8888 = 5,
	E_FB_CLRFMT_ARGB1555 = 6,
	E_FB_CLRFMT_YUV422 = 9,
	E_FB_CLRFMT_INVALID = 12,

} FBColorFmt;
#endif
