//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_sync #(
    parameter p_DWIDTH = 8
)(
    input  wire                    i_clk             , 
    input  wire                    i_rst_n           , 
    input  wire [p_DWIDTH-1:0]     i_in              , 
    output wire [p_DWIDTH-1:0]     o_out               
);

osr_sync_level #( 
    .WIDTH        (p_DWIDTH     )  
    ) u_osr_sync_level ( 
    .i_clk        (i_clk        ), 
    .i_rst_n      (i_rst_n      ), 
    .i_async      (i_in         ), 
    .o_sync       (o_out        )  
); 

endmodule  
