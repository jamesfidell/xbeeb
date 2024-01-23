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


#ifdef	DISASS
	printf ( "%4x : ", GetProgramCounter );
#endif

	switch ( *EmulatorPC++ )
	{
		case BRK :
#ifdef  DISASS
			printf ( "BRK\n" );
#endif
			AddClockCycles ( 7 );
			EmulatorPC++;
			StackPC();
			StackSR();
			SetIRQDisableFlag;
			SetProgramCounter ( IRQAddress );
			break;

		case ORA_IndX :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int	OperandAddress;

#ifdef  DISASS
			printf ( "ORA (&%2x,X)\n", *EmulatorPC ));
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef	RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
				DisplayRegisters();
				exit ( 4 );
			}
#endif
			OperandAddress = ReadWord ( ZeroPageAddress );
			Ora ( ReadByte ( OperandAddress ));
			AddClockCycles ( 6 );
			break;
		}
		case ORA_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "ORA &%x\n", ZeroPageAddress );
#endif
			Ora ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 3 );
			break;
		}
		case ASL_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "ASL &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			ShiftLeft ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 5 );
			break;
		}
		case PHP :
#ifdef  DISASS
			printf ( "PHP\n" );
#endif
			StackSR();
			AddClockCycles ( 3 );
			break;

		case ORA_Im :
#ifdef  DISASS
			printf ( "ORA #&%x\n", *EmulatorPC ));
#endif
			Ora ( *EmulatorPC++);
			AddClockCycles ( 2 );
			break;

		case ASL_A :
#ifdef	DISASS
			printf ( "ASL A\n" );
#endif
			ShiftLeft ( Accumulator );
			AddClockCycles ( 2 );
			break;

		case ORA_Ab :
		{
			register unsigned int	OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "ORA &%x\n", OperandAddress );
#endif
			Ora ( ReadByte ( OperandAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case ASL_Ab :
		{
			register unsigned char	temp1;
			register unsigned int	Address;

			Address = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "ASL &%x\n", Address );
#endif
			temp1 = ReadByte ( Address );
			ShiftLeft ( temp1 );
			WriteByte ( Address, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case BPL :
		{
			unsigned char			BranchCycleCount = 2;

#ifdef  DISASS
			printf ( "BPL #&%x\n", *EmulatorPC );
#endif
			DoBranch ( !NegativeFlag );
			AddClockCycles ( BranchCycleCount );
			break;
		}
		case ORA_IndY :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int	BaseAddress;
			register unsigned int   Address;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef  DISASS
			printf ( "ORA (&%2x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
				DisplayRegisters();
				exit ( 5 );
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
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "ORA &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			Ora ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case ASL_ZPX :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;
#ifdef	DISASS
			printf ( "ASL &%x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			ShiftLeft ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case CLC :
#ifdef  DISASS
			printf ( "CLC\n" );
#endif
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
#ifdef  DISASS
			printf ( "ORA &%4x,Y\n", BaseAddress );
#endif
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
#ifdef  DISASS
			printf ( "ORA &%4x,X\n", BaseAddress );
#endif
			Ora ( ReadByte ( Address ));
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case ASL_AbX :
		{
			register unsigned char	temp1;
			register unsigned int   Address;

#ifdef	DISASS
			printf ( "ASL &%x,X\n", ReadWordAtPC());
#endif
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
#ifdef  DISASS
			printf ( "JSR &%x\n", Address );
#endif
			SetProgramCounter ( Address );
			AddClockCycles ( 6 );
			break;

		}
		case AND_IndX :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int	OperandAddress;

#ifdef	DISASS
			printf ( "AND (&%2x,X)\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
				DisplayRegisters();
				exit ( 4 );
			}
#endif
			OperandAddress = ReadWord ( ZeroPageAddress );
			And ( ReadByte ( OperandAddress ));
			AddClockCycles ( 6 );
			break;
		}
		case BIT_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "BIT &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Bit ( temp1 );
			AddClockCycles ( 3 );
			break;
		}
		case AND_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "AND &%x\n", ZeroPageAddress );
#endif
			And ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 3 );
			break;
		}
		case ROL_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "ROL &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			RotateLeft ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 5 );
			break;
		}
		case PLP :
#ifdef  DISASS
			printf ( "PLP\n" );
#endif
			UnstackSR();
			AddClockCycles ( 4 );
			break;

		case AND_Im :
#ifdef  DISASS
			printf ( "AND #&%x\n", *EmulatorPC );
#endif
			And ( *EmulatorPC++);
			AddClockCycles ( 2 );
			break;

		case ROL_A :
		{
			register unsigned char	temp2;

#ifdef  DISASS
			printf ( "ROL A\n" );
#endif
			RotateLeft ( Accumulator );
			AddClockCycles ( 2 );
			break;
		}
		case BIT_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "BIT &%x\n", OperandAddress );
#endif
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
#ifdef  DISASS
			printf ( "AND &%x\n", OperandAddress );
#endif
			And ( ReadByte ( OperandAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case ROL_Ab :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned int   Address;

			Address = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "ROL &%x\n", Address );
#endif
			temp1 = ReadByte ( Address );
			RotateLeft ( temp1 );
			WriteByte ( Address, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case BMI :
		{
			unsigned char			BranchCycleCount = 2;

#ifdef  DISASS
			printf ( "BMI #&%x\n", *EmulatorPC );
#endif
			DoBranch ( NegativeFlag );
			AddClockCycles ( BranchCycleCount );
			break;
		}
		case AND_IndY :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int	BaseAddress;
			register unsigned int   Address;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef  DISASS
			printf ( "AND (&%2x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
				DisplayRegisters();
				exit ( 5 );
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
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "AND &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			And ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case ROL_ZPX :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "ROL &%x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			RotateLeft ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case SEC :
#ifdef  DISASS
			printf ( "SEC\n" );
#endif
			SetCarryFlag;
			AddClockCycles ( 2 );
			break;

		case AND_AbY :
		{
			register unsigned int	BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "AND &%4x,Y\n", BaseAddress );
#endif
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
#ifdef  DISASS
			printf ( "AND &%4x,X\n", BaseAddress );
#endif
			And ( ReadByte ( Address ));
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case ROL_AbX :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned int   Address;

#ifdef  DISASS
			printf ( "ROL &%x,X\n", ReadWordAtPC());
#endif
			Address = ReadWordAtPC() + RegisterX;
			EmulatorPC += 2;
			temp1 = ReadByte ( Address );
			RotateLeft ( temp1 );
			WriteByte ( Address, temp1 );
			AddClockCycles ( 7 );
			break;
		}
		case RTI :
#ifdef  DISASS
			printf ( "RTI\n" );
#endif
			UnstackSR();
			UnstackPC();
			AddClockCycles ( 6 );
			break;

		case EOR_IndX :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int   OperandAddress;

			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  DISASS
			printf ( "EOR (&%2x,X)\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
				DisplayRegisters();
				exit ( 4 );
			}
#endif
			OperandAddress = ReadWord ( ZeroPageAddress );
			Eor ( ReadByte ( OperandAddress ));
			AddClockCycles ( 6 );
			break;
		}
		case EOR_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "EOR &%x\n", ZeroPageAddress );
#endif
			Eor ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 3 );
			break;
		}
		case LSR_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "LSR &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			ShiftRight ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 5 );
			break;
		}
		case PHA :
#ifdef  DISASS
			printf ( "PHA\n" );
#endif
			StackByte ( Accumulator );
			AddClockCycles ( 3 );
			break;

		case EOR_Im :
#ifdef  DISASS
			printf ( "EOR #&%x\n", *EmulatorPC );
#endif
			Eor ( *EmulatorPC++);
			AddClockCycles ( 2 );
			break;

		case LSR_A :
#ifdef	DISASS
			printf ( "LSR A\n" );
#endif
			ShiftRight ( Accumulator );
			AddClockCycles ( 2 );
			break;

		case JMP_Ab :
			SetProgramCounter ( ReadWordAtPC());
#ifdef  DISASS
			printf ( "JMP &%x\n", GetProgramCounter );
#endif
			AddClockCycles ( 3 );
			break;

		case EOR_Ab :
		{
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "EOR &%x\n", OperandAddress );
#endif
			Eor ( ReadByte ( OperandAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case LSR_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   Address;

			Address = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "LSR &%x\n", Address );
#endif
			temp1 = ReadByte ( Address );
			ShiftRight ( temp1 );
			WriteByte ( Address, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case BVC :
		{
		unsigned char           BranchCycleCount = 2;

#ifdef  DISASS
			printf ( "BVC #&%x\n", *EmulatorPC );
#endif
			DoBranch ( !OverflowFlag );
			AddClockCycles ( BranchCycleCount );
			break;

		}
		case EOR_IndY :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef  DISASS
			printf ( "EOR (&%2x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
				DisplayRegisters();
				exit ( 5 );
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
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "EOR &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			Eor ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case LSR_ZPX :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

#ifdef	DISASS
			printf ( "LSR &%x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			ShiftRight ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case CLI :
#ifdef  DISASS
			printf ( "CLI\n" );
#endif
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
#ifdef  DISASS
			printf ( "EOR &%4x,Y\n", BaseAddress );
#endif
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
#ifdef  DISASS
			printf ( "EOR &%4x,X\n", BaseAddress );
#endif
			Eor ( ReadByte ( Address ));
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case LSR_AbX :
		{
			register unsigned char	temp1;
			register unsigned int   Address;

#ifdef	DISASS
			printf ( "LSR &%x,X\n", ReadWordAtPC());
#endif
			Address = ReadWordAtPC() + RegisterX;
			EmulatorPC += 2;
			temp1 = ReadByte ( Address );
			ShiftRight ( temp1 );
			WriteByte ( Address, temp1 );
			AddClockCycles ( 7 );
			break;
		}
		case RTS :
#ifdef  DISASS
			printf ( "RTS\n" );
#endif
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
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;
			register unsigned int   OperandAddress;

#ifdef	DISASS
			printf ( "ADC (&%2x,X)\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle ADC (0xff,X)\n" );
				DisplayRegisters();
				exit ( 4 );
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
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "ADC &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Adc ( temp1 );
			AddClockCycles ( 3 );
			break;
		}
		case ROR_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "ROR &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			RotateRight ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 5 );
			break;
		}
		case PLA :
#ifdef  DISASS
			printf ( "PLA\n" );
#endif
			Accumulator = UnstackByte();
			ResetZeroFlag ( Accumulator == 0 );
			NegativeFlag = Accumulator & 0x80;
			AddClockCycles ( 4 );
			break;

		case ADC_Im :
		{
			register unsigned char	temp1;

#ifdef  DISASS
			printf ( "ADC #&%x\n", *EmulatorPC );
#endif
			temp1 = *EmulatorPC++;
			Adc ( temp1 );
			AddClockCycles ( 2 );
			break;
		}
		case ROR_A :
		{
			register unsigned char	temp2;

#ifdef  DISASS
			printf ( "ROR A\n" );
#endif
			RotateRight ( Accumulator );
			AddClockCycles ( 2 );
			break;
		}
		case JMP_Ind :
		{
			register unsigned int   OperandAddress;
			register unsigned int   Address;

			OperandAddress = ReadWordAtPC();
#ifdef  DISASS
			printf ( "JMP (&%x)\n", OperandAddress );
#endif
			/*
			 * On a page boundary, the address is read from the last and
			 * first bytes of the same page.
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
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "ADC &%x\n", OperandAddress );
#endif
			temp1 = ReadByte ( OperandAddress );
			Adc ( temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case ROR_Ab :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned int   Address;

			Address = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "ROR &%x\n", Address );
#endif
			temp1 = ReadByte ( Address );
			RotateRight ( temp1 );
			WriteByte ( Address, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case BVS :
		{
			unsigned char           BranchCycleCount = 2;

#ifdef  DISASS
			printf ( "BVS #&%x\n", *EmulatorPC );
#endif
			DoBranch ( OverflowFlag );
			AddClockCycles ( BranchCycleCount );
			break;
		}
		case ADC_IndY :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef  DISASS
	printf ( "ADC (&%2x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
				DisplayRegisters();
				exit ( 5 );
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
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "ADC &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Adc ( temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case ROR_ZPX :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "ROR &%x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			RotateRight ( temp1 );
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case SEI :
#ifdef  DISASS
			printf ( "SEI\n" );
#endif
			SetIRQDisableFlag;
			AddClockCycles ( 2 );
			break;

		case ADC_AbY :
		{
			register unsigned char	temp1;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
			Address = BaseAddress + RegisterY;
#ifdef  DISASS
			printf ( "ADC &%4x,Y\n", BaseAddress );
#endif
			temp1 = ReadByte ( Address );
			Adc ( temp1 );
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case ADC_AbX :
		{
			register unsigned char	temp1;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
			Address = BaseAddress + RegisterX;
#ifdef  DISASS
			printf ( "ADC &%4x,X\n", BaseAddress );
#endif
			temp1 = ReadByte ( Address );
			Adc ( temp1 );
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case ROR_AbX :
		{
			register unsigned char	temp1;
			register unsigned char	temp2;
			register unsigned int   Address;

#ifdef  DISASS
			printf ( "ROR &%x,X\n", ReadWordAtPC());
#endif
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
			register unsigned char	ZeroPageAddress;
			register unsigned int   OperandAddress;

#ifdef DISASS
			printf ( "STA (&%2x,X)\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
				DisplayRegisters();
				exit ( 4 );
			}
#endif
			OperandAddress = ReadWord ( ZeroPageAddress );
			WriteByte ( OperandAddress, Accumulator );
			AddClockCycles ( 6 );
			break;
		}
		case STY_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "STY &%2x\n", ZeroPageAddress );
#endif
			WriteLoPageByte ( ZeroPageAddress, RegisterY );
			AddClockCycles ( 3 );
			break;
		}
		case STA_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "STA &%x\n", ZeroPageAddress );
#endif
			WriteLoPageByte ( ZeroPageAddress, Accumulator );
			AddClockCycles ( 3 );
			break;
		}
		case STX_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "STX &%x\n", ZeroPageAddress );
#endif
			WriteLoPageByte ( ZeroPageAddress, RegisterX );
			AddClockCycles ( 3 );
			break;
		}
		case DEY :
#ifdef	DISASS
			printf ( "DEY\n" );
#endif
			RegisterY--;
			ResetZeroFlag ( RegisterY == 0 );
			NegativeFlag = RegisterY & 0x80;
			AddClockCycles ( 2 );
			break;
		case TXA :
#ifdef  DISASS
			printf ( "TXA\n" );
#endif
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
#ifdef  DISASS
			printf ( "STY &%x\n", OperandAddress );
#endif
			WriteByte ( OperandAddress, RegisterY );
			AddClockCycles ( 4 );
			break;
		}
		case STA_Ab :
		{
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "STA &%x\n", OperandAddress );
#endif
			WriteByte ( OperandAddress, Accumulator );
			AddClockCycles ( 4 );
			break;
		}
		case STX_Ab :
		{
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "STX &%x\n", OperandAddress );
#endif
			WriteByte ( OperandAddress, RegisterX );
			AddClockCycles ( 4 );
			break;
		}
		case BCC :
		{
			unsigned char           BranchCycleCount = 2;

#ifdef  DISASS
			printf ( "BCC #&%x\n", *EmulatorPC );
#endif
			DoBranch ( !CarryFlag );
			AddClockCycles ( BranchCycleCount );
			break;
		}
		case STA_IndY :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int   OperandAddress;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef	DISASS
			printf ( "STA (&%x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
				DisplayRegisters();
				exit ( 5 );
			}
#endif
			OperandAddress = ReadWord ( ZeroPageAddress ) + RegisterY;
			WriteByte ( OperandAddress, Accumulator );
			AddClockCycles ( 6 );
			break;
		}
		case STY_ZPX :
		{
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "STY &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			WriteLoPageByte ( ZeroPageAddress, RegisterY );
			AddClockCycles ( 4 );
			break;
		}
		case STA_ZPX :
		{
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "STA &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = ( *EmulatorPC++ + RegisterX ) & 0xff;
			WriteByte ( ZeroPageAddress, Accumulator );
			AddClockCycles ( 4 );
			break;
		}
		case STX_ZPY :
		{
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "STX &%2x,Y\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterY;
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle ZP,Y > 0xff\n" );
				DisplayRegisters();
				exit ( 3 );
			}
#endif
			WriteLoPageByte ( ZeroPageAddress, RegisterX );
			AddClockCycles ( 4 );
			break;
		}
		case TYA :
#ifdef  DISASS
			printf ( "TYA\n" );
#endif
			Accumulator = RegisterY;
			ResetZeroFlag ( Accumulator == 0 );
			NegativeFlag = Accumulator & 0x80;
			AddClockCycles ( 2 );
			break;

		case STA_AbY :
		{
			register unsigned int   OperandAddress;

#ifdef  DISASS
			printf ( "STA &%4x,Y\n", ReadWordAtPC());
#endif
			OperandAddress = ReadWordAtPC() + RegisterY;
			EmulatorPC += 2;
			WriteByte ( OperandAddress, Accumulator );
			AddClockCycles ( 5 );
			break;
		}
		case TXS :
#ifdef  DISASS
			printf ( "TXS\n" );
#endif
			StackPointer = RegisterX;
			break;

		case STA_AbX :
		{
			register unsigned int   OperandAddress;

#ifdef  DISASS
			printf ( "STA &%4x,X\n", ReadWordAtPC());
#endif
			OperandAddress = ReadWordAtPC() + RegisterX;
			EmulatorPC += 2;
			WriteByte ( OperandAddress, Accumulator );
			AddClockCycles ( 5 );
			break;

		}
		case LDY_Im :
#ifdef	DISASS
			printf ( "LDY #&%x\n", *EmulatorPC );
#endif
			LoadY ( *EmulatorPC++);
			AddClockCycles ( 2 );
			break;

		case LDA_IndX :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int   OperandAddress;

#ifdef  DISASS
			printf ( "LDA (&%2x,X)\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
				DisplayRegisters();
				exit ( 4 );
			}
#endif
			OperandAddress = ReadWord ( ZeroPageAddress );
			LoadA ( ReadByte ( OperandAddress ));
			AddClockCycles ( 6 );
			break;
		}
		case LDX_Im :
#ifdef	DISASS
			printf ( "LDX #%x\n", *EmulatorPC );
#endif
			LoadX ( *EmulatorPC++);
			AddClockCycles ( 2 );
			break;

		case LDY_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "LDY &%x\n", ZeroPageAddress );
#endif
			LoadY ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 3 );
			break;
		}
		case LDA_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "LDA &%x\n", ZeroPageAddress );
#endif
			LoadA ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 3 );
			break;
		}
		case LDX_ZP :
		{
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "LDX &%x\n", ZeroPageAddress );
#endif
			LoadX ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 3 );
			break;
		}
		case TAY :
#ifdef  DISASS
			printf ( "TAY\n" );
#endif
			RegisterY = Accumulator;
			ResetZeroFlag ( RegisterY == 0 );
			NegativeFlag = RegisterY & 0x80;
			AddClockCycles ( 2 );
			break;

		case LDA_Im :
#ifdef	DISASS
			printf ( "LDA #%x\n", *EmulatorPC );
#endif
			LoadA ( *EmulatorPC++);
			AddClockCycles ( 2 );
			break;

		case TAX :
#ifdef  DISASS
			printf ( "TAX\n" );
#endif
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
#ifdef  DISASS
			printf ( "LDY &%x\n", OperandAddress );
#endif
			LoadY ( ReadByte ( OperandAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case LDA_Ab :
		{
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "LDA &%x\n", OperandAddress );
#endif
			LoadA ( ReadByte ( OperandAddress ));
			AddClockCycles ( 4 );
			break;

		}
		case LDX_Ab :
		{
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "LDX &%x\n", OperandAddress );
#endif
			LoadX ( ReadByte ( OperandAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case BCS :
		{
			unsigned char           BranchCycleCount = 2;
#ifdef  DISASS
			printf ( "BCS #&%x\n", *EmulatorPC );
#endif
			DoBranch ( CarryFlag );
			AddClockCycles ( BranchCycleCount );
			break;
		}
		case LDA_IndY :
		{
			register unsigned char	ZeroPageAddress;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef  DISASS
			printf ( "LDA (&%2x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
				DisplayRegisters();
				exit ( 5 );
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
			register unsigned char	ZeroPageAddress;

#ifdef	DISASS
			printf ( "LDY &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			LoadY ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case LDA_ZPX :
		{
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "LDA &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			LoadA ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case LDX_ZPY :
		{
			register unsigned char	ZeroPageAddress;

#ifdef	DISASS
			printf ( "LDX &%2x,Y\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterY;
			if ( ZeroPageAddress > 0xff )
			{
				fprintf ( stderr, "Can't handle ZP,Y > 0xff\n" );
				DisplayRegisters();
				exit ( 3 );
			}
			LoadX ( ReadLoPageByte ( ZeroPageAddress ));
			AddClockCycles ( 4 );
			break;
		}
		case CLV :
#ifdef  DISASS
			printf ( "CLV\n" );
#endif
			OverflowFlag = 0;
			AddClockCycles ( 2 );
			break;

		case LDA_AbY :
		{
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "LDA &%4x,Y\n", BaseAddress );
#endif
			Address = BaseAddress + RegisterY;
			LoadA ( ReadByte ( Address ));
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case TSX :
#ifdef  DISASS
			printf ( "TSX\n" );
#endif
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
#ifdef	DISASS
			printf ( "LDY &%4x,X\n", BaseAddress );
#endif
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
#ifdef  DISASS
			printf ( "LDA &%4x,X\n", BaseAddress );
#endif
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
#ifdef	DISASS
			printf ( "LDX &%4x,Y\n", BaseAddress );
#endif
			LoadX ( ReadByte ( Address ));
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case CPY_Im :
		{
			register unsigned char	temp1;

#ifdef	DISASS
			printf ( "CPY #&%x\n", *EmulatorPC );
#endif
			temp1 = *EmulatorPC++;
			Cmp ( RegisterY, temp1 );
			AddClockCycles ( 2 );
			break;
		}
		case CMP_IndX :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;
			register unsigned int   OperandAddress;

#ifdef	DISASS
			printf ( "CMP (&%2x,X)\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle CMP (0xff,X)\n" );
				DisplayRegisters();
				exit ( 4 );
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
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "CPY &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Cmp ( RegisterY, temp1 );
			AddClockCycles ( 3 );
			break;
		}
		case CMP_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "CMP &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Cmp ( Accumulator, temp1 );
			AddClockCycles ( 3 );
			break;
		}
		case DEC_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "DEC &%2x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			temp1--;
			ResetZeroFlag ( temp1 == 0 );
			NegativeFlag = temp1 & 0x80;
			WriteByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 5 );
			break;
		}
		case INY :
#ifdef	DISASS
			printf ( "INY\n" );
#endif
			RegisterY++;
			ResetZeroFlag ( RegisterY == 0 );
			NegativeFlag = RegisterY & 0x80;
			AddClockCycles ( 2 );
			break;

		case CMP_Im :
		{
			register unsigned char	temp1;

#ifdef	DISASS
			printf ( "CMP #&%x\n", *EmulatorPC );
#endif
			temp1 = *EmulatorPC++;
			Cmp ( Accumulator, temp1 );
			AddClockCycles ( 2 );
			break;
		}
		case DEX :
#ifdef	DISASS
			printf ( "DEX\n" );
#endif
			RegisterX--;
			ResetZeroFlag ( RegisterX == 0 );
			NegativeFlag = RegisterX & 0x80;
			AddClockCycles ( 2 );
			break;

		case CPY_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "CPY &%x\n", OperandAddress );
#endif
			temp1 = ReadByte ( OperandAddress );
			Cmp ( RegisterY, temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case CMP_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "CMP &%x\n", OperandAddress );
#endif
			temp1 = ReadByte ( OperandAddress );
			Cmp ( Accumulator, temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case DEC_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "DEC &%4x\n", OperandAddress );
#endif
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
			unsigned char           BranchCycleCount = 2;

#ifdef  DISASS
			printf ( "BNE #&%x\n", *EmulatorPC );
#endif
			DoBranch ( !ZeroFlag );
			AddClockCycles ( BranchCycleCount );
			break;
		}
		case CMP_IndY :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef	DISASS
			printf ( "CMP (&%2x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle CMP (0xff),Y\n" );
				DisplayRegisters();
				exit ( 5 );
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
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

#ifdef	DISASS
			printf ( "CMP &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Cmp ( Accumulator, temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case DEC_ZPX :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef	DISASS
			printf ( "DEC &%2x,X\n", *EmulatorPC );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			temp1--;
			ResetZeroFlag ( temp1 == 0 );
			NegativeFlag = temp1 & 0x80;
			WriteByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 6 );
			break;
		}
		case CLD :
#ifdef  DISASS
			printf ( "CLD\n" );
#endif
			DecimalModeFlag = 0;
			AddClockCycles ( 2 );
			break;

		case CMP_AbY :
		{
			register unsigned char	temp1;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
	printf ( "CMP &%4x,Y\n",  BaseAddress );
#endif
			Address = BaseAddress + RegisterY;
			temp1 = ReadByte ( Address );
			Cmp ( Accumulator, temp1 );
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case CMP_AbX :
		{
			register unsigned char	temp1;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "CMP &%4x,X\n", BaseAddress );
#endif
			Address = BaseAddress + RegisterX;
			temp1 = ReadByte ( Address );
			Cmp ( Accumulator, temp1 );
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case DEC_AbX :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC() + RegisterX;
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "DEC &%4x,X\n", ReadWordAtPC());
#endif
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
			register unsigned char	temp1;
#ifdef	DISASS
			printf ( "CPX #&%x\n", *EmulatorPC );
#endif
			temp1 = *EmulatorPC++;
			Cmp ( RegisterX, temp1 );
			AddClockCycles ( 2 );
			break;
		}
		case SBC_IndX :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;
			register unsigned int   OperandAddress;

#ifdef  DISASS
			printf ( "SBC (&%2x,X)\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff, X )\n" );
				DisplayRegisters();
				exit ( 4 );
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
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "CPX &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Cmp ( RegisterX, temp1 );
			AddClockCycles ( 3 );
			break;
		}
		case SBC_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef  DISASS
			printf ( "SBC &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Sbc ( temp1 );
			AddClockCycles ( 3 );
			break;
		}
		case INC_ZP :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

			ZeroPageAddress = *EmulatorPC++;
#ifdef	DISASS
			printf ( "INC &%x\n", ZeroPageAddress );
#endif
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			temp1++;
			ResetZeroFlag ( temp1 == 0 );
			NegativeFlag = temp1 & 0x80;
			WriteLoPageByte ( ZeroPageAddress, temp1 );
			AddClockCycles ( 5 );
			break;
		}
		case INX :
#ifdef	DISASS
			printf ( "INX\n" );
#endif
			RegisterX++;
			ResetZeroFlag ( RegisterX == 0 );
			NegativeFlag = RegisterX & 0x80;
			AddClockCycles ( 2 );
			break;

		case SBC_Im :
		{
			register unsigned char	temp1;

#ifdef  DISASS
			printf ( "SBC #&%x\n", *EmulatorPC );
#endif
			temp1 = *EmulatorPC++;
			Sbc ( temp1 );
			AddClockCycles ( 2 );
			break;
		}
		case NOP :
#ifdef  DISASS
			printf ( "NOP\n" );
#endif
			AddClockCycles ( 2 );
			break;

		case CPX_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "CPX &%x\n", OperandAddress );
#endif
			temp1 = ReadByte ( OperandAddress );
			Cmp ( RegisterX, temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case SBC_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef  DISASS
			printf ( "SBC &%x\n", OperandAddress );
#endif
			temp1 = ReadByte ( OperandAddress );
			Sbc ( temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case INC_Ab :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

			OperandAddress = ReadWordAtPC();
			EmulatorPC += 2;
#ifdef	DISASS
			printf ( "INC &%x\n", OperandAddress );
#endif
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
			unsigned char           BranchCycleCount = 2;

#ifdef  DISASS
			printf ( "BEQ #&%x\n", *EmulatorPC );
#endif
			DoBranch ( ZeroFlag );
			AddClockCycles ( BranchCycleCount );
			break;
		}
		case SBC_IndY :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			ZeroPageAddress = ( *EmulatorPC++);
#ifdef  DISASS
			printf ( "SBC (&%2x),Y\n", ZeroPageAddress );
#endif
#ifdef  RANGE_CHECK
			if ( ZeroPageAddress == 0xff )
			{
				fprintf ( stderr, "Can't handle <op> ( 0xff ), Y\n" );
				DisplayRegisters();
				exit ( 5 );
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
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

#ifdef  DISASS
			printf ( "SBC &%2x,X\n", *EmulatorPC );
#endif
			ZeroPageAddress = *EmulatorPC++ + RegisterX;
			temp1 = ReadLoPageByte ( ZeroPageAddress );
			Sbc ( temp1 );
			AddClockCycles ( 4 );
			break;
		}
		case INC_ZPX :
		{
			register unsigned char	temp1;
			register unsigned char	ZeroPageAddress;

#ifdef	DISASS
			printf ( "INC &%x,Y\n", *EmulatorPC );
#endif
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
#ifdef  DISASS
			printf ( "SED\n" );
#endif
			SetDecimalModeFlag;
			AddClockCycles ( 2 );
			break;

		case SBC_AbY :
		{
			register unsigned char	temp1;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
			Address = BaseAddress + RegisterY;
#ifdef  DISASS
			printf ( "SBC &%4x,Y\n", BaseAddress );
#endif
			temp1 = ReadByte ( Address );
			Sbc ( temp1 );
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case SBC_AbX :
		{
			register unsigned char	temp1;
			register unsigned int   BaseAddress;
			register unsigned int   Address;

			BaseAddress = ReadWordAtPC();
			EmulatorPC += 2;
			Address = BaseAddress + RegisterX;
#ifdef  DISASS
			printf ( "SBC &%4x,X\n", BaseAddress );
#endif
			temp1 = ReadByte ( Address );
			Sbc ( temp1 );
			AddClockCycles((( BaseAddress >> 8 ) == ( Address >> 8 )) ? 4 : 5);
			break;
		}
		case INC_AbX :
		{
			register unsigned char	temp1;
			register unsigned int   OperandAddress;

#ifdef	DISASS
			printf ( "INC &%x,Y\n", ReadWordAtPC());
#endif
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
		default :
#ifdef  DISASS
			printf ( "???\n" );
#endif
			fprintf ( stderr, "Illegal opcode -- 0x%x\n", *( EmulatorPC - 1 ));
			DisplayRegisters();
			exit ( 1 );
			break;
	}

	if ( MaskableInterruptRequest && !IRQDisableFlag )
	{
		/*
		 * We have to explicitly stack the status register here, otherwise
		 * it will be pushed on the stack with the BRK flag set.
		 */

		StackPC();
		GenerateIRQStatusRegister;
		StackByte ( StatusRegister );
		SetIRQDisableFlag;
		SetProgramCounter ( IRQAddress );
		AddClockCycles ( 7 );
		MaskableInterruptRequest--;
	}
