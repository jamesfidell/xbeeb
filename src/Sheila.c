/*
 * Copyright (c) James Fidell 1994.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of the copyright holder
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission. The
 * copyright holder makes no representations about the suitability of
 * this software for any purpose. It is provided "as is" without express
 * or implied warranty.
 *
 * THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */


#include <stdio.h>

#include "Config.h"
#include "Sheila.h"
#include "Crtc.h"
#include "Acia.h"
#include "SerialUla.h"
#include "VideoUla.h"
#include "RomSelect.h"
#include "UserVia.h"
#include "SystemVia.h"
#include "Disk.h"
#include "Econet.h"
#include "ADC.h"
#include "TubeUla.h"


byteval
ReadSheila ( int addr )
{
	/*
	 * 0xfe00 - 0xfe07 : 6845 CRTC
	 */

	if ( addr < 0xfe08 )
		return ReadCrtc ( addr - 0xfe00 );

	/*
	 * 0xfe08 - 0xfe0f : 6850 ACIA
	 */

	if ( addr < 0xfe10 )
		return ReadAcia ( addr - 0xfe08 );

	/*
	 * 0xfe10 - 0xfe1f : Serial ULA
	 */

	if ( addr < 0xfe20 )
		return ReadSerialUla ( addr - 0xfe10 );

	/*
	 * 0xfe20 - 0xfe2f : Video ULA
	 */

	if ( addr < 0xfe30 )
		return ReadVideoUla ( addr - 0xfe20 );

	/*
	 * 0xfe30 - 0xfe3f : Paged ROM selector
	 */

	if ( addr < 0xfe40 )
		return ReadPagedRomSelector ( addr - 0xfe30 );

	/*
	 * 0xfe40 - 0xfe5f : System VIA
	 */

	if ( addr < 0xfe60 )
		return ReadSystemVia ( addr - 0xfe40 );

	/*
	 * 0xfe60 - 0xfe7f : User VIA
	 */

	if ( addr < 0xfe80 )
		return ReadUserVia ( addr - 0xfe60 );

	/*
	 * 0xfe80 - 0xfe9f : Disk Controller
	 */

	if ( addr < 0xfea0 )
		return ReadDiskController ( addr - 0xfe80 );

	/*
	 * 0xfea0 - 0xfebf : Econet Controller
	 */

	if ( addr < 0xfec0 )
		return ReadEconetController ( addr - 0xfea0 );

	/*
	 * 0xfec0 - 0xfedf : Analogue to Digital Converter
	 */

	if ( addr < 0xfee0 )
		return ReadADConverter ( addr - 0xfec0 );

	/*
	 * 0xfee0 - 0xfeff : Tube ULA
	 */

	return ReadTubeUla ( addr - 0xfee0 );
}


void
WriteSheila ( int addr, byteval val )
{
	/*
	 * 0xfe00 - 0xfe07 : 6845 CRTC
	 */

	if ( addr < 0xfe08 )
	{
		WriteCrtc ( addr - 0xfe00, val );
		return;
	}

	/*
	 * 0xfe08 - 0xfe0f : 6850 ACIA
	 */

	if ( addr < 0xfe10 )
	{
		WriteAcia ( addr - 0xfe08, val );
		return;
	}

	/*
	 * 0xfe10 - 0xfe1f : Serial ULA
	 */

	if ( addr < 0xfe20 )
	{
		WriteSerialUla ( addr - 0xfe10, val );
		return;
	}

	/*
	 * 0xfe20 - 0xfe2f : Video ULA
	 */

	if ( addr < 0xfe30 )
	{
		WriteVideoUla ( addr - 0xfe20, val );
		return;
	}

	/*
	 * 0xfe30 - 0xfe3f : Paged ROM selector
	 */

	if ( addr < 0xfe40 )
	{
		WritePagedRomSelector ( addr - 0xfe30, val );
		return;
	}

	/*
	 * 0xfe40 - 0xfe5f : System VIA
	 */

	if ( addr < 0xfe60 )
	{
		WriteSystemVia ( addr - 0xfe40, val );
		return;
	}

	/*
	 * 0xfe60 - 0xfe7f : User VIA
	 */

	if ( addr < 0xfe80 )
	{
		WriteUserVia ( addr - 0xfe60, val );
		return;
	}

	/*
	 * 0xfe80 - 0xfe9f : Disk Controller
	 */

	if ( addr < 0xfea0 )
	{
		WriteDiskController ( addr - 0xfe80, val );
		return;
	}

	/*
	 * 0xfea0 - 0xfebf : Econet Controller
	 */

	if ( addr < 0xfec0 )
	{
		WriteEconetController ( addr - 0xfea0, val );
		return;
	}

	/*
	 * 0xfec0 - 0xfedf : Analogue to Digital Converter
	 */

	if ( addr < 0xfee0 )
	{
		WriteADConverter ( addr - 0xfec0, val );
		return;
	}

	/*
	 * 0xfee0 - 0xfeff : Tube ULA
	 */

	WriteTubeUla ( addr - 0xfee0, val );
	return;
}
