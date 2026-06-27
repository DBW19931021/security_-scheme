//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_timer_cnt(
input  wire        presetn        ,
input  wire        pclk           ,
input  wire        i_cnt_clk_en   ,
input  wire [2:0]  i_tctl         ,
input  wire [31:0] i_tmod         ,
input  wire        i_tim_start    ,

output wire [31:0] o_tcnt         ,
output wire        o_triger_int    
  );

  wire inten   = i_tctl[2];  
  wire optmode = i_tctl[1];  
  wire timeren = i_tctl[0];  

  reg         TCNT_load;
  reg  [31:0] TCNT;
  wire TCNT_load_in = (i_cnt_clk_en & (TCNT==32'h1)) ? 1'b1 :
                       i_cnt_clk_en                  ? 1'b0 : TCNT_load; 
  wire [31:0] TCNT_in = ~timeren   ? 32'h0 :
                         (i_tim_start | (i_cnt_clk_en & TCNT_load & optmode)) ? i_tmod :
                         (i_cnt_clk_en & (TCNT != 32'h0)) ? TCNT - 1'b1 : TCNT;

  reg  Triger_int;
  wire Triger_int_in = (i_cnt_clk_en & TCNT_load) & inten;

  always @(negedge presetn or posedge pclk)
      if(!presetn) begin
          TCNT_load  <= 1'b0;
          TCNT       <= 32'h0;
          Triger_int <= 1'b0;
      end
      else begin
          TCNT_load  <= TCNT_load_in;
          TCNT       <= TCNT_in;
          Triger_int <= Triger_int_in;
      end 

  assign o_tcnt = TCNT;
  assign o_triger_int = Triger_int;

endmodule
