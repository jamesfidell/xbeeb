/*
 *
 * $Id: Keyboard.h,v 1.11 2002/01/15 15:46:43 james Exp $
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
 * $Log: Keyboard.h,v $
 * Revision 1.11  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.10  2002/01/13 23:40:37  james
 * More keyboard-handling changes from Russell Marks
 *
 * Revision 1.9  2000/08/16 17:58:27  james
 * Update copyright message
 *
 * Revision 1.8  1996/12/01 21:26:25  james
 * Improved keyboard handling from DRS.
 *
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


#define	KEY_SHIFT		0x00
#define	KEY_CTRL		0x01
#define	SW2_7			0x02
#define	SW2_6			0x03
#define	SW2_5			0x04
#define	SW2_4			0x05
#define	SW2_3			0x06
#define	SW2_2			0x07
#define	SW2_1			0x08
#define	SW2_0			0x09

#define	KEY_Q			0x10
#define	KEY_3			0x11
#define	KEY_4			0x12
#define	KEY_5			0x13
#define	KEY_F4			0x14
#define	KEY_8			0x15
#define	KEY_F7			0x16
#define	KEY_MINUS		0x17
#define	KEY_HAT			0x18
#define	KEY_LEFT		0x19

#define	KEY_F0			0x20
#define	KEY_W			0x21
#define	KEY_E			0x22
#define	KEY_T			0x23
#define	KEY_7			0x24
#define	KEY_I			0x25
#define	KEY_9			0x26
#define	KEY_0			0x27
#define	KEY_USCORE		0x28
#define	KEY_DOWN		0x29

#define	KEY_1			0x30
#define	KEY_2			0x31
#define	KEY_D			0x32
#define	KEY_R			0x33
#define	KEY_6			0x34
#define	KEY_U			0x35
#define	KEY_O			0x36
#define	KEY_P			0x37
#define	KEY_LBRACKET	0x38
#define	KEY_UP			0x39

#define	KEY_CAPSLOCK	0x40
#define	KEY_A			0x41
#define	KEY_X			0x42
#define	KEY_F			0x43
#define	KEY_Y			0x44
#define	KEY_J			0x45
#define	KEY_K			0x46
#define	KEY_AT			0x47
#define	KEY_COLON		0x48
#define	KEY_RETURN		0x49

#define	KEY_SHIFTLOCK	0x50
#define	KEY_S			0x51
#define	KEY_C			0x52
#define	KEY_G			0x53
#define	KEY_H			0x54
#define	KEY_N			0x55
#define	KEY_L			0x56
#define	KEY_SEMICOLON	0x57
#define	KEY_RBRACKET	0x58
#define	KEY_DELETE		0x59

#define	KEY_TAB			0x60
#define	KEY_Z			0x61
#define	KEY_SPACE		0x62
#define	KEY_V			0x63
#define	KEY_B			0x64
#define	KEY_M			0x65
#define	KEY_COMMA		0x66
#define	KEY_PERIOD		0x67
#define	KEY_SLASH		0x68
#define	KEY_COPY		0x69

#define	KEY_ESCAPE		0x70
#define	KEY_F1			0x71
#define	KEY_F2			0x72
#define	KEY_F3			0x73
#define	KEY_F5			0x74
#define	KEY_F6			0x75
#define	KEY_F8			0x76
#define	KEY_F9			0x77
#define	KEY_BACKSLASH	0x78
#define	KEY_RIGHT		0x79


#define	KEY_PRESSED		1
#define	KEY_RELEASED	0

#endif	/* KEYBOARD_H */
