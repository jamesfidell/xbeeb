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


#ifndef	OPCODES_H
#define	OPCODES_H


#define	BRK			0x00
#define	ORA_IndX	0x01



#define	ORA_ZP		0x05
#define	ASL_ZP		0x06

#define	PHP			0x08
#define	ORA_Im		0x09
#define	ASL_A		0x0a


#define	ORA_Ab		0x0d
#define	ASL_Ab		0x0e

#define	BPL			0x10
#define	ORA_IndY	0x11



#define	ORA_ZPX		0x15
#define	ASL_ZPX		0x16

#define	CLC			0x18
#define	ORA_AbY		0x19



#define	ORA_AbX		0x1d
#define	ASL_AbX		0x1e

#define	JSR			0x20
#define	AND_IndX	0x21


#define	BIT_ZP		0x24
#define	AND_ZP		0x25
#define	ROL_ZP		0x26

#define	PLP			0x28
#define	AND_Im		0x29
#define	ROL_A		0x2a

#define	BIT_Ab		0x2c
#define	AND_Ab		0x2d
#define	ROL_Ab		0x2e

#define	BMI			0x30
#define	AND_IndY	0x31



#define	AND_ZPX		0x35
#define	ROL_ZPX		0x36

#define	SEC			0x38
#define	AND_AbY		0x39



#define	AND_AbX		0x3d
#define	ROL_AbX		0x3e

#define	RTI			0x40
#define	EOR_IndX	0x41



#define	EOR_ZP		0x45
#define	LSR_ZP		0x46

#define	PHA			0x48
#define	EOR_Im		0x49
#define	LSR_A		0x4a

#define	JMP_Ab		0x4c
#define	EOR_Ab		0x4d
#define	LSR_Ab		0x4e

#define	BVC			0x50
#define	EOR_IndY	0x51



#define	EOR_ZPX		0x55
#define	LSR_ZPX		0x56

#define	CLI			0x58
#define	EOR_AbY		0x59



#define	EOR_AbX		0x5d
#define	LSR_AbX		0x5e

#define	RTS			0x60
#define	ADC_IndX	0x61



#define	ADC_ZP		0x65
#define	ROR_ZP		0x66

#define	PLA			0x68
#define	ADC_Im		0x69
#define	ROR_A		0x6a

#define	JMP_Ind		0x6c
#define	ADC_Ab		0x6d
#define	ROR_Ab		0x6e

#define	BVS			0x70
#define	ADC_IndY	0x71



#define	ADC_ZPX		0x75
#define	ROR_ZPX		0x76

#define	SEI			0x78
#define	ADC_AbY		0x79



#define	ADC_AbX		0x7d
#define	ROR_AbX		0x7e


#define	STA_IndX	0x81


#define	STY_ZP		0x84
#define	STA_ZP		0x85
#define	STX_ZP		0x86

#define	DEY			0x88

#define	TXA			0x8a

#define	STY_Ab		0x8c
#define	STA_Ab		0x8d
#define	STX_Ab		0x8e

#define	BCC			0x90
#define	STA_IndY	0x91


#define	STY_ZPX		0x94
#define	STA_ZPX		0x95
#define	STX_ZPY		0x96

#define	TYA			0x98
#define	STA_AbY		0x99
#define	TXS			0x9a


#define	STA_AbX		0x9d


#define	LDY_Im		0xa0
#define	LDA_IndX	0xa1
#define	LDX_Im		0xa2

#define	LDY_ZP		0xa4
#define	LDA_ZP		0xa5
#define	LDX_ZP		0xa6

#define	TAY			0xa8
#define	LDA_Im		0xa9
#define	TAX			0xaa

#define	LDY_Ab		0xac
#define	LDA_Ab		0xad
#define	LDX_Ab		0xae

#define	BCS			0xb0
#define	LDA_IndY	0xb1


#define	LDY_ZPX		0xb4
#define	LDA_ZPX		0xb5
#define	LDX_ZPY		0xb6

#define	CLV			0xb8
#define	LDA_AbY		0xb9
#define	TSX			0xba

#define	LDY_AbX		0xbc
#define	LDA_AbX		0xbd
#define	LDX_AbY		0xbe

#define	CPY_Im		0xc0
#define	CMP_IndX	0xc1


#define	CPY_ZP		0xc4
#define	CMP_ZP		0xc5
#define	DEC_ZP		0xc6

#define	INY			0xc8
#define	CMP_Im		0xc9
#define	DEX			0xca

#define	CPY_Ab		0xcc
#define	CMP_Ab		0xcd
#define	DEC_Ab		0xce

#define	BNE			0xd0
#define	CMP_IndY	0xd1


#ifdef	EMUL_FS
#define	EFS_OSFILE	0xd4		/* Dummy opcode for OSFILE entry trap */
#endif
#define	CMP_ZPX		0xd5
#define	DEC_ZPX		0xd6
#ifdef	EMUL_FS
#define	EFS_OSFSC	0xd7		/* Dummy opcode for OSFSC entry trap */
#endif

#define	CLD			0xd8
#define	CMP_AbY		0xd9



#define	CMP_AbX		0xdd
#define	DEC_AbX		0xde

#define	CPX_Im		0xe0
#define	SBC_IndX	0xe1


#define	CPX_ZP		0xe4
#define	SBC_ZP		0xe5
#define	INC_ZP		0xe6

#define	INX			0xe8
#define	SBC_Im		0xe9
#define	NOP			0xea

#define	CPX_Ab		0xec
#define	SBC_Ab		0xed
#define	INC_Ab		0xee

#define	BEQ			0xf0
#define	SBC_IndY	0xf1



#define	SBC_ZPX		0xf5
#define	INC_ZPX		0xf6

#define	SED			0xf8
#define	SBC_AbY		0xf9



#define	SBC_AbX		0xfd
#define	INC_AbX		0xfe

#endif	/* OPCODES_H */
