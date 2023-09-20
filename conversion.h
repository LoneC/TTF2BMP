#ifndef _TTF2BMP_CONVERSION_H_
#define _TTF2BMP_CONVERSION_H_

#include "parseargs.h"

#define CONV_SUCCESS    0
#define CONV_FT         1
#define CONV_FACE       2
#define CONV_MEMORY     3
#define CONV_SIZE       4

typedef struct {
	uint64_t  width;
	uint64_t  height;
	uint64_t  rows;
	uint64_t  cols;
	uint64_t  wcell;
	uint64_t  hcell;
	void     *pixels;
} img_t;

int  conv_init(const args_t *a);
void conv_free();
int  conv_convert();
void conv_get_img(img_t *img);
void conv_printerr(void *out, int err);


#endif
