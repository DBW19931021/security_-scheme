//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_sync_rst #(
    parameter               SYNC_TIMES  =   2      
) (
    input  wire             i_scan_rst_n    ,
    input  wire             i_scan_mode     ,

    input  wire             i_clk           ,
    input  wire             i_rst_n         ,
    input  wire             i_soft_rst_n    ,

    output wire             o_rst_n          
);

reg rstn_r;
always @(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        rstn_r <=   1'h0;
    end else begin
        rstn_r <=   i_soft_rst_n;
    end
end 

wire rst_n;
wire irst_n   = rstn_r & i_rst_n ;

osr_mux2 u_irst_mux(.A(irst_n), .B(i_scan_rst_n), .S(i_scan_mode), .Z(rst_n));

wire [SYNC_TIMES-1:0] sync;

osr_sync_level u_osr_sync_level0 (
    .i_clk   (i_clk         ),
    .i_rst_n (rst_n         ),

    .i_async (1'h1          ),
    .o_sync  (sync[0]       )
);

genvar i;
generate
    for(i = 1; i < SYNC_TIMES; i = i + 1) begin : reset_sync
        osr_sync_level u_osr_sync_level (
            .i_clk   (i_clk         ),
            .i_rst_n (rst_n         ),

            .i_async (sync[i-1]     ),
            .o_sync  (sync[i]       )
        );
    end
endgenerate

wire w_rst_n   = sync[SYNC_TIMES-1] ;

osr_mux2 u_orst_mux(.A(w_rst_n), .B(i_scan_rst_n), .S(i_scan_mode), .Z(o_rst_n));

endmodule 
