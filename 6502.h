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


/*
 * InitCPU sets the CPU up with the correct PC/flags.
 */

extern	void	InitialiseCPU();

/*
 * Function to request an interrupt
 */

extern	void	IRQ();

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

extern	unsigned char				CarryFlag;
extern	unsigned char				ZeroFlag;
extern	unsigned char				IRQDisableFlag;
extern	unsigned char				DecimalModeFlag;
extern	unsigned char				OverflowFlag;
extern	unsigned char				NegativeFlag;

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
