// arm_pipelined.sv
// David_Harris@hmc.edu, Sarah.Harris@unlv.edu 4 January 2014
// Pipelined implementation of a subset of ARMv4
//
// This version has been modified by:
//     Dr. James Stine (james.stine@okstate.edu)
//     Alex Underwood  (alexander.underwood@okstate.edu)
// for formatting and additional functionality.

// 16 32-bit registers
// Data-processing instructions
//   ADD, SUB, AND, ORR
//   OP <Cond> <S> <Rd>, <Rn>, #immediate
//   OP <Cond> <S> <Rd>, <Rn>, <Rm>
//    Rd <- <Rn> OP <Rm>        if (S) Update Status Flags
//    Rd <- <Rn> OP immediate   if (S) Update Status Flags
//   Instr[31:28] = Cond
//   Instr[27:26] = Op = 00
//   Instr[25:20] = Funct
//                  [25]:    1 for immediate, 0 for register
//                  [24:21]: 0100 (ADD) / 0010 (SUB) /
//                           0000 (AND) / 1100 (ORR)
//                  [20]:    S (1 = update CPSR status Flags)
//   Instr[19:16] = Rn
//   Instr[15:12] = Rd
//   Instr[11:8]  = 0000
//   Instr[7:0]   = immed_8  (for #immediate type) / 
//                  0000<Rm> (for register type)
//   
// Load/Store instructions
//   LDR, STR
//   OP <Rd>, <Rn>, #offset
//    LDR: Rd <- Mem[<Rn>+offset]
//    STR: Mem[<Rn>+offset] <- Rd
//   Instr[31:28] = Cond
//   Instr[27:26] = Op = 01 
//   Instr[25:20] = Funct
//                  [25]:    0 (A)
//                  [24:21]: 1100 (P/U/B/W)
//                  [20]:    L (1 for LDR, 0 for STR)
//   Instr[19:16] = Rn
//   Instr[15:12] = Rd
//   Instr[11:0]  = imm (zero extended)
//
// Branch instruction (PC <= PC + offset, PC holds 8 bytes past Branch Instr)
//   B
//   OP <target>
//    PC <- PC + 8 + imm << 2
//   Instr[31:28] = Cond
//   Instr[27:25] = Op = 10
//   Instr[25:24] = Funct
//                  [25]: 1 (Branch)
//                  [24]: 0 (link)
//   Instr[23:0]  = offset (sign extend, shift left 2)
//   Note: no Branch delay slot on ARM
//
// Other:
//   R15 reads as PC+8
//   Conditional Encoding
//    Cond  Meaning                       Flag
//    0000  Equal                         Z = 1
//    0001  Not Equal                     Z = 0
//    0010  Carry Set                     C = 1
//    0011  Carry Clear                   C = 0
//    0100  Minus                         N = 1
//    0101  Plus                          N = 0
//    0110  Overflow                      V = 1
//    0111  No Overflow                   V = 0
//    1000  Unsigned Higher               C = 1 & Z = 0
//    1001  Unsigned Lower/Same           C = 0 | Z = 1
//    1010  Signed greater/equal          N = V
//    1011  Signed less                   N != V
//    1100  Signed greater                N = V & Z = 0
//    1101  Signed less/equal             N != V | Z = 1
//    1110  Always                        any

module arm (input  logic        clk, reset,
            output logic [31:0] PCF,
            input  logic [31:0] InstrF,
            output logic        MemWriteM,
            output logic [31:0] ALUOutM, WriteDataM,
            input  logic [31:0] ReadDataM,
            output logic        MemStrobe,
            input  logic        PCReady);
   logic [31:0] instr;
   logic [2:0]  RegSrcD;
   logic [1:0]  ImmSrcD;
   logic [3:0]  ALUControlE;
   logic        ALUSrcE, BranchTakenE, MemtoRegW,
                PCSrcW, RegWriteW;
   logic [3:0]  ALUFlagsE;
   logic [31:0] InstrD;
   logic        RegWriteM, MemtoRegE, PCWrPendingF;
   logic [1:0]  ForwardAE, ForwardBE;
   logic        StallF, StallD, FlushD, FlushE;
   logic        Match_1E_M, Match_1E_W, 
                Match_2E_M, Match_2E_W, 
                Match_12D_E;
   
   controller c (.clk(clk),
                 .reset(reset),
                 .InstrD(InstrD[31:12]),
                 .ALUFlagsE(ALUFlagsE),
                 .RegSrcD(RegSrcD), 
                 .ImmSrcD(ImmSrcD), 
                 .ALUSrcE(ALUSrcE),
                 .BranchTakenE(BranchTakenE),
                 .ALUControlE(ALUControlE),
                 .MemWriteM(MemWriteM),
                 .MemtoRegW(MemtoRegW),
                 .PCSrcW(PCSrcW),
                 .RegWriteW(RegWriteW),
                 // hazard interface
                 .RegWriteM(RegWriteM),
                 .MemtoRegE(MemtoRegE),
                 .PCWrPendingF(PCWrPendingF),
                 .FlushE(FlushE),
                 .MemStrobeM(MemStrobe),
                 .MemSysReady(PCReady),
                 .I(I),
                 .compareOnly(compareOnly));
   datapath dp (.clk(clk),
                .reset(reset),
                .RegSrcD(RegSrcD),
                .ImmSrcD(ImmSrcD),
                .ALUSrcE(ALUSrcE),
                .BranchTakenE(BranchTakenE),
                .ALUControlE(ALUControlE), 
                .MemtoRegW(MemtoRegW),
                .PCSrcW(PCSrcW),
                .RegWriteW(RegWriteW),
                .PCF(PCF),
                .InstrF(InstrF),
                .InstrD(InstrD),
                .ALUOutM(ALUOutM),
                .WriteDataM(WriteDataM),
                .ReadDataM(ReadDataM),
                .ALUFlagsE(ALUFlagsE),
                // hazard logic
                .Match_1E_M(Match_1E_M),
                .Match_1E_W(Match_1E_W), 
                .Match_2E_M(Match_2E_M),
                .Match_2E_W(Match_2E_W),
                .Match_12D_E(Match_12D_E),
                .ForwardAE(ForwardAE),
                .ForwardBE(ForwardBE),
                .StallF(StallF),
                .StallD(StallD),
                .FlushD(FlushD),
                .MemSysReady(PCReady),
                .I(I),
                .instr(instr),
                .compareOnly(compareOnly));
   hazard h (.clk(clk),
             .reset(reset),
             .Match_1E_M(Match_1E_M),
             .Match_1E_W(Match_1E_W), 
             .Match_2E_M(Match_2E_M),
             .Match_2E_W(Match_2E_W),
             .Match_12D_E(Match_12D_E),
             .RegWriteM(RegWriteM),
             .RegWriteW(RegWriteW),
             .BranchTakenE(BranchTakenE),
             .MemtoRegE(MemtoRegE),
             .PCWrPendingF(PCWrPendingF),
             .PCSrcW(PCSrcW),
             .ForwardAE(ForwardAE),
             .ForwardBE(ForwardBE),
             .StallF(StallF),
             .StallD(StallD),
             .FlushD(FlushD),
             .FlushE(FlushE));
   
endmodule // arm

module controller (input  logic         clk, reset,
                   input  logic [31:12] InstrD,
                   input  logic [3:0]   ALUFlagsE,
                   output logic [2:0]   RegSrcD, 
                   output logic [1:0]   ImmSrcD, 
                   output logic         ALUSrcE, BranchTakenE,
                   output logic [3:0]   ALUControlE,
                   output logic         MemWriteM,
                   output logic         MemtoRegW, PCSrcW, RegWriteW,
                   // hazard interface
                   output logic         RegWriteM, MemtoRegE,
                   output logic         PCWrPendingF,
                   input  logic         FlushE,
                   output logic         MemStrobeM,
                   input  logic         MemSysReady,
                   output logic         I,
                   input  logic        compareOnly);

   logic [11:0] controlsD;
   logic        CondExE, ALUOpD;
   logic [3:0]  ALUControlD;
   logic        ALUSrcD;
   logic        MemtoRegD, MemtoRegM;
   logic        RegWriteD, RegWriteE, RegWriteGatedE;
   logic        MemWriteD, MemWriteE, MemWriteGatedE;
   logic        BranchD, BranchE;
   logic [1:0]  FlagWriteD, FlagWriteE;
   logic        PCSrcD, PCSrcE, PCSrcM;
   logic [3:0]  FlagsE, FlagsNextE, CondE;
   logic        MemStrobeD, MemStrobeE, MemStrobeGatedE;

   // Decode stage   
   always_comb
     casex(InstrD[27:26])
       2'b00: if (InstrD[25]) controlsD = 12'b0000_0101_0010; // DP imm
              else            controlsD = 12'b0000_0001_0010; // DP reg
       2'b01: if (InstrD[20]) controlsD = 12'b0000_1111_0001; // LDR
              else            controlsD = 12'b0100_1110_1001; // STR
       2'b10: if (InstrD[24]) controlsD = 12'b1011_0101_0100; // BL
              else            controlsD = 12'b0011_0100_0100; // B
       default:               controlsD = 12'bx;             // unimplemented
     endcase

   // bits:  3,2,1,1,1,1,1,1,1 = 12
   assign {RegSrcD, ImmSrcD, ALUSrcD, MemtoRegD, 
           RegWriteD, MemWriteD, BranchD, ALUOpD, MemStrobeD} = controlsD; 
   
  //  always_comb
  //    if (ALUOpD) 
  //      begin                 // which Data-processing Instr?
  //        case(InstrD[24:21]) 
  //          4'b0100: ALUControlD = 2'b00; // ADD
  //          4'b0010: ALUControlD = 2'b01; // SUB
  //          4'b0000: ALUControlD = 2'b10; // AND
  //          4'b1100: ALUControlD = 2'b11; // ORR
  //          default: ALUControlD = 2'bx;  // unimplemented
  //        endcase
  //        FlagWriteD[1]   = InstrD[20];   // update N/Z Flags if S bit is set
  //        FlagWriteD[0]   = InstrD[20] &
  //                          (ALUControlD == 2'b00 | ALUControlD == 2'b01);
  //      end 
  //    else 
  //      begin
  //        ALUControlD     = 2'b00;        // perform addition for non-dp instr
  //        FlagWriteD      = 2'b00;        // don't update Flags
  //      end
  always_comb
     if (ALUOpD)
       begin                 // which DP Instr?
         case(InstrD[24:21]) 
           4'b0100: ALUControlD = 4'b0000; // ADD  0
           4'b0010: ALUControlD = 4'b0001; // SUB  1
           4'b0101: ALUControlD = 4'b0010; // ADC  2
           4'b0110: ALUControlD = 4'b0011; // SBC  3
           4'b0000: ALUControlD = 4'b0100; // AND  4
           4'b1101: ALUControlD = 4'b0101; // MOV & ASR & LSL & LSR & ROR  5
           4'b1110: ALUControlD = 4'b0110; // BIC  6
           4'b1011: ALUControlD = 4'b0111; // CMN  7
           4'b1010: ALUControlD = 4'b1000; // CMP  8
           4'b0001: ALUControlD = 4'b1001; // EOR  9
           4'b1111: ALUControlD = 4'b1010; // MVN  10
           4'b1100: ALUControlD = 4'b1011; // ORR  11
           4'b1001: ALUControlD = 4'b1100; // TEQ  12
           4'b1000: ALUControlD = 4'b1101; // TST  13
           default: ALUControlD = 4'bx;  // unimplemented
         endcase
         // update flags if S bit is set 
         // (C & V only updated for arith instructions)
         //FlagW -> [1:0] -> flagW1 = 1 NZ flags saved -> FlagW0 = 1 CV Flags saved
         FlagWriteD[1]      = InstrD[20]; // FlagW[1] = S-bit
         // FlagW[0] = S-bit & (ADD | SUB | ADC | SBC | CMP | CMN | MOV  | AND | ORR | EOR | BIC | TEQ | TST)
         FlagWriteD[0]      = InstrD[20]; // Only implemeted 
       end
     else
       begin
         ALUControlD = 4'b0000; // add for non-DP instructions
         FlagWriteD  = 2'b00; // don't update Flags
       end

   assign PCSrcD = (((InstrD[15:12] == 4'b1111) & RegWriteD) | BranchD);
   assign I         = InstrD[25];
   
   // Execute stage
   flopenrc #(8) flushedregsE(.clk(clk),
                            .reset(reset),
                            .en(MemSysReady),
                            .clear(FlushE), 
                            .d({FlagWriteD, BranchD, MemWriteD, 
                                RegWriteD, PCSrcD, MemtoRegD, MemStrobeD}),
                            .q({FlagWriteE, BranchE, MemWriteE, 
                                RegWriteE, PCSrcE, MemtoRegE, MemStrobeE}));
   flopenr #(5)  regsE(.clk(clk),
                     .reset(reset),
                     .en(MemSysReady),
                     .d({ALUSrcD, ALUControlD}),
                     .q({ALUSrcE, ALUControlE}));
   
   flopenr  #(4) condregE(.clk(clk),
                        .reset(reset),
                        .en(MemSysReady),
                        .d(InstrD[31:28]),
                        .q(CondE));
   flopenr  #(4) flagsreg(.clk(clk),
                        .reset(reset),
                        .en(MemSysReady),
                        .d(FlagsNextE),
                        .q(FlagsE));

   // write and Branch controls are conditional
   conditional Cond (.Cond(CondE),
                     .Flags(FlagsE),
                     .ALUFlags(ALUFlagsE),
                     .FlagsWrite(FlagWriteE), 
                     .CondEx(CondExE),
                     .FlagsNext(FlagsNextE));
   assign BranchTakenE    = BranchE & CondExE;
   assign RegWriteGatedE  = RegWriteE & CondExE & ~compareOnly;
   assign MemWriteGatedE  = MemWriteE & CondExE;
   assign PCSrcGatedE     = PCSrcE & CondExE;
   assign MemStrobeGatedE = MemStrobeE & CondExE;
   
   // Memory stage
   flopenr #(5) regsM(.clk(clk),
                    .reset(reset),
                    .en(MemSysReady),
                    .d({MemWriteGatedE, MemtoRegE, RegWriteGatedE, PCSrcGatedE,
                        MemStrobeGatedE}),
                    .q({MemWriteM, MemtoRegM, RegWriteM, PCSrcM,
                        MemStrobeM}));
   
   // Writeback stage
   flopenr #(3) regsW(.clk(clk),
                    .reset(reset),
                    .en(MemSysReady),
                    .d({MemtoRegM, RegWriteM, PCSrcM}),
                    .q({MemtoRegW, RegWriteW, PCSrcW}));
   
   // Hazard Prediction
   assign PCWrPendingF = PCSrcD | PCSrcE | PCSrcM;

endmodule // controller

module conditional (input  logic [3:0] Cond,
                    input  logic [3:0] Flags,
                    input  logic [3:0] ALUFlags,
                    input  logic [1:0] FlagsWrite,
                    output logic       CondEx,
                    output logic [3:0] FlagsNext);
   
   logic                   neg, zero, carry, overflow, ge;
   
   assign {neg, zero, carry, overflow} = Flags;
   assign ge = (neg == overflow);
   
   always_comb
     case(Cond)
       4'b0000: CondEx = zero;             // EQ
       4'b0001: CondEx = ~zero;            // NE
       4'b0010: CondEx = carry;            // CS
       4'b0011: CondEx = ~carry;           // CC
       4'b0100: CondEx = neg;              // MI
       4'b0101: CondEx = ~neg;             // PL
       4'b0110: CondEx = overflow;         // VS
       4'b0111: CondEx = ~overflow;        // VC
       4'b1000: CondEx = carry & ~zero;    // HI
       4'b1001: CondEx = ~(carry & ~zero); // LS
       4'b1010: CondEx = ge;               // GE
       4'b1011: CondEx = ~ge;              // LT
       4'b1100: CondEx = ~zero & ge;       // GT
       4'b1101: CondEx = ~(~zero & ge);    // LE
       4'b1110: CondEx = 1'b1;             // Always
       default: CondEx = 1'bx;             // undefined
     endcase
   
   assign FlagsNext[3:2] = (FlagsWrite[1] & CondEx) ? 
                            ALUFlags[3:2] : Flags[3:2];
   assign FlagsNext[1:0] = (FlagsWrite[0] & CondEx) ? 
                            ALUFlags[1:0] : Flags[1:0];

endmodule // conditional

module datapath (input  logic        clk, reset,
                 input  logic [2:0]  RegSrcD,
                 input  logic [1:0]  ImmSrcD,
                 input  logic        ALUSrcE, BranchTakenE,
                 input  logic [3:0]  ALUControlE, 
                 input  logic        MemtoRegW, PCSrcW, RegWriteW,
                 output logic [31:0] PCF,
                 input  logic [31:0] InstrF,
                 output logic [31:0] InstrD,
                 output logic [31:0] ALUOutM, WriteDataM,
                 input  logic [31:0] ReadDataM,
                 output logic [3:0]  ALUFlagsE,
                 // hazard logic
                 output logic        Match_1E_M, Match_1E_W, 
                 output logic        Match_2E_M, Match_2E_W, Match_12D_E,
                 input  logic [1:0]  ForwardAE, ForwardBE,
                 input  logic        StallF, StallD, FlushD,
                 input  logic        MemSysReady,
                 input logic         I,
                 input logic  [31:0] instr,
                 output logic         compareOnly);
   
   logic [31:0] PCPlus4F, PCnext1F, PCnextF;
   logic [31:0] PCPlus4D, PCPlus4E, PCPlus4M, PCPlus4W;   
   logic [31:0] ExtImmD, rd1D, rd2D, PCPlus8D;
   logic [31:0] rd1E, rd2E, ExtImmE, SrcAE, SrcBE;
   logic [31:0] WriteDataE, ALUResultE;
   logic [31:0] ReadDataW, ALUOutW, ResultW;
   logic [3:0]  RA1D, RA2D, RA3D, RA1E, RA2E;
   logic [31:0] RA4D;   
   logic [3:0]  WA3E, WA3M, WA3W;
   logic        Match_1D_E, Match_2D_E;
   logic [2:0]  RegSrcE, RegSrcM, RegSrcW;
      
   // Fetch stage
   mux2 #(32) pcnextmux (.d0(PCPlus4F),
                         .d1(ResultW),
                         .s(PCSrcW),
                         .y(PCnext1F));
   mux2 #(32) branchmux (.d0(PCnext1F),
                         .d1(ALUResultE),
                         .s(BranchTakenE),
                         .y(PCnextF));
   flopenr #(32) pcreg (.clk(clk),
                        .reset(reset),
                        .en(~StallF & MemSysReady),
                        .d(PCnextF),
                        .q(PCF));
   adder #(32) pcadd (.a(PCF),
                      .b(32'h4),
                      .y(PCPlus4F));
   
   // Decode Stage
   assign PCPlus8D = PCPlus4F; // skip register
   flopenrc #(32) instrreg (.clk(clk),
                            .reset(reset),
                            .en(~StallD & MemSysReady),
                            .clear(FlushD),
                            .d(InstrF),
                            .q(InstrD));
   flopenrc #(32) pcadd4d (.clk(clk),
                           .reset(reset),
                           .en(~StallD & MemSysReady),
                           .clear(FlushD),
                           .d(PCPlus4F),
                           .q(PCPlus4D));
   mux2 #(4)   ra1mux (.d0(InstrD[19:16]),
                       .d1(4'b1111),
                       .s(RegSrcD[0]),
                       .y(RA1D));
   mux2 #(4)   ra2mux (.d0(InstrD[3:0]),
                       .d1(InstrD[15:12]),
                       .s(RegSrcD[1]),
                       .y(RA2D));
   mux2 #(4)   ra3mux (.d0(WA3W),
                       .d1(4'hE),
                       .s(RegSrcW[2]),
                       .y(RA3D));
   mux2 #(32)  ra4mux (.d0(ResultW),
                       .d1(PCPlus4W),
                       .s(RegSrcW[2]),
                       .y(RA4D));   
   regfile     rf (.clk(clk),
                   .we3(RegWriteW),
                   .ra1(RA1D),
                   .ra2(RA2D),
                   .wa3(RA3D),
                   .wd3(RA4D),
                   .r15(PCPlus8D), 
                   .rd1(rd1D),
                   .rd2(rd2D)); 
   extend      ext (.Instr(InstrD[23:0]),
                    .ImmSrc(ImmSrcD),
                    .ExtImm(ExtImmD));
   
   // Execute Stage
   flopenr #(32) rd1reg (.clk(clk),
                       .reset(reset),
                       .en(MemSysReady),
                       .d(rd1D),
                       .q(rd1E));
   flopenr #(32) rd2reg (.clk(clk),
                       .reset(reset),
                       .en(MemSysReady),
                       .d(rd2D),
                       .q(rd2E));
   flopenr #(32) immreg (.clk(clk),
                       .reset(reset),
                       .en(MemSysReady),
                       .d(ExtImmD),
                       .q(ExtImmE));
   flopenr #(4)  wa3ereg (.clk(clk),
                        .reset(reset),
                        .en(MemSysReady),
                        .d(InstrD[15:12]),
                        .q(WA3E));
   flopenr #(4)  ra1reg (.clk(clk),
                       .reset(reset),
                       .en(MemSysReady),
                       .d(RA1D),
                       .q(RA1E));
   flopenr #(4)  ra2reg (.clk(clk),
                       .reset(reset),
                       .en(MemSysReady),
                       .d(RA2D),
                       .q(RA2E));
   flopenr #(32) pcadd4e (.clk(clk),
                        .reset(reset),
                        .en(MemSysReady),
                        .d(PCPlus4D),
                        .q(PCPlus4E));
   flopenr #(3)  regsrce (.clk(clk),
                        .reset(reset),
                        .en(MemSysReady),
                        .d(RegSrcD),
                        .q(RegSrcE));
   mux3 #(32)  byp1mux (.d0(rd1E),
                        .d1(ResultW),
                        .d2(ALUOutM),
                        .s(ForwardAE),
                        .y(SrcAE));
   mux3 #(32)  byp2mux (.d0(rd2E),
                        .d1(ResultW),
                        .d2(ALUOutM),
                        .s(ForwardBE),
                        .y(WriteDataE));
   mux2 #(32)  srcbmux (.d0(WriteDataE),
                        .d1(ExtImmE),
                        .s(ALUSrcE),
                        .y(SrcBE));
   alu         alu (.a(SrcAE),
                    .b(SrcBE),
                    .ALUControl(ALUControlE),
                    .Result(ALUResultE),
                    .Flags(ALUFlagsE),
                    .I(I),
                    .instr(instr),
                    .compareOnly(compareOnly));
   
   // Memory Stage
   flopenr #(32) aluresreg (.clk(clk),
                            .reset(reset),
                            .en(MemSysReady),
                            .d(ALUResultE),
                            .q(ALUOutM));
   flopenr #(32) wdreg (.clk(clk),
                        .reset(reset),
                        .en(MemSysReady),
                        .d(WriteDataE),
                        .q(WriteDataM));
   flopenr #(4)  wa3mreg (.clk(clk),
                          .reset(reset),
                          .en(MemSysReady),
                          .d(WA3E),
                          .q(WA3M));
   flopenr #(32) pcadd4m (.clk(clk),
                          .reset(reset),
                          .en(MemSysReady),
                          .d(PCPlus4E),
                          .q(PCPlus4M));
   flopenr #(3)  regsrcm (.clk(clk),
                          .reset(reset),
                          .en(MemSysReady),
                          .d(RegSrcE),
                          .q(RegSrcM));
   
   // Writeback Stage
   flopenr #(32) aluoutreg (.clk(clk),
                            .reset(reset),
                            .en(MemSysReady),
                            .d(ALUOutM),
                            .q(ALUOutW));
   flopenr #(32) rdreg (.clk(clk),
                        .reset(reset),
                        .en(MemSysReady),
                        .d(ReadDataM),
                        .q(ReadDataW));
   flopenr #(4)  wa3wreg (.clk(clk),
                          .reset(reset),
                          .en(MemSysReady),
                          .d(WA3M),
                          .q(WA3W));
   flopenr #(32) pcadd4w (.clk(clk),
                          .reset(reset),
                          .en(MemSysReady),
                          .d(PCPlus4M),
                          .q(PCPlus4W));
   flopenr #(3)  regsrcw (.clk(clk),
                          .reset(reset),
                          .en(MemSysReady),
                          .d(RegSrcM),
                          .q(RegSrcW));
   mux2 #(32)  resmux (.d0(ALUOutW),
                       .d1(ReadDataW),
                       .s(MemtoRegW),
                       .y(ResultW));
   
   // hazard comparison
   eqcmp #(4) m0 (.a(WA3M),
                  .b(RA1E),
                  .y(Match_1E_M));
   eqcmp #(4) m1 (.a(WA3W),
                  .b(RA1E),
                  .y(Match_1E_W));
   eqcmp #(4) m2 (.a(WA3M),
                  .b(RA2E),
                  .y(Match_2E_M));
   eqcmp #(4) m3 (.a(WA3W),
                  .b(RA2E),
                  .y(Match_2E_W));
   eqcmp #(4) m4a (.a(WA3E),
                   .b(RA1D),
                   .y(Match_1D_E));
   eqcmp #(4) m4b (.a(WA3E),
                   .b(RA2D),
                   .y(Match_2D_E));
   assign Match_12D_E = Match_1D_E | Match_2D_E;
   
endmodule // datapath

module hazard (input  logic       clk, reset,
               input  logic       Match_1E_M, Match_1E_W, 
               input  logic       Match_2E_M, Match_2E_W, Match_12D_E,
               input  logic       RegWriteM, RegWriteW,
               input  logic       BranchTakenE, MemtoRegE,
               input  logic       PCWrPendingF, PCSrcW,
               output logic [1:0] ForwardAE, ForwardBE,
               output logic       StallF, StallD,
               output logic       FlushD, FlushE);

   logic ldrStallD;

   // forwarding logic
   always_comb begin
      if (Match_1E_M & RegWriteM)      ForwardAE = 2'b10;
      else if (Match_1E_W & RegWriteW) ForwardAE = 2'b01;
      else                             ForwardAE = 2'b00;
      
      if (Match_2E_M & RegWriteM)      ForwardBE = 2'b10;
      else if (Match_2E_W & RegWriteW) ForwardBE = 2'b01;
      else                             ForwardBE = 2'b00;
   end
   
   // stalls and flushes
   // Load RAW
   //   when an instruction reads a register loaded by the previous,
   //   stall in the decode stage until it is ready
   // Branch hazard
   //   When a branch is taken, flush the incorrectly fetched instrs
   //   from decode and execute stages
   // PC Write Hazard
   //   When the PC might be written, stall all following instructions
   //   by stalling the fetch and flushing the decode stage
   // when a stage stalls, stall all previous and flush next
   
   assign ldrStallD = Match_12D_E & MemtoRegE;
   
   assign StallD = ldrStallD;
   assign StallF = ldrStallD | PCWrPendingF; 
   assign FlushE = ldrStallD | BranchTakenE; 
   assign FlushD = PCWrPendingF | PCSrcW | BranchTakenE;
   
endmodule // hazard

module regfile (input  logic        clk, 
                input  logic        we3, 
                input  logic [3:0]  ra1, ra2, wa3, 
                input  logic [31:0] wd3, r15,
                output logic [31:0] rd1, rd2);
   
   logic [31:0] rf[14:0];

   // three ported register file
   // read two ports combinationally
   // write third port on falling edge of clock (midcycle)
   //   so that writes can be read on same cycle
   // register 15 reads PC+8 instead

   always_ff @(negedge clk)
     if (we3) rf[wa3] <= wd3;   

   assign rd1 = (ra1 == 4'b1111) ? r15 : rf[ra1];
   assign rd2 = (ra2 == 4'b1111) ? r15 : rf[ra2];

endmodule // regfile

module extend (input  logic [23:0] Instr,
               input  logic [1:0]  ImmSrc,
               output logic [31:0] ExtImm);
   
   always_comb
     case(ImmSrc) 
       2'b00:   ExtImm = {24'b0, Instr[7:0]};  // 8-bit unsigned immediate
       2'b01:   ExtImm = {20'b0, Instr[11:0]}; // 12-bit unsigned immediate 
       2'b10:   ExtImm = {{6{Instr[23]}}, Instr[23:0], 2'b00}; // Branch
       default: ExtImm = 32'bx; // undefined
     endcase             

endmodule // extend

module alu (input  logic [31:0] a, b,
            input  logic [3:0]  ALUControl,
            output logic [31:0] Result,
            output logic [3:0]  Flags,
            input  logic        I,
            input  logic [31:0] instr,
            output logic        compareOnly);

   logic        neg, zero, carry, overflow;
   logic [31:0] condinvb;
   logic [32:0] sum;

   //logic compareOnly;
   logic [32:0] fakeReg;
   
  //  assign condinvb = ALUControl[0] ? ~b : b;
  //  assign sum = a + condinvb + ALUControl[0];

  //  always_comb
  //    casex (ALUControl[1:0])
  //      2'b0?: Result = sum;
  //      2'b10: Result = a & b;
  //      2'b11: Result = a | b;
  //    endcase

  //  assign neg      = Result[31];
  //  assign zero     = (Result == 32'b0);
  //  assign carry    = (ALUControl[1] == 1'b0) & sum[32];
  //  assign overflow = (ALUControl[1] == 1'b0) & 
  //                    ~(a[31] ^ b[31] ^ ALUControl[0]) & 
  //                    (a[31] ^ sum[31]); 

    assign c = (ALUControl[1] & ~ALUControl[3] & ~ALUControl[2]) ? Flags[1] : 0; // set Carry if required for ADC/SBC case
    assign condinvb = (ALUControl[0] & ~ALUControl[3] & ~ALUControl[2]) ? ~b : b; // invert operand for sub case
    assign sum = a + condinvb + ALUControl[0] + c; // sum for all add/sub cases
  

   always_comb
   begin
   compareOnly = 0;
     casex (ALUControl[3:0])
       4'b00??:  Result = sum; // ADD & SUB & ADC & SBC ??
       //4'b0010:  Result = ; // ADC
       //4'b0011:  Result = ; // SBC
       4'b0100:  Result = a & b; // AND
       4'b0101:  begin // MOV & ASR & LSL & LSR & ROR
                      if(I) 
                        Result = b;
                      else begin
                        casex(b[6:5])
                          2'b00: Result = instr[3:0] << instr[11:7];
                          2'b01: Result = instr[3:0] >> instr[11:7];
                          2'b10: Result = instr[3:0] >>> instr[11:7];
                          2'b11: Result = (instr[3:0] >> instr[11:7]) | (instr[3:0] << (6'b100000 - instr[11:7]));
                          default: Result = 32'bx;
                        endcase
                      end
                 end
       4'b0110:  Result = a & ~b; // BIC
       4'b0111:  begin            // CMN
                      assign fakeReg = a + b; 
                      compareOnly = 1; 
                end
       4'b1000:  begin            // CMP
                      assign fakeReg = a - b; 
                      compareOnly = 1; 
                end 
       4'b1001:  Result = a ^ b; // EOR ??
       4'b1010:  Result = ~a; // MVN
       4'b1011:  Result = a | b; // ORR
       4'b1100:  begin // TEQ
                      assign fakeReg = a ^ b;
                      compareOnly = 1;
                 end
       4'b1101:  begin // TST
                      assign fakeReg = a & b;
                      compareOnly = 1;
                 end
       default: Result = 32'bx;
     endcase
   end

   assign neg      = compareOnly ? fakeReg[31] : Result[31];
   assign zero     = compareOnly ? (fakeReg == 32'b0) : (Result == 32'b0);
   assign carry    = compareOnly ? (ALUControl[1] == 1'b0) & fakeReg[32] : (ALUControl[1] == 1'b0) & sum[32];
   assign overflow = compareOnly ? (ALUControl[1] == 1'b0) & 
                     ~(a[31] ^ b[31] ^ ALUControl[0]) & 
                     (a[31] ^ fakeReg[31]) : (ALUControl[1] == 1'b0) & 
                     ~(a[31] ^ b[31] ^ ALUControl[0]) & 
                     (a[31] ^ sum[31]); 

   assign Flags = {neg, zero, carry, overflow};

endmodule // alu

module adder #(parameter WIDTH=8)
   (input  logic [WIDTH-1:0] a, b,
    output logic [WIDTH-1:0] y);
   
   assign y = a + b;

endmodule // adder

module flopenr #(parameter WIDTH = 8)
   (input  logic             clk, reset, en,
    input  logic [WIDTH-1:0] d, 
    output logic [WIDTH-1:0] q);

   always_ff @(posedge clk, posedge reset)
     if (reset)   q <= 0;
     else if (en) q <= d;

endmodule // flopenr

module flopr #(parameter WIDTH = 8)
   (input  logic             clk, reset,
    input  logic [WIDTH-1:0] d, 
    output logic [WIDTH-1:0] q);

   always_ff @(posedge clk, posedge reset)
     if (reset) q <= 0;
     else       q <= d;

endmodule // flopr

module flopenrc #(parameter WIDTH = 8)
   (input  logic             clk, reset, en, clear,
    input  logic [WIDTH-1:0] d, 
    output logic [WIDTH-1:0] q);

   always_ff @(posedge clk, posedge reset)
     if (reset)   q <= 0;
     else if (en) 
       if (clear) q <= 0;
       else       q <= d;

endmodule // flopenrc

module floprc #(parameter WIDTH = 8)
   (input  logic             clk, reset, clear,
    input  logic [WIDTH-1:0] d, 
    output logic [WIDTH-1:0] q);

   always_ff @(posedge clk, posedge reset)
     if (reset) q <= 0;
     else       
       if (clear) q <= 0;
       else       q <= d;

endmodule // floprc

module mux2 #(parameter WIDTH = 8)
   (input  logic [WIDTH-1:0] d0, d1, 
    input  logic             s, 
    output logic [WIDTH-1:0] y);

   assign y = s ? d1 : d0; 

endmodule // mux2

module mux3 #(parameter WIDTH = 8)
   (input  logic [WIDTH-1:0] d0, d1, d2,
    input  logic [1:0]       s, 
    output logic [WIDTH-1:0] y);

   assign y = s[1] ? d2 : (s[0] ? d1 : d0); 

endmodule // mux3

module eqcmp #(parameter WIDTH = 8)
   (input  logic [WIDTH-1:0] a, b,
    output logic             y);

   assign y = (a == b); 

endmodule // eqcmp

