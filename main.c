#include <stdio.h>
#include <stdlib.h>

#include "parseargs.h"
#include "conversion.h"
#include "writer.h"

int main(int argc, const char **argv) {
	if(argc == 1) {
		printf("%s Use '-h' or '--help'\n", pre);
		return 0;
	}
	args_t args = PARSE_DEFAULT;
	int ret = parse_args(&args, argc - 1, argv + 1);
	
	if(ret == PARSE_NO_OPERATION)
		return PARSE_SUCCESS;

	if(ret != PARSE_SUCCESS) {
		parse_printerr(stdout, ret);
		return ret;
	}

	/* -- FOR DEBUG PURPOSES, VERBOSE WILL ALWAYS BE ON */
	//args.verbose = true;

	if(args.verbose) {
		printf(
			"%s "
			"Settings for conversion '%s'->'%s'\n"
			"\tFont width: %ld\n"
			"\tFont height: %ld\n"
			"\tCell width: %ld\n"
			"\tCell height: %ld\n"
			"\tRows: %ld\n"
			"\tColumns: %ld\n"
			"\tBackground color: %08X / %3hhu %3hhu %3hhu %3hhu\n"
			"\tForeground color: %08X / %3hhu %3hhu %3hhu %3hhu\n"
			"\tUnicode: %s\n"
			"\tVerbose: %s\n"
			"\tWrite text info file: %s\n"
			"\tWrite binary info binary: %s\n",
			pre, 
			args.input,
			args.output,
			args.fontwidth,
			args.fontheight,
			args.cellwidth,
			args.cellheight,
			args.rows,
			args.cols,
			args.bgcolor,
			(int8_t)(args.bgcolor & 0xFF000000) >> 24, 
			(int8_t)(args.bgcolor & 0x00FF0000) >> 16, 
			(int8_t)(args.bgcolor & 0x0000FF00) >>  8,
			         args.bgcolor & 0x000000FF,
			args.fgcolor,
			(int8_t)(args.fgcolor & 0xFF000000) >> 24, 
			(int8_t)(args.fgcolor & 0x00FF0000) >> 16, 
			(int8_t)(args.fgcolor & 0x0000FF00) >>  8, 
			         args.fgcolor & 0x000000FF,
			(args.unicode ? "True" : "False"),
			(args.verbose ? "True" : "False"),
			(args.infotext ? "True" : "False"),
			(args.infobinary ? "True" : "False")
		);
	}

	ret = conv_init(&args);
	if(ret != CONV_SUCCESS) {
		conv_printerr(stdout, ret);
		return ret;
	}
	
	ret = conv_convert();
	if(ret != CONV_SUCCESS) {
		conv_printerr(stdout, ret);
		return ret;
	}

	img_t out;
	conv_get_img(&out);
	writer_init(&out, &args);

	ret = writer_write();
	if(ret != WRITER_SUCCESS) {
		writer_printerr(stdout, ret);
		return ret;
	}
	
	writer_free();
	conv_free();
	return 0;
}

/* End */
