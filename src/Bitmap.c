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
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Config.h"
#include "Bitmap.h"
#include "Memory.h"
#include "Screen.h"
#include "Modes.h"
#include "Crtc.h"
#include "VideoUla.h"

byteval					BitsForColourInfo;
int						BitmapWindowX = 0;
int						BitmapWindowY = 0;

static int				CursorTimer;
static byteval			CursorOnScreen;

static int				CursorX = 255;
static int				CursorY = 255;
static int				LastScanline = -1;

static unsigned int		CursorDepth = 255;
static unsigned int		CursorWidth = 255;
static unsigned int		CursorViewable = 0;

static void				DrawCursor();

#define	MIN(x,y)	(( x < y ) ? x : y )

static byteval	DecodeColour ( byteval, byteval );


void
InitialiseBitmap()
{
	if ( CurrentScreenMode != MODE_BITMAP )
	{
		CurrentScreenMode = MODE_BITMAP;
		XUnmapWindow ( dpy, TeletextScreen );
		XMapRaised ( dpy, BitmapScreen );
		XFlush ( dpy );
	}

	/*
	 * Make sure that the screen gets updated from the contents of
	 * memory...
	 */

	( void ) memset (( void* ) ScreenCheck, 1, 32768 );

	/*
	 * FIX ME
	 *
	 * And do the rest...
	 */

	CursorTimer = 0;
	CursorOnScreen = 0;

	RecalculateScreenInfo();

	return;
}


void
BitmapScanlineUpdate ( unsigned int scanline )
{
	unsigned int			p, byte, hpos, h, scanline2;
	byteval					colour_info, pix, colour;

	/*
	 * FIX ME
	 *
	 * Handle cursor flashing
	 */

	/*
	 * FIX ME
	 *
	 * an ugly hack to make sure we only update each scanline once.
	 */

	if ( scanline == LastScanline )
		return;
	LastScanline = scanline;

	/*
	 * FIX ME
	 *
	 * I think that scanlines 0 thru' 311 go down the screen one scan line
     * apart , then 312 goes off the bottom and back on the top.  I can't
     * exactly do that because the scan lines are exactly horizontal, so
     * instead scanline 314 goes back to screen line 1 (between scan lines
	 * zero and one).
	 *
	 * FIX ME
	 *
	 * I assume that the interlaced lines (313 onwards) will draw the same
	 * as the line above them, thus I can completely ignore them.
	 *
	 * FIX ME
	 *
	 * The 312 figure shouldn't come out of thin air here -- it should
	 * come from CRTC R4/R5.
	 */

	if ( scanline > 312 )
		return;

	scanline2 = scanline * 2;

	/*
	 * The easy bit -- if the line we're tracing doesn't even cross one of
	 * the 8 lines of a character, then we're done -- all that's required
	 * is to blank out the line.
	 *
	 * If we've gone past the end of the characters we have to display,
	 * then there's no work to do either... (This is set in CRTC R6.)
	 */

	if (( scanline % ScanLinesPlus1 >= 8 ) ||
						scanline >= ( VertDisplayed * ScanLinesPlus1 ))
	{
		XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ 0 ], 0, scanline2,
																	639, 2 );
		return;
	}

	p = ( scanline / ScanLinesPlus1 ) * HorizDisplayed8 +
		scanline % ScanLinesPlus1 + TopOfScreen;
	if ( p > 0x8000 )
		p -= StartOfScreenMemory;

	hpos = 0;
	for ( byte = 0; byte < HorizDisplayed; byte++, p += 8 )
	{
		if ( p >= 0x8000 )
			p -= StartOfScreenMemory;

		if ( ScreenCheck [ p ] )
		{
			/*
			 * POSSIBLE ENHANCEMENT ?
			 *
	 		 * Really shouldn't address memory directly here because we
			 * don't know what effect it might have on the system if it's
			 * mapped to somewhere strange.
			 */
	
			colour_info = Mem [ p ];
			for ( pix = 0, h = hpos; pix < PixelsPerByte;
													pix++, h += PixelWidth )
			{
				colour = DecodeColour ( colour_info, pix );
				XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ colour ], h,
												scanline2, PixelWidth, 2 );
			}
			ScreenCheck [ p ] = 0;
		}
		hpos += ByteWidth;
	}
	return;
}


void
BitmapScreenUpdate()
{
	/*
	 * FIX ME
	 *
	 * The zero values for the origin should change based on the settings
	 * of the CRTC registers.
	 */

	XCopyArea ( dpy, BitmapPixmap, BitmapScreen, CopyAreaGC,
													0, 0, 640, 512, 0, 0 );
	XFlush ( dpy );
	return;
}


static void
DrawCursor()
{
	/*
	 * FIX ME
	 *
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


static byteval
DecodeColour ( byteval info, byteval pixel )
{
	byteval			bits, col;

	switch ( BitsForColourInfo )
	{
		case 1 :
			return (( info >> ( 7 - pixel )) & 0x1 );
			break;
		case 2 :
			bits = ( info >> ( 3 - pixel )) & 0x11;
			switch ( bits )
			{
				case 0x11 :
					col = 3;
					break;
				case 0x10 :
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

	return col;
}


void
ChangeBitmapWindowX ( int newx )
{
	BitmapWindowX = newx;
	XMoveWindow ( dpy, BitmapScreen, BitmapWindowX, BitmapWindowY );
	XFlush ( dpy );
}


void
ChangeBitmapWindowY ( int newy )
{
	BitmapWindowY = newy;
	XMoveWindow ( dpy, BitmapScreen, BitmapWindowX, BitmapWindowY );
	XFlush ( dpy );
}
