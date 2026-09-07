//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_sync_expand #(
    parameter   WIDTH = 1 
    )(
    input   wire                i_clk           ,
    input   wire                i_rst_n         ,

    input   wire [WIDTH-1:0]    i_async         ,
    output  wire [WIDTH-1:0]    o_sync_expand                  
);

localparam     SYN_WIDTH = WIDTH ;

reg [SYN_WIDTH-1:0] sync_expand0 ; 
reg [SYN_WIDTH-1:0] sync_expand  ; 

wire [SYN_WIDTH-1:0]    syn_o_sync  ;

wire [SYN_WIDTH-1:0] sync_expand0_next = syn_o_sync;    
wire [SYN_WIDTH-1:0] sync_expand_next  = syn_o_sync | sync_expand0;

wire                    syn_i_clk   = i_clk     ;
wire                    syn_i_rst_n = i_rst_n   ;
wire [SYN_WIDTH-1:0]    syn_i_async = i_async   ;

assign o_sync_expand = sync_expand ;

osr_sync_level #(
    .WIDTH     ( SYN_WIDTH )  
    ) u_syn (
    .i_clk      ( syn_i_clk   ), 
    .i_rst_n    ( syn_i_rst_n ), 
    .i_async    ( syn_i_async ), 
    .o_sync     ( syn_o_sync  )  
    ); 

always @(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        sync_expand0      <= {SYN_WIDTH{1'h0}} ; 
        sync_expand       <= {SYN_WIDTH{1'h0}} ; 
    end else begin
        sync_expand0      <= sync_expand0_next ; 
        sync_expand       <= sync_expand_next  ; 
    end
end

endmodule 
