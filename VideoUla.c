/*
 *
 * $Id$
 *
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

/*
 * Modification History
 *
 * $Log$
 *
 */


#include <stdio.h>
#include <unistd.h>

#include "Config.h"
#include "VideoUla.h"
#include "Crtc.h"
#include "Modes.h"

unsigned char				CharsPerLine;
unsigned char				CursorByteWidth = 0;
unsigned char				MasterCursorWidth = 0;

static	unsigned char		FlashColour;
static	unsigned char		Teletext = 0;
static	unsigned char		ClockRate;

static	unsigned char		ColourMap [ 16 ];
static	unsigned char		Register0;

static	unsigned char		oldval = 0xff;

unsigned char
ReadVideoUla ( int addr )
{
	/*
	 * FIX ME
	 */

	fprintf ( stderr, "Attempt to read video ULA (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	exit ( 1 );
	return 0;
}


void
WriteVideoUla ( int addr, unsigned char val )
{
	unsigned char		ocbw, omcw, ot;

	switch ( addr )
	{
		case 0x0 :
			Register0 = val;
			/*
			 * FIX ME
			 *
			 * When this bit gets changed the flashing colours in the
			 * palette need to be altered.
			 */

			FlashColour = Register0 & 0x01;

			/*
			 * FIX ME
			 *
			 * This bit is essentially what makes the difference between
			 * MODE 7 and all the others.  It determines whether charaters
			 * come from the bitmapped screen or from the SA5050 teletext
			 * character generator.
			 *
			 * At the moment, non-teletext MODEs are not handled at all.
			 */

			ot = Teletext;
			Teletext = Register0 & 0x02;
#ifdef	INFO
			if ( Teletext )
				printf ( "teletext mode selected\n" );
#endif

			/*
			 * FIX ME
			 *
			 * As far as I can see, this has no effect when in teletext
			 * MODE.
			 *
			 * In bitmapped mode, it appears to stretch the characters
			 * somehow, so that they fit across the whole screen.  I'm not
			 * exactly sure how the stretching works, however.
			 */

			switch ( Register0 & 0x0c )
			{
				case 0x00 :
					CharsPerLine = 10;
					break;
				case 0x04 :
					CharsPerLine = 20;
					break;
				case 0x08 :
					CharsPerLine = 40;
					break;
				case 0x0c :
					CharsPerLine = 80;
					break;
			}

			/*
			 * I'm sure changing this will cause something horrible to go
			 * wrong, but it's probably so horrible that the screen will
			 * completely collapse...
			 *
			 * Until I find otherwise, I will always ignore this...
			 */

			ClockRate = Register0 & 0x10;

			/*
			 * FIX ME
			 *
			 * A cursor width value of 0x01 is undefined, so I leave it
			 * to do nothing.
			 *
			 * The Master Cursor Width and Cursor Width in Bytes values
			 * interact somehow, but it's not clear to me at present
			 * exactly how.
			 */

			ocbw = CursorByteWidth;
			omcw = MasterCursorWidth;

			switch ( Register0 & 0x60 )
			{
				case 0x00 :
					CursorByteWidth = 1;
					break;
				case 0x20 :
					break;
				case 0x40 :
					CursorByteWidth = 2;
					break;
				case 0x60 :
					CursorByteWidth = 4;
					break;
			}

			MasterCursorWidth = Register0 >> 7;

			if ( ocbw != CursorByteWidth )
				CursorResized = 1;

			if ( omcw != MasterCursorWidth )
			{
				CursorResized = 1;
				CursorMoved = 1;
			}

#ifdef	INFO
			switch ( Register0 )
			{
				case 0x9c : case 0x9d :
					printf ( "mode 0 or 3 selected\n" );
					break;
				case 0xd8 : case 0xd9 :
					printf ( "mode 1 selected\n" );
					break;
				case 0xf4 : case 0xf5 :
					printf ( "mode 2 selected\n" );
					break;
				case 0x88 : case 0x89 :
					printf ( "mode 4 or 6 selected\n" );
					break;
				case 0xc4 : case 0xc5 :
					printf ( "mode 5 selected\n" );
					break;
				case 0x4b : case 0x4a :
					printf ( "mode 7 selected\n" );
					break;
				default :
					printf ( "strange video ULA CR settings 0x%x\n", val );
					break;
			}
#endif

			if (( oldval & 0xfe ) != ( Register0 & 0xfe ))
			{
				if ( ot != Teletext )
					InitialiseTeletext();
				else
					if ( !Teletext )
					{
						fprintf ( stderr, "Unimplemented MODE request\n" );
						exit ( 1 );
					}
			}
			oldval = Register0;
			break;

		case 0x01 :
			/*
			 * FIX ME
			 *
			 * The colourmap will need to be changed when this register
			 * is written...
			 */

			ColourMap [ val >> 4 ] = val & 0xf;
#ifdef	INFO
			printf ( "Video ULA palette colour %x set\n", val >> 4 );
#endif
			break;

		default :
			fprintf ( stderr, "Illegal write video ULA (addr = %x)\n", addr );
			break;
	}
	return;
}


int
SaveVideoUla ( int fd )
{
	unsigned char			vid [ 32 ];

	memcpy ( vid, ColourMap, 16 );
	vid [ 16 ] = Register0;

	if ( write ( fd, vid, 32 ) != 32 )
		return -1;

	return 0;
}


int
RestoreVideoUla ( int fd, unsigned int ver )
{
	unsigned char			vid [ 32 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, vid, 32 ) != 32 )
		return -1;

	memcpy ( ColourMap, vid, 16 );

	Register0 = vid [ 16 ];
	oldval = Register0;

	FlashColour = Register0 & 0x01;
	Teletext = Register0 & 0x02;
	switch ( Register0 & 0x0c )
	{
		case 0x00 :
			CharsPerLine = 10;
			break;
		case 0x04 :
			CharsPerLine = 20;
			break;
		case 0x08 :
			CharsPerLine = 40;
			break;
		case 0x0c :
			CharsPerLine = 80;
			break;
	}
	ClockRate = Register0 & 0x10;
	switch ( Register0 & 0x60 )
	{
		case 0x00 :
			CursorByteWidth = 1;
			break;
		case 0x20 :
			break;
		case 0x40 :
			CursorByteWidth = 2;
			break;
		case 0x60 :
			CursorByteWidth = 4;
			break;
	}
	MasterCursorWidth = Register0 >> 7;

	if ( Teletext )
		InitialiseTeletext();
	else
	{
		fprintf ( stderr, "Unimplemented MODE in RestoreVideoUla\n" );
		exit ( 1 );
	}

	return 0;
}
