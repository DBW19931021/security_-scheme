//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_crc32 (
   crc_reg, 
   crc,
   d,
   calc,
   init,
   d_valid,
   clk,
   rst_n
   );

output [31:0] crc_reg;
output [7:0]  crc;

input  [7:0]  d;
input         calc;
input         init;
input         d_valid;
input         clk;
input         rst_n;

reg    [31:0] crc_reg;
reg    [7:0]  crc;

wire   [31:0] next_crc;

always @ (posedge clk or negedge rst_n)
begin
   if (~rst_n) begin
      crc_reg <= 32'hFFFFFFFF;
      crc     <= 8'hFF;
   end

   else if (init) begin
      crc_reg <= 32'hFFFFFFFF;
      crc     <=  8'hFF;
   end

   else if (calc & d_valid) begin
      crc_reg <= next_crc;
      crc     <= ~{next_crc[24], next_crc[25], next_crc[26], next_crc[27],
                   next_crc[28], next_crc[29], next_crc[30], next_crc[31]};
   end

   else if (~calc & d_valid) begin
      crc_reg <=  {crc_reg[23:0], 8'hFF};
      crc     <= ~{crc_reg[16], crc_reg[17], crc_reg[18], crc_reg[19],
                   crc_reg[20], crc_reg[21], crc_reg[22], crc_reg[23]};
   end
end

assign next_crc[0] = d[7] ^ crc_reg[30] ^ d[1] ^ crc_reg[24];
assign next_crc[1] = d[7] ^ crc_reg[25] ^ d[1] ^ crc_reg[31] ^ d[6] ^ crc_reg[30] ^ d[0] ^ crc_reg[24];
assign next_crc[2] = d[7] ^ crc_reg[25] ^ d[1] ^ d[5] ^ crc_reg[31] ^ d[6] ^ crc_reg[30] ^ d[0] ^ crc_reg[26] ^ crc_reg[24];
assign next_crc[3] = crc_reg[25] ^ d[5] ^ crc_reg[31] ^ crc_reg[27] ^ d[6] ^ d[0] ^ crc_reg[26] ^ d[4];
assign next_crc[4] = d[7] ^ d[1] ^ d[5] ^ d[3] ^ crc_reg[27] ^ crc_reg[30] ^ crc_reg[28] ^ crc_reg[26] ^ crc_reg[24] ^ d[4];
assign next_crc[5] = d[7] ^ crc_reg[25] ^ d[1] ^ crc_reg[31] ^ d[3] ^ crc_reg[27] ^ d[2] ^ d[6] ^ crc_reg[29] ^ crc_reg[30] ^ crc_reg[28] ^ d[0] ^ crc_reg[24] ^ d[4];
assign next_crc[6] = crc_reg[25] ^ d[5] ^ d[1] ^ crc_reg[31] ^ d[3] ^ d[2] ^ d[6] ^ crc_reg[29] ^ crc_reg[28] ^ crc_reg[30] ^ d[0] ^ crc_reg[26];
assign next_crc[7] = d[7] ^ d[5] ^ crc_reg[31] ^ crc_reg[27] ^ d[2] ^ crc_reg[29] ^ crc_reg[26] ^ d[0] ^ crc_reg[24] ^ d[4];
assign next_crc[8] = d[7] ^ crc_reg[25] ^ crc_reg[0] ^ d[3] ^ crc_reg[27] ^ d[6] ^ crc_reg[28] ^ crc_reg[24] ^ d[4];
assign next_crc[9] = crc_reg[25] ^ d[5] ^ d[3] ^ d[2] ^ d[6] ^ crc_reg[29] ^ crc_reg[1] ^ crc_reg[28] ^ crc_reg[26];
assign next_crc[10] = d[7] ^ d[5] ^ crc_reg[27] ^ d[2] ^ crc_reg[29] ^ crc_reg[26] ^ crc_reg[2] ^ crc_reg[24] ^ d[4];
assign next_crc[11] = d[7] ^ crc_reg[25] ^ d[3] ^ crc_reg[27] ^ d[6] ^ crc_reg[28] ^ crc_reg[3] ^ crc_reg[24] ^ d[4];
assign next_crc[12] = d[7] ^ crc_reg[4] ^ crc_reg[25] ^ d[1] ^ d[5] ^ d[3] ^ d[2] ^ d[6] ^ crc_reg[29] ^ crc_reg[30] ^ crc_reg[28] ^ crc_reg[26] ^ crc_reg[24];
assign next_crc[13] = crc_reg[5] ^ crc_reg[25] ^ d[5] ^ d[1] ^ crc_reg[31] ^ crc_reg[27] ^ d[2] ^ d[6] ^ crc_reg[29] ^ crc_reg[30] ^ d[0] ^ crc_reg[26] ^ d[4];
assign next_crc[14] = d[5] ^ d[1] ^ d[3] ^ crc_reg[31] ^ crc_reg[27] ^ crc_reg[28] ^ crc_reg[30] ^ crc_reg[26] ^ d[0] ^ crc_reg[6] ^ d[4];
assign next_crc[15] = crc_reg[7] ^ d[3] ^ crc_reg[31] ^ crc_reg[27] ^ d[2] ^ crc_reg[29] ^ crc_reg[28] ^ d[0] ^ d[4];
assign next_crc[16] = d[7] ^ crc_reg[8] ^ d[3] ^ d[2] ^ crc_reg[29] ^ crc_reg[28] ^ crc_reg[24];
assign next_crc[17] = crc_reg[25] ^ crc_reg[9] ^ d[1] ^ d[2] ^ d[6] ^ crc_reg[29] ^ crc_reg[30];
assign next_crc[18] = crc_reg[10] ^ d[5] ^ d[1] ^ crc_reg[31] ^ crc_reg[30] ^ crc_reg[26] ^ d[0];
assign next_crc[19] = crc_reg[11] ^ crc_reg[31] ^ crc_reg[27] ^ d[0] ^ d[4];
assign next_crc[20] = d[3] ^ crc_reg[12] ^ crc_reg[28];
assign next_crc[21] = crc_reg[13] ^ d[2] ^ crc_reg[29];
assign next_crc[22] = d[7] ^ crc_reg[14] ^ crc_reg[24];
assign next_crc[23] = d[7] ^ crc_reg[25] ^ d[1] ^ crc_reg[15] ^ d[6] ^ crc_reg[30] ^ crc_reg[24];
assign next_crc[24] = crc_reg[25] ^ d[5] ^ crc_reg[31] ^ d[6] ^ crc_reg[16] ^ d[0] ^ crc_reg[26];
assign next_crc[25] = crc_reg[17] ^ d[5] ^ crc_reg[27] ^ crc_reg[26] ^ d[4];
assign next_crc[26] = d[7] ^ crc_reg[18] ^ d[1] ^ d[3] ^ crc_reg[27] ^ crc_reg[30] ^ crc_reg[28] ^ crc_reg[24] ^ d[4];
assign next_crc[27] = crc_reg[19] ^ crc_reg[25] ^ crc_reg[31] ^ d[3] ^ d[2] ^ d[6] ^ crc_reg[29] ^ crc_reg[28] ^ d[0];
assign next_crc[28] = crc_reg[20] ^ d[5] ^ d[1] ^ d[2] ^ crc_reg[29] ^ crc_reg[30] ^ crc_reg[26];
assign next_crc[29] = crc_reg[21] ^ d[1] ^ crc_reg[31] ^ crc_reg[27] ^ crc_reg[30] ^ d[0] ^ d[4];
assign next_crc[30] = crc_reg[22] ^ d[3] ^ crc_reg[31] ^ crc_reg[28] ^ d[0];
assign next_crc[31] = d[2] ^ crc_reg[23] ^ crc_reg[29];
endmodule
