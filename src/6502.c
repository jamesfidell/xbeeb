/*
 *
 * $Id: 6502.c,v 1.23 2002/01/15 15:46:43 james Exp $
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
 * $Log: 6502.c,v $
 * Revision 1.23  2002/01/15 15:46:43  james
 * *** empty log message ***
 *
 * Revision 1.22  2000/08/16 17:58:25  james
 * Update copyright message
 *
 * Revision 1.21  1996/11/15 23:04:49  james
 * Misc. tidying up of code.
 *
 * Revision 1.20  1996/11/15 08:54:50  james
 * Corrected instruction display for RRA
 *
 * Revision 1.19  1996/11/15 08:34:00  james
 * Correction to flag-handling for 65C12 BIT #imm operation (from David Ralph
 * Stacey).
 *
 * Revision 1.18  1996/11/15 00:00:43  james
 * Forced address wrap-around for (Ind),Y  Abs,X and Abs,X instructions.
 * Some games rely on this wrap-around to write into zero page.
 *
 * Revision 1.17  1996/10/10 23:06:02  james
 * Opcodes 82, C2 & E2 are NOPs, not HALTs.
 *
 * Revision 1.16  1996/10/08 23:05:22  james
 * Corrections to allow clean compilation under GCC 2.7.2 with -Wall -pedantic
 *
 * Revision 1.15  1996/09/24 23:05:32  james
 * Update copyright dates.
 *
 * Revision 1.14  1996/09/24 22:40:14  james
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
 * Revision 1.13  1996/09/24 17:48:32  james
 * In EFS_OSFSC, UnstackPC should only be called if the OSFSC call doesn't
 * result in a jump to code elsewhere.  Where there is no jump we will now
 * go back to the calling routine and where there is a jump we will return
 * to the address pushed onto the stack at the appropriate time.
 *
 * Revision 1.12  1996/09/22 21:00:53  james
 * Changed IRQ-handling code to be called only when I think an IRQ may have
 * happened.  Converted it all to #defines so that it can be compiled inline.
 *
 * Revision 1.11  1996/09/22 19:23:20  james
 * Add the emulated filing system code.
 *
 * Revision 1.10  1996/09/22 18:15:26  james
 * Corrected levels of indentation.
 *
 * Revision 1.9  1996/09/21 23:21:46  james
 * Improvements to branch-handling code.
 *
 * Revision 1.8  1996/09/21 23:07:34  james
 * Call FatalError() rather than exit() so that screen stuff etc. can
 * be cleaned up.
 *
 * Revision 1.7  1996/09/21 22:56:50  james
 * Add some commentary to BRK code.
 *
 * Revision 1.6  1996/09/21 22:54:48  james
 * Fix code for displaying registers (because the variables are no longer
 * globals)
 *
 * Revision 1.5  1996/09/21 22:48:20  james
 * Add instruction counting code.
 *
 * Revision 1.4  1996/09/21 22:39:52  james
 * Improved handling of instruction disassembly.
 *
 * Revision 1.3  1996/09/21 22:13:45  james
 * Replaced "unsigned char" representation of 1 byte with "byteval".
 *
 * Revision 1.2  1996/09/21 18:15:27  james
 * Corrections to number of parentheses in DEBUG code.
 *
 * Revision 1.1  1996/09/21 17:20:34  james
 * Source files moved to src directory.
 *
 * Revision 1.1.1.1  1996/09/21 13:52:48  james
 * Xbeeb v0.1 initial release
 *
 *
 */


		Disassemble2 ( "%04lx : ", GetProgramCounter );

		opcode = *EmulatorPC++;

#ifdef	COUNT_INSTRS

		InstrCount [ opcode ]++;

#endif	/* COUNT_INSTRS */

		switch ( opcode )
		{
			case op_BRK :
				Disassemble1 ( "BRK\n" );
				EmulatorPC++;
				StackPC();
				StackSR();
				SetIRQDisableFlag;
#ifndef	M6502
				DecimalModeFlag = 0x0;
#endif
				SetProgramCounter ( IRQAddress );
				AddClockCycles ( CLK_INTERRUPT );
				break;

			case op_ORA_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int	OperandAddress;

				Disassemble2 ( "ORA (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;

				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );
				Ora ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
#ifdef	M6502
#ifdef	HALT02
			case op_HALT02 :
				Disassemble1 ( "HALT02\n" );
				Halt();
				break;
#endif	/* HALT02 */
#endif	/* M6502 */
#ifdef	M6502
			case op_SLO_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int	OperandAddress;
				register byteval		temp;

				Disassemble2 ( "SLO (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp = Accumulator;
				Slo ( temp, ReadByte ( OperandAddress ));
				WriteByte ( OperandAddress, temp );
				AddClockCycles ( CLK_IDXIND_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP04 :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "NOP &%02x\n", ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * Probably, this next line doesn't need to happen.
				 */

				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_TSB_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "TSB &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Tsb ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_ORA_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "ORA &%02x\n", ZeroPageAddress );
				Ora ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_ASL_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "ASL &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftLeft ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SLO_ZP :
			{
				register byteval		ZeroPageAddress, temp;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SLO &%02x\n", ZeroPageAddress );
				temp = Accumulator;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Slo ( temp, ReadLoPageByte ( ZeroPageAddress ));
				WriteLoPageByte ( ZeroPageAddress, temp );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB0 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB0 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0xfe;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_PHP :
				Disassemble1 ( "PHP\n" );
				StackSR();
				AddClockCycles ( CLK_STACK_PUSH );
				break;

			case op_ORA_Im :
				Disassemble2 ( "ORA #&%02x\n", *EmulatorPC );
				Ora ( *EmulatorPC++);
				AddClockCycles ( CLK_IMMEDIATE );
				break;

			case op_ASL_A :
				Disassemble1 ( "ASL A\n" );
				ShiftLeft ( Accumulator );
				AddClockCycles ( CLK_IMPLIED );
				break;

#ifdef	M6502
			case op_ANC0b :
			{
				Disassemble2 ( "ANC #&%02x\n", *EmulatorPC );
				And ( *EmulatorPC++);
				ShiftLeft ( Accumulator );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP0c :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "NOP &%04x\n", OperandAddress );
				/*
				 * Theoretically, this isn't necessary, but it might
				 * be that the read resets and IRQ flag in a VIA
				 * because of this read...
				 */
				( void ) ReadByte ( OperandAddress );
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_TSB_Ab :
			{
				register byteval		temp1;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "TSB &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Tsb ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */

			case op_ORA_Ab :
			{
				register unsigned int	OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ORA &%04x\n", OperandAddress );
				Ora ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_ASL_Ab :
			{
				register byteval		temp1;
				register unsigned int	Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ASL &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftLeft ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SLO_Ab :
			{
				register byteval		temp1;
				register unsigned int	Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SLO &%04x\n", Address );
				temp1 = Accumulator;
				Slo ( temp1, ReadByte ( Address ));
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR0 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR0 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x01 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BPL :
			{
				byteval				BranchCycleCount;

				Disassemble2 ( "BPL #&%02x\n", *EmulatorPC );
				DoBranch ( !NegativeFlag );
				AddClockCycles ( BranchCycleCount );
				break;
			}
			case op_ORA_IndY :
			{
				register byteval		ZeroPageAddress;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "ORA (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				Ora ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#ifdef	M6502
			case op_HALT12 :
				Disassemble1 ( "HALT12\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_ORA_Ind :
			{
				register byteval		ZeroPageAddress;
				register unsigned int	Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "ORA (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					Address = ReadByte ( ZeroPageAddress );
					Address += 256 * ReadByte ( 0x00 );
				}
				else
					Address = ReadWord ( ZeroPageAddress );

				Ora ( ReadByte ( Address ));
				AddClockCycles ( CLK_IND_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_SLO_IndY :
			{
				register byteval		ZeroPageAddress, temp1;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "SLO (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = Accumulator;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Slo ( temp1, ReadByte ( Address ));
				AddClockCycles ( CLK_INDIDX_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP14 :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "NOP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This read may not be required.
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_TRB_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "TRB &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Trb ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_ORA_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "ORA &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				Ora ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_ASL_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "ASL &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftLeft ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SLO_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "SLO &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = Accumulator;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Slo ( temp1, ReadLoPageByte ( ZeroPageAddress ));
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB1 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB1 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0xfd;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_CLC :
				Disassemble1 ( "CLC\n" );
				CarryFlag = 0;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_ORA_AbY :
			{
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ORA &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				Ora ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#ifdef	M6502
			case op_NOP1a :
				Disassemble1 ( "NOP\n" );
				AddClockCycles ( CLK_IMPLIED );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_INC_A :
				Disassemble1 ( "INC A\n" );
				Accumulator++;
				ResetZeroFlag ( Accumulator == 0 );
				NegativeFlag = Accumulator & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_SLO_AbY :
			{
				register byteval		temp1;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SLO &%04x,Y\n", BaseAddress );
				temp1 = Accumulator;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Slo ( temp1, ReadByte ( Address ));
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP1c :
			{
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "NOP &%04x,X\n", BaseAddress );
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = BaseAddress + RegisterX;
				/*
				 * This read may not be required, but it's possible
				 * that reading the value could have some affect if
				 * it's from a hardware register.
				 */
				( void ) ReadByte ( Address );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_TRB_Ab :
			{
				register byteval		temp1;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "TRB &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Trb ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_ORA_AbX :
			{
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ORA &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				Ora ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_ASL_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ASL &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftLeft ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SLO_AbX :
			{
				register byteval		temp1;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SLO &%04x,X\n", BaseAddress );
				temp1 = Accumulator;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Slo ( temp1, ReadByte ( Address ));
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR1 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR1 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x02 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_JSR :
			{
				register unsigned int   Address;

				/*
				 * When the CPU does a JSR, it puts the address of the
				 * low byte of the called address on the stack, not the
				 * return address itself.
				 */

				Address = ReadWordAtPC();
				EmulatorPC++;
				StackPC();
				Disassemble2 ( "JSR &%04x\n", Address );
				SetProgramCounter ( Address );
				AddClockCycles ( CLK_JSR );
				break;

			}
			case op_AND_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int	OperandAddress;

				Disassemble2 ( "AND (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				And ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
#ifdef	M6502
#ifdef	HALT22
			case op_HALT22 :
				Disassemble1 ( "HALT22\n" );
				Halt();
				break;
#endif	/* HALT22 */
#endif	/* M6502 */
#ifdef	M6502
			case op_RLA_IndX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;
				register unsigned int	OperandAddress;

				Disassemble2 ( "RLA (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteLoPageByte ( OperandAddress, temp1 );
				And ( temp1 );
				AddClockCycles ( CLK_IDXIND_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
			case op_BIT_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "BIT &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Bit ( temp1 );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_AND_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "AND &%02x\n", ZeroPageAddress );
				And ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_ROL_ZP :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "ROL &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RLA_ZP :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RLA &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				And ( temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB2 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB2 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0xfb;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_PLP :
				Disassemble1 ( "PLP\n" );
				UnstackSR();
				AddClockCycles ( CLK_STACK_PULL );
				break;

			case op_AND_Im :
				Disassemble2 ( "AND #&%02x\n", *EmulatorPC );
				And ( *EmulatorPC++);
				AddClockCycles ( CLK_IMMEDIATE );
				break;

			case op_ROL_A :
			{
				register byteval		temp2;

				Disassemble1 ( "ROL A\n" );
				RotateLeft ( Accumulator );
				AddClockCycles ( CLK_IMPLIED );
				break;
			}
#ifdef	M6502
			case op_ANC2b :
			{
				Disassemble2 ( "ANC #&%02x\n", *EmulatorPC );
				And ( *EmulatorPC++);
				ShiftLeft ( Accumulator );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
#endif	/* M6502 */
			case op_BIT_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				AddClockCycles ( 3 );
				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "BIT &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );
				Bit ( temp1 );
				AddClockCycles ( CLK_ABS_READ - 3);
				break;
			}
			case op_AND_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "AND &%04x\n", OperandAddress );
				And ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_ROL_Ab :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ROL &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RLA_Ab :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "RLA &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteByte ( Address, temp1 );
				And ( temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR2 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR2 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x04 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BMI :
			{
				byteval				BranchCycleCount;

				Disassemble2 ( "BMI #&%02x\n", *EmulatorPC );
				DoBranch ( NegativeFlag );
				AddClockCycles ( BranchCycleCount );
				break;
			}
			case op_AND_IndY :
			{
				register byteval		ZeroPageAddress;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "AND (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				And ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#ifdef	M6502
			case op_HALT32 :
				Disassemble1 ( "HALT32\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_AND_Ind :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "AND (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					Address = ReadByte ( ZeroPageAddress );
					Address += 256 * ReadByte ( 0x00 );
				}
				else
					Address = ReadWord ( ZeroPageAddress );

				And ( ReadByte ( Address ));
				AddClockCycles ( CLK_IND_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_RLA_IndY :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "RLA (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteByte ( Address, temp1 );
				And ( temp1 );
				AddClockCycles ( CLK_INDIDX_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP34 :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "NOP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This may not be required.
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BIT_ZPX :
			{
				register byteval		ZeroPageAddress;
				register byteval		temp1;

				Disassemble2 ( "BIT &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Bit ( temp1 );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_AND_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "AND &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				And ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_ROL_ZPX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "ROL &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RLA_ZPX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "RLA &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				And ( temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB3 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB3 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0xf7;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_SEC :
				Disassemble1 ( "SEC\n" );
				SetCarryFlag;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_AND_AbY :
			{
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "AND &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				And ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#ifdef	M6502
			case op_NOP3a :
				Disassemble1 ( "NOP\n" );
				AddClockCycles ( CLK_IMPLIED );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_DEC_A :
				Disassemble1 ( "DEC A\n" );
				Accumulator--;
				ResetZeroFlag ( Accumulator == 0 );
				NegativeFlag = Accumulator & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_RLA_AbY :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "RLA &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteByte ( Address, temp1 );
				And ( temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP3c :
			{
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "NOP &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = BaseAddress + RegisterX;

				/*
				 * This is required in case the read resets something
				 * in the hardware implementation.
				 */

				( void ) ReadByte ( Address );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BIT_AbX :
			{
				register unsigned int	BaseAddress;
				register unsigned int   Address;
				register byteval		temp1;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "BIT &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );
				Bit ( temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_AND_AbX :
			{
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "AND &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				And ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_ROL_AbX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ROL &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RLA_AbX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "RLA &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateLeft ( temp1 );
				WriteByte ( Address, temp1 );
				And ( temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR3 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR3 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x08 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_RTI :
				Disassemble1 ( "RTI\n" );
				UnstackSR();
				UnstackPC();
				AddClockCycles ( CLK_RTI );
				break;

			case op_EOR_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "EOR (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				Eor ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
#ifdef	M6502
			case op_HALT42 :
				Disassemble1 ( "HALT42\n" );
				Halt();
				break;
#endif	/* M6502 */
#ifdef	M6502
			case op_SRE_IndX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int	OperandAddress;

				Disassemble2 ( "SRE (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteLoPageByte ( OperandAddress, temp1 );
				Eor ( temp1 );
				AddClockCycles ( CLK_IDXIND_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP44 :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "NOP &%02x\n", ZeroPageAddress );
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This probably isn't necessary...
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
#endif	/* M6502 */
			case op_EOR_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "EOR &%02x\n", ZeroPageAddress );
				Eor ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_LSR_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "LSR &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SRE_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SRE &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address.
				 */

				ShiftRight ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				Eor ( temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB4 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB4 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0xef;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_PHA :
				Disassemble1 ( "PHA\n" );
				StackByte ( Accumulator );
				AddClockCycles ( CLK_STACK_PUSH );
				break;

			case op_EOR_Im :
				Disassemble2 ( "EOR #&%02x\n", *EmulatorPC );
				Eor ( *EmulatorPC++);
				AddClockCycles ( CLK_IMMEDIATE );
				break;

			case op_LSR_A :
				Disassemble1 ( "LSR A\n" );
				ShiftRight ( Accumulator );
				AddClockCycles ( CLK_IMPLIED );
				break;

#ifdef	M6502
			case op_ASR_Im :
				Disassemble2 ( "EOR #&%02x\n", *EmulatorPC );
				Accumulator &= *EmulatorPC++;
				ShiftRight ( Accumulator );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#endif	/* M6502 */

			case op_JMP_Ab :
				SetProgramCounter ( ReadWordAtPC());
				Disassemble2 ( "JMP &%04x\n", GetProgramCounter );
				AddClockCycles ( CLK_ABS_JMP );
				break;

			case op_EOR_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "EOR &%04x\n", OperandAddress );
				Eor ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_LSR_Ab :
			{
				register byteval		temp1;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LSR &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SRE_Ab :
			{
				register byteval		temp1;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SRE &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteByte ( Address, temp1 );
				Eor ( temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR4 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR4 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x10 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BVC :
			{
			byteval	           BranchCycleCount;

				Disassemble2 ( "BVC #&%02x\n", *EmulatorPC );
				DoBranch ( !OverflowFlag );
				AddClockCycles ( BranchCycleCount );
				break;

			}
			case op_EOR_IndY :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "EOR (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				Eor ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#ifdef	M6502
			case op_HALT52 :
				Disassemble1 ( "HALT52\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_EOR_Ind :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "EOR (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					Address = ReadByte ( ZeroPageAddress );
					Address += 256 * ReadByte ( 0x00 );
				}
				else
					Address = ReadWord ( ZeroPageAddress );

				Eor ( ReadByte ( Address ));
				AddClockCycles ( CLK_IND_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_SRE_IndY :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "SRE (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteByte ( Address, temp1 );
				Eor ( temp1 );
				AddClockCycles ( CLK_INDIDX_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP54 :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "NOP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This probably isn't necessary.
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#endif	/* M6502 */
			case op_EOR_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "EOR &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				Eor ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_LSR_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "LSR &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SRE_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "SRE &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				Eor ( temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB5 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB5 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0xdf;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_CLI :
				Disassemble1 ( "CLI\n" );
				IRQDisableFlag = 0;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_EOR_AbY :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "EOR &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				Eor ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#ifdef	M6502
			case op_NOP5a :
				Disassemble1 ( "NOP\n" );
				AddClockCycles ( CLK_IMPLIED );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_PHY :
				Disassemble1 ( "PHY\n" );
				StackByte ( RegisterY );
				AddClockCycles ( CLK_STACK_PUSH );
				break;
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_SRE_AbY :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SRE &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress  + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteByte ( Address, temp1 );
				Eor ( temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP5c :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "NOP &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = BaseAddress + RegisterX;

				/*
				 * This is required in case it resets something in the
				 * hardware...
				 */

				( void ) ReadByte ( Address );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#endif	/* M6502 */
			case op_EOR_AbX :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "EOR &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				Eor ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_LSR_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LSR &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress  + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_SRE_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SRE &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress  + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				ShiftRight ( temp1 );
				WriteByte ( Address, temp1 );
				Eor ( temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR5 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR5 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x20 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_RTS :
				Disassemble1 ( "RTS\n" );
				/*
				 * When the CPU executes a JSR, the value in the PC pushed
				 * onto the stack is the return address less one, so we
				 * have to correct for that.
				 */

				UnstackPC();
				EmulatorPC++;
				AddClockCycles ( CLK_RTS );
				break;

			case op_ADC_IndX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "ADC (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress );
				Adc ( temp1 );
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
#ifdef	M6502
			case op_HALT62 :
				Disassemble1 ( "HALT62\n" );
				Halt();
				break;
#endif	/* M6502 */
#ifdef	M6502
			case op_RRA_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int	OperandAddress;
				register byteval		temp1;
				register byteval		temp2;

				Disassemble2 ( "RRA (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Rra ( temp1 );
				WriteByte ( OperandAddress, temp1 );
				AddClockCycles ( CLK_IDXIND_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP64 :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "NOP &%02x\n", ZeroPageAddress );
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This probably isn't necessary...
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_STZ_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "STZ &%02x\n", ZeroPageAddress );
				WriteLoPageByte ( ZeroPageAddress, 0x0 );
				AddClockCycles ( CLK_ZP_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_ADC_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "ADC &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Adc ( temp1 );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_ROR_ZP :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "ROR &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateRight ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RRA_ZP :
			{
				register byteval		ZeroPageAddress;
				register byteval		temp1;
				register byteval		temp2;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RRA &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Rra ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB6 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB6 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0xbf;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_PLA :
				Disassemble1 ( "PLA\n" );
				Accumulator = UnstackByte();
				ResetZeroFlag ( Accumulator == 0 );
				NegativeFlag = Accumulator & 0x80;
				AddClockCycles ( CLK_STACK_PULL );
				break;
			case op_ADC_Im :
			{
				register byteval		temp1;

				Disassemble2 ( "ADC #&%02x\n", *EmulatorPC );
				temp1 = *EmulatorPC++;
				Adc ( temp1 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
			case op_ROR_A :
			{
				register byteval		temp2;

				Disassemble1 ( "ROR A\n" );
				RotateRight ( Accumulator );
				AddClockCycles ( CLK_IMPLIED );
				break;
			}
#ifdef	M6502
			case op_ARR :
			{
				Disassemble2 ( "ARR #&%02x\n", *EmulatorPC );
				Accumulator &= ( *EmulatorPC++ );

				if ( DecimalModeFlag )
				{
					register byteval		temp1, temp2;

					NegativeFlag = CarryFlag << 7;
					temp1 = Accumulator & 0x40;
					temp2 = Accumulator;
					Accumulator >>= 1;
					OverflowFlag = temp1 ^ ( Accumulator & 0x40 );

					temp1 = temp2 & 0x1;
					if ((( temp2 + temp1 ) & 0xf ) > 5 )
					{
						Accumulator = ( Accumulator & 0xf0 ) +
							((( Accumulator & 0xf ) + 6 ) & 0xf );
					}

					temp1 = temp2 & 0x10;
					if ((( temp2 + temp1 ) & 0xf0 ) > 0x50 )
					{
						Accumulator = ( Accumulator & 0xf ) +
							((( Accumulator & 0xf0 ) + 0x60 ) & 0xf0 );
						SetCarryFlag;
					}
					else
						CarryFlag = 0;
				}
				else
				{
					Accumulator >>= 1;
					Accumulator |= ( CarryFlag << 7 );
					NegativeFlag = Accumulator & 0x80;
					ResetCarryFlag ( Accumulator & 0x40 );
					ResetOverflowFlag ( CarryFlag ^
									(( Accumulator >> 5 ) & 0x1 ));
				}
				ResetZeroFlag ( Accumulator == 0x0 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
#endif	/* M6502 */
			case op_JMP_Ind :
			{
				register unsigned int   OperandAddress;
				register unsigned int   Address;

				OperandAddress = ReadWordAtPC();
				Disassemble2 ( "JMP (&%04x)\n", OperandAddress );

				/*
				 * On a page boundary, the address is read from the last
				 * and first bytes of the same page (yes, it's a 6502
				 * bug).  This was corrected for the CMOS versions of
				 * the chip.
				 */

#ifdef	M6502
				if (( OperandAddress & 0xff ) == 0xff )
				{
					Address = ReadByte ( OperandAddress );
					Address += 256 * ReadByte (( OperandAddress & 0xff00 ));
				}
				else
#endif	/* M6502 */
				Address = ReadWord ( OperandAddress );
				SetProgramCounter ( Address );
				AddClockCycles ( CLK_ABS_INDIRECT );
				break;
			}
			case op_ADC_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ADC &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );
				Adc ( temp1 );
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_ROR_Ab :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ROR &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateRight ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RRA_Ab :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   Address;

				Address = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "RRA &%04x\n", Address );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Rra ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR6 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR6 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x40 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BVS :
			{
				byteval	           BranchCycleCount;

				Disassemble2 ( "BVS #&%02x\n", *EmulatorPC );
				DoBranch ( OverflowFlag );
				AddClockCycles ( BranchCycleCount );
				break;
			}
			case op_ADC_IndY :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "ADC (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );
				Adc ( temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#ifdef	M6502
			case op_HALT72 :
				Disassemble1 ( "HALT72\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_ADC_Ind :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "ADC (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					Address = ReadByte ( ZeroPageAddress );
					Address += 256 * ReadByte ( 0x00 );
				}
				else
					Address = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( Address );
				Adc ( temp1 );
				AddClockCycles ( CLK_IND_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_RRA_IndY :
			{
				register byteval		ZeroPageAddress;
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "RRA (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY );
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Rra ( temp1 );
				AddClockCycles ( CLK_INDIDX_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP74 :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "NOP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This probably isn't necessary...
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_STZ_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "STZ &%02x\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				WriteLoPageByte ( ZeroPageAddress, 0x0 );
				AddClockCycles ( CLK_ZPI_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_ADC_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "ADC &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Adc ( temp1 );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_ROR_ZPX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "ROR &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateRight ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RRA_ZPX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "RRA &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Rra ( temp1 );
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_RMB7 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "RMB7 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) & 0x7f;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_SEI :
				Disassemble1 ( "SEI\n" );
				SetIRQDisableFlag;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_ADC_AbY :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ADC &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );
				Adc ( temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#ifdef	M6502
			case op_NOP7a :
				Disassemble1 ( "NOP\n" );
				AddClockCycles ( CLK_IMPLIED );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_PLY :
				Disassemble1 ( "PLY\n" );
				RegisterY = UnstackByte();
				ResetZeroFlag ( RegisterY == 0 );
				NegativeFlag = RegisterY & 0x80;
				AddClockCycles ( CLK_STACK_PULL );
				break;
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_RRA_AbY :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "RRA &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Rra ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP7c :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "NOP &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = BaseAddress + RegisterX;

				/*
				 * This is required in case it resets something in the
				 * hardware...
				 */

				( void ) ReadByte ( Address );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_JMP_IndX :
			{
				register unsigned int   BaseAddress;
				register unsigned int   OperandAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				Disassemble2 ( "JMP (&%04x,X)\n", BaseAddress );
				OperandAddress = BaseAddress + RegisterX;
				Address = ReadWord ( OperandAddress );
				SetProgramCounter ( Address );
				AddClockCycles (( BaseAddress & 0xff00 ) ==
					( OperandAddress & 0xff00 ) ? CLK_ABS_INDIRECT :
					CLK_ABS_INDIRECT_PAGECROSS );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_ADC_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ADC &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );
				Adc ( temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_ROR_AbX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ROR &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				RotateRight ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_RRA_AbX :
			{
				register byteval		temp1;
				register byteval		temp2;
				register unsigned int	BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "RRA &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Rra ( temp1 );
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBR7 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBR7 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ( ! (ReadByte ( ZeroPageAddress) & 0x80 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_NOP80 :
				Disassemble2 ( "NOP #&%02x\n", *EmulatorPC );
				EmulatorPC++;
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_BRA :
			{
				unsigned int		old, new;

				Disassemble2 ( "BRA #&%02x\n", *EmulatorPC );
				old = GetProgramCounter & 0xff00;
				EmulatorPC += ( signed char )( *EmulatorPC++ );
				new = GetProgramCounter & 0xff00;
				AddClockCycles ( old == new ? CLK_REL_OK :
													CLK_REL_PAGECROSS );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_STA_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "STA (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				WriteByte ( OperandAddress, Accumulator );
				AddClockCycles ( CLK_IDXIND_WRITE );
				break;
			}
#ifdef	M6502
			case op_NOP82 :
				Disassemble1 ( "NOP &82\n" );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#endif	/* M6502 */
#ifdef	M6502
			case op_SAX_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "SAX (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				WriteByte ( OperandAddress, ( Accumulator & RegisterX ));
				AddClockCycles ( CLK_IDXIND_WRITE );
				break;
			}
#endif	/* M6502 */
			case op_STY_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "STY &%02x\n", ZeroPageAddress );
				WriteLoPageByte ( ZeroPageAddress, RegisterY );
				AddClockCycles ( CLK_ZP_WRITE );
				break;
			}
			case op_STA_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "STA &%02x\n", ZeroPageAddress );
				WriteLoPageByte ( ZeroPageAddress, Accumulator );
				AddClockCycles ( CLK_ZP_WRITE );
				break;
			}
			case op_STX_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "STX &%02x\n", ZeroPageAddress );
				WriteLoPageByte ( ZeroPageAddress, RegisterX );
				AddClockCycles ( CLK_ZP_WRITE );
				break;
			}
#ifdef	M6502
			case op_SAX_ZP :
			{
				register byteval		ZPAddress;

				ZPAddress = *EmulatorPC++;
				Disassemble2 ( "SAX &%02x\n", ZPAddress );
				WriteLoPageByte ( ZPAddress, ( Accumulator & RegisterX ));
				AddClockCycles ( CLK_ZP_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB0 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB0 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x01;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_DEY :
				Disassemble1 ( "DEY\n" );
				RegisterY--;
				ResetZeroFlag ( RegisterY == 0 );
				NegativeFlag = RegisterY & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;
#ifdef	M6502
			case op_NOP89 :
				Disassemble2 ( "NOP #&%02x\n", *EmulatorPC );
				EmulatorPC++;
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_BIT_Im :
			{
				register byteval		temp1;

				Disassemble2 ( "BIT #&%02x\n", *EmulatorPC );
				temp1 = *EmulatorPC++;

				/*
				 * When immediate addressing is used, the V and N flags
				 * are not changed (New AUG, page 40), so we don't use
				 * the normal BIT routine here.
				 */

				ResetZeroFlag (( temp1 & Accumulator ) == 0x0 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_TXA :
				Disassemble1 ( "TXA\n" );
				Accumulator = RegisterX;
				ResetZeroFlag ( Accumulator == 0 );
				NegativeFlag = Accumulator & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;
#ifdef	M6502
			case op_ANE :
				Disassemble2 ( "ANE #&%02x\n", *EmulatorPC );
				LoadA (( Accumulator | 0xee ) & RegisterX & *EmulatorPC++);
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#endif	/* M6502 */
			case op_STY_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "STY &%04x\n", OperandAddress );
				WriteByte ( OperandAddress, RegisterY );
				AddClockCycles ( CLK_ABS_WRITE );
				break;
			}
			case op_STA_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "STA &%04x\n", OperandAddress );
				WriteByte ( OperandAddress, Accumulator );
				AddClockCycles ( CLK_ABS_WRITE );
				break;
			}
			case op_STX_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "STX &%04x\n", OperandAddress );
				WriteByte ( OperandAddress, RegisterX );
				AddClockCycles ( CLK_ABS_WRITE );
				break;
			}
#ifdef	M6502
			case op_SAX_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SAX &%04x\n", OperandAddress );
				WriteByte ( OperandAddress, ( Accumulator & RegisterX ));
				AddClockCycles ( CLK_ABS_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS0 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS0 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x01 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BCC :
			{
				byteval	           BranchCycleCount;

				Disassemble2 ( "BCC #&%02x\n", *EmulatorPC );
				DoBranch ( !CarryFlag );
				AddClockCycles ( BranchCycleCount );
				break;
			}
			case op_STA_IndY :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "STA (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				OperandAddress += RegisterY;
				OperandAddress &= 0xffff;
				WriteByte ( OperandAddress, Accumulator );
				AddClockCycles ( CLK_INDIDX_WRITE );
				break;
			}
#ifdef	M6502
			case op_HALT92 :
				Disassemble1 ( "HALT92\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_STA_Ind :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "STA (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadByte ( ZeroPageAddress );
					OperandAddress += 256 * ReadByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				WriteByte ( OperandAddress, Accumulator );
				AddClockCycles ( CLK_IND_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_SHA_IndY :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * There are things going on here that need to be
				 * sorted out -- like the potential read when fixing
				 * up the high byte of the effective address, for
				 * example.
				 */

				Disassemble2 ( "SHA (&%02x),Y", *EmulatorPC );
				ZeroPageAddress = *EmulatorPC++;
				OperandAddress = ReadWord ( ZeroPageAddress );
				temp1 = Accumulator & RegisterX;
				temp1 &= ((( OperandAddress >> 8 ) + 1 ) & 0xff );
				if ((( OperandAddress & 0xff ) + RegisterY ) > 0xff )
					OperandAddress = ( OperandAddress & 0xff ) +
													( temp1 << 8 );
				WriteByte (( OperandAddress + RegisterY ) & 0xffff, temp1 );
				AddClockCycles ( CLK_INDIDX_WRITE );
				break;
			}
#endif	/* M6502 */
			case op_STY_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "STY &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				WriteLoPageByte ( ZeroPageAddress, RegisterY );
				AddClockCycles ( CLK_ZPI_WRITE );
				break;
			}
			case op_STA_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "STA &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				WriteByte ( ZeroPageAddress, Accumulator );
				AddClockCycles ( CLK_ZPI_WRITE );
				break;
			}
			case op_STX_ZPY :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "STX &%02x,Y\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterY ) & 0xff;
				WriteLoPageByte ( ZeroPageAddress, RegisterX );
				AddClockCycles ( CLK_ZPI_WRITE );
				break;
			}
#ifdef	M6502
			case op_SAX_ZPY :
			{
				register byteval		ZPAddress;

				Disassemble2 ( "SAX &%02x,Y\n", *EmulatorPC );
				ZPAddress = ( *EmulatorPC++ + RegisterY ) & 0xff;
				WriteLoPageByte ( ZPAddress, ( Accumulator & RegisterX ));
				AddClockCycles ( CLK_ZPI_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB1 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB1 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x02;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_TYA :
				Disassemble1 ( "TYA\n" );
				Accumulator = RegisterY;
				ResetZeroFlag ( Accumulator == 0 );
				NegativeFlag = Accumulator & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_STA_AbY :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "STA &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				WriteByte ( Address, Accumulator );
				AddClockCycles ( CLK_ABI_WRITE );
				break;
			}
			case op_TXS :
				Disassemble1 ( "TXS\n" );
				StackPointer = RegisterX;
				break;
#ifdef	M6502
			case op_SHS :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * Something needs sorting out here...  There's a
				 * possible intermediate read of some address that
				 * needs considering...
				 */

				Disassemble2 ( "SHS (&%02x),Y", *EmulatorPC );
				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				temp1 = Accumulator & RegisterX;
				StackPointer = temp1;
				temp1 &= ((( OperandAddress >> 8 ) + 1 ) & 0xff );
				if ((( OperandAddress & 0xff ) + RegisterY ) > 0xff )
					OperandAddress = ( OperandAddress & 0xff ) +
													( temp1 << 8 );
				WriteByte ( OperandAddress, temp1 );
				AddClockCycles ( CLK_ABI_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_SHY :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * Something needs sorting out here...  There's a
				 * possible intermediate read of some address that
				 * needs considering...
				 */

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SHY &%04x,X", OperandAddress );
				temp1 = RegisterY;
				temp1 &= ((( OperandAddress >> 8 ) + 1 ) & 0xff );
				if ((( OperandAddress & 0xff ) + RegisterX ) > 0xff )
					OperandAddress = ( OperandAddress & 0xff ) +
													( temp1 << 8 );
				WriteByte ( OperandAddress + RegisterX, temp1 );
				AddClockCycles ( CLK_ABI_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_STZ_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "STZ &%04x\n", OperandAddress );
				WriteByte ( OperandAddress, 0x0 );
				AddClockCycles ( CLK_ABS_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_STA_AbX :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "STA &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				WriteByte ( Address, Accumulator );
				AddClockCycles ( CLK_ABI_WRITE );
				break;
			}
#ifdef	M6502
			case op_SHX :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * Something needs sorting out here...  There's a
				 * possible intermediate read of some address that
				 * needs considering...
				 */

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SHX &%04x,Y", OperandAddress );
				temp1 = RegisterX;
				temp1 &= ((( OperandAddress >> 8 ) + 1 ) & 0xff );
				if ((( OperandAddress & 0xff ) + RegisterY ) > 0xff )
					OperandAddress = ( OperandAddress & 0xff ) +
													( temp1 << 8 );
				WriteByte ( OperandAddress + RegisterY, temp1 );
				AddClockCycles ( CLK_ABI_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_STZ_AbX :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "STZ &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				WriteByte ( Address, 0x0 );
				AddClockCycles ( CLK_ABI_WRITE );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_SHA_AbY :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * Something needs sorting out here...  There's a
				 * possible intermediate read of some address that
				 * needs considering...
				 */

				OperandAddress = ReadWordAtPC();
				Disassemble2 ( "SHA &%04x,Y", OperandAddress );
				EmulatorPC += 2;

				/*
				 * FIX ME
				 *
				 * Does this one need to wrap-around ?
				 */

				temp1 = Accumulator & RegisterX;
				temp1 &= ((( OperandAddress >> 8 ) + 1 ) & 0xff );
				if ((( OperandAddress & 0xff ) + RegisterY ) > 0xff )
					OperandAddress = ( OperandAddress & 0xff ) +
													( temp1 << 8 );
				WriteByte (( OperandAddress + RegisterY ) & 0xffff, temp1 );
				AddClockCycles ( CLK_ABI_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS1 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS1 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x02 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */

			case op_LDY_Im :
				Disassemble2 ( "LDY #&%02x\n", *EmulatorPC );
				LoadY ( *EmulatorPC++);
				AddClockCycles ( CLK_IMMEDIATE );
				break;

			case op_LDA_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "LDA (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				LoadA ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
			case op_LDX_Im :
				Disassemble2 ( "LDX #&%02x\n", *EmulatorPC );
				LoadX ( *EmulatorPC++);
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#ifdef	M6502
			case op_LAX_IndX :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "LAX (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				LoadA ( ReadByte ( OperandAddress ));
				RegisterX = Accumulator;
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
#endif	/* M6502 */
			case op_LDY_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "LDY &%02x\n", ZeroPageAddress );
				LoadY ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_LDA_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "LDA &%02x\n", ZeroPageAddress );
				LoadA ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_LDX_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "LDX &%02x\n", ZeroPageAddress );
				LoadX ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
#ifdef	M6502
			case op_LAX_ZP :
			{
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "LAX &%02x\n", ZeroPageAddress );
				LoadA ( ReadLoPageByte ( ZeroPageAddress ));
				RegisterX = Accumulator;
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB2 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB2 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x04;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_TAY :
				Disassemble1 ( "TAY\n" );
				RegisterY = Accumulator;
				ResetZeroFlag ( RegisterY == 0 );
				NegativeFlag = RegisterY & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_LDA_Im :
				Disassemble2 ( "LDA #&%02x\n", *EmulatorPC );
				LoadA ( *EmulatorPC++ );
				AddClockCycles ( CLK_IMMEDIATE );
				break;

			case op_TAX :
				Disassemble1 ( "TAX\n" );
				RegisterX = Accumulator;
				ResetZeroFlag ( RegisterX == 0 );
				NegativeFlag = RegisterX & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;
#ifdef	M6502
			/*
			 * POSSIBLE ENHANCEMENT ?
			 *
			 * Apparently the results of this instruction are somewhat
			 * random...
			 */
			case op_LXA :
				Disassemble2 ( "LXA #&%02x\n", *EmulatorPC );
				And ( *EmulatorPC++ ) ;
				RegisterX = Accumulator;
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#endif	/* M6502 */
			case op_LDY_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LDY &%04x\n", OperandAddress );
				LoadY ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_LDA_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LDA &%04x\n", OperandAddress );
				LoadA ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_ABS_READ );
				break;

			}
			case op_LDX_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LDX &%04x\n", OperandAddress );
				LoadX ( ReadByte ( OperandAddress ));
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
#ifdef	M6502
			case op_LAX_Ab :
			{
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LAX &%04x\n", OperandAddress );
				LoadA ( ReadByte ( OperandAddress ));
				RegisterX = Accumulator;
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS2 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS2 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x04 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BCS :
			{
				byteval	           BranchCycleCount;
				Disassemble2 ( "BCS #&%02x\n", *EmulatorPC );
				DoBranch ( CarryFlag );
				AddClockCycles ( BranchCycleCount );
				break;
			}
			case op_LDA_IndY :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "LDA (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				LoadA ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#ifdef	M6502
			case op_HALTb2 :
				Disassemble1 ( "HALTb2\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_LDA_Ind :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "LDA (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					Address = ReadByte ( ZeroPageAddress );
					Address += 256 * ReadByte ( 0x00 );
				}
				else
					Address = ReadWord ( ZeroPageAddress );

				LoadA ( ReadByte ( Address ));
				AddClockCycles ( CLK_IND_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_LAX_IndY :
			{
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "LAX (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				LoadA ( ReadByte ( Address ));
				RegisterX = Accumulator;
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#endif	/* M6502 */
			case op_LDY_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "LDY &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				LoadY ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_LDA_ZPX :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "LDA &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				LoadA ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_LDX_ZPY :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "LDX &%02x,Y\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterY ) & 0xff;
				LoadX ( ReadLoPageByte ( ZeroPageAddress ));
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#ifdef	M6502
			case op_LAX_ZPY :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "LAX &%02x,Y\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterY ) & 0xff;
				LoadA ( ReadLoPageByte ( ZeroPageAddress ));
				RegisterX = Accumulator;
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB3 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB3 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x08;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_CLV :
				Disassemble1 ( "CLV\n" );
				OverflowFlag = 0;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_LDA_AbY :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LDA &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				LoadA ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_TSX :
				Disassemble1 ( "TSX\n" );
				RegisterX = StackPointer;
				ResetZeroFlag ( RegisterX == 0 );
				NegativeFlag = RegisterX & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;
#ifdef	M6502
			case op_LAS :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LAS &%04x,Y\n",  BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = BaseAddress + RegisterY;
				And ( ReadByte ( Address ));
				RegisterX = StackPointer = Accumulator;
				AddClockCycles ( CLK_ABI_READ );
				break;
			}
#endif	/* M6502 */
			case op_LDY_AbX :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LDY &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				LoadY ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_LDA_AbX :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LDA &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				LoadA ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_LDX_AbY :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LDX &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				LoadX ( ReadByte ( Address ));
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#ifdef	M6502
			case op_LAX_AbY :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "LAX &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				LoadA ( ReadByte ( Address ));
				RegisterX = Accumulator;
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS3 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS3 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x08 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_CPY_Im :
			{
				register byteval		temp1;

				Disassemble2 ( "CPY #&%02x\n", *EmulatorPC );
				temp1 = *EmulatorPC++;
				Cmp ( RegisterY, temp1 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
			case op_CMP_IndX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "CMP (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
#ifdef	M6502
			case op_NOPc2 :
				Disassemble1 ( "NOP &c2\n" );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#endif	/* M6502 */
#ifdef	M6502
			case op_DCP_IndX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "DCP (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress ) - 1;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				WriteByte ( OperandAddress, temp1 );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_IDXIND_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
			case op_CPY_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "CPY &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Cmp ( RegisterY, temp1 );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_CMP_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "CMP &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_DEC_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "DEC &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1--;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_DCP_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "DCP &%02x\n", ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1 = ReadLoPageByte ( ZeroPageAddress ) - 1;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB4 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB4 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x10;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_INY :
				Disassemble1 ( "INY\n" );
				RegisterY++;
				ResetZeroFlag ( RegisterY == 0 );
				NegativeFlag = RegisterY & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_CMP_Im :
			{
				register byteval		temp1;

				Disassemble2 ( "CMP #&%02x\n", *EmulatorPC );
				temp1 = *EmulatorPC++;
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
			case op_DEX :
				Disassemble1 ( "DEX\n" );
				RegisterX--;
				ResetZeroFlag ( RegisterX == 0 );
				NegativeFlag = RegisterX & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;
#ifdef	M6502
			case op_SBX :
			{
				register int		temp1;

				Disassemble2 ( "SBX #&%02x\n", *EmulatorPC );
				temp1 = ( RegisterX & Accumulator ) - *EmulatorPC;
				RegisterX = temp1 & 0xff;
				ResetCarryFlag ( temp1 >= 0x0 );
				NegativeFlag = RegisterX & 0x80;
				ResetZeroFlag ( RegisterX == 0x0 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
#endif	/* M6502 */
			case op_CPY_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "CPY &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );
				Cmp ( RegisterY, temp1 );
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_CMP_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "CMP &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_DEC_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "DEC &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1--;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteByte ( OperandAddress, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_DCP_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "DCP &%04x\n", OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1 = ReadByte ( OperandAddress ) - 1;
				WriteByte ( OperandAddress, temp1 );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS4 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS4 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x10 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BNE :
			{
				byteval	           BranchCycleCount;

				Disassemble2 ( "BNE #&%02x\n", *EmulatorPC );
				DoBranch ( !ZeroFlag );
				AddClockCycles ( BranchCycleCount );
				break;
			}
			case op_CMP_IndY :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "CMP (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );
				Cmp ( Accumulator, temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#ifdef	M6502
			case op_HALTd2 :
				Disassemble1 ( "HALTd2\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_CMP_Ind :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "CMP (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					Address = ReadByte ( ZeroPageAddress );
					Address += 256 * ReadByte ( 0x00 );
				}
				else
					Address = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( Address );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_IND_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_DCP_IndY :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "DCP (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address ) - 1;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				WriteByte ( Address, temp1 );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_INDIDX_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOPd4 :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "NOP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This read may not be required.
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#endif	/* M6502 */
			case op_CMP_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "CMP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_DEC_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				Disassemble2 ( "DEC &%02x,X\n", *EmulatorPC );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1--;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_DCP_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "DCP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress ) - 1;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				WriteLoPageByte ( ZeroPageAddress, temp1 );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB5 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB5 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x20;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_CLD :
				Disassemble1 ( "CLD\n" );
				DecimalModeFlag = 0;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_CMP_AbY :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "CMP &%04x,Y\n",  BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );
				Cmp ( Accumulator, temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#ifdef	M6502
			case op_NOPda :
				Disassemble1 ( "NOP\n" );
				AddClockCycles ( CLK_IMPLIED );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_PHX :
				Disassemble1 ( "PHX\n" );
				StackByte ( RegisterX );
				AddClockCycles ( CLK_STACK_PUSH );
				break;
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_DCP_AbY :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "DCP &%04x,Y\n",  BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address ) - 1;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				WriteByte ( Address, temp1 );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
#ifdef	M6502
			case op_NOPdc :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "NOP &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = BaseAddress + RegisterX;
				( void ) ReadByte ( Address );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#endif	/* M6502 */
			case op_CMP_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "CMP &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );
				Cmp ( Accumulator, temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_DEC_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "DEC &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1--;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteByte ( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_DCP_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "DCP &%04x,X\n",  BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address ) - 1;

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				WriteByte ( Address, temp1 );
				Cmp ( Accumulator, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS5 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS5 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x20 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_CPX_Im :
			{
				register byteval		temp1;
				Disassemble2 ( "CPX #&%02x\n", *EmulatorPC );
				temp1 = *EmulatorPC++;
				Cmp ( RegisterX, temp1 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
			case op_SBC_IndX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "SBC (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress );
				Sbc ( temp1 );
				AddClockCycles ( CLK_IDXIND_READ );
				break;
			}
#ifdef	M6502
			case op_NOPe2 :
				Disassemble1 ( "NOP &e2\n" );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
#endif	/* M6502 */
#ifdef	M6502
			case op_ISB_IndX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   OperandAddress;

				Disassemble2 ( "ISB (&%02x,X)\n", *EmulatorPC );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * A read from ZP should take place here, whilst
				 * ZP+X is being calculated.
				 */

				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				if ( ZeroPageAddress == 0xff )
				{
					OperandAddress = ReadLoPageByte ( 0xff );
					OperandAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					OperandAddress = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				WriteByte ( OperandAddress, temp1 );
				Sbc ( temp1 );
				AddClockCycles ( CLK_IDXIND_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */
			case op_CPX_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "CPX &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Cmp ( RegisterX, temp1 );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_SBC_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SBC &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				Sbc ( temp1 );
				AddClockCycles ( CLK_ZP_READ );
				break;
			}
			case op_INC_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "INC &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_ISB_ZP :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "ISB &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				Sbc ( temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB6 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB6 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x40;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_INX :
				Disassemble1 ( "INX\n" );
				RegisterX++;
				ResetZeroFlag ( RegisterX == 0 );
				NegativeFlag = RegisterX & 0x80;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_SBC_Im :
			{
				register byteval		temp1;

				Disassemble2 ( "SBC #&%02x\n", *EmulatorPC );
				temp1 = *EmulatorPC++;
				Sbc ( temp1 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
			case op_NOP :
				Disassemble1 ( "NOP\n" );
				AddClockCycles ( CLK_IMPLIED );
				break;
#ifdef	M6502
			case op_SBC_Im2 :
			{
				register int		temp1;

				Disassemble2 ( "SBC2 #&%02x\n", *EmulatorPC );
				temp1 = Accumulator - *EmulatorPC;
				Accumulator = temp1 & 0xff;
				ResetCarryFlag ( temp1 >= 0x0 );
				NegativeFlag = Accumulator & 0x80;
				ResetZeroFlag ( Accumulator == 0x0 );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}
#endif	/* M6502 */
			case op_CPX_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "CPX &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );
				Cmp ( RegisterX, temp1 );
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_SBC_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SBC &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );
				Sbc ( temp1 );
				AddClockCycles ( CLK_ABS_READ );
				break;
			}
			case op_INC_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "INC &%04x\n", OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1 = ReadByte( OperandAddress );
				temp1++;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteByte( OperandAddress, temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_ISB_Ab :
			{
				register byteval		temp1;
				register unsigned int   OperandAddress;

				OperandAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ISB &%04x\n", OperandAddress );
				temp1 = ReadByte ( OperandAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				WriteByte ( OperandAddress, temp1 );
				Sbc ( temp1 );
				AddClockCycles ( CLK_ABS_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS6 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS6 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x40 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
			case op_BEQ :
			{
				byteval	           BranchCycleCount;

				Disassemble2 ( "BEQ #&%02x\n", *EmulatorPC );
				DoBranch ( ZeroFlag );
				AddClockCycles ( BranchCycleCount );
				break;
			}
			case op_SBC_IndY :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "SBC (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );
				Sbc ( temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00) ? CLK_INDIDX_READ : CLK_INDIDX_READ_PAGECROSS);
				break;
			}
#ifdef	M6502
			case op_HALTf2 :
				Disassemble1 ( "HALTf2\n" );
				Halt();
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_SBC_Ind :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "SBC (&%02x)\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					Address = ReadByte ( ZeroPageAddress );
					Address += 256 * ReadByte ( 0x00 );
				}
				else
					Address = ReadWord ( ZeroPageAddress );

				temp1 = ReadByte ( Address );
				Sbc ( temp1 );
				AddClockCycles ( CLK_IND_READ );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_ISB_IndY :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				ZeroPageAddress = ( *EmulatorPC++);
				Disassemble2 ( "ISB (&%02x),Y\n", ZeroPageAddress );
				if ( ZeroPageAddress == 0xff )
				{
					BaseAddress = ReadLoPageByte ( 0xff );
					BaseAddress += 256 * ReadLoPageByte ( 0x00 );
				}
				else
					BaseAddress = ReadWord ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT
				 *
				 * If adding Y causes a page boundary to be crossed,
				 * there will be an additional read here whilst the
				 * high byte of the effective address is fixed up.
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				WriteByte ( Address, temp1 );
				Sbc ( temp1 );
				AddClockCycles ( CLK_INDIDX_READ_MOD_WRITE );
				break;
			}
#endif
#ifdef	M6502
			case op_NOPf4 :
			{
				register byteval		ZeroPageAddress;

				Disassemble2 ( "NOP &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * This probably isn't required...
				 */
				( void ) ReadLoPageByte ( ZeroPageAddress );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
#endif	/* M6502 */
			case op_SBC_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "SBC &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );
				Sbc ( temp1 );
				AddClockCycles ( CLK_ZPI_READ );
				break;
			}
			case op_INC_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "INC &%02x,Y\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_ISB_ZPX :
			{
				register byteval		temp1;
				register byteval		ZeroPageAddress;

				Disassemble2 ( "ISB &%02x,X\n", *EmulatorPC );
				ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
				temp1 = ReadLoPageByte ( ZeroPageAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				Sbc ( temp1 );
				AddClockCycles ( CLK_ZPI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C02
			case op_SMB7 :
			{
				register byteval		ZeroPageAddress, temp1;

				ZeroPageAddress = *EmulatorPC++;
				Disassemble2 ( "SMB7 &%02x\n", ZeroPageAddress );
				temp1 = ReadLoPageByte ( ZeroPageAddress ) | 0x80;
				WriteLoPageByte ( ZeroPageAddress, temp1 );
				AddClockCycles ( CLK_ZP_READ_MOD_WRITE );
				break;
			}
#endif	/* R65C02 */
#endif	/* M6502 */
			case op_SED :
				Disassemble1 ( "SED\n" );
				SetDecimalModeFlag;
				AddClockCycles ( CLK_IMPLIED );
				break;

			case op_SBC_AbY :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SBC &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );
				Sbc ( temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#ifdef	M6502
			case op_NOPfa :
				Disassemble1 ( "NOP\n" );
				AddClockCycles ( CLK_IMPLIED );
				break;
#else	/* M6502 */
#ifdef	R65C12
			case op_PLX :
				Disassemble1 ( "PLX\n" );
				RegisterX = UnstackByte();
				ResetZeroFlag ( RegisterX == 0 );
				NegativeFlag = RegisterX & 0x80;
				AddClockCycles ( CLK_STACK_PULL );
				break;
#endif	/* R65C12 */
#endif	/* M6502 */
#ifdef	M6502
			case op_ISB_AbY :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ISB &%04x,Y\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding Y causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + Y ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterY ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				WriteByte ( Address, temp1 );
				Sbc ( temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#endif	/* M6502 */

#ifdef	M6502
			case op_NOPfc :
			{
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "NOP &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = BaseAddress + RegisterX;

				/*
				 * This is required in case the read resets some
				 * hardware.
				 */

				( void ) ReadByte ( Address );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
#endif	/* M6502 */
			case op_SBC_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "SBC &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );
				Sbc ( temp1 );
				AddClockCycles (( BaseAddress & 0xff00 ) == ( Address &
					0xff00 ) ? CLK_ABI_READ : CLK_ABI_READ_PAGECROSS );
				break;
			}
			case op_INC_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "INC &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress  + RegisterX ) & 0xffff;
				temp1 = ReadByte( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				ResetZeroFlag ( temp1 == 0 );
				NegativeFlag = temp1 & 0x80;
				WriteByte( Address, temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#ifdef	M6502
			case op_ISB_AbX :
			{
				register byteval		temp1;
				register unsigned int   BaseAddress;
				register unsigned int   Address;

				BaseAddress = ReadWordAtPC();
				EmulatorPC += 2;
				Disassemble2 ( "ISB &%04x,X\n", BaseAddress );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * If adding X causes a page crossing, whilst the high
				 * byte of the effective address is being fixed up with
				 * the carry, there will be a read of
				 * (( BaseAddress & 0xff00 )+(( BaseAddress + X ) & 0xff ))
				 */

				Address = ( BaseAddress + RegisterX ) & 0xffff;
				temp1 = ReadByte ( Address );

				/*
				 * POSSIBLE ENHANCEMENT ?
				 *
				 * While the operation is being carried out, the original
				 * value is written back to the same address (on the 6502)
				 * or the address is re-read (on 65c[01]2).
				 */

				temp1++;
				WriteByte ( Address, temp1 );
				Sbc ( temp1 );
				AddClockCycles ( CLK_ABI_READ_MOD_WRITE );
				break;
			}
#else	/* M6502 */
#ifdef	R65C12
			case op_BBS7 :
			{
				byteval					BranchCycleCount;
				register byteval		ZeroPageAddress;

				ZeroPageAddress = ( *EmulatorPC++ );
				Disassemble3 ( "BBS7 &%02x, #&0x2x\n", ZeroPageAddress,
															*EmulatorPC );
				BitBranch ((ReadByte ( ZeroPageAddress) & 0x80 ));
				AddClockCycles ( BranchCycleCount );
				break;
			}
#endif	/* R65C12 */
#endif	/* M6502 */


/*
 * All the CMOS NOP intstructions are grouped together here for
 * simplicity's sake.
 */

#ifdef	R65C12

#ifdef	NOP02
			case op_NOP02 :
#endif
#ifdef	NOP22
			case op_NOP22 :
#endif
			case op_NOP42 : case op_NOP62 : case op_NOP82 : case op_NOPc2 :
			case op_NOPe2 :
			case op_NOP03 : case op_NOP13 : case op_NOP23 : case op_NOP33 :
			case op_NOP43 : case op_NOP53 : case op_NOP63 : case op_NOP73 :
			case op_NOP83 : case op_NOP93 : case op_NOPa3 : case op_NOPb3 :
			case op_NOPc3 : case op_NOPd3 : case op_NOPe3 : case op_NOPf3 :
			case op_NOP44 : case op_NOP54 : case op_NOPd4 : case op_NOPf4 :
			case op_NOP5c : case op_NOPdc : case op_NOPfc :
#ifndef	R65C02
			case op_NOP07 : case op_NOP17 : case op_NOP27 : case op_NOP37 :
			case op_NOP47 : case op_NOP57 : case op_NOP67 : case op_NOP77 :
			case op_NOP87 : case op_NOP97 : case op_NOPa7 : case op_NOPb7 :
			case op_NOPc7 : case op_NOPd7 : case op_NOPe7 : case op_NOPf7 :
#endif	/* R65C02 */
			case op_NOP0b : case op_NOP1b : case op_NOP2b : case op_NOP3b :
			case op_NOP4b : case op_NOP5b : case op_NOP6b : case op_NOP7b :
			case op_NOP8b : case op_NOP9b : case op_NOPab : case op_NOPbb :
			case op_NOPcb : case op_NOPdb : case op_NOPeb : case op_NOPfb :
			{
				Disassemble2 ( "NOP #&0x02\n", *EmulatorPC++ );
				AddClockCycles ( CLK_IMMEDIATE );
				break;
			}

#endif	/* R65C12 */

#ifdef	EMUL_FS
			case EFS_OSFSC :
			{
				unsigned int	ret;
				Disassemble1 ( "EFS_OSFSC\n       RTS\n" );
				ret = Emulate_OSFSC ( Accumulator, RegisterX, RegisterY,
																	&NewPC );
				Accumulator = ret & 0xff;
				RegisterX = ( ret >> 8 ) & 0xff;
				RegisterY = ( ret >> 16 ) & 0xff;
				if ( NewPC >= 0 )
					SetProgramCounter ( NewPC );
				else
				{
					UnstackPC();
					EmulatorPC++;
				}
				break;
			}

			case EFS_OSFILE :
			{
				Disassemble1 ( "EFS_OSFILE\n       RTS\n" );
				Accumulator = Emulate_OSFILE ( Accumulator, RegisterX,
																RegisterY );
				UnstackPC();
				EmulatorPC++;
				break;
			}
#endif	/* EMUL_FS */

			default :
				Disassemble1 ( "???\n" );
				fprintf ( stderr, "Illegal opcode -- 0x%02x\n", opcode );
				RegDisp;
				FatalError();
				break;
		}
