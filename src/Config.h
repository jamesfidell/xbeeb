/*
 *
 * $Id: Config.h,v 1.34 2002/01/15 15:46:43 james Exp $
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
 * $Log: Config.h,v $
 * Revision 1.34  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.33  2002/01/14 23:20:44  james
 * Prevent rolling over 31 disk catalog entries
 *
 * Revision 1.32  2002/01/14 22:18:51  james
 * Added support for .inf files
 *
 * Revision 1.31  2002/01/13 22:27:19  james
 * Fix compile-time warnings
 *
 * Revision 1.30  2000/09/07 21:59:03  james
 * Add FASTHOST configurable.  Make bitmap displays read data properly rather
 * than direct from the memory array when this is enabled.
 *
 * Revision 1.29  2000/09/07 21:30:39  james
 * Fix coredump
 *
 * Revision 1.28  2000/09/06 11:41:13  james
 * First cut at "proper" sound code
 *
 * Revision 1.27  2000/09/02 18:48:25  james
 * Changed all VoxWare references to OSS
 *
 * Revision 1.26  2000/08/16 17:58:27  james
 * Update copyright message
 *
 * Revision 1.25  2000/08/16 17:41:45  james
 * Changes to work on TrueColor displays
 *
 * Revision 1.24  1996/12/04 23:39:58  james
 * Remove the whole VSYNC_TIME thing.
 *
 * Revision 1.23  1996/11/24 22:21:56  james
 * Don't use the FASTCLOCK mechanism by default.
 *
 * Revision 1.22  1996/11/18 00:53:55  james
 * Add new XDFS code and ROM (v0.90) from David Ralph Stacey.
 *
 * Revision 1.21  1996/10/13 17:12:48  james
 * Add local version of "strtoul" for those systems that don't have it.
 *
 * Revision 1.20  1996/10/09 23:19:08  james
 * Added support for using the MIT X11 Shared Memory Extensions.
 *
 * Revision 1.19  1996/10/08 00:04:30  james
 * Added InfoWindow to show LED status.  Also required addition of the
 * SHIFTLOCK_SOUND_HACK to prevent the Shift Lock LED being light up
 * whenever the sound buffer is full, which means that far too much
 * time can be spent re-drawing the LED.
 *
 * Revision 1.18  1996/10/07 22:59:46  james
 * Modified FASTCLOCK implementation to allow configuration of the number
 * of instructions between interrupts.
 *
 * Revision 1.17  1996/10/07 22:06:32  james
 * Added XDFS ROM & support code from David Ralph Stacey.
 *
 * Revision 1.16  1996/10/01 22:30:32  james
 * Added VoxWare sound code from James Murray <jsm@jsm-net.demon.co.uk>.
 *
 * Revision 1.15  1996/10/01 22:09:58  james
 * Split keyboard handling into kEYMAP_STRICT and KEYMAP_LEGEND models.
 *
 * Revision 1.14  1996/09/30 23:39:32  james
 * Split out option processing into Options.[ch].  Updated the help message,
 * added support for the Model A using the -a switch (and added the
 * MODEL_B_ONLY #define in Config.h, added the -m and -s switches to set the
 * initial screen mode and keyboard DIP switches.
 *
 * Revision 1.13  1996/09/30 22:32:11  james
 * Allowed external definition of XBEEBROOT.
 *
 * Revision 1.12  1996/09/24 23:05:35  james
 * Update copyright dates.
 *
 * Revision 1.11  1996/09/24 22:40:16  james
 * Massive overhaul of instruction decoding code.  Includes :
 *
 *   Correct implementation of (indirect),Y instructions when overflow occurs,
 *   allowing the removal of the RANGE_CHECK directive for those instructions.
 *
 *   Correct handling of address wrap-around for all zp,X and zp,Y
 *   instructions.  This removes the need for the RANGE_CHECK define.  Removed
 *   that, too.
 *
 *   Updated all disassembly instructions to give the full number of hex
 *   digits when displaying their parameters.
 *
 *   Split opcodes.h to give NMOS 6502 opcodes in 6502ops.h and EFS opcodes
 *   in EFSops.h
 *
 *   Add all NMOS 6502 HALT opcodes.
 *
 *   Add all NMOS 6502 NOP opcodes.
 *
 *   Coded for the undocumented NMOS 6502 NOP operations so that
 *   they load a value from memory according to their addressing mode (but
 *   neither store it anywhere nor set any SR flags).
 *
 *   Coded all other undocumented NMOS 6502 operations.
 *
 *   Changed the EFS dummy opcodes because of a clash with the undocumented
 *   NMOS 6502 DCP instructions.  The new trap values are now codes that would
 *   normally halt the CPU.
 *
 *   Added all the R65C02 opcodes.
 *
 *   Added all the R65C12 opcodes.
 *
 *   Correctly coded (zp,X) addressing mode where zp+X(+1) overlaps the
 *   page boundary.
 *
 *   Added #defined values for the number of cycles taken by each instruction
 *   in 6502.h
 *
 *   Added #defines for the original 6502 and Rockwell 65C02 and 65C12.
 *
 * Revision 1.10  1996/09/24 22:12:09  james
 * Prevented multiple inclusion of Config.h
 *
 * Revision 1.9  1996/09/24 18:22:33  james
 * Change LITTLE_ENDIAN #define to ENDIAN_6502 because of a clash with
 * header files on some systems.
 *
 * Revision 1.8  1996/09/23 16:21:16  james
 * Improvements to snapshot code.
 *
 * Revision 1.7  1996/09/22 21:14:07  james
 * Added NEED_STRCASECMP #define for EMUL_FS code.
 *
 * Revision 1.6  1996/09/22 19:23:20  james
 * Add the emulated filing system code.
 *
 * Revision 1.5  1996/09/21 22:48:21  james
 * Add instruction counting code.
 *
 * Revision 1.4  1996/09/21 22:39:52  james
 * Improved handling of instruction disassembly.
 *
 * Revision 1.3  1996/09/21 22:13:47  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.2  1996/09/21 18:16:22  james
 * Added definitions of PATH_MAX and NAME_MAX for those systems that don't have
 * them.  This needs to be properly resolved some time.
 *
 * Revision 1.1  1996/09/21 17:20:36  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	CONFIG_H
#define	CONFIG_H

/*
 * Potential performance gains
 *
 * LO_PAGE			assumes that writes to pages 0 and 1 can be made directly
 *					without checking that any side-effects might occur (such as
 *					the screen being modified.
 *
 * FASTCLOCK		causes the VIA timers etc. to be updated every FASTCLOCK
 * 					cycles rather than every instruction.
 *
 * ENDIAN_6502		Causes writing words to the memory array to be done in one
 *					operation as an unsigned 16-bit value rather than as two
 *					8-bit values.  Only possible because the 6502 is little-
 *					endian.
 *
 * MITSHM			Use MIT X11 Shared Memory Extensions if they are
 *					supported by the server.
 *
 * also:
 *
 * FASTHOST			define this if your system can easily run the emulator
 *					in excess of "real time" speed.
 *
 */

#undef		LO_PAGE
#undef		FASTCLOCK	/* 100 */
#undef		ENDIAN_6502
#define		MITSHM
#undef		FASTHOST
#define		USE_NANOSLEEP

/*
 * Misc. config. stuff
 *
 * NO_FRED_JIM		This is defined because xbeeb has no support for the FRED
 *					and JIM memory-mapped IO areas yet.  It makes the emulator
 *					faster because there's less checking to do an memory
 *					read/write.
 *
 * LIMIT			Define to be the number of instructions executed before
 *					xbeeb exits.  I just use it for timing purposes.
 *
 * DISASS			Compile in code for disassembling all instructions the
 *                  CPU executes.  Very useful for debugging.
 *
 * INFO				Gives all sorts of information about what's happening
 *					with the hardware emulation.
 *
 * NEED_STRCASECMP	If your system doesn't have the "strcasecmp" function.
 *
 * NEED_STRTOUL		If your system doesn't have the "strtoul" function.
 *
 * EFS				Enable original Xbeeb emulated FS code
 * INF_FS			Enable FS emulation using .inf files
 * EMUL_FS			Enables the emulated file-system code
 *
 * CATALOG_SIZE		The number of entries allowed in the emulated disk catalog
 *
 * COUNT_INSTRS		Count how many times each op-code was executed
 *
 * M6502			CPU is the original NMOS 6502
 * R65C02			CPU is the Rockwell CMOS 65C02
 * R65C12			CPU is the Rockwell CMOS 65C12
 *
 * MODEL_B_ONLY		Only compile in support for the model B machine.
 *
 * DIP_SWITCHES		The default setting for the keyboard DIP switches
 *
 * XBEEBROOT		The default root location of all of the disk images etc.
 *
 * SOUND_OSS		Include OSS sound support.
 *
 * SHIFTLOCK_SOUND_HACK
 *					Disable Shift Lock indicating that the sound buffer is
 *					full
 *
 * KEYMAP_STRICT	Use the keymap based on key positions on the keyboard.
 * KEYMAP_LEGEND	Use the keymap based on key legends.
 *
 */

#define		NO_FRED_JIM
#undef		LIMIT			/* 5000000 */
#define		DISASS			0
#undef		INFO
#undef		NEED_STRCASECMP

#undef		EFS
#define		INF_FS
#if defined(EFS) || defined(INF_FS)
#define		EMUL_FS
#define		XDFS
#endif

#undef		COUNT_INSTRS

/*
 * Processor type
 */

#define		M6502
#undef		R65C02
#undef		R65C12

/*
 * Processor speed (in MHz)
 */

#define		CPU_SPEED	2

/*
 * Model A/B support
 */

#define		MODEL_B_ONLY


/*
 * default keyboard DIP switch setting
 */

#define		DIP_SWITCHES	0x0

/*
 * Default location for all of xbeeb's files
 */

#ifndef	XBEEBROOT
#define		XBEEBROOT		""
#endif
#define		XBEEBROMS		XBEEBROOT"roms/"
#define		XBEEBSNAPS		XBEEBROOT"snaps/"
#define		XBEEBDISKS		XBEEBROOT"disks/"
#define		XBEEBTAPES		XBEEBROOT"tapes/"

/*
 * Sound emulation #defines
 */

#define		SOUND_OSS
#undef		SHIFTLOCK_SOUND_HACK


/*
 * Keyboard tpe.  Define one of these
 */

#define		KEYMAP_STRICT
#undef		KEYMAP_LEGEND

/*
 * Names of ROMs to load by default
 */
  
#ifdef	EMUL_FS
#define		OS_ROM			"OS1.2p1.rom"
#else
#define		OS_ROM			"OS1.2.rom"
#endif

#ifdef	M6502
#define		LANG_ROM		"BASIC2.rom"
#else
#define		LANG_ROM		"BASIC4.rom"
#endif

#ifdef	XDFS
#define		XDFS_ROM		"xdfs0.90.rom"
#endif
  
/*
 * Default snapshot name
 */

#define		SNAPSHOT_EXT	".sst"
#define		DEF_SNAPSHOT	"xbeeb"

/*
 * FIX ME
 *
 * A portable way of fixing these values is required.  For now, they
 * are just set to the fixed values so that the code will compile.
 */

#ifndef	PATH_MAX
#define	PATH_MAX			1024
#endif

#ifndef	NAME_MAX
#define	NAME_MAX			14
#endif

/********************************************************************
         YOU SHOULDN'T NEED TO CHANGE ANYTHING FROM HERE DOWN
 ********************************************************************/

#ifdef	EMUL_FS

#define	CATALOG_SIZE	31

#ifdef	EFS

/*
 * Filenames to use in the EFS system
 */

#define		CAT_NAME		"__CATALOG__"

#define		TMP_FILE		"__TMPSAVE__"
#define		TMP_CAT			"__TMP_CAT__"
#define		BAK_FILE		"__BAKFILE__"
#define		BAK_CAT			"__BAK_CAT__"

#endif	/* EFS */

#ifdef	INF_FS

#define		TMP_FILE		"__TMPSAVE__"
#define		TMP_INF			"__TMP_INF__"

#endif	/* INFO_FS */
#endif	/* EMUL_FS */

/*
 * The R65C02 includes all the R65C12 instructions...
 */

#ifdef		R65C02
#define		R65C12
#endif


/*
 * handy typedefs
 */

typedef	unsigned char		byteval;

/*
 * Bits to handle instruction disassembly
 */

#ifdef	DISASS

#define	Disassemble1(x)		if ( DebugLevel & DISASSEMBLE ) printf ( x )
#define	Disassemble2(x,y)	if ( DebugLevel & DISASSEMBLE ) printf ( x, y )
#define	Disassemble3(x,y,z)	if ( DebugLevel & DISASSEMBLE ) printf ( x, y, z )

#else	/* DISASS */

#define	Disassemble1(x)		/* nothing */
#define	Disassemble2(x,y)	/* nothing */
#define	Disassemble3(x,y,z)	/* nothing */

#endif	/* DISASS */

#endif	/* CONFIG_H */
