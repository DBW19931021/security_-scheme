//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_crc16 (
   crc_reg, 
   crc,
   d,
   calc,
   init,
   d_valid,
   clk,
   rst_n
   );

output [15:0] crc_reg;
output [7:0]  crc;

input  [7:0]  d;
input         calc;
input         init;
input         d_valid;
input         clk;
input         rst_n;

reg    [15:0] crc_reg;
reg    [7:0]  crc;

wire   [15:0] next_crc;

always @ (posedge clk or negedge rst_n)
begin
   if (~rst_n) begin
      crc_reg <= 16'hFFFF;
      crc     <= 8'hFF;
   end

   else if (init) begin
      crc_reg <= 16'hFFFF;
      crc     <=  8'hFF;
   end

   else if (calc & d_valid) begin
      crc_reg <= next_crc;
      crc     <= ~{next_crc[8], next_crc[9], next_crc[10], next_crc[11],
                   next_crc[12], next_crc[13], next_crc[14], next_crc[15]};
   end

   else if (~calc & d_valid) begin
      crc_reg <=  {crc_reg[7:0], 8'h00};
      crc     <= ~{crc_reg[0], crc_reg[1], crc_reg[2], crc_reg[3],
                   crc_reg[4], crc_reg[5], crc_reg[6], crc_reg[7]};
   end
end

assign next_crc[0] = d[4] ^ d[0] ^ crc_reg[13] ^ crc_reg[9] ^ d[5] ^ d[1] ^ crc_reg[14] ^ crc_reg[10] ^ d[6] ^ d[2] ^ crc_reg[15] ^ crc_reg[11] ^ d[7] ^ d[3] ^ crc_reg[12] ^ crc_reg[8];
assign next_crc[1] = d[4] ^ d[0] ^ crc_reg[13] ^ crc_reg[9] ^ d[5] ^ d[1] ^ crc_reg[14] ^ crc_reg[10] ^ d[6] ^ d[2] ^ crc_reg[15] ^ crc_reg[11] ^ d[3] ^ crc_reg[12];
assign next_crc[2] = crc_reg[9] ^ d[6] ^ d[7] ^ crc_reg[8];
assign next_crc[3] = crc_reg[9] ^ d[5] ^ crc_reg[10] ^ d[6];
assign next_crc[4] = d[4] ^ d[5] ^ crc_reg[10] ^ crc_reg[11];
assign next_crc[5] = d[4] ^ crc_reg[11] ^ crc_reg[12] ^ d[3];
assign next_crc[6] = crc_reg[12] ^ crc_reg[13] ^ d[2] ^ d[3];
assign next_crc[7] = crc_reg[13] ^ d[1] ^ crc_reg[14] ^ d[2];
assign next_crc[8] = d[0] ^ d[1] ^ crc_reg[14] ^ crc_reg[15] ^ crc_reg[0];
assign next_crc[9] = d[0] ^ crc_reg[15] ^ crc_reg[1];
assign next_crc[10] = crc_reg[2];
assign next_crc[11] = crc_reg[3];
assign next_crc[12] = crc_reg[4];
assign next_crc[13] = crc_reg[5];
assign next_crc[14] = crc_reg[6];
assign next_crc[15] = d[4] ^ crc_reg[9] ^ d[5] ^ crc_reg[10] ^ d[6] ^ crc_reg[11] ^ d[7] ^ crc_reg[12] ^ d[0] ^ crc_reg[13] ^ d[1] ^ crc_reg[14] ^ d[2] ^ crc_reg[15] ^ crc_reg[7] ^ d[3] ^ crc_reg[8];
endmodule
