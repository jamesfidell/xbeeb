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
#include <unistd.h>
#include <X11/Xlib.h>

#include "Config.h"
#include "VideoUla.h"
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

byteval
ReadVideoUla ( int addr )
{
	/*
	 * FIX ME
	 *
	 * I'm not even sure that this is a valid thing to do -- the AUG
	 * says that both vULA registers are read-only.
	 */

	fprintf ( stderr, "Attempt to read video ULA (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	FatalError();
	return 0;
}


void
WriteVideoUla ( int addr, byteval val )
{
	unsigned char		ocbw, omcw, ot;

	switch ( addr )
	{
		case 0x0 :
			Register0 = val;

			if ( FlashColour != Register0 & 0x01 )
			{
				register int		i, c;
				XColor				colour;

				FlashColour ^= 0x01;
				for ( i = 0; i < 16; i++ )
				{
					if ( ColourMap [ i ] & 0x8 )
					{
						Cells [ i ] = ColourBits | ( i & 1 ? Masks [ 0 ] : 0) |
												( i & 2 ? Masks [ 1 ] : 0 ) |
												( i & 4 ? Masks [ 2 ] : 0 ) |
												( i & 8 ? Masks [ 3 ] : 0 );
						colour.pixel = Cells [ i ];
						c = ( ColourMap [ i ] & 0x7 ) ^
												( FlashColour ? 0x7 : 0x0);
						colour.red = RgbValues [ c ][ 0 ];
						colour.green = RgbValues [ c ][ 1 ];
						colour.blue = RgbValues [ c ][ 2 ];
						colour.flags = DoRed | DoGreen | DoBlue;
						XStoreColor ( dpy, Cmap, &colour );
					}
				}
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
			 *
			 * Together with R1 of the CRTC, it appears to control the
			 * number of bits used for storing pixel colours -- maybe
			 * that's how the stretching of the characters works, too.
			 */

			/*
			 * PixelWidth is the number of pixels in our real display that
			 * each Beeb screen pixel takes up.
			 */

			switch ( Register0 & 0x0c )
			{
				case 0x00 :
					CharsPerLine = 10;
					PixelWidth = 8;
					break;
				case 0x04 :
					CharsPerLine = 20;
					PixelWidth = 4;
					break;
				case 0x08 :
					CharsPerLine = 40;
					PixelWidth = 2;
					break;
				case 0x0c :
					CharsPerLine = 80;
					PixelWidth = 1;
					break;
			}

			if (!( BitsForColourInfo = HorizDisplayed / CharsPerLine ))
				BitsForColourInfo = 1;
			PixelsPerByte = 8 / BitsForColourInfo;
			ByteWidth = PixelWidth * PixelsPerByte;

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
			break;

		case 0x01 :
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
			XColor		colour;

			logical = val >> 4;
			switch ( BitsForColourInfo )
			{
				case 1 :
					if ( logical & 0x8 )
						logical = 1;
					else
						logical = 0;
					break;
				case 2 :
					switch ( logical & 0xa )
					{
						case 0x0 :
							logical = 0;
							break;
						case 0x2 :
							logical = 1;
							break;
						case 0x8 :
							logical = 2;
							break;
						case 0xa :
							logical = 3;
							break;
					}
					break;
				case 4 :
					break;
				default :
					/*
					 * FIX ME
					 *
					 * Should tidy up here...
					 */

					fprintf( stderr, "VideoUla.c -- bad no. of bits/colour\n");
					FatalError();
					break;
			}

			physical = ( val & 0xf ) ^ 0x7;

			ColourMap [ logical ] = physical;

			Cells [ logical ] = ColourBits | ( logical & 1 ? Masks [ 0 ] : 0) |
											( logical & 2 ? Masks [ 1 ] : 0 ) |
											( logical & 4 ? Masks [ 2 ] : 0 ) |
											( logical & 8 ? Masks [ 3 ] : 0 );
			colour.pixel = Cells [ logical ];
			colour.red = RgbValues [ physical ][ 0 ];
			colour.green = RgbValues [ physical ][ 1 ];
			colour.blue = RgbValues [ physical ][ 2 ];
			colour.flags = DoRed | DoGreen | DoBlue;
			XStoreColor ( dpy, Cmap, &colour );
#ifdef	INFO
			printf ( "Video ULA palette colour %d = %d\n", val>>4, physical );
#endif
			break;
		}
		default :
			fprintf ( stderr, "Illegal write video ULA (addr = %x)\n", addr );
			break;
	}
	return;
}


void
RestoreColourMap()
{
	int			max, logical, physical;
	XColor		colour;

	switch ( BitsForColourInfo )
	{
		case 1 :
			max = 2;
			break;
		case 2 :
			max = 4;
			break;
		case 4 :
			max = 16;
			break;
		default :
			/*
			 * FIX ME
			 *
			 * Should tidy up here...
			 */

			fprintf( stderr, "VideoUla.c -- bad no. of bits/colour\n");
			FatalError();
			break;
	}

	for ( logical = 0; logical < max; logical++ )
	{
		physical = ColourMap [ logical ];
		Cells [ logical ] = ColourBits | ( logical & 1 ? Masks [ 0 ] : 0) |
										( logical & 2 ? Masks [ 1 ] : 0 ) |
										( logical & 4 ? Masks [ 2 ] : 0 ) |
										( logical & 8 ? Masks [ 3 ] : 0 );
		colour.pixel = Cells [ logical ];
		colour.red = RgbValues [ physical ][ 0 ];
		colour.green = RgbValues [ physical ][ 1 ];
		colour.blue = RgbValues [ physical ][ 2 ];
		colour.flags = DoRed | DoGreen | DoBlue;
		XStoreColor ( dpy, Cmap, &colour );
	}
}


int
SaveVideoUla ( int fd )
{
	byteval			vid [ 32 ];

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
	switch ( Register0 & 0x0c )
	{
		case 0x00 :
			CharsPerLine = 10;
			PixelWidth = 8;
			break;
		case 0x04 :
			CharsPerLine = 20;
			PixelWidth = 4;
			break;
		case 0x08 :
			CharsPerLine = 40;
			PixelWidth = 2;
			break;
		case 0x0c :
			CharsPerLine = 80;
			PixelWidth = 1;
			break;
	}
	ByteWidth = PixelWidth * PixelsPerByte;

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
		InitialiseBitmap();

	return 0;
}
