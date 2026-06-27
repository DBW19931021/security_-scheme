//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_timer_prs(
input  wire        pclk           ,
input  wire        presetn        ,
input  wire [7:0]  i_tprs         ,

output wire        o_cnt_clk_en     
);

  reg  [7:0]  prs_counter;
  wire        prs_clr;

  assign prs_clr = (prs_counter >= i_tprs) ? 1'b1 : 1'b0;
  wire  [7:0] prs_counter_in = prs_clr ? 8'h0 : prs_counter + 1'b1;

  always @(posedge pclk or negedge presetn)
      if(!presetn) prs_counter <= 8'h0;
      else         prs_counter <= prs_counter_in;

  assign o_cnt_clk_en = prs_clr;

endmodule
