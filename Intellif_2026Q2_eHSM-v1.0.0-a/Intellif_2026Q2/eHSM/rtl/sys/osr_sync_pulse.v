//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

//Pulse synchronization module built using Level synchronization module : osr_sync_pulse.v
//             __    __    __    __    __    __    __    __    __    __    __    __    __ 
//i_a_clk   __|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |
//             ___________________________________________________________________________
//i_a_rst_n __|                                                                           
//                   _____                                                                
//i_a_in    ________|     |_______________________________________________________________
//            ______        ______        ______        ______        ______        ______
//i_b_clk   _|      |______|      |______|      |______|      |______|      |______|      
//            ____________________________________________________________________________
//i_b_rst_n _|                                                                            
//                                                                    _____________       
//o_b_out   _________________________________________________________|             |______

module osr_sync_pulse(
    input  wire         i_a_clk              ,
    input  wire         i_a_rst_n            ,
    input  wire         i_a_in               ,

    input  wire         i_b_clk              ,
    input  wire         i_b_rst_n            ,

    output wire         o_b_out               
);

//----------------------------------------------------------
//i_a_clk domain
reg  a_flag ;
always @(posedge i_a_clk or negedge i_a_rst_n) begin
    if(!i_a_rst_n) begin
        a_flag  <= 1'h0   ;
    end else begin
        a_flag  <= i_a_in ? ~a_flag : a_flag ;
    end
end 

//----------------------------------------------------------
//i_b_clk domain
wire b_flag ;
reg  r_flag ;
reg  b_out  ;

osr_sync_level u_sync(.i_clk(i_b_clk), .i_rst_n(i_b_rst_n), .i_async(a_flag), .o_sync(b_flag));

always @(posedge i_b_clk or negedge i_b_rst_n) begin
    if(!i_b_rst_n) begin
        r_flag  <= 1'h0            ;
        b_out   <= 1'h0            ;
    end else begin
        r_flag  <= b_flag          ;
        b_out   <= r_flag ^ b_flag ;
    end
end

assign o_b_out  = b_out ;

endmodule // osr_sync_pulse
