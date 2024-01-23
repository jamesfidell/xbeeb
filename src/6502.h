/*
 *
 * $Id: 6502.h,v 1.6 1996/09/30 22:59:06 james Exp $
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
 * $Log: 6502.h,v $
 * Revision 1.6  1996/09/30 22:59:06  james
 * Prevent multiple inclusion of header files.
 *
 * Revision 1.5  1996/09/24 23:05:32  james
 * Update copyright dates.
 *
 * Revision 1.4  1996/09/24 22:40:15  james
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
 * Revision 1.3  1996/09/22 21:00:53  james
 * Changed IRQ-handling code to be called only when I think an IRQ may have
 * happened.  Converted it all to #defines so that it can be compiled inline.
 *
 * Revision 1.2  1996/09/21 22:13:45  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:34  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	CPU6502_H
#define	CPU6502_H

/*
 * InitCPU sets the CPU up with the correct PC/flags.
 */

extern	void	InitialiseCPU();

/*
 * Function to request an interrupt
 */

#ifndef	IRQ
extern	void	IRQ();
#endif

/*
 * Save the CPU state to a file
 */

extern	int		SaveCPU ( int );
extern	int		RestoreCPU ( int, unsigned int );


#define	STACK_PAGE					0x0100
#define	NMI_VECTOR					0xfffa
#define RESET_VECTOR				0xfffc
#define IRQ_VECTOR					0xfffe

extern	unsigned int				NMIAddress;
extern	unsigned int				IRQAddress;
extern	unsigned int				ResetAddress;

extern	byteval						CarryFlag;
extern	byteval						ZeroFlag;
extern	byteval						IRQDisableFlag;
extern	byteval						DecimalModeFlag;
extern	byteval						OverflowFlag;
extern	byteval						NegativeFlag;

#define	SetCarryFlag				CarryFlag = 0x01
#define	SetZeroFlag					ZeroFlag = 0x02
#define	SetIRQDisableFlag			IRQDisableFlag = 0x04
#define	SetDecimalModeFlag			DecimalModeFlag = 0x08
#define SetOverflowFlag				OverflowFlag = 0x40
#define	SetNegativeFlag				NegativeFlag = 0x80

#define	ResetCarryFlag(cond)		CarryFlag = ( cond ) ? 0x01 : 0x00
#define	ResetZeroFlag(cond)			ZeroFlag = ( cond ) ? 0x02 : 0x00
#define ResetOverflowFlag(cond)		OverflowFlag = ( cond ) ? 0x40 : 0x00

#define	GenerateStatusRegister \
	StatusRegister = CarryFlag | ZeroFlag | IRQDisableFlag | DecimalModeFlag \
					| 0x30 | OverflowFlag | NegativeFlag

#define	GenerateIRQStatusRegister \
	StatusRegister = CarryFlag | ZeroFlag | IRQDisableFlag | DecimalModeFlag \
					| 0x20 | OverflowFlag | NegativeFlag

#define	GenerateStatusFlags \
	CarryFlag = StatusRegister & 0x01; \
	ZeroFlag = StatusRegister & 0x02; \
	IRQDisableFlag = StatusRegister & 0x04; \
	DecimalModeFlag = StatusRegister & 0x08; \
	OverflowFlag = StatusRegister & 0x40; \
	NegativeFlag = StatusRegister & 0x80


/*
 * Timings for all the different instruction types...
 */

#define	CLK_IMPLIED					2		/* Implied addressing */
#define	CLK_IMMEDIATE				2		/* Immediate addressing */
#define	CLK_REL_FAIL				2		/* Failed branch */

#define	CLK_STACK_PUSH				3		/* Push onto stack */
#define	CLK_ZP_READ					3		/* Read from ZP */
#define	CLK_ZP_WRITE				3		/* Write to ZP */
#define	CLK_ABS_JMP					3		/* Read JMP from absolute addr. */

#define	CLK_STACK_PULL				4		/* Pull off stack */
#define	CLK_REL_OK					4		/* Branch to same page */
#define	CLK_ZPI_READ				4		/* Read from ZP,[XY] */
#define	CLK_ZPI_WRITE				4		/* Write to ZP,[XY] */
#define	CLK_ABS_READ				4		/* Read from Absolute address */
#define	CLK_ABS_WRITE				4		/* Write to Absolute address */
#define	CLK_ABI_READ				4		/* Read from Abs,[XY] */
											/* (no cross of page boundary) */

#define	CLK_REL_PAGECROSS			5		/* Branch to different page */
#define	CLK_ZP_READ_MOD_WRITE		5		/* Read-Modify-Write ZP */
#define	CLK_ABI_WRITE				5		/* Write to Abs,[XY] */
#define	CLK_ABS_INDIRECT			5		/* Absolute indirect (JMP only) */
#define	CLK_ABI_READ_PAGECROSS		5		/* Read from Abs,[XY] */
											/* and cross page boundary */
#define	CLK_INDIDX_READ				5		/* Read from (ZP),Y not crossing */
											/* page boundary when adding Y */

#define	CLK_RTI						6		/* RTI only */
#define	CLK_RTS						6		/* RTS only */
#define	CLK_JSR						6		/* JSR only */
#define	CLK_ZPI_READ_MOD_WRITE		6		/* Read-Modify-Write ZP,[XY] */
#define	CLK_ABS_READ_MOD_WRITE		6		/* Read-Modify-Write Absolute */
#define	CLK_IDXIND_READ				6		/* Read from (ZP,X) */
#define	CLK_IDXIND_WRITE			6		/* Write to (ZP,X) */
#define	CLK_INDIDX_WRITE			6		/* Write to (ZP),Y */
#define	CLK_INDIDX_READ_PAGECROSS	6		/* Read from (ZP),Y crossing */
											/* page boundary when adding Y */

#define	CLK_INTERRUPT				7		/* BRK or IRQ */
#define	CLK_ABI_READ_MOD_WRITE		7		/* Read-Modify-Write Abs,[XY] */

#define	CLK_IDXIND_READ_MOD_WRITE	8		/* Read-Modify-Write (ZP,X) */
#define	CLK_INDIDX_READ_MOD_WRITE	8		/* Read-Modify-Write (ZP),Y */


#ifdef	R65C12

#define	CLK_BB_REL_FAIL				5		/* Failed bit test & branch */
#define	CLK_IND_READ				5		/* Read from (ZP) */
#define	CLK_IND_WRITE				5		/* Write to (ZP) */

#undef	CLK_ABS_INDIRECT
#define	CLK_ABS_INDIRECT			6		/* Absolute indirect (JMP only) */
#define	CLK_BB_REL_OK				6		/* bit test/branch to same page */

#define	CLK_BB_REL_PAGECROSS		7		/* bit test/branch to other page */
#define	CLK_ABS_INDIRECT_PAGECROSS	7		/* Absolute indirect (JMP only) */

#endif	/* R65C12 */

#endif	/* CPU6502_H */
