/*
 *
 * $Id: KeymapStrict.c,v 1.2 1996/10/10 21:55:45 james Exp $
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
 * $Log: KeymapStrict.c,v $
 * Revision 1.2  1996/10/10 21:55:45  james
 * Correctly place #ifdef's for KEYMAP_LEGEND & KEYMAP_STRICT.
 *
 * Revision 1.1  1996/10/01 22:10:01  james
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


#ifdef	KEYMAP_STRICT

void
HandleKey ( XKeyEvent *key_event, signed char action )
{
	switch ( XLookupKeysym ( key_event, ShiftMapIndex ))
	{
		case XK_Escape :	/* Esc */
			KeyboardMatrixUpdate ( KEY_ESCAPE, action );
			break;

		case XK_F1 :	/* F1 */
			KeyboardMatrixUpdate ( KEY_F1, action );
			break;

		case XK_F2 :	/* F2 */
			KeyboardMatrixUpdate ( KEY_F2, action );
			break;

		case XK_F3 :	/* F3 */
			KeyboardMatrixUpdate ( KEY_F3, action );
			break;

		case XK_F4 :	/* F4 */
			KeyboardMatrixUpdate ( KEY_F4, action );
			break;

		case XK_F5 :	/* F5 */
			KeyboardMatrixUpdate ( KEY_F5, action );
			break;

		case XK_F6 :	/* F6 */
			KeyboardMatrixUpdate ( KEY_F6, action );
			break;

		case XK_F7 :	/* F7 */
			KeyboardMatrixUpdate ( KEY_F7, action );
			break;

		case XK_F8 :	/* F8 */
			KeyboardMatrixUpdate ( KEY_F8, action );
			break;

		case XK_F9 :	/* F9 */
			KeyboardMatrixUpdate ( KEY_F9, action );
			break;

		case XK_F10 :	/* F10 */
			KeyboardMatrixUpdate ( KEY_F0, action );
			break;

		case XK_1 :
			KeyboardMatrixUpdate ( KEY_1, action );
			break;

		case XK_2 :
			KeyboardMatrixUpdate ( KEY_2, action );
			break;

		case XK_3 :
			KeyboardMatrixUpdate ( KEY_3, action );
			break;

		case XK_4 :
			KeyboardMatrixUpdate ( KEY_4, action );
			break;

		case XK_5 :
			KeyboardMatrixUpdate ( KEY_5, action );
			break;

		case XK_6 :
			KeyboardMatrixUpdate ( KEY_6, action );
			break;

		case XK_7 :
			KeyboardMatrixUpdate ( KEY_7, action );
			break;

		case XK_8 :
			KeyboardMatrixUpdate ( KEY_8, action );
			break;

		case XK_9 :
			KeyboardMatrixUpdate ( KEY_9, action );
			break;

		case XK_0 :
			KeyboardMatrixUpdate ( KEY_0, action );
			break;

		case XK_minus :
			KeyboardMatrixUpdate ( KEY_MINUS, action );
			break;

		case XK_equal :
			KeyboardMatrixUpdate ( KEY_HAT, action );
			break;

		case XK_Tab :
			KeyboardMatrixUpdate ( KEY_TAB, action );
			break;

		case XK_q :
			KeyboardMatrixUpdate ( KEY_Q, action );
			break;

		case XK_w :
			KeyboardMatrixUpdate ( KEY_W, action );
			break;

		case XK_e :
			KeyboardMatrixUpdate ( KEY_E, action );
			break;

		case XK_r :
			KeyboardMatrixUpdate ( KEY_R, action );
			break;

		case XK_t :
			KeyboardMatrixUpdate ( KEY_T, action );
			break;

		case XK_y :
			KeyboardMatrixUpdate ( KEY_Y, action );
			break;

		case XK_u :
			KeyboardMatrixUpdate ( KEY_U, action );
			break;

		case XK_i :
			KeyboardMatrixUpdate ( KEY_I, action );
			break;

		case XK_o :
			KeyboardMatrixUpdate ( KEY_O, action );
			break;

		case XK_p :
			KeyboardMatrixUpdate ( KEY_P, action );
			break;

		case XK_bracketleft :
			KeyboardMatrixUpdate ( KEY_LBRACKET, action );
			break;

		case XK_bracketright :
			KeyboardMatrixUpdate ( KEY_USCORE, action );
			break;

		case XK_a :
			KeyboardMatrixUpdate ( KEY_A, action );
			break;

		case XK_s :
			KeyboardMatrixUpdate ( KEY_S, action );
			break;

		case XK_d :
			KeyboardMatrixUpdate ( KEY_D, action );
			break;

		case XK_f :
			KeyboardMatrixUpdate ( KEY_F, action );
			break;

		case XK_g :
			KeyboardMatrixUpdate ( KEY_G, action );
			break;

		case XK_h :
			KeyboardMatrixUpdate ( KEY_H, action );
			break;

		case XK_j :
			KeyboardMatrixUpdate ( KEY_J, action );
			break;

		case XK_k :
			KeyboardMatrixUpdate ( KEY_K, action );
			break;

		case XK_l :
			KeyboardMatrixUpdate ( KEY_L, action );
			break;

		case XK_semicolon :
			KeyboardMatrixUpdate ( KEY_SEMICOLON, action );
			break;

		case XK_quoteright :
			KeyboardMatrixUpdate ( KEY_COLON, action );
			break;

		case XK_numbersign :
			KeyboardMatrixUpdate ( KEY_RBRACKET, action );
			break;

		case XK_Return :
			KeyboardMatrixUpdate ( KEY_RETURN, action );
			break;

		case XK_Shift_L : case XK_Shift_R :
			KeyboardMatrixUpdate ( KEY_SHIFT, action );
			break;

		case XK_z :
			KeyboardMatrixUpdate ( KEY_Z, action );
			break;

		case XK_x :
			KeyboardMatrixUpdate ( KEY_X, action );
			break;

		case XK_c :
			KeyboardMatrixUpdate ( KEY_C, action );
			break;

		case XK_v :
			KeyboardMatrixUpdate ( KEY_V, action );
			break;

		case XK_b :
			KeyboardMatrixUpdate ( KEY_B, action );
			break;

		case XK_n :
			KeyboardMatrixUpdate ( KEY_N, action );
			break;

		case XK_m :
			KeyboardMatrixUpdate ( KEY_M, action );
			break;

		case XK_comma :
			KeyboardMatrixUpdate ( KEY_COMMA, action );
			break;

		case XK_period :
			KeyboardMatrixUpdate ( KEY_PERIOD, action );
			break;

		case XK_slash :
			KeyboardMatrixUpdate ( KEY_SLASH, action );
			break;

		case XK_Control_L :
			KeyboardMatrixUpdate ( KEY_CAPSLOCK, action );
			break;

		case XK_Alt_L :
			KeyboardMatrixUpdate ( KEY_CTRL, action );
			break;

		case XK_space :
			KeyboardMatrixUpdate ( KEY_SPACE, action );
			break;

		case XK_Up :
			KeyboardMatrixUpdate ( KEY_UP, action );
			break;

		case XK_Left :
			KeyboardMatrixUpdate ( KEY_LEFT, action );
			break;

		case XK_Right :
			KeyboardMatrixUpdate ( KEY_RIGHT, action );
			break;

		case XK_Down :
			KeyboardMatrixUpdate ( KEY_DOWN, action );
			break;

		case XK_backslash :
			KeyboardMatrixUpdate ( KEY_BACKSLASH, action );
			break;

		case XK_Delete : case XK_BackSpace :
			KeyboardMatrixUpdate ( KEY_DELETE, action );
			break;

		case XK_Insert :
			KeyboardMatrixUpdate ( KEY_COPY, action );
			break;

		case XK_quoteleft :
			KeyboardMatrixUpdate ( KEY_AT, action );
			break;

		case XK_Alt_R :
			KeyboardMatrixUpdate ( KEY_SHIFTLOCK, action );
			break;

		/*
		 * Special keys
		 */

		case XK_Home :
#ifdef	XK_KP_Home
		case XK_KP_Home :
#endif
			if ( action == KEY_PRESSED )
				SnapshotRequested = 1;
			break;

		case XK_Pause :	/* Pause */
			BreakKeypress  = 1 - BreakKeypress;
			break;

		case XK_End :
#ifdef	XK_KP_End
		case XK_KP_End :
#endif
			if ( action == KEY_PRESSED )
				QuitEmulator = 1;
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

#endif	/* STRICT_KEYMAP */
