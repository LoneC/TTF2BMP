#ifndef _TTF2BMP_WRITER_H_
#define _TTF2BMP_WRITER_H_

#include "parseargs.h"
#include "conversion.h"

#define WRITER_SUCCESS   0
#define IMAGE_PPM        1
#define IMAGE_BMP        2
#define IMAGE_PNG        3
#define IMAGE_JPG        4
#define WRITER_FILE      5
#define WRITER_INFO_FILE 6
#define WRITER_FORMAT    7
void writer_init(img_t *img, args_t *a);
void writer_free();
int  writer_determine();
int  writer_write();
void writer_printerr(void *out, int err);

#endif
