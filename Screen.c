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
#include <X11/keysym.h>

#include "Config.h"
#include "Beeb.h"
#include "Screen.h"
#include "Modes.h"
#include "Teletext.h"
#include "Keyboard.h"

static void			HandleKey ( XKeyEvent*, signed char );


Display				*dpy;
Window				BeebScreen;
GC					DefaultGraphicsContext;
GC					CursorGC;
Colormap			Cmap;

/*
 * Bits for the teletext MODE
 */

GC					TtextTextGC;
GC					TtextMosaicGC;

Font				TtextText;
Font				TtextTextDblU;
Font				TtextTextDblL;
Font				TtextContiguousMosaic;
Font				TtextSeparateMosaic;


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

void
InitialiseScreen()
{
	int					i;
	Window				Root;
	XGCValues			CursorGCValues;
	XColor				colour;

	if (( dpy = XOpenDisplay ( 0 )) == 0 )
	{
		fprintf ( stderr, "Couldn't open connection to dispay\n" );
		exit ( 1 );
	}

	Root = DefaultRootWindow ( dpy );
	DefaultGraphicsContext = DefaultGC ( dpy, DefaultScreen ( dpy ));

	/*
	 * Allocate all of the colourmap colours
	 */

	Cmap = XCreateColormap( dpy, Root, DefaultVisual ( dpy,
										DefaultScreen ( dpy )), AllocAll );
	for ( i = 0; i < 256; i++ )
	{
		colour.red = RgbValues [ i % 8 ][ 0 ];
		colour.green = RgbValues [ i % 8 ][ 1 ];
		colour.blue = RgbValues [ i % 8 ][ 2 ];
		colour.flags = DoRed | DoGreen | DoBlue;
		colour.pixel = i;

		XStoreColor ( dpy, Cmap, &colour );
	}

	BeebScreen = XCreateSimpleWindow ( dpy, Root, 0, 0, 480, 475, 0, White,
																	Black );
	XSetWindowColormap ( dpy, BeebScreen, Cmap );
	XSelectInput ( dpy, BeebScreen, KeyPressMask | KeyReleaseMask | 
						ExposureMask | EnterWindowMask | LeaveWindowMask );
	XMapRaised ( dpy, BeebScreen );

	CursorGC = XCreateGC ( dpy, BeebScreen, None, 0 );
	XCopyGC ( dpy, DefaultGraphicsContext, ~0L, CursorGC );

	CursorGCValues.function = GXxor;
	CursorGCValues.foreground = White;
	CursorGCValues.background = Black;
	CursorGCValues.fill_style = FillSolid;
	XChangeGC ( dpy, CursorGC, GCFunction | GCForeground | GCBackground |
		GCFillStyle, &CursorGCValues );

	/*
	 * FIX ME
	 *
	 * should check for errors when loading fonts.
	 */

	TtextText = XLoadFont ( dpy, "ttext-std" );
	TtextTextDblU = XLoadFont ( dpy, "ttext-std-udh" );
	TtextTextDblL = XLoadFont ( dpy, "ttext-std-ldh" );
	TtextContiguousMosaic = XLoadFont ( dpy, "ttext-grc" );
	TtextSeparateMosaic = XLoadFont ( dpy, "ttext-grs" );

	TtextTextGC = XCreateGC ( dpy, BeebScreen, None, 0 );
	TtextMosaicGC = XCreateGC ( dpy, BeebScreen, None, 0 );

	XCopyGC ( dpy, DefaultGraphicsContext, 0xffff, TtextTextGC );
	XCopyGC ( dpy, DefaultGraphicsContext, 0xffff, TtextMosaicGC );

	XSetFont ( dpy, TtextTextGC, TtextText );
	XSetFont ( dpy, TtextMosaicGC, TtextContiguousMosaic );

	XFlush ( dpy );
	return;
}


void
ShutdownScreen()
{
	/*
	 * FIX ME
	 *
	 * Should unload the fonts, really.
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
				/*
				 * Need to re-display the screen.
				 *
				 * Just set the flag so it will get done at the next
				 * vertical sync. interrupt.
				 */

				ScreenChanged++;
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
				 * something should be done here.
				 *
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

		case XK_F1 :
			KeyboardMatrixUpdate ( KEY_F1, action );
			break;

		case XK_F2 :
			KeyboardMatrixUpdate ( KEY_F2, action );
			break;

		case XK_F3 :
			KeyboardMatrixUpdate ( KEY_F3, action );
			break;

		case XK_F4 :
			KeyboardMatrixUpdate ( KEY_F4, action );
			break;

		case XK_F5 :
			KeyboardMatrixUpdate ( KEY_F5, action );
			break;

		case XK_F6 :
			KeyboardMatrixUpdate ( KEY_F6, action );
			break;

		case XK_F7 :
			KeyboardMatrixUpdate ( KEY_F7, action );
			break;

		case XK_F8 :
			KeyboardMatrixUpdate ( KEY_F8, action );
			break;

		case XK_F9 :
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
			if ( action == KEY_PRESSED )
				SnapshotRequested = 1;
			break;

		case XK_End :
			if ( action == KEY_PRESSED )
				QuitEmulator = 1;
			break;

		default :
			/*
			 * FIX ME
			 *
			 * something should be done here.
			 *
			 */

			break;
	}
}


void
UpdateScreenImage()
{
	/*
	 * FIX ME
	 *
	 * This should be done with function pointers.
	 */

	switch ( CurrentMode )
	{
		case 0 :
		case 1 :
		case 2 :
		case 3 :
		case 4 :
		case 5 :
		case 6 :
			break;
		case 7 :
			TeletextScreenUpdate();
			break;
		default :
			break;
	}
	ScreenChanged = 0;

	return;
}


void
ScreenAddressStartHi ( unsigned char bit )
{
	/*
	 * FIX ME
	 */

#ifdef	TODO
	fprintf ( stderr, "screen start address high = %d\n", bit );
	fprintf ( stderr, "not yet implemented\n" );
#endif
	return;
}


void
ScreenAddressStartLo ( unsigned char bit )
{
	/*
	 * FIX ME
	 */

#ifdef	TODO
	fprintf ( stderr, "screen start address low = %d\n", bit );
	fprintf ( stderr, "not yet implemented\n" );
#endif
	return;
}
