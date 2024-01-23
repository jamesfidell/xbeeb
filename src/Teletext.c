/*
 *
 * $Id: Teletext.c,v 1.11 1996/10/10 22:09:02 james Exp $
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
 * $Log: Teletext.c,v $
 * Revision 1.11  1996/10/10 22:09:02  james
 * HoldCharacter needs to change back to space for TT_DOUBLE_HEIGHT
 *
 * Revision 1.10  1996/10/09 22:06:57  james
 * Overhaul of the bitmapped screen handling code with particular respect to
 * colour maps.
 *
 * Revision 1.9  1996/10/08 00:04:35  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.8  1996/09/30 23:39:35  james
 * Split out option processing into Options.[ch].  Updated the help message,
 * added support for the Model A using the -a switch (and added the
 * MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
 * initial screen mode and keyboard DIP switches.
 *
 * Revision 1.7  1996/09/24 23:05:44  james
 * Update copyright dates.
 *
 * Revision 1.6  1996/09/23 16:09:52  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.5  1996/09/22 22:34:33  james
 * Completed handling of teletext HOLD mode.
 *
 * Revision 1.4  1996/09/22 22:21:09  james
 * Add handling of double-height fonts.
 *
 * Revision 1.3  1996/09/22 22:01:53  james
 * Use #defined values for teletext control characters.
 *
 * Revision 1.2  1996/09/21 18:18:18  james
 * Corrections to variable typing for CursorX, CursorY and line.
 *
 * Revision 1.1  1996/09/21 17:20:42  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Config.h"
#include "Teletext.h"
#include "Memory.h"
#include "Screen.h"
#include "Modes.h"
#include "Crtc.h"
#include "VideoUla.h"

/*
 * Teletext font characters
 */

#define	TT_SPACE			 32
#define	TT_NUL_128			128
#define	TT_ALPHA_RED		129
#define	TT_ALPHA_GREEN		130
#define	TT_ALPHA_YELLOW		131
#define	TT_ALPHA_BLUE		132
#define	TT_ALPHA_MAGENTA	133
#define	TT_ALPHA_CYAN		134
#define	TT_ALPHA_WHITE		135
#define	TT_STEADY			136
#define	TT_FLASH			137
#define	TT_NUL_138			138
#define	TT_NUL_139			139
#define	TT_NORMAL_HEIGHT	140
#define	TT_DOUBLE_HEIGHT	141
#define	TT_NUL_142			142
#define	TT_NUL_143			143
#define	TT_NUL_144			144
#define	TT_GRAPHIC_RED		145
#define	TT_GRAPHIC_GREEN	146
#define	TT_GRAPHIC_YELLOW	147
#define	TT_GRAPHIC_BLUE		148
#define	TT_GRAPHIC_MAGENTA	149
#define	TT_GRAPHIC_CYAN		150
#define	TT_GRAPHIC_WHITE	151
#define	TT_CONCEAL			152
#define	TT_CONTIGUOUS		153
#define	TT_SEPARATED		154
#define	TT_NUL_155			155
#define	TT_BLACK_BG			156
#define	TT_NEW_BG			157
#define	TT_HOLD_GRAPHICS	158
#define	TT_REL_GRAPHICS		159

/*
 * Current state of double height font.
 *
 * POSSIBLE ENHANCEMENT ?
 *
 * Values are important for this.  Possibly these should really be an
 * enumerated type.
 */

#define	DH_NONE			0
#define	DH_UPPER		1
#define	DH_LOWER		2

int						TeletextWindowX = 80;
int						TeletextWindowY = 0;

static int				FlashTimer;
static int				CursorTimer;
static unsigned char	FlashStatus;
static unsigned char	CursorOnScreen;

static int				CursorX = 255;
static int				CursorY = 255;

static unsigned int		CursorDepth = 255;
static unsigned int		CursorWidth = 255;
static unsigned int		CursorViewable = 0;

static void				DrawCursor();

#define	MIN(x,y)	(( x < y ) ? x : y )

void
InitialiseTeletext()
{
	if ( CurrentScreenMode != MODE_TELETEXT )
	{
		int				i;
		XColor			colour;

		CurrentScreenMode = MODE_TELETEXT;

		/*
		 * FIX ME
	 	 *
		 * I'm not actually sure I need to do this...
		 *
		 * Set the first 8 entries into the colourmap back to their
		 * default values -- they might have been changed if we've
		 * changed from a graphics mode where the colours were
		 * redefined.
		 */

		for ( i = 0; i < 8; i++ )
		{
			Cells [ i ] = ColourBits | ( i & 1 ? Masks [ 0 ] : 0 ) |
					( i & 2 ? Masks [ 1 ] : 0 ) | ( i & 4 ? Masks [ 2 ] : 0 );
			colour.pixel = Cells [ i ];
			colour.red = RgbValues [ i % 8 ][ 0 ];
			colour.green = RgbValues [ i % 8 ][ 1 ];
			colour.blue = RgbValues [ i % 8 ][ 2 ];
			colour.flags = DoRed | DoGreen | DoBlue;
			XStoreColor ( dpy, DefCmap, &colour );
		}

		/*
		 * FIX ME
		 *
		 * Should update the screen to be consistent with whatever is
		 * in screen memory at the moment...
		 */

		XUnmapWindow ( dpy, BitmapScreen );
		XMapRaised ( dpy, TeletextScreen );
		XFlush ( dpy );
	}
	FlashTimer = CursorTimer = 0;
	FlashStatus = CursorOnScreen = 0;

	RecalculateScreenInfo();
	return;
}


void
TeletextScreenUpdate()
{
	unsigned int		charpos, p;
	unsigned char		lines, c, hpos, got, HoldCharacter;
	unsigned char		DblHeight = DH_NONE, ContGraphics;
	unsigned char		Conceal, Flash, DoFlash = 0;
	unsigned char		line[40];
	unsigned long		LastColour;
	GC					CurrGC;
	static char			SeenFlash = 0, HoldMode;

	/*
	 * Sort out the flash counter first -- we can return if it isn't
	 * time to update flashing colours and the screen hasn't changed.
	 */

	/*
	 * FIX ME
	 *
	 * I don't know what the mark and space periods for the teletext
	 * flash are.  This routine gets called every 25th of a second.
	 * I've guessed at values for the time being.
	 */

	if ( ++FlashTimer == 35 )
	{
		DoFlash = SeenFlash;
		FlashStatus = ~FlashStatus;
	}
	else
		if ( FlashTimer == 50 )
		{
			FlashStatus = ~FlashStatus;
			FlashTimer = 0;
			DoFlash = SeenFlash;
		}

	/*
	 * Update the cursor timer too.
	 *
	 * This is done here rather than in the cursor drawing code because
	 * that might be called more than once from this function and the timer
	 * should only be updated once per call to this function.
	 */

	CursorTimer = ( CursorTimer + 1 ) % CursorBlinkFrequency;

	if ( ScreenMemoryChanged || ScreenImageChanged || DoFlash )
	{
		SeenFlash = 0;
		CursorOnScreen = 0;

		p = TopOfScreen;
		for ( lines = 1; lines <= VertDisplayed; lines++ )
		{
			hpos = 0;
			got = 0;

			CurrGC = TtextTextGC;
			XSetForeground ( dpy, CurrGC, White );
			XSetBackground ( dpy, CurrGC, Black );

			XSetForeground ( dpy, TtextMosaicGC, White );
			XSetBackground ( dpy, TtextMosaicGC, Black );

			XSetFont ( dpy, TtextTextGC, TtextText );
			XSetFont ( dpy, TtextMosaicGC, TtextContiguousMosaic );
			ContGraphics = 1;

			LastColour = White;
			HoldCharacter = ' ';
			HoldMode = 0;
			Conceal = Flash = 0;

			/*
			 * This is why DH_xxx must be set to specific values
			 */
 
			if ( DblHeight )
				DblHeight = ( ++DblHeight ) % 3;

			for ( charpos = 0; charpos < HorizDisplayed; charpos++ )
			{
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * Shouldn't really address memory directly here, I know,
				 * but speed is of the essence
				 */

#ifdef	MODEL_B_ONLY
				c = Mem [ p++ ];
#else
				c = Mem [ p & MaxRAMAddress ];
				p++;
#endif

				/*
				 * Non-displayable characters (I think!)
				 */

				if ( c < TT_SPACE || c == TT_NUL_128 || c == TT_NUL_138 ||
							c == TT_NUL_139 || ( c > TT_DOUBLE_HEIGHT &&
							c < TT_GRAPHIC_RED ) || c == TT_NUL_155 )
				{
					line [ got++ ] = ' ';
					HoldCharacter = ' ';
					goto next_char;
				}

				/*
				 * printable characters
				 */

				if ( c < TT_NUL_128 || c > TT_REL_GRAPHICS )
				{
					if ( Flash || Conceal )
						line [ got++ ] = ' ';
					else
						line [ got++ ] = c;
					HoldCharacter = ' ';
					goto next_char;
				}

				/*
				 * New text colour.
				 *
				 * FIX ME
				 *
				 * should this character look like the current hold character ?
				 */

				if ( c < TT_STEADY )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );

					CurrGC = TtextTextGC;
					XSetForeground ( dpy, CurrGC, Cells [ c & 7 ]);
					LastColour = c & 7;
					hpos += got;
					got = 0;
					Conceal = 0;
					HoldCharacter = ' ';
					goto next_char;
				}

				/*
				 * Steady text
				 */

				if ( c == TT_STEADY )
				{
					line [ got++ ] = HoldMode ? HoldCharacter : ' ';
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					hpos += got;
					Flash = 0;
					got = 0;
					Conceal = 0;
					goto next_char;
				}

				/*
				 * Flashing text
				 */

				if ( c == TT_FLASH )
				{
					line [ got++ ] = HoldMode ? HoldCharacter : ' ';
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					hpos += got;
					Flash = FlashStatus;
					SeenFlash = 1;
					got = 0;
					Conceal = 0;
					goto next_char;
				}

				/*
				 * standard height
				 *
				 * FIX ME
				 *
				 * I'm sure the handling of this isn't quite right, but it's
				 * reasonably close for the moment.
				 * 
				 * Problems will, I believe, occur if the teletext DH/SH
				 * control codes do not appear in the same position on
				 * the second line.
				 *
				 */

				if ( c == TT_NORMAL_HEIGHT )
				{
					line [ got++ ] = HoldMode ? HoldCharacter : ' ';
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					hpos += got;
					got = 0;
					XSetFont ( dpy, TtextTextGC, TtextText );
					if ( ContGraphics )
						XSetFont ( dpy, TtextMosaicGC, TtextContiguousMosaic );
					else
						XSetFont ( dpy, TtextMosaicGC, TtextSeparateMosaic );
					goto next_char;
				}

				/*
				 * double height
				 *
				 * FIX ME
				 *
				 * I'm sure the handling of this isn't quite right, but it's
				 * reasonably close for the moment.
				 *
				 * Problems will, I believe, occur if the teletext DH/SH
				 * control codes do not appear in the same position on
				 * the second line.
				 */

				if ( c == TT_DOUBLE_HEIGHT )
				{
					line [ got++ ] = ' ';
					HoldCharacter = ' ';
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					hpos += got;
					got = 0;
					if ( DblHeight == DH_LOWER )
					{
						XSetFont ( dpy, TtextTextGC, TtextTextDblL );
						if ( ContGraphics )
							XSetFont ( dpy, TtextMosaicGC,
											TtextContiguousMosaicDblL );
						else
							XSetFont ( dpy, TtextMosaicGC,
											TtextSeparateMosaicDblL );
					}
					else
					{
						DblHeight = DH_UPPER;
						XSetFont ( dpy, TtextTextGC, TtextTextDblU );
						if ( ContGraphics )
							XSetFont ( dpy, TtextMosaicGC,
											TtextContiguousMosaicDblU );
						else
							XSetFont ( dpy, TtextMosaicGC,
											TtextSeparateMosaicDblU );
					}

					goto next_char;
				}

				/*
				 * change graphics colour
				 *
				 * FIX ME
				 *
				 * Does the colour change before or after the hold character ?
				 */

				if ( c < TT_CONCEAL )
				{
					line [ got++ ] = Conceal ? ' ' : HoldCharacter;
					Conceal = 0;
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );

					/*
					 * FIX ME
					 *
					 * I'm making the assumption here that contiguous graphics
					 * will be enabled until a separated graphics code has been
					 * seen, which is then enabled until EOL or another
					 * contiguous control is seen.
					 *
					 * Really out to check that out on the beeb, though.
					 */

					CurrGC = TtextMosaicGC;
					XSetForeground ( dpy, CurrGC, Cells [ c & 7 ]);
					LastColour = c & 7;
					hpos += got;
					got = 0;
					goto next_char;
				}

				/*
				 * conceal display.
				 */

				if ( c == TT_CONCEAL )
				{
					Conceal = 1;
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					hpos += got;
					got = 1;
					line [ 0 ] = ' ';
					HoldCharacter = ' ';
					goto next_char;
				}

				/*
				 * contiguous graphics
				 */

				if ( c == TT_CONTIGUOUS )
				{
					line [ got++ ] = HoldMode ? HoldCharacter : ' ';
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					XSetFont ( dpy, TtextMosaicGC, TtextContiguousMosaic );
					hpos += got;
					got = 0;
					goto next_char;
				}

				/*
				 * separate graphics
				 */

				if ( c == TT_SEPARATED )
				{
					line [ got++ ] = HoldMode ? HoldCharacter : ' ';
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					XSetFont ( dpy, TtextMosaicGC, TtextSeparateMosaic );
					hpos += got;
					got = 0;
					goto next_char;
				}

				/*
				 * change background colour to black
				 */

				if ( c == TT_BLACK_BG )
				{
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					hpos += got;
					got = 1;
					line [ 0 ] = ' ';
					XSetBackground ( dpy, TtextTextGC, Black );
					XSetBackground ( dpy, TtextMosaicGC, Black );
					goto next_char;
				}

				/*
				 * Change background colour
				 */

				if ( c == TT_NEW_BG )
				{
					XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
					hpos += got;
					got = 1;
					line [ 0 ] = ' ';
					XSetBackground ( dpy, TtextTextGC, Cells [ LastColour ]);
					XSetBackground ( dpy, TtextMosaicGC, Cells [ LastColour ]);
					goto next_char;
				}

				/*
				 * Hold graphics
				 *
				 * This is supposed to make a subsequent colour control code
				 * appear the same as the previously displayed character.
				 *
				 */

				if ( c == TT_HOLD_GRAPHICS )
				{
					/*
					 * POSSIBLE ENHANCEMENT ?
					 *
					 * Shouldn't really address memory directly
					 * here.
					 */

					HoldMode = 1;

					/*
					 * This character is only displayed as non-space if
					 * we're currently in graphics mode
					 */

					HoldCharacter = ( CurrGC == TtextTextGC ) ? ' ' :
#ifdef	MODEL_B_ONLY
											Mem [ p - 2 ];
#else
											Mem [ ( p  & MaxRAMAddress ) - 2 ];
#endif

					line [ got++ ] = HoldCharacter;
					goto next_char;
				}

				/*
				 * Release graphics
				 *
				 */

				if ( c == TT_REL_GRAPHICS )
				{
					HoldCharacter = ' ';
					HoldMode = 0;
					line [ got++ ] = ' ';
					goto next_char;
				}

				/*
				 * FIX ME
				 *
				 * shouldn't even get here...
				 */

				line [ got++ ] = ' ';

next_char:

				if ( p == 0x8000 )
					p = StartOfScreenMemory;
			}

			XDrawImageString ( dpy, TeletextScreen, CurrGC, hpos * 12,
											lines * 19, (char *)line, got );
		}
	}

	/*
	 * Restore the cursor to it's correct state
	 */

	DrawCursor();

	ScreenImageChanged = ScreenMemoryChanged = 0;
	XFlush ( dpy );
	return;
}


static void
DrawCursor()
{
	/*
	 * This cursor-handling code is pretty inefficient and it doesn't
	 * handle non-blinking cursors.
	 */

	if ( CursorEnabled && CursorBlinkEnable )
	{
		if ( CursorTimer == 0 )
		{
			if ( CursorOnScreen )
			{
				/*
				 * Remove the cursor from the screen, update the position
				 * and size.
				 */

				CursorOnScreen = 0;
				if ( CursorViewable )
					XFillRectangle ( dpy, TeletextScreen, CursorGC, CursorX,
						CursorY, CursorWidth, CursorDepth );

				if ( CursorMoved )
				{
					CursorX = ( NewCursorX - MasterCursorWidth ) * 12;
					CursorY = NewCursorY * 19 + CursorStartLine;
					CursorMoved = 0;
				}

				if ( CursorResized )
				{
					if ( CursorStartLine > 19 || ( CursorByteWidth == 0 &&
							MasterCursorWidth == 0 ) || CursorEndLine <
															CursorStartLine )
						CursorViewable = 0;
					else
						CursorViewable = 1;
					CursorY = NewCursorY * 19 + CursorStartLine;
					CursorDepth = MIN(19, CursorEndLine - CursorStartLine + 1);
					CursorWidth = ( CursorByteWidth - 1 + MasterCursorWidth )
																		* 12;
					CursorResized = 0;
				}
			}
			else
			{
				/*
				 * Update the position and size of the cursor,
				 * then redraw it
				 */

				CursorOnScreen = 1;
				if ( CursorMoved )
				{
					CursorX = ( NewCursorX - MasterCursorWidth ) * 12;
					CursorY = NewCursorY * 19 + CursorStartLine;
					CursorMoved = 0;
				}

				if ( CursorResized )
				{
					if ( CursorStartLine > 19 || ( CursorByteWidth == 0 &&
							MasterCursorWidth == 0 ) || CursorEndLine <
															CursorStartLine )
						CursorViewable = 0;
					else
						CursorViewable = 1;
					CursorY = NewCursorY * 19 + CursorStartLine;
					CursorDepth = MIN(19, CursorEndLine - CursorStartLine + 1);
					CursorWidth = ( CursorByteWidth - 1 + MasterCursorWidth )
																		* 12;
					CursorResized = 0;
				}

				if ( CursorViewable )
					XFillRectangle ( dpy, TeletextScreen, CursorGC, CursorX,
						CursorY, CursorWidth, CursorDepth );
			}
		}
		else
		{
			if ( CursorOnScreen )
			{
				/*
				 * Need to update the cursor if it has changed size or
				 * position...
				 */

				if ( CursorMoved || CursorResized )
				{
					if ( CursorViewable )
						XFillRectangle ( dpy, TeletextScreen, CursorGC,
								CursorX, CursorY, CursorWidth, CursorDepth );

					if ( CursorMoved )
					{
						CursorX = ( NewCursorX - MasterCursorWidth ) * 12;
						CursorY = NewCursorY * 19 + CursorStartLine;
						CursorMoved = 0;
					}

					if ( CursorResized )
					{
						if ( CursorStartLine > 19 || ( CursorByteWidth == 0 &&
							MasterCursorWidth == 0 ) || CursorEndLine <
															CursorStartLine )
							CursorViewable = 0;
						else
							CursorViewable = 1;
						CursorY = NewCursorY * 19 + CursorStartLine;
						CursorDepth = MIN ( 19, CursorEndLine -
														CursorStartLine + 1 );
						CursorWidth = ( CursorByteWidth - 1 +
													MasterCursorWidth ) * 12;
						CursorResized = 0;
					}

					if ( CursorViewable )
						XFillRectangle ( dpy, TeletextScreen, CursorGC,
								CursorX, CursorY, CursorWidth, CursorDepth );
				}
			}
			else
			{
				if ( CursorMoved )
				{
					CursorX = ( NewCursorX - MasterCursorWidth ) * 12;
					CursorY = NewCursorY * 19 + CursorStartLine;
					CursorMoved = 0;
				}

				if ( CursorResized )
				{
					if ( CursorStartLine > 19 || ( CursorByteWidth == 0 &&
							MasterCursorWidth == 0 ) || CursorEndLine <
															CursorStartLine )
						CursorViewable = 0;
					else
						CursorViewable = 1;
					CursorY = NewCursorY * 19 + CursorStartLine;
					CursorDepth = MIN ( 19, CursorEndLine -
													CursorStartLine + 1 );
					CursorWidth = ( CursorByteWidth - 1 + MasterCursorWidth )
																		* 12;
					CursorResized = 0;
				}
			}
		}
	}
	else
	{
		if ( CursorOnScreen && CursorViewable )
		{
			XFillRectangle ( dpy, TeletextScreen, CursorGC, CursorX, CursorY,
													CursorWidth, CursorDepth );
			CursorOnScreen = 0;
		}
	}
}


void
ChangeTeletextWindowX ( int newx )
{
	TeletextWindowX = newx;
	XMoveWindow ( dpy, TeletextScreen, TeletextWindowX, TeletextWindowY );
	XFlush ( dpy );
}


void
ChangeTeletextWindowY ( int newy )
{
	TeletextWindowY = newy;
	XMoveWindow ( dpy, TeletextScreen, TeletextWindowX, TeletextWindowY );
	XFlush ( dpy );
}
