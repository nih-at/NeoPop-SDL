/* $NiH: system_io.c,v 1.5 2002/12/02 14:25:23 wiz Exp $ */

#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "neopop.h"

static BOOL
read_file_to_buffer(char *filename, _u8 *buffer, _u32 len)
{
    FILE *fp;
    _u32 got;

    if ((fp=fopen(filename, "rb")) == NULL)
	return FALSE;

    while ((got=fread(buffer, 1, len, fp)) < len) {
	if (feof(fp)) {
	    fclose(fp);
	    return FALSE;
	}
	if (ferror(fp) && errno != EINTR) {
	    fclose(fp);
	    return FALSE;
	}

	len -= got;
	buffer += got;
    }
	    
    if (fclose(fp) != 0)
	return FALSE;

    return TRUE;
}

static BOOL
write_file_from_buffer(char *filename, _u8 *buffer, _u32 len)
{
    FILE *fp;
    _u32 written;

    if ((fp=fopen(filename, "wb")) == NULL)
	return FALSE;

    while ((written=fwrite(buffer, 1, len, fp)) < len) {
	if (feof(fp)) {
	    fclose(fp);
	    return FALSE;
	}
	if (ferror(fp) && errno != EINTR) {
	    fclose(fp);
	    return FALSE;
	}

	len -= written;
	buffer += written;
    }
	    
    if (fclose(fp) != 0)
	return FALSE;

    return TRUE;
}


static BOOL
validate_dir(char *path)
{
    struct stat sb;

    if (stat(path, &sb) == -1 && errno == ENOENT) {
    	if (mkdir(path, 0777) == -1) {
	    fprintf(stderr, "Config directory `%s' does not exist and "
		    "creation failed: %s", path, strerror(errno));
	    return FALSE;
	}
	if (stat(path, &sb) == -1) {
	    fprintf(stderr, "Config directory `%s' creation failed: %s",
		    path, strerror(errno));
	    return FALSE;
	}
    }

    if ((sb.st_mode & S_IFDIR) != S_IFDIR) {
	fprintf(stderr, "Config directory `%s' is not a directory", path);
	return FALSE;
    }

    return TRUE;
}

void
system_state_load(void)
{
    char fn[256];

    snprintf(fn, sizeof(fn), "%s.ngs", rom.filename);
    state_restore(fn);

    return;
}

void
system_state_save(void)
{
    char fn[256];

    snprintf(fn, sizeof(fn), "%s.ngs", rom.filename);
    state_store(fn);

    return;
}

BOOL
system_io_rom_read(char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL
system_io_flash_read(_u8* buffer, _u32 len)
{
    char path[256];
    char *homedir;

    if ((homedir=getenv("HOME")) == NULL) {
	fprintf(stderr, "can't find home directory");
	return FALSE;
    }

    /* Build a path to the flash file */
    snprintf(path, sizeof(path), "%s/.neopop/%s.ngf", homedir,
	     rom.filename);

    return read_file_to_buffer(path, buffer, len);
}

BOOL
system_io_flash_write(_u8* buffer, _u32 len)
{
    char path[256];
    char *homedir;
    char flash_dir[256];

    if ((homedir=getenv("HOME")) == NULL) {
	fprintf(stderr, "can't find home directory");
	return FALSE;
    }

    /* Make sure flash dir exists */
    snprintf(flash_dir, sizeof(flash_dir), "%s/.neopop", homedir);
    if (validate_dir(flash_dir) < 0)
	return FALSE;

    /* Build a path for the flash file */
    snprintf(path, sizeof(path), "%s/%s.ngf", flash_dir,
	     rom.filename);

    return write_file_from_buffer(path, buffer, len);
}

BOOL
system_io_state_read(char *filename, _u8 *buffer, _u32 len)
{
    return read_file_to_buffer(filename, buffer, len);
}

BOOL
system_io_state_write(char *filename, _u8 *buffer, _u32 len)
{
    return write_file_from_buffer(filename, buffer, len);
}
