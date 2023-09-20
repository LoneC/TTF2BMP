#include "writer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <png.h>
#include <jpeglib.h>

static const char *filepath;
static uint64_t    width, height, rows, cols, wcell, hcell;
static void       *pixels;
static uint32_t    bgcolor, fgcolor;
static bool        tinfo, binfo;

void writer_init(img_t *img, args_t *a) {
	width = img->width;
	height = img->height;
	rows = img->rows;
	cols = img->cols;
	wcell = img->wcell;
	hcell = img->hcell;
	pixels = img->pixels;
	filepath = a->output;
	bgcolor = a->bgcolor;
	fgcolor = a->fgcolor;
	tinfo = a->infotext;
	binfo = a->infobinary;
}

void writer_free() {
}

void endian_swap(void *data, uint64_t size, uint64_t elsize) {
	if(elsize == 1)
		return;
	uint8_t *bytes = (uint8_t *)(data);
	uint64_t i, j;
	uint8_t t;
	for(i = 0; i < size; i += elsize) {
		for(j = 0; j < elsize / 2; j++) {
			t = bytes[i + j];
			bytes[i + j] = bytes[i + elsize - 1 - j];
			bytes[i + elsize - 1 - j] = t;
		}
	}
}

int writer_determine() {
	uint64_t len = strlen(filepath);
	int64_t i, dot = 0;
	for(i = len - 1; i --> 0;) {
		if(filepath[i] == '.') {
			dot = i;
			break;
		}
	}
	if(dot == 0 || dot + 1 >= len)
		return WRITER_FORMAT;
	char ext[8];
	strncpy(ext, &filepath[dot + 1], 7);
	for(i = 0; ext[i]; i++) {
		if(ext[i] >= 'A' && ext[i] <= 'Z')
			ext[i] -= ('A' - 'a');
	}
	/**/ if(strcmp(ext, "ppm") == 0)
		return IMAGE_PPM;
	else if(strcmp(ext, "bmp") == 0)
		return IMAGE_BMP;
	else if(strcmp(ext, "png") == 0)
		return IMAGE_PNG;
	else if(strcmp(ext, "jpg") == 0 ||
	        strcmp(ext, "jpeg") == 0)
		return IMAGE_JPG;
	return WRITER_FORMAT;
}

static int write_ppm() {
	FILE *os = fopen(filepath, "w");
	if(os == NULL)
		return WRITER_FILE;
	fprintf(os, "P3 %lu %lu 255\n", width, height);
	uint32_t *pix = (uint32_t *)(pixels);
	uint64_t x, y;
	for(y = 0; y < height; y++) {
		for(x = 0; x < width; x++) {
			fprintf(
				os,
				"%4hhu %4hhu %4hhu\n",
				pix[y * width + x] >> 24,
				pix[y * width + x] >> 16,
				pix[y * width + x] >>  8
			);
		}
	}
	fclose(os);
	return WRITER_SUCCESS;
}


static int write_bmp() {
	assert("[ttf2bmp] BMP not implemented" && 0);
	return WRITER_SUCCESS;
}


static int write_png() {
	FILE *os = fopen(filepath, "wb");
	if(os == NULL)
		return WRITER_FILE;

	png_structp png = png_create_write_struct(
		PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL
	);

	png_infop info = png_create_info_struct(png);
	png_init_io(png, os);
	
	png_set_IHDR(
		png,
		info,
		width, height, 8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);
	png_write_info(png, info);
	
	uint64_t y;
	uint32_t *pix = (uint32_t *)(pixels);
	endian_swap(pix, width * height * 4, 4);
	for(y = 0; y < height; y++) {
		png_bytep row = (png_bytep)(&pix[y * width]);
		png_write_row(png, row);
	}
	
	png_write_end(png, NULL);
	png_destroy_write_struct(&png, &info);
	fclose(os);
	return WRITER_SUCCESS;
}


static int write_jpg() {
	assert("[ttf2bmp] JPG not implemented" && 0);
	return WRITER_SUCCESS;
}

static int tinfo_write() {
	char *path = malloc(strlen(filepath) + 5);
	strcpy(path, filepath);
	strcat(&path[strlen(path) - 5], ".txt");
	FILE *os = fopen(path, "w");
	free(path);
	if(os == NULL)
		return WRITER_INFO_FILE;
	fprintf(
		os,
		"# %s\n"
		"width-height %lu %lu\n"
		"rows-cols %lu %lu\n"
		"wcell-hcell %lu %lu\n"
		"background %hhu %hhu %hhu %hhu\n"
		"foreground %hhu %hhu %hhu %hhu\n",
		filepath,
		width, height,
		rows, cols,
		wcell, hcell,
		((bgcolor & 0xFF000000) >> 24),
		((bgcolor & 0x00FF0000) >> 16),
		((bgcolor & 0x0000FF00) >>  8),
		((bgcolor & 0x000000FF)      ),
		((fgcolor & 0xFF000000) >> 24),
		((fgcolor & 0x00FF0000) >> 16),
		((fgcolor & 0x0000FF00) >>  8),
		((fgcolor & 0x000000FF)      )
	);
	fclose(os);
	return WRITER_SUCCESS;
}
static int binfo_write() {
	assert("[ttf2bmp] Writing binary info file not implemented" && 0);
	return WRITER_SUCCESS;
}

int writer_write() {
	int ret = writer_determine();
	switch(ret) {
		default:
			return ret;
			break;
		case IMAGE_PPM:
			ret = write_ppm();
			break;
		case IMAGE_BMP:
			ret = write_bmp();
			break;
		case IMAGE_PNG:
			ret = write_png();
			break;
		case IMAGE_JPG:
			ret = write_jpg();
			break;
	}
	if(tinfo)
		ret = tinfo_write();
	if(binfo)
		ret = binfo_write();
	return ret;
}


void writer_printerr(void *out, int err) {
	FILE *os = (FILE *)(out);
	switch(err) {
		default:
			break;
		case WRITER_FILE:
			fprintf(os, fmt, pre, "File could not be written to");
			break;
		case WRITER_FORMAT:
			fprintf(os, fmt, pre, "Unknown file type for image");
			break;
	}
}






/* End */
