#include "parseargs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void pri_help() {
	printf(
		"%s Arguments ('-h'/'--help')\n"
		"TTF2BMP converts a single .TTF file into a single image as a bitmap font. Can export to varying file types.\n"
		"\t\t--version\t\t| Print version and license info\n"
		"\t-io\t--input-output\t\t|Required argument; Provide input file and output file in that order\n"
		"\t-s\t--font-size\t\t| Default x64/0x64; Font size to set TTF before conversion\n"
		"\t-c\t--cell-size\t\t| Default 0/0x0; Override output cell sizes in bitmap, format <w>x<h>\n"
			"\t\t\tratio in reguard to the default font's aspect ratio\n"
		"\t-j\t--rows-cols\t\t| Default 16x16, see unicode; Override rows and columns, format <r>x<c>\n"
		"\t-b\t--background-color\t| Default 0x000000FF; Set background color, format 0xRRGGBBAA\n"
		"\t-f\t--foreground-color\t| Default 0xFFFFFFFF; Set foreground color, format 0xRRGGBBAA\n"
		"\t-u\t--unicode\t\t| Default false; Write a unicode bitmap as opposed to default ASCII, chainable argument\n"
		"\t-v\t--verbose\t\t| Print progress to screen, chainable argument\n"
		"\t-t\t--info-text\t\t| Write file with bitmap info as text, chainable argument\n"
		"\t-x\t--info-binary\t\t| Write file with bitmap info as binary, chainable argument\n"
		"Notes -\n"
		"\tSome fonts won't like provided sizes given, so output may be close but not exact."
		" '-v'/'--verbose'/'-t'/'--info-text' may be helpful.\n"
		"\tUnicode currently UNSUPPORTED.\n"
		"\tThe <x>x<y> commands can be passed as just <x>, x<y>, or <x>y<y>, leaving the ommited zero. Ommited values are zero, which means\n"
		"\t\t\tcalculate default\n"
		"\t--foreground-color and --background-color CANNOT be the same value.\n"
		"\t--font-size, --cell-size, and --rows-cols take two positive integers with an 'x' inbetween. --font-size and --cell-size can\n"
		"\t\t\tommit the 'x' and second value, see their descriptions for more info.\n"
		"\t--font-size has UNPREDICTABLE results\n",
		pre	
		
	);
}
static void pri_version() {
	printf(
		"%s Version, licensing, and authors ('--version')\n"
		"\tVersion UNSTABLE\n"
		"\tAuthors & Contributors\n"
		"\t\tJoseph Donald Bingheim\n",
		pre
	);
}
static void pri_badarg(const char *badarg) {
	printf(
		"%s ERROR\n"
		"\t'%s' is an unrecognized argument.\n"
		"\tUse '-h' or '--help' for the list of viable arguments.\n",
		pre, badarg
	);
}
static void pri_unfinished(const char *unfinished) {
	printf(
		"%s ERROR\n"
		"\t'%s' was left unfinished at the end of the command.\n",
		pre, unfinished
	);
}

static void pri_noinput() {
	printf(
		"%s ERROR\n"
		"\tNo input file given ('-i'/'--input').\n",
		pre
	);
}

static void pri_nooutput() {
	printf(
		"%s ERROR\n"
		"\tNo output file given ('-o'/'--output').\n",
		pre
	);
}

const args_t PARSE_DEFAULT = {
	.input        =        NULL,
	.output       =        NULL,
	.fontwidth    =           0,
	.fontheight   =          64,
	.cellwidth    =           0,
	.cellheight   =  	  0,
	.rows         =  	 16,
	.cols         =  	 16,
	.bgcolor      =  0x000000FF,
	.fgcolor      =  0xFFFFFFFF,
	.unicode      =       false,
	.verbose      =       false,
	.infotext     =       false,
	.infobinary   =       false
};

static int verify(args_t *args) {
	if(args->input == NULL)
		return PARSE_NO_INPUT;
	if(args->output == NULL)
		return PARSE_NO_OUTPUT;
	if((args->bgcolor & 0xFFFFFF00) == (args->fgcolor & 0xFFFFFF00))
		return PARSE_COLORS;
	if((args->cellwidth & 0x8000000000000000) || (args->cellheight & 0x8000000000000000))
		return PARSE_CELL_SIZE;
	if(
		(args->fontwidth & 0x8000000000000000) || (args->fontheight & 0x8000000000000000) ||
		(args->fontwidth == 0 && args->fontheight == 0)
	) return PARSE_FONT_SIZE;
	if(args->unicode && (args->rows * args->cols) < 65536)
		return PARSE_PROPS_UNICODE;
	if(!args->unicode && (args->rows * args->cols) < 256)
		return PARSE_PROPS_ASCII;
	return PARSE_SUCCESS;
}

static void set_unicode(args_t *args) {
	args->unicode = true;
	if(args->rows * args->cols < 65536) {
		args->rows = 256;
		args->cols = 256;
	}
}

static int split(const char *arg, int64_t *dst1, int64_t *dst2) {
	if(dst1 == NULL || dst2 == NULL)
		return PARSE_OTHER;
	*dst1 = 0;
	*dst2 = 0;
#define OVERFLOW_CAP 63
	char left[OVERFLOW_CAP + 1], right[OVERFLOW_CAP + 1];
	int64_t i, middle = -1, len = strlen(arg);
	
	for(i = 0; i < len && i < OVERFLOW_CAP; i++) {
		if(arg[i] == 'x') {
			middle = i;
			break;
		}
	}

	if(middle == -1) {
		*dst1 = (int64_t)(strtoul(arg, NULL, 10));
		return PARSE_SUCCESS;
	}

	for(i = 0; i < middle && i < OVERFLOW_CAP; i++)
		left[i] = arg[i];
	left[i] = '\0';
	for(i = 0; i < len && i < OVERFLOW_CAP; i++)
		right[i] = arg[middle + i + 1];
	right[i] = '\0';

	*dst1 = (int64_t)(strtoul(left, NULL, 10));
	*dst2 = (int64_t)(strtoul(right, NULL, 10));
	printf("%ld %ld\n", *dst1, *dst2);
	return PARSE_SUCCESS;
}

static int get_hex(const char *arg, int32_t *dst) {
	*dst = strtol(arg, NULL, 16);
	return PARSE_SUCCESS;
}

static int handle_chained(args_t *args, const char *arg) {
	if(arg[0] != '-')
		return PARSE_OTHER;
	int i;
	for(i = 1; arg[i]; i++) {
		switch(arg[i]) {
			case 'u': set_unicode(args);       break;
			case 'v': args->verbose    = true; break;
			case 't': args->infotext   = true; break;
			case 'x': args->infobinary = true; break;
			default:
				pri_badarg(arg);
				return PARSE_OTHER;	
				break;
		}
	}
	return PARSE_SUCCESS;
}

int parse_args(args_t *args, int argc, const char **argv) {
#define ADVANCE                        \
if(++i >= argc) {                      \
	pri_unfinished(argv[i - 1]);   \
	return PARSE_OTHER;            \
}                  
#define SCMP(a, b)		       \
(strcmp(a, b) == 0)                    
	int i, ret;

	for(i = 0; i < argc; i++) {
		if(SCMP(argv[i], "-h") || SCMP(argv[i], "--help") ) {
			pri_help();
			return PARSE_NO_OPERATION;
		}
		if(SCMP(argv[i], "--version") ) {
			pri_version();
			return PARSE_NO_OPERATION;
		}
	}
	for(i = 0; i < argc; i++) {
		ret = PARSE_SUCCESS;
		if(
			SCMP(argv[i], "-h") || 
			SCMP(argv[i], "--help") ||
			SCMP(argv[i], "--version") 
		) continue;
		/**/ if(SCMP(argv[i], "-io") || SCMP(argv[i], "--input-output")) {
			ADVANCE
			args->input = argv[i];
			ADVANCE
			args->output = argv[i];
		}
		else if(SCMP(argv[i], "-s") || SCMP(argv[i], "--font-size")) {
			ADVANCE	
			split(argv[i], &args->fontwidth, &args->fontheight);
		}
		else if(SCMP(argv[i], "-c") || SCMP(argv[i], "--cell-size")) {
			ADVANCE	
			split(argv[i], &args->cellwidth, &args->cellheight);
		}
		else if(SCMP(argv[i], "-j") || SCMP(argv[i], "--rows-cols") ) {
			ADVANCE
			split(argv[i], &args->rows, &args->cols);
		}
		else if(SCMP(argv[i], "-b") || SCMP(argv[i], "--background-color") ) {
			ADVANCE
			get_hex(argv[i], &args->bgcolor);
		}
		else if(SCMP(argv[i], "-f") || SCMP(argv[i], "--foreground-color") ) {
			ADVANCE
			get_hex(argv[i], &args->fgcolor);

		}
		else if(SCMP(argv[i], "--unicode") )
			set_unicode(args);
		else if(SCMP(argv[i], "--verbose") )
			args->verbose = true;
		else if(SCMP(argv[i], "--info-text") )
			args->infotext = true;
		else if(SCMP(argv[i], "--info-binary") )
			args->infobinary = true;
		else {
			ret = handle_chained(args, argv[i]);
			if(ret != PARSE_SUCCESS)
				break;
		}
	}
	if(ret != PARSE_SUCCESS)
		return ret;
	return verify(args);
#undef ADVANCE
#undef SCMP
}

void parse_printerr(void *out, int err) {
	FILE *os = (FILE *)(out);
	switch(err) {
		default:
			break;
		case PARSE_FONT_SIZE:
			fprintf(os, fmt, pre, "Invalid font size.");
			break;
		case PARSE_CELL_SIZE:
			fprintf(os, fmt, pre, "Invalid cell size.");
			break;
		case PARSE_COLORS:
			fprintf(os, fmt, pre, "Colors are the same.");
			break;
		case PARSE_PROPS_ASCII:
			fprintf(os, fmt, pre, "Cannot fit ASCII into the provided rows and columns.");
			break;
		case PARSE_PROPS_UNICODE:
			fprintf(os, fmt, pre, "Cannot fit UNICODE into the provided rows and columns.");
			break;
		case PARSE_NO_INPUT:
			fprintf(os, fmt, pre, "No TTF was provided, cannot perform operation.");
			break;
		case PARSE_NO_OUTPUT:
			fprintf(os, fmt, pre, "No output destination was given, cannot perform operation.");
			break;
		case PARSE_OTHER:
			fprintf(os, "%s General error; undefined error code <%d>\n", pre, err);
			break;
	}
}











