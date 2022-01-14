module regfile(input  logic        clk, //clock 
               input  logic        we3, //write enable signal
               input  logic [4:0]  ra1, ra2, wa3, //source reg # read port x2, destination regwrite port 
               input  logic [31:0] wd3, //data port for write
               output logic [31:0] rd1, rd2); //data port for read

  logic [31:0]     rf[31:0];

  // three ported register file
  // read two ports combinationally
  // write third port on rising edge of clock
  // register 0 hardwired to 0
  
  //if we3 is enabled -> writes synchronously on rising edge of clk
  
  
  //


endmodule // regfile
