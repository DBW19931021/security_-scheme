// Author           : Zhangxiqi
// Description      : ECC based on Hsiao code for 64bit
// Version history  : Creat version 1.0 on 2022-05-05 
// Email            : xiqi.zhang@osr-tech.com

//----------------------------Modification list--------------------------------
// 20220425 initial
// 20220804 add 49 and 81 ecc by MIKE
//=============================================================================
module osr_ecc_decode
#(parameter WIDTH_DOUT = 32,
  parameter WIDTH_DIN  = (WIDTH_DOUT == 32) ? 39 : (WIDTH_DOUT == 49) ? 56 : (WIDTH_DOUT == 64) ? 72 : (WIDTH_DOUT == 81) ? 89 : (WIDTH_DOUT == 128) ? 137 : 266
)
(
input  wire [WIDTH_DIN-1:0]   decode_din,              
input  wire [7:0]             ecc_en,               // ECC enable
input  wire [WIDTH_DIN-1:0]   xor_dedin,            // ECC for test rom data
output wire                   ecc_sec,              // ECC signal error crrection
output wire                   ecc_ded,              // ECC mulity error dect
output wire [WIDTH_DOUT-1:0]  decode_dout
                  );

//==========================   generate     =====================================
generate 
if (WIDTH_DOUT==32) begin : gen_ecc32decode
ecc32_decode  u_ecc32_decode(
  .decode_din(decode_din), .ecc_en(ecc_en),   .xor_dedin(xor_dedin),   
  .ecc_sec(ecc_sec),       .ecc_ded(ecc_ded), .decode_dout(decode_dout));
end

else if (WIDTH_DOUT==49) begin : gen_ecc49decode
wire [6:0] syndrome;
ecc_check_32  ecc_chk (
  .ecc_i(decode_din[55:49]), .data_i({decode_din[48:17],decode_din[16:0]}), .xor_dedin(xor_dedin), .syndrome_o(syndrome[6:0]));

ecc_repair_32 ecc_rep ( 
  .only_data_i(decode_din[48:0]), .syndrome_i(syndrome[6:0]), .repair_en_i(ecc_en), 
  .any_ecc_err_o(ecc_sec), .fatal_ecc_err_o(ecc_ded), .repaired_data_o(decode_dout));
end

else if (WIDTH_DOUT==64) begin : gen_ecc64decode
ecc64_decode u_ecc64_decode(
  .decode_din(decode_din), .ecc_en(ecc_en),   .xor_dedin(xor_dedin), 
  .ecc_sec(ecc_sec),       .ecc_ded(ecc_ded), .decode_dout(decode_dout));
end

else if (WIDTH_DOUT==81) begin : gen_ecc81decode
wire [7:0] syndrome;
ecc_check_64  ecc_chk (
  .ecc_i(decode_din[88:81]), .data_i({decode_din[80:17],decode_din[16:0]}), .xor_dedin(xor_dedin), .syndrome_o(syndrome[7:0]));
 
ecc_repair_64 ecc_rep (
  .only_data_i(decode_din[80:0]), .syndrome_i(syndrome[7:0]), .repair_en_i(ecc_en), 
  .any_ecc_err_o(ecc_sec), .fatal_ecc_err_o(ecc_ded), .repaired_data_o(decode_dout));
end

else if (WIDTH_DOUT==128) begin : gen_ecc128decode
ecc128_decode u_ecc128_decode(
  .decode_din(decode_din), .ecc_en(ecc_en),   .xor_dedin(xor_dedin), 
  .ecc_sec(ecc_sec),       .ecc_ded(ecc_ded), .decode_dout(decode_dout));
end

else begin : gen_ecc256decode
ecc256_decode u_ecc256_decode(
  .decode_din(decode_din), .ecc_en(ecc_en),   .xor_dedin(xor_dedin), 
  .ecc_sec(ecc_sec),       .ecc_ded(ecc_ded), .decode_dout(decode_dout));
end

endgenerate

endmodule
