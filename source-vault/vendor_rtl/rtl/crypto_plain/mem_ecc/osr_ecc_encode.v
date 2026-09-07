// Author           : Zhangxiqi
// Description      : ECC based on Hsiao code for 32bit,64bit,128bit,256bit
// Version history  : Creat version 1.0 on 2022-05-05 
// Email            : xiqi.zhang@osr-tech.com

//----------------------------Modification list--------------------------------
// 20220505 initial
// 20220804 add 49 and 81 ecc by MIKE
//===============================================================================
module osr_ecc_encode
#(parameter WIDTH_DIN    = 32, 
  parameter WIDTH_CHKBIT = (WIDTH_DIN == 32) | (WIDTH_DIN == 49) ? 7 : (WIDTH_DIN == 64) | (WIDTH_DIN == 81) ? 8 : (WIDTH_DIN == 128) ? 9 : 10,
  parameter WIDTH_DOUT   = WIDTH_DIN + WIDTH_CHKBIT)

(
input  wire [WIDTH_DIN-1:0]         encode_din,
input  wire [WIDTH_CHKBIT-1:0]      tm_chkbits,         // test mode check bits
input  wire [7:0]                   tm_sel_ecc_code,    // test mode select signal
output wire [WIDTH_DOUT-1:0]        encode_dout
                  );

//==========================   generate     =====================================
generate 
if (WIDTH_DIN==32) begin : gen_ecc32encode
ecc32_encode u_ecc32_encode(
  .encode_din(encode_din), .tm_chkbits(tm_chkbits), .tm_sel_ecc_code(tm_sel_ecc_code), .encode_dout(encode_dout));
end

else if (WIDTH_DIN==49) begin : gen_ecc49encode
ecc_generate_32  ecc_ge(
  .data_i (encode_din), .tm_chkbits(tm_chkbits), .tm_sel_ecc_code(tm_sel_ecc_code), .ecc_o (encode_dout));
end

else if (WIDTH_DIN==64) begin : gen_ecc64encode
ecc64_encode u_ecc64_encode(
  .encode_din(encode_din), .tm_chkbits(tm_chkbits), .tm_sel_ecc_code(tm_sel_ecc_code), .encode_dout(encode_dout));
end

else if (WIDTH_DIN==81) begin : gen_ecc81encode
ecc_generate_64  ecc_ge(
    .data_i (encode_din), .tm_chkbits(tm_chkbits), .tm_sel_ecc_code(tm_sel_ecc_code), .ecc_o (encode_dout));
end

else if (WIDTH_DIN==128) begin : gen_ecc128encode
ecc128_encode u_ecc128_encode(
  .encode_din(encode_din), .tm_chkbits(tm_chkbits), .tm_sel_ecc_code(tm_sel_ecc_code), .encode_dout(encode_dout));
end

else begin : gen_ecc256encode
ecc256_encode u_ecc256_encode(
  .encode_din(encode_din), .tm_chkbits(tm_chkbits), .tm_sel_ecc_code(tm_sel_ecc_code), .encode_dout(encode_dout));
end

endgenerate

endmodule
