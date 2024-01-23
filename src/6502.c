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


	Disassemble2 ( "%4x : ", GetProgramCounter );

	opcode = *EmulatorPC++;

#ifdef	COUNT_INSTRS

	InstrCount [ opcode ]++;

#endif

			switch ( opcode )
			{
				case BRK :
					Disassemble1 ( "BRK\n" );
					EmulatorPC++;
					StackPC();
					StackSR();
					SetIRQDisableFlag;
					SetProgramCounter ( IRQAddress );

					/*
					 * POSSIBLE ENHANCEMENT ?
					 *
					 * Too much work will be done here -- after all
					 * there's no possibility of causing and IRQ
					 * because we've just disabled them
					 */

					AddClockCycles ( 7 );
					break;

				case ORA_IndX :
				{
					register byteval		ZeroPageAddress;
					register unsigned int	OperandAddress;

					Disassemble2 ( "ORA (&%2x,X)\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef	RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					Ora ( ReadByte ( OperandAddress ));
					AddClockCycles ( 6 );
					break;
				}
				case ORA_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "ORA &%x\n", ZeroPageAddress );
					Ora ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 3 );
					break;
				}
				case ASL_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "ASL &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					ShiftLeft ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 5 );
					break;
				}
				case PHP :
					Disassemble1 ( "PHP\n" );
					StackSR();
					AddClockCycles ( 3 );
					break;

				case ORA_Im :
					Disassemble2 ( "ORA #&%x\n", *EmulatorPC );
					Ora ( *EmulatorPC++);
					AddClockCycles ( 2 );
					break;

				case ASL_A :
					Disassemble1 ( "ASL A\n" );
					ShiftLeft ( Accumulator );
					AddClockCycles ( 2 );
					break;

				case ORA_Ab :
				{
					register unsigned int	OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "ORA &%x\n", OperandAddress );
					Ora ( ReadByte ( OperandAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case ASL_Ab :
				{
					register byteval		temp1;
					register unsigned int	Address;

					Address = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "ASL &%x\n", Address );
					temp1 = ReadByte ( Address );
					ShiftLeft ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case BPL :
				{
					byteval				BranchCycleCount;

					Disassemble2 ( "BPL #&%x\n", *EmulatorPC );
					DoBranch ( !NegativeFlag );
					AddClockCycles ( BranchCycleCount );
					break;
				}
				case ORA_IndY :
				{
					register byteval		ZeroPageAddress;
					register unsigned int	BaseAddress;
					register unsigned int   Address;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "ORA (&%2x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					BaseAddress = ReadWord ( ZeroPageAddress );
					Address = BaseAddress + RegisterY;
					Ora ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 5 : 6);
					break;
				}
				case ORA_ZPX :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "ORA &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					Ora ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case ASL_ZPX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;
					Disassemble2 ( "ASL &%x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					ShiftLeft ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case CLC :
					Disassemble1 ( "CLC\n" );
					CarryFlag = 0;
					AddClockCycles ( 2 );
					break;

				case ORA_AbY :
				{
					register unsigned int	BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterY;
					Disassemble2 ( "ORA &%4x,Y\n", BaseAddress );
					Ora ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case ORA_AbX :
				{
					register unsigned int	BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterX;
					Disassemble2 ( "ORA &%4x,X\n", BaseAddress );
					Ora ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case ASL_AbX :
				{
					register byteval		temp1;
					register unsigned int   Address;

					Disassemble2 ( "ASL &%x,X\n", ReadWordAtPC());
					Address = ReadWordAtPC() + RegisterX;
					EmulatorPC += 2;
					temp1 = ReadByte ( Address );
					ShiftLeft ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 7 );
					break;
				}
				case JSR :
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
					Disassemble2 ( "JSR &%x\n", Address );
					SetProgramCounter ( Address );
					AddClockCycles ( 6 );
					break;

				}
				case AND_IndX :
				{
					register byteval		ZeroPageAddress;
					register unsigned int	OperandAddress;

					Disassemble2 ( "AND (&%2x,X)\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					And ( ReadByte ( OperandAddress ));
					AddClockCycles ( 6 );
					break;
				}
				case BIT_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "BIT &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Bit ( temp1 );
					AddClockCycles ( 3 );
					break;
				}
				case AND_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "AND &%x\n", ZeroPageAddress );
					And ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 3 );
					break;
				}
				case ROL_ZP :
				{
					register byteval		temp1;
					register byteval		temp2;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "ROL &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					RotateLeft ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 5 );
					break;
				}
				case PLP :
					Disassemble1 ( "PLP\n" );
					UnstackSR();
					AddClockCycles ( 4 );
					break;

				case AND_Im :
					Disassemble2 ( "AND #&%x\n", *EmulatorPC );
					And ( *EmulatorPC++);
					AddClockCycles ( 2 );
					break;

				case ROL_A :
				{
					register byteval		temp2;

					Disassemble1 ( "ROL A\n" );
					RotateLeft ( Accumulator );
					AddClockCycles ( 2 );
					break;
				}
				case BIT_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "BIT &%x\n", OperandAddress );
					temp1 = ReadByte ( OperandAddress );
					Bit ( temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case AND_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "AND &%x\n", OperandAddress );
					And ( ReadByte ( OperandAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case ROL_Ab :
				{
					register byteval		temp1;
					register byteval		temp2;
					register unsigned int   Address;

					Address = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "ROL &%x\n", Address );
					temp1 = ReadByte ( Address );
					RotateLeft ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case BMI :
				{
					byteval				BranchCycleCount;

					Disassemble2 ( "BMI #&%x\n", *EmulatorPC );
					DoBranch ( NegativeFlag );
					AddClockCycles ( BranchCycleCount );
					break;
				}
				case AND_IndY :
				{
					register byteval		ZeroPageAddress;
					register unsigned int	BaseAddress;
					register unsigned int   Address;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "AND (&%2x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					BaseAddress = ReadWord ( ZeroPageAddress );
					Address = BaseAddress + RegisterY;
					And ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 5 : 6);
					break;
				}
				case AND_ZPX :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "AND &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					And ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case ROL_ZPX :
				{
					register byteval		temp1;
					register byteval		temp2;
					register byteval		ZeroPageAddress;

					Disassemble2 ( "ROL &%x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					RotateLeft ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case SEC :
					Disassemble1 ( "SEC\n" );
					SetCarryFlag;
					AddClockCycles ( 2 );
					break;

				case AND_AbY :
				{
					register unsigned int	BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "AND &%4x,Y\n", BaseAddress );
					Address = BaseAddress + RegisterY;
					And ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case AND_AbX :
				{
					register unsigned int	BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterX;
					Disassemble2 ( "AND &%4x,X\n", BaseAddress );
					And ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case ROL_AbX :
				{
					register byteval		temp1;
					register byteval		temp2;
					register unsigned int   Address;

					Disassemble2 ( "ROL &%x,X\n", ReadWordAtPC());
					Address = ReadWordAtPC() + RegisterX;
					EmulatorPC += 2;
					temp1 = ReadByte ( Address );
					RotateLeft ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 7 );
					break;
				}
				case RTI :
					Disassemble1 ( "RTI\n" );
					UnstackSR();
					UnstackPC();
					AddClockCycles ( 6 );
					break;

				case EOR_IndX :
				{
					register byteval		ZeroPageAddress;
					register unsigned int   OperandAddress;

					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					Disassemble2 ( "EOR (&%2x,X)\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					Eor ( ReadByte ( OperandAddress ));
					AddClockCycles ( 6 );
					break;
				}
				case EOR_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "EOR &%x\n", ZeroPageAddress );
					Eor ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 3 );
					break;
				}
				case LSR_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "LSR &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					ShiftRight ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 5 );
					break;
				}
				case PHA :
					Disassemble1 ( "PHA\n" );
					StackByte ( Accumulator );
					AddClockCycles ( 3 );
					break;

				case EOR_Im :
					Disassemble2 ( "EOR #&%x\n", *EmulatorPC );
					Eor ( *EmulatorPC++);
					AddClockCycles ( 2 );
					break;

				case LSR_A :
					Disassemble1 ( "LSR A\n" );
					ShiftRight ( Accumulator );
					AddClockCycles ( 2 );
					break;

				case JMP_Ab :
					SetProgramCounter ( ReadWordAtPC());
					Disassemble2 ( "JMP &%x\n", GetProgramCounter );
					AddClockCycles ( 3 );
					break;

				case EOR_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "EOR &%x\n", OperandAddress );
					Eor ( ReadByte ( OperandAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case LSR_Ab :
				{
					register byteval		temp1;
					register unsigned int   Address;

					Address = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "LSR &%x\n", Address );
					temp1 = ReadByte ( Address );
					ShiftRight ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case BVC :
				{
				byteval	           BranchCycleCount;

					Disassemble2 ( "BVC #&%x\n", *EmulatorPC );
					DoBranch ( !OverflowFlag );
					AddClockCycles ( BranchCycleCount );
					break;

				}
				case EOR_IndY :
				{
					register byteval		ZeroPageAddress;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "EOR (&%2x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					BaseAddress = ReadWord ( ZeroPageAddress );
					Address = BaseAddress + RegisterY;
					Eor ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 5 : 6);
					break;
				}
				case EOR_ZPX :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "EOR &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					Eor ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case LSR_ZPX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					Disassemble2 ( "LSR &%x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					ShiftRight ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case CLI :
					Disassemble1 ( "CLI\n" );
					IRQDisableFlag = 0;
					AddClockCycles ( 2 );
					break;

				case EOR_AbY :
				{
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterY;
					Disassemble2 ( "EOR &%4x,Y\n", BaseAddress );
					Eor ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case EOR_AbX :
				{
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterX;
					Disassemble2 ( "EOR &%4x,X\n", BaseAddress );
					Eor ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case LSR_AbX :
				{
					register byteval		temp1;
					register unsigned int   Address;

					Disassemble2 ( "LSR &%x,X\n", ReadWordAtPC());
					Address = ReadWordAtPC() + RegisterX;
					EmulatorPC += 2;
					temp1 = ReadByte ( Address );
					ShiftRight ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 7 );
					break;
				}
				case RTS :
					Disassemble1 ( "RTS\n" );
					/*
					 * When the CPU executes a JSR, the value in the PC pushed
					 * onto the stack is the return address less one, so we have
					 * to correct for that.
					 */

					UnstackPC();
					EmulatorPC++;
					AddClockCycles ( 6 );
					break;

				case ADC_IndX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;
					register unsigned int   OperandAddress;

					Disassemble2 ( "ADC (&%2x,X)\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle ADC (0xff,X)\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					temp1 = ReadByte ( OperandAddress );
					Adc ( temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case ADC_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "ADC &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Adc ( temp1 );
					AddClockCycles ( 3 );
					break;
				}
				case ROR_ZP :
				{
					register byteval		temp1;
					register byteval		temp2;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "ROR &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					RotateRight ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 5 );
					break;
				}
				case PLA :
					Disassemble1 ( "PLA\n" );
					Accumulator = UnstackByte();
					ResetZeroFlag ( Accumulator == 0 );
					NegativeFlag = Accumulator & 0x80;
					AddClockCycles ( 4 );
					break;

				case ADC_Im :
				{
					register byteval		temp1;

					Disassemble2 ( "ADC #&%x\n", *EmulatorPC );
					temp1 = *EmulatorPC++;
					Adc ( temp1 );
					AddClockCycles ( 2 );
					break;
				}
				case ROR_A :
				{
					register byteval		temp2;

					Disassemble1 ( "ROR A\n" );
					RotateRight ( Accumulator );
					AddClockCycles ( 2 );
					break;
				}
				case JMP_Ind :
				{
					register unsigned int   OperandAddress;
					register unsigned int   Address;

					OperandAddress = ReadWordAtPC();
					Disassemble2 ( "JMP (&%x)\n", OperandAddress );

					/*
					 * On a page boundary, the address is read from the last
					 * and first bytes of the same page (yes, it's a 6502
					 * bug).
					 */

					if (( OperandAddress & 0xff ) == 0xff )
					{
						Address = ReadByte ( OperandAddress );
						Address += 256 * ReadByte (( OperandAddress & 0xff00 ));
					}
					else
						Address = ReadWord ( OperandAddress );
					SetProgramCounter ( Address );
					AddClockCycles ( 5 );
					break;
				}
				case ADC_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "ADC &%x\n", OperandAddress );
					temp1 = ReadByte ( OperandAddress );
					Adc ( temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case ROR_Ab :
				{
					register byteval		temp1;
					register byteval		temp2;
					register unsigned int   Address;

					Address = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "ROR &%x\n", Address );
					temp1 = ReadByte ( Address );
					RotateRight ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case BVS :
				{
					byteval	           BranchCycleCount;

					Disassemble2 ( "BVS #&%x\n", *EmulatorPC );
					DoBranch ( OverflowFlag );
					AddClockCycles ( BranchCycleCount );
					break;
				}
				case ADC_IndY :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "ADC (&%2x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					BaseAddress = ReadWord ( ZeroPageAddress );
					Address = BaseAddress + RegisterY;
					temp1 = ReadByte ( Address );
					Adc ( temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 5 : 6);
					break;
				}
				case ADC_ZPX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					Disassemble2 ( "ADC &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Adc ( temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case ROR_ZPX :
				{
					register byteval		temp1;
					register byteval		temp2;
					register byteval		ZeroPageAddress;

					Disassemble2 ( "ROR &%x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					RotateRight ( temp1 );
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case SEI :
					Disassemble1 ( "SEI\n" );
					SetIRQDisableFlag;
					AddClockCycles ( 2 );
					break;

				case ADC_AbY :
				{
					register byteval		temp1;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterY;
					Disassemble2 ( "ADC &%4x,Y\n", BaseAddress );
					temp1 = ReadByte ( Address );
					Adc ( temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case ADC_AbX :
				{
					register byteval		temp1;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterX;
					Disassemble2 ( "ADC &%4x,X\n", BaseAddress );
					temp1 = ReadByte ( Address );
					Adc ( temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case ROR_AbX :
				{
					register byteval		temp1;
					register byteval		temp2;
					register unsigned int   Address;

					Disassemble2 ( "ROR &%x,X\n", ReadWordAtPC());
					Address = ReadWordAtPC() + RegisterX;
					EmulatorPC += 2;
					temp1 = ReadByte ( Address );
					RotateRight ( temp1 );
					WriteByte ( Address, temp1 );
					AddClockCycles ( 7 );
					break;
				}
				case STA_IndX :
				{
					register byteval		ZeroPageAddress;
					register unsigned int   OperandAddress;

					Disassemble2 ( "STA (&%2x,X)\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					WriteByte ( OperandAddress, Accumulator );
					CheckIRQ();
					AddClockCycles ( 6 );
					break;
				}
				case STY_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "STY &%2x\n", ZeroPageAddress );
					WriteLoPageByte ( ZeroPageAddress, RegisterY );
					AddClockCycles ( 3 );
					break;
				}
				case STA_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "STA &%x\n", ZeroPageAddress );
					WriteLoPageByte ( ZeroPageAddress, Accumulator );
					AddClockCycles ( 3 );
					break;
				}
				case STX_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "STX &%x\n", ZeroPageAddress );
					WriteLoPageByte ( ZeroPageAddress, RegisterX );
					AddClockCycles ( 3 );
					break;
				}
				case DEY :
					Disassemble1 ( "DEY\n" );
					RegisterY--;
					ResetZeroFlag ( RegisterY == 0 );
					NegativeFlag = RegisterY & 0x80;
					AddClockCycles ( 2 );
					break;
				case TXA :
					Disassemble1 ( "TXA\n" );
					Accumulator = RegisterX;
					ResetZeroFlag ( Accumulator == 0 );
					NegativeFlag = Accumulator & 0x80;
					AddClockCycles ( 2 );
					break;

				case STY_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "STY &%x\n", OperandAddress );
					WriteByte ( OperandAddress, RegisterY );
					CheckIRQ();
					AddClockCycles ( 4 );
					break;
				}
				case STA_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "STA &%x\n", OperandAddress );
					WriteByte ( OperandAddress, Accumulator );
					CheckIRQ();
					AddClockCycles ( 4 );
					break;
				}
				case STX_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "STX &%x\n", OperandAddress );
					WriteByte ( OperandAddress, RegisterX );
					CheckIRQ();
					AddClockCycles ( 4 );
					break;
				}
				case BCC :
				{
					byteval	           BranchCycleCount;

					Disassemble2 ( "BCC #&%x\n", *EmulatorPC );
					DoBranch ( !CarryFlag );
					AddClockCycles ( BranchCycleCount );
					break;
				}
				case STA_IndY :
				{
					register byteval		ZeroPageAddress;
					register unsigned int   OperandAddress;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "STA (&%x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress ) + RegisterY;
					WriteByte ( OperandAddress, Accumulator );
					CheckIRQ();
					AddClockCycles ( 6 );
					break;
				}
				case STY_ZPX :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "STY &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					WriteLoPageByte ( ZeroPageAddress, RegisterY );
					AddClockCycles ( 4 );
					break;
				}
				case STA_ZPX :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "STA &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
					WriteByte ( ZeroPageAddress, Accumulator );
					AddClockCycles ( 4 );
					break;
				}
				case STX_ZPY :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "STX &%2x,Y\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterY;
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle ZP,Y > 0xff\n" );
						RegDisp;
						FatalError();
					}
#endif
					WriteLoPageByte ( ZeroPageAddress, RegisterX );
					AddClockCycles ( 4 );
					break;
				}
				case TYA :
					Disassemble1 ( "TYA\n" );
					Accumulator = RegisterY;
					ResetZeroFlag ( Accumulator == 0 );
					NegativeFlag = Accumulator & 0x80;
					AddClockCycles ( 2 );
					break;

				case STA_AbY :
				{
					register unsigned int   OperandAddress;

					Disassemble2 ( "STA &%4x,Y\n", ReadWordAtPC());
					OperandAddress = ReadWordAtPC() + RegisterY;
					EmulatorPC += 2;
					WriteByte ( OperandAddress, Accumulator );
					CheckIRQ();
					AddClockCycles ( 5 );
					break;
				}
				case TXS :
					Disassemble1 ( "TXS\n" );
					StackPointer = RegisterX;
					break;

				case STA_AbX :
				{
					register unsigned int   OperandAddress;

					Disassemble2 ( "STA &%4x,X\n", ReadWordAtPC());
					OperandAddress = ReadWordAtPC() + RegisterX;
					EmulatorPC += 2;
					WriteByte ( OperandAddress, Accumulator );
					CheckIRQ();
					AddClockCycles ( 5 );
					break;
				}
				case LDY_Im :
					Disassemble2 ( "LDY #&%x\n", *EmulatorPC );
					LoadY ( *EmulatorPC++);
					AddClockCycles ( 2 );
					break;

				case LDA_IndX :
				{
					register byteval		ZeroPageAddress;
					register unsigned int   OperandAddress;

					Disassemble2 ( "LDA (&%2x,X)\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					LoadA ( ReadByte ( OperandAddress ));
					AddClockCycles ( 6 );
					break;
				}
				case LDX_Im :
					Disassemble2 ( "LDX #%x\n", *EmulatorPC );
					LoadX ( *EmulatorPC++);
					AddClockCycles ( 2 );
					break;

				case LDY_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "LDY &%x\n", ZeroPageAddress );
					LoadY ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 3 );
					break;
				}
				case LDA_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "LDA &%x\n", ZeroPageAddress );
					LoadA ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 3 );
					break;
				}
				case LDX_ZP :
				{
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "LDX &%x\n", ZeroPageAddress );
					LoadX ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 3 );
					break;
				}
				case TAY :
					Disassemble1 ( "TAY\n" );
					RegisterY = Accumulator;
					ResetZeroFlag ( RegisterY == 0 );
					NegativeFlag = RegisterY & 0x80;
					AddClockCycles ( 2 );
					break;

				case LDA_Im :
					Disassemble2 ( "LDA #%x\n", *EmulatorPC );
					LoadA ( *EmulatorPC++);
					AddClockCycles ( 2 );
					break;

				case TAX :
					Disassemble1 ( "TAX\n" );
					RegisterX = Accumulator;
					ResetZeroFlag ( RegisterX == 0 );
					NegativeFlag = RegisterX & 0x80;
					AddClockCycles ( 2 );
					break;

				case LDY_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "LDY &%x\n", OperandAddress );
					LoadY ( ReadByte ( OperandAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case LDA_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "LDA &%x\n", OperandAddress );
					LoadA ( ReadByte ( OperandAddress ));
					AddClockCycles ( 4 );
					break;

				}
				case LDX_Ab :
				{
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "LDX &%x\n", OperandAddress );
					LoadX ( ReadByte ( OperandAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case BCS :
				{
					byteval	           BranchCycleCount;
					Disassemble2 ( "BCS #&%x\n", *EmulatorPC );
					DoBranch ( CarryFlag );
					AddClockCycles ( BranchCycleCount );
					break;
				}
				case LDA_IndY :
				{
					register byteval		ZeroPageAddress;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "LDA (&%2x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					BaseAddress = ReadWord ( ZeroPageAddress );
					Address = BaseAddress + RegisterY;
					LoadA ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 5 : 6);
					break;
				}
				case LDY_ZPX :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "LDY &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					LoadY ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case LDA_ZPX :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "LDA &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					LoadA ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case LDX_ZPY :
				{
					register byteval		ZeroPageAddress;

					Disassemble2 ( "LDX &%2x,Y\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterY;
					if ( ZeroPageAddress > 0xff )
					{
						fprintf ( stderr, "Can't handle ZP,Y > 0xff\n" );
						RegDisp;
						FatalError();
					}
					LoadX ( ReadLoPageByte ( ZeroPageAddress ));
					AddClockCycles ( 4 );
					break;
				}
				case CLV :
					Disassemble1 ( "CLV\n" );
					OverflowFlag = 0;
					AddClockCycles ( 2 );
					break;

				case LDA_AbY :
				{
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "LDA &%4x,Y\n", BaseAddress );
					Address = BaseAddress + RegisterY;
					LoadA ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case TSX :
					Disassemble1 ( "TSX\n" );
					RegisterX = StackPointer;
					ResetZeroFlag ( RegisterX == 0 );
					NegativeFlag = RegisterX & 0x80;
					AddClockCycles ( 2 );
					break;

				case LDY_AbX :
				{
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterX;
					Disassemble2 ( "LDY &%4x,X\n", BaseAddress );
					LoadY ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case LDA_AbX :
				{
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "LDA &%4x,X\n", BaseAddress );
					Address = BaseAddress + RegisterX;
					LoadA ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case LDX_AbY :
				{
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterY;
					Disassemble2 ( "LDX &%4x,Y\n", BaseAddress );
					LoadX ( ReadByte ( Address ));
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case CPY_Im :
				{
					register byteval		temp1;

					Disassemble2 ( "CPY #&%x\n", *EmulatorPC );
					temp1 = *EmulatorPC++;
					Cmp ( RegisterY, temp1 );
					AddClockCycles ( 2 );
					break;
				}
				case CMP_IndX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;
					register unsigned int   OperandAddress;

					Disassemble2 ( "CMP (&%2x,X)\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle CMP (0xff,X)\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					temp1 = ReadByte ( OperandAddress );
					Cmp ( Accumulator, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case CPY_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "CPY &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Cmp ( RegisterY, temp1 );
					AddClockCycles ( 3 );
					break;
				}
				case CMP_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "CMP &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Cmp ( Accumulator, temp1 );
					AddClockCycles ( 3 );
					break;
				}
				case DEC_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "DEC &%2x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					temp1--;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 5 );
					break;
				}
				case INY :
					Disassemble1 ( "INY\n" );
					RegisterY++;
					ResetZeroFlag ( RegisterY == 0 );
					NegativeFlag = RegisterY & 0x80;
					AddClockCycles ( 2 );
					break;

				case CMP_Im :
				{
					register byteval		temp1;

					Disassemble2 ( "CMP #&%x\n", *EmulatorPC );
					temp1 = *EmulatorPC++;
					Cmp ( Accumulator, temp1 );
					AddClockCycles ( 2 );
					break;
				}
				case DEX :
					Disassemble1 ( "DEX\n" );
					RegisterX--;
					ResetZeroFlag ( RegisterX == 0 );
					NegativeFlag = RegisterX & 0x80;
					AddClockCycles ( 2 );
					break;

				case CPY_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "CPY &%x\n", OperandAddress );
					temp1 = ReadByte ( OperandAddress );
					Cmp ( RegisterY, temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case CMP_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "CMP &%x\n", OperandAddress );
					temp1 = ReadByte ( OperandAddress );
					Cmp ( Accumulator, temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case DEC_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "DEC &%4x\n", OperandAddress );
					temp1 = ReadByte ( OperandAddress );
					temp1--;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteByte ( OperandAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case BNE :
				{
					byteval	           BranchCycleCount;

					Disassemble2 ( "BNE #&%x\n", *EmulatorPC );
					DoBranch ( !ZeroFlag );
					AddClockCycles ( BranchCycleCount );
					break;
				}
				case CMP_IndY :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "CMP (&%2x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle CMP (0xff),Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					BaseAddress = ReadWord ( ZeroPageAddress );
					Address = BaseAddress + RegisterY;
					temp1 = ReadByte ( Address );
					Cmp ( Accumulator, temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 5 : 6);
					break;
				}
				case CMP_ZPX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					Disassemble2 ( "CMP &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Cmp ( Accumulator, temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case DEC_ZPX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					Disassemble2 ( "DEC &%2x,X\n", *EmulatorPC );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					temp1--;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case CLD :
					Disassemble1 ( "CLD\n" );
					DecimalModeFlag = 0;
					AddClockCycles ( 2 );
					break;

				case CMP_AbY :
				{
					register byteval		temp1;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "CMP &%4x,Y\n",  BaseAddress );
					Address = BaseAddress + RegisterY;
					temp1 = ReadByte ( Address );
					Cmp ( Accumulator, temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case CMP_AbX :
				{
					register byteval		temp1;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "CMP &%4x,X\n", BaseAddress );
					Address = BaseAddress + RegisterX;
					temp1 = ReadByte ( Address );
					Cmp ( Accumulator, temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case DEC_AbX :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC() + RegisterX;
					EmulatorPC += 2;
					Disassemble2 ( "DEC &%4x,X\n", ReadWordAtPC());
					temp1 = ReadByte ( OperandAddress );
					temp1--;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteByte ( OperandAddress, temp1 );
					AddClockCycles ( 7 );
					break;
				}
				case CPX_Im :
				{
					register byteval		temp1;
					Disassemble2 ( "CPX #&%x\n", *EmulatorPC );
					temp1 = *EmulatorPC++;
					Cmp ( RegisterX, temp1 );
					AddClockCycles ( 2 );
					break;
				}
				case SBC_IndX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;
					register unsigned int   OperandAddress;

					Disassemble2 ( "SBC (&%2x,X)\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
						RegDisp;
						FatalError();
					}
#endif
					OperandAddress = ReadWord ( ZeroPageAddress );
					temp1 = ReadByte ( OperandAddress );
					Sbc ( temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case CPX_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "CPX &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Cmp ( RegisterX, temp1 );
					AddClockCycles ( 3 );
					break;
				}
				case SBC_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "SBC &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Sbc ( temp1 );
					AddClockCycles ( 3 );
					break;
				}
				case INC_ZP :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					ZeroPageAddress = *EmulatorPC++;
					Disassemble2 ( "INC &%x\n", ZeroPageAddress );
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					temp1++;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 5 );
					break;
				}
				case INX :
					Disassemble1 ( "INX\n" );
					RegisterX++;
					ResetZeroFlag ( RegisterX == 0 );
					NegativeFlag = RegisterX & 0x80;
					AddClockCycles ( 2 );
					break;

				case SBC_Im :
				{
					register byteval		temp1;

					Disassemble2 ( "SBC #&%x\n", *EmulatorPC );
					temp1 = *EmulatorPC++;
					Sbc ( temp1 );
					AddClockCycles ( 2 );
					break;
				}
				case NOP :
					Disassemble1 ( "NOP\n" );
					AddClockCycles ( 2 );
					break;

				case CPX_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "CPX &%x\n", OperandAddress );
					temp1 = ReadByte ( OperandAddress );
					Cmp ( RegisterX, temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case SBC_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "SBC &%x\n", OperandAddress );
					temp1 = ReadByte ( OperandAddress );
					Sbc ( temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case INC_Ab :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					OperandAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Disassemble2 ( "INC &%x\n", OperandAddress );
					temp1 = ReadByte( OperandAddress );
					temp1++;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteByte( OperandAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case BEQ :
				{
					byteval	           BranchCycleCount;

					Disassemble2 ( "BEQ #&%x\n", *EmulatorPC );
					DoBranch ( ZeroFlag );
					AddClockCycles ( BranchCycleCount );
					break;
				}
				case SBC_IndY :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					ZeroPageAddress = ( *EmulatorPC++);
					Disassemble2 ( "SBC (&%2x),Y\n", ZeroPageAddress );
#ifdef  RANGE_CHECK
					if ( ZeroPageAddress == 0xff )
					{
						fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
						RegDisp;
						FatalError();
					}
#endif
					BaseAddress = ReadWord ( ZeroPageAddress );
					Address = BaseAddress + RegisterY;
					temp1 = ReadByte ( Address );
					Sbc ( temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 5 : 6);
					break;
				}
				case SBC_ZPX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					Disassemble2 ( "SBC &%2x,X\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					Sbc ( temp1 );
					AddClockCycles ( 4 );
					break;
				}
				case INC_ZPX :
				{
					register byteval		temp1;
					register byteval		ZeroPageAddress;

					Disassemble2 ( "INC &%x,Y\n", *EmulatorPC );
					ZeroPageAddress = *EmulatorPC++ + RegisterX;
					temp1 = ReadLoPageByte ( ZeroPageAddress );
					temp1++;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteLoPageByte ( ZeroPageAddress, temp1 );
					AddClockCycles ( 6 );
					break;
				}
				case SED :
					Disassemble1 ( "SED\n" );
					SetDecimalModeFlag;
					AddClockCycles ( 2 );
					break;

				case SBC_AbY :
				{
					register byteval		temp1;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterY;
					Disassemble2 ( "SBC &%4x,Y\n", BaseAddress );
					temp1 = ReadByte ( Address );
					Sbc ( temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case SBC_AbX :
				{
					register byteval		temp1;
					register unsigned int   BaseAddress;
					register unsigned int   Address;

					BaseAddress = ReadWordAtPC();
					EmulatorPC += 2;
					Address = BaseAddress + RegisterX;
					Disassemble2 ( "SBC &%4x,X\n", BaseAddress );
					temp1 = ReadByte ( Address );
					Sbc ( temp1 );
					AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
					break;
				}
				case INC_AbX :
				{
					register byteval		temp1;
					register unsigned int   OperandAddress;

					Disassemble2 ( "INC &%x,Y\n", ReadWordAtPC());
					OperandAddress = ReadWordAtPC() + RegisterX;
					EmulatorPC += 2;
					temp1 = ReadByte( OperandAddress );
					temp1++;
					ResetZeroFlag ( temp1 == 0 );
					NegativeFlag = temp1 & 0x80;
					WriteByte( OperandAddress, temp1 );
					AddClockCycles ( 7 );
					break;
				}
#ifdef	EMUL_FS
				case EFS_OSFILE :
				{
					Disassemble1 ( "EFS_OSFILE\n\tRTS\n" );
					Accumulator = Emulate_OSFILE ( Accumulator, RegisterX, RegisterY );
					UnstackPC();
					EmulatorPC++;
					break;
				}
				case EFS_OSFSC :
				{
					unsigned int	ret;
					Disassemble1 ( "EFS_OSFSC\n\tRTS\n" );
					ret = Emulate_OSFSC ( Accumulator, RegisterX, RegisterY, &NewPC );
					Accumulator = ret & 0xff;
					RegisterX = ( ret >> 8 ) & 0xff;
					RegisterY = ( ret >> 16 ) & 0xff;
					UnstackPC();
					if ( NewPC >= 0 )
						SetProgramCounter ( NewPC );
					else
						EmulatorPC++;
					break;
				}
#endif
				default :
					Disassemble1 ( "???\n" );
					fprintf ( stderr, "Illegal opcode -- 0x%x\n", *( EmulatorPC - 1 ));
					RegDisp;
					FatalError();
					break;
			}
