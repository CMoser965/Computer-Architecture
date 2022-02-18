/***************************************************************/
/*                                                             */
/*   ARMv4-32 Instruction Level Simulator                      */
/*                                                             */
/*   ECEN 4243                                                 */
/*   Oklahoma State University                                 */
/*                                                             */
/***************************************************************/

#ifndef _SIM_ISA_H_
#define _SIM_ISA_H_
#define N_CUR ( (CURRENT_STATE.CPSR>>31) & 0x00000001 )
#define Z_CUR ( (CURRENT_STATE.CPSR>>30) & 0x00000001 )
#define C_CUR ( (CURRENT_STATE.CPSR>>29) & 0x00000001 )
#define V_CUR ( (CURRENT_STATE.CPSR>>28) & 0x00000001 )
#define N_NXT ( (NEXT_STATE.CPSR>>31) & 0x00000001 )
#define Z_NXT ( (NEXT_STATE.CPSR>>30) & 0x00000001 )
#define C_NXT ( (NEXT_STATE.CPSR>>29) & 0x00000001 )
#define V_NXT ( (NEXT_STATE.CPSR>>28) & 0x00000001 )

#define N_N 0x80000000 //negative
#define Z_N 0x40000000 //zero
#define C_N 0x20000000 //carry
#define V_N 0x10000000 //overflow

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"


/**
 * 
 * DATA PROCESSING
 * Functions used in data process to decode and execute subset of data 
 * processing and instructions of ARM ISA
 * 
 */
int AND (int Rd, int Rn, int Operand2, int I, int S, int CC){
  // Rd <- Rn & Src2(which is Operand2)
  int cur = 0;
  if(I == 0){ //Register or Register-Shifted Register
    int shamt5 = (Operand2 & 0x00000F80) >> 7; // shift amount (5 bit unsighed integer)
    int sh = (Operand2 & 0x00000060) >> 5;
      /*
        00 -> (0) -> logical left
        01 -> (1) -> logical right
        10 -> (2) -> arithmetic right
        11 -> (3) -> rotate right

      */
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    int bit4 = (Operand2 & 0x00000010) >> 4;  

    if (bit4 == 0) { //0: Register-Shifted Register (immediate shift value)
      switch (sh) {
        case 0: //logical left shift
          cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] << shamt5);
          break;
        case 1: //logical right shift
          cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 2: //arithmetic right shift
          cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> shamt5);
          break;
        case 3: //rotate right shift
          cur = CURRENT_STATE.REGS[Rn] & ((CURRENT_STATE.REGS[Rm] >> shamt5) | (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
          break;
      }
    } else { //1: Register
      switch (sh) {
        case 0: //logical left shift
          cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
          break;
        case 1: //logical right shift
          cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 2: //arithmetic right shift
          cur = CURRENT_STATE.REGS[Rn] & (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
          break;
        case 3: //rotate right shift
          cur = CURRENT_STATE.REGS[Rn] & ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) | (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
          break;
      }     
    }
  } else if (I == 1){  //Immediate
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0) {
      NEXT_STATE.CPSR |= N_N;
    } 
    if (cur == 0) {
      NEXT_STATE.CPSR |= Z_N;
    }
  }	
  return 0;
}

int EOR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int SUB (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int RSB (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int ADD (int Rd, int Rn, int Operand2, int I, int S, int CC) {

  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    	  break;
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;

}

int ADC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int SBC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int RSC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int TST (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int TEQ (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int CMP (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int CMN (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int ORR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int MOV (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

//1101 -> with MOV
int LSL (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

//1101 -> with MOV
int LSR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

//1101 -> with MOV
int ASR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

//1101 -> with MOV
int RRX (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

//1101 -> with MOV
int ROR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int BIC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int MVN (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}





/**
 * 
 * BRANCH PROCESS
 * 
 */ 
int B (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int BL (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}


/**
 * 
 * MUL PROCESS
 * 
 */
int MUL (char* i_);

int MLA (char* i_);

int UMULL (char* i_);

int UMLAL (char* i_);

int SMULL (char* i_);

int SMLAL (char* i_);

/**
 * 
 * TRANSFER PROCESS
 * 
 */
//memory: or transfer???
int STR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int LDR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int STRB (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int LDRB (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int STRH (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int LDRH (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int LDRSB (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

int LDRSH (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}


/**
 * 
 * INTERRUPTION PROCESS
 * 
 */
int SWI (int Rd, int Rn, int Operand2, int I, int S, int CC){
  return 0;
}

#endif
