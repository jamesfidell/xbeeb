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


#ifndef	KEYBOARD_H
#define	KEYBOARD_H

extern void		KeyboardWrite ( unsigned char, unsigned char* );
extern void		LedSetCapsLock ( unsigned char );
extern void		LedSetShiftLock ( unsigned char );
extern void		InitialiseKeyboard ( void );
extern void		KeyboardMatrixUpdate ( unsigned char, signed char );

extern int		SaveKeyboard ( int );
extern int		RestoreKeyboard ( int, unsigned int );


#define	KEY_SHIFT		 0
#define	KEY_CTRL		 1
#define	SW2_7			 2
#define	SW2_6			 3
#define	SW2_5			 4
#define	SW2_4			 5
#define	SW2_3			 6
#define	SW2_2			 7
#define	SW2_1			 8
#define	SW2_0			 9

#define	KEY_Q			10
#define	KEY_3			11
#define	KEY_4			12
#define	KEY_5			13
#define	KEY_F4			14
#define	KEY_8			15
#define	KEY_F7			16
#define	KEY_MINUS		17
#define	KEY_HAT			18
#define	KEY_LEFT		19

#define	KEY_F0			20
#define	KEY_W			21
#define	KEY_E			22
#define	KEY_T			23
#define	KEY_7			24
#define	KEY_I			25
#define	KEY_9			26
#define	KEY_0			27
#define	KEY_POUND		28
#define	KEY_DOWN		29

#define	KEY_1			30
#define	KEY_2			31
#define	KEY_D			32
#define	KEY_R			33
#define	KEY_6			34
#define	KEY_U			35
#define	KEY_O			36
#define	KEY_P			37
#define	KEY_LBRACKET	38
#define	KEY_UP			39

#define	KEY_CAPSLOCK	40
#define	KEY_A			41
#define	KEY_X			42
#define	KEY_F			43
#define	KEY_Y			44
#define	KEY_J			45
#define	KEY_K			46
#define	KEY_AT			47
#define	KEY_COLON		48
#define	KEY_RETURN		49

#define	KEY_SHIFTLOCK	50
#define	KEY_S			51
#define	KEY_C			52
#define	KEY_G			53
#define	KEY_H			54
#define	KEY_N			55
#define	KEY_L			56
#define	KEY_SEMICOLON	57
#define	KEY_RBRACKET	58
#define	KEY_DELETE		59

#define	KEY_TAB			60
#define	KEY_Z			61
#define	KEY_SPACE		62
#define	KEY_V			63
#define	KEY_B			64
#define	KEY_M			65
#define	KEY_COMMA		66
#define	KEY_PERIOD		67
#define	KEY_SLASH		68
#define	KEY_COPY		69

#define	KEY_ESCAPE		70
#define	KEY_F1			71
#define	KEY_F2			72
#define	KEY_F3			73
#define	KEY_F5			74
#define	KEY_F6			75
#define	KEY_F8			76
#define	KEY_F9			77
#define	KEY_BACKSLASH	78
#define	KEY_RIGHT		79


#define	KEY_PRESSED		1
#define	KEY_RELEASED	-1

#endif	/* KEYBOARD_H */
