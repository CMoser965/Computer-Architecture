@ memfile.s
@ david_harris@hmc.edu and sarah.harris@unlv.edu 20 Jan 2014
@ Test ARM processor
@ ADD, SUB, AND, ORR, LDR, STR, B
@ If successful, it should write the value 7 to address 100

MAIN:	SUB R0, R15, R15 	@ R0 = 0				
	ADD R2, R0, #5      	@ R2 = 5             
	ADD R3, R0, #12    	@ R3 = 12            
	SUB R7, R3, #9    	@ R7 = 3             
	ORR R4, R7, R2    	@ R4 = 3 OR 5 = 7              	  		
        AND R5, R3, R4    	@ R5 = 12 AND 7 = 4            	
	ADD R5, R5, R4    	@ R5 = 4 + 7 = 11              			
        SUBS R8, R5, R7    	@ R8 <= 11 - 3 = 8, set Flags   	  		
        BEQ END        		@ shouldn't be taken            	  		
        SUBS R8, R3, R4    	@ R8 = 12 - 7  = 5             			
        BGE AROUND       	@ should be taken               
	ADD R5, R0, #0     	@ should be skipped             	
AROUND:	
	SUBS R8, R7, R2   	@ R8 = 3 - 5 = -2, set Flags   	         	
        ADDLT R7, R5, #1  	@ R7 = 11 + 1 = 12				          	
        SUB R7, R7, R2    	@ R7 = 12 - 5 = 7				
    	STR R7, [R3, #84]  	@ mem[12+84] = 7		     	
	LDR R2, [R0, #96]  	@ R2 = mem[96] = 7				
	ADD R15, R15, R0	@ PC <- PC + 8 (skips next)     	         
	ADD R2, R0, #14    	@ shouldn't happen              	
	B END             	@ always taken					
	ADD R2, R0, #13   	@ shouldn't happen				
  	ADD R2, R0, #10		@ shouldn't happen			  
END:	STR R2, [R0, #100] 	@ mem[100] = 7                  	

