CC?=	gcc
CFLAGS=-g
CFLAGS+=	`sdl-config --cflags` -I../Core/z80  -I../Core/TLCS-900h \
		-I../Core -D__cdecl=
LDFLAGS+=	`sdl-config --libs`

# gmake support
VPATH=../Core:../Core/TLCS-900h:../Core/z80

# BSD make support
.PATH=${VPATH}
.SUFFIXES: .c .o

FILES=	bios.c biosHLE.c dma.c flash.c gfx.c gfx_scanline_mono.c \
	gfx_scanline_colour.c interrupt.c mem.c neopop.c rom.c sound.c \
	state.c Z80_interface.c \
	TLCS900h_disassemble_extra.c TLCS900h_disassemble_reg.c \
	TLCS900h_interpret_single.c TLCS900h_disassemble_src.c \
	TLCS900h_interpret.c TLCS900h_registers.c TLCS900h_interpret_reg.c \
	TLCS900h_disassemble.c TLCS900h_interpret_src.c \
	TLCS900h_interpret_dst.c TLCS900h_disassemble_dst.c \
	dasm.c Z80.c \
	system_comms.c system_graphics.c system_io.c system_language.c \
	system_main.c system_rom.c system_sound.c

.c.o:
	${CC} ${CFLAGS} -c $<

neopop: ${FILES:.c=.o}
	${CC} ${CFLAGS} ${LDFLAGS} -o neopop ${FILES:.c=.o}

clean:
	rm -f *.core *.o neopop
