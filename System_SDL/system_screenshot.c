/* $NiH$ */

#include "config.h"
#include "neopop-SDL.h"
#include <stdio.h>

#if !defined(HAVE_LIBPNG)
int
system_screenshot(void)
{
    fprintf(stderr, "PNG library not found during compilation -- "
	    "screenshot feature disabled\n");
    return -1;
}
#else /* HAVE LIBPNG */
#include <sys/stat.h>
#include <errno.h>
#include <png.h>
#include <stdlib.h>

static char *
sanename(char *name)
{
    char sanename[100];
    int start, end, span;

    end = snprintf(sanename, sizeof(sanename), "%s", rom.name);
    /* sanitize file name */
    start = 0;
    while (start < end) {
	span = strspn(sanename+start, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		      "abcdefghijklmnopqrstuvwxyz0123456789-_");
	start += span;
	if (start < end) {
	    sanename[start] = '_';
	    start++;
	}
    };

    return strdup(sanename);
}

static void
converter(png_structp ptr, png_row_infop row_info, png_bytep row)
{
    int i;
    _u16 *inp;
    _u8 out[3*SCREEN_WIDTH];
    _u8 *outp;

    outp = out;
    inp = (_u16 *)row;
    for (i=0; i<SCREEN_WIDTH; i++) {
	*outp++ = (_u8)((*inp & 0x000f)<<4); /* red */
	*outp++ = (_u8)(*inp & 0x00f0);      /* green */
	*outp++ = (_u8)((*inp & 0x0f00)>>4); /* blue */
	inp++;
    }

    memcpy(row, out, sizeof(out));
    return;
}

static char *
get_screenshot_name(void)
{
    static int scount;
    char name[100], *romname;
    struct stat sb;

    if ((romname=sanename(rom.name)) == NULL)
	return NULL;

    do {
	snprintf(name, sizeof(name), "neopop-%s%03d.png", romname, scount);
	if (stat(name, &sb) == -1 && errno == ENOENT)
	    return strdup(name);
	scount++;
    } while (scount < 1000);

    return NULL;
}
    
int
system_screenshot(void)
{
    char *pngname;
    int ret, i;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_color_8 sig_bit;
    png_bytep row_pointer;
    png_bytep row_pointers[SCREEN_HEIGHT];
    FILE *fp;

    ret = 0;
    /* get file name */
    if ((pngname=get_screenshot_name()) == NULL)
	return -1;

    /* open file for writing */
    if ((fp=fopen(pngname, "wb")) == NULL) {
	ret = -1;
	goto end;
    }

    /* init png structures */
    if ((png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
					 NULL, NULL)) == NULL) {
	ret = -1;
	goto closeend;
    }
    
    if ((info_ptr=png_create_info_struct(png_ptr)) == NULL) {
	ret = -1;
	goto freepng;
    }

    png_init_io(png_ptr, fp);
    png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);
    png_set_IHDR(png_ptr, info_ptr, SCREEN_WIDTH, SCREEN_HEIGHT, 8,
		 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    sig_bit.red = sig_bit.green = sig_bit.blue = 4;
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_set_shift(png_ptr, &sig_bit);
    png_set_write_user_transform_fn(png_ptr, converter);
    /* write image to file */
    row_pointer = (png_bytep)cfb;
    for (i=0; i<SCREEN_HEIGHT; i++) {
	row_pointers[i] = row_pointer;
	row_pointer += 2*SCREEN_WIDTH;
    }
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, info_ptr);

 freepng:
    png_destroy_write_struct(&png_ptr, &info_ptr);

 closeend:
    /* close file and cleanup */
    if (ret == 0)
	ret = fclose(fp);
    else
	fclose(fp);

 end:
    free(pngname);
    return ret;
}
#endif /* HAVE_LIBPNG */
