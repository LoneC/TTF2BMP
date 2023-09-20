#ifndef _TTF2BMP_PARSE_ARGS_H_
#define _TTF2BMP_PARSE_ARGS_H_

#include <stdint.h>
#include <stdbool.h>

static const char *pre = "[ttf2bmp]:";
static const char *fmt = "%s %s\n";

typedef struct {
	const char *input;
	const char *output;
	int64_t     fontwidth;
	int64_t     fontheight;
	int64_t     cellwidth;
	int64_t     cellheight;
	int64_t     rows;
	int64_t     cols;
	int32_t     bgcolor;
	int32_t     fgcolor;
	bool        unicode;
	bool        verbose;
	bool        infotext;
	bool        infobinary;
} args_t;

const extern args_t PARSE_DEFAULT;

#define PARSE_SUCCESS         0
#define PARSE_NO_OPERATION    1
#define PARSE_FONT_SIZE       2
#define PARSE_CELL_SIZE       3
#define PARSE_COLORS          4
#define PARSE_PROPS_ASCII     5
#define PARSE_PROPS_UNICODE   6
#define PARSE_NO_INPUT        7
#define PARSE_NO_OUTPUT       8
#define PARSE_GAP_SIZE        9
#define PARSE_OTHER          10
int  parse_args(args_t *args, int argc, const char **argv);
void parse_printerr(void *out, int err);

#endif
