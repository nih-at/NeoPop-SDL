#include "neopop.h"

/* copied from Win32/system_language.c */
typedef struct {
    char label[9];
    char string[256];
} STRING_TAG;

static STRING_TAG string_tags[]={
    { "SDEFAULT",     "Are you sure you want to revert to the default control setup?" }, 
    { "ROMFILT",      "Rom Files (*.ngp,*.ngc,*.npc,*.zip)\0*.ngp;*.ngc;*.npc;*.zip\0\0" }, 
    { "STAFILT",      "State Files (*.ngs)\0*.ngs\0\0" },
    { "FLAFILT",      "Flash Memory Files (*.ngf)\0*.ngf\0\0" },
    { "BADFLASH",     "The flash data for this rom is from a different version of NeoPop, it will be destroyed soon." },
    { "POWER",        "The system has been signalled to power down. You must reset or load a new rom." },
    { "BADSTATE",     "State is from an unsupported version of NeoPop." },
    { "ERROR1",       "An error has occured creating the application window" },
    { "ERROR2",       "An error has occured initialising DirectDraw" },
    { "ERROR3",       "An error has occured initialising DirectInput" },
    { "TIMER",        "This system does not have a high resolution timer." },
    { "WRONGROM",     "This state is from a different rom, Ignoring." },
    { "EROMFIND",     "Cannot find ROM file" },
    { "EROMOPEN",     "Cannot open ROM file" },
    { "EZIPNONE",     "No roms found" } ,
    { "EZIPBAD",      "Corrupted ZIP file" },
    { "EZIPFIND",     "Cannot find ZIP file" }
};

char *
system_get_string(STRINGS string_id)
{
    if (string_id >= STRINGS_MAX)
	return "Unknown String";

    return string_tags[string_id].string;
}
