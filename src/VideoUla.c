/*
 *
 * $Id: VideoUla.c,v 1.15 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 1994-2002.
 *
 * Permission to use, copy, modify and distribute this software
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
 * Revision 1.15  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.14  2000/08/16 17:58:29  james
 * Update copyright message
 *
 * Revision 1.13  1996/11/17 23:19:39  james
 * Only update the screen on a flashing colour swap if there are flashing
 * colours in the colour map.  From an idea from David Ralph Stacey.
 *
 * Revision 1.12  1996/11/17 22:43:43  james
 * Convert the DecodeColour function to use lookup tables.  (Code from
 * David Ralph Stacey.)  Much faster, but eats memory :-)
 *
 * Revision 1.11  1996/11/09 23:02:32  james
 * Take account of the horizontal start of display based on the clock rate
 * set in the Video ULA and the CRTC Horiz. Sync. register.
 *
 * Revision 1.10  1996/10/13 17:23:38  james
 * Renamed Register0 to VidRegister0 because of potential confusion with the
 * Serial ULA Register 0
 *
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
unsigned char				ClockRate;

static	unsigned char		FlashColour = 0;
static	unsigned char		FlashColourCount;
static	unsigned char		Teletext = 0;

static	byteval				ColourMap [ 16 ];
static	byteval				VidRegister0;

static	byteval				oldval = 0xff;

static	byteval				FlashMap [ 16 ] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7
};

static	byteval				TwoColourTable [ 256 * 8 ];
static	byteval				FourColourTable [ 256 * 4 ];
static	byteval				SixteenColourTable [ 256 * 2 ];


void
ResetVideoUla()
{
	/*
	 * FIX ME
	 *
	 * I don't know what happens to this chip on power-up/reset
	 */

	int			i, j;
	byteval		bits;

	/*
	 * Set up the colour map tables...
	 */

	for ( i = 0; i < 256; i++ )
	{
		for ( j = 0; j < 8; j++ )
			TwoColourTable [ i << 3 | j ] = (( i >> ( 7 - j )) & 0x1 ) * 15;

		for ( j = 0; j < 4; j++ )
		{
			bits = (( i >> ( 3 - j )) & 0x11 );
			switch ( bits )
			{
				case 0x00:
					FourColourTable [ i << 2 | j ] = 0;
					break;
				case 0x01:
					FourColourTable [ i << 2 | j ] =  2;
					break;
				case 0x10:
					FourColourTable [ i << 2 | j ] =  8;
					break;
				case 0x11:
					FourColourTable [ i << 2 | j ] = 10;
					break;
			}
		}

		for ( j = 0; j < 2; j ++ )
		{
			bits = (( i >> ( 1 - j )) & 0x55 );
			switch ( bits )
			{
				case 0x55:
					SixteenColourTable [ i << 1 | j ] = 15;
					break;
				case 0x54:
					SixteenColourTable [ i << 1 | j ] = 14;
					break;
				case 0x51:
					SixteenColourTable [ i << 1 | j ] = 13;
					break;
				case 0x50:
					SixteenColourTable [ i << 1 | j ] = 12;
					break;
				case 0x45:
					SixteenColourTable [ i << 1 | j ] = 11;
					break;
				case 0x44:
					SixteenColourTable [ i << 1 | j ] = 10;
					break;
				case 0x41:
					SixteenColourTable [ i << 1 | j ] =  9;
					break;
				case 0x40:
					SixteenColourTable [ i << 1 | j ] =  8;
					break;
				case 0x15:
					SixteenColourTable [ i << 1 | j ] =  7;
					break;
				case 0x14:
					SixteenColourTable [ i << 1 | j ] =  6;
					break;
				case 0x11:
					SixteenColourTable [ i << 1 | j ] =  5;
					break;
				case 0x10:
					SixteenColourTable [ i << 1 | j ] =  4;
					break;
				case 0x05:
					SixteenColourTable [ i << 1 | j ] =  3;
					break;
				case 0x04:
					SixteenColourTable [ i << 1 | j ] =  2;
					break;
				case 0x01:
					SixteenColourTable [ i << 1 | j ] =  1;
					break;
				case 0x00:
					SixteenColourTable [ i << 1 | j ] =  0;
					break;
			}
		}
	}

	FlashColourCount = 0;

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
			/*
			 * Update the number of flashing colours that are in the colour
			 * map
			 */

			if (( ColourMap [ logical ] ^ physical ) & 0x8 )
			{
				if ( ColourMap [ logical ] < 0x8 )
					FlashColourCount++;
				else
					FlashColourCount--;
			}

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

		VidRegister0 = val;

		/*
		 * FIX ME
		 *
		 * Hmmm.  This only has relevance for bitmapped modes.  Do we want
		 * to enforce that ?
		 */

		if ( FlashColour != ( VidRegister0 & 0x01 ))
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

			/*
			 * Only redraw the screen because of the flash colour swap
			 * if we actually have physical flashing colours in the
			 * colour map.
			 */

			if ( FlashColourCount )
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
		Teletext = VidRegister0 & 0x02;
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
		 *
		 * FIX ME
		 *
		 * Where do the numbers come from here ?
		 */

		ClockRate = VidRegister0 & 0x10;
		if ( ClockRate )
			StartPosnX = ( 98 - HorizSync ) * 8;
		else
			StartPosnX = ( 49 - HorizSync ) * 16;

		/*
		 * PixelWidth is the number of pixels in our real display that
		 * each Beeb screen pixel takes up.
		 */

		switch ( VidRegister0 & 0x0c )
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

		switch ( VidRegister0 & 0x60 )
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

		MasterCursorWidth = VidRegister0 >> 7;

		if ( ocbw != CursorByteWidth )
			CursorResized = 1;

		if ( omcw != MasterCursorWidth )
		{
			CursorResized = 1;
			CursorMoved = 1;
		}

		if (( oldval & 0xfe ) != ( VidRegister0 & 0xfe ))
		{
#ifdef	INFO
			switch ( VidRegister0 )
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
		oldval = VidRegister0;
	}
	return;
}


byteval
DecodeColour ( byteval info, byteval pixel )
{
	byteval			col = 0;

	/*
	 * FIX ME
	 *
	 * I'm sure a lookup table would be faster than this.
	 */

	switch ( BitsForColourInfo )
	{
		case 1 :
			col = TwoColourTable [ ( info << 3 ) | pixel ];
			break;
		case 2 :
			col = FourColourTable [ ( info << 2 ) | pixel ];
			break;
		case 4 :
			col = SixteenColourTable [ ( info << 1 ) | pixel ];
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
	vid [ 16 ] = VidRegister0;

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

	VidRegister0 = vid [ 16 ];
	oldval = VidRegister0;

	FlashColour = VidRegister0 & 0x01;
	Teletext = VidRegister0 & 0x02;
	ClockRate = VidRegister0 & 0x10;
	switch ( VidRegister0 & 0x0c )
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

	switch ( VidRegister0 & 0x60 )
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

	MasterCursorWidth = VidRegister0 >> 7;

	if ( Teletext )
		InitialiseTeletext();
	else
		InitialiseBitmap();

	return 0;
}
