//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_ch_arbmux #(
    parameter [31:0] p_CH_NUM   =  8,
    parameter [31:0] p_CH_WIDTH = 32'd64+32'd24 
)(
    input   wire                           i_clk      ,
    input   wire                           i_rst_n    ,

    output  wire [p_CH_WIDTH-1:0]          o_dst_data ,
    output  wire                           o_dst_vld  ,
    input   wire                           i_dst_rdy  ,

    input   wire [p_CH_NUM*p_CH_WIDTH-1:0] i_src_data ,
    input   wire [p_CH_NUM*1-1:0]          i_src_vld  ,
    output  wire [p_CH_NUM*1-1:0]          o_src_rdy
);
    genvar i;

    reg                                    dst_vld       ;
    wire                                   dst_vld_in    ;

    wire                                   fifo_wr       ; 
    wire [p_CH_WIDTH-1:0]                  fifo_wdata    ;
    wire                                   fifo_full_n   ; 

    wire                                   fifo_rd       ; 
    wire [p_CH_WIDTH-1:0]                  fifo_rdata    ;
    wire                                   fifo_empty_n  ;

    wire [p_CH_NUM-1:0]                    req           ;
    wire [p_CH_NUM-1:0]                    grant         ;
    reg  [p_CH_NUM-1:0]                    grant_out     ;
    wire [p_CH_NUM-1:0]                    grant_out_in  ;
    reg  [p_CH_NUM-1:0]                    base          ;
    wire [p_CH_NUM-1:0]                    base_in       ;
    wire [p_CH_NUM*2-1:0]                  double_req    ;
    wire [p_CH_NUM*2-1:0]                  double_grant  ;

generate
if(p_CH_NUM>1) begin:gen_multi_channel
    assign double_req   = {req,req};
    assign double_grant = double_req&~(double_req-{{(p_CH_NUM){1'b0}},base});
    assign grant        = double_grant[p_CH_NUM-1:0]|double_grant[2*p_CH_NUM-1:p_CH_NUM] ;

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            base     <= {{(p_CH_NUM-1){1'b0}},{1'b1}};
            grant_out<= {{(p_CH_NUM-1){1'b0}},{1'b1}};
        end
        else begin
            base     <=  base_in ;
            grant_out<=  grant_out_in ;
        end
    end

    assign base_in      = (|req)&(fifo_full_n) ? {grant[p_CH_NUM-2:0],grant[p_CH_NUM-1]} : base ;

    assign grant_out_in = (|req)&(fifo_full_n) ? grant : grant_out ;

    assign o_src_rdy  = grant_out&{(p_CH_NUM){fifo_full_n}} ;

    assign req = i_src_vld ;

    assign fifo_wr = (|(grant_out&req))&(fifo_full_n)  ;

    osr_axi_ch_mux #(
        .p_DWIDTH(p_CH_WIDTH ),
        .p_NUM   (p_CH_NUM   )
    ) u_axi_ch_mux(
        .i_sel (o_src_rdy    ),
        .i_data(i_src_data   ),
        .o_data(fifo_wdata   )
    );

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (1         ),
        .p_DWIDTH       (p_CH_WIDTH)
    ) u_fifo(
        .i_clk          (i_clk           ),
        .i_rst_n        (i_rst_n         ),    

        .i_wr           (fifo_wr           ),
        .i_wr_data      (fifo_wdata        ),
        .o_wr_num_count (                  ),
        .o_wr_full      (                  ),
        .o_wr_full_n    (fifo_full_n       ),
        .o_wr_al_full   (                  ),
        .o_wr_al_full_n (                  ),

        .i_rd           (fifo_rd           ),
        .o_rd_data      (fifo_rdata        ),
        .o_rd_num_count (                  ),
        .o_rd_empty     (                  ),
        .o_rd_empty_n   (fifo_empty_n      ),
        .o_rd_al_empty  (                  ),
        .o_rd_al_empty_n(                  ),

        .i_clear        (1'b0              )
    ); 

    assign fifo_rd = (fifo_empty_n)&((!dst_vld)|(dst_vld&i_dst_rdy));

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            dst_vld <= 1'b0 ;
        end
        else begin
            dst_vld <=  dst_vld_in ;
        end
    end

    assign dst_vld_in = (fifo_rd)  ? 1'b1 :
                        i_dst_rdy  ? 1'b0 : dst_vld ;

    assign o_dst_vld  = dst_vld ;

    assign o_dst_data = fifo_rdata ;
end
else begin:bypass
    assign o_dst_data = i_src_data ; 
    assign o_dst_vld  = i_src_vld  ; 
    assign o_src_rdy  = i_dst_rdy  ;
end

endgenerate

endmodule
