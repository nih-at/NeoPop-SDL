/* $NiH: system_comms.c,v 1.4 2003/10/15 12:30:02 wiz Exp $ */
/*
  system_comms.c -- comm port support functions
  Copyright (C) 2002-2003 Thomas Klausner

  This file is part of NeoPop-SDL, a NeoGeo Pocket emulator
  The author can be contacted at <wiz@danbala.tuwien.ac.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "neopop.h"

BOOL
system_comms_poll(_u8* buffer)
{
    return FALSE;
}

BOOL
system_comms_read(_u8* buffer)
{
    return FALSE;
}

void
system_comms_write(_u8 data)
{
    return;
}
