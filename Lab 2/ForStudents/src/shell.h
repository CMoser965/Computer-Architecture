/***************************************************************/
/*                                                             */
/*   ARMv8-32 Instruction Level Simulator                      */
/*                                                             */
/*   ECEN 4243                                                 */
/*   Oklahoma State University                                 */
/*                                                             */
/***************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
/*          DO NOT MODIFY THIS FILE!                            */
/*          You should only change sim.c!                       */
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#ifndef _SIM_SHELL_H_
#define _SIM_SHELL_H_

#include <stdint.h>

#define FALSE 0
#define TRUE  1

#define ARM_REGS 16
#define PC REGS[15]

typedef struct CPU_State_Struct {

  uint32_t REGS[ARM_REGS]; /* register file. */
  uint32_t CPSR; /* current program status register */
} CPU_State;

extern CPU_State CURRENT_STATE, NEXT_STATE;
extern int RUN_BIT;	/* run bit */

uint32_t mem_read_32 (uint32_t address);
void     mem_write_32 (uint32_t address, uint32_t value);
void process_instruction ();

#endif
