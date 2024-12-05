// Gate Level Verilog Code Generated!
// GateLvl:20 GateNum:20 GateInputNum:2
// ****** Basic Gate Module Defination ******
module or2(out, in1, in2);
  output out;
  input in1, in2;
  wire in1, in2, out;
  assign out = in1 || in2;
endmodule

module and2(out, in1, in2);
  output out;
  input in1, in2;
  wire in1, in2, out;
  assign out = in1 && in2;
endmodule

module not1(out, in);
  output out;
  input in;
  wire in,out;
  assign out = ~in;
endmodule

module nand2(out, in1, in2);
  output out;
  input in1, in2;
  wire in1, in2, out;
  assign out = ~(in1 && in2);
endmodule
// ****** Basic Gate Module Defination End ******

// ****** Combined Logic Module Defination ******
module combLogic(w_003_001, w_003_002, w_003_003, w_003_004, w_003_005, w_003_006, w_003_007, w_003_008, OscFlag);

  input w_003_001, w_003_002, w_003_003, w_003_004, w_003_005, w_003_006, w_003_007, w_003_008;
  output OscFlag;

  wire w_000_001, w_000_002, w_000_003, w_000_004, w_000_005;
  wire w_001_006, w_001_007, w_001_008, w_001_009, w_001_010;
  wire w_002_012, w_002_013, w_002_014, w_002_015;

  reg w_002_012_stable;
  reg w_000_003_stable;

  // Logic gates
  not1    I001_001(w_000_001, w_000_002);
  and2    I001_002(w_000_002, w_003_001, w_000_003_stable);
  and2    I001_003(w_000_004, w_003_002, w_000_005);
  and2    I001_004(w_001_006, w_000_005, w_003_003);
  //or2     I001_005(w_001_007, w_003_004, w_000_003);
  and2     I001_005(w_001_007, w_003_004, w_000_003_stable);
  and2    I001_006(w_001_008, w_000_001, w_000_004);
  nand2   I001_007(w_000_005, w_001_009, w_001_010);
  //or2     I001_008(w_002_012, w_001_006, w_001_007);
  and2    I001_008(w_002_012, w_001_006, w_001_007);
  nand2   I001_009(w_000_003, w_002_013, w_002_014);
  nand2   I001_010(w_001_009, w_003_005, w_001_008);
  //or2     I002_012(w_001_010, w_002_012, w_003_006);
  and2     I002_012(w_001_010, w_002_012_stable, w_003_006);
  and2    I002_013(w_002_013, w_003_007, w_002_012);
  not1    I002_014(w_002_014, w_002_015);
  and2    I002_015(w_002_015, w_001_008, w_003_008);

  // Combined assignment to OscFlag
  assign OscFlag = ((!w_003_001)&&(w_003_002)&&(w_003_003)&&(w_003_004)&&(!w_003_005)&&(w_003_006)&&(w_003_007)&&(w_003_008)) ||
                   ((w_003_001)&&(w_003_002)&&(w_003_003)&&(w_003_004)&&(!w_003_006)&&(w_003_007)&&(w_003_008)) ||
                   ((w_003_003)&&(w_003_004)&&(!w_003_006)&&(w_003_007)&&(!w_003_008));
  
  always @(OscFlag) begin
  if (!OscFlag) begin
    w_002_012_stable = w_002_012;
    w_000_003_stable = w_000_003;
  end 
  else begin
    w_002_012_stable = 1'b0;
    w_000_003_stable = 1'b0;
  end
  end

  initial begin
    w_002_012_stable = 1'b0;
    w_000_003_stable = 1'b0;
  end
endmodule