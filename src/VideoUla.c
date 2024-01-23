/*
 *
 * $Id: VideoUla.c,v 1.9 1996/10/09 23:43:49 james Exp $
 *
 * Copyright (c) James Fidell 1994, 1995, 1996.
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
 * $Log: VideoUla.c,v $
 * Revision 1.9  1996/10/09 23:43:49  james
 * Sort out Video ULA register wrap around.  Also tidy up setting of
 * BitsForColourInfo.
 *
 * Revision 1.8  1996/10/09 22:06:58  james
 * Overhaul of the bitmapped screen handling code with particular respect to
 * colour maps.
 *
 * Revision 1.7  1996/10/08 23:05:33  james
 * Corrections to allow clean compilation under GCC 2.7.2 with -Wall -pedantic
 *
 * Revision 1.6  1996/10/08 00:04:36  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.5  1996/10/01 00:33:07  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.4  1996/09/24 23:05:46  james
 * Update copyright dates.
 *
 * Revision 1.3  1996/09/23 16:09:53  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.2  1996/09/21 22:13:53  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:43  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <memory.h>

#include "Config.h"
#include "VideoUla.h"
#include "Beeb.h"
#include "Memory.h"
#include "Crtc.h"
#include "Modes.h"
#include "Bitmap.h"
#include "Screen.h"


unsigned char				CharsPerLine;
unsigned char				PixelWidth;
unsigned char				ByteWidth;
unsigned char				PixelsPerByte;
unsigned char				CursorByteWidth = 0;
unsigned char				MasterCursorWidth = 0;

static	unsigned char		FlashColour = 0;
static	unsigned char		Teletext = 0;
static	unsigned char		ClockRate;

static	byteval				ColourMap [ 16 ];
static	byteval				Register0;

static	byteval				oldval = 0xff;

static	byteval				FlashMap [ 16 ] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7
};


void
ResetVideoUla()
{
	/*
	 * FIX ME
	 *
	 * I don't know what happens to this chip on power-up/reset
	 */

	return;
}


byteval
ReadVideoUla ( int addr )
{
	/*
	 * FIX ME
	 *
	 * I'm not even sure that these values are right -- they're just
	 * what are returned when these locations are written on my Beeb.
	 * The registers are both supposed to be read-only anyway.
	 */

	/*
	 * Wrap-around on the addresses means that there are only two locations
	 * to worry about here.  In fact, they both appear to return the
	 * same value when read, so it's all very simple...
	 */

	return 0xfe;
}


void
WriteVideoUla ( int addr, byteval val )
{
	/*
	 * Wrap around on the addresses means that we only have to worry
	 * about the odd and even addresses here (there are only two registers
	 * on the Video ULA
	 */

	if ( addr & 0x1 )
	{
		/*
		 * For some reason, the physical colour is XORed with 0x7
		 * when it's written to the palette.
		 */

		/*
		 * FIX ME
		 *
		 * The values set for logical colours are dependent on the
		 * number of colours being used in each mode.  (AUG, p380).
		 * All possibilities need to be set for non 16-colour modes
		 * and strange things happen if you don't do the job properly.
		 * These strange effects are not emulated.  I need to look
		 * much more closely at how they tie into the screen layout.
		 */

		byteval		logical, physical;

		logical = val >> 4;
		physical = ( val & 0x0f ) ^ 0x07;

		if ( ColourMap [ logical ] != physical )
		{
			ColourMap [ logical ] = physical;

			/*
			 * FIX ME
			 *
			 * This is really icky -- update the whole screen just for a
			 * colour map change! ?
			 */

			( void ) memset (( void* ) ScreenCheck, 1, 32768 );
		}
#ifdef	INFO
		printf ( "Video ULA palette colour %d = %d\n", val>>4, physical );
#endif
	}
	else
	{
		unsigned char		ocbw, omcw, ot;

		Register0 = val;

		if ( FlashColour != ( Register0 & 0x01 ))
		{
			register int		i;

			FlashColour ^= 0x01;
			for ( i = 8; i < 16; i++ )
				FlashMap [ i ] = FlashMap [ i ] ^ 0x07;

			/*
			 * FIX ME
			 *
			 * This is really icky -- update the whole screen just for a
			 * flash colour change! ?
			 */

			( void ) memset (( void* ) ScreenCheck, 1, 32768 );
		}

		/*
		 * This bit is essentially what makes the difference between
		 * MODE 7 and all the others.  It determines whether charaters
		 * come from the bitmapped screen or from the SA5050 teletext
		 * character generator.
		 *
		 * Now, if I was a lot smarter than the average bear, I'd
		 * emulate the SA5050 and not have fonts at all -- just do
		 * that emulation of the CRTC in the display scanning across
		 * the screen.
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
		 * As far as I can see, this has no effect when RGB input comes
		 * from the teletext chip
		 *
		 * In bitmapped mode, it appears to stretch the characters
		 * somehow, so that they fit across the whole screen.  I'm not
		 * exactly sure how the stretching works, however.
		 */

		/*
		 * FIX ME
		 *
		 * When the clock rate changes, we also need to change the position
		 * across the screen where the display starts.
		 */

		ClockRate = Register0 & 0x10;

		/*
		 * PixelWidth is the number of pixels in our real display that
		 * each Beeb screen pixel takes up.
		 */

		switch ( Register0 & 0x0c )
		{
			case 0x00 :
				CharsPerLine = 10;
				PixelWidth = 8;
				BitsForColourInfo = ClockRate ? 8 : 4;
				break;
			case 0x04 :
				CharsPerLine = 20;
				PixelWidth = 4;
				BitsForColourInfo = ClockRate ? 4 : 2;
				break;
			case 0x08 :
				CharsPerLine = 40;
				PixelWidth = 2;
				BitsForColourInfo = ClockRate ? 2 : 1;
				break;
			case 0x0c :
				CharsPerLine = 80;
				PixelWidth = 1;
				BitsForColourInfo = ClockRate ? 1 : 4;
				break;
		}		
		PixelsPerByte = 8 / BitsForColourInfo;
		ByteWidth = PixelWidth * PixelsPerByte;

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

		if (( oldval & 0xfe ) != ( Register0 & 0xfe ))
		{
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

			if ( Teletext )
				InitialiseTeletext();
			else
				InitialiseBitmap();
		}
		oldval = Register0;
	}
	return;
}


byteval
DecodeColour ( byteval info, byteval pixel )
{
	byteval			bits, col = 0;

	/*
	 * FIX ME
	 *
	 * I'm sure a lookup table would be faster than this.
	 */

	switch ( BitsForColourInfo )
	{
		case 1 :
			col = (( info >> ( 7 - pixel )) & 0x1 ) * 15;
			break;
		case 2 :
			bits = ( info >> ( 3 - pixel )) & 0x11;
			switch ( bits )
			{
				case 0x11 :
					col = 10;
					break;
				case 0x10 :
					col = 8;
					break;
				case 0x01 :
					col = 2;
					break;
				case 0x00 :
					col = 0;
					break;
			}
			break;
		case 4 :
			bits = ( info >> ( 1 - pixel )) & 0x55;
			switch ( bits )
			{
				case 0x55 :
					col = 15;
					break;
				case 0x54 :
					col = 14;
					break;
				case 0x51 :
					col = 13;
					break;
				case 0x50 :
					col = 12;
					break;
				case 0x45 :
					col = 11;
					break;
				case 0x44 :
					col = 10;
					break;
				case 0x41 :
					col = 9;
					break;
				case 0x40 :
					col = 8;
					break;
				case 0x15 :
					col = 7;
					break;
				case 0x14 :
					col = 6;
					break;
				case 0x11 :
					col = 5;
					break;
				case 0x10 :
					col = 4;
					break;
				case 0x05 :
					col = 3;
					break;
				case 0x04 :
					col = 2;
					break;
				case 0x01 :
					col = 1;
					break;
				case 0x00 :
					col = 0;
					break;
			}
			break;
		default :
			/*
			 * FIX ME
			 *
			 * Should do something very nasty here...
			 *
			 * Should also exit cleanly, since we've almost certainly
			 * started up an X window and buggered about with the
			 * server configuration by now.
			 *
			 */
			fprintf ( stderr, "unrecognised number bits per colour\n" );
			FatalError();
			break;
	}

	/*
	 * Now look up that logical colour in the colour map to find out what
	 * physical colour it is, then look that up to find out which (if any)
	 * of the pair of flashing colours should be showing on the screen.
	 */

	return FlashMap [ ColourMap [ col ]];
}


int
SaveVideoUla ( int fd )
{
	byteval			vid [ 32 ];

	/*
	 * FIX ME
	 *
	 * Need to save the FlashMap here, too
	 */

	memcpy ( vid, ColourMap, 16 );
	vid [ 16 ] = Register0;

	if ( write ( fd, vid, 32 ) != 32 )
		return -1;

	return 0;
}


int
RestoreVideoUla ( int fd, unsigned int ver )
{
	byteval			vid [ 32 ];

	if ( ver > 1 )
		return -1;

	/*
	 * FIX ME
	 *
	 * Need to restore the FlashMap here, too
	 */

	if ( read ( fd, vid, 32 ) != 32 )
		return -1;

	/*
	 * FIX ME
	 *
	 * The colour map should be restored here...
	 */

	memcpy ( ColourMap, vid, 16 );

	Register0 = vid [ 16 ];
	oldval = Register0;

	FlashColour = Register0 & 0x01;
	Teletext = Register0 & 0x02;
	ClockRate = Register0 & 0x10;
	switch ( Register0 & 0x0c )
	{
		case 0x00 :
			CharsPerLine = 10;
			PixelWidth = 8;
			BitsForColourInfo = ClockRate ? 8 : 4;
			break;
		case 0x04 :
			CharsPerLine = 20;
			PixelWidth = 4;
			BitsForColourInfo = ClockRate ? 4 : 2;
			break;
		case 0x08 :
			CharsPerLine = 40;
			PixelWidth = 2;
			BitsForColourInfo = ClockRate ? 2 : 1;
			break;
		case 0x0c :
			CharsPerLine = 80;
			PixelWidth = 1;
			BitsForColourInfo = ClockRate ? 1 : 4;
			break;
	}		
	PixelsPerByte = 8 / BitsForColourInfo;
	ByteWidth = PixelWidth * PixelsPerByte;

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
		InitialiseBitmap();

	return 0;
}
