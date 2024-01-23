/*
 *
 * $Id: Ops.h,v 1.12 1996/09/24 23:05:40 james Exp $
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
 * $Log: Ops.h,v $
 * Revision 1.12  1996/09/24 23:05:40  james
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
 * Revision 1.10  1996/09/24 21:35:42  james
 * Correct syntax error.
 *
 * Revision 1.9  1996/09/24 18:43:04  james
 * Updates to SBC as per jopi's documentation (and some additional related
 * commentary on CMP).
 *
 * Revision 1.8  1996/09/24 18:40:12  james
 * Put all parameters to #defined operations in parentheses wherever they
 * are used, to reduce the chance of an error and to keep the pedants
 * happy.
 *
 * Revision 1.7  1996/09/24 18:29:21  james
 * Added code for properly handling BDC ADC flags, as per Jouko Valta's
 * 65xx instruction set document
 *
 * Revision 1.6  1996/09/24 18:25:29  james
 * Add defines to prevent problems with multiple inclusion of Ops.h
 *
 * Revision 1.5  1996/09/22 21:42:57  james
 * Decimal mode CMP works the same way as non-decimal mode.
 *
 * Revision 1.4  1996/09/21 23:21:47  james
 * Improvements to branch-handling code.
 *
 * Revision 1.3  1996/09/21 22:54:49  james
 * Fix code for displaying registers (because the variables are no longer
 * globals)
 *
 * Revision 1.2  1996/09/21 22:13:49  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.1  1996/09/21 17:20:39  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


#ifndef	OPS_H
#define	OPS_H

/*
 * Flag-setting for the ADC command is implemented as per Jouko Valta's
 * <jopi@stekt.oulu.fi> 65xx instruction code document.
 */

#define	Adc(v) \
{ \
    byteval 		Aold; \
\
	Aold = Accumulator; \
	if ( DecimalModeFlag ) \
	{ \
		int			Alo = Accumulator & 0x0f; \
		int			Ahi = Accumulator >> 4; \
		int			vlo = (v) & 0x0f; \
		int			vhi = (v) >> 4; \
\
		Alo += vlo + CarryFlag; \
		if ( Alo >= 10 ) \
		{ \
			Alo -= 10; \
			Ahi++; \
		} \
		Ahi += vhi; \
		Accumulator = ( Alo + ( Ahi << 4 )) & 0xff; \
		ResetZeroFlag ( Accumulator == 0x0 ); \
		NegativeFlag = Accumulator & 0x80; \
		ResetOverflowFlag ((!(( Aold ^ (v)) & 0x80 )) && \
							(( Aold ^ Accumulator ) & 0x80 )); \
		CarryFlag = 0; \
		if ( Ahi >= 10 ) \
		{ \
			Ahi += 6; \
			if ( Ahi > 15 ) \
				SetCarryFlag; \
			Ahi &= 0xf; \
		} \
		Accumulator = Alo + ( Ahi << 4 ); \
	} \
	else \
	{ \
		int				Anew; \
\
		Anew = Accumulator + (v) + CarryFlag; \
		CarryFlag = Anew >> 8; \
		Accumulator = Anew & 0xff; \
		ResetZeroFlag ( Accumulator == 0x0 ); \
		NegativeFlag = Accumulator & 0x80; \
		ResetOverflowFlag ((!(( Aold ^ (v)) & 0x80 )) && \
							(( Aold ^ Accumulator ) & 0x80 )); \
	} \
}


#define	And(v) \
	Accumulator &= (v); \
	ResetZeroFlag ( Accumulator == 0x0 ); \
	NegativeFlag = Accumulator & 0x80


#define ShiftLeft(v) \
	CarryFlag = (v) >> 7; \
	(v) <<= 1; \
	NegativeFlag = (v) & 0x80; \
	ResetZeroFlag ((v) == 0 )


#define Bit(v) \
	ResetZeroFlag (((v) & Accumulator ) == 0x0 ); \
	OverflowFlag = (v) & 0x40; \
	NegativeFlag = (v) & 0x80


/*
 * According to jopi's 65xx docs., the D flag doesn't affect CMP, so
 * we're ok here.
 */

#define Cmp(reg,mem) \
{ \
	int			 	Rnew; \
 \
	ResetCarryFlag (( reg ) >= ( mem )); \
	ResetZeroFlag (( reg ) == ( mem )); \
	Rnew = ( reg ) - ( mem ); \
	NegativeFlag = Rnew & 0x80; \
}


#define	Eor(v) \
	Accumulator ^= (v); \
	ResetZeroFlag ( Accumulator == 0x0 ); \
	NegativeFlag = Accumulator & 0x80


#define ShiftRight(v) \
	CarryFlag = (v) & 0x01; \
	(v) >>= 1; \
	NegativeFlag = 0; \
	ResetZeroFlag ((v) == 0 )


#define Ora(v) \
	Accumulator |= (v); \
	ResetZeroFlag ( Accumulator == 0x00 ); \
	NegativeFlag = Accumulator & 0x80;


#define RotateLeft(v) \
	temp2 = CarryFlag; \
	CarryFlag = (v) >> 7; \
	(v) <<= 1; \
	(v) |= temp2; \
	ResetZeroFlag ((v) == 0 ); \
	NegativeFlag = (v) & 0x80


#define RotateRight(v) \
	temp2 = CarryFlag; \
	CarryFlag = (v) & 0x01; \
	(v) >>= 1; \
	(v) |= ( temp2 << 7 ); \
	ResetZeroFlag ((v) == 0 ); \
	NegativeFlag = (v) & 0x80


#define Sbc(v) \
{ \
	byteval			Aold = Accumulator; \
	int				Anew; \
\
	Anew = Aold - (v) + CarryFlag - 1; \
	Accumulator = Anew & 0xff; \
\
	ResetCarryFlag ( Anew >= 0x0 ); \
	NegativeFlag = Accumulator & 0x80; \
	ResetOverflowFlag ((!(( Aold ^ (v)) & 0x80 )) && \
						(( Aold ^ Accumulator ) & 0x80 )); \
	ResetZeroFlag ( Accumulator == 0x0 ); \
\
	if ( DecimalModeFlag ) \
	{ \
		int			Alo = Aold & 0x0f; \
		int			Ahi = Aold >> 4; \
		int			ilo = (v) & 0x0f; \
		int			ihi = (v) >> 4; \
\
		Alo -= ilo + CarryFlag - 1; \
		if ( Alo < 0 ) \
		{ \
			Alo += 10; \
			Ahi--; \
		} \
\
		Ahi -= ihi; \
		if ( Ahi < 0 ) \
			Ahi += 10; \
\
		Accumulator = Alo + ( Ahi << 4 ); \
	} \
} 


#define	LoadA(v) \
	Accumulator = (v); \
	ResetZeroFlag ( Accumulator == 0 ); \
	NegativeFlag = Accumulator & 0x80


#define	LoadX(v) \
	RegisterX = (v); \
	ResetZeroFlag ( RegisterX == 0 ); \
	NegativeFlag = RegisterX & 0x80


#define	LoadY(v) \
	RegisterY = (v); \
	ResetZeroFlag ( RegisterY == 0 ); \
	NegativeFlag = RegisterY & 0x80


/*
 * POSSIBLE ENHANCEMENT ?
 *
 * If the branch takes place and we go to a different page, there is an
 * intermediate read of the same page whilst the high byte of the real
 * branch address has the carry added.
 */

#define	DoBranch(c) \
if ( c ) \
{ \
	unsigned int		old; \
	unsigned int		new; \
\
	old = GetProgramCounter & 0xff00; \
	EmulatorPC += ( signed char )( *EmulatorPC++ ); \
	new = GetProgramCounter & 0xff00; \
	BranchCycleCount = ( old == new ) ? CLK_REL_OK : CLK_REL_PAGECROSS; \
} \
else \
{ \
	EmulatorPC++; \
	BranchCycleCount = CLK_REL_FAIL; \
}


#define	StackByte(v) \
	WriteLoPageByte ( StackPointer | STACK_PAGE, (v)); \
	StackPointer--;


#define StackWord(v) \
	StackPointer--; \
	WriteWord ( StackPointer | STACK_PAGE, (v)); \
	StackPointer--;


#define	UnstackWord() ( \
	StackPointer += 2, \
	ReadWord ( StackPointer + STACK_PAGE - 1 ))


#define UnstackByte() \
	( StackPointer++, ReadLoPageByte ( StackPointer | STACK_PAGE ))


#define	StackPC()		StackWord ( GetProgramCounter )


#define	UnstackPC()		SetProgramCounter ( UnstackWord());


#define	StackSR() \
	GenerateStatusRegister; \
	StackByte ( StatusRegister )


#define	UnstackSR() \
	StatusRegister = UnstackByte(); \
	GenerateStatusFlags


#ifdef	M6502

#define Halt() \
	fprintf ( stderr, "Processor executing HALT opcode %02x\n", opcode ); \
	fprintf ( stderr, "Xbeeb terminating\n" ); \
	goto ExitEmulator;


#define	Slo(v,m) \
	CarryFlag = (v) >> 7; \
    (v) <<= 1; \
	(v) |= (m); \
	(v) &= 0xff; \
	NegativeFlag = (v) & 0x80; \
	ResetZeroFlag ((v) == 0 )


#define	Rra(v) \
	temp2 = (v) >> 1; \
	temp2 |= ( CarryFlag << 7 ); \
	ResetZeroFlag ( temp2 == 0 ); \
	NegativeFlag = temp2 & 0x80; \
	OverflowFlag = ( temp2 ^ (v)) & 0x40; \
	if ( DecimalModeFlag ) \
	{ \
		if (((v) & 0xf ) > 0x4 ) \
			temp2 = ( temp2 & 0xf0 ) | (( temp2 + 6 ) & 0xf ); \
		if ((v) >= 0x50 ) \
			temp2 += 0x60; \
		ResetCarryFlag ((v) >= 0x50 ); \
	} \
    else \
        CarryFlag = (v) >> 7; \
	(v) = temp2

#endif	/* M6502 */


#ifdef	R65C12

/*
 * POSSIBLE ENHANCEMENT ?
 *
 * If the branch takes place and we go to a different page, there is an
 * intermediate read of the same page whilst the high byte of the real
 * branch address has the carry added.
 */

#define	BitBranch(c) \
if (c) \
{ \
	unsigned int		old; \
	unsigned int		new; \
\
	old = GetProgramCounter & 0xff00; \
	EmulatorPC += ( signed char )( *EmulatorPC++ ); \
	new = GetProgramCounter & 0xff00; \
	BranchCycleCount = ( old == new ) ? CLK_BB_REL_OK : CLK_BB_REL_PAGECROSS; \
} \
else \
{ \
	EmulatorPC++; \
	BranchCycleCount = CLK_BB_REL_FAIL; \
}


#define	Trb(v) \
	(v) &= ~Accumulator; \
	ResetZeroFlag ((v) == 0 )


#define	Tsb(v) \
	(v) |= Accumulator; \
	ResetZeroFlag ((v) == 0 )

#endif	/* R65C12 */

#endif	/* OPS_H */
