/* our copyright! */

#include "chunk.h"
#include "sound.h"
#include "interrupt.h"
#include "dma.h"
#include "flash.h"
#include "mem.h"
#include "TLCS900h_registers.h"
#include "Z80_interface.h"

#define HEADER "\0\x60\0\0NGPS"
#define HEADER_SIZE	8

#define SIZE_CHUNK	8
#define SIZE_EOD	0
#define SIZE_RAM	0xC000
#define SIZE_REGS	418
#define SIZE_ROM	(rom.length)
#define SIZE_ROMH	64
#define SIZE_TIME	4

static _u8 read1(const _u8 *);
static _u16 read2(const _u8 *);
static _u32 read4(const _u8 *);
static _u8 *read_chunk_data(FILE *, _u32);
static void read_soundchip(SoundChip *, _u8 **);
static void read_REGS(const _u8 *);

static BOOL write1(FILE *, _u8);
static BOOL write2(FILE *, _u16);
static BOOL write4(FILE *, _u32);
static BOOL write_chunk(FILE *, _u32, const _u8 *, _u32);
static BOOL write_soundchip(FILE *, const SoundChip *);
static BOOL write_FLSH(FILE *, const _u8 *, _u32);
static BOOL write_RAM(FILE *);
static BOOL write_REGS(FILE *);
static BOOL write_ROM(FILE *);
static BOOL write_ROMH(FILE *);
static BOOL write_TIME(FILE *);


BOOL read_chunk(FILE *fp, _u32 *tagp, _u32 *sizep)
{
	_u8 buf[SIZE_CHUNK];
	
	if (fread(buf, 1, SIZE_CHUNK, fp) != SIZE_CHUNK)
		return FALSE;
	
	*tagp = read4(buf);
	*sizep = read4(buf+4);
	
	return TRUE;
}

BOOL read_header(FILE *fp)
{
	_u8 buf[HEADER_SIZE];

	if (fread(buf, 1, HEADER_SIZE, fp) != HEADER_SIZE)
		return FALSE;

	if (memcmp(buf, HEADER, HEADER_SIZE) != 0)
		return FALSE;

	return TRUE;
}

BOOL read_SNAP(FILE *fp, _u32 size)
{
	_u8 *data, *end, *p;
	int got, new, subsize;
	
	if ((data=read_chunk_data(fp, size)) == NULL)
		return FALSE;
	
	got = 0;
	end = data+size;
	for (p=data; p<end; p += subsize+SIZE_CHUNK) {
		subsize = read4(p+4);
		switch (read4(p)) {
		case TAG_FLSH:
			new = OPT_FLSH;
			break;
		case TAG_RAM:
			if (subsize != SIZE_RAM)
				new = -1;
			else
				new = OPT_RAM;
			break;
		case TAG_REGS:
			if (subsize != SIZE_REGS)
				new = -1;
			else
				new = OPT_REGS;
			break;
		case TAG_ROM:
			new = OPT_ROM;
			break;
		case TAG_ROMH:
			if (subsize != SIZE_ROMH)
				new = -1;
			else
				new = OPT_ROMH;
			if (memcmp(rom_header, p+SIZE_CHUNK,
				   sizeof(RomHeader)) != 0) {
				system_message(system_get_string(IDS_WRONGROM));
				free(data);
				return FALSE;
			}
			break;
		case TAG_TIME:
			if (subsize != SIZE_TIME)
				new = -1;
			else
				new = OPT_TIME;
			break;
		default:
			new = 0;
		}
		
		if (new == -1 || (got & new)) {
			/* illegal chunk or duplicate chunk */
			free(data);
			return FALSE;
		}
		got |= new;
	}
	
	if (p != end) {
		/* chunk overruns SNAP chunk */
		free(data);
		return FALSE;
	}
	
	if (((got & (OPT_REGS|OPT_RAM)) != (OPT_REGS|OPT_RAM))
	    || (got & (OPT_ROM|OPT_ROMH)) == (OPT_ROM|OPT_ROMH)) {
		/* missing chunks or ROM and ROMH */
		free(data);
		return FALSE;
	}
	
	
	/* apply state */
	
	reset();
	
	for (p=data; p<end; p += subsize) {
		subsize = read4(p+4);
		p += SIZE_CHUNK;
		switch (read4(p-SIZE_CHUNK)) {
		case TAG_FLSH:
			/* XXX: load flash */
			break;
		case TAG_RAM:
			memcpy(ram, p, SIZE_RAM);
			break;
		case TAG_REGS:
			read_REGS(p);
			break;
		case TAG_ROM:
			/* XXX: load rom */
			break;
		case TAG_ROMH:
			/* handled in verify loop */
			break;
		case TAG_TIME:
			frame_count = read4(p);
			break;
		}
	}
	
	free(data);
	return TRUE;
}


BOOL write_header(FILE *fp)
{
	if (fwrite(HEADER, 1, HEADER_SIZE, fp) != HEADER_SIZE)
		return FALSE;

	return TRUE;
}

BOOL write_EOD(FILE *fp)
{
	return write_chunk(fp, TAG_EOD, NULL, SIZE_EOD);
}

BOOL write_SNAP(FILE *fp, int options)
{
	_u32 size, flash_size;
	_u8 *flash;
	BOOL ret;

	if (options & OPT_ROMH && options & OPT_ROM)
		return FALSE;

	flash = NULL;
	flash_size = 0;
	if (options & OPT_FLSH)
		flash = flash_prepare(&flash_size);
	
	size = SIZE_RAM + SIZE_REGS + SIZE_CHUNK*2;
	if (options & OPT_TIME)
		size += SIZE_TIME + SIZE_CHUNK;
	if (options & OPT_ROM)
		size += SIZE_ROM + SIZE_CHUNK;
	if (options & OPT_ROMH)
		size += SIZE_ROMH + SIZE_CHUNK;
	if (options & OPT_FLSH)
		size += flash_size + SIZE_CHUNK;

	ret = write_chunk(fp, TAG_SNAP, NULL, size);

	if (options & OPT_TIME)
		ret &= write_TIME(fp);
	if (options & OPT_ROM)
		ret &= write_ROM(fp);
	if (options & OPT_ROMH)
		ret &= write_ROMH(fp);
	if (options & OPT_FLSH) {
		ret &= write_FLSH(fp, flash, flash_size);
		free(flash);
	}

	ret &= write_RAM(fp);
	ret &= write_REGS(fp);

	return ret;
}


static _u8 read1(const _u8 *d)
{
	return d[0];
}

static _u16 read2(const _u8 *d)
{
	return (d[0]<<8)|d[1];
}

static _u32 read4(const _u8 *d)
{
	return (d[0]<<24)|(d[1]<<16)|(d[2]<<8)|d[3];
}

static _u8 *read_chunk_data(FILE *fp, _u32 size)
{
	_u8 *data;

	if ((data=malloc(size)) == NULL)
		return NULL;

	if (fread(data, 1, size, fp) != size) {
		free(data);
		return NULL;
	}

	return data;
}

static void read_soundchip(SoundChip *chip, _u8 **pp)
{
	_u8 *p;
	int i;

	p = *pp;
	
	chip->LastRegister = read4(p), p+=4;
	for (i=0; i<8; i++)
		chip->Register[i] = read4(p), p+=4;
	for (i=0; i<4; i++)
		chip->Volume[i] = read4(p), p+=4;
	for (i=0; i<4; i++)
		chip->Period[i] = read4(p), p+=4;
	for (i=0; i<4; i++)
		chip->Count[i] = read4(p), p+=4;
	for (i=0; i<4; i++)
		chip->Output[i] = read4(p), p+=4;
	chip->RNG = read4(p), p+=4;
	chip->NoiseFB = read4(p), p+=4;

	*pp = p;
}

static void read_REGS(const _u8 *p)
{
	int i, j;
	
	pc = read4(p), p+=4;
	sr = read2(p), p+=2;
	for (i=0; i<4; i++)	
		for (j=0; j<4; j++)
			gprBank[i][j] = read4(p), p+=4;
	for (i=0; i<4; i++)
		gpr[i] = read4(p), p+=4;
	f_dash = read1(p), p+=1;
	eepromStatusEnable = read1(p), p+=1;
	Z80_regs.AF.W = read2(p), p+=2;
	Z80_regs.BC.W = read2(p), p+=2;
	Z80_regs.DE.W = read2(p), p+=2;
	Z80_regs.HL.W = read2(p), p+=2;
	Z80_regs.IX.W = read2(p), p+=2;
	Z80_regs.IY.W = read2(p), p+=2;
	Z80_regs.PC.W = read2(p), p+=2;
	Z80_regs.SP.W = read2(p), p+=2;
	Z80_regs.AF1.W = read2(p), p+=2;
	Z80_regs.BC1.W = read2(p), p+=2;
	Z80_regs.DE1.W = read2(p), p+=2;
	Z80_regs.HL1.W = read2(p), p+=2;
	Z80_regs.IFF = read1(p), p+=1;
	Z80_regs.I = read1(p), p+=1;
	Z80_regs.R = read1(p), p+=1;
	Z80_regs.IPeriod = read4(p), p+=4;
	Z80_regs.ICount = read4(p), p+=4;
	Z80_regs.IBackup = read4(p), p+=4;
	Z80_regs.IRequest = read2(p), p+=2;
	Z80_regs.IAutoReset = read1(p), p+=1;
	Z80_regs.TrapBadOps = read1(p), p+=1;
	Z80_regs.Trap = read2(p), p+=2;
	Z80_regs.Trace = read1(p), p+=1;
	timer_hint = read4(p), p+=4;
	for (i=0; i<4; i++)
		timer[i] = read1(p), p+=1;
	timer_clock0 = read4(p), p+=4;
	timer_clock1 = read4(p), p+=4;
	timer_clock2 = read4(p), p+=4;
	timer_clock3 = read4(p), p+=4;
	read_soundchip(&toneChip, &p);
	read_soundchip(&noiseChip, &p);
	for (i=0; i<4; i++)
		dmaS[i] = read4(p), p+=4;
	for (i=0; i<4; i++)
		dmaD[i] = read4(p), p+=4;
	for (i=0; i<4; i++)
		dmaC[i] = read2(p), p+=2;
	for (i=0; i<4; i++)
		dmaM[i] = read1(p), p+=1;
}


static BOOL write1(FILE *fp, _u8 val)
{
	return fputc(val, fp) == 0;
}

static BOOL write2(FILE *fp, _u16 val)
{
	int ret;

	ret = fputc((val>>8) & 0xff, fp) == 0;
	ret &= fputc(val & 0xff, fp) == 0;

	return ret;
}

static BOOL write4(FILE *fp, _u32 val)
{
	int ret;

	ret = fputc((val>>24) & 0xff, fp) == 0;
	ret &= fputc((val>>16) & 0xff, fp) == 0;
	ret &= fputc((val>>8) & 0xff, fp) == 0;
	ret &= fputc(val & 0xff, fp) == 0;

	return ret;
}

static BOOL write_chunk(FILE *fp, _u32 name, const _u8 *data, _u32 size)
{
	int ret;

	ret = write4(fp, name);
	ret &= write4(fp, size);
	if (data && size > 0)
	    ret &= fwrite(data, 1, size, fp) == size;

	return ret;
}

static BOOL write_soundchip(FILE *fp, const SoundChip *chip)
{
	BOOL ret;
	int i;

	ret = write4(fp, chip->LastRegister);
	for (i=0; i<8; i++)
		ret &= write4(fp, chip->Register[i]);
	for (i=0; i<4; i++)
		ret &= write4(fp, chip->Volume[i]);
	for (i=0; i<4; i++)
		ret &= write4(fp, chip->Period[i]);
	for (i=0; i<4; i++)
		ret &= write4(fp, chip->Count[i]);
	for (i=0; i<4; i++)
		ret &= write4(fp, chip->Output[i]);
	ret &= write4(fp, chip->RNG);
	ret &= write4(fp, chip->NoiseFB);

	return ret;
}

static BOOL write_FLSH(FILE *fp, const _u8 *data, _u32 size)
{
	return write_chunk(fp, TAG_FLSH, data, size);
}

static BOOL write_RAM(FILE *fp)
{
	return write_chunk(fp, TAG_RAM, ram, SIZE_RAM);
}

static BOOL write_REGS(FILE *fp)
{
	BOOL ret;
	int i, j;

	ret = write_chunk(fp, TAG_REGS, NULL, SIZE_REGS);
	ret &= write4(fp, pc);
	ret &= write2(fp, sr);
	for (i=0; i<4; i++)	
		for (j=0; j<4; j++)
			ret &= write4(fp, gprBank[i][j]);
	for (i=0; i<4; i++)
		ret &= write4(fp, gpr[i]);
	ret &= write1(fp, f_dash);
	ret &= write1(fp, eepromStatusEnable);
	ret &= write2(fp, Z80_regs.AF.W);
	ret &= write2(fp, Z80_regs.BC.W);
	ret &= write2(fp, Z80_regs.DE.W);
	ret &= write2(fp, Z80_regs.HL.W);
	ret &= write2(fp, Z80_regs.IX.W);
	ret &= write2(fp, Z80_regs.IY.W);
	ret &= write2(fp, Z80_regs.PC.W);
	ret &= write2(fp, Z80_regs.SP.W);
	ret &= write2(fp, Z80_regs.AF1.W);
	ret &= write2(fp, Z80_regs.BC1.W);
	ret &= write2(fp, Z80_regs.DE1.W);
	ret &= write2(fp, Z80_regs.HL1.W);
	ret &= write1(fp, Z80_regs.IFF);
	ret &= write1(fp, Z80_regs.I);
	ret &= write1(fp, Z80_regs.R);
	ret &= write4(fp, Z80_regs.IPeriod);
	ret &= write4(fp, Z80_regs.ICount);
	ret &= write4(fp, Z80_regs.IBackup);
	ret &= write2(fp, Z80_regs.IRequest);
	ret &= write1(fp, Z80_regs.IAutoReset);
	ret &= write1(fp, Z80_regs.TrapBadOps);
	ret &= write2(fp, Z80_regs.Trap);
	ret &= write1(fp, Z80_regs.Trace);
	ret &= write4(fp, timer_hint);
	for (i=0; i<4; i++)
		ret &= write1(fp, timer[i]);
	ret &= write4(fp, timer_clock0);
	ret &= write4(fp, timer_clock1);
	ret &= write4(fp, timer_clock2);
	ret &= write4(fp, timer_clock3);
	ret &= write_soundchip(fp, &toneChip);
	ret &= write_soundchip(fp, &noiseChip);
	for (i=0; i<4; i++)
		ret &= write4(fp, dmaS[i]);
	for (i=0; i<4; i++)
		ret &= write4(fp, dmaD[i]);
	for (i=0; i<4; i++)
		ret &= write2(fp, dmaC[i]);
	for (i=0; i<4; i++)
		ret &= write1(fp, dmaM[i]);

	return ret;
}

static BOOL write_ROM(FILE *fp)
{
	return write_chunk(fp, TAG_ROM, rom.data, SIZE_ROM);
}

static BOOL write_ROMH(FILE *fp)
{
	return write_chunk(fp, TAG_ROMH, (_u8*)rom_header, SIZE_ROMH);
}

static BOOL write_TIME(FILE *fp)
{
	BOOL ret;

	ret = write_chunk(fp, TAG_TIME, NULL, SIZE_TIME);
	ret &= write4(fp, frame_count);

	return ret;
}
