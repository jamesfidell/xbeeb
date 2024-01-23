/*
 *
 * $Id: Screen.c,v 1.24 2002/01/15 15:46:43 james Exp $
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
 * $Log: Screen.c,v $
 * Revision 1.24  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.23  2002/01/14 22:18:51  james
 * Added support for .inf files
 *
 * Revision 1.22  2002/01/13 23:29:46  james
 * Unbreak MODE7 cursor
 *
 * Revision 1.21  2002/01/13 22:27:19  james
 * Fix compile-time warnings
 *
 * Revision 1.20  2000/09/07 21:30:39  james
 * Fix coredump
 *
 * Revision 1.19  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.18  2000/08/16 17:41:45  james
 * Changes to work on TrueColor displays
 *
 * Revision 1.17  1996/11/11 23:52:31  james
 * Corrections for non-MITSHM code.
 *
 * Revision 1.16  1996/10/13 22:17:18  james
 * Helps to #include the file that TV_LINES is #defined in :-)
 *
 * Revision 1.15  1996/10/13 22:03:56  james
 * Set up constant TV_LINES (=625) and used that instead of local magic numbers.
 *
 * Revision 1.14  1996/10/13 21:59:57  james
 * Changed all window size/position parameters to #defined values.
 *
 * Revision 1.13  1996/10/12 14:30:25  james
 * Corrections to MITSHM #ifdefs around code freeing up resources on shutdown.
 *
 * Revision 1.12  1996/10/09 23:19:10  james
 * Added support for using the MIT X11 Shared Memory Extensions.
 *
 * Revision 1.11  1996/10/09 22:06:55  james
 * Overhaul of the bitmapped screen handling code with particular respect to
 * colour maps.
 *
 * Revision 1.10  1996/10/08 00:12:02  james
 * Display the program name and version on the icon and title bar.
 *
 * Revision 1.9  1996/10/08 00:04:33  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.8  1996/10/01 22:10:02  james
 * Split keyboard handling into kEYMAP_STRICT and KEYMAP_LEGEND models.
 *
 * Revision 1.7  1996/09/24 23:05:42  james
 * Update copyright dates.
 *
 * Revision 1.6  1996/09/23 16:09:52  james
 * Initial implementation of bitmap MODEs -- including modification of
 * screen handling to use different windows for teletext and bitmapped
 * modes and corrections/improvements to colour- and cursor-handling
 * code.
 *
 * Revision 1.5  1996/09/22 19:23:21  james
 * Add the emulated filing system code.
 *
 * Revision 1.4  1996/09/21 23:16:13  james
 * Loading of new X fonts for double height.  Unloading of all fonts.
 *
 * Revision 1.3  1996/09/21 22:39:53  james
 * Improved handling of instruction disassembly.
 *
 * Revision 1.2  1996/09/21 22:13:49  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:40  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "Config.h"
#include "Beeb.h"
#include "Screen.h"
#include "Crtc.h"
#include "Modes.h"
#include "Teletext.h"
#include "Bitmap.h"
#include "InfoWindow.h"
#include "Keyboard.h"
#include "Keymap.h"
#include "Memory.h"
#include "Patchlevel.h"

#ifdef	EMUL_FS
#include "EmulFS.h"
#endif

#ifdef	DGA
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>
#else	/* DGA */
#ifdef	MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif	/* MITSHM */
#endif	/* DGA */


Display				*dpy;
Window				BeebScreen;
Window				InfoWindow;
GC					CursorGC, InfoWindowGC;
Colormap			DefCmap;
Font				InfoWindowFont;
unsigned long		InfoWindowRed, InfoWindowWhite, InfoWindowBlack;

/*
 * Stuff for the teletext MODE
 */

GC					TtextTextGC;
GC					TtextMosaicGC;

Window				TeletextScreen;

Font				TtextText;
Font				TtextTextDblU;
Font				TtextTextDblL;
Font				TtextContiguousMosaic;
Font				TtextContiguousMosaicDblU;
Font				TtextContiguousMosaicDblL;
Font				TtextSeparateMosaic;
Font				TtextSeparateMosaicDblU;
Font				TtextSeparateMosaicDblL;

/*
 * Stuff for bitmap MODEs
 */

GC					BitmapGC [ 16 ];
GC					CopyAreaGC;
Window				BitmapScreen;
Pixmap				BitmapPixmap;
int					BytesPerImageLine;

/*
 * General colour-handling stuff
 */

unsigned long		Cells [ 16 ];
unsigned long		Masks [ 4 ];
unsigned long		ColourBits;

unsigned char		ScreenMemoryChanged = 0;
unsigned char		ScreenImageChanged = 0;

int					RgbValues [ 8 ][ 3 ] =
{
	{ 0x0000, 0x0000, 0x0000 },		/* 0 = Black */
	{ 0xffff, 0x0000, 0x0000 },		/* 1 = Red */
	{ 0x0000, 0xffff, 0x0000 },		/* 2 = Green */
	{ 0xffff, 0xffff, 0x0000 },		/* 3 = Yellow */
	{ 0x0000, 0x0000, 0xffff },		/* 4 = Blue */
	{ 0xffff, 0x0000, 0xffff },		/* 5 = Magenta */
	{ 0x0000, 0xffff, 0xffff },		/* 6 = Cyan */
	{ 0xffff, 0xffff, 0xffff }		/* 7 = White */
};

static const char	*Colours[8] = {
	"black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"
};


static unsigned int	ScreenLengths [ 4 ] =
{
	0x4000,		/* MODE 3 */
	0x5000,		/* MODES 0, 1, 2 */
	0x2000,		/* MODE 6 */
	0x2800		/* MODE 4,5 */
};

byteval					ScreenLengthIndex = 0;

/*
 * Just for the window title...
 */

#define	TITLE_STRING	"Xbeeb v%d.%d.%d"


#ifdef	MITSHM
unsigned char			UseSharedXImage = 0;
unsigned char			UneSharedPixmap = 0;
XImage					*BitmapImage;
static XShmSegmentInfo	SharedSegInfo;
#endif /* MITSHM */


void
InitialiseScreen()
{
	int					i;
	XColor				colour, scol, xcol;
	Window				Root;
	XGCValues			CursorGCValues;
	unsigned long		PlaneMask = ~0;
	int					DefScreen, DefDepth, visuals, VisualClass;
	Visual				*DefVisual;
	XVisualInfo			VisualTmpl, *MyVisual;
	GC					DefGC;
	XWindowChanges		Raise;
	char				TitleString [ 80 ];
	XSizeHints			SizeHints;
#ifdef	MITSHM
	int					PixmapFormat, dummy;
	Bool				SharedPixmapSupport;
#endif


	if (( dpy = XOpenDisplay ( 0 )) == 0 )
	{
		fprintf ( stderr, "Couldn't open connection to display\n" );
		exit ( 1 );
	}

	Root = DefaultRootWindow ( dpy );
	DefScreen = DefaultScreen ( dpy );
	DefVisual = DefaultVisual ( dpy, DefScreen );
	DefDepth = DefaultDepth ( dpy, DefScreen );
	DefGC = DefaultGC ( dpy, DefScreen );

	if ( DefDepth < 3 ) {
		fprintf ( stderr, "Screen depth of %d is not supported\n", DefDepth );
		exit ( 1 );
	}

	VisualTmpl.visual = DefVisual;
	VisualTmpl.screen = DefScreen;
	VisualTmpl.depth = DefDepth;
	MyVisual = XGetVisualInfo ( dpy, VisualScreenMask | VisualDepthMask,
		&VisualTmpl, &visuals );
	if ( visuals != 1 ) {
		fprintf ( stderr, "can't handle default screen with multiple "
			"visuals\n" );
		exit ( 1 );
	}
	VisualClass = MyVisual->class;
	XFree ( MyVisual );

	/*
	 * FIX ME
	 *
	 * Check for errors here...
	 */

	DefCmap = DefaultColormap ( dpy, DefScreen );

	XAllocNamedColor ( dpy, DefCmap, "red", &scol, &xcol );
	InfoWindowRed = scol.pixel;
	InfoWindowBlack = BlackPixel ( dpy, DefScreen );
	InfoWindowWhite = WhitePixel ( dpy, DefScreen );

	if ( VisualClass == PseudoColor ) {
		/*
	 	* Get four planes in the colourmap
	 	*/

		if ( XAllocColorCells( dpy, DefCmap, False, Masks, 4,
			&ColourBits, 1 ) == 0)
		{
			fprintf ( stderr, "Failed to allocate colour planes\n" );
			exit ( 1 );
		}

		for ( i = 0; i < 16; i++ )
		{
			Cells [ i ] = ColourBits | ( i & 1 ? Masks [ 0 ] : 0 ) |
				( i & 2 ? Masks [ 1 ] : 0 ) | ( i & 4 ? Masks [ 2 ] : 0 ) |
				( i & 8 ? Masks [ 3 ] : 0 );
			colour.pixel = Cells [ i ];
			colour.red = RgbValues [ i % 8 ][ 0 ];
			colour.green = RgbValues [ i % 8 ][ 1 ];
			colour.blue = RgbValues [ i % 8 ][ 2 ];
			colour.flags = DoRed | DoGreen | DoBlue;
			XStoreColor ( dpy, DefCmap, &colour );
		}

		PlaneMask = ( Masks [ 0 ] | Masks [ 1 ] | Masks [ 2 ] | Masks [ 3 ] ) &
					~( Masks [ 0 ] & Masks [ 1 ] & Masks [ 2 ] & Masks [ 4 ] );
	} else if ( VisualClass == TrueColor ) {
		Cells[0] = BlackPixel ( dpy, DefScreen );
		Cells[7] = WhitePixel ( dpy, DefScreen );
		Cells[8] = BlackPixel ( dpy, DefScreen );
		Cells[15] = WhitePixel ( dpy, DefScreen );
		for ( i = 1; i < 7; i++ ) {
			XAllocNamedColor ( dpy, DefCmap, Colours[i], &scol, &xcol );
			Cells[i] = Cells[i+8] = scol.pixel;
		}
	} else {
		fprintf ( stderr, "Can't handle visual class %d\n", VisualClass );
		exit ( 1 );
	}

	/*
	 * FIX ME
	 *
	 * Should check for errors...
	 * How about window attributes, too ?
	 * Possibly don't need all these windows, either -- using Pixmaps
	 * directly.
	 */

	BeebScreen = XCreateSimpleWindow ( dpy, Root, 0, 0, MAIN_WIN_W,
											MAIN_WIN_H, 0, White, Black );
	InfoWindow = XCreateSimpleWindow ( dpy, BeebScreen, INFO_WIN_X,
					INFO_WIN_Y, INFO_WIN_W, INFO_WIN_H, 0, White, Black );
	TeletextScreen = XCreateSimpleWindow ( dpy, BeebScreen, TeletextWindowX,
					TeletextWindowY, TTXT_WIN_W, TTXT_WIN_H, 0, White, Black );
	BitmapScreen = XCreateSimpleWindow ( dpy, BeebScreen, BitmapWindowX,
					BitmapWindowY, BMAP_WIN_W, BMAP_WIN_H, 0, White, Black );

	(void) sprintf ( TitleString, TITLE_STRING, VERSION, RELEASE, PATCHLEVEL );

	SizeHints.flags = PSize;
	SizeHints.width = MAIN_WIN_W;
	SizeHints.height = MAIN_WIN_H;

	XSetStandardProperties ( dpy, BeebScreen, TitleString, TitleString, None,
														0, 0, &SizeHints );

#ifdef	MITSHM

	/*
	 * See if we can use shared memory
	 */

    if ( XShmQueryVersion ( dpy, &dummy, &dummy, &SharedPixmapSupport ))
	{
		if ( SharedPixmapSupport )
		{
			PixmapFormat = XShmPixmapFormat ( dpy );

			/*
			 * FIX ME
			 *
			 * Should check return codes here.
			 */

			BitmapImage = XShmCreateImage ( dpy, DefVisual, DefDepth, ZPixmap,
				0, &SharedSegInfo, BMAP_WIN_W, TV_LINES );

			SharedSegInfo.shmid = shmget ( IPC_PRIVATE,
				BitmapImage -> bytes_per_line * BitmapImage -> height,
				IPC_CREAT | 0777 );

			SharedSegInfo.shmaddr = BitmapImage -> data =
				shmat ( SharedSegInfo.shmid, 0, 0 );

			memset ( SharedSegInfo.shmaddr, 0, BitmapImage -> bytes_per_line *
												BitmapImage -> height );

			SharedSegInfo.readOnly = False;

			XShmAttach ( dpy, &SharedSegInfo );

			/*
			 * FIX ME
			 *
			 * This should be removed.
			 */

			BitmapPixmap = XShmCreatePixmap ( dpy, BitmapScreen,
				SharedSegInfo.shmaddr, &SharedSegInfo, BMAP_WIN_W,
														TV_LINES, DefDepth );

			BytesPerImageLine = BitmapImage -> bytes_per_line;
		}
		else
		{
			fprintf ( stderr, "but no shared Pixmaps :-(\n" );
		}
	}
	else
	{
		fprintf ( stderr, "Your server doesn't support shared memory\n" );
		fprintf ( stderr, "Expect Xbeeb to run a little slowly\n" );

#endif	/* MITSHM */

	/*
	 * The Pixmap for the bitmapped screen is actually bigger than the
	 * bitmapped screen -- this is because I handle the Pixmap as if
	 * it were the whole scan-line area, whereas the bitmapped screen
	 * is just the bit that can be seen on the display.
	 */

	BitmapPixmap = XCreatePixmap ( dpy, BitmapScreen, BMAP_WIN_W,
													BMAP_WIN_H, DefDepth );
	

#ifdef	MITSHM
	}
#endif

	/*
	 * Set up the InfoWindow so that it is always at the top of the
	 * window stack.
	 */

	Raise.stack_mode = TopIf;
	XConfigureWindow ( dpy, InfoWindow, CWStackMode, &Raise );

	/*
	 * Set up the colourmaps for the display windows.  BeebScreen and
	 * InfoWindow use the default map.
	 */

	XSetWindowColormap ( dpy, TeletextScreen, DefCmap );
	XSetWindowColormap ( dpy, BitmapScreen, DefCmap );

	/*
	 * FIX ME
	 *
	 * Should I stop these events propagating to the root window ?
	 */

	XSelectInput ( dpy, BeebScreen, KeyPressMask | KeyReleaseMask | 
						ExposureMask | EnterWindowMask | LeaveWindowMask );
	XSelectInput ( dpy, TeletextScreen, ExposureMask );
	XSelectInput ( dpy, BitmapScreen, ExposureMask );
	XSelectInput ( dpy, InfoWindow, ExposureMask );

	/*
	 * FIX ME
	 *
	 * Need to do these for the Bitmap screen too ?
	 */

	CursorGC = XCreateGC ( dpy, BeebScreen, None, 0 );
	XCopyGC ( dpy, DefGC, ~(( unsigned long ) 0), CursorGC );

	CursorGCValues.function = GXxor;
	CursorGCValues.foreground = White;
	CursorGCValues.background = Black;
	CursorGCValues.fill_style = FillSolid;
	CursorGCValues.plane_mask = PlaneMask;

	XChangeGC ( dpy, CursorGC, GCFunction | GCForeground | GCBackground |
		GCFillStyle | GCPlaneMask, &CursorGCValues );

	/*
	 * FIX ME
	 *
	 * should check for errors when loading fonts.
	 */

	TtextText = XLoadFont ( dpy, "ttext-std" );
	TtextTextDblU = XLoadFont ( dpy, "ttext-std-udh" );
	TtextTextDblL = XLoadFont ( dpy, "ttext-std-ldh" );
	TtextContiguousMosaic = XLoadFont ( dpy, "ttext-grc" );
	TtextContiguousMosaicDblU = XLoadFont ( dpy, "ttext-grc-udh" );
	TtextContiguousMosaicDblL = XLoadFont ( dpy, "ttext-grc-ldh" );
	TtextSeparateMosaic = XLoadFont ( dpy, "ttext-grs" );
	TtextSeparateMosaicDblU = XLoadFont ( dpy, "ttext-grs-udh" );
	TtextSeparateMosaicDblL = XLoadFont ( dpy, "ttext-grs-ldh" );
	InfoWindowFont = XLoadFont ( dpy, "7x13bold" );

	TtextTextGC = XCreateGC ( dpy, TeletextScreen, None, 0 );
	TtextMosaicGC = XCreateGC ( dpy, TeletextScreen, None, 0 );
	InfoWindowGC = XCreateGC ( dpy, InfoWindow, None, 0 );

	/*
	 * Set up the 17 bitmap GCs -- for each of the 16 different
	 * colours and the CopyAreaGC to prevent GraphicsExposure event
	 * checking when updating the BitmapScreen.
	 */

	for ( i = 0; i < 16; i++ )
	{
		BitmapGC [ i ]  = XCreateGC ( dpy, BitmapPixmap, None, 0 );
		XCopyGC ( dpy, DefGC, 0xffff, BitmapGC [ i ] );
		XSetForeground ( dpy, BitmapGC [ i ], Cells [ i ] );
	}
	CopyAreaGC =  XCreateGC ( dpy, BitmapPixmap, None, 0 );
	XCopyGC ( dpy, DefGC, 0xffff, CopyAreaGC );
	XSetGraphicsExposures ( dpy, CopyAreaGC, False );

	XCopyGC ( dpy, DefGC, 0xffff, TtextTextGC );
	XCopyGC ( dpy, DefGC, 0xffff, TtextMosaicGC );

	XSetFont ( dpy, TtextTextGC, TtextText );
	XSetFont ( dpy, TtextMosaicGC, TtextContiguousMosaic );

	/*
	 * Set up the InfoWindow GC
	 */

	XCopyGC ( dpy, DefGC, 0xffff, InfoWindowGC );
	XSetFont ( dpy, InfoWindowGC, InfoWindowFont );

	/*
	 * Now clear the main Beeb window and the bitmapped screen to black.
	 */

	XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ 0 ], 0, 0, MAIN_WIN_W - 1,
															MAIN_WIN_H - 1 );
	XClearWindow ( dpy, BeebScreen );
	XClearWindow ( dpy, InfoWindow );

	XMapRaised ( dpy, BeebScreen );
	XMapRaised ( dpy, InfoWindow );
	XFlush ( dpy );
	return;
}


void
ShutdownScreen()
{
	XUnloadFont ( dpy, TtextText );
	XUnloadFont ( dpy, TtextTextDblU );
	XUnloadFont ( dpy, TtextTextDblL );
	XUnloadFont ( dpy, TtextContiguousMosaic );
	XUnloadFont ( dpy, TtextContiguousMosaicDblU );
	XUnloadFont ( dpy, TtextContiguousMosaicDblL );
	XUnloadFont ( dpy, TtextSeparateMosaic );
	XUnloadFont ( dpy, TtextSeparateMosaicDblU );
	XUnloadFont ( dpy, TtextSeparateMosaicDblL );

#ifdef	MITSHM
	XShmDetach ( dpy, &SharedSegInfo );
#endif
	XFreePixmap ( dpy, BitmapPixmap );
#ifdef	MITSHM
	shmdt ( SharedSegInfo.shmaddr );
	shmctl ( SharedSegInfo.shmid, IPC_RMID, 0 );

	XDestroyImage ( BitmapImage );
#endif

	/*
	 * FIX ME
	 *
	 * Should destroy all the SimpleWindows here ?
	 */

	XAutoRepeatOn ( dpy );
	XCloseDisplay ( dpy );
	return;
}


void
CheckEvents()
{
	XEvent			xevent;

	if ( XEventsQueued ( dpy, QueuedAfterReading ))
	{
		XNextEvent ( dpy, &xevent );
		switch ( xevent.type )
		{
			case Expose :
				if ( xevent.xexpose.window == InfoWindow )
					InfoWindowRedraw();
				else
				{
					/*
					 * Need to re-display the screen.
					 */

					ScreenImageChanged = 1;
					( void ) memset (( void* ) ScreenCheck, 1, 32768 );
				}
				break;

			case KeyPress :
				HandleKey (( XKeyEvent* ) &xevent, KEY_PRESSED );
				break;

			case KeyRelease :
				HandleKey (( XKeyEvent* ) &xevent, KEY_RELEASED );
				break;

			case EnterNotify :
				XAutoRepeatOff ( dpy );
				XFlush ( dpy );
				break;

			case LeaveNotify :
				XAutoRepeatOn ( dpy );
				XFlush ( dpy );
				break;

			default :
				/*
				 * FIX ME
				 *
				 * All other events are masked out, so this should
				 * barf big-time.
				 */

				break;
		}
	}
	return;
}


void
ScreenAddressStartHi ( byteval bit )
{
	if ( bit )
		ScreenLengthIndex |= 0x2;
	else
		ScreenLengthIndex &= 0x1;

	RecalculateScreenInfo();
	return;
}


void
ScreenAddressStartLo ( byteval bit )
{
	if ( bit )
		ScreenLengthIndex |= 0x1;
	else
		ScreenLengthIndex &= 0x2;

	RecalculateScreenInfo();
	return;
}


void
RecalculateScreenInfo()
{
	/*
	 * FIX ME
	 *
	 * I really don't have a clue how the MODE 7 screen length stuff
	 * works, so I've just hacked it here...
	 */

	if ( CurrentScreenMode == MODE_TELETEXT )
		ScreenLength = 0x400;
	else
		ScreenLength = ScreenLengths [ ScreenLengthIndex ];

	StartOfScreenMemory = 0x8000 - ScreenLength;
	return;
}
