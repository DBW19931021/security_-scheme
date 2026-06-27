//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_lfsr(
input  wire                 clk        ,
input  wire                 rst_n      ,
input  wire                 i_lfsr_mode, 
input  wire                 i_lfsr_ks  ,
input  wire                 i_lfsr_en  ,
input  wire                 i_lfsr_sel , 
input  wire                 i_seed_vld ,
input  wire [31:0]          i_seed_data,

output wire [31:0]          o_lfsr_data
);

reg  [32 : 1]   r_lfsr;
wire            w_xnor;

wire            shift_en  = i_lfsr_en & (~i_lfsr_mode | (i_lfsr_mode & i_lfsr_ks));
wire [32 : 1]   w_lfsr_in = i_seed_vld ? i_seed_data : 
                            shift_en   ? (i_lfsr_sel ? {r_lfsr[31:1], w_xnor} : {16'h0, r_lfsr[15:1], w_xnor}) : r_lfsr;
assign          w_xnor    = i_lfsr_sel ? r_lfsr[32] ^ (~r_lfsr[22]) ^ (~r_lfsr[2]) ^ (~r_lfsr[1]) :
                                         r_lfsr[16] ^ (~r_lfsr[15]) ^ (~r_lfsr[13]) ^ (~r_lfsr[4]) ;

assign          o_lfsr_data    = r_lfsr; 

always @(posedge clk or negedge rst_n)
    if(~rst_n) begin
        r_lfsr      <= 32'h0;
    end
    else begin
        r_lfsr      <= w_lfsr_in;
    end

endmodule        
