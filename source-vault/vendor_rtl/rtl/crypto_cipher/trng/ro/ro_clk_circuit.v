//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module ro_clk_circuit
#(
    parameter p_IDX = 0,
    parameter INV_N = 97 ,
    parameter delay = 5000
)(
    input   wire            enable      ,
    input   wire            clk         ,
    input   wire            resetn      ,
    input   wire            scan        ,
    output  wire            clk_out
);
  genvar  inv_idx;
  (*dont_touch = "true"*) wire [INV_N-1:0] inv_n   /* synthesis syn_keep=1 */;
  (*dont_touch = "true"*) wire             inv_0   /* synthesis syn_keep=1 */;
  (*dont_touch = "true"*) wire             inv_1   /* synthesis syn_keep=1 */;
  (*dont_touch = "true"*) wire             clk_in  /* synthesis syn_keep=1 */;
  (*dont_touch = "true"*) wire             inv_in  /* synthesis syn_keep=1 */;
  (*dont_touch = "true"*) reg              dft_reg /* synthesis syn_keep=1 */; 
  osr_nand2 OSR_DONTTOUCH_n1(.Z(inv_0),.A(enable),.B(inv_in));

    `ifdef OSR_TRNG_RAND_SIM
        reg ro_clk ;

        initial begin
            forever begin
                if(enable) 
                    ro_clk = #(($random*(INV_N*delay)%5)+5) ~ro_clk; 
                else
                    ro_clk = #2 1'b0 ;
            end
        end
        assign clk_out = ro_clk ;

    `else
        `ifdef OSR_TRNG_SIM
            assign    #(delay*1ps) inv_n[0] = inv_0;
        `else
            assign    #(delay/1000.0) inv_n[0] = inv_0;
        `endif

      generate
        for (inv_idx = 0; inv_idx < (INV_N-1); inv_idx = inv_idx+1)
        begin: INV_CELL
          osr_ro_inv OSR_DONTTOUCH_n(.Z(inv_n[inv_idx+1]),.A(inv_n[inv_idx]));
        end
      endgenerate

      assign  inv_1 = inv_n[INV_N-1];

      osr_inv   OSR_DONTTOUCH_nout(.Z(clk_in),.A(inv_n[INV_N>>1]));

      osr_mux2  OSR_DONTTOUCH_mux(.A(inv_1),.B(dft_reg),.S(scan),.Z(inv_in));

      assign  clk_out = clk_in;
    `endif

  always @(posedge clk or negedge resetn)
    if(!resetn)   dft_reg <= 1'b0   ;
    else if(scan) dft_reg <= inv_1  ;
    else          dft_reg <= 1'b0   ;

endmodule
