//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ske_wrapper (
    input   wire                     i_s_clk              ,   
    input   wire                     i_s_rst_n            ,
    input   wire                     i_dma_clk            ,
    input   wire                     i_dma_rst_n          ,
    input   wire                     i_alg_clk            ,

    input   wire                     i_alg_rst_n          ,
    input   wire                     i_s_ahb_endian       ,
    input   wire                     i_s_hsel             ,
    input   wire  [11:0]             i_s_haddr            ,
    input   wire  [1:0]              i_s_htrans           ,
    input   wire                     i_s_hwrite           ,
    input   wire  [2:0]              i_s_hburst           ,
    input   wire  [2:0]              i_s_hsize            ,
    input   wire  [3:0]              i_s_hprot            ,
    input   wire                     i_s_hmastlock        ,
    input   wire                     i_s_hready           ,
    input   wire  [31:0]             i_s_hwdata           ,
    output  wire  [31:0]             o_s_hrdata           ,
    output  wire                     o_s_hresp            ,
    output  wire                     o_s_hreadyout        ,
    input   wire                     i_scan_mode          ,
    output  wire                     o_dma_rstart         ,
    output  wire                     o_dma_wstart         ,
    input   wire                     i_dma_rdone          ,
    input   wire                     i_dma_wdone          ,
    output  wire  [64-1:0]  o_dma_saddr          ,
    output  wire  [64-1:0]  o_dma_daddr          ,
    output  wire  [31:0]             o_dma_rlen           ,
    output  wire  [31:0]             o_dma_wlen           ,
    output  wire  [64-1:0]  o_m_w_fifo_rd_data   ,
    output  wire                     o_m_w_fifo_empty     ,
    output  wire  [4:0] o_m_w_fifo_num_count ,
    input   wire                     i_m_w_fifo_rd        ,
    input   wire  [64-1:0]  i_m_r_fifo_wr_data   ,
    input   wire                     i_m_r_fifo_wr        ,
    output  wire                     o_m_r_fifo_full      ,
    output  wire  [4:0] o_m_r_fifo_num_count ,
    output  wire                     o_dma_suspend        ,
    input   wire                     i_dma_suspend_done   ,
    output  wire                     o_dma_done_clr       ,
    output  wire                     o_irq                ,
    input   wire                     i_sp_valid           ,
    input   wire  [255:0]            i_sp_data             
);

wire [255:0] fi = i_sp_data  ;

localparam  P_SM4_2ND_DPA_EN            =  0 ;
localparam  P_AES_SECURITY_EN           =  0 ;
localparam  P_DES_SECURITY_EN           =  0 ;
localparam  P_SM4_SECURITY_EN           =  0 ;
localparam  P_DPA_LEVEL                 =  0 ;
localparam  P_AES_FI_LEVEL              =  0 ;
localparam  P_DES_FI_LEVEL              =  0 ;
localparam  P_SM4_FI_LEVEL              =  0 ;
localparam  P_AES_DUMMY_LEVEL           =  0 ;
localparam  P_DES_DUMMY_LEVEL           =  0 ;
localparam  P_SM4_DUMMY_LEVEL           =  0 ;
localparam  P_SM4_ROUND_NUM             =  2 ;

osr_ske_hp_top # (
    .p_DMA_BYTE_GRANULARITY  (16'h20          ),
    .p_DMA_RTX_FIFO_DEPTH_WIDTH  (4            ),
    .p_CONFIG_ENC_EN         (1        ),
    .p_DMA_ADDR_WIDTH        (64                      ),
    .p_DMA_DATA_WIDTH        (64                      ),
    .p_CONFIG_DEC_EN         (1        ),
    .p_CONFIG_AES_EN         (1        ),
    .p_CONFIG_SM4_EN         (1        ),
    .p_CONFIG_DES_EN         (1        ),
    .p_CONFIG_AES_KSZ_128_EN (1),
    .p_CONFIG_AES_KSZ_192_EN (1),
    .p_CONFIG_AES_KSZ_256_EN (1),
    .p_CONFIG_ECB_EN         (1        ),
    .p_CONFIG_CBC_EN         (1        ),
    .p_CONFIG_CFB_EN         (1        ),
    .p_CONFIG_OFB_EN         (1        ),
    .p_CONFIG_CTR_EN         (1        ), 
    .p_CONFIG_XTS_EN         (1        ), 
    .p_CONFIG_CMAC_EN        (1       ), 
    .p_CONFIG_CBC_MAC_EN     (1    ), 
    .p_CONFIG_GCM_EN         (1        ), 
    .p_CONFIG_CCM_EN         (1        ),
    .p_SM4_ROUND_NUM         (P_SM4_ROUND_NUM                  ),
    .p_SM4_2ND_DPA_EN        (P_SM4_2ND_DPA_EN                 ),
    .p_AES_SECURITY_EN       (P_AES_SECURITY_EN                ),   
    .p_DES_SECURITY_EN       (P_DES_SECURITY_EN                ),   
    .p_SM4_SECURITY_EN       (P_SM4_SECURITY_EN                ),   
    .p_DPA_LEVEL             (P_DPA_LEVEL                      ),   
    .p_AES_FI_LEVEL          (P_AES_FI_LEVEL                   ),   
    .p_DES_FI_LEVEL          (P_DES_FI_LEVEL                   ),   
    .p_SM4_FI_LEVEL          (P_SM4_FI_LEVEL                   ),   
    .p_AES_DUMMY_LEVEL       (P_AES_DUMMY_LEVEL                ),   
    .p_DES_DUMMY_LEVEL       (P_DES_DUMMY_LEVEL                ),   
    .p_SM4_DUMMY_LEVEL       (P_SM4_DUMMY_LEVEL                )
    ) u_ske (
    .i_s_clk                 ( i_s_clk              ), 
    .i_s_rst_n               ( i_s_rst_n            ), 
    .i_dma_clk               ( i_dma_clk            ), 
    .i_dma_rst_n             ( i_dma_rst_n          ), 
    .i_alg_clk               ( i_alg_clk            ), 
    .i_alg_rst_n             ( i_alg_rst_n          ), 
    .i_s_ahb_endian          ( i_s_ahb_endian       ), 
    .i_s_hsel                ( i_s_hsel             ), 
    .i_s_haddr               ( i_s_haddr            ), 
    .i_s_hwrite              ( i_s_hwrite           ), 
    .i_s_hsize               ( i_s_hsize            ), 
    .i_s_hburst              ( i_s_hburst           ), 
    .i_s_hprot               ( i_s_hprot            ), 
    .i_s_htrans              ( i_s_htrans           ), 
    .i_s_hmastlock           ( i_s_hmastlock        ), 
    .i_s_hready              ( i_s_hready           ), 
    .i_s_hwdata              ( i_s_hwdata           ), 
    .o_s_hreadyout           ( o_s_hreadyout        ), 
    .o_s_hresp               ( o_s_hresp            ), 
    .o_s_hrdata              ( o_s_hrdata           ), 
    .o_dma_rstart            ( o_dma_rstart         ), 
    .o_dma_wstart            ( o_dma_wstart         ), 
    .i_dma_wdone             ( i_dma_wdone          ), 
    .o_dma_saddr             ( o_dma_saddr          ), 
    .o_dma_daddr             ( o_dma_daddr          ), 
    .o_dma_rlen              ( o_dma_rlen           ), 
    .o_dma_wlen              ( o_dma_wlen           ), 
    .o_m_w_fifo_rd_data      ( o_m_w_fifo_rd_data   ), 
    .o_m_w_fifo_empty        ( o_m_w_fifo_empty     ), 
    .o_m_w_fifo_num_count    ( o_m_w_fifo_num_count ), 
    .i_m_w_fifo_rd           ( i_m_w_fifo_rd        ), 
    .i_m_r_fifo_wr_data      ( i_m_r_fifo_wr_data   ), 
    .i_m_r_fifo_wr           ( i_m_r_fifo_wr        ), 
    .o_m_r_fifo_full         ( o_m_r_fifo_full      ), 
    .o_m_r_fifo_num_count    ( o_m_r_fifo_num_count ), 
    .o_irq                   ( o_irq                ), 
    .i_sp_valid              ( i_sp_valid           ), 
    .i_sp_data               ( fi                   )  
    ); 
    assign o_dma_done_clr = 1'b0;
    assign o_dma_suspend = 1'b0;
endmodule 
