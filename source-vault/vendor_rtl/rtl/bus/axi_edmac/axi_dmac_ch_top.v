//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_ch_top #(
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
    input   wire                        i_clk                 ,
    input   wire                        i_rst_n               ,
    input   wire [ 7:0]                 i_ch_id                 ,

    output  wire                        o_ch_r_busy             ,
    output  wire                        o_ch_r_done             ,
    output  wire [5:0]                  o_ch_r_qid              ,

    input   wire                        i_ch_r_start            ,
    input   wire                        i_ch_r_suspend          ,
    input   wire [p_BUS_AWIDTH-1:0]     i_ch_raddr              ,
    input   wire [31:0]                 i_ch_rlen               ,
    input   wire [15:0]                 i_ch_r_strans_max       ,
    input   wire [31:0]                 i_ch_r_arbus            ,
    input   wire [5:0]                  i_ch_r_qid              ,

    output  wire [p_BUS_DWIDTH-1:0]     o_r_fifo_wdata          ,
    output  wire                        o_r_fifo_wr             ,
    input   wire [p_R_FIFO_AWIDTH:0]    i_r_fifo_num            ,

    output  wire [2*p_BUS_AWIDTH+61:0]  o_r_abus_data           ,
    output  wire                        o_r_abus_vld            ,
    input   wire                        i_r_abus_rdy            ,

    input   wire                        i_r_dbus_vld            ,
    input   wire [p_BUS_DWIDTH-1:0]     i_r_dbus_data           ,
    input   wire                        i_r_dbus_last           ,

    output  wire                        o_ch_w_busy             ,
    output  wire                        o_ch_w_done             ,
    output  wire [5:0]                  o_ch_w_qid              ,

    input   wire                        i_ch_w_start            ,
    input   wire                        i_ch_w_suspend          ,
    input   wire [p_BUS_AWIDTH-1:0]     i_ch_waddr              ,
    input   wire [31:0]                 i_ch_wlen               ,
    input   wire [15:0]                 i_ch_w_strans_max       ,
    input   wire [31:0]                 i_ch_w_awbus            ,
    input   wire [5:0]                  i_ch_w_qid              ,

    input   wire [p_BUS_DWIDTH-1:0]     i_w_fifo_rdata          ,
    output  wire                        o_w_fifo_rd             ,
    input   wire                        i_w_fifo_ack            ,
    input   wire [p_R_FIFO_AWIDTH:0]    i_w_fifo_num            ,

    output  wire [p_BUS_AWIDTH+61:0]    o_w_abus_data           ,
    output  wire                        o_w_abus_vld            ,
    input   wire                        i_w_abus_rdy            ,

    output  wire [p_BUS_DWIDTH-1:0]     o_w_dbus_data           ,
    output  wire                        o_w_dbus_vld            ,
    input   wire                        i_w_dbus_rdy            ,

    input   wire                        i_w_wbus_vld            ,
    input   wire                        i_w_wbus_end            ,

    input   wire                        i_w_bbus_vld            ,
    input   wire [1:0]                  i_w_bbus_data           
);

osr_axi_dmac_ch_rd #(
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
) u_axi_dmac_ch_rd(
    .i_clk             (i_clk             ),
    .i_rst_n           (i_rst_n           ),
    .i_ch_id           (i_ch_id           ),

    .o_ch_r_busy       (o_ch_r_busy       ),
    .o_ch_r_done       (o_ch_r_done       ),
    .o_ch_r_qid        (o_ch_r_qid        ),

    .i_ch_r_start      (i_ch_r_start      ),
    .i_ch_r_suspend    (i_ch_r_suspend    ),
    .i_ch_raddr        (i_ch_raddr        ),
    .i_ch_rlen         (i_ch_rlen         ),
    .i_ch_r_strans_max (i_ch_r_strans_max ),
    .i_ch_r_arbus      (i_ch_r_arbus      ),
    .i_ch_r_qid        (i_ch_r_qid        ),

    .o_r_fifo_wdata    (o_r_fifo_wdata    ),
    .o_r_fifo_wr       (o_r_fifo_wr       ),
    .i_r_fifo_num      (i_r_fifo_num      ),

    .o_r_abus_data     (o_r_abus_data     ),
    .o_r_abus_vld      (o_r_abus_vld      ),
    .i_r_abus_rdy      (i_r_abus_rdy      ),

    .i_r_dbus_vld      (i_r_dbus_vld      ),
    .i_r_dbus_data     (i_r_dbus_data     ),
    .i_r_dbus_last     (i_r_dbus_last     )
);

osr_axi_dmac_ch_wr #(
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
) u_axi_dmac_ch_wr(
    .i_clk             (i_clk             ),
    .i_rst_n           (i_rst_n           ),
    .i_ch_id           (i_ch_id           ),

    .o_ch_w_busy       (o_ch_w_busy       ),
    .o_ch_w_done       (o_ch_w_done       ),
    .o_ch_w_qid        (o_ch_w_qid        ),

    .i_ch_w_start      (i_ch_w_start      ),
    .i_ch_w_suspend    (i_ch_w_suspend    ),
    .i_ch_waddr        (i_ch_waddr        ),
    .i_ch_wlen         (i_ch_wlen         ),
    .i_ch_w_strans_max (i_ch_w_strans_max ),
    .i_ch_w_awbus      (i_ch_w_awbus      ),
    .i_ch_w_qid        (i_ch_w_qid        ),

    .i_w_fifo_rdata    (i_w_fifo_rdata    ),
    .o_w_fifo_rd       (o_w_fifo_rd       ),
    .i_w_fifo_ack      (i_w_fifo_ack      ),
    .i_w_fifo_num      (i_w_fifo_num      ),

    .o_w_abus_data     (o_w_abus_data     ),
    .o_w_abus_vld      (o_w_abus_vld      ),
    .i_w_abus_rdy      (i_w_abus_rdy      ),

    .o_w_dbus_data     (o_w_dbus_data     ),
    .o_w_dbus_vld      (o_w_dbus_vld      ),
    .i_w_dbus_rdy      (i_w_dbus_rdy      ),

    .i_w_wbus_vld      (i_w_wbus_vld      ),
    .i_w_wbus_end      (i_w_wbus_end      ),

    .i_w_bbus_vld      (i_w_bbus_vld      ),
    .i_w_bbus_data     (i_w_bbus_data     )
);

endmodule
