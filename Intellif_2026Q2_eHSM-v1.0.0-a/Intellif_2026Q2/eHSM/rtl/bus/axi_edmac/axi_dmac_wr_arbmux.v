//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_wr_arbmux #(
    parameter [31:0] p_BUS_DWIDTH   =512,
    parameter [31:0] p_BUS_AWIDTH   = 64,
    parameter [31:0] p_W_FIFO_AWIDTH=  3,
    parameter [31:0] p_W_SMEM_AWIDTH=  3,
    parameter [31:0] p_CH_NUM       =  8,
    parameter [31:0] p_CH_WIDTH     = 32'd64+32'd16+32'd8

)(
    input   wire                             i_clk           ,
    input   wire                             i_rst_n         ,

    output  wire [p_CH_WIDTH-1:0]            o_dst_data      ,
    output  wire                             o_dst_vld       ,
    input   wire                             i_dst_rdy       ,

    input   wire [p_CH_NUM*p_CH_WIDTH-1:0]   i_src_data      ,
    input   wire [p_CH_NUM*1-1:0]            i_src_vld       ,
    output  wire [p_CH_NUM*1-1:0]            o_src_rdy       ,

    output  wire [p_BUS_DWIDTH-1:0]          o_dbus_dst_data ,
    output  wire                             o_dbus_dst_vld  ,
    input   wire                             i_dbus_dst_rdy  ,

    input   wire [p_CH_NUM*p_BUS_DWIDTH-1:0] i_dbus_src_data ,
    input   wire [p_CH_NUM*1-1:0]            i_dbus_src_vld  ,
    output  wire [p_CH_NUM*1-1:0]            o_dbus_src_rdy  
);
    localparam [2:0]ADDR_OFFSET_WIDTH = (p_BUS_DWIDTH/8<=4  ) ? 3'd2 :    
                                        (p_BUS_DWIDTH/8<=8  ) ? 3'd3 :    
                                        (p_BUS_DWIDTH/8<=16 ) ? 3'd4 :    
                                        (p_BUS_DWIDTH/8<=32 ) ? 3'd5 :    
                                        (p_BUS_DWIDTH/8<=64 ) ? 3'd6 : 3'd0 ;

    localparam [31:0]MUX_FIFO_WIDTH   = p_CH_NUM+p_W_FIFO_AWIDTH+1 ;

    genvar i;

    reg                                      dst_vld       ;
    wire                                     dst_vld_in    ;

    wire                                     fifo_wr       ;
    wire [p_CH_WIDTH-1:0]                    fifo_wdata    ;
    wire [p_CH_WIDTH+p_CH_NUM-1:0]           fifo_wdata_tmp;
    wire                                     fifo_full_n   ; 

    wire                                     fifo_rd       ;
    wire [p_CH_WIDTH-1:0]                    fifo_rdata    ;
    wire [p_CH_WIDTH+p_CH_NUM-1:0]           fifo_rdata_tmp;
    wire                                     fifo_empty_n  ;

    wire [p_CH_NUM-1:0]                      req           ;
    wire [p_CH_NUM-1:0]                      grant         ;
    reg  [p_CH_NUM-1:0]                      grant_out     ;
    wire [p_CH_NUM-1:0]                      grant_out_in  ;
    reg  [p_CH_NUM-1:0]                      base          ;
    wire [p_CH_NUM-1:0]                      base_in       ;
    wire [p_CH_NUM*2-1:0]                    double_req    ;
    wire [p_CH_NUM*2-1:0]                    double_grant  ;

    wire                                     mux_fifo_wr            ;
    wire [MUX_FIFO_WIDTH-1:0]                mux_fifo_wdata         ;
    wire                                     mux_fifo_al_full_n     ; 

    wire                                     mux_fifo_rd            ;
    wire [MUX_FIFO_WIDTH-1:0]                mux_fifo_rdata         ;
    wire                                     mux_fifo_empty_n       ;
    reg                                      mux_fifo_vld           ;
    wire                                     mux_fifo_vld_in        ;
    wire                                     mux_fifo_rdy           ;

    wire                                     dbus_fifo_wr           ;
    wire [p_BUS_DWIDTH-1:0]                  dbus_fifo_wdata        ;
    wire                                     dbus_fifo_full_n       ; 

    wire                                     dbus_fifo_rd           ;
    wire [p_BUS_DWIDTH-1:0]                  dbus_fifo_rdata        ;
    wire                                     dbus_fifo_empty_n      ;

    wire [15:0]                              w_strans_len_small     ;
    wire [p_BUS_AWIDTH-1:0]                  ch_waddr               ;

    wire [p_CH_NUM-1:0]                      w_strans_mux_in        ;
    wire [p_CH_NUM-1:0]                      w_strans_mux_out       ;
    wire [16:0]                              w_strans_burst_range   ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_burst_len     ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_burst_len_out ;
    reg  [p_W_FIFO_AWIDTH:0]                 w_strans_beat_cnt      ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_beat_cnt_in   ;

    reg                                      dbus_dst_vld           ;
    wire                                     dbus_dst_vld_in        ;
    wire [p_CH_NUM*1-1:0]                    dbus_src_rdy           ;

generate
if(p_CH_NUM>1) begin: gen_multi_channel
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
        .p_DWIDTH(p_CH_WIDTH   ),
        .p_NUM   (p_CH_NUM     )
    ) u_abus_mux(
        .i_sel (o_src_rdy    ),
        .i_data(i_src_data   ),
        .o_data(fifo_wdata   )
    );

    assign fifo_wdata_tmp = {o_src_rdy,fifo_wdata};

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (1                   ),
        .p_DWIDTH       (p_CH_WIDTH+p_CH_NUM )
    ) u_abus_fifo(
        .i_clk          (i_clk             ),
        .i_rst_n        (i_rst_n           ),    

        .i_wr           (fifo_wr           ),
        .i_wr_data      (fifo_wdata_tmp    ),
        .o_wr_num_count (                  ),
        .o_wr_full      (                  ),
        .o_wr_full_n    (fifo_full_n       ),
        .o_wr_al_full   (                  ),
        .o_wr_al_full_n (                  ),

        .i_rd           (fifo_rd           ),
        .o_rd_data      (fifo_rdata_tmp    ),
        .o_rd_num_count (                  ),
        .o_rd_empty     (                  ),
        .o_rd_empty_n   (fifo_empty_n      ),
        .o_rd_al_empty  (                  ),
        .o_rd_al_empty_n(                  ),

        .i_clear        (1'b0              )
    ); 

    assign fifo_rd    = (fifo_empty_n&mux_fifo_al_full_n)&    
                        ((!dst_vld)|(dst_vld&i_dst_rdy));

    assign fifo_rdata = fifo_rdata_tmp[p_CH_WIDTH-1:0];

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

    assign w_strans_mux_in       = fifo_rdata_tmp[p_CH_WIDTH+:p_CH_NUM] ;
    assign w_strans_len_small    = fifo_rdata_tmp[p_BUS_AWIDTH+:16] ;
    assign ch_waddr              = fifo_rdata_tmp[0+:p_BUS_AWIDTH] ;

    assign w_strans_burst_range        = (w_strans_len_small+{{(16-ADDR_OFFSET_WIDTH){1'b0}},ch_waddr[ADDR_OFFSET_WIDTH-1:0]}) + 17'b0;

wire                          w_strans_burst_range_inc_tmp = (|w_strans_burst_range[ADDR_OFFSET_WIDTH-1:0]);
wire [16-ADDR_OFFSET_WIDTH:0] w_strans_burst_range_add_tmp = w_strans_burst_range[16:ADDR_OFFSET_WIDTH]+{{(16-ADDR_OFFSET_WIDTH){1'b0}},w_strans_burst_range_inc_tmp};
    assign w_strans_burst_len          = (w_strans_len_small==0) ? {(p_W_FIFO_AWIDTH+1){1'b0}}  : w_strans_burst_range_add_tmp[p_W_FIFO_AWIDTH:0];

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (p_W_SMEM_AWIDTH   ),
        .p_DWIDTH       (MUX_FIFO_WIDTH    )
    ) u_mux_fifo(
        .i_clk          (i_clk             ),
        .i_rst_n        (i_rst_n           ),    

        .i_wr           (mux_fifo_wr       ),
        .i_wr_data      (mux_fifo_wdata    ),
        .o_wr_num_count (                  ),
        .o_wr_full      (                  ),
        .o_wr_full_n    (                  ),
        .o_wr_al_full   (                  ),
        .o_wr_al_full_n (mux_fifo_al_full_n),

        .i_rd           (mux_fifo_rd       ),
        .o_rd_data      (mux_fifo_rdata    ),
        .o_rd_num_count (                  ),
        .o_rd_empty     (                  ),
        .o_rd_empty_n   (mux_fifo_empty_n  ),
        .o_rd_al_empty  (                  ),
        .o_rd_al_empty_n(                  ),

        .i_clear        (1'b0              )
    ); 

    assign mux_fifo_wr           = dst_vld&i_dst_rdy ;
    assign mux_fifo_wdata        = {w_strans_mux_in,w_strans_burst_len};

    assign mux_fifo_rd           = (mux_fifo_empty_n)&((!mux_fifo_vld)|(mux_fifo_vld&mux_fifo_rdy));

    assign {w_strans_mux_out,w_strans_burst_len_out} = mux_fifo_rdata;

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            mux_fifo_vld      <= 1'b0 ;
            w_strans_beat_cnt <= {(p_W_FIFO_AWIDTH+1){1'b0}};
        end
        else begin
            w_strans_beat_cnt <=  w_strans_beat_cnt_in ;
            mux_fifo_vld      <=  mux_fifo_vld_in ;
        end
    end

    assign mux_fifo_vld_in = (mux_fifo_rd) ? 1'b1 :
                             mux_fifo_rdy  ? 1'b0 : mux_fifo_vld ;

    assign mux_fifo_rdy         = (w_strans_beat_cnt==w_strans_burst_len_out)&dbus_fifo_wr;

    assign w_strans_beat_cnt_in = mux_fifo_rd  ? {{(p_W_FIFO_AWIDTH){1'b0}},1'b1} :
                                  dbus_fifo_wr ? w_strans_beat_cnt+1'b1              : w_strans_beat_cnt ;

    assign dbus_src_rdy    = w_strans_mux_out&{(p_CH_NUM){mux_fifo_vld&dbus_fifo_full_n}};

    assign o_dbus_src_rdy  = dbus_src_rdy ;

    osr_axi_ch_mux #(
        .p_DWIDTH(p_BUS_DWIDTH ),
        .p_NUM   (p_CH_NUM     )
    ) u_dbus_mux(
        .i_sel (w_strans_mux_out  ),
        .i_data(i_dbus_src_data   ),
        .o_data(dbus_fifo_wdata   )
    );

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (1                  ),
        .p_DWIDTH       (p_BUS_DWIDTH       )
    ) u_dbus_fifo(
        .i_clk          (i_clk              ),
        .i_rst_n        (i_rst_n            ),    

        .i_wr           (dbus_fifo_wr       ),
        .i_wr_data      (dbus_fifo_wdata    ),
        .o_wr_num_count (                   ),
        .o_wr_full      (                   ),
        .o_wr_full_n    (dbus_fifo_full_n   ),
        .o_wr_al_full   (                   ),
        .o_wr_al_full_n (                   ),

        .i_rd           (dbus_fifo_rd       ),
        .o_rd_data      (dbus_fifo_rdata    ),
        .o_rd_num_count (                   ),
        .o_rd_empty     (                   ),
        .o_rd_empty_n   (dbus_fifo_empty_n  ),
        .o_rd_al_empty  (                   ),
        .o_rd_al_empty_n(                   ),

        .i_clear        (1'b0               )
    ); 

    assign dbus_fifo_wr    = (|(i_dbus_src_vld&dbus_src_rdy))&dbus_fifo_full_n ;

    assign dbus_fifo_rd    = (dbus_fifo_empty_n)&((!dbus_dst_vld)|(dbus_dst_vld&i_dbus_dst_rdy));

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            dbus_dst_vld <= 1'b0 ;
        end
        else begin
            dbus_dst_vld <=  dbus_dst_vld_in ;
        end
    end

    assign dbus_dst_vld_in = (dbus_fifo_rd)   ? 1'b1 :
                              i_dbus_dst_rdy  ? 1'b0 : dbus_dst_vld ;

    assign o_dbus_dst_vld  = dbus_dst_vld ; 
    assign o_dbus_dst_data = dbus_fifo_rdata ; 
end
else begin:bypass
    assign o_dst_data = i_src_data; 
    assign o_dst_vld  = i_src_vld ; 
    assign o_src_rdy  = i_dst_rdy ; 

    assign o_dbus_dst_data = i_dbus_src_data ; 
    assign o_dbus_dst_vld  = i_dbus_src_vld  ; 
    assign o_dbus_src_rdy  = i_dbus_dst_rdy  ; 

end

endgenerate

endmodule
