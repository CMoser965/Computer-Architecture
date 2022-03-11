// tb.sv
// David_Harris@hmc.edu and Sarah_Harris@hmc.edu 25 June 2013
// Single-cycle implementation of a subset of ARMv4
//
// This version has been modified by:
//     Dr. James Stine (james.stine@okstate.edu)
//     Alex Underwood  (alexander.underwood@okstate.edu)
// for formatting and additional functionality.

module testbench();

   logic        clk;
   logic        reset;

   logic [31:0] WriteData, DataAdr;
   logic        MemWrite;

   // instantiate device to be tested
   top dut (clk, reset, WriteData, DataAdr, MemWrite);
   
   // initialize test
   initial
     begin
    reset <= 1; # 22; reset <= 0;
     end

   // generate clock to sequence tests
   always
     begin
    clk <= 1; # 5; clk <= 0; # 5;
     end

endmodule // testbench
