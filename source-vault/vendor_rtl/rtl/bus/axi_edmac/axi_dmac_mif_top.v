//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_mif_top #(
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

    input   wire                          i_clk        ,
    input   wire                          i_rst_n      ,

    input   wire [15:0]                   i_dma_to          ,
    input   wire [7:0]                    i_dma_rost_max    ,
    input   wire [7:0]                    i_dma_rburst_max  ,
    input   wire [7:0]                    i_dma_wost_max    ,
    input   wire [7:0]                    i_dma_wburst_max  ,

    input   wire [2*p_BUS_AWIDTH+61:0]    i_r_abus_data  ,
    input   wire                          i_r_abus_vld   ,
    output  wire                          o_r_abus_rdy   ,

    output  wire                          o_r_dbus_vld   ,
    output  wire [7:0]                    o_r_dbus_rid   ,
    output  wire [p_BUS_DWIDTH-1:0]       o_r_dbus_data  ,
    output  wire                          o_r_dbus_last  ,

    output  wire                          o_r_rato       ,
    output  wire                          o_r_rdto       ,
    output  wire                          o_r_bus_err    ,

    input   wire [p_BUS_AWIDTH+61:0]      i_w_abus_data  ,
    input   wire                          i_w_abus_vld   ,
    output  wire                          o_w_abus_rdy   ,

    input   wire [p_BUS_DWIDTH-1:0]       i_w_dbus_data  ,
    input   wire                          i_w_dbus_vld   ,
    output  wire                          o_w_dbus_rdy   ,

    output  wire [7:0]                    o_w_wbus_wid   ,
    output  wire                          o_w_wbus_vld   ,
    output  wire                          o_w_wbus_end   ,

    output  wire [7:0]                    o_w_bbus_bid   ,
    output  wire                          o_w_bbus_vld   ,
    output  wire [1:0]                    o_w_bbus_data  ,

    output  wire                          o_w_wato       ,
    output  wire                          o_w_wdto       ,
    output  wire                          o_w_bus_err    ,

    output  wire [ 1:0]                   o_w_awbar      ,
    output  wire [ 2:0]                   o_w_awsnoop    ,
    output  wire [ 1:0]                   o_w_awdomain   ,

    output  wire [13:0]                   o_w_awid       ,
    output  wire [p_BUS_AWIDTH-1:0]       o_w_awaddr     ,
    output  wire [ 7:0]                   o_w_awlen      ,
    output  wire [ 2:0]                   o_w_awsize     ,
    output  wire [ 1:0]                   o_w_awburst    ,
    output  wire [ 1:0]                   o_w_awlock     ,
    output  wire [ 3:0]                   o_w_awcache    ,
    output  wire [ 2:0]                   o_w_awprot     ,
    output  wire [ 3:0]                   o_w_awqos      ,
    output  wire [ 3:0]                   o_w_awregion   ,
    output  wire                          o_w_awvalid    ,
    input   wire                          i_w_awready    ,

    output  wire [ 7:0]                   o_w_wid        ,
    output  wire [p_BUS_DWIDTH-1:0]       o_w_wdata      ,
    output  wire [p_BUS_DWIDTH/8-1:0]     o_w_wstrb      ,
    output  wire                          o_w_wlast      ,
    output  wire                          o_w_wvalid     ,
    input   wire                          i_w_wready     ,

    input   wire [ 7:0]                   i_w_bid        ,
    input   wire [ 1:0]                   i_w_bresp      ,
    input   wire                          i_w_bvalid     ,
    output  wire                          o_w_bready     ,

    output  wire [ 1:0]                   o_r_arbar      ,
    output  wire [ 3:0]                   o_r_arsnoop    ,
    output  wire [ 1:0]                   o_r_ardomain   ,

    output  wire [13:0]                   o_r_arid       ,
    output  wire [p_BUS_AWIDTH-1:0]       o_r_araddr     ,
    output  wire [ 7:0]                   o_r_arlen      ,
    output  wire [ 2:0]                   o_r_arsize     ,
    output  wire [ 1:0]                   o_r_arburst    ,
    output  wire [ 1:0]                   o_r_arlock     ,
    output  wire [ 3:0]                   o_r_arcache    ,
    output  wire [ 2:0]                   o_r_arprot     ,
    output  wire [ 3:0]                   o_r_arqos      ,
    output  wire [ 3:0]                   o_r_arregion   ,
    output  wire                          o_r_arvalid    ,
    input   wire                          i_r_arready    , 

    input   wire [ 7:0]                   i_r_rid        ,
    input   wire [p_BUS_DWIDTH-1:0]       i_r_rdata      ,
    input   wire [ 1:0]                   i_r_rresp      ,
    input   wire                          i_r_rlast      ,
    input   wire                          i_r_rvalid     ,
    output  wire                          o_r_rready     
);

osr_axi_dmac_mif_rd #(
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
) u_axi_mif_rd(

    .i_clk           (i_clk                ),
    .i_rst_n         (i_rst_n              ),
    .i_dma_to        (i_dma_to             ),
    .i_dma_rost_max  (i_dma_rost_max       ),
    .i_dma_rburst_max(i_dma_rburst_max     ),
    .o_r_rato        (o_r_rato             ),    
    .o_r_rdto        (o_r_rdto             ),    
    .o_r_bus_err     (o_r_bus_err          ),  

    .i_r_abus_data   (i_r_abus_data        ),
    .i_r_abus_vld    (i_r_abus_vld         ),
    .o_r_abus_rdy    (o_r_abus_rdy         ),

    .o_r_dbus_vld    (o_r_dbus_vld         ),
    .o_r_dbus_rid    (o_r_dbus_rid         ),
    .o_r_dbus_data   (o_r_dbus_data        ),
    .o_r_dbus_last   (o_r_dbus_last        ),

    .o_r_arbar       (o_r_arbar            ),
    .o_r_arsnoop     (o_r_arsnoop          ), 
    .o_r_ardomain    (o_r_ardomain         ), 

    .o_r_arid        (o_r_arid             ),
    .o_r_araddr      (o_r_araddr           ),
    .o_r_arlen       (o_r_arlen            ),
    .o_r_arsize      (o_r_arsize           ),
    .o_r_arburst     (o_r_arburst          ),
    .o_r_arlock      (o_r_arlock           ),
    .o_r_arcache     (o_r_arcache          ),
    .o_r_arprot      (o_r_arprot           ),
    .o_r_arqos       (o_r_arqos            ),
    .o_r_arregion    (o_r_arregion         ),
    .o_r_arvalid     (o_r_arvalid          ),
    .i_r_arready     (i_r_arready          ), 

    .i_r_rid         (i_r_rid              ),
    .i_r_rdata       (i_r_rdata            ),
    .i_r_rresp       (i_r_rresp            ),
    .i_r_rlast       (i_r_rlast            ),
    .i_r_rvalid      (i_r_rvalid           ),
    .o_r_rready      (o_r_rready           )
);

osr_axi_dmac_mif_wr #(
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
) u_axi_mif_wr(

    .i_clk                (i_clk                ),
    .i_rst_n              (i_rst_n              ),

    .i_dma_to             (i_dma_to             ),
    .i_dma_wost_max       (i_dma_wost_max       ),
    .i_dma_wburst_max     (i_dma_wburst_max     ),
    .o_w_wato             (o_w_wato             ),    
    .o_w_wdto             (o_w_wdto             ),    
    .o_w_bus_err          (o_w_bus_err          ),  

    .i_w_abus_data        (i_w_abus_data        ),
    .i_w_abus_vld         (i_w_abus_vld         ),
    .o_w_abus_rdy         (o_w_abus_rdy         ),

    .i_w_dbus_data        (i_w_dbus_data        ),
    .i_w_dbus_vld         (i_w_dbus_vld         ),
    .o_w_dbus_rdy         (o_w_dbus_rdy         ),

    .o_w_wbus_wid         (o_w_wbus_wid         ),
    .o_w_wbus_vld         (o_w_wbus_vld         ),
    .o_w_wbus_end         (o_w_wbus_end         ),

    .o_w_bbus_bid         (o_w_bbus_bid         ),
    .o_w_bbus_vld         (o_w_bbus_vld         ),
    .o_w_bbus_data        (o_w_bbus_data        ),

    .o_w_awbar            (o_w_awbar            ),
    .o_w_awsnoop          (o_w_awsnoop          ), 
    .o_w_awdomain         (o_w_awdomain         ), 

    .o_w_awid             (o_w_awid             ),
    .o_w_awaddr           (o_w_awaddr           ),
    .o_w_awlen            (o_w_awlen            ),
    .o_w_awsize           (o_w_awsize           ),
    .o_w_awburst          (o_w_awburst          ),
    .o_w_awlock           (o_w_awlock           ),
    .o_w_awcache          (o_w_awcache          ),
    .o_w_awprot           (o_w_awprot           ),
    .o_w_awqos            (o_w_awqos            ),
    .o_w_awregion         (o_w_awregion         ),
    .o_w_awvalid          (o_w_awvalid          ),
    .i_w_awready          (i_w_awready          ),

    .o_w_wid              (o_w_wid              ),
    .o_w_wdata            (o_w_wdata            ),
    .o_w_wstrb            (o_w_wstrb            ),
    .o_w_wlast            (o_w_wlast            ),
    .o_w_wvalid           (o_w_wvalid           ),
    .i_w_wready           (i_w_wready           ),

    .i_w_bid              (i_w_bid              ),
    .i_w_bresp            (i_w_bresp            ),
    .i_w_bvalid           (i_w_bvalid           ),
    .o_w_bready           (o_w_bready           )
);

endmodule
