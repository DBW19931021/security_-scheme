//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_sync_pp(
i_clk              ,
i_in               ,
o_out              
);
  parameter p_DWIDTH = 8;
  input  wire               i_clk  ;
  input  wire[p_DWIDTH-1:0] i_in   ;
  output wire[p_DWIDTH-1:0] o_out  ;
  reg    [p_DWIDTH-1:0]     r_sync0;
  reg    [p_DWIDTH-1:0]     r_sync1;

  always @ (posedge i_clk)
      begin
          r_sync0 <=  i_in;
          r_sync1 <=  r_sync0;
      end

  assign o_out = r_sync1;

endmodule
