module stimulus();
	logic clk;
	logic we3;
	logic [31:0] wd3, rd1, rd2;
	logic [4:0] wa3, ra1, ra2;

	// Instantiate DUT
    regfile dut(.clk(clk),
				.we3(we3),
				.wd3(wd3),
				.rd1(rd1),
				.rd2(rd2),
				.wa3(wa3),
				.ra1(ra1),
				.ra2(ra2));

	// Setup the clock to toggle every 1 time units 
    initial 
      begin  
        clk = 1'b1;
        forever #5 clk = ~clk;
      end
	
	initial
	begin
		// set registers and vals
		//initialize data to zero
		#0 	wd3 = 32'b110; 	// set write data to 6_10
		#0 	wa3 = 5'b01; 	// set address to 1_10
		#0 	ra1 = 5'b01; 	// set read1 addr to 1_10
		#0 	ra2 = 5'b0; 		// set read2 addr ro 0_10
		#0 	rd1 = 32'b0; 	// init read busses . . .
		#0 	rd2 = 32'b0; 	// . . .
		#10 we3 = 1'b1;  								// set enable high
		#15 we3 = 1'b0; 								// set enable low
		#20 wd3 = 32'b111;	//set write data 7_10
		#30 wd3 = 32'b1000;	//set write data to 8_10
		#0 	we3 = 1'b1; 									// set enable high
		#5 	wa3 = ra2; 		// set write to addr 0_10 
		#10 we3 = 1'b0; 								// set enable low
		#15 ra1 = 1'b0; 	//READ ZERO BLOCK
		#20 wd3 = 32'b101;
		#20 wa3 = 1'b11;
		#20 ra1 = wa3;
		#20 ra2 = wa3;
		#20 we3 = 1'b1;
		
	end
		

endmodule // regfile_tb.sv