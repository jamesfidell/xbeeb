/*
 *
 * $Id: Keyboard.h,v 1.7 1996/10/08 00:04:31 james Exp $
 *
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

/*
 * Modification History
 *
 * $Log: Keyboard.h,v $
 * Revision 1.7  1996/10/08 00:04:31  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.6  1996/10/01 22:09:59  james
 * Split keyboard handling into kEYMAP_STRICT and KEYMAP_LEGEND models.
 *
 * Revision 1.5  1996/09/30 23:39:33  james
 * Split out option processing into Options.[ch].  Updated the help message,
 * added support for the Model A using the -a switch (and added the
 * MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
 * initial screen mode and keyboard DIP switches.
 *
 * Revision 1.4  1996/09/25 19:19:57  james
 * Major overhaul of VIA emulation code :
 *
 *   Enabled toggling of PB7 in system VIA depending on ACR bit 6 and the
 *   one-shot/free-run mode of T1
 *
 *   Implemented User VIA T1 free-running mode.  Set the initial value of
 *   the User VIA ORA to 0x80.  Planetoid/Defender now works for the first
 *   time!
 *
 *   Corrected value returned by read from VIA T2CL and T2CH.  Frak! now
 *   works.
 *
 *   Set up dummy return for reads from the system VIA IRA and SR.
 *
 *   Implemented address wrap-around for memory-mapped registers in the VIA.
 *
 *   Set up dummy return for reads from the user VIA SR.
 *
 *   Implemented address wrap-around for memory-mapped registers in the VIA.
 *
 *   Updated 6522 VIA emulation to have correct initial values for VIA
 *   registers wherever possible.
 *
 *   Heavily modified 6522 VIA code to separate out the input/output
 *   registers and what is actually on the data pins.  This has the benefits
 *   of tidying up the whole VIA i/o emulation and not requiring any nasty
 *   configuration hacks to get software to work (apart from those that exist
 *   because of uncompleted emulation).
 *
 *   Tidied up 6522Via interrupt handling code.
 *
 * Revision 1.3  1996/09/24 23:05:39  james
 * Update copyright dates.
 *
 * Revision 1.2  1996/09/21 22:13:49  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:38  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	KEYBOARD_H
#define	KEYBOARD_H

extern byteval			KeyboardWrite ( byteval );
extern void				LedSetCapsLock ( byteval );
extern void				LedSetShiftLock ( byteval );
extern void				InitialiseKeyboard ( void );
extern void				KeyboardMatrixUpdate ( unsigned char, signed char );

extern int				SaveKeyboard ( int );
extern int				RestoreKeyboard ( int, unsigned int );

extern unsigned long	DIPSwitches;
extern unsigned char	CapsLockLED;
extern unsigned char	ShiftLockLED;
#ifdef	SHIFTLOCK_SOUND_HACK
extern unsigned char	LockKeysChanged;
#endif


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
#define	KEY_USCORE		28
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
