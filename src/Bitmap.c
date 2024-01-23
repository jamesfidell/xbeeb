/*
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

#ifdef	MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

byteval					BitsForColourInfo;
int						BitmapWindowX = 0;
int						BitmapWindowY = 0;

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

#define	MIN(x,y)		((( x ) < ( y )) ? ( x ) : ( y ))

#ifdef	MITSHM
static unsigned int		Xmax = 0, Xmin = 639, Ymin = 511, Ymax = 0;
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
	char					*ip;
#ifdef	MITSHM
	unsigned int			Xcurr = 0;
#else
	unsigned int			hpos = 0;
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

#ifdef	MITSHM
	ip = ImageData + ( scanline2 * BytesPerImageLine );
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
		for ( p = 0; p < 640; p++, ip++ )
		{
			*ip = Cells [ 0 ];
			ip [ BytesPerImageLine ] = Cells [ 0 ];
		}
		Xmin = 0;
		Xmax = 639;
		if ( scanline2 < Ymin )
			Ymin = scanline2;
		if ( scanline2 > Ymax )
			Ymax = scanline2 + 1;
#else
		XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ 0 ], 0, scanline2,
																	639, 2 );
#endif	/* MITSHM */
		return;
	}

	p = ( scanline / ScanLinesPlus1 ) * HorizDisplayed8 +
		scanline % ScanLinesPlus1 + TopOfScreen;
	if ( p > 0x8000 )
		p -= ScreenLength;

	for ( byte = 0; byte < HorizDisplayed; byte++, p += 8 )
	{
		if ( p >= 0x8000 )
			p -= ScreenLength;

		if ( ScreenCheck [ p ] )
		{
			/*
			 * POSSIBLE ENHANCEMENT ?
			 *
	 		 * Really shouldn't address memory directly here because we
			 * don't know what effect it might have on the system if it's
			 * mapped to somewhere strange.  It's a damn sight faster,
			 * though.
			 */
	
#ifdef	MODEL_B_ONLY
			colour_info = Mem [ p ];
#else
			colour_info = Mem [ p & MaxRAMAddress ];
#endif
#ifdef	MITSHM
			for ( pix = 0; pix < PixelsPerByte; pix++ )
			{
				colour = DecodeColour ( colour_info, pix );

				/*
				 * FIX ME
				 *
				 * There's a problem here that causes a line to be left on
				 * the far right-hand side of the screen.  Don't know
				 * what it is yet.
				 */

				if ( *ip != Cells [ colour ] )
				{
					for ( h = 0; h < PixelWidth; h++, ip++ )
					{
						*ip = Cells [ colour ];
						ip [ BytesPerImageLine ] = Cells [ colour ];
					}
					ScreenImageChanged = 1;
					if ( Xcurr < Xmin )
						Xmin = Xcurr;
					Xcurr += PixelWidth;
					if ( Xcurr > Xmax )
						Xmax = Xcurr - 1;
				}
				else
				{
					Xcurr += PixelWidth;
					ip += PixelWidth;
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
			ip += ( PixelsPerByte * PixelWidth );
			Xcurr += PixelWidth;
		}
#else	/* MITSHM */
		hpos += ByteWidth;
#endif	/* MITSHM */
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

	if ( ScreenImageChanged )
	{
#ifdef	MITSHM
		/*
		 * FIX ME
		 *
		 * This doesn't work properly because of interactions that I don't
		 * yet understand...  So, set the values to copy the whole screen.
		 * 
		 */
	
		Xmax = 639;
		Ymax = 511;
		Xmin = Ymin = 0;
		if ( Xmin <= Xmax && Ymin <= Ymax )
		{
			unsigned int		Xsize = Xmax - Xmin + 1;
			unsigned int		Ysize = Ymax - Ymin + 1;

			XShmPutImage ( dpy, BitmapScreen, CopyAreaGC, BitmapImage,
					Xmin, Ymin, Xmin, Ymin, Xsize, Ysize, False );
			Xmax = Ymax = 0;
			Xmin = 639;
			Ymin = 511;
			XFlush ( dpy );
		}
#else
		XCopyArea ( dpy, BitmapPixmap, BitmapScreen, CopyAreaGC,
													0, 0, 640, 512, 0, 0 );
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
