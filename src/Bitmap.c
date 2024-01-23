/*
 *
 * $Id: Bitmap.c,v 1.29 2002/01/15 15:46:43 james Exp $
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
 * $Log: Bitmap.c,v $
 * Revision 1.29  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.28  2002/01/13 22:27:19  james
 * Fix compile-time warnings
 *
 * Revision 1.27  2000/09/07 23:10:34  james
 * Tidyups and correct fix for not correctly handling bitmap screens where
 * existing colour matches current
 *
 * Revision 1.26  2000/09/07 22:05:09  james
 * Fix bug with drawing new pixels on the screen (not checking that both
 * components of the pixel needed to change colour).
 *
 * Revision 1.25  2000/09/07 21:59:03  james
 * Add FASTHOST configurable.  Make bitmap displays read data properly rather
 * than direct from the memory array when this is enabled.
 *
 * Revision 1.24  2000/09/07 21:30:39  james
 * Fix coredump
 *
 * Revision 1.23  2000/08/17 22:57:43  james
 * Make screen updates work when they're restricted to only the area of the
 * display that has changed.
 *
 * Revision 1.22  2000/08/16 17:58:26  james
 * Update copyright message
 *
 * Revision 1.21  2000/08/16 17:41:45  james
 * Changes to work on TrueColor displays
 *
 * Revision 1.20  1996/11/25 00:01:16  james
 * Remove the "Default Vertical Sync value" magic number.
 *
 * Revision 1.19  1996/11/24 21:54:07  james
 * Change MIN to XBEEB_MIN to avoid clashes with some OSes.
 *
 * Revision 1.18  1996/11/18 01:01:27  james
 * Changes to comments.
 *
 * Revision 1.17  1996/11/16 23:49:54  james
 * Only update blanked lines if they don't already have the correct colour.
 *
 * Revision 1.16  1996/11/11 23:52:30  james
 * Corrections for non-MITSHM code.
 *
 * Revision 1.15  1996/11/09 23:16:05  james
 * Clear scan line to end of screen display in non-shared memory
 * configuration.
 *
 * Revision 1.14  1996/11/09 23:02:27  james
 * Take account of the horizontal start of display based on the clock rate
 * set in the Video ULA and the CRTC Horiz. Sync. register.
 *
 * Revision 1.13  1996/11/09 22:32:53  james
 * Updated bitmapped screen-handling code to take account of the Vertical
 * Adjust register in the CRTC.
 *
 * Revision 1.12  1996/11/07 23:41:36  james
 * Add CVS headers.
 *
 *
 */


#include <stdio.h>
#include <memory.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "Config.h"
#include "Bitmap.h"
#include "Beeb.h"
#include "Memory.h"
#include "Screen.h"
#include "Modes.h"
#include "Crtc.h"
#include "VideoUla.h"
#include "Sheila.h"

#ifdef	MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

byteval					BitsForColourInfo;
int						BitmapWindowX = BMAP_WIN_X;
int						BitmapWindowY = BMAP_WIN_Y;

unsigned short			StartPosnX;

static int				CursorTimer;
static byteval			CursorOnScreen;

static int				LastScanline = -1;

#ifdef	NOT_YET_IMPLEMENTED
static int				CursorX = 255;
static int				CursorY = 255;
static unsigned int		CursorDepth = 255;
static unsigned int		CursorWidth = 255;
static unsigned int		CursorViewable = 0;

static void				DrawCursor();
#endif


#ifdef	MITSHM
static unsigned int		Xmin = BMAP_WIN_W - 1, Xmax = 0;
static unsigned int		Ymin = BMAP_WIN_H - 1, Ymax = 0;
#endif


void
InitialiseBitmap()
{
	if ( CurrentScreenMode != MODE_BITMAP )
	{
		CurrentScreenMode = MODE_BITMAP;

		/*
		 * Need to recalculate the top of screen information here because
		 * it's not going to be right given that teletext screen start
		 * addresses are calculated differently to bitmap screen start
		 * addresses.  The same applies to the cursor address.
		 */

		TopOfScreen = ScreenStartHi << 8;
		TopOfScreen |= ScreenStartLo;
		TopOfScreen *= 8;

		/*
		 * FIX ME
		 *
		 * I'm sure there's going to be a problem here if the cursor
		 * drawing code doesn't realise that the cursor won't be on
		 * the screen when the window pops up.
		 */

		CursorAddress = CursorPosHi << 8;
		CursorAddress |= CursorPosLo;
		CursorAddress *= 8;

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
		 *
		 * This code should be corrected at the same time as the code
		 * in Crtc.c (where I pinched it from).
		 */

		NewCursorX = CursorOffset % CharsPerLine;
		NewCursorY = CursorOffset / CharsPerLine;

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
	 * Default vertical sync. position (so we put the window somewhere
	 * sane on the screen...
	 */

	DefaultVertSync = 35;

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
	unsigned int			p, byte, h, scanline2;
	byteval					colour_info, pix, colour;
#ifdef	MITSHM
	unsigned int			dX = 0, dY;
	unsigned int			Xcurr = 0;
#else
	unsigned int			hpos;
#endif	/* MITSHM */

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
	 * All given that TV_LINES = 625 :
	 *
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
	 * The TV_LINES / 2 figure shouldn't come out of thin air here -- it
     * should come from CRTC R4/R5.
	 */

	if (( scanline + 1 ) > ( TV_LINES / 2 ))
		return;

	scanline2 = scanline * 2;

	/*
	 * FIX ME
	 *
	 * Below we cope with the Vertical Adjust value for shared memory
	 * configurations.  Code is required for the non-shared memory
	 * implementation too.
	 */

#ifdef	MITSHM
	dY = scanline2 + VertAdjust2;
	if ( dY > BMAP_WIN_H )
		return;
#endif

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
#ifdef	MITSHM

		/*
		 * FIX ME
		 *
		 * Not only is this flawed... It doesn't work, either.
		 */

		for ( p = 0; p < BMAP_WIN_W; p++ )
		{
			/*
			 * If the image already has the right colour, don't bother
			 * changing it.
			 */

			if (( XGetPixel ( BitmapImage, p, dY )) != Cells[0] )
			{
				XPutPixel ( BitmapImage, p, dY, Cells[0] );
				XPutPixel ( BitmapImage, p, dY+1, Cells[0] );
				ScreenImageChanged = 1;
			}
		}
		Xmin = 0;
		Xmax = BMAP_WIN_W - 1;
		if ( scanline2 < Ymin )
			Ymin = scanline2;
		if ( scanline2 > Ymax )
			Ymax = scanline2 + 1;
#else
		XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ 0 ], 0, scanline2,
														BMAP_WIN_W - 1, 2 );
#endif	/* MITSHM */
		return;
	}

	p = ( scanline / ScanLinesPlus1 ) * HorizDisplayed8 +
		scanline % ScanLinesPlus1 + TopOfScreen;
	if ( p > 0x8000 )
		p -= ScreenLength;

#ifndef	MITSHM
	if (( hpos = StartPosnX ) > 0 )
		 XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ 0 ], 0, scanline2,
																	 hpos, 2 );
#endif	/* MITSHM */

	for ( byte = 0; byte < HorizDisplayed; byte++, p += 8 )
	{
		if ( p >= 0x8000 )
			p -= ScreenLength;

		if ( ScreenCheck [ p ] )
		{
#ifdef	MODEL_B_ONLY
#ifdef	FASTHOST
			colour_info = ReadByte ( p );
#else
			colour_info = Mem [ p ];
#endif	/* FASTHOST */
#else	/* MODEL_B_ONLY */
#ifdef	FASTHOST
			colour_info = ReadByte ( p & MaxRAMAddress );
#else
			colour_info = Mem [ p & MaxRAMAddress ];
#endif	/* FASTHOST */
#endif
#ifdef	MITSHM
			for ( pix = 0; pix < PixelsPerByte; pix++ )
			{
				colour = DecodeColour ( colour_info, pix );
				for ( h = 0; h < PixelWidth; h++, dX++ )
				{
					if ( XGetPixel ( BitmapImage, dX, dY ) != Cells[colour] )
					{
						XPutPixel ( BitmapImage, dX, dY, Cells[colour] );
						XPutPixel ( BitmapImage, dX, dY+1, Cells[colour] );
						ScreenImageChanged = 1;
						if ( Xcurr < Xmin )
							Xmin = Xcurr;
						if ( Xcurr > Xmax )
							Xmax = Xcurr;
					}
					Xcurr++;
				}
			}
#else	/* MITSHM */
			for ( pix = 0, h = hpos; pix < PixelsPerByte;
													pix++, h += PixelWidth )
			{
				colour = DecodeColour ( colour_info, pix );
				XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ colour ], h,
												scanline2, PixelWidth, 2 );
			}
#endif	/* MITSHM */
			ScreenCheck [ p ] = 0;
#ifdef	MITSHM
			if ( ScreenImageChanged )
			{
				if ( scanline2 < Ymin )
					Ymin = scanline2;
				if ( scanline2 > Ymax )
					Ymax = scanline2 + 1;
			}
#else	/* MITSHM */
			ScreenImageChanged = 1;
#endif	/* MITSHM */
		}
#ifdef	MITSHM
		else
		{
			dX += ( PixelsPerByte * PixelWidth );
			Xcurr += PixelWidth;
		}
#else	/* MITSHM */
		hpos += ByteWidth;
#endif	/* MITSHM */
	}

#ifndef	MITSHM
	/*
	 * FIX ME
	 *
	 * This doesn't take account of the fact that the colour shouldn't be
	 * affected by colour map changes -- a bit like the missing scanlines
	 * in MODEs 3 and 6.
	 */

	if ( hpos < BMAP_WIN_X )
		XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ 0 ], hpos, scanline2,
											( BMAP_WIN_X - hpos ), 2 );
#endif	/* MITSHM */

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

	if ( ScreenImageChanged )
	{
#ifdef	MITSHM
		/*
		 * Bit of a hack, this, but we may have set Ymax to be one too
		 * many, above, reduce it if required here.
		 */

		if ( Ymax >= BMAP_WIN_H )
			Ymax = BMAP_WIN_H - 1;

		if ( Xmin <= Xmax && Ymin <= Ymax )
		{
			unsigned int		Xsize = Xmax - Xmin + 1;
			unsigned int		Ysize = Ymax - Ymin + 1;

			XShmPutImage ( dpy, BitmapScreen, CopyAreaGC, BitmapImage,
					Xmin, Ymin, Xmin, Ymin, Xsize, Ysize, False );
			Xmax = Ymax = 0;
			Xmin = BMAP_WIN_W - 1;
			Ymin = BMAP_WIN_H - 1;
			XFlush ( dpy );
		}
#else
		XCopyArea ( dpy, BitmapPixmap, BitmapScreen, CopyAreaGC, BMAP_WIN_X,
								BMAP_WIN_Y, BMAP_WIN_W, BMAP_WIN_H, 0, 0 );
		XFlush ( dpy );
#endif	/* MITSHM */
		ScreenImageChanged = 0;
	}

	return;
}


#ifdef	NOT_YET_IMPLEMENTED
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
					CursorDepth = XBEEB_MIN ( 19, ( CursorEndLine -
														CursorStartLine + 1 ));
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
					CursorDepth = XBEEB_MIN ( 19, ( CursorEndLine -
														CursorStartLine + 1 ));
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
						CursorDepth = XBEEB_MIN ( 19, ( CursorEndLine -
														CursorStartLine + 1 ));
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
					CursorDepth = XBEEB_MIN ( 19, ( CursorEndLine -
													CursorStartLine + 1 ));
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
#endif	/* NOT_YET_IMPLEMENTED */


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
