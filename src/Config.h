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


/*
 * Potential performance gains
 *
 * LO_PAGE			assumes that writes to pages 0 and 1 can be made directly
 *					without checking that any side-effects might occur (such as
 *					the screen being modified.
 *
 * FASTCLOCK		causes the VIA timers etc. to be updated every 100 cycles
 *					rather than every instruction.
 *
 * LITTLE_ENDIAN	Causes writing words to the memory array to be done in one
 *					operation as an unsigned 16-bit value rather than as two
 *					8-bit values.  Only possible because the 6502 is little-
 *					endian.
 *
 */

#define		LO_PAGE
#define		FASTCLOCK
#define		LITTLE_ENDIAN

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
 *					CPU executes.  Very useful for debugging.
 *
 * INFO				Gives all sorts of information about what's happening
 *					with the hardware emulation.
 *
 * RANGE_CHECK		Adds code for range-checking of operands etc.  Just
 *					slows things down, but helpful for debugging.
 *
 * NEED_STRCASECMP	If your system doesn't have the "strcasecmp" function.
 *
 * EMUL_FS			Enables the emulated file-system code
 *
 * COUNT_INSTRS		Count how many times each op-code was executed
 *
 * EFS_CATALOG_SIZE	The number of entries allowed in the EFS catalog
 *
 */

#define		NO_FRED_JIM
#undef		LIMIT			/* 5000000 */
#undef		DISASS			/* DISASSEMBLE */
#undef		INFO
#undef		RANGE_CHECK
#undef		NEED_STRCASECMP

#define		EMUL_FS
#undef		COUNT_INSTRS

#define		EFS_CATALOG_SIZE	31

/*
 * Default location for all of xbeeb's files
 */

#define		XBEEBROOT		"/home/users/james/beeb/"

#define		XBEEBROMS		XBEEBROOT"roms/"
#define		XBEEBSNAPS		XBEEBROOT"snaps/"
#define		XBEEBDISKS		XBEEBROOT"disks/"
#define		XBEEBTAPES		XBEEBROOT"tapes/"

#define		CAT_NAME		"__CATALOG__"

#define		TMP_FILE		"__TMPSAVE__"
#define		TMP_CAT			"__TMP_CAT__"
#define		BAK_FILE		"__BAKFILE__"
#define		BAK_CAT			"__BAK_CAT__"

/*
 * Names of ROMs to load by default
 */

#ifdef	EMUL_FS
#define		OS_ROM			"OS1.2p1.rom"
#else
#define		OS_ROM			"OS1.2.rom"
#endif
#define		LANG_ROM		"BASIC2.rom"

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

/*
 * handy typedefs
 */

typedef	unsigned char	byteval;

/*
 * Bits to handle instructions disassembly
 */

#ifdef	DISASS
#define	Disassemble1(x)		if ( DebugLevel & DISASSEMBLE ) printf ( x )
#define	Disassemble2(x,y)	if ( DebugLevel & DISASSEMBLE ) printf ( x, y )
#else
#define	Disassemble1(x)		/* nothing */
#define	Disassemble2(x,y)	/* nothing */
#endif
