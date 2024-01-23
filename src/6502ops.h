/*
 *
 * $Id: 6502ops.h,v 1.2 1996/09/24 23:05:33 james Exp $
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
 * $Log: 6502ops.h,v $
 * Revision 1.2  1996/09/24 23:05:33  james
 * Update copyright dates.
 *
 * Revision 1.1  1996/09/24 22:40:15  james
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


#ifndef	OPS6502_H
#define	OPS6502_H


#define	op_BRK			0x00
#define	op_ORA_IndX		0x01
#define	op_HALT02		0x02
#define	op_SLO_IndX		0x03
#define	op_NOP04		0x04
#define	op_ORA_ZP		0x05
#define	op_ASL_ZP		0x06
#define	op_SLO_ZP		0x07
#define	op_PHP			0x08
#define	op_ORA_Im		0x09
#define	op_ASL_A		0x0a
#define	op_ANC0b		0x0b
#define	op_NOP0c		0x0c
#define	op_ORA_Ab		0x0d
#define	op_ASL_Ab		0x0e
#define	op_SLO_Ab		0x0f
#define	op_BPL			0x10
#define	op_ORA_IndY		0x11
#define	op_HALT12		0x12
#define	op_SLO_IndY		0x13
#define	op_NOP14		0x14
#define	op_ORA_ZPX		0x15
#define	op_ASL_ZPX		0x16
#define	op_SLO_ZPX		0x17
#define	op_CLC			0x18
#define	op_ORA_AbY		0x19
#define	op_NOP1a		0x1a
#define	op_SLO_AbY		0x1b
#define	op_NOP1c		0x1c
#define	op_ORA_AbX		0x1d
#define	op_ASL_AbX		0x1e
#define	op_SLO_AbX		0x1f
#define	op_JSR			0x20
#define	op_AND_IndX		0x21
#define	op_HALT22		0x22
#define	op_RLA_IndX		0x23
#define	op_BIT_ZP		0x24
#define	op_AND_ZP		0x25
#define	op_ROL_ZP		0x26
#define	op_RLA_ZP		0x27
#define	op_PLP			0x28
#define	op_AND_Im		0x29
#define	op_ROL_A		0x2a
#define	op_ANC2b		0x2b
#define	op_BIT_Ab		0x2c
#define	op_AND_Ab		0x2d
#define	op_ROL_Ab		0x2e
#define	op_RLA_Ab		0x2f
#define	op_BMI			0x30
#define	op_AND_IndY		0x31
#define	op_HALT32		0x32
#define	op_RLA_IndY		0x33
#define	op_NOP34		0x34
#define	op_AND_ZPX		0x35
#define	op_ROL_ZPX		0x36
#define	op_RLA_ZPX		0x37
#define	op_SEC			0x38
#define	op_AND_AbY		0x39
#define	op_NOP3a		0x3a
#define	op_RLA_AbY		0x3b
#define	op_NOP3c		0x3c
#define	op_AND_AbX		0x3d
#define	op_ROL_AbX		0x3e
#define	op_RLA_AbX		0x3f
#define	op_RTI			0x40
#define	op_EOR_IndX		0x41
#define	op_HALT42		0x42
#define	op_SRE_IndX		0x43
#define	op_NOP44		0x44
#define	op_EOR_ZP		0x45
#define	op_LSR_ZP		0x46
#define	op_SRE_ZP		0x47
#define	op_PHA			0x48
#define	op_EOR_Im		0x49
#define	op_LSR_A		0x4a
#define	op_ASR_Im		0x4b
#define	op_JMP_Ab		0x4c
#define	op_EOR_Ab		0x4d
#define	op_LSR_Ab		0x4e
#define	op_SRE_Ab		0x4f
#define	op_BVC			0x50
#define	op_EOR_IndY		0x51
#define	op_HALT52		0x52
#define	op_SRE_IndY		0x53
#define	op_NOP54		0x54
#define	op_EOR_ZPX		0x55
#define	op_LSR_ZPX		0x56
#define	op_SRE_ZPX		0x57
#define	op_CLI			0x58
#define	op_EOR_AbY		0x59
#define	op_NOP5a		0x5a
#define	op_SRE_AbY		0x5b
#define	op_NOP5c		0x5c
#define	op_EOR_AbX		0x5d
#define	op_LSR_AbX		0x5e
#define	op_SRE_AbX		0x5f
#define	op_RTS			0x60
#define	op_ADC_IndX		0x61
#define	op_HALT62		0x62
#define	op_RRA_IndX		0x63
#define	op_NOP64		0x64
#define	op_ADC_ZP		0x65
#define	op_ROR_ZP		0x66
#define	op_RRA_ZP		0x67
#define	op_PLA			0x68
#define	op_ADC_Im		0x69
#define	op_ROR_A		0x6a
#define	op_ARR			0x6b
#define	op_JMP_Ind		0x6c
#define	op_ADC_Ab		0x6d
#define	op_ROR_Ab		0x6e
#define	op_RRA_Ab		0x6f
#define	op_BVS			0x70
#define	op_ADC_IndY		0x71
#define	op_HALT72		0x72
#define	op_RRA_IndY		0x73
#define	op_NOP74		0x74
#define	op_ADC_ZPX		0x75
#define	op_ROR_ZPX		0x76
#define	op_RRA_ZPX		0x77
#define	op_SEI			0x78
#define	op_ADC_AbY		0x79
#define	op_NOP7a		0x7a
#define	op_RRA_AbY		0x7b
#define	op_NOP7c		0x7c
#define	op_ADC_AbX		0x7d
#define	op_ROR_AbX		0x7e
#define	op_RRA_AbX		0x7f
#define	op_NOP80		0x80
#define	op_STA_IndX		0x81
#define	op_NOP82		0x82
#define	op_SAX_IndX		0x83
#define	op_STY_ZP		0x84
#define	op_STA_ZP		0x85
#define	op_STX_ZP		0x86
#define	op_SAX_ZP		0x87
#define	op_DEY			0x88
#define	op_NOP89		0x89
#define	op_TXA			0x8a
#define	op_ANE			0x8b
#define	op_STY_Ab		0x8c
#define	op_STA_Ab		0x8d
#define	op_STX_Ab		0x8e
#define	op_SAX_Ab		0x8f
#define	op_BCC			0x90
#define	op_STA_IndY		0x91
#define	op_HALT92		0x92
#define	op_SHA_IndY		0x93
#define	op_STY_ZPX		0x94
#define	op_STA_ZPX		0x95
#define	op_STX_ZPY		0x96
#define	op_SAX_ZPY		0x97
#define	op_TYA			0x98
#define	op_STA_AbY		0x99
#define	op_TXS			0x9a
#define	op_SHS			0x9b
#define	op_SHY			0x9c
#define	op_STA_AbX		0x9d
#define	op_SHX			0x9e
#define	op_SHA_AbY		0x9f
#define	op_LDY_Im		0xa0
#define	op_LDA_IndX		0xa1
#define	op_LDX_Im		0xa2
#define	op_LAX_IndX		0xa3
#define	op_LDY_ZP		0xa4
#define	op_LDA_ZP		0xa5
#define	op_LDX_ZP		0xa6
#define	op_LAX_ZP		0xa7
#define	op_TAY			0xa8
#define	op_LDA_Im		0xa9
#define	op_TAX			0xaa
#define	op_LXA			0xab
#define	op_LDY_Ab		0xac
#define	op_LDA_Ab		0xad
#define	op_LDX_Ab		0xae
#define	op_LAX_Ab		0xaf
#define	op_BCS			0xb0
#define	op_LDA_IndY		0xb1
#define	op_HALTb2		0xb2
#define	op_LAX_IndY		0xb3
#define	op_LDY_ZPX		0xb4
#define	op_LDA_ZPX		0xb5
#define	op_LDX_ZPY		0xb6
#define	op_LAX_ZPY		0xb7
#define	op_CLV			0xb8
#define	op_LDA_AbY		0xb9
#define	op_TSX			0xba
#define	op_LAS			0xbb
#define	op_LDY_AbX		0xbc
#define	op_LDA_AbX		0xbd
#define	op_LDX_AbY		0xbe
#define	op_LAX_AbY		0xbf
#define	op_CPY_Im		0xc0
#define	op_CMP_IndX		0xc1
#define	op_NOPc2		0xc2
#define	op_DCP_IndX		0xc3
#define	op_CPY_ZP		0xc4
#define	op_CMP_ZP		0xc5
#define	op_DEC_ZP		0xc6
#define	op_DCP_ZP		0xc7
#define	op_INY			0xc8
#define	op_CMP_Im		0xc9
#define	op_DEX			0xca
#define	op_SBX			0xcb
#define	op_CPY_Ab		0xcc
#define	op_CMP_Ab		0xcd
#define	op_DEC_Ab		0xce
#define	op_DCP_Ab		0xcf
#define	op_BNE			0xd0
#define	op_CMP_IndY		0xd1
#define	op_HALTd2		0xd2
#define	op_DCP_IndY		0xd3
#define	op_NOPd4		0xd4
#define	op_CMP_ZPX		0xd5
#define	op_DEC_ZPX		0xd6
#define	op_DCP_ZPX		0xd7
#define	op_CLD			0xd8
#define	op_CMP_AbY		0xd9
#define	op_NOPda		0xda
#define	op_DCP_AbY		0xdb
#define	op_NOPdc		0xdc
#define	op_CMP_AbX		0xdd
#define	op_DEC_AbX		0xde
#define	op_DCP_AbX		0xdf
#define	op_CPX_Im		0xe0
#define	op_SBC_IndX		0xe1
#define	op_NOPe2		0xe2
#define	op_ISB_IndX		0xe3
#define	op_CPX_ZP		0xe4
#define	op_SBC_ZP		0xe5
#define	op_INC_ZP		0xe6
#define	op_ISB_ZP		0xe7
#define	op_INX			0xe8
#define	op_SBC_Im		0xe9
#define	op_NOP			0xea
#define	op_SBC_Im2		0xeb
#define	op_CPX_Ab		0xec
#define	op_SBC_Ab		0xed
#define	op_INC_Ab		0xee
#define	op_ISB_Ab		0xef
#define	op_BEQ			0xf0
#define	op_SBC_IndY		0xf1
#define	op_HALTf2		0xf2
#define	op_ISB_IndY		0xf3
#define	op_NOPf4		0xf4
#define	op_SBC_ZPX		0xf5
#define	op_INC_ZPX		0xf6
#define	op_ISB_ZPX		0xf7
#define	op_SED			0xf8
#define	op_SBC_AbY		0xf9
#define	op_NOPfa		0xfa
#define	op_ISB_AbY		0xfb
#define	op_NOPfc		0xfc
#define	op_SBC_AbX		0xfd
#define	op_INC_AbX		0xfe
#define	op_ISB_AbX		0xff

#endif	/* OPS6502_H */
