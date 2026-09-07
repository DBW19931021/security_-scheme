//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_mbox_decoder (
    input  wire         i_hsel      ,
    input  wire [31:0]  i_haddr     ,

    output wire         o_hsel0     ,
    output wire         o_hsel1     ,
    output wire         o_hsel2     ,
    output wire         o_hsel3     ,
    output wire         o_hsel4     ,
    output wire         o_hsel5     ,
    output wire         o_hsel6     ,
    output wire         o_hsel7     ,
    output wire         o_hsel8     ,
    output wire         o_hsel9     ,
    output wire         o_hsel10    ,
    output wire         o_hsel11    ,
    output wire         o_hsel12    ,
    output wire         o_hsel13    ,
    output wire         o_hsel14    ,
    output wire         o_hsel15

);

assign  o_hsel0  = i_hsel & (i_haddr[15:12] == 4'h0); 
assign  o_hsel1  = i_hsel & (i_haddr[15:12] == 4'h1);
assign  o_hsel2  = i_hsel & (i_haddr[15:12] == 4'h2);
assign  o_hsel3  = i_hsel & (i_haddr[15:12] == 4'h3);
assign  o_hsel4  = i_hsel & (i_haddr[15:12] == 4'h4);
assign  o_hsel5  = i_hsel & (i_haddr[15:12] == 4'h5);
assign  o_hsel6  = i_hsel & (i_haddr[15:12] == 4'h6);
assign  o_hsel7  = i_hsel & (i_haddr[15:12] == 4'h7);
assign  o_hsel8  = i_hsel & (i_haddr[15:12] == 4'h8);
assign  o_hsel9  = i_hsel & (i_haddr[15:12] == 4'h9);
assign  o_hsel10 = i_hsel & (i_haddr[15:12] == 4'ha);
assign  o_hsel11 = i_hsel & (i_haddr[15:12] == 4'hb);
assign  o_hsel12 = i_hsel & (i_haddr[15:12] == 4'hc);
assign  o_hsel13 = i_hsel & (i_haddr[15:12] == 4'hd);
assign  o_hsel14 = i_hsel & (i_haddr[15:12] == 4'he);
assign  o_hsel15 = i_hsel & (i_haddr[15:12] == 4'hf);

endmodule 
