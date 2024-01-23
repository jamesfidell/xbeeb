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


#define	Adc(v) \
{ \
    unsigned char	Aold; \
\
	Aold = Accumulator; \
	if ( DecimalModeFlag ) \
	{ \
		int			Alo = Accumulator & 0x0f; \
		int			Ahi = Accumulator >> 4; \
		int			vlo = v & 0x0f; \
		int			vhi = v >> 4; \
\
		Alo += vlo + CarryFlag; \
		if ( Alo >= 10 ) \
		{ \
			Alo -= 10; \
			Ahi++; \
		} \
		Ahi += vhi; \
		if ( Ahi >= 10 ) \
		{ \
			Ahi -= 10; \
			SetCarryFlag; \
		} \
		else \
			CarryFlag = 0; \
		Accumulator = Alo + ( Ahi << 4 ); \
	} \
	else \
	{ \
		int				Anew; \
\
		Anew = Accumulator + v + CarryFlag; \
		CarryFlag = Anew >> 8; \
		Accumulator = Anew & 0xff; \
		ResetZeroFlag ( Accumulator == 0x0 ); \
		NegativeFlag = Accumulator & 0x80; \
		ResetOverflowFlag ((!(( Aold ^ v) & 0x80 )) && \
							(( Aold ^ Accumulator ) & 0x80 )); \
	} \
}


#define	And(v) \
	Accumulator &= v; \
	ResetZeroFlag ( Accumulator == 0x0 ); \
	NegativeFlag = Accumulator & 0x80


#define ShiftLeft(v) \
	CarryFlag = v >> 7; \
	v <<= 1; \
	NegativeFlag = v & 0x80; \
	ResetZeroFlag ( v == 0 )


#define Bit(v) \
	ResetZeroFlag (( v & Accumulator ) == 0x0 ); \
	OverflowFlag = v & 0x40; \
	NegativeFlag = v & 0x80


#define Cmp(reg,mem) \
{ \
	int			 	Rnew; \
 \
	if ( DecimalModeFlag ) \
	{ \
		printf ( "BCD compare not implemented\n" ); \
		DisplayRegisters(); \
		exit ( 1 ); \
	} \
	ResetCarryFlag ( reg >= mem ); \
	ResetZeroFlag ( reg == mem ); \
	Rnew = reg - mem; \
	NegativeFlag = Rnew & 0x80; \
}


#define	Eor(v) \
	Accumulator ^= v; \
	ResetZeroFlag ( Accumulator == 0x0 ); \
	NegativeFlag = Accumulator & 0x80


#define ShiftRight(v) \
	CarryFlag = v & 0x01; \
	v >>= 1; \
	NegativeFlag = 0; \
	ResetZeroFlag ( v == 0 )


#define Ora(v) \
	Accumulator |= v; \
	ResetZeroFlag ( Accumulator == 0x00 ); \
	NegativeFlag = Accumulator & 0x80;


#define RotateLeft(v) \
	temp2 = CarryFlag; \
	CarryFlag = v >> 7; \
	v <<= 1; \
	v |= temp2; \
	ResetZeroFlag ( v == 0 ); \
	NegativeFlag = v & 0x80


#define RotateRight(v) \
	temp2 = CarryFlag; \
	CarryFlag = v & 0x01; \
	v >>= 1; \
	v |= ( temp2 << 7 ); \
	ResetZeroFlag ( v == 0 ); \
	NegativeFlag = v & 0x80


#define Sbc(v) \
{ \
	unsigned char	Aold = Accumulator; \
\
	if ( DecimalModeFlag ) \
	{ \
		int			Alo = Accumulator & 0x0f; \
		int			Ahi = Accumulator >> 4; \
		int			ilo = v & 0x0f; \
		int			ihi = v >> 4; \
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
		{ \
			Ahi += 10; \
			CarryFlag = 0; \
		} \
		else \
			SetCarryFlag; \
\
		Accumulator = Alo + ( Ahi << 4 ); \
	} \
	else \
	{ \
		int				Anew; \
\
		Anew = Aold - v + CarryFlag - 1; \
		Accumulator = Anew & 0xff; \
\
		ResetCarryFlag ( Anew >= 0x0 ); \
		ResetZeroFlag ( Accumulator == 0x0 ); \
		NegativeFlag = Accumulator & 0x80; \
		ResetOverflowFlag ((!(( Aold ^ v ) & 0x80 )) && \
							(( Aold ^ Accumulator ) & 0x80 )); \
\
	} \
} 


#define	LoadA(v) \
	Accumulator = v; \
	ResetZeroFlag ( Accumulator == 0 ); \
	NegativeFlag = Accumulator & 0x80


#define	LoadX(v) \
	RegisterX = v; \
	ResetZeroFlag ( RegisterX == 0 ); \
	NegativeFlag = RegisterX & 0x80


#define	LoadY(v) \
	RegisterY = v; \
	ResetZeroFlag ( RegisterY == 0 ); \
	NegativeFlag = RegisterY & 0x80


#define	DoBranch(c) \
if ( c ) \
{ \
	unsigned int		old; \
	unsigned int		new; \
	union \
	{ \
		unsigned char	u; \
		signed char		s; \
	} offset; \
\
	old = GetProgramCounter & 0xff00; \
	offset.u = *EmulatorPC++; \
	EmulatorPC += offset.s; \
	new = GetProgramCounter & 0xff00; \
	BranchCycleCount += ( 1 + (( old == new ) ? 0 : 2 )); \
} \
else \
	EmulatorPC++


#define	StackByte(v) \
	WriteLoPageByte ( StackPointer | STACK_PAGE, v ); \
	StackPointer--;


#define StackWord(v) \
	StackPointer--; \
	WriteWord ( StackPointer | STACK_PAGE, v ); \
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
