/* our copyright! */

#include "chunk.h"
#include "sound.h"
#include "interrupt.h"
#include "dma.h"
#include "flash.h"
#include "mem.h"
#include "TLCS900h_registers.h"
#include "Z80_interface.h"

#define SIZE_CHUNK	8
#define SIZE_EOD	0
#define SIZE_RAM	0xC000
#define SIZE_REGS	418
#define SIZE_ROM	(rom.length)
#define SIZE_ROMH	64
#define SIZE_TIME	4

BOOL write_TIME(FILE *);

BOOL write1(FILE *fp, _u8 val)
{
	return fputc(val, fp) == 0;
}

BOOL write2(FILE *fp, _u16 val)
{
	int ret;

	ret = fputc((val>>8) & 0xff, fp) == 0;
	ret &= fputc(val & 0xff, fp) == 0;

	return ret;
}

BOOL write4(FILE *fp, _u32 val)
{
	int ret;

	ret = fputc((val>>24) & 0xff, fp) == 0;
	ret &= fputc((val>>16) & 0xff, fp) == 0;
	ret &= fputc((val>>8) & 0xff, fp) == 0;
	ret &= fputc(val & 0xff, fp) == 0;

	return ret;
}

BOOL write_chunk(FILE *fp, _u32 name, _u32 size)
{
	int ret;

	ret = write4(fp, name);
	ret &= write4(fp, size);

	return ret;
}

BOOL write_header(FILE *fp)
{
	if (fwrite("\0\x60\0\0NGPS", 1, 8, fp) != 8)
		return FALSE;

	return TRUE;
}

BOOL write_soundchip(FILE *fp, SoundChip *chip)
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

BOOL write_EOD(FILE *fp)
{
	return write_chunk(fp, TAG_EOD, SIZE_EOD);
}

BOOL write_FLSH(FILE *fp, _u8 *data, _u32 size)
{
	BOOL ret;

	ret = write_chunk(fp, TAG_FLSH, size);

	ret &= fwrite(data, 1, size, fp) == size;

	return ret;
}

BOOL write_RAM(FILE *fp)
{
	BOOL ret;

	ret = write_chunk(fp, TAG_RAM, SIZE_RAM);

	if (fwrite(ram, 1, SIZE_RAM, fp) != SIZE_RAM)
		ret = FALSE;

	return ret;
}

BOOL write_REGS(FILE *fp)
{
	BOOL ret;
	int i, j;

	ret = write_chunk(fp, TAG_REGS, SIZE_REGS);
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

BOOL write_ROM(FILE *fp)
{
	BOOL ret;

	ret = write_chunk(fp, TAG_ROM, SIZE_ROM);

	if (fwrite(rom.data, 1, rom.length, fp) != rom.length)
		ret = FALSE;

	return ret;
}

BOOL write_ROMH(FILE *fp)
{
	BOOL ret;

	ret = write_chunk(fp, TAG_ROMH, SIZE_ROMH);
	if (fwrite(rom_header, 1, sizeof(RomHeader), fp) != sizeof(RomHeader))
		ret = FALSE;

	return ret;
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

	ret = write_chunk(fp, TAG_SNAP, size);

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

BOOL write_TIME(FILE *fp)
{
	BOOL ret;

	ret = write_chunk(fp, TAG_TIME, SIZE_TIME);
	ret &= write4(fp, frame_count);

	return ret;
}
