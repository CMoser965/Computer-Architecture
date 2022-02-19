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
  int a = 0;
  int b = 0;

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

    if (bit4 == 0) 
      switch (sh) {
      case 0: // LLS
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] << shamt5;
        cur = a & b;
	      break;
      case 1: // LRS
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> shamt5;
        cur = a & b;
	      break;
      case 2: // ARS
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> shamt5;
        int msb = CURRENT_STATE.REGS[Rm] & 0x10000000;
        for(int i = 0; i < shamt5; i++) {
          msb >> 1;
          b |= msb;
        }
        cur = a & b;
    	  break;
      case 3: // ROR
	      a = CURRENT_STATE.REGS[Rn];
        b = (CURRENT_STATE.REGS[Rm] >> shamt5) | (CURRENT_STATE.REGS[Rm] << (32 - shamt5));
        cur = a & b;
    	  break;
      }     
    else
      switch (sh) {
      case 0:
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs];
        cur = a & b;
    	  break;
      case 1:
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
        cur = a & b;
    	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
        int msb = CURRENT_STATE.REGS[Rm] & 0x10000000;
        for(int i = 0; i < shamt5; i++) {
          msb >> 1;
          b |= msb;
        }
        cur = a & b;
    	  break;
      case 3: 
        a = CURRENT_STATE.REGS[Rn];
        b = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) | (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]));
        cur = a & b;
    	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    a = CURRENT_STATE.REGS[Rn];
    b = Imm>>2*rotate|(Imm<<(32-2*rotate));
    cur = a & b;
  }

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0) {
      NEXT_STATE.CPSR |= N_N;
    } 
    if (cur == 0) {
      NEXT_STATE.CPSR |= Z_N;
    }
    if (setOverflow(a, b, cur)){
      NEXT_STATE.CPSR |= V_N;
    }
    if (cur > 0x11111111){
      NEXT_STATE.CPSR |= C_N;
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
  int a = 0;
  int b = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0:
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] << shamt5;
        cur = a + b;
	      break;
      case 1:
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> shamt5;
        cur = a + b;
	      break;
      case 2: 
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> shamt5;
        int msb = CURRENT_STATE.REGS[Rm] & 0x10000000;
        for(int i = 0; i < shamt5; i++) {
          msb >> 1;
          b |= msb;
        }
        cur = a + b;
    	  break;
      case 3:
	      a = CURRENT_STATE.REGS[Rn];
        b = (CURRENT_STATE.REGS[Rm] >> shamt5) | (CURRENT_STATE.REGS[Rm] << (32 - shamt5));
        cur = a + b;
    	  break;
      }     
    else
      switch (sh) {
      case 0:
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs];
        cur = a + b;
    	  break;
      case 1:
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
        cur = a + b;
    	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
        int msb = CURRENT_STATE.REGS[Rm] & 0x10000000;
        for(int i = 0; i < shamt5; i++) {
          msb >> 1;
          b |= msb;
        }
        cur = a + b;
    	  break;
      case 3: 
        a = CURRENT_STATE.REGS[Rn];
        b = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) | (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]));
        cur = a + b;
    	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    a = CURRENT_STATE.REGS[Rn];
    b = Imm>>2*rotate|(Imm<<(32-2*rotate));
    cur = a + b;
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (setOverflow(a,b,cur))
      NEXT_STATE.CPSR |= V_N;
    if (cur > 0x11111111)
      NEXT_STATE.CPSR |= C_N;
  }	
  return 0;

}

int ADC (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  int a = 0;
  int b = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {
      case 0:
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] << shamt5;
        cur = a + b + C_CUR;
	      break;
      case 1:
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> shamt5;
        cur = a + b + C_CUR;
	      break;
      case 2: 
        a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> shamt5;
        int msb = CURRENT_STATE.REGS[Rm] & 0x10000000;
        for(int i = 0; i < shamt5; i++) {
          msb >> 1;
          b |= msb;
        }
        cur = a + b + C_CUR;
    	  break;
      case 3:
	      a = CURRENT_STATE.REGS[Rn];
        b = (CURRENT_STATE.REGS[Rm] >> shamt5) | (CURRENT_STATE.REGS[Rm] << (32 - shamt5));
        cur = a + b + C_CUR;
    	  break;
      }     
    else
      switch (sh) {
      case 0:
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs];
        cur = a + b + C_CUR;
    	  break;
      case 1:
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
        cur = a + b + C_CUR;
    	  break;
      case 2: cur = CURRENT_STATE.REGS[Rn] + (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	      a = CURRENT_STATE.REGS[Rn];
        b = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];
        int msb = CURRENT_STATE.REGS[Rm] & 0x10000000;
        for(int i = 0; i < shamt5; i++) {
          msb >> 1;
          b |= msb;
        }
        cur = a + b + C_CUR;
    	  break;
      case 3: 
        a = CURRENT_STATE.REGS[Rn];
        b = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) | (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]));
        cur = a + b + C_CUR;
    	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    a = CURRENT_STATE.REGS[Rn];
    b = Imm>>2*rotate|(Imm<<(32-2*rotate));
    cur = a + b + C_CUR;
  }
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (setOverflow(a,b,cur))
      NEXT_STATE.CPSR |= V_N;
    if (cur > 0x11111111)
      NEXT_STATE.CPSR |= C_N;
  }	
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


/**
 * 
 * Extra Functions:
 * 
 */ 

/*
  Overflow affects: 
    Add:        ADDS  ADCS  
    Substract:  SUBS  SBCS  RSBS
    Compare:    CMP   CMN
    Shifts:     ASRS  LSLS  LSRS  RORS  RRXS
    Logical:    ANDS  ORRS  EORS  BICS
    Test:       TEQ   TST
    Move:       MOVS  MVNS
    Multiply:   MULS  MLAS  SMLALS  SMULLS  UMLALS  UMULLS
*/
int setOverflow (int a, int b, int c){
  //0xXXXX_XXXX 
  int MSB = 0x10000000;
  if ( ( (a & MSB) & (b & MSB) & !(c & MSB) ) || ( !(a & MSB) & !(b & MSB) & (c & MSB) ) ) {
    return 1;
  }
  return 0;
}

#endif
