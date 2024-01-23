/*
 *
 * $Id: KeymapLegend.c,v 1.2 1996/10/10 21:55:44 james Exp $
 *
 * Copyright (c) James Fidell 1996.
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
 * $Log: KeymapLegend.c,v $
 * Revision 1.2  1996/10/10 21:55:44  james
 * Correctly place #ifdef's for KEYMAP_LEGEND & KEYMAP_STRICT.
 *
 * Revision 1.1  1996/10/01 22:10:00  james
 * Split keyboard handling into kEYMAP_STRICT and KEYMAP_LEGEND models.
 *
 *
 */


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "Config.h"
#include "Beeb.h"
#include "Keyboard.h"
#include "Keymap.h"
#include "EFS.h"


#ifdef	KEYMAP_LEGEND

void
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
			KeyboardMatrixUpdate ( KEY_USCORE, action );
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
			/*
			 * FIX ME
			 *
			 * The CAPS LOCK key is a sod because the keyboard automatically
			 * releases the key as soon as it is pressed
			 */

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

		case XK_Alt_L :
			KeyboardMatrixUpdate ( KEY_SHIFTLOCK, action );
			break;

#ifdef	XK_KP_Home
		case XK_KP_Home :
#endif
		case XK_Home :
			if ( action == KEY_PRESSED )
				SnapshotRequested = 1;
			break;

#ifdef	XK_KP_End
		case XK_KP_End :
#endif
		case XK_End :
			if ( action == KEY_PRESSED )
				QuitEmulator = 1;
			break;

#ifdef	XK_Pause
		case XK_Pause :
#else
		case XK_F12 :
#endif
			BreakKeypress  = 1 - BreakKeypress;
			break;

#ifdef	EMUL_FS
		case XK_KP_Enter :
			if ( action == KEY_PRESSED )
				ChangeDisk();
			break;
#endif

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

#endif	/* KEYMAP_LEGEND */
