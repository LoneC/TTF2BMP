#include "conversion.h"

#include "writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <assert.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

/* Args */
static const char *ipath;
static uint64_t    fontwidth, fontheight;
static uint64_t    cellwidth, cellheight;
static uint64_t    rows,      cols;
static uint64_t    bgcolor,   fgcolor;
static bool        unicode,   verbose;
static bool        infotext,  infobinary;

#define VPRI(fmt, ...)                      \
	if(verbose)			    \
	do {                                \
		printf("%s ", pre);         \
		printf(fmt, ##__VA_ARGS__); \
	} while(0)
static char _2c[16];
static const char *CESC(char c) {
	switch(c) {
		case  ' ': return "<space>"; break;
		case '\n': return "<newline>"; break;
		case '\r': return "<return>"; break;
		case '\b': return "<back>"; break;
		case '\t': return "<tab>"; break;
		case '\0': return "<nullterm>"; break;
		case '\a': return "<dingdong>"; break;
		case '\f': return "<f?>"; break;
		case '\v': return "<v?>"; break;
	}
	_2c[0] = c;
	_2c[1] = '\0';
	return _2c;
}


/* Conversion data */
static FT_Library    ft = NULL;
static FT_Face       face = NULL;
static uint64_t      imagewidth, imageheight;
static uint64_t      pixelssize, pixelslen;
static uint32_t     *pixels = NULL;
static FILE         *tpath,     *bpath;
static uint64_t      maxfontwidth, maxfontheight;

static void print_init() {
	VPRI(
		"Constrained results\n"
		"\tInput file: \"%s\"\n"
		"\tFont w/h: %lu x %lu\n"
		"\tCell w/h: %lu x %lu\n"
		"\tImage w/h: %lu x %lu\n"
		"\t# of pixels: %lu\n"
		"\tTotal bytes: %lu\n",
		ipath,
		maxfontwidth, maxfontheight,
		cellwidth, cellheight,
		imagewidth, imageheight,
		pixelslen,
		pixelssize
	);
}

static void print_conv() {
	VPRI(
		"%s...\n", "Converting..."
	);
}

static void print_cell() {

}

static void enumerate_max_sizes() {
	uint32_t c;
	maxfontwidth = 0;
	maxfontheight = 0;
	for(c = 0; c < 256; c++) {
		FT_Load_Char(face, (char)(c), FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);
		if(face->glyph->bitmap.width > maxfontwidth)
			maxfontwidth = face->glyph->bitmap.width;
		if(face->glyph->bitmap.rows > maxfontheight)
			maxfontheight = face->glyph->bitmap.rows;
	}
}

int conv_init(const args_t *a) {
	ipath = a->input;
	fontwidth = a->fontwidth;
	fontheight = a->fontheight;
	cellwidth = a->cellwidth;
	cellheight = a->cellheight;
	rows = a->rows;
	cols = a->cols;
	bgcolor = a->bgcolor;
	fgcolor = a->fgcolor;
	unicode = a->unicode;
	verbose = a->verbose;
	infotext = a->infotext;
	infobinary = a->infobinary;
	
	if(FT_Init_FreeType(&ft))
		return CONV_FT;
	if(FT_New_Face(ft, ipath, 0, &face)) {
		conv_free();
		return CONV_FACE;
	}
	if(FT_Set_Pixel_Sizes(face, fontwidth, fontheight)) {
		conv_free();
		return CONV_SIZE;
	}
	/* This code not consistent >:(
	 * Use the hack function!
	maxfontwidth = face->size->metrics.max_advance >> 6;
	maxfontheight = (face->ascender - face->descender) >> 6; */
	enumerate_max_sizes();


	if(cellwidth == 0 && cellheight == 0) {
		cellwidth = maxfontwidth;
		cellheight = maxfontheight;
	}
	else if(cellwidth == 0) {
		double props = (double)(maxfontwidth) / (double)(maxfontheight);
		cellwidth = (uint64_t)((double)(cellheight) * props);
	}
	else if(cellheight == 0) /* cellheight == 0 */ {
		double props = (double)(maxfontheight) / (double)(maxfontwidth);
		cellheight = (uint64_t)((double)(cellwidth) * props);
	}

	imagewidth = cellwidth * cols;
	imageheight = cellheight * rows;
	pixelslen = imagewidth * imageheight;
	pixelssize = pixelslen * sizeof(uint32_t);
	pixels = malloc(pixelssize);
	
	if(pixels == NULL) {
		conv_free();
		return CONV_MEMORY;
	}
	print_init();
	
	return CONV_SUCCESS;
}

void conv_free() {
	if(face != NULL)
		FT_Done_Face(face);
	if(ft != NULL)
		FT_Done_FreeType(ft);
	if(pixels != NULL)
		free(pixels);
}

static int write_cell(FT_Bitmap *bitmap, uint32_t c) {
	uint64_t xspot, yspot;
	uint64_t row = c / cols; 
	uint64_t col = c % cols;
	uint64_t x, y;
	uint64_t srcwidth = bitmap->width;
	uint64_t srcheight = bitmap->rows;
	
	if(srcwidth == 0 || srcheight == 0) {
		for(y = 0; y < cellheight; y++) {
			for(x = 0; x < cellwidth; x++) {
				xspot = col * cellwidth;
				yspot = row * cellheight;
				pixels[(yspot + y) * imagewidth + (xspot + x)] = bgcolor;
			}
		}
		return CONV_SUCCESS;
	}
	
	double   targetwratio = (double)(maxfontwidth) / (double)(srcwidth);
	uint64_t targetwidth  = (uint64_t)( (double)(cellwidth) / targetwratio );
	double   targethratio = (double)(maxfontheight) / (double)(srcheight);
	uint64_t targetheight = (uint64_t)( (double)(cellheight) / targethratio );
	
	double   xscale = (double)(srcwidth)  / (double)(targetwidth);
	double   yscale = (double)(srcheight) / (double)(targetheight);
	uint64_t srcx, srcy;
	uint8_t  gray;
	int64_t left = face->glyph->bitmap_left /
		      ((double)(maxfontwidth) / (double)(cellwidth));
	int64_t top = (double)((face->ascender >> 6) - face->glyph->bitmap_top) / 
		      ((double)(maxfontheight) / (double)(cellheight));
	if(left < 0) left = 0;
	if(top < 0) top = 0;
	
	if(verbose) {
		printf(
			"%s=["
			"sw%lu,sh%lu,tw%lu,th%lu,"
			"rw%.2lf,rh%.2lf,xs%.2lf,ys%.2lf,"
			"t%lu,l%lu]\n",
			CESC(c),
			srcwidth, srcheight, targetwidth, targetheight,
			targetwratio, targethratio, xscale, yscale,
			top, left
		);
	}

	for(y = 0; y < targetheight; y++) {
		for(x = 0; x < targetwidth; x++) {
			xspot = col * cellwidth;
			yspot = row * cellheight;
			srcx = (uint64_t)(x * xscale);
			srcy = (uint64_t)(y * yscale);
			gray = bitmap->buffer[srcy * srcwidth + srcx];
			pixels[
				(yspot + y + top) * 
				imagewidth + 
				(xspot + x + left)
			] = (gray != 0) ? fgcolor : bgcolor;
		}
	}
	for(y = 0; y < top; y++) {
		for(x = 0; x < cellwidth; x++) {
			pixels[
				(yspot + y) *
				imagewidth +
				(xspot + x)
			] = bgcolor;
		}
	}
	for(y = targetheight + top; y < cellheight; y++) {
		for(x = 0; x < cellwidth; x++) {
			pixels[
				(yspot + y) *
				imagewidth +
				(xspot + x)
			] = bgcolor;
		}
	}
	for(y = top; y < cellheight; y++) {
		for(x = 0; x < left; x++) {
			pixels[
				(yspot + y) *
				imagewidth +
				(xspot + x)
			] = bgcolor;
		}
	}
	for(y = top; y < cellheight; y++) {
		for(x = targetwidth + left; x < cellwidth; x++) {
			pixels[
				(yspot + y) *
				imagewidth +
				(xspot + x)
			] = bgcolor;
		}
	}
	return PARSE_SUCCESS;
}

int conv_convert() {
	print_conv();

	uint32_t c;
	for(c = 0; c < 256; c++) { 
		assert(!FT_Load_Char(
			face, (char)(c),
			FT_LOAD_DEFAULT |
			FT_LOAD_RENDER
		));
		write_cell(&face->glyph->bitmap, c);
	}


	return CONV_SUCCESS;
}

void conv_get_img(img_t *img) {
	img->width = imagewidth;
	img->height = imageheight;
	img->rows = rows;
	img->cols = cols;
	img->wcell = cellwidth;
	img->hcell = cellheight;
	img->pixels = pixels;
}

void conv_printerr(void *out, int err) {
	FILE *os = (FILE *)(out);
	switch(err) {
		default:
			break;
		case CONV_FT:
			fprintf(os, fmt, pre, "Could not intialize FreeType2.");
			break;
		case CONV_FACE:
			fprintf(os, fmt, pre, "Could not create requested font or the file path does not exist.");
			break;
		case CONV_MEMORY:
			fprintf(os, fmt, pre, "Fatal memory error.");
			break;
		case CONV_SIZE:
			fprintf(os, fmt, pre, "Could not set the size, validation has failed.");
			break;
	}
}






/* end */
