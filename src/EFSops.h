/*
 *
 * $Id: EFSops.h,v 1.2 1996/09/24 23:05:37 james Exp $
 *
 * Copyright (c) James Fidell 1995, 1996.
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
 * $Log: EFSops.h,v $
 * Revision 1.2  1996/09/24 23:05:37  james
 * Update copyright dates.
 *
 * Revision 1.1  1996/09/24 22:40:16  james
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
 *
 */


#ifndef	EFSOPS_H
#define	EFSOPS_H

#undef	HALT02
#undef	NOP02
#define	EFS_OSFSC	0x02		/* Dummy opcode for OSFSC entry trap */

#undef	HALT22
#undef	NOP22
#define	EFS_OSFILE	0x22		/* Dummy opcode for OSFILE entry trap */

#endif	/* EFSOPS_H */
