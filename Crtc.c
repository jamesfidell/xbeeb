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


#include	<stdio.h>
#include	<unistd.h>

#include	"Config.h"
#include	"Crtc.h"
#include	"Screen.h"
#include	"Modes.h"
#include	"VideoUla.h"

static unsigned char	RegisterToAccess = 0xff;
unsigned char			CrtcRegisters [ 18 ];

unsigned char			CursorBlinkEnable;
unsigned char			CursorBlinkFrequency;
unsigned char			CursorStartLine;
unsigned char			DisplayDelay;
unsigned char			CursorDelay;
unsigned char			CursorEnabled;
unsigned char			CursorMoved = 0;
unsigned char			CursorResized = 0;
unsigned char			NewCursorX = 255;
unsigned char			NewCursorY = 255;

static unsigned int		CursorAddress, CursorOffset;


#define	MIN(a,b)		a < b ? a : b;

unsigned char
ReadCrtc ( int addr )
{
	/*
	 * FIX ME
	 */

	fprintf ( stderr, "Attempt to read CRTC (addr = %x)\n", addr );
	fprintf ( stderr, "Not yet implemented\n" );
	exit ( 1 );
	return 0;
}

void
WriteCrtc ( int addr, unsigned char val )
{
	switch ( addr )
	{
		case 0x0 :
			RegisterToAccess = val & 0x1f;
#ifdef	INFO
			printf ( "CRTC address register set to 0x%x\n", RegisterToAccess );
#endif
			break;

		case 0x1 :
			switch ( RegisterToAccess )
			{
				case 0x0 :
					/*
					 * Changing this value with probably completely destroy
					 * the display, so I feel justified in ignoring it.
					 * For the moment, I'll just display a warning if it's
					 * set to an unexpected value.
					 */

					if (( HorizTotal = val ) != 63 && val != 127 )
						fprintf ( stderr, "CRTC R0 set to odd value\n" );
#ifdef	INFO
					printf ( "CRTC Horiz. Total set to 0x%2x\n", val );
#endif
					break;

				case 0x1 :
					/*
					 * Changing this changes the number of characters that
					 * are displayed on each line, without changing the
					 * geometry of the screen, so lines just appear to
					 * wrap around earlier on the screen.
					 *
					 * FIX ME
					 *
					 * If this is made smaller mid-MODE, then the right
					 * hand side of the screen will probably end up with
					 * junk on it, whereas it should be blank.
					 *
					 * Perhaps that should be handled by reducing the
					 * width of the window which represents the screen ?
					 */

					HorizDisplayed = val;
#ifdef	INFO
					printf ( "CRTC Horiz. Displayed set to 0x%2x\n", val );
#endif
					break;

				case 0x2 :
					/*
					 * FIX ME
					 *
					 * This sets the position of the screen relative to the
					 * left edge of the scan line field, so it has no
					 * direct effect on the actual display.
					 *
					 * I can't decide whether to ignore it, or to handle it
					 * by changing the size/layout of the screen, somehow.
					 *
					 * One possibility might be to take the value of (R0-R2)
					 * and use that as some sort of offset from the edge of
					 * the display window, so that the display is moved
					 * left and right as this register is changed.
					 *
					 * For the moment, I'll just give a warning message if
					 * the register is set to an unexpected value.
					 */

					if (( HorizSync = val ) != 51 && val != 49 && val != 98 )
						fprintf ( stderr, "CRTC R2 set to odd value\n" );
#ifdef	INFO
					printf ( "CRTC Horiz. Sync. pos. set to 0x%2x\n", val );
#endif
					break;

				case 0x3 :
					/*
					 * Changing this value will probably cause the display
					 * to collapse completely, so I'm going to ignore it.
					 * A warning is issued if it isn't a recognised value.
					 *
					 */

					if (( SyncWidth = val ) != 0x28 && val != 0x24 )
						fprintf ( stderr, "CRTC R3 set to odd value\n" );
#ifdef	INFO
					printf ( "CRTC Sync. width set to 0x%2x\n", val );
#endif
					break;

				case 0x4 :
					/*
					 * FIX ME
					 *
					 * Changing this register should move the display up and
					 * down on the display.
					 *
					 * Much the same comments apply as for R2.
					 *
					 * For now, I'll just give a warning if it's set to
					 * a value I don't expect.
					 */

					if (( VertTotal = val ) != 38 && val != 30 )
						fprintf ( stderr, "CRTC R4 set to odd value\n" );
#ifdef	INFO
					printf ( "CRTC Vert. Total set to 0x%2x\n", val );
#endif
					break;

				case 0x5 :
					/*
					 * FIX ME
					 *
					 * Move the display up/down a number of scan lines.
					 * Same situation as for R4.
					 */

					if (( VertAdjust = val ) != 0 && val != 2 )
						fprintf ( stderr, "CRTC R5 set to odd value\n" );
#ifdef	INFO
					printf ( "CRTC Vert. Adjust set to 0x%2x\n", val );
#endif
					break;

				case 0x6 :
					/*
					 * FIX ME
					 *
					 * This is the vertical equivalent of R1 and the same
					 * comments apply.
					 */

					VertDisplayed = val;
#ifdef	INFO
					printf ( "CRTC Vert. Displayed set to 0x%2x\n", val );
#endif
					break;

				case 0x7 :
					/*
					 * FIX ME
					 *
					 * This is the vertical equivalent of R2 and again, the
					 * same comments apply.
					 *
					 */

					if (( VertSync = val ) != 34 && val != 27 && val != 28 )
						fprintf ( stderr, "CRTC R7 set to odd value\n" );
#ifdef	INFO
					printf ( "CRTC Vert. Sync. pos. set to 0x%2x\n", val );
#endif
					break;

				case 0x8 :
					/*
					 * FIX ME
					 *
					 * Ignore the low two bits of this value -- they only
					 * set the raster scan mode and interlace mode.
					 *
					 * Bits 4/5 set the display blanking delay, used to allow
					 * for reading data from memory or the SA5050.  It should
					 * be properly handled.
					 *
					 * Bits 6/7 set the cursor delay / disable the cursor.
					 * Disable is handled, the delay isn't.
					 *
					 */

					IlaceDelay = val;
					DisplayDelay = IlaceDelay & 0x30;
					CursorDelay = IlaceDelay & 0xc0;
					CursorEnabled = CursorDelay ^ 0xc0;
#ifdef	INFO
					printf ( "CRTC Interlace & delay set to 0x%2x\n", val );
#endif
					break;

				case 0x9 :
					/*
					 * FIX ME
					 *
					 * Changing this appears to completely nobble the display
					 * so just give a warning if it's changed to a value that
					 * isn't expected.
					 *
					 */

					if (( ScanLines = val ) != 7 && val != 9 && val != 18 )
						fprintf ( stderr, "CRTC R9 set to odd value\n" );
#ifdef	INFO
					printf ( "CRTC Scan lines/char set to 0x%2x\n", val );
#endif
					break;

				case 0xa :
					CursorStart = val;
					CursorResized = ( val & 0x1f == CursorStartLine ) ? 0 : 1;
					CursorBlinkEnable = CursorStart & 0x40;
					CursorBlinkFrequency = ( CursorStart & 0x20 ) ? 32 : 16;
					CursorStartLine = CursorStart & 0x1f;
#ifdef	INFO
					printf ( "CRTC Cursor start set to 0x%2x\n", val );
#endif
					break;

				case 0xb :
					CursorResized = ( val & 0x1f == CursorEndLine ) ? 0 : 1;
					CursorEndLine = val & 0x1f;
#ifdef	INFO
					printf ( "CRTC Cursor end set to 0x%2x\n", val );
#endif
					break;

				case 0xc :
#ifdef	INFO
					printf ( "CRTC Screen addr (hi) set to 0x%2x\n", val );
#endif
					/*
					 * Only the low 6 bits are valid
					 */

					val &= 0x3f;

					if ( ScreenStartHi != val )
					{
						ScreenStartHi = val;

						/*
						 * Calculating the address of the top of the
						 * screen is a mess in mode 7.  In other modes
						 * it's OK.
						 */

						if ( CurrentMode == 7 )
							TopOfScreen = (( val ^ 0x20 ) + 0x74 ) << 8;
						else
							TopOfScreen = val << 8;
						TopOfScreen |= ScreenStartLo;

						/*
						 * We may also need to calculate new values for the
						 * StartOfScreenMemory and EndOfScreenMemory
						 */

						if ( TopOfScreen < StartOfScreenMemory ||
								TopOfScreen > EndOfScreenMemory )
						{
							StartOfScreenMemory = MIN ( TopOfScreen,
										0x8000 - ScreenLength );
							EndOfScreenMemory = MIN ( 0x8000,
										TopOfScreen + ScreenLength );
						}
						ScreenChanged++;
					}
					break;

				case 0xd :
#ifdef	INFO
					printf ( "CRTC Screen start (lo) set to 0x%2x\n", val );
#endif
					if ( ScreenStartLo != val )
					{
						ScreenStartLo = val;
						TopOfScreen = ( TopOfScreen & 0xff00 ) | val;

						/*
						 * We may also need to calculate new values for the
						 * StartOfScreenMemory and EndOfScreenMemory
						 */

						if ( TopOfScreen < StartOfScreenMemory ||
								TopOfScreen > EndOfScreenMemory )
						{
							StartOfScreenMemory = MIN ( TopOfScreen,
										0x8000 - ScreenLength );
							EndOfScreenMemory = MIN ( 0x8000,
										TopOfScreen + ScreenLength );
						}
						ScreenChanged++;
					}
					break;

				case 0xe :
					/*
					 * Only the low 6 bits are valid.
					 *
					 * Calculating the cursor address is easy, but working
					 * out the X and Y positions of the cursor isn't
					 * such a doddle.
					 *
					 * It would be nice to do it as (cursor addr - top of
					 * screen addr), mod (chars per line) for the X position
					 * and div (chars per line) for the Y position.
					 *
					 * However, the cursor address could be lower than the
					 * top of screen address once the screen has scrolled,
					 * and the 6845 doesn't know how many screen characters
					 * exist per line, only the horizontal displayed value,
					 * which is not the same.  The number of chars per line
					 * is given, though in one of the video ULA registers.
					 *
					 */

					val &= 0x3f;

					if ( CursorPosHi != val )
					{
#ifdef	INFO
						printf ( "CRTC Cursor pos (hi) set to 0x%2x\n", val );
#endif
						CursorPosHi = val;

						/*
						 * Calculating the address of the cusor is the same
						 * as the top of the screen.
						 */

						if ( CurrentMode == 7 )
							CursorAddress = (( val ^ 0x20 ) + 0x74 ) << 8;
						else
							CursorAddress = val << 8;
						CursorAddress |= CursorPosLo;

						CursorOffset = CursorAddress - TopOfScreen;
						if ( CursorAddress < TopOfScreen )
							CursorOffset += ScreenLength;

						/*
						 * FIX ME
						 *
						 * This is not right, because it doesn't take
						 * account of the horizontal displayed value.
						 *
						 * It would be better to use
						 * HorizDisplayed / ( 6845 chars per real char )
						 * rather than CharsPerLine.
						 */

						NewCursorX = CursorOffset % CharsPerLine;
						NewCursorY = CursorOffset / CharsPerLine;
						CursorMoved = 1;
					}
					break;

				case 0xf :
					if ( CursorPosLo != val )
					{
#ifdef	INFO
						printf ( "CRTC Cursor pos (lo) set to 0x%2x\n", val );
#endif
						CursorPosLo = val;
						CursorAddress = ( CursorAddress & 0xff00 ) | val;

						CursorOffset = CursorAddress - TopOfScreen;
						if ( CursorAddress < TopOfScreen )
							CursorOffset += ScreenLength;

						/*
						 * FIX ME
						 *
						 * This is not right, because it doesn't take
						 * account of the horizontal displayed value.
						 *
						 * It would be better to use
						 * HorizDisplayed / ( 6845 chars per real char )
						 * rather than CharsPerLine.
						 */

						NewCursorX = CursorOffset % CharsPerLine;
						NewCursorY = CursorOffset / CharsPerLine;
						CursorMoved = 1;
					}
					break;

				default :
					fprintf(stderr, "Illegal write CRTC (addr = 0x%x)\n", addr );
					exit ( 1 );
					break;
			}
			break;

		default :
			fprintf ( stderr, "Illegal write to CRTC (addr = 0x%x)\n", addr );
			exit ( 1 );
			break;
	}
}


int
SaveCRTC ( int fd )
{
	unsigned char		crtc [ 32 ];

	memcpy ( crtc, CrtcRegisters, 18 );
	crtc [ 20 ] = RegisterToAccess;

	if ( write ( fd, crtc, 32 ) != 32 )
		return -1;

	return 0;
}


int
RestoreCRTC ( int fd, unsigned int ver )
{
	unsigned char		crtc [ 32 ];

	if ( ver > 1 )
		return -1;

	if ( read ( fd, crtc, 32 ) != 32 )
		return -1;

	memcpy ( CrtcRegisters, crtc, 18 );
	RegisterToAccess = crtc [ 20 ];

	/*
	 * We also need to re-calculate the top of screen address.
	 * StartOfScreenMemory and EndOfScreen memory will initially be
	 * 0xffff, so we'll be alright messing around with them.
	 * CurrentMode and ScreenLength will have been set when the
	 * Video ULA was restored.
	 *
	 */

	if ( CurrentMode == 7 )
		TopOfScreen = (( ScreenStartHi ^ 0x20 ) + 0x74 ) << 8;
	else
		TopOfScreen = ScreenStartHi << 8;
	TopOfScreen |= ScreenStartLo;

	if ( TopOfScreen < StartOfScreenMemory || TopOfScreen > EndOfScreenMemory )
	{
		StartOfScreenMemory = MIN ( TopOfScreen, 0x8000 - ScreenLength );
		EndOfScreenMemory = MIN ( 0x8000, TopOfScreen + ScreenLength );
	}

	ScreenChanged++;

	CursorBlinkEnable = CursorStart & 0x40;
	CursorBlinkFrequency = ( CursorStart & 0x20 ) ? 32 : 16;
	CursorStartLine = CursorStart & 0x1f;
	CursorEnabled = IlaceDelay & 0xc0;

	return 0;
}
