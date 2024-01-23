/*
 *
 * $Id: R65C12ops.h,v 1.4 2002/01/15 15:46:43 james Exp $
 *
 * Copyright (C) James Fidell 1995-2002.
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
 * $Log: R65C12ops.h,v $
 * Revision 1.4  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.3  2000/08/16 17:58:28  james
 * Update copyright message
 *
 * Revision 1.2  1996/09/24 23:05:42  james
 * Update copyright dates.
 *
 * Revision 1.1  1996/09/24 22:41:21  james
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


#ifndef	OPS65c12_H
#define	OPS65c12_H

#define	op_NOP02		0x02
#define	op_NOP03		0x03
#define	op_TSB_ZP		0x04
#define	op_NOP07		0x07
#define	op_TSB_Ab		0x0c
#define	op_BBR0			0x0f
#define	op_NOP0b		0x0b

#define	op_ORA_Ind		0x12
#define	op_NOP13		0x13
#define	op_TRB_ZP		0x14
#define	op_NOP17		0x17
#define	op_INC_A		0x1a
#define	op_NOP1b		0x1b
#define	op_TRB_Ab		0x1c
#define	op_BBR1			0x1f

#define	op_NOP22		0x22
#define	op_NOP23		0x23
#define	op_NOP27		0x27
#define	op_NOP2b		0x2b
#define	op_BBR2			0x2f

#define	op_AND_Ind		0x32
#define	op_NOP33		0x33
#define	op_BIT_ZPX		0x34
#define	op_NOP37		0x37
#define	op_DEC_A		0x3a
#define	op_NOP3b		0x3b
#define	op_BIT_AbX		0x3c
#define	op_BBR3			0x3f

#define	op_NOP42		0x42
#define	op_NOP43		0x43
#define	op_NOP44		0x44
#define	op_NOP47		0x47
#define	op_NOP4b		0x4b
#define	op_BBR4			0x4f

#define	op_EOR_Ind		0x52
#define	op_NOP53		0x53
#define	op_NOP54		0x54
#define	op_NOP57		0x57
#define	op_PHY			0x5a
#define	op_NOP5b		0x5b
#define	op_BBR5			0x5f

#define	op_NOP62		0x62
#define	op_NOP63		0x63
#define	op_STZ_ZP		0x64
#define	op_NOP67		0x67
#define	op_NOP6b		0x6b
#define	op_BBR6			0x6f

#define	op_ADC_Ind		0x72
#define	op_NOP73		0x73
#define	op_STZ_ZPX		0x74
#define	op_NOP77		0x77
#define	op_PLY			0x7a
#define	op_NOP7b		0x7b
#define	op_JMP_IndX		0x7c
#define	op_BBR7			0x7f

#define	op_BRA			0x80
#define	op_NOP82		0x82
#define	op_NOP83		0x83
#define	op_NOP87		0x87
#define	op_BIT_Im		0x89
#define	op_NOP8b		0x8b
#define	op_BBS0			0x8f

#define	op_STA_Ind		0x92
#define	op_NOP93		0x93
#define	op_NOP97		0x97
#define	op_NOP9b		0x9b
#define	op_STZ_Ab		0x9c
#define	op_STZ_AbX		0x9e
#define	op_BBS1			0x9f

#define	op_NOPa3		0xa3
#define	op_NOPa7		0xa7
#define	op_NOPab		0xab
#define	op_BBS2			0xaf

#define	op_LDA_Ind		0xb2
#define	op_NOPb3		0xb3
#define	op_NOPb7		0xb7
#define	op_NOPbb		0xbb
#define	op_BBS3			0xbf

#define	op_NOPc2		0xc2
#define	op_NOPc3		0xc3
#define	op_NOPc7		0xc7
#define	op_NOPcb		0xcb
#define	op_BBS4			0xcf

#define	op_CMP_Ind		0xd2
#define	op_NOPd3		0xd3
#define	op_NOPd4		0xd4
#define	op_NOPd7		0xd7
#define	op_PHX			0xda
#define	op_NOPdb		0xdb
#define	op_NOPdc		0xdc
#define	op_BBS5			0xdf

#define	op_NOPe2		0xe2
#define	op_NOPe3		0xe3
#define	op_NOPe7		0xe7
#define	op_NOPeb		0xeb
#define	op_BBS6			0xef

#define	op_SBC_Ind		0xf2
#define	op_NOPf3		0xf3
#define	op_NOPf4		0xf4
#define	op_NOPf7		0xf7
#define	op_PLX			0xfa
#define	op_NOPfb		0xfb
#define	op_NOPfc		0xfc
#define	op_BBS7			0xff

#endif	/* OPS65c12_H */
