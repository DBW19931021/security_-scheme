//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_edmac_top # (
    parameter [31:0] p_BUS_DWIDTH             =512,
    parameter [31:0] p_BUS_AWIDTH             = 64,
    parameter [31:0] p_R_BMEM_AWIDTH          =  3,
    parameter [31:0] p_W_BMEM_AWIDTH          =  3,
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
    input   wire                                   i_clk                 ,
    input   wire                                   i_rst_n               ,

    input   wire [15:0]                            i_dma_to              ,
    input   wire [7:0]                             i_dma_rost_max        ,
    input   wire [7:0]                             i_dma_rburst_max      ,
    input   wire [7:0]                             i_dma_wost_max        ,
    input   wire [7:0]                             i_dma_wburst_max      ,

    input   wire [p_CH_NUM*1-1:0]                  i_r_w_suspend           ,
    output  wire [p_CH_NUM*1-1:0]                  o_r_w_suspend_done      ,

    output  wire                                   o_r_bus_err             ,
    output  wire [5:0]                             o_r_bus_qid             ,

    output  wire                                   o_w_bus_err             ,
    output  wire [5:0]                             o_w_bus_qid             ,

    output  wire [3:0]                             o_r_w_bus_to            ,

    output  wire [p_CH_NUM*1-1:0]                  o_ch_r_busy             ,
    output  wire [p_CH_NUM*1-1:0]                  o_ch_r_done             ,
    output  wire [p_CH_NUM*1-1:0]                  o_ch_r_done_level       ,

    input   wire [p_CH_NUM*1-1:0]                  i_ch_r_start            ,
    input   wire [p_CH_NUM*p_BUS_AWIDTH-1:0]       i_ch_raddr              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_rlen               ,
    input   wire [p_CH_NUM*16-1:0]                 i_ch_r_strans_max       ,
    input   wire [p_CH_NUM*6-1:0]                  i_ch_r_qid              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_r_arbus            ,

    output  wire [p_CH_NUM*p_BUS_DWIDTH-1:0]       o_r_fifo_wdata          ,
    output  wire [p_CH_NUM*1-1:0]                  o_r_fifo_wr             ,
    input   wire [p_CH_NUM*(p_R_FIFO_AWIDTH+1)-1:0]i_r_fifo_num            ,

    output  wire [p_CH_NUM*1-1:0]                  o_ch_w_busy             ,
    output  wire [p_CH_NUM*1-1:0]                  o_ch_w_done             ,
    output  wire [p_CH_NUM*1-1:0]                  o_ch_w_done_level       ,

    input   wire [p_CH_NUM*1-1:0]                  i_ch_w_start            ,
    input   wire [p_CH_NUM*p_BUS_AWIDTH-1:0]       i_ch_waddr              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_wlen               ,
    input   wire [p_CH_NUM*16-1:0]                 i_ch_w_strans_max       ,
    input   wire [p_CH_NUM*6-1:0]                  i_ch_w_qid              ,
    input   wire [p_CH_NUM*32-1:0]                 i_ch_w_awbus            ,

    input   wire [p_CH_NUM*p_BUS_DWIDTH-1:0]       i_w_fifo_rdata          ,
    output  wire [p_CH_NUM*1-1:0]                  o_w_fifo_rd             ,
    input   wire [p_CH_NUM*(p_W_FIFO_AWIDTH+1)-1:0]i_w_fifo_num            ,

    input   wire [p_CH_NUM*1-1:0]                  i_r_w_done_clear        ,

    output  wire [ 1:0]                               o_w_awbar             ,
    output  wire [ 2:0]                               o_w_awsnoop           ,
    output  wire [ 1:0]                               o_w_awdomain          ,

    output  wire [13:0]                               o_w_awid              ,
    output  wire [p_BUS_AWIDTH-1:0]                   o_w_awaddr            ,
    output  wire [ 7:0]                               o_w_awlen             ,
    output  wire [ 2:0]                               o_w_awsize            ,
    output  wire [ 1:0]                               o_w_awburst           ,
    output  wire [ 1:0]                               o_w_awlock            ,
    output  wire [ 3:0]                               o_w_awcache           ,
    output  wire [ 2:0]                               o_w_awprot            ,
    output  wire [ 3:0]                               o_w_awqos             ,
    output  wire [ 3:0]                               o_w_awregion          ,
    output  wire                                      o_w_awvalid           ,
    input   wire                                      i_w_awready           ,

    output  wire [ 7:0]                               o_w_wid               ,
    output  wire [p_BUS_DWIDTH-1:0]                   o_w_wdata             ,
    output  wire [p_BUS_DWIDTH/8-1:0]                 o_w_wstrb             ,
    output  wire                                      o_w_wlast             ,
    output  wire                                      o_w_wvalid            ,
    input   wire                                      i_w_wready            ,

    input   wire [ 7:0]                               i_w_bid               ,
    input   wire [ 1:0]                               i_w_bresp             ,
    input   wire                                      i_w_bvalid            ,
    output  wire                                      o_w_bready            ,

    output  wire [ 1:0]                               o_r_arbar             ,
    output  wire [ 3:0]                               o_r_arsnoop           ,
    output  wire [ 1:0]                               o_r_ardomain          ,

    output  wire [13:0]                               o_r_arid              ,
    output  wire [p_BUS_AWIDTH-1:0]                   o_r_araddr            ,
    output  wire [ 7:0]                               o_r_arlen             ,
    output  wire [ 2:0]                               o_r_arsize            ,
    output  wire [ 1:0]                               o_r_arburst           ,
    output  wire [ 1:0]                               o_r_arlock            ,
    output  wire [ 3:0]                               o_r_arcache           ,
    output  wire [ 2:0]                               o_r_arprot            ,
    output  wire [ 3:0]                               o_r_arqos             ,
    output  wire [ 3:0]                               o_r_arregion          ,
    output  wire                                      o_r_arvalid           ,
    input   wire                                      i_r_arready           , 

    input   wire [ 7:0]                               i_r_rid               ,
    input   wire [p_BUS_DWIDTH-1:0]                   i_r_rdata             ,
    input   wire [ 1:0]                               i_r_rresp             ,
    input   wire                                      i_r_rlast             ,
    input   wire                                      i_r_rvalid            ,
    output  wire                                      o_r_rready            
);

    localparam [2:0]ADDR_OFFSET_WIDTH = (p_BUS_DWIDTH/8<=4  ) ? 3'd2 :    
                                        (p_BUS_DWIDTH/8<=8  ) ? 3'd3 :    
                                        (p_BUS_DWIDTH/8<=16 ) ? 3'd4 :    
                                        (p_BUS_DWIDTH/8<=32 ) ? 3'd5 :    
                                        (p_BUS_DWIDTH/8<=64 ) ? 3'd6 : 3'd0 ;

    wire [2*p_BUS_AWIDTH+61:0]             r_abus_data   ; 
    wire                                   r_abus_vld    ; 
    wire                                   r_abus_rdy    ; 

    wire                                   r_dbus_vld    ; 
    wire [7:0]                             r_dbus_rid    ; 
    wire [p_BUS_DWIDTH-1:0]                r_dbus_data   ; 
    wire                                   r_dbus_last   ; 

    wire [p_BUS_AWIDTH+61:0]               w_abus_data   ;
    wire                                   w_abus_vld    ;
    wire                                   w_abus_rdy    ;

    wire [p_BUS_DWIDTH-1:0]                w_dbus_data   ;
    wire                                   w_dbus_vld    ;
    wire                                   w_dbus_rdy    ;

    wire [7:0]                             w_wbus_wid    ;
    wire                                   w_wbus_vld    ;
    wire                                   w_wbus_end    ;

    wire [7:0]                             w_bbus_bid    ;
    wire                                   w_bbus_vld    ;
    wire [1:0]                             w_bbus_data   ;

    wire                                   r_bus_err     ;
    wire                                   w_bus_err     ;

    wire                                   r_rato        ;
    wire                                   r_rdto        ;
    wire                                   w_wato        ;
    wire                                   w_wdto        ;

    wire [p_CH_NUM*1-1:0]                  ch_r_done     ;
    wire [p_CH_NUM*1-1:0]                  ch_w_done     ;

    assign o_r_w_bus_to[3:0]  = {r_rato, r_rdto, w_wato, w_wdto }  ;

    osr_axi_dmac_ch_array_top #(
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
    ) u_axi_ch_array_top(
        .i_clk             (i_clk                                               ),
        .i_rst_n           (i_rst_n                                             ),

        .i_r_bus_err       (r_bus_err                                           ),
        .o_r_bus_err       (o_r_bus_err                                         ),
        .o_r_bus_qid       (o_r_bus_qid                                         ),

        .i_w_bus_err       (w_bus_err                                           ),
        .o_w_bus_err       (o_w_bus_err                                         ),
        .o_w_bus_qid       (o_w_bus_qid                                         ),

        .i_ch_suspend      (i_r_w_suspend                                       ),
        .o_ch_suspend_done (o_r_w_suspend_done                                  ),
        .o_ch_r_busy       (o_ch_r_busy                                         ),
        .o_ch_r_done       (  ch_r_done                                         ),

        .i_ch_r_start      (i_ch_r_start                                        ),
        .i_ch_raddr        (i_ch_raddr                                          ),
        .i_ch_rlen         (i_ch_rlen                                           ),
        .i_ch_r_strans_max (i_ch_r_strans_max                                   ),
        .i_ch_r_qid        (i_ch_r_qid                                          ),
        .i_ch_r_arbus      (i_ch_r_arbus                                        ),

        .o_r_fifo_wdata    (o_r_fifo_wdata                                      ),
        .o_r_fifo_wr       (o_r_fifo_wr                                         ),
        .i_r_fifo_num      (i_r_fifo_num                                        ),

        .o_r_abus_data     (r_abus_data                                         ),
        .o_r_abus_vld      (r_abus_vld                                          ),
        .i_r_abus_rdy      (r_abus_rdy                                          ),

        .i_r_dbus_rid      (r_dbus_rid                                          ),
        .i_r_dbus_vld      (r_dbus_vld                                          ),
        .i_r_dbus_data     (r_dbus_data                                         ),
        .i_r_dbus_last     (r_dbus_last                                         ),

        .o_ch_w_busy       (o_ch_w_busy                                         ),
        .o_ch_w_done       (  ch_w_done                                         ),

        .i_ch_w_start      (i_ch_w_start                                        ),
        .i_ch_waddr        (i_ch_waddr                                          ),
        .i_ch_wlen         (i_ch_wlen                                           ),
        .i_ch_w_strans_max (i_ch_w_strans_max                                   ),
        .i_ch_w_qid        (i_ch_w_qid                                          ),
        .i_ch_w_awbus      (i_ch_w_awbus                                        ),

        .i_w_fifo_rdata    (i_w_fifo_rdata                                      ),
        .o_w_fifo_rd       (o_w_fifo_rd                                         ),
        .i_w_fifo_num      (i_w_fifo_num                                        ),

        .o_w_abus_data     (w_abus_data                                         ),
        .o_w_abus_vld      (w_abus_vld                                          ),
        .i_w_abus_rdy      (w_abus_rdy                                          ),

        .o_w_dbus_data     (w_dbus_data                                         ),
        .o_w_dbus_vld      (w_dbus_vld                                          ),
        .i_w_dbus_rdy      (w_dbus_rdy                                          ),

        .i_w_wbus_wid      (w_wbus_wid                                          ),
        .i_w_wbus_vld      (w_wbus_vld                                          ),
        .i_w_wbus_end      (w_wbus_end                                          ),

        .i_w_bbus_bid      (w_bbus_bid                                          ),
        .i_w_bbus_vld      (w_bbus_vld                                          ),
        .i_w_bbus_data     (w_bbus_data                                         )
);

    osr_axi_dmac_mif_top #(
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
    ) u_axi_mif_top(
        .i_clk                   (i_clk                 ),
        .i_rst_n                 (i_rst_n               ),

        .i_dma_to                (i_dma_to              ),
        .i_dma_rost_max          (i_dma_rost_max        ),
        .i_dma_rburst_max        (i_dma_rburst_max      ),
        .i_dma_wost_max          (i_dma_wost_max        ),
        .i_dma_wburst_max        (i_dma_wburst_max      ),

        .i_r_abus_data           (r_abus_data           ),
        .i_r_abus_vld            (r_abus_vld            ),
        .o_r_abus_rdy            (r_abus_rdy            ),

        .o_r_dbus_vld            (r_dbus_vld            ),
        .o_r_dbus_rid            (r_dbus_rid            ),
        .o_r_dbus_data           (r_dbus_data           ),
        .o_r_dbus_last           (r_dbus_last           ),
        .o_r_bus_err             (r_bus_err             ),

        .o_r_rato                (r_rato                ),    
        .o_r_rdto                (r_rdto                ),    

        .i_w_abus_data           (w_abus_data           ),
        .i_w_abus_vld            (w_abus_vld            ),
        .o_w_abus_rdy            (w_abus_rdy            ),

        .i_w_dbus_data           (w_dbus_data           ),
        .i_w_dbus_vld            (w_dbus_vld            ),
        .o_w_dbus_rdy            (w_dbus_rdy            ),

        .o_w_wbus_wid            (w_wbus_wid            ),
        .o_w_wbus_vld            (w_wbus_vld            ),
        .o_w_wbus_end            (w_wbus_end            ),

        .o_w_bbus_bid            (w_bbus_bid            ),
        .o_w_bbus_vld            (w_bbus_vld            ),
        .o_w_bbus_data           (w_bbus_data           ),
        .o_w_bus_err             (w_bus_err             ),

        .o_w_wato                (w_wato                ),
        .o_w_wdto                (w_wdto                ),

        .o_w_awbar               (o_w_awbar             ),
        .o_w_awsnoop             (o_w_awsnoop           ), 
        .o_w_awdomain            (o_w_awdomain          ), 

        .o_w_awid                (o_w_awid              ),
        .o_w_awaddr              (o_w_awaddr            ),
        .o_w_awlen               (o_w_awlen             ),
        .o_w_awsize              (o_w_awsize            ),
        .o_w_awburst             (o_w_awburst           ),
        .o_w_awlock              (o_w_awlock            ),
        .o_w_awcache             (o_w_awcache           ),
        .o_w_awprot              (o_w_awprot            ),
        .o_w_awqos               (o_w_awqos             ),
        .o_w_awregion            (o_w_awregion          ),
        .o_w_awvalid             (o_w_awvalid           ),
        .i_w_awready             (i_w_awready           ),

        .o_w_wid                 (o_w_wid               ),
        .o_w_wdata               (o_w_wdata             ),
        .o_w_wstrb               (o_w_wstrb             ),
        .o_w_wlast               (o_w_wlast             ),
        .o_w_wvalid              (o_w_wvalid            ),
        .i_w_wready              (i_w_wready            ),

        .i_w_bid                 (i_w_bid               ),
        .i_w_bresp               (i_w_bresp             ),
        .i_w_bvalid              (i_w_bvalid            ),
        .o_w_bready              (o_w_bready            ),

        .o_r_arbar               (o_r_arbar             ),
        .o_r_arsnoop             (o_r_arsnoop           ), 
        .o_r_ardomain            (o_r_ardomain          ),

        .o_r_arid                (o_r_arid              ),
        .o_r_araddr              (o_r_araddr            ),
        .o_r_arlen               (o_r_arlen             ),
        .o_r_arsize              (o_r_arsize            ),
        .o_r_arburst             (o_r_arburst           ),
        .o_r_arlock              (o_r_arlock            ),
        .o_r_arcache             (o_r_arcache           ),
        .o_r_arprot              (o_r_arprot            ),
        .o_r_arqos               (o_r_arqos             ),
        .o_r_arregion            (o_r_arregion          ),
        .o_r_arvalid             (o_r_arvalid           ),
        .i_r_arready             (i_r_arready           ), 

        .i_r_rid                 (i_r_rid               ),
        .i_r_rdata               (i_r_rdata             ),
        .i_r_rresp               (i_r_rresp             ),
        .i_r_rlast               (i_r_rlast             ),
        .i_r_rvalid              (i_r_rvalid            ),
        .o_r_rready              (o_r_rready            )
);

reg [p_CH_NUM*1-1:0] r_r_done;
reg [p_CH_NUM*1-1:0] r_w_done;

always @(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        r_r_done    <= {p_CH_NUM{1'h0}}      ;  
        r_w_done    <= {p_CH_NUM{1'h0}}      ; 
    end else begin
        r_r_done    <= ch_r_done | (r_r_done & (~i_r_w_done_clear));
        r_w_done    <= ch_w_done | (r_w_done & (~i_r_w_done_clear));
    end
end 
assign o_ch_r_done_level  = ch_r_done | r_r_done ;
assign o_ch_w_done_level  = ch_w_done | r_w_done ;

assign o_ch_r_done  = ch_r_done ;
assign o_ch_w_done  = ch_w_done ;

endmodule
