//========================================================================================
//         Copyright (C) 2024 Open Security Research Inc. - All Rights Reserved           
//----------------------------------------------------------------------------------------
//
//Pulse synchronization module built using Level synchronization module : async_pulse.v
//             __    __    __    __    __    __    __    __    __    __    __    __    __ 
//i_a_clk   __|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |
//             ___________________________________________________________________________
//i_a_rst_n __|                                                                           
//                   _____                                                                
//o_a_in    ________|     |_______________________________________________________________
//            _______         _______         _______         _______         _______     
//i_b_clk   _|       |_______|       |_______|       |_______|       |_______|       |____
//            ____________________________________________________________________________
//i_b_rst_n _|                                                                            
//                                                            _______________             
//i_b_out   _________________________________________________|               |____________

module osr_async_pulse(
    input  wire         i_a_clk              ,
    input  wire         i_a_rst_n            ,
    input  wire         i_a_in               ,
    input  wire         i_b_clk              ,
    input  wire         i_b_rst_n            ,
    output wire         o_b_out               
);
  wire  w_sync1;
  reg  r_sync2;
  reg  a_flag;
  reg  b_out;

  always @ (posedge i_a_clk or negedge i_a_rst_n)
      if(~i_a_rst_n) 
          a_flag  <= 1'b0;
      else if(i_a_in)
          a_flag  <= ~a_flag;     

osr_sync_cell u_sync(
        .clk	(i_b_clk),
        .rstn	(i_b_rst_n),
        .i_async (a_flag),
        .o_sync (w_sync1)
        );

  always @ (posedge i_b_clk or negedge i_b_rst_n)
      if(~i_b_rst_n) begin
          r_sync2 <= 1'b0;
          b_out   <= 1'b0;
      end
      else begin
          r_sync2 <= w_sync1;
          b_out   <= r_sync2 ^ w_sync1;
      end

  assign o_b_out  = b_out;

endmodule



