/*
 * Top level module for the arm_single processor simulation.
 *
 * Modeled to be like the version for the FPGA so the arm module can be drop-in
 * swapped into the Vivado project and things should "just werk".
 */

module top (input  logic        clk, reset, 
            output logic [31:0] WriteData, DataAdr, 
            output logic        MemWrite);

   logic [31:0] PC, Instr, ReadData;
   logic        PCReady, MStrobe;
   
   // instantiate processor and memories
   arm arm (.clk(clk),
            .reset(reset),
            .PC(PC),
            .Instr(Instr),
            .MemWrite(MemWrite),
            .ALUResult(DataAdr), 
            .WriteData(WriteData),
            .ReadData(ReadData),
            .MemStrobe(MStrobe),
            .PCReady(PCReady));

   imem imem (.mem_addr(PC),
              .mem_out(Instr));
   dmem dmem (.mem_out(ReadData),
              .r_w(MemWrite),
              .clk(clk),
              .mem_addr(DataAdr),
              .mem_data(WriteData),
              .MStrobe(MStrobe),
              .PCReady(PCReady));
   
endmodule // top
