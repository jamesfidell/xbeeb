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
#include <X11/keysym.h>

#include "Config.h"
#include "Beeb.h"
#include "Screen.h"
#include "Modes.h"
#include "Teletext.h"
#include "Bitmap.h"
#include "Keyboard.h"
#include "Memory.h"
#include "EFS.h"

static void			HandleKey ( XKeyEvent*, signed char );


Display				*dpy;
Window				BeebScreen;
GC					DefaultGraphicsContext;
GC					CursorGC;
Colormap			Cmap;

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

/*
 * General colour-handling stuff
 */

unsigned long		Cells [ 16 ];
unsigned long		Masks [ 4 ];
unsigned long		ColourBits;

unsigned char		ScreenChanged = 0;

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


static unsigned int	ScreenLengths [ 4 ] =
{
	0x4000,		/* MODE 3 */
	0x5000,		/* MODES 0, 1, 2 */
	0x2000,		/* MODE 6 */
	0x2800		/* MODE 4,5 */
};

byteval					ScreenLengthIndex = 0;


void
InitialiseScreen()
{
	int					i;
	XColor				colour;
	Window				Root;
	XGCValues			CursorGCValues;
	unsigned long		PlaneMask;

	if (( dpy = XOpenDisplay ( 0 )) == 0 )
	{
		fprintf ( stderr, "Couldn't open connection to dispay\n" );
		exit ( 1 );
	}

	Root = DefaultRootWindow ( dpy );
	DefaultGraphicsContext = DefaultGC ( dpy, DefaultScreen ( dpy ));

	/*
	 * Get four planes in the colourmap
	 */

	Cmap = DefaultColormap ( dpy, DefaultScreen ( dpy ));
	if ( XAllocColorCells ( dpy, Cmap, False, Masks, 4, &ColourBits, 1 ) == 0 )
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
		XStoreColor ( dpy, Cmap, &colour );
	}

	PlaneMask = ( Masks [ 0 ] | Masks [ 1 ] | Masks [ 2 ] | Masks [ 3 ] ) &
				~( Masks [ 0 ] & Masks [ 1 ] & Masks [ 2 ] & Masks [ 4 ] );

	/*
	 * FIX ME
	 *
	 * Should check for errors...
	 * Should allocate and set sizing hints here...
	 * How about window attributes, too ?
	 * Possibly don't need all these windows, either -- using Pixmaps
	 * directly.
	 */

	BeebScreen = XCreateSimpleWindow ( dpy, Root, 0, 0, 640, 526, 0,
															White, Black );
	TeletextScreen = XCreateSimpleWindow ( dpy, BeebScreen, TeletextWindowX,
								TeletextWindowY, 480, 475, 0, White, Black );
	BitmapScreen = XCreateSimpleWindow ( dpy, BeebScreen, BitmapWindowX,
								BitmapWindowY, 640, 512, 0, White, Black );

	/*
	 * The Pixmap for the bitmapped screen is actually bigger than the
	 * bitmapped screen -- this is because I handle the Pixmap as if
	 * it were the whole scan-line area, whereas the bitmapped screen
	 * is just the bit that can be seen on the display.
	 */

	BitmapPixmap = XCreatePixmap ( dpy, BitmapScreen, 640, 624,
								DefaultDepth ( dpy, DefaultScreen ( dpy )));
	
	/*
	 * FIX ME
	 *
	 * Do I need to do this ?
	 *
	XSetWindowColormap ( dpy, BeebScreen, Cmap );
	 */
	XSetWindowColormap ( dpy, TeletextScreen, Cmap );
	XSetWindowColormap ( dpy, BitmapScreen, Cmap );

	/*
	 * FIX ME
	 *
	 * Should I stop these events propagating to the root window ?
	 */

	XSelectInput ( dpy, BeebScreen, KeyPressMask | KeyReleaseMask | 
						ExposureMask | EnterWindowMask | LeaveWindowMask );
	XSelectInput ( dpy, TeletextScreen, ExposureMask );
	XSelectInput ( dpy, BitmapScreen, ExposureMask );

	/*
	 * FIX ME
	 *
	 * Need to do these for the Bitmap screen too ?
	 */

	CursorGC = XCreateGC ( dpy, BeebScreen, None, 0 );
	XCopyGC ( dpy, DefaultGraphicsContext, ~(( unsigned long ) 0), CursorGC );

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

	TtextTextGC = XCreateGC ( dpy, TeletextScreen, None, 0 );
	TtextMosaicGC = XCreateGC ( dpy, TeletextScreen, None, 0 );

	/*
	 * Set up the 17 bitmap GCs -- for each of the 16 different
	 * colours and the CopyAreaGC to prevent GraphicsExposure event
	 * checking when updating the BitmapScreen.
	 */

	for ( i = 0; i < 16; i++ )
	{
		BitmapGC [ i ]  = XCreateGC ( dpy, BitmapPixmap, None, 0 );
		XCopyGC ( dpy, DefaultGraphicsContext, 0xffff, BitmapGC [ i ] );
		XSetForeground ( dpy, BitmapGC [ i ], Cells [ i ] );
	}
	CopyAreaGC =  XCreateGC ( dpy, BitmapPixmap, None, 0 );
	XCopyGC ( dpy, DefaultGraphicsContext, 0xffff, CopyAreaGC );
	XSetGraphicsExposures ( dpy, CopyAreaGC, False );

	XCopyGC ( dpy, DefaultGraphicsContext, 0xffff, TtextTextGC );
	XCopyGC ( dpy, DefaultGraphicsContext, 0xffff, TtextMosaicGC );

	XSetFont ( dpy, TtextTextGC, TtextText );
	XSetFont ( dpy, TtextMosaicGC, TtextContiguousMosaic );

	/*
	 * Now clear the main Beeb window and the bitmapped screen to black.
	 */

	XFillRectangle ( dpy, BitmapPixmap, BitmapGC [ 0 ], 0, 0, 639, 623 );
	XClearWindow ( dpy, BeebScreen );
	XMapRaised ( dpy, BeebScreen );
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
				if ( xevent.xexpose.window == BeebScreen )
				{
					/*
					 * FIX ME
					 *
					 * Update the LED display etc.
					 *
					 */
					XFlush ( dpy );
				}
				else
				{
					/*
					 * Need to re-display the screen.
					 */

					ScreenChanged++;
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


static void
HandleKey ( XKeyEvent *key_event, signed char action )
{
	KeySym				sym;
	char				buf[3];

	XLookupString ( key_event, buf, 2, &sym, 0 );

	switch ( sym )
	{
		case 'A' : case 'a' :
			KeyboardMatrixUpdate ( KEY_A, action );
			break;

		case 'B' : case 'b' :
			KeyboardMatrixUpdate ( KEY_B, action );
			break;

		case 'C' : case 'c' :
			KeyboardMatrixUpdate ( KEY_C, action );
			break;

		case 'D' : case 'd' :
			KeyboardMatrixUpdate ( KEY_D, action );
			break;

		case 'E' : case 'e' :
			KeyboardMatrixUpdate ( KEY_E, action );
			break;

		case 'F' : case 'f' :
			KeyboardMatrixUpdate ( KEY_F, action );
			break;

		case 'G' : case 'g' :
			KeyboardMatrixUpdate ( KEY_G, action );
			break;

		case 'H' : case 'h' :
			KeyboardMatrixUpdate ( KEY_H, action );
			break;

		case 'I' : case 'i' :
			KeyboardMatrixUpdate ( KEY_I, action );
			break;

		case 'J' : case 'j' :
			KeyboardMatrixUpdate ( KEY_J, action );
			break;

		case 'K' : case 'k' :
			KeyboardMatrixUpdate ( KEY_K, action );
			break;

		case 'L' : case 'l' :
			KeyboardMatrixUpdate ( KEY_L, action );
			break;

		case 'M' : case 'm' :
			KeyboardMatrixUpdate ( KEY_M, action );
			break;

		case 'N' : case 'n' :
			KeyboardMatrixUpdate ( KEY_N, action );
			break;

		case 'O' : case 'o' :
			KeyboardMatrixUpdate ( KEY_O, action );
			break;

		case 'P' : case 'p' :
			KeyboardMatrixUpdate ( KEY_P, action );
			break;

		case 'Q' : case 'q' :
			KeyboardMatrixUpdate ( KEY_Q, action );
			break;

		case 'R' : case 'r' :
			KeyboardMatrixUpdate ( KEY_R, action );
			break;

		case 'S' : case 's' :
			KeyboardMatrixUpdate ( KEY_S, action );
			break;

		case 'T' : case 't' :
			KeyboardMatrixUpdate ( KEY_T, action );
			break;

		case 'U' : case 'u' :
			KeyboardMatrixUpdate ( KEY_U, action );
			break;

		case 'V' : case 'v' :
			KeyboardMatrixUpdate ( KEY_V, action );
			break;

		case 'W' : case 'w' :
			KeyboardMatrixUpdate ( KEY_W, action );
			break;

		case 'X' : case 'x' :
			KeyboardMatrixUpdate ( KEY_X, action );
			break;

		case 'Y' : case 'y' :
			KeyboardMatrixUpdate ( KEY_Y, action );
			break;

		case 'Z' : case 'z' :
			KeyboardMatrixUpdate ( KEY_Z, action );
			break;

		case '1' : case '!' :
			KeyboardMatrixUpdate ( KEY_1, action );
			break;

		case '2' : case '"' :
			KeyboardMatrixUpdate ( KEY_2, action );
			break;

		case '3' : case '#' :
			KeyboardMatrixUpdate ( KEY_3, action );
			if ( sym == '#' )
				KeyboardMatrixUpdate ( KEY_SHIFT, action );
			break;

		case '4' : case '$' :
			KeyboardMatrixUpdate ( KEY_4, action );
			break;

		case '5' : case '%' :
			KeyboardMatrixUpdate ( KEY_5, action );
			break;

		case '6' : case '&' :
			KeyboardMatrixUpdate ( KEY_6, action );
			break;

		case '7' : case XK_quoteright :
			KeyboardMatrixUpdate ( KEY_7, action );
			if ( sym == XK_quoteright )
				KeyboardMatrixUpdate ( KEY_SHIFT, action );
			break;

		case '8' : case '(' :
			KeyboardMatrixUpdate ( KEY_8, action );
			break;

		case '9' : case ')' :
			KeyboardMatrixUpdate ( KEY_9, action );
			break;

		case '0' :
			KeyboardMatrixUpdate ( KEY_0, action );
			break;

		case ' ' :
			KeyboardMatrixUpdate ( KEY_SPACE, action );
			break;

		case '.' : case '>' :
			KeyboardMatrixUpdate ( KEY_PERIOD, action );
			break;

		case ',' : case '<' :
			KeyboardMatrixUpdate ( KEY_COMMA, action );
			break;

		case '/' : case '?' :
			KeyboardMatrixUpdate ( KEY_SLASH, action );
			break;

		case ';' : case '+' :
			KeyboardMatrixUpdate ( KEY_SEMICOLON, action );
			break;

		case ':' : case '*' :
			KeyboardMatrixUpdate ( KEY_COLON, action );
			if ( sym == ':' )
				KeyboardMatrixUpdate ( KEY_SHIFT, -action );
			break;

		case '[' : case '{' :
			KeyboardMatrixUpdate ( KEY_LBRACKET, action );
			break;

		case ']' : case '}' :
			KeyboardMatrixUpdate ( KEY_RBRACKET, action );
			break;

		case '@' :
			KeyboardMatrixUpdate ( KEY_AT, action );
			break;

		case '_' : case XK_sterling :
			KeyboardMatrixUpdate ( KEY_POUND, action );
			break;

		case '-' :
			KeyboardMatrixUpdate ( KEY_MINUS, action );
			break;

		case '=' :
			KeyboardMatrixUpdate ( KEY_SHIFT, action );
			KeyboardMatrixUpdate ( KEY_MINUS, action );
			break;

		case '^' : case '~' :
			KeyboardMatrixUpdate ( KEY_HAT, action );
			if ( sym == '^' )
				KeyboardMatrixUpdate ( KEY_SHIFT, -action );
			break;

		case XK_F1 :
			KeyboardMatrixUpdate ( KEY_F0, action );
			break;

		case XK_F2 :
			KeyboardMatrixUpdate ( KEY_F1, action );
			break;

		case XK_F3 :
			KeyboardMatrixUpdate ( KEY_F2, action );
			break;

		case XK_F4 :
			KeyboardMatrixUpdate ( KEY_F3, action );
			break;

		case XK_F5 :
			KeyboardMatrixUpdate ( KEY_F4, action );
			break;

		case XK_F6 :
			KeyboardMatrixUpdate ( KEY_F5, action );
			break;

		case XK_F7 :
			KeyboardMatrixUpdate ( KEY_F6, action );
			break;

		case XK_F8 :
			KeyboardMatrixUpdate ( KEY_F7, action );
			break;

		case XK_F9 :
			KeyboardMatrixUpdate ( KEY_F8, action );
			break;

		case XK_F10 :
			KeyboardMatrixUpdate ( KEY_F9, action );
			break;

		case XK_Shift_L : case XK_Shift_R :
			KeyboardMatrixUpdate ( KEY_SHIFT, action );
			break;

		case XK_Control_L : case XK_Control_R :
			KeyboardMatrixUpdate ( KEY_CTRL, action );
			break;

		case XK_Tab :
			KeyboardMatrixUpdate ( KEY_TAB, action );
			break;

		case XK_BackSpace : case XK_Delete :
			KeyboardMatrixUpdate ( KEY_DELETE, action );
			break;

		case XK_Caps_Lock :
			KeyboardMatrixUpdate ( KEY_CAPSLOCK, action );
			break;

		case XK_Return :
			KeyboardMatrixUpdate ( KEY_RETURN, action );
			break;

		case XK_Up :
			KeyboardMatrixUpdate ( KEY_UP, action );
			break;

		case XK_Down :
			KeyboardMatrixUpdate ( KEY_DOWN, action );
			break;

		case XK_Left :
			KeyboardMatrixUpdate ( KEY_LEFT, action );
			break;

		case XK_Right :
			KeyboardMatrixUpdate ( KEY_RIGHT, action );
			break;

		case XK_Insert :
			KeyboardMatrixUpdate ( KEY_COPY, action );
			break;

		case XK_backslash : case '|' :
			KeyboardMatrixUpdate ( KEY_BACKSLASH, action );
			break;

		case XK_Escape :
			KeyboardMatrixUpdate ( KEY_ESCAPE, action );
			break;

		case XK_Home :
		case XK_KP_Home :
			if ( action == KEY_PRESSED )
				SnapshotRequested = 1;
			break;

		case XK_End :
		case XK_KP_End :
			if ( action == KEY_PRESSED )
				QuitEmulator = 1;
			break;

		case XK_KP_Enter :
			if ( action == KEY_PRESSED )
				ChangeDisk();
			break;

#ifdef	DISASS
		case XK_KP_Add :
			if ( action == KEY_PRESSED )
				DebugLevel |= DISASSEMBLE;
			break;

		case XK_KP_Subtract :
			if ( action == KEY_PRESSED )
				DebugLevel &= ~DISASSEMBLE;
			break;
#endif

		default :
			/*
			 * if it's not a key we recognise, just forget it...
			 */

			break;
	}
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
