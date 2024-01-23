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
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Config.h"
#include "Memory.h"
#include "Screen.h"
#include "Modes.h"
#include "Crtc.h"
#include "VideoUla.h"

static int				FlashTimer;
static int				CursorTimer;
static unsigned char	FlashStatus;
static unsigned char	CursorOnScreen;

static unsigned int		CursorX = 255;
static unsigned int		CursorY = 255;

static unsigned int		CursorDepth = 255;
static unsigned int		CursorWidth = 255;
static unsigned int		CursorViewable = 0;

static void				DrawCursor();

#define	MIN(x,y)	(( x < y ) ? x : y )

void
InitialiseTeletext()
{
	CurrentMode = 7;
	ScreenLength = 0x400;
	FlashTimer = CursorTimer = 0;
	FlashStatus = CursorOnScreen = 0;

	return;
}


void
TeletextScreenUpdate()
{
	unsigned int		charpos, p;
	unsigned char		lines, c, hpos, got, HoldCharacter;
	unsigned char		DblHeight = 0, DblHeightPosn = 0;
	unsigned char		Conceal, Flash, DoFlash = 0;
	unsigned char		line[40];
	unsigned long		LastColour;
	GC					CurrGC;
	static char			SeenFlash = 0;

	/*
	 * Sort out the flash counter first -- we can return if it isn't
	 * time to update flashing colours and the screen hasn't changed.
	 *
	 * FIX ME
	 *
	 * I don't know what the mark and space periods for the teletext
	 * flash are.  This routine gets called every 50th of a second.
	 * I've guessed at values for the time being.
	 */

	if ( ++FlashTimer == 70 )
	{
		DoFlash = SeenFlash;
		FlashStatus = ~FlashStatus;
	}
	else
		if ( FlashTimer == 100 )
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

	if ( ScreenChanged || DoFlash )
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

			LastColour = 7;
			HoldCharacter = ' ';
			Conceal = Flash = 0;

			if (( DblHeightPosn = ( DblHeight + DblHeightPosn ) % 3 ) == 0 )
				DblHeight = 0;

			for ( charpos = 0; charpos < HorizDisplayed; charpos++ )
			{
				c = Mem [ p++ ];

				/*
				 * Non-displayable characters (I think!)
				 */

				if ( c < 32 || c == 128 || c == 138 || c == 139 ||
								( c > 141 && c < 145 ) || c == 155 )
				{
					line [ got++ ] = ' ';
					goto next_char;
				}

				/*
				 * printable characters
				 */

				if ( c < 128 || c > 159 )
				{
					if ( Flash || Conceal )
						line [ got++ ] = ' ';
					else
						line [ got++ ] = c;
					goto next_char;
				}

				/*
				 * New text colour.
				 *
				 * FIX ME
				 *
				 * should this character look like the current hold character ?
				 */

				if ( c < 136 )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );

					CurrGC = TtextTextGC;
					XSetForeground ( dpy, CurrGC, c );
					LastColour = c;
					hpos += got;
					got = 0;
					Conceal = 0;
					goto next_char;
				}

				/*
				 * Steady text
				 */

				if ( c == 136 )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					hpos += got;
					Flash = 0;
					got = 0;
					Conceal = 0;
					goto next_char;
				}

				/*
				 * Flashing text
				 */

				if ( c == 137 )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
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
				 */

				if ( c == 140 )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					hpos += got;
					got = 0;
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
				 * Also, double height graphics isn't catered for yet.
				 *
				 */

				if ( c == 141 )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					hpos += got;
					got = 0;
					if ( DblHeight )
						XSetFont ( dpy, TtextTextGC, TtextTextDblL );
					else
					{
						DblHeight = 1;
						XSetFont ( dpy, TtextTextGC, TtextTextDblU );
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

				if ( c < 152 )
				{
					line [ got++ ] = Conceal ? ' ' : HoldCharacter;
					Conceal = 0;
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );

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
					XSetForeground ( dpy, CurrGC, c );
					LastColour = c;
					hpos += got;
					got = 0;
					goto next_char;
				}

				/*
				 * conceal display.
				 *
				 * FIX ME
				 *
				 * Need to check on the behaviour here, but I believe this
				 * prevents anything being displayed until the next
				 * text or graphics colour control character.
				 */

				if ( c == 152 )
				{
					Conceal = 1;
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					hpos += got;
					got = 1;
					line [ 0 ] = ' ';
					goto next_char;
				}

				/*
				 * contiguous graphics
				 */

				if ( c == 153 )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					XSetFont ( dpy, TtextMosaicGC, TtextContiguousMosaic );
					hpos += got;
					got = 0;
					goto next_char;
				}

				/*
				 * separate graphics
				 */

				if ( c == 154 )
				{
					line [ got++ ] = ' ';
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					XSetFont ( dpy, TtextMosaicGC, TtextSeparateMosaic );
					hpos += got;
					got = 0;
					goto next_char;
				}

				/*
				 * change background colour to black
				 *
				 * FIX ME
				 *
				 * Does the background colour change on the character
				 * postition, or after it ?
				 *
				 */

				if ( c == 156 )
				{
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					hpos += got;
					got = 1;
					line [ 0 ] = ' ';
					XSetBackground ( dpy, TtextTextGC, Black );
					XSetBackground ( dpy, TtextMosaicGC, Black );
					goto next_char;
				}

				/*
				 * Change background colour
				 *
				 * FIX ME
				 *
				 * Where does the background colour start from if the
				 * colour code isn't in the character position immediately
				 * before this one ?
				 *
				 * In fact, in that case, what's the background colour going
				 * to be ?
				 *
				 */

				if ( c == 157 )
				{
					XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12,
													lines * 19, line, got );
					hpos += got;
					got = 1;
					line [ 0 ] = ' ';
					XSetBackground ( dpy, TtextTextGC, LastColour );
					XSetBackground ( dpy, TtextMosaicGC, LastColour );
					goto next_char;
				}

				/*
				 * Hold graphics
				 *
				 * This is supposed to make all subsequent colour control codes
				 * appear the same as the character before this one.
				 *
				 * FIX ME
				 *
				 * What's it supposed to do if it appears at the start of a
				 * line ?
				 *
				 * How does this character and the character before it appear ?
				 *
				 * The contiguous/separated mode should also be kept for hold
				 * characters when they are displayed.
				 */

				if ( c == 158 )
				{
					if ( charpos == 0 )
						HoldCharacter = ' ';
					else
						HoldCharacter = Mem [ p - 2 ];
					line [ got++ ] = ' ';
					goto next_char;
				}

				/*
				 * Release graphics
				 *
				 */

				if ( c == 159 )
				{
					HoldCharacter = ' ';
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

			XDrawImageString ( dpy, BeebScreen, CurrGC, hpos * 12, lines * 19,
																line, got );
		}
	}

	/*
	 * Restore the cursor to it's correct state
	 */

	DrawCursor();

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
					XFillRectangle ( dpy, BeebScreen, CursorGC, CursorX,
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
													MasterCursorWidth == 0 ))
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
													MasterCursorWidth == 0 ))
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
					XFillRectangle ( dpy, BeebScreen, CursorGC, CursorX,
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
						XFillRectangle ( dpy, BeebScreen, CursorGC, CursorX,
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
													MasterCursorWidth == 0 ))
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
						XFillRectangle ( dpy, BeebScreen, CursorGC, CursorX,
										CursorY, CursorWidth, CursorDepth );
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
													MasterCursorWidth == 0 ))
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
			XFillRectangle ( dpy, BeebScreen, CursorGC, CursorX, CursorY,
													CursorWidth, CursorDepth );
			CursorOnScreen = 0;
		}
	}
}
