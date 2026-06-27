//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_ch_array_top #(
    parameter [31:0] p_BUS_DWIDTH             =512,
    parameter [31:0] p_BUS_AWIDTH             = 64,
    parameter [31:0] p_R_BMEM_AWIDTH          =  8,
    parameter [31:0] p_W_BMEM_AWIDTH          =  8,
    parameter [31:0] p_R_SMEM_AWIDTH          =  3,
    parameter [31:0] p_W_SMEM_AWIDTH          =  3,
    parameter [31:0] p_R_FIFO_AWIDTH          =  3,
    parameter [31:0] p_W_FIFO_AWIDTH          =  3,
    parameter [31:0] p_CH_NUM                 =  8,
    parameter [ 0:0] p_W_ALIGN                =  0,
    parameter [ 0:0] p_R_ALIGN                =  0,
    parameter [31:0] p_W_ALIGN_STEP           =  8,
    parameter [31:0] p_R_ALIGN_STEP           =  8,
    parameter [ 0:0] p_TIMING_ENHANCE         =  0
)(
    input   wire                                   i_clk                   ,
    input   wire                                   i_rst_n                 ,

    input   wire                                   i_r_bus_err             ,
    output  wire                                   o_r_bus_err             ,
    output  wire [5:0]                             o_r_bus_qid             ,

    input   wire                                   i_w_bus_err             ,
    output  wire                                   o_w_bus_err             ,
    output  wire [5:0]                             o_w_bus_qid             ,

    input   wire [p_CH_NUM*1-1:0]                  i_ch_suspend            ,
    output  wire [p_CH_NUM*1-1:0]                  o_ch_suspend_done       ,

    output  wire [p_CH_NUM*1-1:0]                  o_ch_r_busy             ,
    output  wire [p_CH_NUM*1-1:0]                  o_ch_r_done             ,

    input   wire [p_CH_NUM*1-1:0]                  i_ch_r_start            ,
    input   wire [p_CH_NUM*p_BUS_AWIDTH-1:0]       i_ch_raddr              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_rlen               ,
    input   wire [p_CH_NUM*16-1:0]                 i_ch_r_strans_max       ,
    input   wire [p_CH_NUM*6-1:0]                  i_ch_r_qid              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_r_arbus            ,

    output  wire [p_CH_NUM*p_BUS_DWIDTH-1:0]       o_r_fifo_wdata          ,
    output  wire [p_CH_NUM*1-1:0]                  o_r_fifo_wr             ,
    input   wire [p_CH_NUM*(p_R_FIFO_AWIDTH+1)-1:0]i_r_fifo_num            ,

    output  wire [2*p_BUS_AWIDTH+61:0]             o_r_abus_data           ,
    output  wire                                   o_r_abus_vld            ,
    input   wire                                   i_r_abus_rdy            ,

    input   wire                                   i_r_dbus_vld            ,
    input   wire [7:0]                             i_r_dbus_rid            ,
    input   wire [p_BUS_DWIDTH-1:0]                i_r_dbus_data           ,
    input   wire                                   i_r_dbus_last           ,

    output  wire [p_CH_NUM*1-1:0]                  o_ch_w_busy             ,
    output  wire [p_CH_NUM*1-1:0]                  o_ch_w_done             ,

    input   wire [p_CH_NUM*1-1:0]                  i_ch_w_start            ,
    input   wire [p_CH_NUM*p_BUS_AWIDTH-1:0]       i_ch_waddr              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_wlen               ,
    input   wire [p_CH_NUM*16-1:0]                 i_ch_w_strans_max       ,
    input   wire [p_CH_NUM*6-1:0]                  i_ch_w_qid              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_w_awbus            ,

    input   wire [p_CH_NUM*p_BUS_DWIDTH-1:0]       i_w_fifo_rdata          ,
    output  wire [p_CH_NUM*1-1:0]                  o_w_fifo_rd             ,
    input   wire [p_CH_NUM*(p_R_FIFO_AWIDTH+1)-1:0]i_w_fifo_num            ,

    output  wire [p_BUS_AWIDTH+61:0]               o_w_abus_data           ,
    output  wire                                   o_w_abus_vld            ,
    input   wire                                   i_w_abus_rdy            ,

    output  wire [p_BUS_DWIDTH-1:0]                o_w_dbus_data           ,
    output  wire                                   o_w_dbus_vld            ,
    input   wire                                   i_w_dbus_rdy            ,

    input   wire [7:0]                             i_w_wbus_wid            ,
    input   wire                                   i_w_wbus_vld            ,
    input   wire                                   i_w_wbus_end            ,

    input   wire [7:0]                             i_w_bbus_bid            ,
    input   wire                                   i_w_bbus_vld            ,
    input   wire [1:0]                             i_w_bbus_data           
);

genvar  i;
integer ii;
wire [p_CH_NUM*8-1:0]                  ch_id         ;

wire [p_CH_NUM*6-1:0]                  ch_r_qid      ;
wire [p_CH_NUM*6-1:0]                  ch_w_qid      ;

reg                                    r_bus_err     ;
reg                                    w_bus_err     ;

wire [p_CH_NUM-1:0]                    r_bus_err_sel ;
wire [p_CH_NUM-1:0]                    w_bus_err_sel ;

reg                                    r_bus_err_out ; 
reg  [5:0]                             r_bus_qid_out ; 
reg                                    w_bus_err_out ; 
reg  [5:0]                             w_bus_qid_out ; 
wire                                   r_bus_err_pre ; 
wire [5:0]                             r_bus_qid_pre ; 
wire                                   w_bus_err_pre ; 
wire [5:0]                             w_bus_qid_pre ; 

reg  [p_CH_NUM-1:0]                    ch_suspend_1d   ;
wire [p_CH_NUM-1:0]                    ch_suspend_flag_in;
reg  [p_CH_NUM-1:0]                    ch_suspend_flag   ;

reg                                    r_dbus_vld    ;
reg  [7:0]                             r_dbus_rid    ;
reg  [p_BUS_DWIDTH-1:0]                r_dbus_data   ;
reg                                    r_dbus_last   ;
wire [p_CH_NUM-1:0]                    r_dbus_ch_vld ;

wire [p_CH_NUM*(2*p_BUS_AWIDTH+62)-1:0]r_abus_data   ;
wire [p_CH_NUM*1-1:0]                  r_abus_vld    ;
wire [p_CH_NUM*1-1:0]                  r_abus_rdy    ;

reg  [7:0]                             w_wbus_wid    ;
reg                                    w_wbus_vld    ;
reg                                    w_wbus_end    ;
wire [p_CH_NUM-1:0]                    w_wbus_ch_vld ;

reg  [7:0]                             w_bbus_bid    ;
reg                                    w_bbus_vld    ;
reg  [1:0]                             w_bbus_data   ;
wire [p_CH_NUM-1:0]                    w_bbus_ch_vld ;

wire [p_CH_NUM*(p_BUS_AWIDTH+62)-1:0]  w_abus_data   ;
wire [p_CH_NUM*1-1:0]                  w_abus_vld    ;
wire [p_CH_NUM*1-1:0]                  w_abus_rdy    ;

wire [p_CH_NUM*(p_BUS_DWIDTH)-1:0]     w_dbus_data   ;
wire [p_CH_NUM*1-1:0]                  w_dbus_vld    ;
wire [p_CH_NUM*1-1:0]                  w_dbus_rdy    ;

wire [p_CH_NUM*1-1:0]                  w_fifo_rd     ;
wire [p_CH_NUM*1-1:0]                  w_fifo_ack    ;

    wire [p_CH_NUM-1:0]                w_req         ;
    wire [p_CH_NUM-1:0]                w_grant       ;
    reg  [p_CH_NUM-1:0]                w_base        ;
    wire [p_CH_NUM-1:0]                w_base_in     ;
    wire [p_CH_NUM*2-1:0]              w_double_req  ;
    wire [p_CH_NUM*2-1:0]              w_double_grant;

generate
if(p_CH_NUM>1) begin:gen_arbiter

    assign w_double_req   = {w_req,w_req};
    assign w_double_grant = w_double_req&~(w_double_req-{{(p_CH_NUM){1'b0}},w_base});
    assign w_grant        = w_double_grant[p_CH_NUM-1:0]|w_double_grant[2*p_CH_NUM-1:p_CH_NUM] ;

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            w_base <= {{(p_CH_NUM-1){1'b0}},{1'b1}};
        end
        else begin
            w_base <=  w_base_in ;
        end
    end

    assign w_base_in = (|w_req) ? {w_grant[p_CH_NUM-2:0],w_grant[p_CH_NUM-1]} : w_base ;

    assign w_req       =  w_fifo_rd ;
    assign w_fifo_ack  =  w_grant   ;

    assign o_w_fifo_rd =  w_grant   ;
end
else begin:bypass
    assign o_w_fifo_rd =  w_fifo_rd ;
    assign w_fifo_ack  =  w_fifo_rd ;
end
endgenerate

generate
if(1) begin:suspend_done_gen
    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            ch_suspend_1d   <= {(p_CH_NUM){1'b0}};
            ch_suspend_flag <= {(p_CH_NUM){1'b0}};
        end
        else begin
            ch_suspend_1d  <=  i_ch_suspend ;
            ch_suspend_flag <= ch_suspend_flag_in;
        end
    end   

    for(i=0;i<p_CH_NUM;i=i+1) begin:suspend_done_ch
        assign ch_suspend_flag_in[i] =    i_ch_suspend[i] ? 1'b1 : 
                                     o_ch_suspend_done[i] ? 1'b0 : ch_suspend_flag[i] ;
        assign o_ch_suspend_done[i] = ~ch_suspend_flag[i] ? 1'b0 :
                                      o_ch_w_busy[i]&(~o_ch_r_busy[i]) ? o_ch_w_done[i] :
                                      o_ch_r_busy[i] ? o_ch_r_done[i] : ch_suspend_1d[i] ;
    end
end
endgenerate

always@(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        r_dbus_vld    <= 1'b0 ;
        r_dbus_rid    <= 8'b0 ; 
        r_dbus_data   <= {(p_BUS_DWIDTH){1'b0}} ;
        r_dbus_last   <= 1'b0 ;

        w_wbus_wid    <= 8'b0 ; 
        w_wbus_vld    <= 1'b0 ; 
        w_wbus_end    <= 1'b0 ;

        w_bbus_bid    <= 8'b0 ; 
        w_bbus_vld    <= 1'b0 ;
        w_bbus_data   <= 2'b0 ;

        r_bus_err     <= 1'b0 ;

        w_bus_err     <= 1'b0 ;

        r_bus_err_out <= 1'b0 ; 
        r_bus_qid_out <= 6'b0 ; 
        w_bus_err_out <= 1'b0 ; 
        w_bus_qid_out <= 6'b0 ; 
    end
    else begin
        r_dbus_vld    <= i_r_dbus_vld  ;
        r_dbus_rid    <= i_r_dbus_rid  ;
        r_dbus_data   <= i_r_dbus_data ;
        r_dbus_last   <= i_r_dbus_last ;

        w_wbus_wid    <= i_w_wbus_wid  ;
        w_wbus_vld    <= i_w_wbus_vld  ;
        w_wbus_end    <= i_w_wbus_end  ;

        w_bbus_bid    <= i_w_bbus_bid  ;
        w_bbus_vld    <= i_w_bbus_vld  ;
        w_bbus_data   <= i_w_bbus_data ;

        r_bus_err     <= i_r_bus_err   ;

        w_bus_err     <= i_w_bus_err   ;

        r_bus_err_out <= r_bus_err_pre ; 
        r_bus_qid_out <= r_bus_qid_pre ; 
        w_bus_err_out <= w_bus_err_pre ; 
        w_bus_qid_out <= w_bus_qid_pre ;

    end
end

generate
if(1) begin:bus_valid_gen
    for(i=0;i<p_CH_NUM;i=i+1) begin:addr_bus_valid_logic
        assign r_dbus_ch_vld[i] = (r_dbus_rid[7:0]==i)&r_dbus_vld;
        assign w_wbus_ch_vld[i] = (w_wbus_wid[7:0]==i)&w_wbus_vld;
        assign w_bbus_ch_vld[i] = (w_bbus_bid[7:0]==i)&w_bbus_vld;
    end
end
endgenerate

  assign r_bus_err_sel = r_dbus_ch_vld&{(p_CH_NUM){r_bus_err}} ; 

  osr_axi_ch_mux#(.p_DWIDTH (6       ),
              .p_NUM    (p_CH_NUM) 
  ) u_rbus_err_mux(
      .i_sel (r_bus_err_sel        ),
      .i_data(ch_r_qid             ),
      .o_data(r_bus_qid_pre        )
  ); 
  assign r_bus_err_pre = r_bus_err ;

  assign w_bus_err_sel = w_bbus_ch_vld&{(p_CH_NUM){w_bus_err}} ; 

  osr_axi_ch_mux#(.p_DWIDTH (6       ),
              .p_NUM    (p_CH_NUM) 
  ) u_wbus_err_mux(
      .i_sel (w_bus_err_sel        ),
      .i_data(ch_w_qid             ),
      .o_data(w_bus_qid_pre        )
  ); 
  assign w_bus_err_pre = w_bus_err ;

  assign o_r_bus_err = r_bus_err_out ; 
  assign o_r_bus_qid = r_bus_qid_out ; 
  assign o_w_bus_err = w_bus_err_out ; 
  assign o_w_bus_qid = w_bus_qid_out ;

generate
if(1) begin:gen_multi_channel
    for(i=0;i<p_CH_NUM;i=i+1) begin:multi_channel
        assign ch_id[i*8+:8] = i + 8'b0;

        osr_axi_dmac_ch_top #(
            .p_BUS_DWIDTH    (p_BUS_DWIDTH    ),
            .p_BUS_AWIDTH    (p_BUS_AWIDTH    ),
            .p_R_BMEM_AWIDTH (p_R_BMEM_AWIDTH ),
            .p_W_BMEM_AWIDTH (p_W_BMEM_AWIDTH ),
            .p_R_SMEM_AWIDTH (p_R_SMEM_AWIDTH ),
            .p_W_SMEM_AWIDTH (p_W_SMEM_AWIDTH ),
            .p_R_FIFO_AWIDTH (p_R_FIFO_AWIDTH ),
            .p_W_FIFO_AWIDTH (p_W_FIFO_AWIDTH ),
            .p_CH_NUM        (p_CH_NUM        ),
            .p_W_ALIGN       (p_W_ALIGN       ),
            .p_R_ALIGN       (p_R_ALIGN       ),
            .p_W_ALIGN_STEP  (p_W_ALIGN_STEP  ),
            .p_R_ALIGN_STEP  (p_R_ALIGN_STEP  )
        ) u_axi_dmac_ch_top(
            .i_clk             (i_clk                                                        ),
            .i_rst_n           (i_rst_n                                                      ),
            .i_ch_id           ({ch_id[i*8+:8]}                                              ),

            .o_ch_r_busy       (o_ch_r_busy[i]                                               ),
            .o_ch_r_done       (o_ch_r_done[i]                                               ),
            .o_ch_r_qid        (ch_r_qid[i*6+:6]                                             ),

            .i_ch_r_start      (i_ch_r_start[i]                                              ),
            .i_ch_r_suspend    (i_ch_suspend[i]                                              ),
            .i_ch_raddr        (i_ch_raddr[i*p_BUS_AWIDTH+:p_BUS_AWIDTH]                     ),
            .i_ch_rlen         (i_ch_rlen[i*32+:32]                                          ),
            .i_ch_r_strans_max (i_ch_r_strans_max[i*16+:16]                                  ),
            .i_ch_r_arbus      (i_ch_r_arbus[i*32+:32]                                       ),
            .i_ch_r_qid        (i_ch_r_qid[i*6+:6]                                           ),

            .o_r_fifo_wdata    (o_r_fifo_wdata[i*p_BUS_DWIDTH+:p_BUS_DWIDTH]                 ),
            .o_r_fifo_wr       (o_r_fifo_wr[i]                                               ),
            .i_r_fifo_num      (i_r_fifo_num[i*(p_R_FIFO_AWIDTH+1)+:(p_R_FIFO_AWIDTH+1)]     ),

            .o_r_abus_data     (r_abus_data[i*(2*p_BUS_AWIDTH+62)+:(2*p_BUS_AWIDTH+62)]      ),
            .o_r_abus_vld      (r_abus_vld[i]                                                ),
            .i_r_abus_rdy      (r_abus_rdy[i]                                                ),

            .i_r_dbus_vld      (r_dbus_ch_vld[i]                                             ),
            .i_r_dbus_data     (r_dbus_data                                                  ),
            .i_r_dbus_last     (r_dbus_last                                                  ),

            .o_ch_w_busy       (o_ch_w_busy[i]                                               ),
            .o_ch_w_done       (o_ch_w_done[i]                                               ),
            .o_ch_w_qid        (ch_w_qid[i*6+:6]                                             ),

            .i_ch_w_start      (i_ch_w_start[i]                                              ),
            .i_ch_w_suspend    (i_ch_suspend[i]                                              ),
            .i_ch_waddr        (i_ch_waddr[i*p_BUS_AWIDTH+:p_BUS_AWIDTH]                     ),
            .i_ch_wlen         (i_ch_wlen[i*32+:32]                                          ),
            .i_ch_w_strans_max (i_ch_w_strans_max[i*16+:16]                                  ),
            .i_ch_w_awbus      (i_ch_w_awbus[i*32+:32]                                       ),
            .i_ch_w_qid        (i_ch_w_qid[i*6+:6]                                           ),

            .i_w_fifo_rdata    (i_w_fifo_rdata[i*p_BUS_DWIDTH+:p_BUS_DWIDTH]                 ),
            .o_w_fifo_rd       (w_fifo_rd[i]                                                 ),
            .i_w_fifo_ack      (w_fifo_ack[i]                                                ),
            .i_w_fifo_num      (i_w_fifo_num[i*(p_W_FIFO_AWIDTH+1)+:(p_W_FIFO_AWIDTH+1)]     ),

            .o_w_abus_data     (w_abus_data[i*(p_BUS_AWIDTH+62)+:(p_BUS_AWIDTH+62)]          ),
            .o_w_abus_vld      (w_abus_vld[i]                                                ),
            .i_w_abus_rdy      (w_abus_rdy[i]                                                ),

            .o_w_dbus_data     (w_dbus_data[i*p_BUS_DWIDTH+:p_BUS_DWIDTH]                    ),
            .o_w_dbus_vld      (w_dbus_vld[i]                                                ),
            .i_w_dbus_rdy      (w_dbus_rdy[i]                                                ),

            .i_w_wbus_vld      (w_wbus_ch_vld[i]                                             ),
            .i_w_wbus_end      (w_wbus_end                                                   ),

            .i_w_bbus_vld      (w_bbus_ch_vld[i]                                             ),
            .i_w_bbus_data     (w_bbus_data                                                  )
        );
    end
end
endgenerate

osr_axi_dmac_ch_arbmux #(
    .p_CH_NUM  (p_CH_NUM       ),
    .p_CH_WIDTH(2*p_BUS_AWIDTH+62)
) u_dmac_rd_abus_arbmux(
    .i_clk      (i_clk         ),
    .i_rst_n    (i_rst_n       ),

    .o_dst_data (o_r_abus_data ),
    .o_dst_vld  (o_r_abus_vld  ),
    .i_dst_rdy  (i_r_abus_rdy  ),

    .i_src_data (r_abus_data   ),
    .i_src_vld  (r_abus_vld    ),
    .o_src_rdy  (r_abus_rdy    )
);

osr_axi_dmac_wr_arbmux #(
    .p_BUS_DWIDTH    (p_BUS_DWIDTH    ),
    .p_BUS_AWIDTH    (p_BUS_AWIDTH    ),
    .p_W_FIFO_AWIDTH (p_W_FIFO_AWIDTH ),    
    .p_W_SMEM_AWIDTH (p_W_SMEM_AWIDTH ),
    .p_CH_NUM        (p_CH_NUM        ),
    .p_CH_WIDTH      (p_BUS_AWIDTH+62 )
) u_dmac_wr_arbmux(
    .i_clk           (i_clk       ),
    .i_rst_n         (i_rst_n     ),

    .o_dst_data      (o_w_abus_data ),
    .o_dst_vld       (o_w_abus_vld  ),
    .i_dst_rdy       (i_w_abus_rdy  ),

    .i_src_data      (w_abus_data   ),
    .i_src_vld       (w_abus_vld    ),
    .o_src_rdy       (w_abus_rdy    ),

    .o_dbus_dst_data (o_w_dbus_data ),
    .o_dbus_dst_vld  (o_w_dbus_vld  ),
    .i_dbus_dst_rdy  (i_w_dbus_rdy  ),

    .i_dbus_src_data (w_dbus_data   ),
    .i_dbus_src_vld  (w_dbus_vld    ),
    .o_dbus_src_rdy  (w_dbus_rdy    )
);

endmodule
