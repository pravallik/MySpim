//Julianne Truong and Lakshmi Pravallika Kastala

#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch ((int)ALUControl) {


        case '0':
            *ALUresult = A + B;
            break;

        case '1':
            *ALUresult = A - B;
            break;

        case '2':

            if((int)A < (int)B) {
                *ALUresult = 1;
            }

            else {
                *ALUresult = 0;
            }
            break;

        case '3':

            if(A < B) {
                *ALUresult = 1;
            }

            else {
                *ALUresult = 0;
            }
            break;

        case '4':

            *ALUresult = A & B;
            break;

        case '5':

            *ALUresult = A | B;
            break;

        case '6':

            *ALUresult = B << 16;
            break;

        case '7':

            *ALUresult = ~A;
    }


    if(*ALUresult == 0) {
        *Zero = '1';
    }

    else {
        *Zero = '0';
    }
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
// if the program couter is multiple of 4 then the mem[ PC ] is shifed right by 2 un
    if(PC % 4 == 0) {
        *instruction = Mem[PC >> 2];
    }

    else {
        return 1;
    }

    return 0;
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    //instructions are parsed and the values that correspond with it are set in the correct fields

    *op      = (instruction & 0xfc000000) >> 26;
    *funct   = instruction & 0x0000003f;
    *offset  = instruction & 0x0000ffff;
    *jsec    = instruction & 0x03ffffff;

    *r1      = (instruction & 0x03e00000) >> 21;
    *r2      = (instruction & 0x001f0000) >> 16;  // registers
    *r3      = (instruction & 0x0000f800) >> 11;

}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
 // first setting all the control units to zero and the units update to new depending on the instruction type I/r/j - types
       controls->RegDst = '0';
        controls->Jump = '0';
        controls->Branch = '0';
        controls->MemRead = '0';
        controls->MemtoReg = '0';
        controls->ALUOp = '0';
        controls->MemWrite = '0';
        controls->ALUSrc = '0';
        controls->RegWrite = '0';

    switch(op) {
        //R-Type
        case 0:
            controls->RegDst = '1';
            controls->ALUOp = '7';
            controls->RegWrite = '1';
        break;

        //loadUpperI
        case 15:

            controls->ALUOp = '6';
            controls->ALUSrc = '1';
            controls->RegWrite = '1';
        break;

        //BranchOneq
        case 4:
            controls->RegDst = '2';
            controls->Branch = '1';

            controls->MemtoReg = '2';
            controls->ALUOp = '1';

            controls->ALUSrc = '2';

        break;

        //SetLessTI
        case 10:

            controls->ALUOp = '2';
            controls->ALUSrc = '1';
            controls->RegWrite = '1';
        break;

        //SetLessTIU
        case 11:

            controls->ALUOp = '3';

            controls->ALUSrc = '1';
            controls->RegWrite = '1';
        break;

        //LoadWord
        case 35:

            controls->MemRead = '1';
            controls->MemtoReg = '1';

            controls->ALUSrc = '1';
            controls->RegWrite = '1';
        break;

        //StoreWord
        case 43:

            controls->MemWrite = '1';
            controls->ALUSrc = '1';

        break;

        //J-Type
        case 2:
            controls->RegDst = '2';
            controls->Jump = '1';
            controls->Branch = '2';

            controls->MemtoReg = '2';

            controls->ALUSrc = '2';

        break;

        //AddI
        case 8:

            controls->ALUSrc = '1';
            controls->RegWrite = '1';
        break;

        default:
            return 1;
    }

    return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

    *data1 = Reg[r1]; // r1 register in data 1
    *data2 = Reg[r2]; // r2 register in data 2
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

//if  positive, the upper 16 bits  1s
//if negative, the bits are 0s
    unsigned signs = offset  >> 15;

    if(signs == 0) {
//positve number
        *extended_value = 0x0000ffff & offset;
    }
    else {
//negative number
        *extended_value =   0xffff0000 | offset ;
    }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    if(ALUSrc == '1') {
        data2 = extended_value;
    }

    if(ALUOp == '7') {
        switch(funct){
//AND
            case 36:
                ALUOp = '4';
                break;

//ADD
            case 32:
                ALUOp = '0';
                break;

//SUB
            case 34:
                ALUOp = '1';
                break;

//OR
            case 37:
                ALUOp = '5';
                break;

//Slt SIGNED
            case 42:
                ALUOp = '2';
                break;

//Slt UNSIGNED
            case 43:
                ALUOp = '3';
                break;

//SHIFT
            case 6:
                ALUOp = '6';
                break;

//NOR
            case 39:
                ALUOp = '7';
                break;


            default:
                return 1;
        }

        ALU(data1, data2, ALUOp, ALUresult, Zero);
    }
    else {
        ALU(data1, data2, ALUOp, ALUresult, Zero);
    }

    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

if(MemRead == '1')
	{
		if(ALUresult % 4 == 0) {

// if ALUresult multiple of 4 we shift right by 2
			*memdata = Mem[ALUresult >> 2];
		}
		else {
			return 1;
    }
}
// writing the datae into the > memory * 4
if(MemWrite == '1')
	{
    if( ALUresult % 4 == 0){
			Mem[ALUresult >> 2] = data2;
		}
		else {
			return 1;
		}
    }
return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
  if(RegWrite == '1') {
      if(MemtoReg == '1') {
			Reg[r2] = memdata;
      }
		else if(MemtoReg == '0') {
			if(RegDst == '1') {
				Reg[r3] = ALUresult;
        }
			else
				Reg[r2] = ALUresult;
        }
  }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
// add 4 to program counter to update to new inst
   *PC = *PC + 4;

    if(Zero == '1' && Branch == '1') {
// for zero and branch we add by 4
        *PC = *PC + (extended_value << 2);
    }

    if(Jump == '1') {
// for jump we multiply by 4
        *PC = (*PC & 0xf0000000) | (jsec << 2);
    }

}

