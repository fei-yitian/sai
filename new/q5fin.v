module combLogic(
  input w_003_001, w_003_002, w_003_003, w_003_004, w_003_005, w_003_006, w_003_007, w_003_008,
  output OscFlag // 输出震荡检测信号
);

  wire w_000_001, w_000_002, w_000_003, w_000_004, w_000_005;
  wire w_001_006, w_001_007, w_001_008, w_001_009, w_001_010;
  wire w_002_012, w_002_013, w_002_014, w_002_015;

  // 用于抑制震荡的寄存器
  reg w_000_003_stable;

  // 震荡检测信号，用于检测环路震荡
  reg [1:0] change_count;
  reg change_detected;

  always @(*) begin
    // 记录原始信号变化，检测震荡
    if (w_000_003 != w_000_003_stable) begin
      change_detected = 1;
    end
    else begin
      change_detected = 0;
    end

    // 计算震荡的变化次数
    if (change_detected) begin
      change_count = change_count + 1;
    end
    else begin
      change_count = 0;
    end

    // 存储稳定的 w_000_003 信号
    w_000_003_stable = w_000_003;
  end

  // 震荡检测逻辑：如果变化次数大于等于 2 则认为发生震荡
  assign OscFlag = (change_count >= 2) ? 1'b1 : 1'b0;

  // 原始逻辑电路
  not1    I001_001(w_000_001, w_000_002);
  and2    I001_002(w_000_002, w_003_001, w_000_003);
  and2    I001_003(w_000_004, w_003_002, w_000_005);
  and2    I001_004(w_001_006, w_000_005, w_003_003);
  and2    I001_005(w_001_007, w_003_004, w_000_003_stable);
  and2    I001_006(w_001_008, w_000_001, w_000_004);
  nand2   I001_007(w_000_005, w_001_009, w_001_010);
  and2    I001_008(w_002_012, w_001_006, w_001_007);
  nand2   I001_009(w_000_003, w_002_013, w_002_014);
  nand2   I001_010(w_001_009, w_003_005, w_001_008);
  and2    I002_012(w_001_010, w_002_012, w_003_006);
  and2    I002_013(w_002_013, w_003_007, w_002_012);
  not1    I002_014(w_002_014, w_002_015);
  and2    I002_015(w_002_015, w_001_008, w_003_008);

endmodule

