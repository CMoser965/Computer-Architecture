/***************************************************************/
/*                                                             */
/*   ARMv8-32 Instruction Level Simulator                      */
/*                                                             */
/*   ECEN 4243                                                 */
/*   Oklahoma State University                                 */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "isa.h"


char *byte_to_binary12 (int x) {

  static char b[13];
  b[0] = '\0';

  int z;
  for (z = 2048; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

char *byte_to_binary4 (int x) {

  static char b[5];
  b[0] = '\0';

  int z;
  for (z = 8; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

char *byte_to_binary32(int x) {

  static char b[33];
  b[0] = '\0';

  unsigned int z;
  for (z = 2147483648; z > 0; z >>= 1) {
    strcat(b, ((x & z) == z) ? "1" : "0");
  }

  return b;
}

int bchar_to_int(char* rsa) {

  int i = 0;
  int result = 0;
  int t = 0;
  while(rsa[i] != '\0')i++;
  while(i>0)
    {
      --i;
      // printf("%d\n", (rsa[i]-'0')<<t);
      result += (rsa[i] - '0')<<t;
      t++;
    }
  return result;
}


int data_process(char* i_) {

  /*
    This function further decode and execute subset of data processing 
    instructions of ARM ISA.

    0000 = AND - Rd:= Op1 AND Op2
    0001 = EOR - Rd:= Op1 EOR Op2
    0010 = SUB - Rd:= Op1 - Op2
    0011 = RSB - Rd:= Op2 - Op1
    0100 = ADD - Rd:= Op1 + Op2
    0101 = ADC - Rd:= Op1 + Op2 + C
    0110 = SBC - Rd:= Op1 - Op2 + C - 1
    0111 = RSC - Rd:= Op2 - Op1 + C - 1
    1000 = TST - set condition codes on Op1 AND Op2 
    1001 = TEQ - set condition codes on Op1 EOR Op2 
    1010 = CMP - set condition codes on Op1 - Op2 
    1011 = CMN - set condition codes on Op1 + Op2 
    1100 = ORR - Rd:= Op1 OR Op2
    1101 = MOV - Rd:= Op2
    1110 = BIC - Rd:= Op1 AND NOT Op2 
    1111 = MVN - Rd:= NOT Op2
  */

  char d_opcode[5];
  d_opcode[0] = i_[7]; 
  d_opcode[1] = i_[8]; 
  d_opcode[2] = i_[9]; 
  d_opcode[3] = i_[10]; 
  d_opcode[4] = '\0';
  char d_cond[5];
  d_cond[0] = i_[0]; 
  d_cond[1] = i_[1]; 
  d_cond[2] = i_[2]; 
  d_cond[3] = i_[3]; 
  d_cond[4] = '\0';
  char rn[5]; rn[4] = '\0';
  char rd[5]; rd[4] = '\0';
  char operand2[13]; operand2[12] = '\0';
  for(int i = 0; i < 4; i++) {
    rn[i] = i_[12+i];
    rd[i] = i_[16+i];
  }
  for(int i = 0; i < 12; i++) {
    operand2[i] = i_[20+i];
  }
  int Rn = bchar_to_int(rn);
  int Rd = bchar_to_int(rd);
  int Operand2 = bchar_to_int(operand2);
  int I = i_[6]-'0';
  int S = i_[11]-'0';
  int CC = bchar_to_int(d_cond);
  printf("Opcode = %s\n Rn = %d\n Rd = %d\n Operand2 = %s\n I = %d\n S = %d\n COND = %s\n", d_opcode, Rn, Rd, byte_to_binary12(Operand2), I, S, byte_to_binary4(CC));
  printf("\n");
  
  // use for LSL and LSR case
  char shamt5[6]; char sh[3]; char rm[5];
  if(I) {
    shamt5[5] = '\0'; sh[2] = '\0'; rm[4] = '\0';
    for(int i = 0; i < 6; i++) {
      if(i<2) 
        sh[i] = operand2[i+5];
      if(i<4)
        rm[i] = operand2[i+8];
      shamt5[i] = operand2[i];
    }
    printf("shamt5 = %s\n sh = %s\n Rm = %s\n", shamt5, sh, rm);
  }

  /* Example - use and replicate */
  if(!strcmp(d_opcode,"0100")) {
    printf("--- This is an ADD instruction. \n");
    ADD(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }	

  /* Add other data instructions here */ 
  else if(!strcmp(d_opcode, "0000")) {
    printf("--- This is an AND instruction. \n");
    AND(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "0001")) {
    printf("--- This is an EOR instruction. \n");
    AND(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "0010")) {
    printf("--- This is a SUB instruction. \n");
    SUB(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "0011")) {
    printf("--- This is an RSB instruction. \n");
    RSB(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "0101")) {
    printf("--- This is an ADC instruction. \n");
    ADC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "0110")) {
    printf("--- This is an SBC instruction. \n");
    SBC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "0111")) {
    printf("--- This is an RSC instruction. \n");
    RSC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }
  
  else if(!strcmp(d_opcode, "1000")) {
    printf("--- This is a TST instruction. \n");
    TST(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "1001")) {
    printf("--- This is a TEQ instruction. \n");
    TEQ(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "1010")) {
    printf("--- This is a CMP instruction. \n");
    CMP(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "1011")) {
    printf("--- This is a CMN instruction. \n");
    CMN(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "1100")) {
    printf("--- This is an ORR instruction. \n");
    ORR(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "1101")) {
    if(I) { // MOV case
      printf("--- This is an MOV instruction. \n");
      MOV(Rd, Rn, Operand2, I, S, CC);
      return 0;
    } else if(!strcmp(sh, "00")) { // LSL case
      LSL(Rd, rm, Operand2, I, S, CC);
    } else if(!strcmp(sh, "01")) {
      LSR(Rd, rm, Operand2, I, S, CC);
    }
  }

  else if(!strcmp(d_opcode, "1110")) {
    printf("--- This is a BIC instruction. \n");
    BIC(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  else if(!strcmp(d_opcode, "1111")) {
    printf("--- This is an MVN instruction. \n");
    MVN(Rd, Rn, Operand2, I, S, CC);
    return 0;
  }

  return 1;	
}

int branch_process(char* i_) {
  
  /* This function execute branch instruction */

  /* Add branch instructions here */ 
  char d_cond[5];
  d_cond[0] = i_[0];
  d_cond[1] = i_[1];
  d_cond[2] = i_[2];
  d_cond[3] = i_[3];
  d_cond[4] = '\0';
  int L;
  L = i_[7] - '0';
  char imm24[25];
  imm24[24] = '\0';
  for(int i = 0; i < 24; i++) {
    imm24[i] = i_[8+i];
  }
  int IM = bchar_to_int(imm24);
  printf("Cond = %s\n 1L = 1%d\n imm24 = %s\n", d_cond, L, imm24);
  if(!L) {
    printf("--- This is a Branch instruction. \n");
    B(IM);
  }
  else {
    printf("--- This is a Branch with Link Instruction. \n");
    BL(IM);
  }
  return 1;
}

int mul_process(char* i_) {

  /* This function execute multiply instruction */

  /* Add multiply instructions here */ 
  char d_opcode[4]; 
  d_opcode[1] = i_[8]; 
  d_opcode[2] = i_[9]; 
  d_opcode[3] = i_[10]; 
  d_opcode[4] = '\0';
  char d_cond[5];
  d_cond[0] = i_[0]; 
  d_cond[1] = i_[1]; 
  d_cond[2] = i_[2]; 
  d_cond[3] = i_[3]; 
  d_cond[4] = '\0';
  int S = i_[11] - '0';
  char Rd[5]; Rd[4] = '\0';
  char Ra[5]; Ra[4] = '\0';
  char Rm[5]; Rm[4] = '\0';
  char Rn[5]; Rn[4] = '\0';
  for(int i = 0; i < 4; i++) {
    Rd[i] = i_[12+i];
    Ra[i] = i_[16+i];
    Rm[i] = i_[20+i];
    Rn[i] = i_[28+i];
  }
  printf("opcode = %s\n condition = %s\n Rd = %s\n Ra = %s\n Rm = %s\n Rn = %s\n", d_opcode, d_cond, Rd, Ra, Rm, Rn);
  /* function passes */
  // if(!strcmp(d_opcode, "000")) {
  //   printf("--- This is a MUL instruction. \n");
  //   MUL(Rd, Rn, Rm);
  //   return 0;
  // } else if(!strcmp(d_opcode, "001")) {
  //   printf("--- This is an MLA instruction. \n");
  //   MLA(Rd, Rn, Rm, Ra);
  //   return 0;
  // } else if(!strcmp(d_opcode, "100")){
  //   printf("--- This is a UMULL instruction. \n");
  //   UMULL(Rd, Rn, Rm, Ra);
  //   return 0;
  // } else if(!strcmp(d_opcode, "101")){
  //   printf("--- This is a UMLAL instruction. \n");
  //   UMLAL(Rd, Rn, Rm, Ra);
  //   return 0;
  // } else if(!strcmp(d_opcode, "110")){
  //   SMULL(Rd, Rn, Rm, Ra);
  //   return 0;
  // } else if(!strcmp(d_opcode, "111")){
  //   SMLAL(Rd, Rn, Rm, Ra);
  //   return 0;
  // } 
  return 1;
}

int transfer_process(char* i_) {

  /* This function execute memory instruction */ 
  char d_cond[5];
  d_cond[0] = i_[0];
  d_cond[1] = i_[1];
  d_cond[2] = i_[2];
  d_cond[3] = i_[3];
  d_cond[4] = '\0';
  int I, P, U, B, W, L;
  I = i_[6] - '0'; P = i_[7] - '0';
  U = i_[8] - '0'; B = i_[9] - '0';
  W = i_[10] - '0'; L = i_[11] - '0';
  char rn[5]; char rd[5];
  rn[4] = '\0'; rd[4] = '\0';
  for(int i = 0; i < 4; i++) {
    rn[i] = i_[12+i]; rd[i] = i_[16+i];
  }
  char src2[13];
  src2[12] = '\0';
  for(int i = 0; i < 12; i++) {
    src2[i] = i_[20+i];
  }
  char shamt5[6]; char sh[3]; char rm[5];
  printf("I = %d\n", I);
  if(I) {
    shamt5[5] = '\0'; sh[2] = '\0'; rm[4] = '\0';
    for(int i = 0; i < 6; i++) {
      if(i<2) 
        sh[i] = src2[i+5];
      if(i<4)
        rm[i] = src2[i+8];
      shamt5[i] = src2[i];
    }
    printf("shamt5 = %s\n sh = %s\n Rm = %s\n", shamt5, sh, rm);
  } else {
    printf("imm12 = %s\n", src2);
  }
  /* Add memory instructions here */ 
  if(!B && !L)
    STR(rd, rn, src2, I);
  else if(!B && L)
    LDR(rd, rn, src2, I);
  else if(B && !L)
    STRB(rd, rn, src2, I);
  else if(B && L)
    LDRB(rd, rn, src2, I);
  return 1;

}

int interruption_process(char* i_) {

  SWI();
  RUN_BIT = 0;
  return 0;

}

unsigned int COND(unsigned int i_word) {

  return (i_word>>28);

}

unsigned int OPCODE(unsigned int i_word) {

  return ((i_word<<7)>>27);

}


int decode_and_execute(char* i_) {

  /* 
     This function decode the instruction and update 
     CPU_State (NEXT_STATE)
  */

  if((i_[4] == '1') && (i_[5] == '0') && (i_[6] == '1')) {
    printf("- This is a Branch Instruction. \n");
    branch_process(i_);
  }
  if((i_[4] == '0') && (i_[5] == '0') && (i_[6] == '0') && (i_[7] == '0') && (i_[24] == '1') && (i_[25] == '0') && (i_[26] == '0') && (i_[27] == '1')) {
    printf("- This is a Multiply Instruction. \n");
    mul_process(i_);
  }
  else {
    printf("- This is a Data Processing Instruction. \n");
    data_process(i_);
  }
  if((i_[4] == '0') && (i_[5] == '1')) {
    printf("- This is a Single Data Transfer Instruction. \n");
    transfer_process(i_);
  }
  if((i_[4] == '1') && (i_[5] == '1') && (i_[6] == '1') && (i_[7] == '1')) {
    printf("- This is a Software Interruption Instruction. \n");
    interruption_process(i_);
  }
  return 0;

}

void process_instruction() {

  /* 
     execute one instruction here. You should use CURRENT_STATE and modify
     values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     access memory. 
  */   

  unsigned int inst_word = mem_read_32(CURRENT_STATE.PC);
  printf("The instruction is: %x \n", inst_word);
  printf("33222222222211111111110000000000\n");
  printf("10987654321098765432109876543210\n");
  printf("--------------------------------\n");
  printf("%s \n", byte_to_binary32(inst_word));
  printf("\n");
  decode_and_execute(byte_to_binary32(inst_word));

  NEXT_STATE.PC += 4;

}
