AM_CPPFLAGS=	-I$(top_srcdir)/Core/z80  -I$(top_srcdir)/Core/TLCS-900h \
		-I$(top_srcdir)/Core
AM_CFLAGS=	-D__cdecl= @SDL_CFLAGS@ -Wall
LDADD=		@SDL_LIBS@

# GNU make support
VPATH=$(top_srcdir)/Core:$(top_srcdir)/Core/TLCS-900h:$(top_srcdir)/Core/z80:$(top_srcdir)/System_SDL

# BSD make support
.PATH=${VPATH}

bin_PROGRAMS=NeoPop-SDL
noinst_HEADERS=NeoPop-SDL.h system_hq2x.h system_hq3x.h system_hq_intern.h \
	unzip.h font.xpm
EXTRA_DIST=	mknames.sh

nodist_NeoPop_SDL_SOURCES= bios.c biosHLE.c chunk.c dma.c flash.c gfx.c \
	gfx_scanline_mono.c gfx_scanline_colour.c interrupt.c mem.c neopop.c \
	rom.c sound.c state.c Z80_interface.c \
	TLCS900h_disassemble_extra.c TLCS900h_disassemble_reg.c \
	TLCS900h_interpret_single.c TLCS900h_disassemble_src.c \
	TLCS900h_interpret.c TLCS900h_registers.c TLCS900h_interpret_reg.c \
	TLCS900h_disassemble.c TLCS900h_interpret_src.c \
	TLCS900h_interpret_dst.c TLCS900h_disassemble_dst.c \
	dasm.c Z80.c
NeoPop_SDL_SOURCES=system_bindings.c system_comms.c system_graphics.c \
	system_hq2x.c system_hq3x.c \
	system_input.c system_io.c system_language.c system_video.c \
	system_main.c system_names.c system_osd.c system_rc.c system_rom.c \
	system_screenshot.c system_sound.c unzip.c
DISTCLEANFILES=system_names.c

system_names.c: mknames.sh NeoPop-SDL.h
	sh ${srcdir}/mknames.sh ${srcdir}/NeoPop-SDL.h > $@.$$$$ && \
		mv $@.$$$$ $@
