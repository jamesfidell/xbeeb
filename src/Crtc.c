/*
 *
 * $Id: Crtc.c,v 1.25 2002/01/15 15:46:43 james Exp $
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
 * $Log: Crtc.c,v $
 * Revision 1.25  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.24  2000/09/07 23:10:34  james
 * Tidyups and correct fix for not correctly handling bitmap screens where
 * existing colour matches current
 *
 * Revision 1.23  2000/08/16 17:58:27  james
 * Update copyright message
 *
 * Revision 1.22  1996/11/25 00:01:17  james
 * Remove the "Default Vertical Sync value" magic number.
 *
 * Revision 1.21  1996/11/24 22:37:32  james
 * Updates to comments.
 *
 * Revision 1.20  1996/11/24 22:32:27  james
 * Limit Crtc R5 to 5 valid bits.
 *
 * Revision 1.19  1996/11/24 21:54:09  james
 * Change MIN to XBEEB_MIN to avoid clashes with some OSes.
 *
 * Revision 1.18  1996/11/19 01:13:42  james
 * Tidy up display of register settings.
 *
 * Revision 1.17  1996/11/18 01:01:28  james
 * Changes to comments.
 *
 * Revision 1.16  1996/11/15 23:45:57  james
 * Oops!  Forgot to add the lo half of the screen start address back in
 * when calculating the new top of (teletext) screen address when the hi
 * byte is written to the CRTC.
 *
 * Revision 1.15  1996/11/09 23:02:29  james
 * Take account of the horizontal start of display based on the clock rate
 * set in the Video ULA and the CRTC Horiz. Sync. register.
 *
 * Revision 1.14  1996/11/09 22:32:54  james
 * Updated bitmapped screen-handling code to take account of the Vertical
 * Adjust register in the CRTC.
 *
 * Revision 1.13  1996/10/13 12:13:30  james
 * Parenthesise all parameters to #defined macros.
 *
 * Revision 1.12  1996/10/10 21:44:02  james
 * Fixes from David Ralph Stacey for scan-line updates.
 *
 * Revision 1.11  1996/10/09 23:43:48  james
 * Sort out Video ULA register wrap around.  Also tidy up setting of
 * BitsForColourInfo.
 *
 * Revision 1.10  1996/10/09 22:55:41  james
 * Optimise writes to R10 & R11.
 *
 * Revision 1.9  1996/10/09 22:40:56  james
 * Handle register wrap-around for the CRTC.
 *
 * Revision 1.8  1996/10/09 22:06:54  james
 * Overhaul of the bitmapped screen handling code with particular respect to
 * colour maps.
 *
 * Revision 1.7  1996/10/08 23:05:30  james
 * Corrections to allow clean compilation under GCC 2.7.2 with -Wall -pedantic
 *
 * Revision 1.6  1996/10/01 00:32:59  james
 * Created separate hardware reset code for each emulated unit and called
 * these from the main initialisation section of the code to do all of the
 * setup necessary.
 *
 * Revision 1.5  1996/09/24 23:05:35  james
 * Update copyright dates.
 *
 * Revision 1.4  1996/09/23 16:09:50  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.3  1996/09/21 23:07:35  james
 * Call FatalError() rather than exit() so that screen stuff etc. can
 * be cleaned up.
 *
 * Revision 1.2  1996/09/21 22:13:47  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:36  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include	<stdio.h>
#include	<unistd.h>
#include	<memory.h>

#include	"Config.h"
#include	"Crtc.h"
#include	"Beeb.h"
#include	"Screen.h"
#include	"Modes.h"
#include	"Bitmap.h"
#include	"Teletext.h"
#include	"VideoUla.h"
#include	"Memory.h"


static byteval			RegisterToAccess = 0xff;
byteval					CrtcRegisters [ 18 ];
byteval					ScanLinesPlus1;
byteval					DefaultVertSync = 0;
unsigned short			HorizDisplayed8;
unsigned short			VertAdjust2;

unsigned char			CursorBlinkEnable;
unsigned char			CursorBlinkFrequency;
unsigned char			CursorStartLine;
unsigned char			DisplayDelay;
unsigned char			Interlace;
unsigned char			Video;
unsigned char			CursorDelay;
unsigned char			CursorEnabled;
unsigned char			CursorMoved = 0;
unsigned char			CursorResized = 0;
unsigned char			NewCursorX = 255;
unsigned char			NewCursorY = 255;
unsigned int			CrtcMagicNumber;
unsigned int			CursorAddress, CursorOffset;


void
ResetCrtc ( void )
{
	/*
	 * FIX ME
	 *
	 * I have no idea what happens when the CRTC is powered up/reset
	 */

	return;
}


byteval
ReadCrtc ( int addr )
{
	/*
	 * Only the data register on the CRTC appears to be read/write.
	 * The address register always returns zero when it is read (allegedly).
	 * However, since the CRTC is only two addresses, all addresses
	 * from &FE02 to &FE07 wrap back onto the first two.
	 */

	if ( addr & 0x1 )
	{
		/*
		 * We have the data register here.  Only R12-R17 actually
		 * read a useful value, though.
		 */

		switch ( RegisterToAccess )
		{
			case 0x0c :
				return ScreenStartHi;
				break;
	
			case 0x0d :
				return ScreenStartLo;
				break;
	
			case 0x0e :
				return CursorPosHi;
				break;
	
			case 0x0f :
				return CursorPosLo;
				break;
	
			case 0x10 :
				return LightPenHi;
				break;
	
			case 0x11 :
				return LightPenLo;
				break;

			default :
				/*
				 * This will get caught outside the "if"
				 */
				break;
		}
	}

	return 0;
}


void
WriteCrtc ( int addr, byteval val )
{
	switch ( addr & 0x1 )
	{
		case 0x0 :
			/*
			 * FIX ME
			 *
			 * RegisterToAccess = val & 0x1f, because we have to allow for
			 * registers 16 and 17.  But what happens for the values
			 * between 18 & 31 ?  This needs checking to see what actually
			 * gets read when there's a potential wrap-around.
			 */

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
					HorizDisplayed8 = val * 8;
#ifdef	INFO
					printf ( "CRTC Horiz. Displayed set to 0x%2x\n", val );
#endif
					break;

				case 0x2 :
					HorizSync = val;

					/*
					 * FIX ME
					 *
					 * where do these "magic" numbers come from ?
					 */

					if ( ClockRate )
						StartPosnX = ( 98 - HorizSync ) * 8;
					else
						StartPosnX = ( 49 - HorizSync ) * 16;
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
						fprintf ( stderr, "CRTC R3 set to 0x%2x\n", val );
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
						fprintf ( stderr, "CRTC R4 set to 0x%2x\n", val );
#ifdef	INFO
					printf ( "CRTC Vert. Total set to 0x%2x\n", val );
#endif
					break;

				case 0x5 :
					/*
					 * Only the 5 lo bits of this register are valid, so the
					 * rest are masked off.
					 */

					/*
					 * FIX ME
					 *
					 * Psycastria sets this to 0x60.  Why ?
					 *
					 * Also, is there a requirement to update the screen
					 * here, or can (as I am going to do) we rely on things
					 * working out in the normal course of time ?
					 */

					VertAdjust = val & 0x1f;
					VertAdjust2 = val << 1;
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
					 *
					 * Should we update the screen immediately here, or wait
					 * for things to work out in the fullness of time (which
					 * I do) ?
					 */

					VertDisplayed = val;
#ifdef	INFO
					printf ( "CRTC Vert. Displayed set to 0x%2x\n", val );
#endif
					break;

				case 0x7 :
				{
					int			y;

					/*
					 * FIX ME
					 *
					 * Where does 560 come from here ?
					 */

					VertSync = val;
					if ( Video )
						y = 560 - ( ScanLinesPlus1 ) * VertSync;
					else
						y = 560 - ( ScanLinesPlus1 ) * VertSync * 2;

					/*
					 * Could just move the right window, but there's
					 * nothing to say that we actually know which is
					 * the right window at this stage.
					 */

					ChangeBitmapWindowY ( y );
					ChangeTeletextWindowY ( y );

					CrtcMagicNumber = ( DefaultVertSync - VertSync ) *
												ScanLinesPlus1 * uS_PER_LINE;
#ifdef	INFO
					printf ( "CRTC Vert. Sync. pos. set to 0x%2x\n", val );
#endif
					break;
				}
				case 0x8 :
					/*
					 * FIX ME
					 *
					 * Interlace should affect the way that characters
					 * are drawn on the screen.  Especially in bitmap
					 * mode, because using fonts means that it can't
					 * be done in MODE 7.
					 *
					 * Video should, too, because it compresses the
					 * frame.  It's only supposed to be used in MODE 7
					 * and I don't handle it anywhere else at all.
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
					Interlace = IlaceDelay  & 0x1;
					Video = (( val & 0x3 ) == 0x3 );
#ifdef	INFO
					printf ( "CRTC Interlace & delay set to 0x%2x\n", val );
#endif
					break;

				case 0x9 :
					ScanLines = val;
					ScanLinesPlus1 = ScanLines + 1;
					CrtcMagicNumber = ( DefaultVertSync - VertSync ) *
												ScanLinesPlus1 * uS_PER_LINE;
#ifdef	INFO
					printf ( "CRTC Scan lines/char set to 0x%2x\n", val );
#endif
					break;

				case 0xa :
					CursorStart = val;
#ifdef	INFO
					printf ( "CRTC Cursor start set to 0x%2x\n", val );
#endif
					val &= 0x1f;
					CursorResized = ( val == CursorStartLine ) ? 0 : 1;
					CursorBlinkEnable = CursorStart & 0x40;

					/*
					 * The actual frequency is 1/32nd or 1/16th of the
					 * field rate, but I use this to indicate when to
					 * change state, so I half it to get the state duration.
					 */

					CursorBlinkFrequency = ( CursorStart & 0x20 ) ? 16 : 8;
					CursorStartLine = val;
					break;

				case 0xb :
#ifdef	INFO
					printf ( "CRTC Cursor end set to 0x%2x\n", val );
#endif
					val &= 0x1f;
					CursorResized = ( val == CursorEndLine ) ? 0 : 1;
					CursorEndLine = val;
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

						if ( CurrentScreenMode == MODE_TELETEXT )
						{
							TopOfScreen = (( val ^ 0x20 ) + 0x74 ) << 8;
							TopOfScreen |= ScreenStartLo;
						}
						else
						{
							TopOfScreen = val << 8;
							TopOfScreen |= ScreenStartLo;
							TopOfScreen *= 8;
						}

						ScreenMemoryChanged = 1;
						( void ) memset (( void* ) ScreenCheck, 1, 32768 );
					}
					break;

				case 0xd :
#ifdef	INFO
					printf ( "CRTC Screen start (lo) set to 0x%2x\n", val );
#endif
					if ( ScreenStartLo != val )
					{
						ScreenStartLo = val;

						if ( CurrentScreenMode == MODE_TELETEXT )
							TopOfScreen = ( TopOfScreen & 0xff00 ) | val;
						else
						{
							TopOfScreen = ScreenStartHi << 8;
							TopOfScreen |= ScreenStartLo;
							TopOfScreen *= 8;
						}
						ScreenMemoryChanged = 1;
						( void ) memset (( void* ) ScreenCheck, 1, 32768 );
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

						if ( CurrentScreenMode == MODE_TELETEXT )
						{
							CursorAddress = (( val ^ 0x20 ) + 0x74 ) << 8;
							CursorAddress |= CursorPosLo;
						}
						else
						{
							CursorAddress = val << 8;
							CursorAddress |= CursorPosLo;
							CursorAddress *= 8;
						}

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

						if ( CurrentScreenMode == MODE_TELETEXT )
							CursorAddress = ( CursorAddress & 0xff00 ) | val;
						else
						{
							CursorAddress = CursorPosHi << 8;
							CursorAddress |= CursorPosLo;
							CursorAddress *= 8;
						}

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
#ifdef	WARNINGS
					fprintf ( stderr, "Bad write CRTC (R%02d = 0x%02x)\n",
													RegisterToAccess, val );
#endif
					break;
			}
			break;

		default :
			fprintf ( stderr, "Bad write CRTC (R%02d = 0x%02x)\n",
													RegisterToAccess, val );
			FatalError();
			break;
	}
}


int
SaveCRTC ( int fd )
{
	byteval		crtc [ 32 ];

	memcpy ( crtc, CrtcRegisters, 18 );
	crtc [ 20 ] = RegisterToAccess;

	if ( write ( fd, crtc, 32 ) != 32 )
		return -1;

	return 0;
}


int
RestoreCRTC ( int fd, unsigned int ver )
{
	byteval		crtc [ 32 ];

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
	 * CurrentScreenMode and ScreenLength will have been set when the
	 * Video ULA was restored.
	 *
	 */

	if ( CurrentScreenMode == MODE_TELETEXT )
	{
		TopOfScreen = (( ScreenStartHi ^ 0x20 ) + 0x74 ) << 8;
		TopOfScreen |= ScreenStartLo;
	}
	else
	{
		TopOfScreen = ScreenStartHi << 8;
		TopOfScreen |= ScreenStartLo;
		TopOfScreen *= 8;
	}

	/*
	 * FIX ME
	 *
	 * Used to restore the address of the start of the screen here, but
	 * not now because it's calculated when the bits are set by the
	 * System VIA.
	 *
	 * However, it's still got to get restored somewhere...
	 */

	ScreenMemoryChanged = 1;
	( void ) memset (( void* ) ScreenCheck, 1, 32768 );

	CursorBlinkEnable = CursorStart & 0x40;

	/*
	 * See the comments in the main code about this variable...
	 */
	CursorBlinkFrequency = ( CursorStart & 0x20 ) ? 16 : 8;
	CursorStartLine = CursorStart & 0x1f;

	DisplayDelay = IlaceDelay & 0x30;
	CursorDelay = IlaceDelay & 0xc0;
	CursorEnabled = CursorDelay ^ 0xc0;
	Interlace = IlaceDelay  & 0x1;
	Video = IlaceDelay  & 0x2;

	if ( CurrentScreenMode == MODE_TELETEXT )
	{
		CursorAddress = (( CursorPosHi ^ 0x20 ) + 0x74 ) << 8;
		CursorAddress |= CursorPosLo;
	}
	else
	{
		CursorAddress = CursorPosHi << 8;
		CursorAddress |= CursorPosLo;
		CursorAddress *= 8;
	}

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
	CursorMoved = CursorResized = 1;

	/*
	 * FIX ME
	 *
	 * Now deal with the update of the window position based on the
	 * value of R7 and R2 (R2 is now done below...)
  	 */
  
	if ( ClockRate )
		StartPosnX = ( 98 - HorizSync ) * 8;
	else
		StartPosnX = ( 49 - HorizSync ) * 16;

	/*
	 * Set up variables for optimisation of drawing bitmapped display.
	 */

	ScanLinesPlus1 = ScanLines + 1;
	HorizDisplayed8 = HorizDisplayed * 8;
	VertAdjust2 = VertAdjust << 1;
	CrtcMagicNumber = ( DefaultVertSync - VertSync ) * ScanLinesPlus1
															* uS_PER_LINE;

	return 0;
}
