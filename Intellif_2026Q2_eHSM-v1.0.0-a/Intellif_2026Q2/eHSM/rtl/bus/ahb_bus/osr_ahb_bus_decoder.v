//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_bus_decoder (
    input  wire [31:0]   i_haddr        ,
    input  wire          i_hselin       ,
    output wire          o_hsel0        ,
    output wire          o_hsel1        ,
    output wire          o_hsel2        ,
    output wire          o_hsel3        ,
    output wire          o_hsel4        ,
    output wire          o_hsel5        ,
    output wire          o_hsel6        ,
    output wire          o_hsel7        ,
    output wire          o_hsel8        ,
    output wire          o_hsel9        ,
    output wire          o_hsel10       , 
    output wire          o_hsel11       , 
    output wire          o_hsel12       , 
    output wire          o_hsel13       , 
    output wire          o_hsel14       ,
    output wire          o_hsel15         
);

assign o_hsel0  = i_hselin & ( i_haddr[27:20] == 8'h00 ); 
assign o_hsel1  = i_hselin & ( i_haddr[27:20] == 8'h04 ); 
assign o_hsel2  = i_hselin & ( i_haddr[27:20] == 8'h08 ); 
assign o_hsel3  = i_hselin & ( i_haddr[27:20] == 8'h0C ); 
assign o_hsel4  = i_hselin & ( i_haddr[27:20] == 8'h10 ); 
assign o_hsel5  = i_hselin & ( i_haddr[27:20] == 8'h14 ); 
assign o_hsel6  = i_hselin & ( i_haddr[27:20] == 8'h18 ); 
assign o_hsel7  = i_hselin & ( i_haddr[27:20] == 8'h1C ); 
assign o_hsel8  = i_hselin & ( i_haddr[27:20] == 8'h01 ); 
assign o_hsel9  = i_hselin & ( i_haddr[27:24] == 4'hF  ); 
assign o_hsel10 = i_hselin & ( i_haddr[27:20] == 8'h30 ); 
assign o_hsel11 = i_hselin & ( i_haddr[27:20] == 8'h34 ); 
assign o_hsel12 = i_hselin & ( i_haddr[27:20] == 8'h0f ); 
assign o_hsel13 = i_hselin & ( i_haddr[27:20] == 8'h1b ); 
assign o_hsel14 = i_hselin & ( i_haddr[27:20] == 8'h15 ); 
assign o_hsel15 = i_hselin & ( i_haddr[27:20] == 8'h19 ); 

endmodule 
