//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_bus_wrapper # (
    parameter   P_DMA_AW        =  32                   ,
    parameter   P_DMA_DW        =  32
)
(
    input  wire                 i_sclk                  ,
    input  wire                 i_gclk                  ,

    input  wire                 i_scan_mode             ,

    input  wire                 i_cpu_clk               ,
    input  wire                 i_sys_clk               ,
    input  wire                 i_emu_clk               ,
    input  wire                 i_boot_clk              ,

    input  wire                 i_kmu_clk               ,

    input  wire                 i_mbox_clk              ,

    input wire                  i_trng_clk              ,

    input wire                  i_hash0_clk             ,
    input wire                  i_hash0_dma_clk         ,

    input wire                  i_ske0_clk              ,
    input wire                  i_ske0_dma_clk          ,

    input wire                  i_pke_clk               ,

    input wire                  i_ahb_dma_clk           ,

    input wire                  i_axi_dma_clk           ,

    input  wire                 i_rdc_clk               ,
    input  wire                 i_wdt_clk               ,
    input  wire                 i_uart_clk              ,
    input  wire                 i_tim_clk               ,

    input  wire                 i_crc_clk               ,

    input  wire                 i_srst_n                ,
    input  wire                 i_hwrst_n               ,

    input  wire                 i_cpu_rst_n             ,
    input  wire                 i_sys_rst_n             ,
    input  wire                 i_emu_rst_n             ,
    input  wire                 i_boot_rst_n            ,

    input  wire                 i_kmu_rst_n             ,

    input  wire                 i_mbox_rst_n            ,

    input  wire                 i_trng_rst_n            ,

    input  wire                 i_hash0_rst_n            ,
    input  wire                 i_hash0_dma_rst_n        ,

    input  wire                 i_ske0_rst_n             ,
    input  wire                 i_ske0_dma_rst_n         ,

    input  wire                 i_pke_rst_n             ,

    input  wire                 i_ahb_dma_rst_n         ,

    input  wire                 i_axi_dma_rst_n         ,

    input  wire                 i_rdc_rst_n             ,
    input  wire                 i_wdt_rst_n             ,
    input  wire                 i_uart_rst_n            ,
    input  wire                 i_tim_rst_n             ,

    input  wire                 i_crc_rst_n             ,

    output wire                 o_clock_en              ,
    output wire                 o_reset_trng_n          ,

    output wire                 o_keep_logic            ,

    input  wire                 i_m_hsel                ,
    input  wire [31:0]          i_m_haddr               ,
    input  wire [ 1:0]          i_m_htrans              ,
    input  wire                 i_m_hwrite              ,
    input  wire [ 2:0]          i_m_hburst              ,
    input  wire [ 2:0]          i_m_hsize               ,
    input  wire [ 3:0]          i_m_hprot               ,
    input  wire                 i_m_hmastlock           ,
    input  wire                 i_m_hready              ,
    input  wire [31:0]          i_m_hwdata              ,
    output wire [31:0]          o_m_hrdata              ,
    output wire [ 1:0]          o_m_hresp               ,
    output wire                 o_m_hreadyout           ,

    output wire                 o_otp_hsel              ,
    output wire [31:0]          o_otp_haddr             ,
    output wire [1:0]           o_otp_htrans            ,
    output wire                 o_otp_hwrite            ,
    output wire [2:0]           o_otp_hburst            ,
    output wire [2:0]           o_otp_hsize             ,
    output wire [3:0]           o_otp_hprot             ,
    output wire                 o_otp_hmastlock         ,
    output wire                 o_otp_hready            ,
    output wire [31:0]          o_otp_hwdata            ,
    input  wire [31:0]          i_otp_hrdata            ,
    input  wire [1:0]           i_otp_hresp             ,
    input  wire                 i_otp_hreadyout         ,

    input  wire [31:0]          i_mbox_soc_sta          ,

    input  wire                 i_mbox_soc_hsel         ,
    input  wire [31:0]          i_mbox_soc_haddr        ,
    input  wire [ 1:0]          i_mbox_soc_htrans       ,
    input  wire                 i_mbox_soc_hwrite       ,
    input  wire [ 2:0]          i_mbox_soc_hburst       ,
    input  wire [ 2:0]          i_mbox_soc_hsize        ,
    input  wire [ 3:0]          i_mbox_soc_hprot        ,
    input  wire                 i_mbox_soc_hmastlock    ,
    input  wire                 i_mbox_soc_hready       ,
    input  wire [31:0]          i_mbox_soc_hwdata       ,
    output wire [31:0]          o_mbox_soc_hrdata       ,
    output wire [ 1:0]          o_mbox_soc_hresp        ,
    output wire                 o_mbox_soc_hreadyout    ,

    output wire                 o_cfg_hsel              ,
    output wire [31:0]          o_cfg_haddr             ,
    output wire [1:0]           o_cfg_htrans            ,
    output wire                 o_cfg_hwrite            ,
    output wire [2:0]           o_cfg_hburst            ,
    output wire [2:0]           o_cfg_hsize             ,
    output wire [3:0]           o_cfg_hprot             ,
    output wire                 o_cfg_hmastlock         ,
    output wire                 o_cfg_hready            ,
    output wire [31:0]          o_cfg_hwdata            ,
    input  wire [31:0]          i_cfg_hrdata            ,
    input  wire [1:0]           i_cfg_hresp             ,
    input  wire                 i_cfg_hreadyout         ,

    output wire [3:0]           o_dma_m_awid            ,
    output wire [P_DMA_AW-1:0]  o_dma_m_awaddr          ,
    output wire [7:0]           o_dma_m_awlen           ,
    output wire [2:0]           o_dma_m_awsize          ,
    output wire [1:0]           o_dma_m_awburst         ,
    output wire [1:0]           o_dma_m_awlock          ,
    output wire [3:0]           o_dma_m_awcache         ,
    output wire [2:0]           o_dma_m_awprot          ,
    output wire [3:0]           o_dma_m_awqos           ,
    output wire [3:0]           o_dma_m_awregion        ,
    output wire                 o_dma_m_awvalid         ,
    input  wire                 i_dma_m_awready         ,

    output wire [3:0]           o_dma_m_wid             ,
    output wire [P_DMA_DW-1:0]  o_dma_m_wdata           ,
    output wire [P_DMA_DW/8-1:0]o_dma_m_wstrb           ,
    output wire                 o_dma_m_wlast           ,
    output wire                 o_dma_m_wvalid          ,
    input  wire                 i_dma_m_wready          ,

    input  wire [3:0]           i_dma_m_bid             ,
    input  wire [1:0]           i_dma_m_bresp           ,
    input  wire                 i_dma_m_bvalid          ,
    output wire                 o_dma_m_bready          ,

    output wire [3:0]           o_dma_m_arid            ,
    output wire [P_DMA_AW-1:0]  o_dma_m_araddr          ,
    output wire [7:0]           o_dma_m_arlen           ,
    output wire [2:0]           o_dma_m_arsize          ,
    output wire [1:0]           o_dma_m_arburst         ,
    output wire [1:0]           o_dma_m_arlock          ,
    output wire [3:0]           o_dma_m_arcache         ,
    output wire [2:0]           o_dma_m_arprot          ,
    output wire [3:0]           o_dma_m_arqos           ,
    output wire [3:0]           o_dma_m_arregion        ,
    output wire                 o_dma_m_arvalid         ,
    input  wire                 i_dma_m_arready         ,

    input  wire [3:0]           i_dma_m_rid             ,
    input  wire [P_DMA_DW-1:0]  i_dma_m_rdata           ,
    input  wire [1:0]           i_dma_m_rresp           ,
    input  wire                 i_dma_m_rlast           ,
    input  wire                 i_dma_m_rvalid          ,
    output wire                 o_dma_m_rready          ,

    output wire [1:0]           o_dma_m_awbar           ,
    output wire [2:0]           o_dma_m_awsnoop         ,
    output wire [1:0]           o_dma_m_awdomain        ,

    output wire [1:0]           o_dma_m_arbar           ,
    output wire [3:0]           o_dma_m_arsnoop         ,
    output wire [1:0]           o_dma_m_ardomain        ,

    output wire                 o_dma_m_hsel            ,
    output wire [32-1:0]        o_dma_m_haddr           ,
    output wire [1:0]           o_dma_m_htrans          ,
    output wire                 o_dma_m_hwrite          ,
    output wire [2:0]           o_dma_m_hburst          ,
    output wire [2:0]           o_dma_m_hsize           ,
    output wire [3:0]           o_dma_m_hprot           ,
    output wire                 o_dma_m_hmastlock       ,
    output wire [32-1:0]        o_dma_m_hwdata          ,
    input  wire [32-1:0]        i_dma_m_hrdata          ,
    input  wire [1:0]           i_dma_m_hresp           ,
    input  wire                 i_dma_m_hreadyout       ,

    output wire                 o_kbuf_CSB              ,
    output wire [7:0]           o_kbuf_A                ,
    output wire [3:0]           o_kbuf_WEB              ,
    output wire [31:0]          o_kbuf_DI               ,
    input  wire [31:0]          i_kbuf_DO               ,

    output wire                 o_pke_sram0_CK          ,
    output wire                 o_pke_sram0_CSAN        ,
    output wire [8:0]           o_pke_sram0_A           ,
    output wire                 o_pke_sram0_CSBN        ,
    output wire [8:0]           o_pke_sram0_B           ,

    output wire                 o_pke_sram1_CK          ,
    output wire                 o_pke_sram1_CSAN        ,
    output wire [8:0]           o_pke_sram1_A           ,
    output wire                 o_pke_sram1_CSBN        ,
    output wire [8:0]           o_pke_sram1_B           ,

    output wire                 o_pke_sram2_CK          ,
    output wire                 o_pke_sram2_CSAN        ,
    output wire [8:0]           o_pke_sram2_A           ,
    output wire                 o_pke_sram2_CSBN        ,
    output wire [8:0]           o_pke_sram2_B           ,

    output wire                 o_pke_sram3_CK          ,
    output wire                 o_pke_sram3_CSAN        ,
    output wire [8:0]           o_pke_sram3_A           ,
    output wire                 o_pke_sram3_CSBN        ,
    output wire [8:0]           o_pke_sram3_B           ,
    input  wire [71:0]          i_pke_sram0_DO          ,
    output wire [71:0]          o_pke_sram0_DI          ,
    input  wire [71:0]          i_pke_sram1_DO          ,
    output wire [71:0]          o_pke_sram1_DI          ,
    input  wire [71:0]          i_pke_sram2_DO          ,
    output wire [71:0]          o_pke_sram2_DI          ,
    input  wire [71:0]          i_pke_sram3_DO          ,
    output wire [71:0]          o_pke_sram3_DI          ,

    output wire [31:0]          o_clk_ctrl0             ,
    output wire [31:0]          o_clk_ctrl1             ,
    output wire [31:0]          o_alg_clk_ctrl0         ,
    output wire [31:0]          o_alg_clk_ctrl1         ,
    output wire [31:0]          o_alg_clk_ctrl2         ,
    output wire [31:0]          o_alg_clk_ctrl3         ,
    output wire [31:0]          o_alg_clk_ctrl4         ,
    output wire [31:0]          o_alg_clk_ctrl5         ,
    output wire [31:0]          o_alg_clk_ctrl6         ,

    output wire [31:0]          o_rst_ctrl0             ,
    output wire [31:0]          o_rst_ctrl1             ,
    output wire [31:0]          o_alg_rst_ctrl0         ,
    output wire [31:0]          o_alg_rst_ctrl1         ,
    output wire [31:0]          o_alg_rst_ctrl2         ,
    output wire [31:0]          o_alg_rst_ctrl3         ,
    output wire [31:0]          o_alg_rst_ctrl4         ,
    output wire [31:0]          o_alg_rst_ctrl5         ,
    output wire [31:0]          o_alg_rst_ctrl6         ,

    input  wire [31:0]          i_sensor                ,
    input  wire [31:0]          i_soc_status            ,
    input  wire [31:0]          i_soc_err               ,
    output wire [63:0]          o_hsm_status            ,

    output wire                 o_ahb_dma_en            ,

    output wire                 o_boot_hw_ok            ,
    output wire                 o_boot_hw_err           ,

    output wire                 o_otp_ahb_if_sel        ,
    output wire                 o_ahb_nvm_rsp_err_en    ,
    output wire                 o_ahb_cfg_rsp_err_en    ,
    output wire                 o_ahb_otp_rsp_err_en    ,
    output wire                 o_ahb_soc_rsp_err_en    ,
    output wire [15:0]          o_mbox_soc_irq          ,
    output wire [63:0]          o_mbox_ram_ba           ,

    input  wire                 i_uart_rxd              ,
    output wire                 o_uart_txd_oe           ,
    output wire                 o_uart_txd              ,

    output wire [63:0]          o_irq2cpu               ,

    output wire [31:0]          o_emu_err_sensor        ,
    output wire [63:0]          o_emu_err_hw            ,
    output wire [63:0]          o_emu_err_fw            ,
    output wire                 o_emu_resetn_all        ,
    output wire                 o_emu_resetn_noboot     ,
    output wire                 o_emu_resetn_cpu        ,

    output wire [3:0]           o_trng_ro_clk           ,
    output wire [3:0]           o_trng_ro_out           ,

    output wire                 o_bus_cipher_en         ,
    output wire [31:0]          o_bus_cipher_key        ,
    output wire [31:0]          o_bus_cipher_nce        ,

    output wire                 o_ram_cipher_en         ,
    output wire [31:0]          o_ram_cipher_key        ,
    output wire [31:0]          o_ram_cipher_nce        ,

    output wire                 o_nvm_cipher_bypass     ,

    output wire [31:0]          o_irom_ecc_cfg          ,
    output wire [31:0]          o_iram_ecc_cfg          ,
    output wire [31:0]          o_dram_ecc_cfg          ,
    output wire [31:0]          o_kmu_ram_ecc_cfg       ,

    output wire [31:0]          o_cpu_cfg               ,

    input  wire                 i_mem_ecc_1b_irom       ,
    input  wire                 i_mem_ecc_1b_iram       ,
    input  wire                 i_mem_ecc_1b_dram       ,

    input  wire                 i_mem_ecc_1b_kmu        ,
    input  wire                 i_mem_ecc_mb_irom       ,
    input  wire                 i_mem_ecc_mb_iram       ,
    input  wire                 i_mem_ecc_mb_dram       ,
    input  wire                 i_mem_ecc_mb_kmu        ,

    input  wire [14-1:0]          i_mem_ecc_addr_irom     ,
    input  wire [16-1:0]          i_mem_ecc_addr_iram     ,
    input  wire [14-1:0]          i_mem_ecc_addr_dram     ,
    input  wire [16:0]          i_mem_ecc_addr_kmu      ,

    input  wire                 i_soc_err_ahb_mem       ,
    input  wire                 i_soc_err_ahb_otp       ,
    input  wire                 i_soc_err_ahb_nvm       ,
    input  wire                 i_soc_err_ahb_cfg       ,
    input  wire                 i_soc_err_axi_dma_wr    ,
    input  wire                 i_soc_err_axi_dma_rd    ,

    output wire                 o_ahb_bus_pr_en         ,
    input  wire                 i_ipre_pchk_err         ,
    input  wire                 i_dpre_pchk_err         ,
    input  wire                 i_spre_pchk_err         ,
    input  wire                 i_ahbdpre_pchk_err      ,
    input  wire                 i_iromprc_pchk_err      ,
    input  wire                 i_iramprc_pchk_err      ,
    input  wire                 i_dramprc_pchk_err      ,
    input  wire                 i_ahbprc_pchk_err       ,
    input  wire                 i_nvmprc_pchk_err       ,
    input  wire                 i_socprc_pchk_err       ,

    input  wire                 i_cpu_hart_halted       ,
    input  wire                 i_cpu_wfi               ,

    output wire                 o_trng_rdy              ,
    output wire                 o_trng_alarm            ,

    output wire                 o_patch_en              ,
    output wire                 o_patch_info_vld        ,
    output wire [11:0]          o_patch_info_addr       ,
    output wire [31:0]          o_patch_otp_rdata       ,

    input  wire                 i_mem_init_done         ,

    output wire [127:0]         o_soc_dbg_en_128b       ,
    output wire                 o_hsm_dbg_en
);

localparam P_ALG_MASK = 0;

localparam P_OTP_BUS_EN = 1;

localparam TRNG_DRBG_SM4_EN = 1;

localparam [31:0] AXID_P_CH_NUM  = 2  ;

localparam     MBOX_P_S2H_NOTE_N = 1         ; 
localparam     MBOX_P_H2S_NOTE_N = 1         ;
localparam     MBOX_P_MBOX_NUM   = 16 ;
localparam     MBOX_P_HAVE_FUSA  = 0         ;

localparam     PKE_P_RAM_WIDTH   = 72;
localparam     PKE_P_NONE_       = 0;

localparam [31:0] AXID_p_BUS_DWIDTH     = P_DMA_DW         ;
localparam [31:0] AXID_p_BUS_AWIDTH     = P_DMA_AW         ;
localparam [31:0] AXID_p_R_BMEM_AWIDTH  = 3                ;
localparam [31:0] AXID_p_W_BMEM_AWIDTH  = 3                ;
localparam [31:0] AXID_p_R_SMEM_AWIDTH  = 3                ;
localparam [31:0] AXID_p_W_SMEM_AWIDTH  = 3                ;
localparam [31:0] AXID_p_R_FIFO_AWIDTH  = 4;
localparam [31:0] AXID_p_W_FIFO_AWIDTH  = 4;
localparam [31:0] AXID_p_CH_NUM         = AXID_P_CH_NUM    ;
localparam [0:0]  AXID_p_W_ALIGN        = 0                ;
localparam [0:0]  AXID_p_R_ALIGN        = 0                ;
localparam [31:0] AXID_p_W_ALIGN_STEP   = 8                ;
localparam [31:0] AXID_p_R_ALIGN_STEP   = 8                ;
localparam [0:0]  AXID_p_TIMING_ENHANCE = 0                ;

localparam     AHBD_p_DATA_WIDTH          = P_DMA_DW         ;
localparam     AHBD_p_ADDR_WIDTH          = P_DMA_AW         ;
localparam     AHBD_p_CH_FIFO_EXIST       = 0                ;
localparam     AHBD_p_LINK_LIST_EN        = 0                ;
localparam     AHBD_p_AHB_SIMPLE          = 1                ;
localparam     AHBD_p_CDC_MS_EN           = 0                ;
localparam     AHBD_p_CDC_MP_EN           = 0                ;
localparam     AHBD_p_R_FIFO_DEPTH_WIDTH  = 4;
localparam     AHBD_p_W_FIFO_DEPTH_WIDTH  = 4;
localparam     AHBD_p_CH_FIFO_DEPTH_WIDTH = 0                ;

localparam     AHBD2H_p_DATA_WIDTH          = P_DMA_DW         ;
localparam     AHBD2H_p_ADDR_WIDTH          = P_DMA_AW         ;
localparam     AHBD2H_p_CH_FIFO_EXIST       = 0                ;
localparam     AHBD2H_p_LINK_LIST_EN        = 0                ;
localparam     AHBD2H_p_AHB_SIMPLE          = 1                ;
localparam     AHBD2H_p_CDC_MS_EN           = 0                ;
localparam     AHBD2H_p_CDC_MP_EN           = 0                ;
localparam     AHBD2H_p_R_FIFO_DEPTH_WIDTH  = 4;
localparam     AHBD2H_p_W_FIFO_DEPTH_WIDTH  = 4;
localparam     AHBD2H_p_CH_FIFO_DEPTH_WIDTH = 0                ;

localparam     AHBD2S_p_DATA_WIDTH          = P_DMA_DW         ;
localparam     AHBD2S_p_ADDR_WIDTH          = P_DMA_AW         ;
localparam     AHBD2S_p_CH_FIFO_EXIST       = 0                ;
localparam     AHBD2S_p_LINK_LIST_EN        = 0                ;
localparam     AHBD2S_p_AHB_SIMPLE          = 1                ;
localparam     AHBD2S_p_CDC_MS_EN           = 0                ;
localparam     AHBD2S_p_CDC_MP_EN           = 0                ;
localparam     AHBD2S_p_R_FIFO_DEPTH_WIDTH  = 4;
localparam     AHBD2S_p_W_FIFO_DEPTH_WIDTH  = 4;
localparam     AHBD2S_p_CH_FIFO_DEPTH_WIDTH = 0                ;

localparam     AMUX2H_P_PORT0_ENABLE = 1        ;
localparam     AMUX2H_P_PORT1_ENABLE = 1        ;
localparam     AMUX2H_P_PORT2_ENABLE = 0        ;
localparam     AMUX2H_P_AWIDTH       = P_DMA_AW ;
localparam     AMUX2H_P_DWIDTH       = P_DMA_DW ;

localparam     AMUXOUT_P_PORT0_ENABLE = 1        ;
localparam     AMUXOUT_P_PORT1_ENABLE = 1        ;
localparam     AMUXOUT_P_PORT2_ENABLE = 1        ;
localparam     AMUXOUT_P_AWIDTH       = P_DMA_AW ;
localparam     AMUXOUT_P_DWIDTH       = P_DMA_DW ;

localparam     AMUX_P_PORT0_ENABLE = 1        ;
localparam     AMUX_P_PORT1_ENABLE = 1        ;
localparam     AMUX_P_PORT2_ENABLE = 0        ;
localparam     AMUX_P_AWIDTH       = P_DMA_AW ;
localparam     AMUX_P_DWIDTH       = P_DMA_DW ;

localparam     ADNSIZ_HMASTER_WIDTH       = 1 ;
localparam     ADNSIZ_ERR_BURST_BLOCK_ALL = 1 ;

wire            w_hash_i_dma_suspend_done    ;
wire            w_hash_i_dma_rdone           ;
wire            w_hash_i_dma_wdone           ;
wire            w_hash_i_dma_r_fifo_wr       ;
wire [64-1:0]     w_hash_i_dma_r_fifo_wr_data  ;
wire            w_hash_i_dma_w_fifo_rd       ;

wire                    w_ske_i_dma_suspend_done   ;
wire                    w_ske_i_dma_rdone          ;
wire                    w_ske_i_dma_wdone          ;
wire                    w_ske_i_m_w_fifo_rd        ;
wire [64-1:0]  w_ske_i_m_r_fifo_wr_data   ;
wire                    w_ske_i_m_r_fifo_wr        ;

wire [AXID_p_CH_NUM*1-1:0]                        w_axid_i_ch_r_start     ; 
wire [AXID_p_CH_NUM*AXID_p_BUS_AWIDTH-1:0]        w_axid_i_ch_raddr       ; 
wire [AXID_p_CH_NUM*32-1:0]                       w_axid_i_ch_rlen        ; 
wire [AXID_p_CH_NUM*16-1:0]                       w_axid_i_ch_r_strans_max; 
wire [AXID_p_CH_NUM*6-1:0]                        w_axid_i_ch_r_qid       ; 
wire [AXID_p_CH_NUM*32-1:0]                       w_axid_i_ch_r_arbus     ; 
wire [AXID_p_CH_NUM*(AXID_p_R_FIFO_AWIDTH+1)-1:0] w_axid_i_r_fifo_num     ; 
wire [AXID_p_CH_NUM*1-1:0]                        w_axid_i_ch_w_start     ; 
wire [AXID_p_CH_NUM*AXID_p_BUS_AWIDTH-1:0]        w_axid_i_ch_waddr       ; 
wire [AXID_p_CH_NUM*32-1:0]                       w_axid_i_ch_wlen        ; 
wire [AXID_p_CH_NUM*16-1:0]                       w_axid_i_ch_w_strans_max; 
wire [AXID_p_CH_NUM*6-1:0]                        w_axid_i_ch_w_qid       ; 
wire [AXID_p_CH_NUM*32-1:0]                       w_axid_i_ch_w_awbus     ; 
wire [AXID_p_CH_NUM*AXID_p_BUS_DWIDTH-1:0]        w_axid_i_w_fifo_rdata   ; 
wire [AXID_p_CH_NUM*(AXID_p_W_FIFO_AWIDTH+1)-1:0] w_axid_i_w_fifo_num     ; 
wire [AXID_p_CH_NUM*1-1:0]                        w_axid_i_r_w_done_clear ; 
wire [AXID_p_CH_NUM*1-1:0]                        w_axid_i_suspend        ; 

wire [11:0] offset_att = 12'd256 / 12'd10 * 12'd8 ;

reg [1:0]       sys_o_dma_wr_ctl_buf ;
reg             sys_o_dc_dma_sel_buf ;

wire [31:0]    abus_o_m_hrdata    ;
wire           abus_o_m_hready    ;
wire [1:0]     abus_o_m_hresp     ;
wire [3:0]     abus_o_hprot       ;
wire [31:0]    abus_o_haddr       ;
wire [1:0]     abus_o_htrans      ;
wire           abus_o_hwrite      ;
wire [2:0]     abus_o_hsize       ;
wire [2:0]     abus_o_hburst      ;
wire [31:0]    abus_o_hwdata      ;
wire           abus_o_hsel0       ;
wire           abus_o_hsel1       ;
wire           abus_o_hsel2       ;
wire           abus_o_hsel3       ;
wire           abus_o_hsel4       ;
wire           abus_o_hsel5       ;
wire           abus_o_hsel6       ;
wire           abus_o_hsel7       ;
wire           abus_o_hsel8       ;
wire           abus_o_hsel9       ;
wire           abus_o_hsel10      ;
wire           abus_o_hsel11      ;
wire           abus_o_hsel12      ;
wire           abus_o_hsel13      ;
wire           abus_o_hsel14      ;
wire           abus_o_hsel15      ;

wire [31:0]     sys_o_hrdata             ;
wire [1:0]      sys_o_hresp              ;
wire            sys_o_hreadyout          ;
wire [31:0]     sys_o_clk_ctrl0          ;
wire [31:0]     sys_o_clk_ctrl1          ;
wire [31:0]     sys_o_alg_clk_ctrl0      ;
wire [31:0]     sys_o_alg_clk_ctrl1      ;
wire [31:0]     sys_o_alg_clk_ctrl2      ;
wire [31:0]     sys_o_alg_clk_ctrl3      ;
wire [31:0]     sys_o_alg_clk_ctrl4      ;
wire [31:0]     sys_o_alg_clk_ctrl5      ;
wire [31:0]     sys_o_alg_clk_ctrl6      ;
wire [31:0]     sys_o_rst_ctrl0          ;
wire [31:0]     sys_o_rst_ctrl1          ;
wire [31:0]     sys_o_alg_rst_ctrl0      ;
wire [31:0]     sys_o_alg_rst_ctrl1      ;
wire [31:0]     sys_o_alg_rst_ctrl2      ;
wire [31:0]     sys_o_alg_rst_ctrl3      ;
wire [31:0]     sys_o_alg_rst_ctrl4      ;
wire [31:0]     sys_o_alg_rst_ctrl5      ;
wire [31:0]     sys_o_alg_rst_ctrl6      ;
wire            sys_o_bus_cipher_en      ;
wire            sys_o_ram_cipher_en      ;
wire            sys_o_nvm_cipher_bypass  ;
wire [31:0]     sys_o_axi_dma_cfg0       ;
wire [31:0]     sys_o_axi_dma_cfg1       ;
wire [31:0]     sys_o_axi_dma_ch0_cfg0   ;
wire [31:0]     sys_o_axi_dma_ch0_cfg1   ;
wire [31:0]     sys_o_axi_dma_ch0_cfg2   ;
wire [31:0]     sys_o_axi_dma_ch1_cfg0   ;
wire [31:0]     sys_o_axi_dma_ch1_cfg1   ;
wire [31:0]     sys_o_axi_dma_ch1_cfg2   ;
wire [31:0]     sys_o_axi_dma_ch2_cfg0   ;
wire [31:0]     sys_o_axi_dma_ch2_cfg1   ;
wire [31:0]     sys_o_axi_dma_ch2_cfg2   ;
wire [31:0]     sys_o_axi_dma_ch3_cfg0   ;
wire [31:0]     sys_o_axi_dma_ch3_cfg1   ;
wire [31:0]     sys_o_axi_dma_ch3_cfg2   ;
wire [31:0]     sys_o_irom_ecc_cfg       ;
wire [31:0]     sys_o_iram_ecc_cfg       ;
wire [31:0]     sys_o_dram_ecc_cfg       ;
wire [31:0]     sys_o_kmu_ram_ecc_cfg    ;
wire [31:0]     sys_o_pke_ram0_ecc_cfg   ;
wire [31:0]     sys_o_pke_ram1_ecc_cfg   ;
wire [31:0]     sys_o_pke_ram2_ecc_cfg   ;
wire [31:0]     sys_o_pke_ram3_ecc_cfg   ;
wire [63:0]     sys_o_mbox_ram_ba        ;
wire [31:0]     sys_o_cpu_cfg            ;
wire [7:0]      sys_o_ahb_dma_rd_wr_to   ;
wire            sys_o_dc_dma_sel         ;
wire [1:0]      sys_o_dma_wr_ctl         ;
wire            sys_o_otp_ahb_if_sel     ;
wire            sys_o_ahb_cfg_rsp_err_en ;
wire            sys_o_ahb_nvm_rsp_err_en ;
wire            sys_o_ahb_otp_rsp_err_en ;
wire            sys_o_ahb_soc_rsp_err_en ;
wire            sys_o_ahb_bus_pr_en      ;
wire [63:0]     sys_o_hsm_status         ;
wire            sys_o_hsm_dbg_en         ;
wire [127:0]    sys_o_soc_dbg_en_128b    ;
wire            sys_o_trng_drbg_alg_sel  ;
wire            sys_o_sys_irq            ;

wire [31:0]                         emu_o_hrdata                ;
wire [1:0]                          emu_o_hresp                 ;
wire                                emu_o_hreadyout             ;
wire [31:0]                         emu_o_err_sensor            ;
wire [63:0]                         emu_o_err_hw                ;
wire [63:0]                         emu_o_err_fw                ;
wire                                emu_o_irq                   ;
wire                                emu_o_resetn_all            ;
wire                                emu_o_resetn_noboot         ;
wire                                emu_o_resetn_cpu            ;

wire [31:0]     boot_o_hrdata              ;
wire            boot_o_hready              ;
wire [1:0]      boot_o_hresp               ;
wire            boot_o_hw_boot_done        ;
wire            boot_o_hw_boot_ok          ;
wire            boot_o_hw_boot_err         ;
wire            boot_o_hw_boot_done_pulse  ;
wire            boot_o_lc_undef            ;
wire            boot_o_lc_destroy          ;
wire            boot_o_lc_debug            ;
wire            boot_o_lc_user             ;
wire            boot_o_lc_manu             ;
wire            boot_o_lc_dev2             ;
wire            boot_o_lc_dev              ;
wire            boot_o_lc_test             ;
wire [31:0]     boot_o_life_cycle          ;
wire            boot_o_rdbi_st_done        ;
wire            boot_o_otp_cs              ;
wire [31:0]     boot_o_otp_addr            ;
wire            boot_o_crc_err             ;
wire            boot_o_trng_rd_en          ;
wire            boot_o_trng_skip_stp       ;
wire            boot_o_trng_sclk_sel       ;
wire            boot_o_trng_to_en          ;
wire            boot_o_bus_cipher_en       ;
wire [31:0]     boot_o_bus_cipher_key      ;
wire [31:0]     boot_o_bus_cipher_nce      ;
wire            boot_o_ram_cipher_en       ;
wire [31:0]     boot_o_ram_cipher_key      ;
wire [31:0]     boot_o_ram_cipher_nce      ;
wire            boot_o_nvm_cipher_bypass   ;
wire [7:0]      boot_o_last_otp            ;
wire            boot_o_kbuf_CSB            ;
wire [7:0]      boot_o_kbuf_A              ;
wire [3:0]      boot_o_kbuf_WEB            ;
wire [31:0]     boot_o_kbuf_DI             ;
wire            boot_o_random_clock_en_set ;
wire [31:0]     boot_o_con_field0          ;
wire [31:0]     boot_o_con_field1          ;
wire [31:0]     boot_o_con_field2          ;
wire [31:0]     boot_o_con_field3          ;
wire [31:0]     boot_o_con_field4          ;
wire [31:0]     boot_o_con_field5          ;
wire            boot_o_patch_en            ;
wire            boot_o_patch_info_vld      ;
wire [11:0]     boot_o_patch_info_addr     ;
wire [31:0]     boot_o_patch_otp_rdata     ;
wire            boot_o_reset_trng_warning  ;
wire            boot_o_reset_trng_error    ;
wire            boot_o_reset_trng_n        ;
wire [63:0]     boot_o_clc_key             ;

wire [31:0]     kmu_o_hrdata             ;
wire [1:0]      kmu_o_hresp              ;
wire            kmu_o_hreadyout          ;
wire            kmu_o_kbuf_CSB           ;
wire [7:0]      kmu_o_kbuf_A             ;
wire [3:0]      kmu_o_kbuf_WEB           ;
wire [31:0]     kmu_o_kbuf_DI            ;
wire            kmu_o_sp_set_hash_key    ;
wire            kmu_o_sp_set_hash_key_v1 ;
wire            kmu_o_sp_set_ske_key     ;
wire            kmu_o_sp_set_ske_key_v1  ;
wire            kmu_o_sp_set_chacha_key  ;
wire [255:0]    kmu_o_sp_key             ;
wire            kmu_o_key_vld            ;
wire [255:0]    kmu_o_key_data           ;
wire [15:0]     kmu_o_key_index          ;

wire [31:0]    mbox_o_se_hrdata     ;
wire [1:0]     mbox_o_se_hresp      ;
wire           mbox_o_se_hreadyout  ;
wire           mbox_o_se_irq        ;
wire [31:0]    mbox_o_soc_hrdata    ;
wire [1:0]     mbox_o_soc_hresp     ;
wire           mbox_o_soc_hreadyout ;
wire [15:0]    mbox_o_soc_irq       ;

wire [31:0]    trng_o_s_hrdata     ;
wire           trng_o_s_hresp      ;
wire           trng_o_s_hreadyout  ;
wire           trng_o_irq          ;
wire           trng_o_trng_drdy    ;
wire [31:0]    trng_o_trng_data    ;
wire           trng_o_trng_rdy     ;
wire           trng_o_alarm        ;
wire [3:0]     trng_o_ro_clk       ;
wire [3:0]     trng_o_ro_out       ;
wire           trng_o_tero_busy    ;
wire [3:0]     trng_o_tero_es_out  ;

wire [31:0]                   hash0_o_s_hrdata             ;
wire                          hash0_o_s_hresp              ;
wire                          hash0_o_s_hreadyout          ;
wire                          hash0_o_dma_done_clr         ;
wire [64-1:0]        hash0_o_dma_saddr            ;
wire [64-1:0]        hash0_o_dma_daddr            ;
wire [31:0]                   hash0_o_dma_rlen             ;
wire [31:0]                   hash0_o_dma_wlen             ;
wire                          hash0_o_dma_rstart           ;
wire                          hash0_o_dma_suspend          ;
wire                          hash0_o_dma_wstart           ;
wire [4:0]    hash0_o_dma_r_fifo_num_count ;
wire                          hash0_o_dma_r_fifo_full      ;
wire [4:0]    hash0_o_dma_w_fifo_num_count ;
wire                          hash0_o_dma_w_fifo_empty     ;
wire [64-1:0]        hash0_o_dma_w_fifo_rd_data   ;
wire                          hash0_o_irq                  ;

wire [31:0]                   ske0_o_s_hrdata           ;
wire                          ske0_o_s_hresp            ;
wire                          ske0_o_s_hreadyout        ;
wire                          ske0_o_dma_rstart         ;
wire                          ske0_o_dma_wstart         ;
wire [64-1:0]        ske0_o_dma_saddr          ;
wire [64-1:0]        ske0_o_dma_daddr          ;
wire [31:0]                   ske0_o_dma_rlen           ;
wire [31:0]                   ske0_o_dma_wlen           ;
wire [64-1:0]        ske0_o_m_w_fifo_rd_data   ;
wire                          ske0_o_m_w_fifo_empty     ;
wire [4:0]    ske0_o_m_w_fifo_num_count ;
wire                          ske0_o_m_r_fifo_full      ;
wire [4:0]    ske0_o_m_r_fifo_num_count ;
wire                          ske0_o_dma_suspend        ;
wire                          ske0_o_dma_done_clr       ;
wire                          ske0_o_irq                ;

wire [31:0]               chacha_o_s_hrdata           = 32'b0;
wire                      chacha_o_s_hresp            = 1'b0;
wire                      chacha_o_s_hreadyout        = 1'b1;
wire                      chacha_o_irq                = 1'b0;

wire [31:0]     pqc_spxp_o_s_hrdata    = 32'h0  ;
wire            pqc_spxp_o_s_hresp     = 1'h0   ;
wire            pqc_spxp_o_s_hreadyout = 1'h1   ;
wire            pqc_spxp_o_irq         = 1'h0   ;

wire           pqc_kd_o_irq         = 1'h0  ;
wire           pqc_kd_o_s_hreadyout = 1'h1  ;
wire           pqc_kd_o_s_hresp     = 1'h0  ;
wire [31:0]    pqc_kd_o_s_hrdata    = 32'h0 ;

wire                          pke_o_irq              ;
wire                          pke_o_s_hreadyout      ;
wire                          pke_o_s_hresp          ;
wire [32-1:0]                 pke_o_s_hrdata         ;
wire                          pke_o_ram0_wren        ;
wire [8:0]                    pke_o_ram0_waddr       ;
wire [PKE_P_RAM_WIDTH-1:0]    pke_o_ram0_wdata       ;
wire                          pke_o_ram0_rden        ;
wire [8:0]                    pke_o_ram0_raddr       ;
wire                          pke_o_ram1_wren        ;
wire [8:0]                    pke_o_ram1_waddr       ;
wire [PKE_P_RAM_WIDTH-1:0]    pke_o_ram1_wdata       ;
wire                          pke_o_ram1_rden        ;
wire [8:0]                    pke_o_ram1_raddr       ;
wire                          pke_o_ram2_wren        ;
wire [8:0]                    pke_o_ram2_waddr       ;
wire [PKE_P_RAM_WIDTH-1:0]    pke_o_ram2_wdata       ;
wire                          pke_o_ram2_rden        ;
wire [8:0]                    pke_o_ram2_raddr       ;
wire                          pke_o_ram3_wren        ;
wire [8:0]                    pke_o_ram3_waddr       ;
wire [PKE_P_RAM_WIDTH-1:0]    pke_o_ram3_wdata       ;
wire                          pke_o_ram3_rden        ;
wire [8:0]                    pke_o_ram3_raddr       ;
wire [3:0]                    pke_o_ecc_dec_sec      ;
wire [3:0]                    pke_o_ecc_dec_ded      ;
wire [36-1:0]                 pke_o_ecc_err_addr     ;

wire [31:0]    apb_o_hrdata                      ;
wire [1:0]     apb_o_hresp                       ;
wire           apb_o_hreadyout                   ;
wire           apb_o_psel                        ;
wire [31:0]    apb_o_paddr                       ;
wire           apb_o_penable                     ;
wire           apb_o_pwrite                      ;
wire [31:0]    apb_o_pwdata                      ;
wire           apb_o_clock_en                    ;
wire           apb_o_rclk_intr                   ;
wire           apb_o_tim0_intr                   ;
wire           apb_o_tim1_intr                   ;
wire           apb_o_wdt_intr                    ;
wire           apb_o_wdt1_intr                   ;
wire           apb_o_utc_irq                     ;
wire           apb_o_mcnt_irq                    ;
wire           apb_o_wdt_rstn                    ;
wire           apb_o_uart_txd_oe                 ;
wire           apb_o_uart_txd                    ;
wire           apb_o_uart_intr                   ;

wire           b2o_o_otp_vld            ;
wire [31:0]    b2o_o_otp_data           ;
wire           b2o_o_hsel               ;
wire [31:0]    b2o_o_haddr              ;
wire [2:0]     b2o_o_hsize              ;
wire [1:0]     b2o_o_htrans             ;
wire           b2o_o_hwrite             ;
wire [31:0]    b2o_o_hwdata             ;
wire           b2o_o_hready             ;
wire           b2o_o_hmastlock          ;
wire [2:0]     b2o_o_hburst             ;
wire [3:0]     b2o_o_hprot              ;

wire [AXID_p_CH_NUM*1-1:0]                           axid_o_r_w_suspend_done ;
wire                                                 axid_o_r_bus_err        ;
wire [5:0]                                           axid_o_r_bus_qid        ;
wire                                                 axid_o_w_bus_err        ;
wire [5:0]                                           axid_o_w_bus_qid        ;
wire [3:0]                                           axid_o_r_w_bus_to       ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_ch_r_busy        ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_ch_r_done        ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_ch_r_done_level  ;
wire [AXID_p_CH_NUM*AXID_p_BUS_DWIDTH-1:0]           axid_o_r_fifo_wdata     ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_r_fifo_wr        ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_ch_w_busy        ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_ch_w_done        ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_ch_w_done_level  ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_o_w_fifo_rd        ;
wire [1:0]                                           axid_o_w_awbar          ;
wire [2:0]                                           axid_o_w_awsnoop        ;
wire [1:0]                                           axid_o_w_awdomain       ;
wire [13:0]                                          axid_o_w_awid           ;
wire [AXID_p_BUS_AWIDTH-1:0]                         axid_o_w_awaddr         ;
wire [7:0]                                           axid_o_w_awlen          ;
wire [2:0]                                           axid_o_w_awsize         ;
wire [1:0]                                           axid_o_w_awburst        ;
wire [1:0]                                           axid_o_w_awlock         ;
wire [3:0]                                           axid_o_w_awcache        ;
wire [2:0]                                           axid_o_w_awprot         ;
wire [3:0]                                           axid_o_w_awqos          ;
wire [3:0]                                           axid_o_w_awregion       ;
wire                                                 axid_o_w_awvalid        ;
wire [7:0]                                           axid_o_w_wid            ;
wire [AXID_p_BUS_DWIDTH-1:0]                         axid_o_w_wdata          ;
wire [AXID_p_BUS_DWIDTH/8-1:0]                       axid_o_w_wstrb          ;
wire                                                 axid_o_w_wlast          ;
wire                                                 axid_o_w_wvalid         ;
wire                                                 axid_o_w_bready         ;
wire [1:0]                                           axid_o_r_arbar          ;
wire [3:0]                                           axid_o_r_arsnoop        ;
wire [1:0]                                           axid_o_r_ardomain       ;
wire [13:0]                                          axid_o_r_arid           ;
wire [AXID_p_BUS_AWIDTH-1:0]                         axid_o_r_araddr         ;
wire [7:0]                                           axid_o_r_arlen          ;
wire [2:0]                                           axid_o_r_arsize         ;
wire [1:0]                                           axid_o_r_arburst        ;
wire [1:0]                                           axid_o_r_arlock         ;
wire [3:0]                                           axid_o_r_arcache        ;
wire [2:0]                                           axid_o_r_arprot         ;
wire [3:0]                                           axid_o_r_arqos          ;
wire [3:0]                                           axid_o_r_arregion       ;
wire                                                 axid_o_r_arvalid        ;
wire                                                 axid_o_r_rready         ;

wire                                  ahbd_o_w_fifo_rd_enable ;
wire                                  ahbd_o_w_fifo_clear     ;
wire [AHBD_p_DATA_WIDTH-1:0]          ahbd_o_r_fifo_data      ;
wire                                  ahbd_o_r_fifo_wr_enable ;
wire                                  ahbd_o_r_fifo_clear     ;
wire                                  ahbd_o_w_ack            ;
wire                                  ahbd_o_w_finish         ;
wire                                  ahbd_o_r_ack            ;
wire                                  ahbd_o_r_finish         ;
wire [63:0]                           ahbd_o_reg_rdata        ;
wire                                  ahbd_o_w_dma_done       ;
wire                                  ahbd_o_w_dma_done_lvl   ;
wire                                  ahbd_o_r_dma_done       ;
wire                                  ahbd_o_r_dma_done_lvl   ;
wire                                  ahbd_o_suspend_done     ;
wire [1:0]                            ahbd_o_to               ;
wire [AHBD_p_ADDR_WIDTH-1:0]          ahbd_o_w_haddr          ;
wire [2:0]                            ahbd_o_w_hburst         ;
wire [1:0]                            ahbd_o_w_hsize          ;
wire [1:0]                            ahbd_o_w_htrans         ;
wire [AHBD_p_DATA_WIDTH-1:0]          ahbd_o_w_hwdata         ;
wire [3:0]                            ahbd_o_w_hprot          ;
wire                                  ahbd_o_w_hwrite         ;
wire [AHBD_p_ADDR_WIDTH-1:0]          ahbd_o_r_haddr          ;
wire [2:0]                            ahbd_o_r_hburst         ;
wire [1:0]                            ahbd_o_r_hsize          ;
wire [1:0]                            ahbd_o_r_htrans         ;
wire [AHBD_p_DATA_WIDTH-1:0]          ahbd_o_r_hwdata         ;
wire [3:0]                            ahbd_o_r_hprot          ;
wire                                  ahbd_o_r_hwrite         ;
wire                                  ahbd_o_int              ;





wire                        amux_o_hreadyouts0 ;
wire                        amux_o_hresps0     ;
wire [AMUX_P_DWIDTH-1:0]    amux_o_hrdatas0    ;
wire                        amux_o_hreadyouts1 ;
wire                        amux_o_hresps1     ;
wire [AMUX_P_DWIDTH-1:0]    amux_o_hrdatas1    ;
wire                        amux_o_hreadyouts2 ;
wire                        amux_o_hresps2     ;
wire [AMUX_P_DWIDTH-1:0]    amux_o_hrdatas2    ;
wire                        amux_o_hselm       ;
wire [AMUX_P_AWIDTH-1:0]    amux_o_haddrm      ;
wire [1:0]                  amux_o_htransm     ;
wire [2:0]                  amux_o_hsizem      ;
wire                        amux_o_hwritem     ;
wire                        amux_o_hreadym     ;
wire [3:0]                  amux_o_hprotm      ;
wire [2:0]                  amux_o_hburstm     ;
wire                        amux_o_hmastlockm  ;
wire [AMUX_P_DWIDTH-1:0]    amux_o_hwdatam     ;
wire [1:0]                  amux_o_hmasterm    ;

wire [63:0]    adnsiz_o_hrdatas    ;
wire           adnsiz_o_hresps     ;
wire           adnsiz_o_hreadyouts ;
wire           adnsiz_o_hselm      ;
wire [31:0]    adnsiz_o_haddrm     ;
wire [1:0]     adnsiz_o_htransm    ;
wire [2:0]     adnsiz_o_hburstm    ;
wire [3:0]     adnsiz_o_hprotm     ;
wire           adnsiz_o_hwritem    ;
wire [2:0]     adnsiz_o_hsizem     ;
wire           adnsiz_o_hmastlockm ;
wire [31:0]    adnsiz_o_hwdatam    ;
wire           adnsiz_o_hreadym    ;

wire           rh2hb_o_s_hsel     = abus_o_hsel9      ;
wire [1:0]     rh2hb_o_s_htrans   = abus_o_htrans     ;
wire [31:0]    rh2hb_o_g_hrdata   = apb_o_hrdata      ;
wire [1:0]     rh2hb_o_g_hresp    = apb_o_hresp       ;

wire keep_logic    = boot_o_last_otp < 8'h3F ; 

wire [4:0]  w_ske_o_m_r_fifo_num_count = ske0_o_m_r_fifo_num_count;
wire [4:0]  w_ske_o_m_w_fifo_num_count = ske0_o_m_w_fifo_num_count;

wire [4:0]  w_hash_o_dma_r_fifo_num_count = hash0_o_dma_r_fifo_num_count;
wire [4:0]  w_hash_o_dma_w_fifo_num_count = hash0_o_dma_w_fifo_num_count;

assign w_hash_i_dma_suspend_done     = 1'b0 ; 
assign w_hash_i_dma_rdone            = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_ch_r_done[0]        : sys_o_dc_dma_sel ? ahbd_o_r_dma_done       : 1'b0 ;
assign w_hash_i_dma_wdone            = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_ch_w_done[0]        : sys_o_dc_dma_sel ? ahbd_o_w_dma_done       : 1'b0 ;
assign w_hash_i_dma_r_fifo_wr        = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_r_fifo_wr[0]        : sys_o_dc_dma_sel ? ahbd_o_r_fifo_wr_enable : 1'b0 ;
assign w_hash_i_dma_r_fifo_wr_data   = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_r_fifo_wdata[64-1:0]  : sys_o_dc_dma_sel ? ahbd_o_r_fifo_data      : {64{1'b0}};
assign w_hash_i_dma_w_fifo_rd        = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_w_fifo_rd[0]        : sys_o_dc_dma_sel ? ahbd_o_w_fifo_rd_enable : 1'b0 ;

assign w_ske_i_dma_suspend_done   = 1'b0 ;
assign w_ske_i_dma_rdone          = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_ch_r_done[1]         : !sys_o_dc_dma_sel ? ahbd_o_r_dma_done       : 1'b0 ;
assign w_ske_i_dma_wdone          = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_ch_w_done[1]         : !sys_o_dc_dma_sel ? ahbd_o_w_dma_done       : 1'b0 ;
assign w_ske_i_m_w_fifo_rd        = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_w_fifo_rd[1]         : !sys_o_dc_dma_sel ? ahbd_o_w_fifo_rd_enable : 1'b0 ;
assign w_ske_i_m_r_fifo_wr_data   = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_r_fifo_wdata[2*AXID_p_BUS_DWIDTH-1 -:AXID_p_BUS_DWIDTH]:
                                                                                                                    !sys_o_dc_dma_sel ? ahbd_o_r_fifo_data      : {AXID_p_BUS_DWIDTH{1'b0}};
assign w_ske_i_m_r_fifo_wr        = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? axid_o_r_fifo_wr[1]        : !sys_o_dc_dma_sel ? ahbd_o_r_fifo_wr_enable : 1'b0 ;

wire [31:0]          hw_otp_haddr     = b2o_o_haddr;

wire                 otp_hsel         = boot_o_hw_boot_ok ? abus_o_hsel10                       : b2o_o_hsel      ;
wire [31:0]          otp_haddr        = boot_o_hw_boot_ok ? {32{abus_o_hsel10}}& abus_o_haddr   : hw_otp_haddr    ;
wire [1:0]           otp_htrans       = boot_o_hw_boot_ok ? {2{abus_o_hsel10}} & abus_o_htrans  : b2o_o_htrans    ;
wire                 otp_hwrite       = boot_o_hw_boot_ok ? abus_o_hsel10      & abus_o_hwrite  : b2o_o_hwrite    ;
wire [2:0]           otp_hburst       = boot_o_hw_boot_ok ? {3{abus_o_hsel10}} & abus_o_hburst  : b2o_o_hburst    ;
wire [2:0]           otp_hsize        = boot_o_hw_boot_ok ? {3{abus_o_hsel10}} & abus_o_hsize   : b2o_o_hsize     ;
wire [3:0]           otp_hprot        = boot_o_hw_boot_ok ? {4{abus_o_hsel10}} & abus_o_hprot   : b2o_o_hprot     ;
wire                 otp_hmastlock    = boot_o_hw_boot_ok ? 1'b0                                : b2o_o_hmastlock ;
wire                 otp_hready       = boot_o_hw_boot_ok ? abus_o_m_hready                     : b2o_o_hready    ;
wire [31:0]          otp_hwdata       = boot_o_hw_boot_ok ? abus_o_hwdata                       : b2o_o_hwdata    ;

wire w_mem_init_done = i_mem_init_done ;

wire [1:0] sys_o_dma_wr_ctl_buf_next = sys_o_dma_wr_ctl;
wire sys_o_dc_dma_sel_buf_next = sys_o_dc_dma_sel;

wire [16:0]                     pke_ecc_err_addr_ram0 = {8'h0,pke_o_ecc_err_addr[8:0]};
wire [16:0]                     pke_ecc_err_addr_ram1 = {8'h0,pke_o_ecc_err_addr[17:9]};
wire [16:0]                     pke_ecc_err_addr_ram2 = {8'h0,pke_o_ecc_err_addr[26:18]};
wire [16:0]                     pke_ecc_err_addr_ram3 = {8'h0,pke_o_ecc_err_addr[35:27]};

wire switch_wr_done_clr = (sys_o_dma_wr_ctl != sys_o_dma_wr_ctl_buf) |  ((sys_o_dc_dma_sel != sys_o_dc_dma_sel_buf) & (sys_o_dma_wr_ctl != 2'b11));

wire [31:0]          cfg_hrdata            = i_cfg_hrdata    ;
wire [1:0]           cfg_hresp             = i_cfg_hresp     ;
wire                 cfg_hreadyout         = i_cfg_hreadyout ;

wire                 w_ske0_o_dma_suspend         = 1'b0 ;

wire                 w_hash0_o_dma_suspend         = 1'b0 ;

assign  w_axid_i_ch_r_start[2*1-1:0]                            = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? {ske0_o_dma_rstart,hash0_o_dma_rstart} : {2{1'b0}};
assign  w_axid_i_ch_raddr  [2*AXID_p_BUS_AWIDTH-1:0]            = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? {ske0_o_dma_saddr,hash0_o_dma_saddr} : {2*AXID_p_BUS_AWIDTH{1'b0}};
assign  w_axid_i_ch_rlen   [2*32-1:0]                           = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? {ske0_o_dma_rlen,hash0_o_dma_rlen} : {2*32{1'b0}};
assign  w_axid_i_ch_r_strans_max[2*16-1:0]                      = {sys_o_axi_dma_ch1_cfg0[31:16],sys_o_axi_dma_ch0_cfg0[31:16]};
assign  w_axid_i_ch_r_qid  [2*6-1:0]                            = {2*6{1'b0}};
assign  w_axid_i_ch_r_arbus[2*32-1:0]                           = {sys_o_axi_dma_ch1_cfg1,sys_o_axi_dma_ch0_cfg1};
assign  w_axid_i_r_fifo_num[2*(AXID_p_R_FIFO_AWIDTH+1)-1:0]     = ((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b11)) ? {w_ske_o_m_r_fifo_num_count,w_hash_o_dma_r_fifo_num_count} : {2*(AXID_p_R_FIFO_AWIDTH+1){1'b0}};
assign  w_axid_i_ch_w_start[2*1-1:0]                            = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? {ske0_o_dma_wstart,hash0_o_dma_wstart} : 2'h0;
assign  w_axid_i_ch_waddr  [2*AXID_p_BUS_AWIDTH-1:0]            = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? {ske0_o_dma_daddr,hash0_o_dma_daddr} : 128'h0;
assign  w_axid_i_ch_wlen   [2*32-1:0]                           = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? {ske0_o_dma_wlen,hash0_o_dma_wlen} : 64'h0;
assign  w_axid_i_ch_w_strans_max[2*16-1:0]                      = {sys_o_axi_dma_ch1_cfg0[15:0],sys_o_axi_dma_ch0_cfg0[15:0]};
assign  w_axid_i_ch_w_qid    [2*6-1:0]                          = {2*6{1'b0}};
assign  w_axid_i_ch_w_awbus  [2*32-1:0]                         = {sys_o_axi_dma_ch1_cfg2,sys_o_axi_dma_ch0_cfg2};
assign  w_axid_i_w_fifo_rdata[2*AXID_p_BUS_DWIDTH-1:0]          = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? {ske0_o_m_w_fifo_rd_data,hash0_o_dma_w_fifo_rd_data} : 128'h0;
assign  w_axid_i_w_fifo_num  [2*(AXID_p_W_FIFO_AWIDTH+1)-1:0]   = ((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b11)) ? {w_ske_o_m_w_fifo_num_count,w_hash_o_dma_w_fifo_num_count} : {2*(AXID_p_R_FIFO_AWIDTH+1){1'b0}};
assign  w_axid_i_r_w_done_clear[2*1-1:0]                        = switch_wr_done_clr ? {2{1'b1}}: !(sys_o_dma_wr_ctl==2'b10) ? {ske0_o_dma_done_clr,hash0_o_dma_done_clr} : 2'h0 ;
assign  w_axid_i_suspend  [2*1-1:0]                             = 2'b0 ;

wire [63:0] w_irq2cpu = { 32'h0                   , 
                          3'h0                    ,
                          emu_o_irq               , 
                          1'h0                    ,
                          mbox_o_se_irq           , 
                          1'h0                    ,
                          sys_o_sys_irq           , 
                          1'h0                    ,
                          apb_o_mcnt_irq          , 
                          1'h0                    ,
                          apb_o_utc_irq           , 
                          1'h0                    ,
                          1'h0                    , 
                          1'h0                    ,
                          apb_o_tim1_intr         , 
                          apb_o_tim0_intr         , 
                          1'h0                    ,
                          apb_o_uart_intr         , 
                          1'h0                    ,
                          apb_o_wdt1_intr         , 
                          apb_o_wdt_intr          , 
                          pqc_kd_o_irq            , 
                          chacha_o_irq            , 
                          pqc_spxp_o_irq          , 
                          pke_o_irq               , 
                          1'h0                    , 
                          ske0_o_irq              , 
                          1'h0                    , 
                          hash0_o_irq             , 
                          1'h0                    ,
                          trng_o_irq              };

wire           abus_hclk          = i_gclk                  ;
wire           abus_hresetn       = i_hwrst_n               ;
wire           abus_i_m_hsel      = i_m_hsel                ;
wire [31:0]    abus_i_m_haddr     = i_m_haddr               ;
wire [1:0]     abus_i_m_htrans    = i_m_htrans              ;
wire           abus_i_m_hwrite    = i_m_hwrite              ;
wire [2:0]     abus_i_m_hsize     = i_m_hsize               ;
wire [2:0]     abus_i_m_hburst    = i_m_hburst              ;
wire [3:0]     abus_i_m_hprot     = i_m_hprot               ;
wire [31:0]    abus_i_m_hwdata    = i_m_hwdata              ;
wire [31:0]    abus_i_hrdata0     = sys_o_hrdata            ;
wire [1:0]     abus_i_hresp0      = sys_o_hresp             ;
wire           abus_i_hreadyout0  = sys_o_hreadyout         ;
wire [31:0]    abus_i_hrdata1     = boot_o_hrdata           ;
wire [1:0]     abus_i_hresp1      = boot_o_hresp            ;
wire           abus_i_hreadyout1  = boot_o_hready           ;
wire [31:0]    abus_i_hrdata2     = kmu_o_hrdata            ;
wire [1:0]     abus_i_hresp2      = kmu_o_hresp             ;
wire           abus_i_hreadyout2  = kmu_o_hreadyout         ;
wire [31:0]    abus_i_hrdata3     = mbox_o_se_hrdata        ;
wire [1:0]     abus_i_hresp3      = mbox_o_se_hresp         ;
wire           abus_i_hreadyout3  = mbox_o_se_hreadyout     ;
wire [31:0]    abus_i_hrdata4     = trng_o_s_hrdata         ;
wire [1:0]     abus_i_hresp4      = {1'b0,trng_o_s_hresp}   ;
wire           abus_i_hreadyout4  = trng_o_s_hreadyout      ;
wire [31:0]    abus_i_hrdata5     = hash0_o_s_hrdata         ;
wire [1:0]     abus_i_hresp5      = {1'b0,hash0_o_s_hresp}   ;
wire           abus_i_hreadyout5  = hash0_o_s_hreadyout      ;
wire [31:0]    abus_i_hrdata6     = ske0_o_s_hrdata          ;
wire [1:0]     abus_i_hresp6      = {1'b0,ske0_o_s_hresp}    ;
wire           abus_i_hreadyout6  = ske0_o_s_hreadyout       ;
wire [31:0]    abus_i_hrdata7     = pke_o_s_hrdata          ;
wire [1:0]     abus_i_hresp7      = {1'b0,pke_o_s_hresp}    ;
wire           abus_i_hreadyout7  = pke_o_s_hreadyout       ;
wire [31:0]    abus_i_hrdata8     = emu_o_hrdata            ;
wire [1:0]     abus_i_hresp8      = emu_o_hresp             ;
wire           abus_i_hreadyout8  = emu_o_hreadyout         ;
wire [31:0]    abus_i_hrdata9     = rh2hb_o_g_hrdata        ;
wire [1:0]     abus_i_hresp9      = rh2hb_o_g_hresp         ;
wire           abus_i_hreadyout9  = apb_o_hreadyout         ;
wire [31:0]    abus_i_hrdata10    = P_OTP_BUS_EN && boot_o_hw_boot_ok ? i_otp_hrdata      : 32'h0     ;
wire [1:0]     abus_i_hresp10     = P_OTP_BUS_EN && boot_o_hw_boot_ok ? i_otp_hresp       : 2'h0      ;
wire           abus_i_hreadyout10 = P_OTP_BUS_EN && boot_o_hw_boot_ok ? i_otp_hreadyout   : 1'h1      ;
wire [31:0]    abus_i_hrdata11    = cfg_hrdata                ;
wire [1:0]     abus_i_hresp11     = cfg_hresp                 ;
wire           abus_i_hreadyout11 = cfg_hreadyout             ;
wire [31:0]    abus_i_hrdata12    = 32'h0                     ;
wire [1:0]     abus_i_hresp12     = 2'h0                      ;
wire           abus_i_hreadyout12 = 1'h1                      ;
wire [31:0]    abus_i_hrdata13    = chacha_o_s_hrdata         ;
wire [1:0]     abus_i_hresp13     = {1'b0,chacha_o_s_hresp}   ;
wire           abus_i_hreadyout13 = chacha_o_s_hreadyout      ;
wire [31:0]    abus_i_hrdata14    = pqc_spxp_o_s_hrdata       ;
wire [1:0]     abus_i_hresp14     = {1'b0,pqc_spxp_o_s_hresp} ;
wire           abus_i_hreadyout14 = pqc_spxp_o_s_hreadyout    ;
wire [31:0]    abus_i_hrdata15    = pqc_kd_o_s_hrdata         ;
wire [1:0]     abus_i_hresp15     = {1'b0,pqc_kd_o_s_hresp}   ;
wire           abus_i_hreadyout15 = pqc_kd_o_s_hreadyout      ;

wire            sys_i_hclk               = i_sys_clk               ;
wire            sys_i_hresetn            = i_sys_rst_n             ;
wire            sys_i_hsel               = abus_o_hsel0            ;
wire [31:0]     sys_i_haddr              = abus_o_hsel0 ? abus_o_haddr  : 32'b0          ;
wire [1:0]      sys_i_htrans             = abus_o_hsel0 ? abus_o_htrans : 2'b0           ;
wire            sys_i_hwrite             = abus_o_hsel0 ? abus_o_hwrite : 1'b0           ;
wire [2:0]      sys_i_hburst             = 3'h0                    ;
wire [2:0]      sys_i_hsize              = abus_o_hsel0 ? abus_o_hsize  : 3'b0           ;
wire [3:0]      sys_i_hprot              = 4'h0                    ;
wire            sys_i_hmastlock          = 1'h0                    ;
wire            sys_i_hready             = 1'h1                    ;
wire [31:0]     sys_i_hwdata             = abus_o_hwdata           ;
wire            sys_i_scan_mode          = i_scan_mode             ;
wire [31:0]     sys_i_sensor             = i_sensor                ;
wire [31:0]     sys_i_soc_status         = i_soc_status            ; 
wire [31:0]     sys_i_boot_mode          = 32'h0                   ;
wire [31:0]     sys_i_boot_sta           = {30'h0, boot_o_hw_boot_err, boot_o_hw_boot_done};

wire            sys_i_lc_undef           = boot_o_lc_undef             ;
wire            sys_i_lc_destroy         = boot_o_lc_destroy           ;
wire            sys_i_lc_debug           = boot_o_lc_debug             ;
wire            sys_i_lc_user            = boot_o_lc_user              ;
wire            sys_i_lc_manu            = boot_o_lc_manu              ;
wire            sys_i_lc_dev2            = boot_o_lc_dev2              ; 
wire            sys_i_lc_dev             = boot_o_lc_dev               ;
wire            sys_i_lc_test            = boot_o_lc_test              ;
wire [31:0]     sys_i_life_cycle         = boot_o_life_cycle           ;
wire [31:0]     sys_i_con_field0         = boot_o_con_field0           ;
wire [31:0]     sys_i_con_field1         = boot_o_con_field1           ;
wire [31:0]     sys_i_con_field2         = boot_o_con_field2           ;
wire [31:0]     sys_i_con_field3         = boot_o_con_field3           ;
wire [31:0]     sys_i_con_field4         = boot_o_con_field4           ;
wire [31:0]     sys_i_con_field5         = boot_o_con_field5           ;
wire [7:0]      sys_i_hw_rd_otp_end      = boot_o_last_otp             ;
wire            sys_i_cpu_hart_halted    = i_cpu_hart_halted           ;
wire            sys_i_cpu_wfi            = i_cpu_wfi                   ;
wire            sys_i_hw_boot_done_pulse = boot_o_hw_boot_done_pulse   ;

wire                                emu_i_hclk                  = i_emu_clk      ;
wire                                emu_i_hresetn               = i_emu_rst_n    ;
wire                                emu_i_hsel                  = abus_o_hsel8   ;
wire [9:0]                          emu_i_haddr                 = abus_o_hsel8 ? abus_o_haddr[9:0] : 10'h0;
wire [1:0]                          emu_i_htrans                = abus_o_hsel8 ? abus_o_htrans     : 2'h0;
wire                                emu_i_hwrite                = abus_o_hsel8 ? abus_o_hwrite     : 1'h0;
wire [2:0]                          emu_i_hsize                 = abus_o_hsel8 ? abus_o_hsize      : 3'h0;
wire [31:0]                         emu_i_hwdata                = abus_o_hwdata  ;
wire                                emu_i_hready                = 1'h1           ;
wire [2:0]                          emu_i_hburst                = 3'h0           ;
wire [3:0]                          emu_i_hprot                 = 4'h1           ;
wire                                emu_i_hmastlock             = 1'h0           ;

wire [31:0]                         emu_i_sensor                = i_sensor       ;
wire [31:0]                         emu_i_soc_err               = i_soc_err      ; 
wire                                emu_i_mem_ecc_1b_irom       = i_mem_ecc_1b_irom;
wire                                emu_i_mem_ecc_1b_iram       = i_mem_ecc_1b_iram;
wire                                emu_i_mem_ecc_1b_dram       = i_mem_ecc_1b_dram;
wire                                emu_i_mem_ecc_1b_kmu        = i_mem_ecc_1b_kmu ;
wire                                emu_i_mem_ecc_1b_pke0       = pke_o_ecc_dec_sec[0];
wire                                emu_i_mem_ecc_1b_pke1       = pke_o_ecc_dec_sec[1];
wire                                emu_i_mem_ecc_1b_pke2       = pke_o_ecc_dec_sec[2];
wire                                emu_i_mem_ecc_1b_pke3       = pke_o_ecc_dec_sec[3];
wire                                emu_i_mem_ecc_mb_irom       = i_mem_ecc_mb_irom;
wire                                emu_i_mem_ecc_mb_iram       = i_mem_ecc_mb_iram;
wire                                emu_i_mem_ecc_mb_dram       = i_mem_ecc_mb_dram;
wire                                emu_i_mem_ecc_mb_kmu        = i_mem_ecc_mb_kmu ;
wire                                emu_i_mem_ecc_mb_pke0       = pke_o_ecc_dec_ded[0];
wire                                emu_i_mem_ecc_mb_pke1       = pke_o_ecc_dec_ded[1];
wire                                emu_i_mem_ecc_mb_pke2       = pke_o_ecc_dec_ded[2];
wire                                emu_i_mem_ecc_mb_pke3       = pke_o_ecc_dec_ded[3];
wire [14-1:0]    emu_i_mem_ecc_addr_irom     = i_mem_ecc_addr_irom;
wire [16-1:0]    emu_i_mem_ecc_addr_iram     = i_mem_ecc_addr_iram;
wire [14-1:0]    emu_i_mem_ecc_addr_dram     = i_mem_ecc_addr_dram;
wire [16:0]                         emu_i_mem_ecc_addr_kmu      = i_mem_ecc_addr_kmu ;
wire [16:0]                         emu_i_mem_ecc_addr_pke0     = pke_ecc_err_addr_ram0;
wire [16:0]                         emu_i_mem_ecc_addr_pke1     = pke_ecc_err_addr_ram1;
wire [16:0]                         emu_i_mem_ecc_addr_pke2     = pke_ecc_err_addr_ram2;
wire [16:0]                         emu_i_mem_ecc_addr_pke3     = pke_ecc_err_addr_ram3;
wire                                emu_i_wdt_timeout           = ~apb_o_wdt_rstn ;

wire                                emu_i_reset_trng_warning    = boot_o_reset_trng_warning ;
wire                                emu_i_reset_trng_error      = boot_o_reset_trng_error   ;
wire                                emu_i_hw_trng_ht_fail       = trng_o_alarm;
wire                                emu_i_soc_err_axi_dma_wr    = i_soc_err_axi_dma_wr;
wire                                emu_i_soc_err_axi_dma_rd    = i_soc_err_axi_dma_rd;
wire                                emu_i_soc_err_ahb_mem       = i_soc_err_ahb_mem;
wire                                emu_i_soc_err_ahb_otp       = i_soc_err_ahb_otp;
wire                                emu_i_soc_err_ahb_nvm       = i_soc_err_ahb_nvm;
wire                                emu_i_soc_err_ahb_cfg       = i_soc_err_ahb_cfg;
wire                                emu_i_otp_key_crc_err       = boot_o_crc_err;
wire                                emu_i_ipre_pchk_err         = i_ipre_pchk_err   ;
wire                                emu_i_dpre_pchk_err         = i_dpre_pchk_err   ;
wire                                emu_i_spre_pchk_err         = i_spre_pchk_err   ;
wire                                emu_i_ahbdpre_pchk_err      = i_ahbdpre_pchk_err;
wire                                emu_i_iromprc_pchk_err      = i_iromprc_pchk_err;
wire                                emu_i_iramprc_pchk_err      = i_iramprc_pchk_err;
wire                                emu_i_dramprc_pchk_err      = i_dramprc_pchk_err;
wire                                emu_i_ahbprc_pchk_err       = i_ahbprc_pchk_err ;
wire                                emu_i_nvmprc_pchk_err       = i_nvmprc_pchk_err ;
wire                                emu_i_socprc_pchk_err       = i_socprc_pchk_err ;
wire [3:0]                          emu_i_dmac_axi_bus_rd_wr_to = axid_o_r_w_bus_to ;
wire [1:0]                          emu_i_dmac_ahb_bus_rd_wr_to = ahbd_o_to         ;

wire                                emu_i_hsm_soft_rbt_n        = sys_o_rst_ctrl0[31];
wire                                emu_i_hsm_soft_rst_n        = sys_o_rst_ctrl0[30];
wire                                emu_i_cpu_soft_rst_n        = sys_o_rst_ctrl0[29];

wire            boot_i_boot_clk            = i_boot_clk          ;
wire            boot_i_sys_rst_n           = i_boot_rst_n        ;
wire            boot_i_scan_mode           = i_scan_mode         ;
wire            boot_i_hclk                = i_boot_clk          ;
wire            boot_i_hresetn             = i_boot_rst_n        ;
wire            boot_i_hsel                = abus_o_hsel1        ;
wire [31:0]     boot_i_haddr               = abus_o_hsel1 ? abus_o_haddr    : 32'h0  ;
wire [1:0]      boot_i_htrans              = abus_o_hsel1 ? abus_o_htrans   : 2'h0   ;
wire            boot_i_hwrite              = abus_o_hsel1 ? abus_o_hwrite   : 1'h0   ;
wire [2:0]      boot_i_hsize               = abus_o_hsel1 ? abus_o_hsize    : 3'h0   ;
wire [2:0]      boot_i_hburst              = 3'h0                ;
wire [3:0]      boot_i_hprot               = 4'h0                ;
wire [31:0]     boot_i_hwdata              = abus_o_hwdata       ;
wire            boot_i_bus_cipher_en       = sys_o_bus_cipher_en ;
wire            boot_i_ram_cipher_en       = sys_o_ram_cipher_en ;
wire            boot_i_nvm_cipher_bypass   = sys_o_nvm_cipher_bypass;

wire            boot_i_otp_rdy             = 1'h1                ;
wire            boot_i_otp_rd_data_vld     = b2o_o_otp_vld       ;
wire [31:0]     boot_i_otp_rd_data         = b2o_o_otp_data      ;
wire            boot_i_trng_alarm          = trng_o_alarm        ;
wire            boot_i_trng_ffvld          = trng_o_trng_drdy    ;
wire [31:0]     boot_i_trng_rd_data        = trng_o_trng_data    ;
wire [11:0]     boot_i_offset_att          = offset_att          ; 
wire [31:0]     boot_i_kbuf_DO             = i_kbuf_DO           ;

wire            boot_i_mem_init_done       = w_mem_init_done     ;

wire            kmu_i_hclk               = i_kmu_clk                   ;
wire            kmu_i_hresetn            = i_kmu_rst_n                 ;
wire            kmu_i_hsel               = abus_o_hsel2                ;
wire [31:0]     kmu_i_haddr              = abus_o_hsel2 ? {12'h0,abus_o_haddr[19:0]}  : 32'h0;
wire [1:0]      kmu_i_htrans             = abus_o_hsel2 ? abus_o_htrans               : 2'h0;
wire            kmu_i_hwrite             = abus_o_hsel2 ? abus_o_hwrite               : 1'h0;
wire [2:0]      kmu_i_hburst             = abus_o_hsel2 ? abus_o_hburst               : 3'h0;
wire [2:0]      kmu_i_hsize              = abus_o_hsel2 ? abus_o_hsize                : 3'h0;
wire [3:0]      kmu_i_hprot              = abus_o_hsel2 ? abus_o_hprot                : 4'h0;
wire            kmu_i_hmastlock          = 1'h0                        ;
wire            kmu_i_hready             = 1'h1                        ;
wire [31:0]     kmu_i_hwdata             = abus_o_hwdata               ;
wire            kmu_i_scan_mode          = i_scan_mode                 ;
wire [11:0]     kmu_i_offset_att         = offset_att                  ; 
wire [7:0]      kmu_i_last_otp           = boot_o_last_otp             ;
wire [31:0]     kmu_i_kbuf_DO            = i_kbuf_DO                   ;

wire            kmu_i_clear_key          = 1'h0                        ;
wire [63:0]     kmu_i_clc_key            = boot_o_clc_key              ;
wire            kmu_i_lc_undef           = boot_o_lc_undef             ;
wire            kmu_i_lc_destroy         = boot_o_lc_destroy           ;
wire            kmu_i_lc_debug           = boot_o_lc_debug             ;
wire            kmu_i_lc_user            = boot_o_lc_user              ;
wire            kmu_i_lc_manu            = boot_o_lc_manu              ;
wire            kmu_i_lc_dev             = boot_o_lc_dev               ;
wire            kmu_i_lc_dev2            = boot_o_lc_dev2              ;
wire            kmu_i_lc_test            = boot_o_lc_test              ;

wire           mbox_i_hclk          = i_mbox_clk                  ; 
wire           mbox_i_hreset_n      = i_mbox_rst_n                ;
wire           mbox_i_hsm_clk_en    = apb_o_clock_en              ;
wire           mbox_i_se_hsel       = abus_o_hsel3                ;
wire [31:0]    mbox_i_se_haddr      = abus_o_hsel3 ? abus_o_haddr    : 32'h0 ;
wire [1:0]     mbox_i_se_htrans     = abus_o_hsel3 ? abus_o_htrans   : 2'h0  ;
wire           mbox_i_se_hwrite     = abus_o_hsel3 ? abus_o_hwrite   : 1'h0  ;
wire [2:0]     mbox_i_se_hsize      = abus_o_hsel3 ? abus_o_hsize    : 3'h0  ;
wire [31:0]    mbox_i_se_hwdata     = abus_o_hwdata               ;
wire           mbox_i_se_hready     = 1'h1                        ;
wire [2:0]     mbox_i_se_hburst     = abus_o_hsel3 ? abus_o_hburst   : 3'h0  ;
wire [3:0]     mbox_i_se_hprot      = 4'h0                        ;
wire           mbox_i_se_hmastlock  = 1'h0                        ;
wire [63:0]    mbox_i_hsm_status    = sys_o_hsm_status            ;
wire           mbox_i_soc_hsel      = i_mbox_soc_hsel             ;
wire [31:0]    mbox_i_soc_haddr     = i_mbox_soc_haddr            ; 
wire [1:0]     mbox_i_soc_htrans    = i_mbox_soc_htrans           ;
wire           mbox_i_soc_hwrite    = i_mbox_soc_hwrite           ; 
wire [2:0]     mbox_i_soc_hsize     = i_mbox_soc_hsize            ; 
wire [31:0]    mbox_i_soc_hwdata    = i_mbox_soc_hwdata           ;  
wire           mbox_i_soc_hready    = i_mbox_soc_hready           ; 
wire [2:0]     mbox_i_soc_hburst    = i_mbox_soc_hburst           ;
wire [3:0]     mbox_i_soc_hprot     = i_mbox_soc_hprot            ; 
wire           mbox_i_soc_hmastlock = i_mbox_soc_hmastlock        ; 

wire           trng_i_clk          = i_trng_clk           ;
wire           trng_i_rst_n        = P_ALG_MASK ? 1'b0 : i_trng_rst_n         ;
wire           trng_i_endian       = 1'b0                 ;
wire           trng_i_s_hsel       = abus_o_hsel4         ;
wire [11:0]    trng_i_s_haddr      = abus_o_hsel4 ? abus_o_haddr[11:0]   : 12'h0;
wire [1:0]     trng_i_s_htrans     = abus_o_hsel4 ? abus_o_htrans        : 2'h0 ;
wire           trng_i_s_hwrite     = abus_o_hsel4 ? abus_o_hwrite        : 1'h0 ;
wire [2:0]     trng_i_s_hburst     = 3'h0                 ;
wire [2:0]     trng_i_s_hsize      = abus_o_hsel4 ? abus_o_hsize         : 3'h0 ;
wire [3:0]     trng_i_s_hprot      = 4'h0                 ;
wire           trng_i_s_hmastlock  = 1'h0                 ;
wire           trng_i_s_hready     = 1'h1                 ;
wire [31:0]    trng_i_s_hwdata     = abus_o_hwdata        ;
wire           trng_i_trng_pop     = boot_o_trng_rd_en    ;
wire           trng_i_scan_mode    = i_scan_mode          ;

wire           trng_i_drbg_mode    = sys_o_trng_drbg_alg_sel ? 1'b1 :1'b0;
wire [63:0]    trng_i_ro_src_en    = 64'hffff_ffff_ffff_ffff;
wire [3:0]     trng_i_ro_clk_en    = 4'hf          ;
wire [1:0]     trng_i_ro_src_fsel  = 2'h3          ;
wire           trng_i_skip_startup = boot_o_trng_skip_stp ;
wire           trng_i_sclk_sel     = boot_o_trng_sclk_sel ;

wire                          hash0_i_clk_core             = i_hash0_clk            ;
wire                          hash0_i_rst_n_core           = P_ALG_MASK ? 1'b0 : i_hash0_rst_n          ;
wire                          hash0_i_clk_ahb              = i_hash0_clk            ;
wire                          hash0_i_rst_n_ahb            = i_hash0_rst_n          ;
wire                          hash0_i_clk_axi              = i_hash0_dma_clk        ;
wire                          hash0_i_rst_n_axi            = i_hash0_dma_rst_n      ;
wire                          hash0_i_s_hsel               = abus_o_hsel5          ;
wire [11:0]                   hash0_i_s_haddr              = abus_o_hsel5 ? abus_o_haddr[11:0]  : 12'h0  ;
wire [1:0]                    hash0_i_s_htrans             = abus_o_hsel5 ? abus_o_htrans       : 2'h0   ;
wire                          hash0_i_s_hwrite             = abus_o_hsel5 ? abus_o_hwrite       : 1'h0   ;
wire [2:0]                    hash0_i_s_hburst             = 3'h0                  ;
wire [2:0]                    hash0_i_s_hsize              = abus_o_hsel5 ? abus_o_hsize        : 3'b0   ;
wire [3:0]                    hash0_i_s_hprot              = 4'h0                  ;
wire                          hash0_i_s_hmastlock          = 1'h0                  ;
wire                          hash0_i_s_hready             = i_m_hready            ;
wire [31:0]                   hash0_i_s_hwdata             = abus_o_hwdata         ;

wire                          hash0_i_dma_suspend_done     = w_hash_i_dma_suspend_done   ;
wire                          hash0_i_dma_rdone            = w_hash_i_dma_rdone          ;
wire                          hash0_i_dma_wdone            = w_hash_i_dma_wdone          ;
wire                          hash0_i_dma_r_fifo_wr        = w_hash_i_dma_r_fifo_wr      ;
wire [64-1:0]        hash0_i_dma_r_fifo_wr_data   = w_hash_i_dma_r_fifo_wr_data ;
wire                          hash0_i_dma_w_fifo_rd        = w_hash_i_dma_w_fifo_rd      ;

wire                          hash0_i_ahb_endian           = 1'h0                  ;
wire [255:0]                  hash0_i_sp_data              = kmu_o_sp_key          ;
wire                          hash0_i_sp_valid             = kmu_o_sp_set_hash_key ;

wire                          ske0_i_s_clk              = i_ske0_clk            ;
wire                          ske0_i_s_rst_n            = P_ALG_MASK ? 1'b0 : i_ske0_rst_n          ;
wire                          ske0_i_dma_clk            = i_ske0_dma_clk        ;
wire                          ske0_i_dma_rst_n          = i_ske0_dma_rst_n      ;
wire                          ske0_i_alg_clk            = i_ske0_clk            ;
wire                          ske0_i_alg_rst_n          = i_ske0_rst_n          ;
wire                          ske0_i_s_ahb_endian       = 1'h0                 ;
wire                          ske0_i_s_hsel             = abus_o_hsel6         ;
wire [11:0]                   ske0_i_s_haddr            = abus_o_hsel6 ? abus_o_haddr[11:0] : 12'h0  ;
wire [1:0]                    ske0_i_s_htrans           = abus_o_hsel6 ? abus_o_htrans      : 2'h0   ;
wire                          ske0_i_s_hwrite           = abus_o_hsel6 ? abus_o_hwrite      : 1'h0   ;
wire [2:0]                    ske0_i_s_hburst           = 3'h0                 ;
wire [2:0]                    ske0_i_s_hsize            = abus_o_hsel6 ? abus_o_hsize       : 3'h0   ;
wire [3:0]                    ske0_i_s_hprot            = 4'h0                 ;
wire                          ske0_i_s_hmastlock        = 1'h0                 ;
wire                          ske0_i_s_hready           = i_m_hready           ;
wire [31:0]                   ske0_i_s_hwdata           = abus_o_hwdata        ;

wire                          ske0_i_scan_mode          = i_scan_mode              ;
wire                          ske0_i_dma_rdone          = w_ske_i_dma_rdone        ;
wire                          ske0_i_dma_wdone          = w_ske_i_dma_wdone        ;
wire                          ske0_i_m_w_fifo_rd        = w_ske_i_m_w_fifo_rd      ;
wire [64-1:0]        ske0_i_m_r_fifo_wr_data   = w_ske_i_m_r_fifo_wr_data ;
wire                          ske0_i_m_r_fifo_wr        = w_ske_i_m_r_fifo_wr      ;
wire                          ske0_i_dma_suspend_done   =  w_ske_i_dma_suspend_done;

wire                          ske0_i_sp_valid           = kmu_o_sp_set_ske_key ;
wire [255:0]                  ske0_i_sp_data            = kmu_o_sp_key         ;

wire                          pke_i_clk_ahb          = i_pke_clk             ;
wire                          pke_i_rst_n_ahb        = P_ALG_MASK ? 1'b0 : i_pke_rst_n           ;
wire                          pke_i_clk_core         = i_pke_clk             ;
wire                          pke_i_rst_n_core       = i_pke_rst_n           ;
wire                          pke_i_ahb_endian       = 1'h0                  ;
wire                          pke_i_s_hsel           = abus_o_hsel7          ;
wire [16:0]                   pke_i_s_haddr          = abus_o_hsel7 ? abus_o_haddr[16:0]  : 17'h0  ;
wire                          pke_i_s_hwrite         = abus_o_hsel7 ? abus_o_hwrite       : 1'b0   ;
wire [2:0]                    pke_i_s_hsize          = abus_o_hsel7 ? abus_o_hsize        : 3'b0   ;
wire [2:0]                    pke_i_s_hburst         = 3'h0                  ;
wire [3:0]                    pke_i_s_hprot          = 4'h0                  ;
wire [1:0]                    pke_i_s_htrans         = abus_o_hsel7 ? abus_o_htrans       : 2'b0   ;
wire                          pke_i_s_hmastlock      = 1'h0                  ;
wire                          pke_i_s_hready         = i_m_hready            ;
wire [32-1:0]                 pke_i_s_hwdata         = abus_o_hwdata         ;

wire [PKE_P_RAM_WIDTH-1:0]    pke_i_ram0_rdata       = i_pke_sram0_DO        ;
wire [PKE_P_RAM_WIDTH-1:0]    pke_i_ram1_rdata       = i_pke_sram1_DO        ;
wire [PKE_P_RAM_WIDTH-1:0]    pke_i_ram2_rdata       = i_pke_sram2_DO        ;
wire [PKE_P_RAM_WIDTH-1:0]    pke_i_ram3_rdata       = i_pke_sram3_DO        ;

wire [32-1:0]                 pke_i_ecc_ck_en        = {sys_o_pke_ram3_ecc_cfg[7:0],sys_o_pke_ram2_ecc_cfg[7:0],sys_o_pke_ram1_ecc_cfg[7:0],sys_o_pke_ram0_ecc_cfg[7:0]};
wire [32-1:0]                 pke_i_ecc_tm_en        = {sys_o_pke_ram3_ecc_cfg[15:8],sys_o_pke_ram2_ecc_cfg[15:8],sys_o_pke_ram1_ecc_cfg[15:8],sys_o_pke_ram0_ecc_cfg[15:8]};
wire [40-1:0]                 pke_i_ecc_chkbits      = {sys_o_pke_ram3_ecc_cfg[25:16],sys_o_pke_ram2_ecc_cfg[25:16],sys_o_pke_ram1_ecc_cfg[25:16],sys_o_pke_ram0_ecc_cfg[25:16]};

wire           apb_i_rdc_clk                     = i_rdc_clk          ;
wire           apb_i_wdt_clk                     = i_wdt_clk          ;

wire           apb_i_uart_clk                    = i_uart_clk         ;
wire           apb_i_tim_clk                     = i_tim_clk          ;

wire           apb_i_crc_clk                     = i_crc_clk          ;

wire           apb_i_rdc_rst_n                   = i_rdc_rst_n        ;
wire           apb_i_wdt_rst_n                   = i_wdt_rst_n        ;

wire           apb_i_uart_rst_n                  = i_uart_rst_n       ;
wire           apb_i_tim_rst_n                   = i_tim_rst_n        ;

wire           apb_i_crc_rst_n                   = i_crc_rst_n        ;

wire           apb_i_hclk                        = i_sclk             ;
wire           apb_i_hresetn                     = i_hwrst_n          ;
wire           apb_i_hsel                        = rh2hb_o_s_hsel     ;
wire [31:0]    apb_i_haddr                       = rh2hb_o_s_hsel ? abus_o_haddr      : 32'h0    ;
wire [1:0]     apb_i_htrans                      = rh2hb_o_s_hsel ? rh2hb_o_s_htrans  : 2'b0     ;
wire           apb_i_hwrite                      = rh2hb_o_s_hsel ? abus_o_hwrite     : 1'b0     ;
wire [2:0]     apb_i_hburst                      = 3'h0               ;
wire [2:0]     apb_i_hsize                       = 3'h2               ;
wire [3:0]     apb_i_hprot                       = 4'h0               ;
wire           apb_i_hmastlock                   = 1'h0               ;
wire           apb_i_hready                      = 1'h1               ;
wire [31:0]    apb_i_hwdata                      = abus_o_hwdata      ;

wire [31:0]    apb_i_prdata                      = 32'b0              ;
wire           apb_i_pslverr                     = 1'b0               ;
wire           apb_i_pready                      = 1'b1               ;
wire           apb_i_otp_random_en               = boot_o_random_clock_en_set;

wire           apb_i_uart_rxd                    = i_uart_rxd         ;

wire           b2o_i_clk                = i_boot_clk                ;
wire           b2o_i_rst_n              = i_boot_rst_n              ;
wire           b2o_i_otp_en             = boot_o_otp_cs             ;
wire [31:0]    b2o_i_otp_addr           = boot_o_otp_addr           ;

wire           b2o_i_hw_boot_done_pulse = boot_o_hw_boot_done_pulse ;
wire [31:0]    b2o_i_hrdata             = i_otp_hrdata              ;
wire [1:0]     b2o_i_hresp              = i_otp_hresp               ;
wire           b2o_i_hreadyout          = i_otp_hreadyout           ;

wire                                                 axid_i_clk              = i_axi_dma_clk             ;
wire                                                 axid_i_rst_n            = i_axi_dma_rst_n           ;
wire [15:0]                                          axid_i_dma_to           = sys_o_axi_dma_cfg1[15:0]  ;
wire [7:0]                                           axid_i_dma_rost_max     = sys_o_axi_dma_cfg0[23:16] ;
wire [7:0]                                           axid_i_dma_rburst_max   = sys_o_axi_dma_cfg0[7:0]   ;
wire [7:0]                                           axid_i_dma_wost_max     = sys_o_axi_dma_cfg0[31:24] ;
wire [7:0]                                           axid_i_dma_wburst_max   = sys_o_axi_dma_cfg0[15:8]  ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_i_r_w_suspend      = w_axid_i_suspend;
wire [AXID_p_CH_NUM*1-1:0]                           axid_i_ch_r_start       = w_axid_i_ch_r_start       ;
wire [AXID_p_CH_NUM*AXID_p_BUS_AWIDTH-1:0]           axid_i_ch_raddr         = w_axid_i_ch_raddr         ;
wire [AXID_p_CH_NUM*32-1:0]                          axid_i_ch_rlen          = w_axid_i_ch_rlen          ;
wire [AXID_p_CH_NUM*16-1:0]                          axid_i_ch_r_strans_max  = w_axid_i_ch_r_strans_max  ;
wire [AXID_p_CH_NUM*6-1:0]                           axid_i_ch_r_qid         = w_axid_i_ch_r_qid         ;
wire [AXID_p_CH_NUM*32-1:0]                          axid_i_ch_r_arbus       = w_axid_i_ch_r_arbus       ;
wire [AXID_p_CH_NUM*(AXID_p_R_FIFO_AWIDTH+1)-1:0]    axid_i_r_fifo_num       = w_axid_i_r_fifo_num       ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_i_ch_w_start       = w_axid_i_ch_w_start       ;
wire [AXID_p_CH_NUM*AXID_p_BUS_AWIDTH-1:0]           axid_i_ch_waddr         = w_axid_i_ch_waddr         ;
wire [AXID_p_CH_NUM*32-1:0]                          axid_i_ch_wlen          = w_axid_i_ch_wlen          ;
wire [AXID_p_CH_NUM*16-1:0]                          axid_i_ch_w_strans_max  = w_axid_i_ch_w_strans_max  ;
wire [AXID_p_CH_NUM*6-1:0]                           axid_i_ch_w_qid         = w_axid_i_ch_w_qid         ;
wire [AXID_p_CH_NUM*32-1:0]                          axid_i_ch_w_awbus       = w_axid_i_ch_w_awbus       ;
wire [AXID_p_CH_NUM*AXID_p_BUS_DWIDTH-1:0]           axid_i_w_fifo_rdata     = w_axid_i_w_fifo_rdata     ;
wire [AXID_p_CH_NUM*(AXID_p_W_FIFO_AWIDTH+1)-1:0]    axid_i_w_fifo_num       = w_axid_i_w_fifo_num       ;
wire [AXID_p_CH_NUM*1-1:0]                           axid_i_r_w_done_clear   = w_axid_i_r_w_done_clear   ;
wire                                                 axid_i_w_awready        = i_dma_m_awready   ;
wire                                                 axid_i_w_wready         = i_dma_m_wready    ;
wire [7:0]                                           axid_i_w_bid            = {4'b0,i_dma_m_bid};
wire [1:0]                                           axid_i_w_bresp          = i_dma_m_bresp     ;
wire                                                 axid_i_w_bvalid         = i_dma_m_bvalid    ;
wire                                                 axid_i_r_arready        = i_dma_m_arready   ;
wire [7:0]                                           axid_i_r_rid            = {4'b0,i_dma_m_rid};
wire [AXID_p_BUS_DWIDTH-1:0]                         axid_i_r_rdata          = i_dma_m_rdata     ;
wire [1:0]                                           axid_i_r_rresp          = i_dma_m_rresp     ;
wire                                                 axid_i_r_rlast          = i_dma_m_rlast     ;
wire                                                 axid_i_r_rvalid         = i_dma_m_rvalid    ;

wire                                  ahbd_i_s_clk            = i_ahb_dma_clk       ;
wire                                  ahbd_i_s_rst_n          = i_ahb_dma_rst_n     ;
wire                                  ahbd_i_m_clk            = i_ahb_dma_clk       ;
wire                                  ahbd_i_m_rst_n          = i_ahb_dma_rst_n     ;
wire                                  ahbd_i_p_clk            = i_ahb_dma_clk       ;
wire                                  ahbd_i_p_rst_n          = i_ahb_dma_rst_n     ;
wire [AHBD_p_DATA_WIDTH-1:0]          ahbd_i_w_fifo_data      = !((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b10)) ? {AHBD_p_DATA_WIDTH{1'b0}} : sys_o_dc_dma_sel ? hash0_o_dma_w_fifo_rd_data : ske0_o_m_w_fifo_rd_data;
wire                                  ahbd_i_w_fifo_empty     = !((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b10)) ? 1'h0  : sys_o_dc_dma_sel ? hash0_o_dma_w_fifo_empty : ske0_o_m_w_fifo_empty;
wire [AHBD_p_W_FIFO_DEPTH_WIDTH:0]    ahbd_i_w_fifo_num_count = !((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b10)) ? {AHBD_p_W_FIFO_DEPTH_WIDTH+1{1'b0}} : sys_o_dc_dma_sel ? hash0_o_dma_w_fifo_num_count :w_ske_o_m_w_fifo_num_count ;
wire                                  ahbd_i_r_fifo_full      = !((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b10)) ? 1'h0  : sys_o_dc_dma_sel ? hash0_o_dma_r_fifo_full : ske0_o_m_r_fifo_full;
wire [AHBD_p_R_FIFO_DEPTH_WIDTH:0]    ahbd_i_r_fifo_num_count = !((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b10)) ? {AHBD_p_W_FIFO_DEPTH_WIDTH+1{1'b0}} : sys_o_dc_dma_sel ? hash0_o_dma_r_fifo_num_count : w_ske_o_m_r_fifo_num_count;
wire                                  ahbd_i_w_req            = 1'b0                    ;
wire                                  ahbd_i_r_req            = 1'b0                    ;
wire                                  ahbd_i_reg_wr           = 1'b0                    ;
wire [4:0]                            ahbd_i_reg_addr         = 5'b0                    ;
wire [31:0]                           ahbd_i_reg_wdata        = 32'b0                   ;
wire                                  ahbd_i_dma_start        = (sys_o_dma_wr_ctl==2'b11) ? 1'h0 : sys_o_dc_dma_sel && (sys_o_dma_wr_ctl==2'b10) ? (hash0_o_dma_rstart | hash0_o_dma_wstart):
                                                                                                   sys_o_dc_dma_sel && (sys_o_dma_wr_ctl==2'b01) ? hash0_o_dma_rstart :
                                                                                                   sys_o_dc_dma_sel && (sys_o_dma_wr_ctl==2'b00) ? hash0_o_dma_wstart :
                                                                                                  !sys_o_dc_dma_sel && (sys_o_dma_wr_ctl==2'b10) ? (ske0_o_dma_rstart | ske0_o_dma_wstart) :
                                                                                                  !sys_o_dc_dma_sel && (sys_o_dma_wr_ctl==2'b01) ? ske0_o_dma_rstart  : ske0_o_dma_wstart;
wire                                  ahbd_i_suspend          = (sys_o_dma_wr_ctl==2'b11) ? 1'h0 : sys_o_dc_dma_sel ? w_hash0_o_dma_suspend : w_ske0_o_dma_suspend;
wire                                  ahbd_i_lli_en           = 1'b0                    ;
wire [7:0]                            ahbd_i_ch_to            = sys_o_ahb_dma_rd_wr_to  ;
wire                                  ahbd_i_rd_endian        = 1'b0                    ;
wire [7:0]                            ahbd_i_rd_max_byte      = !sys_o_dc_dma_sel ? sys_o_axi_dma_ch1_cfg0[23:16]  : sys_o_axi_dma_ch0_cfg0[23:16] ;
wire                                  ahbd_i_wr_endian        = 1'b0                    ;
wire [7:0]                            ahbd_i_wr_max_byte      = !sys_o_dc_dma_sel ? sys_o_axi_dma_ch1_cfg0[7:0]  : sys_o_axi_dma_ch0_cfg0[7:0] ;
wire [AHBD_p_ADDR_WIDTH-1:0]          ahbd_i_saddr            = !((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b10)) ? 64'h0 : sys_o_dc_dma_sel ? hash0_o_dma_saddr & {AHBD_p_ADDR_WIDTH{hash0_o_dma_rstart}} :
                                                                                                                                                        ske0_o_dma_saddr & {AHBD_p_ADDR_WIDTH{ske0_o_dma_rstart}};
wire [AHBD_p_ADDR_WIDTH-1:0]          ahbd_i_daddr            = !((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b10)) ? 64'h0 : sys_o_dc_dma_sel ? hash0_o_dma_daddr & {AHBD_p_ADDR_WIDTH{hash0_o_dma_wstart}} :
                                                                                                                                                        ske0_o_dma_daddr & {AHBD_p_ADDR_WIDTH{ske0_o_dma_wstart}};
wire [AHBD_p_ADDR_WIDTH-1:0]          ahbd_i_rlen             = !((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b10)) ? 64'h0 : sys_o_dc_dma_sel ? {{AHBD_p_ADDR_WIDTH-32{1'b0}},hash0_o_dma_rlen} & {AHBD_p_ADDR_WIDTH{hash0_o_dma_rstart}} :
                                                                                                                                                        {{AHBD_p_ADDR_WIDTH-32{1'b0}},ske0_o_dma_rlen} & {AHBD_p_ADDR_WIDTH{ske0_o_dma_rstart}};
wire [AHBD_p_ADDR_WIDTH-1:0]          ahbd_i_wlen             = !((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b10)) ? 64'h0 : sys_o_dc_dma_sel ? {{AHBD_p_ADDR_WIDTH-32{1'b0}},hash0_o_dma_wlen} & {AHBD_p_ADDR_WIDTH{hash0_o_dma_wstart}} :
                                                                                                                                                        {{AHBD_p_ADDR_WIDTH-32{1'b0}},ske0_o_dma_wlen} & {AHBD_p_ADDR_WIDTH{ske0_o_dma_wstart}};
wire [AHBD_p_ADDR_WIDTH-1:0]          ahbd_i_llp              = {AHBD_p_ADDR_WIDTH{1'b0}};
wire                                  ahbd_i_w_to_int_en      = 1'b0                     ;
wire                                  ahbd_i_r_to_int_en      = 1'b0                     ;
wire                                  ahbd_i_w_dma_int_en     = 1'b0                     ;
wire                                  ahbd_i_r_dma_int_en     = 1'b0                     ;
wire                                  ahbd_i_w_to_clr         = 1'b0                     ;
wire                                  ahbd_i_r_to_clr         = 1'b0                     ;
wire                                  ahbd_i_w_dma_done_clr   = switch_wr_done_clr ? 1'b1 : !((sys_o_dma_wr_ctl==2'b00) || (sys_o_dma_wr_ctl==2'b10)) ? 1'b0: sys_o_dc_dma_sel ? hash0_o_dma_done_clr : ske0_o_dma_done_clr;
wire                                  ahbd_i_r_dma_done_clr   = switch_wr_done_clr ? 1'b1 : !((sys_o_dma_wr_ctl==2'b01) || (sys_o_dma_wr_ctl==2'b10)) ? 1'b0: sys_o_dc_dma_sel ? hash0_o_dma_done_clr : ske0_o_dma_done_clr;
wire [AHBD_p_DATA_WIDTH-1:0]          ahbd_i_w_hrdata         = amux_o_hrdatas0          ;
wire                                  ahbd_i_w_hready         = amux_o_hreadyouts0       ;
wire [AHBD_p_DATA_WIDTH-1:0]          ahbd_i_r_hrdata         = amux_o_hrdatas1          ;
wire                                  ahbd_i_r_hready         = amux_o_hreadyouts1       ;

wire                        amux_i_hclk        = i_gclk                 ;
wire                        amux_i_hreset_n    = i_hwrst_n              ;
wire                        amux_i_hsels0      = ahbd_o_w_htrans[1]     ;
wire [AMUX_P_AWIDTH-1:0]    amux_i_haddrs0     = ahbd_o_w_haddr         ;
wire [1:0]                  amux_i_htranss0    = ahbd_o_w_htrans        ;
wire [2:0]                  amux_i_hsizes0     = {1'b0,ahbd_o_w_hsize}  ;
wire                        amux_i_hwrites0    = ahbd_o_w_hwrite        ;
wire                        amux_i_hreadys0    = amux_o_hreadyouts0     ;
wire [3:0]                  amux_i_hprots0     = ahbd_o_w_hprot         ;
wire [2:0]                  amux_i_hbursts0    = ahbd_o_w_hburst        ;
wire                        amux_i_hmastlocks0 = 1'h0                   ;
wire [AMUX_P_DWIDTH-1:0]    amux_i_hwdatas0    = ahbd_o_w_hwdata        ;
wire                        amux_i_hsels1      = ahbd_o_r_htrans[1]     ;
wire [AMUX_P_AWIDTH-1:0]    amux_i_haddrs1     = ahbd_o_r_haddr         ;
wire [1:0]                  amux_i_htranss1    = ahbd_o_r_htrans        ;
wire [2:0]                  amux_i_hsizes1     = {1'b0,ahbd_o_r_hsize}  ;
wire                        amux_i_hwrites1    = ahbd_o_r_hwrite        ;
wire                        amux_i_hreadys1    = amux_o_hreadyouts1     ;
wire [3:0]                  amux_i_hprots1     = ahbd_o_r_hprot         ;
wire [2:0]                  amux_i_hbursts1    = ahbd_o_r_hburst        ;
wire                        amux_i_hmastlocks1 = 1'h0                   ;
wire [AMUX_P_DWIDTH-1:0]    amux_i_hwdatas1    = ahbd_o_r_hwdata        ;
wire                        amux_i_hsels2      = 1'b0                   ;
wire [AMUX_P_AWIDTH-1:0]    amux_i_haddrs2     = {AMUX_P_AWIDTH{1'b0}}  ;
wire [1:0]                  amux_i_htranss2    = 2'h0                   ;
wire [2:0]                  amux_i_hsizes2     = 3'h0                   ;
wire                        amux_i_hwrites2    = 1'b0                   ;
wire                        amux_i_hreadys2    = 1'h1                   ;
wire [3:0]                  amux_i_hprots2     = 4'h0                   ;
wire [2:0]                  amux_i_hbursts2    = 3'h0                   ;
wire                        amux_i_hmastlocks2 = 1'h0                   ;
wire [AMUX_P_DWIDTH-1:0]    amux_i_hwdatas2    = {AMUX_P_DWIDTH{1'b0}}  ;
wire                        amux_i_hreadyoutm  = adnsiz_o_hreadyouts    ;
wire                        amux_i_hrespm      = adnsiz_o_hresps        ;
wire [AMUX_P_DWIDTH-1:0]    amux_i_hrdatam     = adnsiz_o_hrdatas[AMUX_P_DWIDTH-1:0];

wire           adnsiz_i_hclk       = i_gclk                 ;
wire           adnsiz_i_hreset_n   = i_hwrst_n              ;
wire           adnsiz_i_hsels      = amux_o_hselm           ;
wire [31:0]    adnsiz_i_haddrs     = amux_o_haddrm[31:0]    ;
wire [1:0]     adnsiz_i_htranss    = amux_o_htransm         ;
wire [2:0]     adnsiz_i_hbursts    = amux_o_hburstm         ;
wire [3:0]     adnsiz_i_hprots     = amux_o_hprotm          ;
wire           adnsiz_i_hwrites    = amux_o_hwritem         ;
wire [2:0]     adnsiz_i_hsizes     = amux_o_hsizem          ;
wire           adnsiz_i_hmastlocks = amux_o_hmastlockm      ;
wire           adnsiz_i_hreadys    = adnsiz_o_hreadyouts    ;
wire [63:0]    adnsiz_i_hwdatas    = amux_o_hwdatam         ;
wire [31:0]    adnsiz_i_hrdatam    = i_dma_m_hrdata         ;
wire           adnsiz_i_hrespm     = i_dma_m_hresp[0]       ;
wire           adnsiz_i_hreadyoutm = i_dma_m_hreadyout      ;

assign o_clock_en                    = apb_o_clock_en            ;
assign o_reset_trng_n                = boot_o_reset_trng_n       ;
assign o_keep_logic                  = keep_logic                ;
assign o_m_hrdata                    = abus_o_m_hrdata           ; 
assign o_m_hresp                     = abus_o_m_hresp            ; 
assign o_m_hreadyout                 = abus_o_m_hready           ; 
assign o_otp_hsel                    = otp_hsel                  ;
assign o_otp_haddr                   = otp_haddr                 ;
assign o_otp_htrans                  = otp_htrans                ;
assign o_otp_hwrite                  = otp_hwrite                ;
assign o_otp_hburst                  = otp_hburst                ;
assign o_otp_hsize                   = otp_hsize                 ;
assign o_otp_hprot                   = otp_hprot                 ;
assign o_otp_hmastlock               = otp_hmastlock             ;
assign o_otp_hready                  = otp_hready                ;
assign o_otp_hwdata                  = otp_hwdata                ;

assign o_mbox_soc_hrdata             = mbox_o_soc_hrdata         ;
assign o_mbox_soc_hresp              = mbox_o_soc_hresp          ;
assign o_mbox_soc_hreadyout          = mbox_o_soc_hreadyout      ;

assign o_cfg_hsel                    = abus_o_hsel11             ;
assign o_cfg_haddr                   = abus_o_hsel11 ? abus_o_haddr   : 32'h0          ;
assign o_cfg_htrans                  = abus_o_hsel11 ? abus_o_htrans  : 2'h0           ;
assign o_cfg_hwrite                  = abus_o_hsel11 ? abus_o_hwrite  : 1'h0           ;
assign o_cfg_hburst                  = abus_o_hsel11 ? abus_o_hburst  : 3'h0           ;
assign o_cfg_hsize                   = abus_o_hsel11 ? abus_o_hsize   : 3'h0           ;
assign o_cfg_hprot                   = abus_o_hsel11 ? abus_o_hprot   : 4'h0           ;
assign o_cfg_hmastlock               = 1'h0                      ;
assign o_cfg_hready                  = abus_o_m_hready           ;
assign o_cfg_hwdata                  = abus_o_hwdata             ;

assign o_dma_m_awid                  = axid_o_w_awid[3:0]        ; 
assign o_dma_m_awaddr                = axid_o_w_awaddr           ;
assign o_dma_m_awlen                 = axid_o_w_awlen            ;
assign o_dma_m_awsize                = axid_o_w_awsize           ;
assign o_dma_m_awburst               = axid_o_w_awburst          ;
assign o_dma_m_awlock                = axid_o_w_awlock           ;
assign o_dma_m_awcache               = axid_o_w_awcache          ;
assign o_dma_m_awprot                = axid_o_w_awprot           ;
assign o_dma_m_awqos                 = axid_o_w_awqos            ;
assign o_dma_m_awregion              = axid_o_w_awregion         ;
assign o_dma_m_awvalid               = axid_o_w_awvalid          ;
assign o_dma_m_wid                   = axid_o_w_wid[3:0]         ;
assign o_dma_m_wdata                 = axid_o_w_wdata            ;
assign o_dma_m_wstrb                 = axid_o_w_wstrb            ;
assign o_dma_m_wlast                 = axid_o_w_wlast            ;
assign o_dma_m_wvalid                = axid_o_w_wvalid           ;
assign o_dma_m_bready                = axid_o_w_bready           ;
assign o_dma_m_arid                  = axid_o_r_arid[3:0]        ;
assign o_dma_m_araddr                = axid_o_r_araddr           ;
assign o_dma_m_arlen                 = axid_o_r_arlen            ;
assign o_dma_m_arsize                = axid_o_r_arsize           ;
assign o_dma_m_arburst               = axid_o_r_arburst          ;
assign o_dma_m_arlock                = axid_o_r_arlock           ;
assign o_dma_m_arcache               = axid_o_r_arcache          ;
assign o_dma_m_arprot                = axid_o_r_arprot           ;
assign o_dma_m_arqos                 = axid_o_r_arqos            ;
assign o_dma_m_arregion              = axid_o_r_arregion         ;
assign o_dma_m_arvalid               = axid_o_r_arvalid          ;
assign o_dma_m_rready                = axid_o_r_rready           ;
assign o_dma_m_awbar                 = axid_o_w_awbar            ;
assign o_dma_m_awsnoop               = axid_o_w_awsnoop          ;
assign o_dma_m_awdomain              = axid_o_w_awdomain         ;
assign o_dma_m_arbar                 = axid_o_r_arbar            ;
assign o_dma_m_arsnoop               = axid_o_r_arsnoop          ;
assign o_dma_m_ardomain              = axid_o_r_ardomain         ;

assign o_dma_m_hsel                  = adnsiz_o_hselm            ;
assign o_dma_m_haddr                 = adnsiz_o_haddrm           ;
assign o_dma_m_htrans                = adnsiz_o_htransm          ;
assign o_dma_m_hwrite                = adnsiz_o_hwritem          ;
assign o_dma_m_hburst                = adnsiz_o_hburstm          ;
assign o_dma_m_hsize                 = adnsiz_o_hsizem           ;
assign o_dma_m_hprot                 = adnsiz_o_hprotm           ;
assign o_dma_m_hmastlock             = adnsiz_o_hmastlockm       ;
assign o_dma_m_hwdata                = adnsiz_o_hwdatam          ;

assign o_kbuf_CSB                    = boot_o_hw_boot_ok ? kmu_o_kbuf_CSB : boot_o_kbuf_CSB ;
assign o_kbuf_A                      = boot_o_hw_boot_ok ? kmu_o_kbuf_A   : boot_o_kbuf_A   ;
assign o_kbuf_WEB                    = boot_o_hw_boot_ok ? kmu_o_kbuf_WEB : boot_o_kbuf_WEB ;
assign o_kbuf_DI                     = boot_o_hw_boot_ok ? kmu_o_kbuf_DI  : boot_o_kbuf_DI  ;

assign o_pke_sram0_CK                = i_pke_clk                 ;
assign o_pke_sram0_CSAN              = ~pke_o_ram0_rden          ;
assign o_pke_sram0_A                 =  pke_o_ram0_raddr         ;
assign o_pke_sram0_CSBN              = ~pke_o_ram0_wren          ;
assign o_pke_sram0_B                 =  pke_o_ram0_waddr         ;
assign o_pke_sram1_CK                = i_pke_clk                 ;
assign o_pke_sram1_CSAN              = ~pke_o_ram1_rden          ;
assign o_pke_sram1_A                 =  pke_o_ram1_raddr         ;
assign o_pke_sram1_CSBN              = ~pke_o_ram1_wren          ;
assign o_pke_sram1_B                 =  pke_o_ram1_waddr         ;
assign o_pke_sram2_CK                = i_pke_clk                 ;
assign o_pke_sram2_CSAN              = ~pke_o_ram2_rden          ;
assign o_pke_sram2_A                 =  pke_o_ram2_raddr         ;
assign o_pke_sram2_CSBN              = ~pke_o_ram2_wren          ;
assign o_pke_sram2_B                 =  pke_o_ram2_waddr         ;
assign o_pke_sram3_CK                = i_pke_clk                 ;
assign o_pke_sram3_CSAN              = ~pke_o_ram3_rden          ;
assign o_pke_sram3_A                 =  pke_o_ram3_raddr         ;
assign o_pke_sram3_CSBN              = ~pke_o_ram3_wren          ;
assign o_pke_sram3_B                 =  pke_o_ram3_waddr         ;
assign o_pke_sram0_DI                =  pke_o_ram0_wdata         ;
assign o_pke_sram1_DI                =  pke_o_ram1_wdata         ;
assign o_pke_sram2_DI                =  pke_o_ram2_wdata         ;
assign o_pke_sram3_DI                =  pke_o_ram3_wdata         ;

assign o_clk_ctrl0                   = sys_o_clk_ctrl0           ;
assign o_clk_ctrl1                   = sys_o_clk_ctrl1           ;
assign o_alg_clk_ctrl0               = sys_o_alg_clk_ctrl0       ;
assign o_alg_clk_ctrl1               = sys_o_alg_clk_ctrl1       ;
assign o_alg_clk_ctrl2               = sys_o_alg_clk_ctrl2       ;
assign o_alg_clk_ctrl3               = sys_o_alg_clk_ctrl3       ;
assign o_alg_clk_ctrl4               = sys_o_alg_clk_ctrl4       ;
assign o_alg_clk_ctrl5               = sys_o_alg_clk_ctrl5       ;
assign o_alg_clk_ctrl6               = sys_o_alg_clk_ctrl6       ;
assign o_rst_ctrl0                   = sys_o_rst_ctrl0           ;
assign o_rst_ctrl1                   = sys_o_rst_ctrl1           ;
assign o_alg_rst_ctrl0               = sys_o_alg_rst_ctrl0       ;
assign o_alg_rst_ctrl1               = sys_o_alg_rst_ctrl1       ;
assign o_alg_rst_ctrl2               = sys_o_alg_rst_ctrl2       ;
assign o_alg_rst_ctrl3               = sys_o_alg_rst_ctrl3       ;
assign o_alg_rst_ctrl4               = sys_o_alg_rst_ctrl4       ;
assign o_alg_rst_ctrl5               = sys_o_alg_rst_ctrl5       ;
assign o_alg_rst_ctrl6               = sys_o_alg_rst_ctrl6       ;
assign o_hsm_status                  = sys_o_hsm_status          ;

assign o_ahb_dma_en                  = (sys_o_dma_wr_ctl!=2'b11);
assign o_boot_hw_ok                  = boot_o_hw_boot_ok         ;
assign o_boot_hw_err                 = boot_o_hw_boot_err        ;
assign o_otp_ahb_if_sel              = sys_o_otp_ahb_if_sel      ;
assign o_ahb_nvm_rsp_err_en          = sys_o_ahb_nvm_rsp_err_en  ;
assign o_ahb_cfg_rsp_err_en          = sys_o_ahb_cfg_rsp_err_en  ;
assign o_ahb_otp_rsp_err_en          = sys_o_ahb_otp_rsp_err_en  ;
assign o_ahb_soc_rsp_err_en          = sys_o_ahb_soc_rsp_err_en  ;
assign o_mbox_soc_irq                = mbox_o_soc_irq            ;
assign o_mbox_ram_ba                 = sys_o_mbox_ram_ba         ;
assign o_uart_txd_oe                 = apb_o_uart_txd_oe         ;
assign o_uart_txd                    = apb_o_uart_txd            ;
assign o_irq2cpu                     = w_irq2cpu        ;
assign o_emu_err_sensor              = emu_o_err_sensor ;
assign o_emu_err_hw                  = emu_o_err_hw     ;
assign o_emu_err_fw                  = emu_o_err_fw     ;
assign o_emu_resetn_all              = emu_o_resetn_all   ;
assign o_emu_resetn_noboot           = emu_o_resetn_noboot;
assign o_emu_resetn_cpu              = emu_o_resetn_cpu   ;
assign o_trng_ro_clk                 = trng_o_ro_clk    ;
assign o_trng_ro_out                 = trng_o_ro_out    ;
assign o_bus_cipher_en               = boot_o_bus_cipher_en ;
assign o_bus_cipher_key              = boot_o_bus_cipher_key;
assign o_bus_cipher_nce              = boot_o_bus_cipher_nce;
assign o_ram_cipher_en               = boot_o_ram_cipher_en ;
assign o_ram_cipher_key              = boot_o_ram_cipher_key;
assign o_ram_cipher_nce              = boot_o_ram_cipher_nce;
assign o_nvm_cipher_bypass           = boot_o_nvm_cipher_bypass;
assign o_irom_ecc_cfg                = sys_o_irom_ecc_cfg;
assign o_iram_ecc_cfg                = sys_o_iram_ecc_cfg;
assign o_dram_ecc_cfg                = sys_o_dram_ecc_cfg;
assign o_kmu_ram_ecc_cfg             = sys_o_kmu_ram_ecc_cfg;
assign o_cpu_cfg                     = sys_o_cpu_cfg;

assign o_ahb_bus_pr_en               = sys_o_ahb_bus_pr_en;

assign o_trng_rdy                    = trng_o_trng_rdy;
assign o_trng_alarm                  = trng_o_alarm   ;

assign o_patch_en                    = boot_o_patch_en        ;
assign o_patch_info_vld              = boot_o_patch_info_vld  ;
assign o_patch_info_addr             = boot_o_patch_info_addr ;
assign o_patch_otp_rdata             = boot_o_patch_otp_rdata ;
assign o_soc_dbg_en_128b             = sys_o_soc_dbg_en_128b; 
assign o_hsm_dbg_en                  = sys_o_hsm_dbg_en;

osr_ahb_bus_top u_ahb_bus (
    .hclk              ( abus_hclk          ), 
    .hresetn           ( abus_hresetn       ), 
    .i_m_hsel          ( abus_i_m_hsel      ), 
    .i_m_haddr         ( abus_i_m_haddr     ), 
    .i_m_htrans        ( abus_i_m_htrans    ), 
    .i_m_hwrite        ( abus_i_m_hwrite    ), 
    .i_m_hsize         ( abus_i_m_hsize     ), 
    .i_m_hburst        ( abus_i_m_hburst    ), 
    .i_m_hprot         ( abus_i_m_hprot     ), 
    .i_m_hwdata        ( abus_i_m_hwdata    ), 
    .o_m_hrdata        ( abus_o_m_hrdata    ), 
    .o_m_hready        ( abus_o_m_hready    ), 
    .o_m_hresp         ( abus_o_m_hresp     ), 
    .o_hprot           ( abus_o_hprot       ), 
    .o_haddr           ( abus_o_haddr       ), 
    .o_htrans          ( abus_o_htrans      ), 
    .o_hwrite          ( abus_o_hwrite      ), 
    .o_hsize           ( abus_o_hsize       ), 
    .o_hburst          ( abus_o_hburst      ), 
    .o_hwdata          ( abus_o_hwdata      ), 
    .i_hrdata0         ( abus_i_hrdata0     ), 
    .i_hresp0          ( abus_i_hresp0      ), 
    .i_hreadyout0      ( abus_i_hreadyout0  ), 
    .o_hsel0           ( abus_o_hsel0       ), 
    .i_hrdata1         ( abus_i_hrdata1     ), 
    .i_hresp1          ( abus_i_hresp1      ), 
    .i_hreadyout1      ( abus_i_hreadyout1  ), 
    .o_hsel1           ( abus_o_hsel1       ), 
    .i_hrdata2         ( abus_i_hrdata2     ), 
    .i_hresp2          ( abus_i_hresp2      ), 
    .i_hreadyout2      ( abus_i_hreadyout2  ), 
    .o_hsel2           ( abus_o_hsel2       ), 
    .i_hrdata3         ( abus_i_hrdata3     ), 
    .i_hresp3          ( abus_i_hresp3      ), 
    .i_hreadyout3      ( abus_i_hreadyout3  ), 
    .o_hsel3           ( abus_o_hsel3       ), 
    .i_hrdata4         ( abus_i_hrdata4     ), 
    .i_hresp4          ( abus_i_hresp4      ), 
    .i_hreadyout4      ( abus_i_hreadyout4  ), 
    .o_hsel4           ( abus_o_hsel4       ), 
    .i_hrdata5         ( abus_i_hrdata5     ), 
    .i_hresp5          ( abus_i_hresp5      ), 
    .i_hreadyout5      ( abus_i_hreadyout5  ), 
    .o_hsel5           ( abus_o_hsel5       ), 
    .i_hrdata6         ( abus_i_hrdata6     ), 
    .i_hresp6          ( abus_i_hresp6      ), 
    .i_hreadyout6      ( abus_i_hreadyout6  ), 
    .o_hsel6           ( abus_o_hsel6       ), 
    .i_hrdata7         ( abus_i_hrdata7     ), 
    .i_hresp7          ( abus_i_hresp7      ), 
    .i_hreadyout7      ( abus_i_hreadyout7  ), 
    .o_hsel7           ( abus_o_hsel7       ), 
    .i_hrdata8         ( abus_i_hrdata8     ), 
    .i_hresp8          ( abus_i_hresp8      ), 
    .i_hreadyout8      ( abus_i_hreadyout8  ), 
    .o_hsel8           ( abus_o_hsel8       ), 
    .i_hrdata9         ( abus_i_hrdata9     ), 
    .i_hresp9          ( abus_i_hresp9      ), 
    .i_hreadyout9      ( abus_i_hreadyout9  ), 
    .o_hsel9           ( abus_o_hsel9       ), 
    .i_hrdata10        ( abus_i_hrdata10    ), 
    .i_hresp10         ( abus_i_hresp10     ), 
    .i_hreadyout10     ( abus_i_hreadyout10 ), 
    .o_hsel10          ( abus_o_hsel10      ), 
    .i_hrdata11        ( abus_i_hrdata11    ), 
    .i_hresp11         ( abus_i_hresp11     ), 
    .i_hreadyout11     ( abus_i_hreadyout11 ), 
    .o_hsel11          ( abus_o_hsel11      ), 
    .i_hrdata12        ( abus_i_hrdata12    ), 
    .i_hresp12         ( abus_i_hresp12     ), 
    .i_hreadyout12     ( abus_i_hreadyout12 ), 
    .o_hsel12          ( abus_o_hsel12      ), 
    .i_hrdata13        ( abus_i_hrdata13    ), 
    .i_hresp13         ( abus_i_hresp13     ), 
    .i_hreadyout13     ( abus_i_hreadyout13 ), 
    .o_hsel13          ( abus_o_hsel13      ), 
    .i_hrdata14        ( abus_i_hrdata14    ), 
    .i_hresp14         ( abus_i_hresp14     ), 
    .i_hreadyout14     ( abus_i_hreadyout14 ), 
    .o_hsel14          ( abus_o_hsel14      ), 
    .i_hrdata15        ( abus_i_hrdata15    ), 
    .i_hresp15         ( abus_i_hresp15     ), 
    .i_hreadyout15     ( abus_i_hreadyout15 ), 
    .o_hsel15          ( abus_o_hsel15      )  
    ); 

osr_sys u_sys (
    .i_hclk                  ( sys_i_hclk               ), 
    .i_hresetn               ( sys_i_hresetn            ), 
    .i_hsel                  ( sys_i_hsel               ), 
    .i_haddr                 ( sys_i_haddr              ), 
    .i_htrans                ( sys_i_htrans             ), 
    .i_hwrite                ( sys_i_hwrite             ), 
    .i_hburst                ( sys_i_hburst             ), 
    .i_hsize                 ( sys_i_hsize              ), 
    .i_hprot                 ( sys_i_hprot              ), 
    .i_hmastlock             ( sys_i_hmastlock          ), 
    .i_hready                ( sys_i_hready             ), 
    .i_hwdata                ( sys_i_hwdata             ), 
    .o_hrdata                ( sys_o_hrdata             ), 
    .o_hresp                 ( sys_o_hresp              ), 
    .o_hreadyout             ( sys_o_hreadyout          ), 
    .i_scan_mode             ( sys_i_scan_mode          ), 
    .i_sensor                ( sys_i_sensor             ), 
    .i_soc_status            ( sys_i_soc_status         ), 
    .i_boot_mode             ( sys_i_boot_mode          ), 
    .i_boot_sta              ( sys_i_boot_sta           ), 
    .o_clk_ctrl0             ( sys_o_clk_ctrl0          ), 
    .o_clk_ctrl1             ( sys_o_clk_ctrl1          ), 
    .o_alg_clk_ctrl0         ( sys_o_alg_clk_ctrl0      ), 
    .o_alg_clk_ctrl1         ( sys_o_alg_clk_ctrl1      ), 
    .o_alg_clk_ctrl2         ( sys_o_alg_clk_ctrl2      ), 
    .o_alg_clk_ctrl3         ( sys_o_alg_clk_ctrl3      ), 
    .o_alg_clk_ctrl4         ( sys_o_alg_clk_ctrl4      ), 
    .o_alg_clk_ctrl5         ( sys_o_alg_clk_ctrl5      ), 
    .o_alg_clk_ctrl6         ( sys_o_alg_clk_ctrl6      ), 
    .o_rst_ctrl0             ( sys_o_rst_ctrl0          ), 
    .o_rst_ctrl1             ( sys_o_rst_ctrl1          ), 
    .o_alg_rst_ctrl0         ( sys_o_alg_rst_ctrl0      ), 
    .o_alg_rst_ctrl1         ( sys_o_alg_rst_ctrl1      ), 
    .o_alg_rst_ctrl2         ( sys_o_alg_rst_ctrl2      ), 
    .o_alg_rst_ctrl3         ( sys_o_alg_rst_ctrl3      ), 
    .o_alg_rst_ctrl4         ( sys_o_alg_rst_ctrl4      ), 
    .o_alg_rst_ctrl5         ( sys_o_alg_rst_ctrl5      ), 
    .o_alg_rst_ctrl6         ( sys_o_alg_rst_ctrl6      ), 
    .o_bus_cipher_en         ( sys_o_bus_cipher_en      ), 
    .o_ram_cipher_en         ( sys_o_ram_cipher_en      ), 
    .o_nvm_cipher_bypass     ( sys_o_nvm_cipher_bypass  ), 
    .o_axi_dma_cfg0          ( sys_o_axi_dma_cfg0       ), 
    .o_axi_dma_cfg1          ( sys_o_axi_dma_cfg1       ), 
    .o_axi_dma_ch0_cfg0      ( sys_o_axi_dma_ch0_cfg0   ), 
    .o_axi_dma_ch0_cfg1      ( sys_o_axi_dma_ch0_cfg1   ), 
    .o_axi_dma_ch0_cfg2      ( sys_o_axi_dma_ch0_cfg2   ), 
    .o_axi_dma_ch1_cfg0      ( sys_o_axi_dma_ch1_cfg0   ), 
    .o_axi_dma_ch1_cfg1      ( sys_o_axi_dma_ch1_cfg1   ), 
    .o_axi_dma_ch1_cfg2      ( sys_o_axi_dma_ch1_cfg2   ), 
    .o_axi_dma_ch2_cfg0      ( sys_o_axi_dma_ch2_cfg0   ), 
    .o_axi_dma_ch2_cfg1      ( sys_o_axi_dma_ch2_cfg1   ), 
    .o_axi_dma_ch2_cfg2      ( sys_o_axi_dma_ch2_cfg2   ), 
    .o_axi_dma_ch3_cfg0      ( sys_o_axi_dma_ch3_cfg0   ), 
    .o_axi_dma_ch3_cfg1      ( sys_o_axi_dma_ch3_cfg1   ), 
    .o_axi_dma_ch3_cfg2      ( sys_o_axi_dma_ch3_cfg2   ), 
    .o_irom_ecc_cfg          ( sys_o_irom_ecc_cfg       ), 
    .o_iram_ecc_cfg          ( sys_o_iram_ecc_cfg       ), 
    .o_dram_ecc_cfg          ( sys_o_dram_ecc_cfg       ), 
    .o_kmu_ram_ecc_cfg       ( sys_o_kmu_ram_ecc_cfg    ), 
    .o_pke_ram0_ecc_cfg      ( sys_o_pke_ram0_ecc_cfg   ), 
    .o_pke_ram1_ecc_cfg      ( sys_o_pke_ram1_ecc_cfg   ), 
    .o_pke_ram2_ecc_cfg      ( sys_o_pke_ram2_ecc_cfg   ), 
    .o_pke_ram3_ecc_cfg      ( sys_o_pke_ram3_ecc_cfg   ), 
    .o_mbox_ram_ba           ( sys_o_mbox_ram_ba        ), 
    .o_cpu_cfg               ( sys_o_cpu_cfg            ), 
    .o_ahb_dma_rd_wr_to      ( sys_o_ahb_dma_rd_wr_to   ), 
    .o_dc_dma_sel            ( sys_o_dc_dma_sel         ), 
    .o_dma_wr_ctl            ( sys_o_dma_wr_ctl         ), 
    .o_otp_ahb_if_sel        ( sys_o_otp_ahb_if_sel     ), 
    .o_ahb_cfg_rsp_err_en    ( sys_o_ahb_cfg_rsp_err_en ), 
    .o_ahb_nvm_rsp_err_en    ( sys_o_ahb_nvm_rsp_err_en ), 
    .o_ahb_otp_rsp_err_en    ( sys_o_ahb_otp_rsp_err_en ), 
    .o_ahb_soc_rsp_err_en    ( sys_o_ahb_soc_rsp_err_en ), 
    .o_ahb_bus_pr_en         ( sys_o_ahb_bus_pr_en      ), 
    .o_hsm_status            ( sys_o_hsm_status         ), 
    .i_lc_undef              ( sys_i_lc_undef           ), 
    .i_lc_destroy            ( sys_i_lc_destroy         ), 
    .i_lc_debug              ( sys_i_lc_debug           ), 
    .i_lc_user               ( sys_i_lc_user            ), 
    .i_lc_manu               ( sys_i_lc_manu            ), 
    .i_lc_dev2               ( sys_i_lc_dev2            ), 
    .i_lc_dev                ( sys_i_lc_dev             ), 
    .i_lc_test               ( sys_i_lc_test            ), 
    .i_life_cycle            ( sys_i_life_cycle         ), 
    .i_con_field0            ( sys_i_con_field0         ), 
    .i_con_field1            ( sys_i_con_field1         ), 
    .i_con_field2            ( sys_i_con_field2         ), 
    .i_con_field3            ( sys_i_con_field3         ), 
    .i_con_field4            ( sys_i_con_field4         ), 
    .i_con_field5            ( sys_i_con_field5         ), 
    .i_hw_rd_otp_end         ( sys_i_hw_rd_otp_end      ), 
    .i_cpu_hart_halted       ( sys_i_cpu_hart_halted    ), 
    .i_cpu_wfi               ( sys_i_cpu_wfi            ), 
    .i_hw_boot_done_pulse    ( sys_i_hw_boot_done_pulse ), 
    .o_hsm_dbg_en            ( sys_o_hsm_dbg_en         ), 
    .o_soc_dbg_en_128b       ( sys_o_soc_dbg_en_128b    ), 
    .o_trng_drbg_alg_sel     ( sys_o_trng_drbg_alg_sel  ), 
    .o_sys_irq               ( sys_o_sys_irq            )  
    ); 

osr_emu u_emu (
    .i_hclk                     ( emu_i_hclk                  ), 
    .i_hresetn                  ( emu_i_hresetn               ), 
    .i_hsel                     ( emu_i_hsel                  ), 
    .i_haddr                    ( emu_i_haddr                 ), 
    .i_htrans                   ( emu_i_htrans                ), 
    .i_hwrite                   ( emu_i_hwrite                ), 
    .i_hsize                    ( emu_i_hsize                 ), 
    .i_hwdata                   ( emu_i_hwdata                ), 
    .i_hready                   ( emu_i_hready                ), 
    .i_hburst                   ( emu_i_hburst                ), 
    .i_hprot                    ( emu_i_hprot                 ), 
    .i_hmastlock                ( emu_i_hmastlock             ), 
    .o_hrdata                   ( emu_o_hrdata                ), 
    .o_hresp                    ( emu_o_hresp                 ), 
    .o_hreadyout                ( emu_o_hreadyout             ), 
    .i_sensor                   ( emu_i_sensor                ), 
    .i_soc_err                  ( emu_i_soc_err               ), 
    .i_mem_ecc_1b_irom          ( emu_i_mem_ecc_1b_irom       ), 
    .i_mem_ecc_1b_iram          ( emu_i_mem_ecc_1b_iram       ), 
    .i_mem_ecc_1b_dram          ( emu_i_mem_ecc_1b_dram       ), 
    .i_mem_ecc_1b_kmu           ( emu_i_mem_ecc_1b_kmu        ), 
    .i_mem_ecc_1b_pke0          ( emu_i_mem_ecc_1b_pke0       ), 
    .i_mem_ecc_1b_pke1          ( emu_i_mem_ecc_1b_pke1       ), 
    .i_mem_ecc_1b_pke2          ( emu_i_mem_ecc_1b_pke2       ), 
    .i_mem_ecc_1b_pke3          ( emu_i_mem_ecc_1b_pke3       ), 
    .i_mem_ecc_mb_irom          ( emu_i_mem_ecc_mb_irom       ), 
    .i_mem_ecc_mb_iram          ( emu_i_mem_ecc_mb_iram       ), 
    .i_mem_ecc_mb_dram          ( emu_i_mem_ecc_mb_dram       ), 
    .i_mem_ecc_mb_kmu           ( emu_i_mem_ecc_mb_kmu        ), 
    .i_mem_ecc_mb_pke0          ( emu_i_mem_ecc_mb_pke0       ), 
    .i_mem_ecc_mb_pke1          ( emu_i_mem_ecc_mb_pke1       ), 
    .i_mem_ecc_mb_pke2          ( emu_i_mem_ecc_mb_pke2       ), 
    .i_mem_ecc_mb_pke3          ( emu_i_mem_ecc_mb_pke3       ), 
    .i_mem_ecc_addr_irom        ( emu_i_mem_ecc_addr_irom     ), 
    .i_mem_ecc_addr_iram        ( emu_i_mem_ecc_addr_iram     ), 
    .i_mem_ecc_addr_dram        ( emu_i_mem_ecc_addr_dram     ), 
    .i_mem_ecc_addr_kmu         ( emu_i_mem_ecc_addr_kmu      ), 
    .i_mem_ecc_addr_pke0        ( emu_i_mem_ecc_addr_pke0     ), 
    .i_mem_ecc_addr_pke1        ( emu_i_mem_ecc_addr_pke1     ), 
    .i_mem_ecc_addr_pke2        ( emu_i_mem_ecc_addr_pke2     ), 
    .i_mem_ecc_addr_pke3        ( emu_i_mem_ecc_addr_pke3     ), 
    .i_wdt_timeout              ( emu_i_wdt_timeout           ), 
    .i_reset_trng_warning       ( emu_i_reset_trng_warning    ), 
    .i_reset_trng_error         ( emu_i_reset_trng_error      ), 
    .i_hw_trng_ht_fail          ( emu_i_hw_trng_ht_fail       ), 
    .i_soc_err_axi_dma_wr       ( emu_i_soc_err_axi_dma_wr    ), 
    .i_soc_err_axi_dma_rd       ( emu_i_soc_err_axi_dma_rd    ), 
    .i_soc_err_ahb_mem          ( emu_i_soc_err_ahb_mem       ), 
    .i_soc_err_ahb_otp          ( emu_i_soc_err_ahb_otp       ), 
    .i_soc_err_ahb_nvm          ( emu_i_soc_err_ahb_nvm       ), 
    .i_soc_err_ahb_cfg          ( emu_i_soc_err_ahb_cfg       ), 
    .i_otp_key_crc_err          ( emu_i_otp_key_crc_err       ), 
    .i_ipre_pchk_err            ( emu_i_ipre_pchk_err         ), 
    .i_dpre_pchk_err            ( emu_i_dpre_pchk_err         ), 
    .i_spre_pchk_err            ( emu_i_spre_pchk_err         ), 
    .i_ahbdpre_pchk_err         ( emu_i_ahbdpre_pchk_err      ), 
    .i_iromprc_pchk_err         ( emu_i_iromprc_pchk_err      ), 
    .i_iramprc_pchk_err         ( emu_i_iramprc_pchk_err      ), 
    .i_dramprc_pchk_err         ( emu_i_dramprc_pchk_err      ), 
    .i_ahbprc_pchk_err          ( emu_i_ahbprc_pchk_err       ), 
    .i_nvmprc_pchk_err          ( emu_i_nvmprc_pchk_err       ), 
    .i_socprc_pchk_err          ( emu_i_socprc_pchk_err       ), 
    .i_dmac_axi_bus_rd_wr_to    ( emu_i_dmac_axi_bus_rd_wr_to ), 
    .i_dmac_ahb_bus_rd_wr_to    ( emu_i_dmac_ahb_bus_rd_wr_to ), 
    .i_hsm_soft_rbt_n           ( emu_i_hsm_soft_rbt_n        ), 
    .i_hsm_soft_rst_n           ( emu_i_hsm_soft_rst_n        ), 
    .i_cpu_soft_rst_n           ( emu_i_cpu_soft_rst_n        ), 
    .o_err_sensor               ( emu_o_err_sensor            ), 
    .o_err_hw                   ( emu_o_err_hw                ), 
    .o_err_fw                   ( emu_o_err_fw                ), 
    .o_irq                      ( emu_o_irq                   ), 
    .o_resetn_all               ( emu_o_resetn_all            ), 
    .o_resetn_noboot            ( emu_o_resetn_noboot         ), 
    .o_resetn_cpu               ( emu_o_resetn_cpu            )  
    ); 

osr_boot u_boot (
    .i_boot_clk                ( boot_i_boot_clk            ), 
    .i_sys_rst_n               ( boot_i_sys_rst_n           ), 
    .i_scan_mode               ( boot_i_scan_mode           ), 
    .i_hclk                    ( boot_i_hclk                ), 
    .i_hresetn                 ( boot_i_hresetn             ), 
    .i_hsel                    ( boot_i_hsel                ), 
    .i_haddr                   ( boot_i_haddr               ), 
    .i_htrans                  ( boot_i_htrans              ), 
    .i_hwrite                  ( boot_i_hwrite              ), 
    .i_hsize                   ( boot_i_hsize               ), 
    .i_hburst                  ( boot_i_hburst              ), 
    .i_hprot                   ( boot_i_hprot               ), 
    .i_hwdata                  ( boot_i_hwdata              ), 
    .o_hrdata                  ( boot_o_hrdata              ), 
    .o_hready                  ( boot_o_hready              ), 
    .o_hresp                   ( boot_o_hresp               ), 
    .i_bus_cipher_en           ( boot_i_bus_cipher_en       ), 
    .i_ram_cipher_en           ( boot_i_ram_cipher_en       ), 
    .i_nvm_cipher_bypass       ( boot_i_nvm_cipher_bypass   ), 
    .o_hw_boot_done            ( boot_o_hw_boot_done        ), 
    .o_hw_boot_ok              ( boot_o_hw_boot_ok          ), 
    .o_hw_boot_err             ( boot_o_hw_boot_err         ), 
    .o_hw_boot_done_pulse      ( boot_o_hw_boot_done_pulse  ), 
    .o_lc_undef                ( boot_o_lc_undef            ), 
    .o_lc_destroy              ( boot_o_lc_destroy          ), 
    .o_lc_debug                ( boot_o_lc_debug            ), 
    .o_lc_user                 ( boot_o_lc_user             ), 
    .o_lc_manu                 ( boot_o_lc_manu             ), 
    .o_lc_dev2                 ( boot_o_lc_dev2             ), 
    .o_lc_dev                  ( boot_o_lc_dev              ), 
    .o_lc_test                 ( boot_o_lc_test             ), 
    .o_life_cycle              ( boot_o_life_cycle          ), 
    .o_rdbi_st_done            ( boot_o_rdbi_st_done        ), 
    .o_otp_cs                  ( boot_o_otp_cs              ), 
    .o_otp_addr                ( boot_o_otp_addr            ), 
    .i_otp_rdy                 ( boot_i_otp_rdy             ), 
    .i_otp_rd_data_vld         ( boot_i_otp_rd_data_vld     ), 
    .i_otp_rd_data             ( boot_i_otp_rd_data         ), 
    .o_crc_err                 ( boot_o_crc_err             ), 
    .i_trng_alarm              ( boot_i_trng_alarm          ), 
    .i_trng_ffvld              ( boot_i_trng_ffvld          ), 
    .o_trng_rd_en              ( boot_o_trng_rd_en          ), 
    .i_trng_rd_data            ( boot_i_trng_rd_data        ), 
    .o_trng_skip_stp           ( boot_o_trng_skip_stp       ), 
    .o_trng_sclk_sel           ( boot_o_trng_sclk_sel       ), 
    .o_trng_to_en              ( boot_o_trng_to_en          ), 
    .o_bus_cipher_en           ( boot_o_bus_cipher_en       ), 
    .o_bus_cipher_key          ( boot_o_bus_cipher_key      ), 
    .o_bus_cipher_nce          ( boot_o_bus_cipher_nce      ), 
    .o_ram_cipher_en           ( boot_o_ram_cipher_en       ), 
    .o_ram_cipher_key          ( boot_o_ram_cipher_key      ), 
    .o_ram_cipher_nce          ( boot_o_ram_cipher_nce      ), 
    .o_nvm_cipher_bypass       ( boot_o_nvm_cipher_bypass   ), 
    .i_offset_att              ( boot_i_offset_att          ), 
    .o_last_otp                ( boot_o_last_otp            ), 
    .o_kbuf_CSB                ( boot_o_kbuf_CSB            ), 
    .o_kbuf_A                  ( boot_o_kbuf_A              ), 
    .o_kbuf_WEB                ( boot_o_kbuf_WEB            ), 
    .o_kbuf_DI                 ( boot_o_kbuf_DI             ), 
    .i_kbuf_DO                 ( boot_i_kbuf_DO             ), 
    .o_random_clock_en_set     ( boot_o_random_clock_en_set ), 
    .o_con_field0              ( boot_o_con_field0          ), 
    .o_con_field1              ( boot_o_con_field1          ), 
    .o_con_field2              ( boot_o_con_field2          ), 
    .o_con_field3              ( boot_o_con_field3          ), 
    .o_con_field4              ( boot_o_con_field4          ), 
    .o_con_field5              ( boot_o_con_field5          ), 
    .o_patch_en                ( boot_o_patch_en            ), 
    .o_patch_info_vld          ( boot_o_patch_info_vld      ), 
    .o_patch_info_addr         ( boot_o_patch_info_addr     ), 
    .o_patch_otp_rdata         ( boot_o_patch_otp_rdata     ), 
    .o_reset_trng_warning      ( boot_o_reset_trng_warning  ), 
    .o_reset_trng_error        ( boot_o_reset_trng_error    ), 
    .o_reset_trng_n            ( boot_o_reset_trng_n        ), 
    .o_clc_key                 ( boot_o_clc_key             ), 
    .i_mem_init_done           ( boot_i_mem_init_done       )  
    ); 

osr_kmu u_kmu (
    .i_hclk                  ( kmu_i_hclk               ), 
    .i_hresetn               ( kmu_i_hresetn            ), 
    .i_hsel                  ( kmu_i_hsel               ), 
    .i_haddr                 ( kmu_i_haddr              ), 
    .i_htrans                ( kmu_i_htrans             ), 
    .i_hwrite                ( kmu_i_hwrite             ), 
    .i_hburst                ( kmu_i_hburst             ), 
    .i_hsize                 ( kmu_i_hsize              ), 
    .i_hprot                 ( kmu_i_hprot              ), 
    .i_hmastlock             ( kmu_i_hmastlock          ), 
    .i_hready                ( kmu_i_hready             ), 
    .i_hwdata                ( kmu_i_hwdata             ), 
    .o_hrdata                ( kmu_o_hrdata             ), 
    .o_hresp                 ( kmu_o_hresp              ), 
    .o_hreadyout             ( kmu_o_hreadyout          ), 
    .i_scan_mode             ( kmu_i_scan_mode          ), 
    .i_offset_att            ( kmu_i_offset_att         ), 
    .i_last_otp              ( kmu_i_last_otp           ), 
    .o_kbuf_CSB              ( kmu_o_kbuf_CSB           ), 
    .o_kbuf_A                ( kmu_o_kbuf_A             ), 
    .o_kbuf_WEB              ( kmu_o_kbuf_WEB           ), 
    .o_kbuf_DI               ( kmu_o_kbuf_DI            ), 
    .i_kbuf_DO               ( kmu_i_kbuf_DO            ), 
    .o_sp_set_hash_key       ( kmu_o_sp_set_hash_key    ), 
    .o_sp_set_hash_key_v1    ( kmu_o_sp_set_hash_key_v1 ), 
    .o_sp_set_ske_key        ( kmu_o_sp_set_ske_key     ), 
    .o_sp_set_ske_key_v1     ( kmu_o_sp_set_ske_key_v1  ), 
    .o_sp_set_chacha_key     ( kmu_o_sp_set_chacha_key  ), 
    .o_sp_key                ( kmu_o_sp_key             ), 
    .i_clear_key             ( kmu_i_clear_key          ), 
    .o_key_vld               ( kmu_o_key_vld            ), 
    .o_key_data              ( kmu_o_key_data           ), 
    .o_key_index             ( kmu_o_key_index          ), 
    .i_clc_key               ( kmu_i_clc_key            ), 
    .i_lc_undef              ( kmu_i_lc_undef           ), 
    .i_lc_destroy            ( kmu_i_lc_destroy         ), 
    .i_lc_debug              ( kmu_i_lc_debug           ), 
    .i_lc_user               ( kmu_i_lc_user            ), 
    .i_lc_manu               ( kmu_i_lc_manu            ), 
    .i_lc_dev                ( kmu_i_lc_dev             ), 
    .i_lc_dev2               ( kmu_i_lc_dev2            ), 
    .i_lc_test               ( kmu_i_lc_test            )  
    ); 

osr_mailbox_wrapper #(
    .P_S2H_NOTE_N     ( MBOX_P_S2H_NOTE_N ), 
    .P_H2S_NOTE_N     ( MBOX_P_H2S_NOTE_N ), 
    .P_MBOX_NUM       ( MBOX_P_MBOX_NUM   ), 
    .P_HAVE_FUSA      ( MBOX_P_HAVE_FUSA  )  
    ) u_mbox_wrapper (
    .i_hclk              ( mbox_i_hclk          ), 
    .i_hreset_n          ( mbox_i_hreset_n      ), 
    .i_hsm_clk_en        ( mbox_i_hsm_clk_en    ), 
    .i_se_hsel           ( mbox_i_se_hsel       ), 
    .i_se_haddr          ( mbox_i_se_haddr      ), 
    .i_se_htrans         ( mbox_i_se_htrans     ), 
    .i_se_hwrite         ( mbox_i_se_hwrite     ), 
    .i_se_hsize          ( mbox_i_se_hsize      ), 
    .i_se_hwdata         ( mbox_i_se_hwdata     ), 
    .i_se_hready         ( mbox_i_se_hready     ), 
    .i_se_hburst         ( mbox_i_se_hburst     ), 
    .i_se_hprot          ( mbox_i_se_hprot      ), 
    .i_se_hmastlock      ( mbox_i_se_hmastlock  ), 
    .o_se_hrdata         ( mbox_o_se_hrdata     ), 
    .o_se_hresp          ( mbox_o_se_hresp      ), 
    .o_se_hreadyout      ( mbox_o_se_hreadyout  ), 
    .i_hsm_status        ( mbox_i_hsm_status    ), 
    .o_se_irq            ( mbox_o_se_irq        ), 
    .i_soc_hsel          ( mbox_i_soc_hsel      ), 
    .i_soc_haddr         ( mbox_i_soc_haddr     ), 
    .i_soc_htrans        ( mbox_i_soc_htrans    ), 
    .i_soc_hwrite        ( mbox_i_soc_hwrite    ), 
    .i_soc_hsize         ( mbox_i_soc_hsize     ), 
    .i_soc_hwdata        ( mbox_i_soc_hwdata    ), 
    .i_soc_hready        ( mbox_i_soc_hready    ), 
    .i_soc_hburst        ( mbox_i_soc_hburst    ), 
    .i_soc_hprot         ( mbox_i_soc_hprot     ), 
    .i_soc_hmastlock     ( mbox_i_soc_hmastlock ), 
    .o_soc_hrdata        ( mbox_o_soc_hrdata    ), 
    .o_soc_hresp         ( mbox_o_soc_hresp     ), 
    .o_soc_hreadyout     ( mbox_o_soc_hreadyout ), 
    .o_soc_irq           ( mbox_o_soc_irq       )  
    ); 

osr_trng_wrapper u_trng (
    .i_clk              ( trng_i_clk          ), 
    .i_rst_n            ( trng_i_rst_n        ), 
    .i_endian           ( trng_i_endian       ), 
    .i_s_hsel           ( trng_i_s_hsel       ), 
    .i_s_haddr          ( trng_i_s_haddr      ), 
    .i_s_htrans         ( trng_i_s_htrans     ), 
    .i_s_hwrite         ( trng_i_s_hwrite     ), 
    .i_s_hburst         ( trng_i_s_hburst     ), 
    .i_s_hsize          ( trng_i_s_hsize      ), 
    .i_s_hprot          ( trng_i_s_hprot      ), 
    .i_s_hmastlock      ( trng_i_s_hmastlock  ), 
    .i_s_hready         ( trng_i_s_hready     ), 
    .i_s_hwdata         ( trng_i_s_hwdata     ), 
    .o_s_hrdata         ( trng_o_s_hrdata     ), 
    .o_s_hresp          ( trng_o_s_hresp      ), 
    .o_s_hreadyout      ( trng_o_s_hreadyout  ), 
    .o_irq              ( trng_o_irq          ), 
    .i_trng_pop         ( trng_i_trng_pop     ), 
    .o_trng_drdy        ( trng_o_trng_drdy    ), 
    .o_trng_data        ( trng_o_trng_data    ), 
    .i_scan_mode        ( trng_i_scan_mode    ), 
    .i_drbg_mode        ( trng_i_drbg_mode    ), 
    .i_ro_src_en        ( trng_i_ro_src_en    ), 
    .i_ro_clk_en        ( trng_i_ro_clk_en    ), 
    .i_ro_src_fsel      ( trng_i_ro_src_fsel  ), 
    .i_skip_startup     ( trng_i_skip_startup ), 
    .i_sclk_sel         ( trng_i_sclk_sel     ), 
    .o_trng_rdy         ( trng_o_trng_rdy     ), 
    .o_alarm            ( trng_o_alarm        ), 
    .o_ro_clk           ( trng_o_ro_clk       ), 
    .o_ro_out           ( trng_o_ro_out       ), 
    .o_tero_busy        ( trng_o_tero_busy    ), 
    .o_tero_es_out      ( trng_o_tero_es_out  )  
    ); 

osr_hash_wrapper u_hash0 (
    .i_clk_core                  ( hash0_i_clk_core             ), 
    .i_rst_n_core                ( hash0_i_rst_n_core           ), 
    .i_clk_ahb                   ( hash0_i_clk_ahb              ), 
    .i_rst_n_ahb                 ( hash0_i_rst_n_ahb            ), 
    .i_clk_axi                   ( hash0_i_clk_axi              ), 
    .i_rst_n_axi                 ( hash0_i_rst_n_axi            ), 
    .i_s_hsel                    ( hash0_i_s_hsel               ), 
    .i_s_haddr                   ( hash0_i_s_haddr              ), 
    .i_s_htrans                  ( hash0_i_s_htrans             ), 
    .i_s_hwrite                  ( hash0_i_s_hwrite             ), 
    .i_s_hburst                  ( hash0_i_s_hburst             ), 
    .i_s_hsize                   ( hash0_i_s_hsize              ), 
    .i_s_hprot                   ( hash0_i_s_hprot              ), 
    .i_s_hmastlock               ( hash0_i_s_hmastlock          ), 
    .i_s_hready                  ( hash0_i_s_hready             ), 
    .i_s_hwdata                  ( hash0_i_s_hwdata             ), 
    .o_s_hrdata                  ( hash0_o_s_hrdata             ), 
    .o_s_hresp                   ( hash0_o_s_hresp              ), 
    .o_s_hreadyout               ( hash0_o_s_hreadyout          ), 
    .o_dma_done_clr              ( hash0_o_dma_done_clr         ), 
    .o_dma_saddr                 ( hash0_o_dma_saddr            ), 
    .o_dma_daddr                 ( hash0_o_dma_daddr            ), 
    .o_dma_rlen                  ( hash0_o_dma_rlen             ), 
    .o_dma_wlen                  ( hash0_o_dma_wlen             ), 
    .o_dma_rstart                ( hash0_o_dma_rstart           ), 
    .o_dma_suspend               ( hash0_o_dma_suspend          ), 
    .o_dma_wstart                ( hash0_o_dma_wstart           ), 
    .i_dma_suspend_done          ( hash0_i_dma_suspend_done     ), 
    .i_dma_rdone                 ( hash0_i_dma_rdone            ), 
    .i_dma_wdone                 ( hash0_i_dma_wdone            ), 
    .o_dma_r_fifo_num_count      ( hash0_o_dma_r_fifo_num_count ), 
    .o_dma_r_fifo_full           ( hash0_o_dma_r_fifo_full      ), 
    .i_dma_r_fifo_wr             ( hash0_i_dma_r_fifo_wr        ), 
    .i_dma_r_fifo_wr_data        ( hash0_i_dma_r_fifo_wr_data   ), 
    .o_dma_w_fifo_num_count      ( hash0_o_dma_w_fifo_num_count ), 
    .o_dma_w_fifo_empty          ( hash0_o_dma_w_fifo_empty     ), 
    .i_dma_w_fifo_rd             ( hash0_i_dma_w_fifo_rd        ), 
    .o_dma_w_fifo_rd_data        ( hash0_o_dma_w_fifo_rd_data   ), 
    .o_irq                       ( hash0_o_irq                  ), 
    .i_ahb_endian                ( hash0_i_ahb_endian           ), 
    .i_sp_data                   ( hash0_i_sp_data              ), 
    .i_sp_valid                  ( hash0_i_sp_valid             )  
    ); 

osr_ske_wrapper u_ske0 (
    .i_s_clk                  ( ske0_i_s_clk              ), 
    .i_s_rst_n                ( ske0_i_s_rst_n            ), 
    .i_dma_clk                ( ske0_i_dma_clk            ), 
    .i_dma_rst_n              ( ske0_i_dma_rst_n          ), 
    .i_alg_clk                ( ske0_i_alg_clk            ), 
    .i_alg_rst_n              ( ske0_i_alg_rst_n          ), 
    .i_s_ahb_endian           ( ske0_i_s_ahb_endian       ), 
    .i_s_hsel                 ( ske0_i_s_hsel             ), 
    .i_s_haddr                ( ske0_i_s_haddr            ), 
    .i_s_htrans               ( ske0_i_s_htrans           ), 
    .i_s_hwrite               ( ske0_i_s_hwrite           ), 
    .i_s_hburst               ( ske0_i_s_hburst           ), 
    .i_s_hsize                ( ske0_i_s_hsize            ), 
    .i_s_hprot                ( ske0_i_s_hprot            ), 
    .i_s_hmastlock            ( ske0_i_s_hmastlock        ), 
    .i_s_hready               ( ske0_i_s_hready           ), 
    .i_s_hwdata               ( ske0_i_s_hwdata           ), 
    .o_s_hrdata               ( ske0_o_s_hrdata           ), 
    .o_s_hresp                ( ske0_o_s_hresp            ), 
    .o_s_hreadyout            ( ske0_o_s_hreadyout        ), 
    .i_scan_mode              ( ske0_i_scan_mode          ), 
    .o_dma_rstart             ( ske0_o_dma_rstart         ), 
    .o_dma_wstart             ( ske0_o_dma_wstart         ), 
    .i_dma_rdone              ( ske0_i_dma_rdone          ), 
    .i_dma_wdone              ( ske0_i_dma_wdone          ), 
    .o_dma_saddr              ( ske0_o_dma_saddr          ), 
    .o_dma_daddr              ( ske0_o_dma_daddr          ), 
    .o_dma_rlen               ( ske0_o_dma_rlen           ), 
    .o_dma_wlen               ( ske0_o_dma_wlen           ), 
    .o_m_w_fifo_rd_data       ( ske0_o_m_w_fifo_rd_data   ), 
    .o_m_w_fifo_empty         ( ske0_o_m_w_fifo_empty     ), 
    .o_m_w_fifo_num_count     ( ske0_o_m_w_fifo_num_count ), 
    .i_m_w_fifo_rd            ( ske0_i_m_w_fifo_rd        ), 
    .i_m_r_fifo_wr_data       ( ske0_i_m_r_fifo_wr_data   ), 
    .i_m_r_fifo_wr            ( ske0_i_m_r_fifo_wr        ), 
    .o_m_r_fifo_full          ( ske0_o_m_r_fifo_full      ), 
    .o_m_r_fifo_num_count     ( ske0_o_m_r_fifo_num_count ), 
    .o_dma_suspend            ( ske0_o_dma_suspend        ), 
    .i_dma_suspend_done       ( ske0_i_dma_suspend_done   ), 
    .o_dma_done_clr           ( ske0_o_dma_done_clr       ), 
    .o_irq                    ( ske0_o_irq                ), 
    .i_sp_valid               ( ske0_i_sp_valid           ), 
    .i_sp_data                ( ske0_i_sp_data            )  
    ); 

osr_pke_wrapper #(
    .P_RAM_WIDTH       ( PKE_P_RAM_WIDTH   ), 
    .P_NONE_           ( PKE_P_NONE_       )  
    ) u_pke (
    .i_clk_ahb             ( pke_i_clk_ahb          ), 
    .i_rst_n_ahb           ( pke_i_rst_n_ahb        ), 
    .i_clk_core            ( pke_i_clk_core         ), 
    .i_rst_n_core          ( pke_i_rst_n_core       ), 
    .i_ahb_endian          ( pke_i_ahb_endian       ), 
    .o_irq                 ( pke_o_irq              ), 
    .i_s_hsel              ( pke_i_s_hsel           ), 
    .i_s_haddr             ( pke_i_s_haddr          ), 
    .i_s_hwrite            ( pke_i_s_hwrite         ), 
    .i_s_hsize             ( pke_i_s_hsize          ), 
    .i_s_hburst            ( pke_i_s_hburst         ), 
    .i_s_hprot             ( pke_i_s_hprot          ), 
    .i_s_htrans            ( pke_i_s_htrans         ), 
    .i_s_hmastlock         ( pke_i_s_hmastlock      ), 
    .i_s_hready            ( pke_i_s_hready         ), 
    .i_s_hwdata            ( pke_i_s_hwdata         ), 
    .o_s_hreadyout         ( pke_o_s_hreadyout      ), 
    .o_s_hresp             ( pke_o_s_hresp          ), 
    .o_s_hrdata            ( pke_o_s_hrdata         ), 
    .o_ram0_wren           ( pke_o_ram0_wren        ), 
    .o_ram0_waddr          ( pke_o_ram0_waddr       ), 
    .o_ram0_wdata          ( pke_o_ram0_wdata       ), 
    .o_ram0_rden           ( pke_o_ram0_rden        ), 
    .o_ram0_raddr          ( pke_o_ram0_raddr       ), 
    .i_ram0_rdata          ( pke_i_ram0_rdata       ), 
    .o_ram1_wren           ( pke_o_ram1_wren        ), 
    .o_ram1_waddr          ( pke_o_ram1_waddr       ), 
    .o_ram1_wdata          ( pke_o_ram1_wdata       ), 
    .o_ram1_rden           ( pke_o_ram1_rden        ), 
    .o_ram1_raddr          ( pke_o_ram1_raddr       ), 
    .i_ram1_rdata          ( pke_i_ram1_rdata       ), 
    .o_ram2_wren           ( pke_o_ram2_wren        ), 
    .o_ram2_waddr          ( pke_o_ram2_waddr       ), 
    .o_ram2_wdata          ( pke_o_ram2_wdata       ), 
    .o_ram2_rden           ( pke_o_ram2_rden        ), 
    .o_ram2_raddr          ( pke_o_ram2_raddr       ), 
    .i_ram2_rdata          ( pke_i_ram2_rdata       ), 
    .o_ram3_wren           ( pke_o_ram3_wren        ), 
    .o_ram3_waddr          ( pke_o_ram3_waddr       ), 
    .o_ram3_wdata          ( pke_o_ram3_wdata       ), 
    .o_ram3_rden           ( pke_o_ram3_rden        ), 
    .o_ram3_raddr          ( pke_o_ram3_raddr       ), 
    .i_ram3_rdata          ( pke_i_ram3_rdata       ), 
    .i_ecc_ck_en           ( pke_i_ecc_ck_en        ), 
    .i_ecc_tm_en           ( pke_i_ecc_tm_en        ), 
    .i_ecc_chkbits         ( pke_i_ecc_chkbits      ), 
    .o_ecc_dec_sec         ( pke_o_ecc_dec_sec      ), 
    .o_ecc_dec_ded         ( pke_o_ecc_dec_ded      ), 
    .o_ecc_err_addr        ( pke_o_ecc_err_addr     )  
    ); 

osr_apb_bus_wrapper u_apb_bus_wrapper (
    .i_rdc_clk                        ( apb_i_rdc_clk                     ), 
    .i_wdt_clk                        ( apb_i_wdt_clk                     ), 
    .i_uart_clk                       ( apb_i_uart_clk                    ), 
    .i_tim_clk                        ( apb_i_tim_clk                     ), 
    .i_crc_clk                        ( apb_i_crc_clk                     ), 
    .i_rdc_rst_n                      ( apb_i_rdc_rst_n                   ), 
    .i_wdt_rst_n                      ( apb_i_wdt_rst_n                   ), 
    .i_uart_rst_n                     ( apb_i_uart_rst_n                  ), 
    .i_tim_rst_n                      ( apb_i_tim_rst_n                   ), 
    .i_crc_rst_n                      ( apb_i_crc_rst_n                   ), 
    .i_hclk                           ( apb_i_hclk                        ), 
    .i_hresetn                        ( apb_i_hresetn                     ), 
    .i_hsel                           ( apb_i_hsel                        ), 
    .i_haddr                          ( apb_i_haddr                       ), 
    .i_htrans                         ( apb_i_htrans                      ), 
    .i_hwrite                         ( apb_i_hwrite                      ), 
    .i_hburst                         ( apb_i_hburst                      ), 
    .i_hsize                          ( apb_i_hsize                       ), 
    .i_hprot                          ( apb_i_hprot                       ), 
    .i_hmastlock                      ( apb_i_hmastlock                   ), 
    .i_hready                         ( apb_i_hready                      ), 
    .i_hwdata                         ( apb_i_hwdata                      ), 
    .o_hrdata                         ( apb_o_hrdata                      ), 
    .o_hresp                          ( apb_o_hresp                       ), 
    .o_hreadyout                      ( apb_o_hreadyout                   ), 
    .o_psel                           ( apb_o_psel                        ), 
    .o_paddr                          ( apb_o_paddr                       ), 
    .o_penable                        ( apb_o_penable                     ), 
    .o_pwrite                         ( apb_o_pwrite                      ), 
    .o_pwdata                         ( apb_o_pwdata                      ), 
    .i_prdata                         ( apb_i_prdata                      ), 
    .i_pslverr                        ( apb_i_pslverr                     ), 
    .i_pready                         ( apb_i_pready                      ), 
    .i_otp_random_en                  ( apb_i_otp_random_en               ), 
    .o_clock_en                       ( apb_o_clock_en                    ), 
    .o_rclk_intr                      ( apb_o_rclk_intr                   ), 
    .o_tim0_intr                      ( apb_o_tim0_intr                   ), 
    .o_tim1_intr                      ( apb_o_tim1_intr                   ), 
    .o_wdt_intr                       ( apb_o_wdt_intr                    ), 
    .o_wdt1_intr                      ( apb_o_wdt1_intr                   ), 
    .o_utc_irq                        ( apb_o_utc_irq                     ), 
    .o_mcnt_irq                       ( apb_o_mcnt_irq                    ), 
    .o_wdt_rstn                       ( apb_o_wdt_rstn                    ), 
    .i_uart_rxd                       ( apb_i_uart_rxd                    ), 
    .o_uart_txd_oe                    ( apb_o_uart_txd_oe                 ), 
    .o_uart_txd                       ( apb_o_uart_txd                    ), 
    .o_uart_intr                      ( apb_o_uart_intr                   )  
    ); 

osr_boot2otp_wrapper u_boot2otp (
    .i_clk                   ( b2o_i_clk                ), 
    .i_rst_n                 ( b2o_i_rst_n              ), 
    .i_otp_en                ( b2o_i_otp_en             ), 
    .i_otp_addr              ( b2o_i_otp_addr           ), 
    .o_otp_vld               ( b2o_o_otp_vld            ), 
    .o_otp_data              ( b2o_o_otp_data           ), 
    .i_hw_boot_done_pulse    ( b2o_i_hw_boot_done_pulse ), 
    .o_hsel                  ( b2o_o_hsel               ), 
    .o_haddr                 ( b2o_o_haddr              ), 
    .o_hsize                 ( b2o_o_hsize              ), 
    .o_htrans                ( b2o_o_htrans             ), 
    .o_hwrite                ( b2o_o_hwrite             ), 
    .o_hwdata                ( b2o_o_hwdata             ), 
    .o_hready                ( b2o_o_hready             ), 
    .o_hmastlock             ( b2o_o_hmastlock          ), 
    .o_hburst                ( b2o_o_hburst             ), 
    .o_hprot                 ( b2o_o_hprot              ), 
    .i_hrdata                ( b2o_i_hrdata             ), 
    .i_hresp                 ( b2o_i_hresp              ), 
    .i_hreadyout             ( b2o_i_hreadyout          )  
    ); 

osr_axi_edmac_top #(
    .p_BUS_DWIDTH         ( AXID_p_BUS_DWIDTH     ), 
    .p_BUS_AWIDTH         ( AXID_p_BUS_AWIDTH     ), 
    .p_R_BMEM_AWIDTH      ( AXID_p_R_BMEM_AWIDTH  ), 
    .p_W_BMEM_AWIDTH      ( AXID_p_W_BMEM_AWIDTH  ), 
    .p_R_SMEM_AWIDTH      ( AXID_p_R_SMEM_AWIDTH  ), 
    .p_W_SMEM_AWIDTH      ( AXID_p_W_SMEM_AWIDTH  ), 
    .p_R_FIFO_AWIDTH      ( AXID_p_R_FIFO_AWIDTH  ), 
    .p_W_FIFO_AWIDTH      ( AXID_p_W_FIFO_AWIDTH  ), 
    .p_CH_NUM             ( AXID_p_CH_NUM         ), 
    .p_W_ALIGN            ( AXID_p_W_ALIGN        ), 
    .p_R_ALIGN            ( AXID_p_R_ALIGN        ), 
    .p_W_ALIGN_STEP       ( AXID_p_W_ALIGN_STEP   ), 
    .p_R_ALIGN_STEP       ( AXID_p_R_ALIGN_STEP   ), 
    .p_TIMING_ENHANCE     ( AXID_p_TIMING_ENHANCE )  
    ) u_axi_edmac (
    .i_clk                  ( axid_i_clk              ), 
    .i_rst_n                ( axid_i_rst_n            ), 
    .i_dma_to               ( axid_i_dma_to           ), 
    .i_dma_rost_max         ( axid_i_dma_rost_max     ), 
    .i_dma_rburst_max       ( axid_i_dma_rburst_max   ), 
    .i_dma_wost_max         ( axid_i_dma_wost_max     ), 
    .i_dma_wburst_max       ( axid_i_dma_wburst_max   ), 
    .i_r_w_suspend          ( axid_i_r_w_suspend      ), 
    .o_r_w_suspend_done     ( axid_o_r_w_suspend_done ), 
    .o_r_bus_err            ( axid_o_r_bus_err        ), 
    .o_r_bus_qid            ( axid_o_r_bus_qid        ), 
    .o_w_bus_err            ( axid_o_w_bus_err        ), 
    .o_w_bus_qid            ( axid_o_w_bus_qid        ), 
    .o_r_w_bus_to           ( axid_o_r_w_bus_to       ), 
    .o_ch_r_busy            ( axid_o_ch_r_busy        ), 
    .o_ch_r_done            ( axid_o_ch_r_done        ), 
    .o_ch_r_done_level      ( axid_o_ch_r_done_level  ), 
    .i_ch_r_start           ( axid_i_ch_r_start       ), 
    .i_ch_raddr             ( axid_i_ch_raddr         ), 
    .i_ch_rlen              ( axid_i_ch_rlen          ), 
    .i_ch_r_strans_max      ( axid_i_ch_r_strans_max  ), 
    .i_ch_r_qid             ( axid_i_ch_r_qid         ), 
    .i_ch_r_arbus           ( axid_i_ch_r_arbus       ), 
    .o_r_fifo_wdata         ( axid_o_r_fifo_wdata     ), 
    .o_r_fifo_wr            ( axid_o_r_fifo_wr        ), 
    .i_r_fifo_num           ( axid_i_r_fifo_num       ), 
    .o_ch_w_busy            ( axid_o_ch_w_busy        ), 
    .o_ch_w_done            ( axid_o_ch_w_done        ), 
    .o_ch_w_done_level      ( axid_o_ch_w_done_level  ), 
    .i_ch_w_start           ( axid_i_ch_w_start       ), 
    .i_ch_waddr             ( axid_i_ch_waddr         ), 
    .i_ch_wlen              ( axid_i_ch_wlen          ), 
    .i_ch_w_strans_max      ( axid_i_ch_w_strans_max  ), 
    .i_ch_w_qid             ( axid_i_ch_w_qid         ), 
    .i_ch_w_awbus           ( axid_i_ch_w_awbus       ), 
    .i_w_fifo_rdata         ( axid_i_w_fifo_rdata     ), 
    .o_w_fifo_rd            ( axid_o_w_fifo_rd        ), 
    .i_w_fifo_num           ( axid_i_w_fifo_num       ), 
    .i_r_w_done_clear       ( axid_i_r_w_done_clear   ), 
    .o_w_awbar              ( axid_o_w_awbar          ), 
    .o_w_awsnoop            ( axid_o_w_awsnoop        ), 
    .o_w_awdomain           ( axid_o_w_awdomain       ), 
    .o_w_awid               ( axid_o_w_awid           ), 
    .o_w_awaddr             ( axid_o_w_awaddr         ), 
    .o_w_awlen              ( axid_o_w_awlen          ), 
    .o_w_awsize             ( axid_o_w_awsize         ), 
    .o_w_awburst            ( axid_o_w_awburst        ), 
    .o_w_awlock             ( axid_o_w_awlock         ), 
    .o_w_awcache            ( axid_o_w_awcache        ), 
    .o_w_awprot             ( axid_o_w_awprot         ), 
    .o_w_awqos              ( axid_o_w_awqos          ), 
    .o_w_awregion           ( axid_o_w_awregion       ), 
    .o_w_awvalid            ( axid_o_w_awvalid        ), 
    .i_w_awready            ( axid_i_w_awready        ), 
    .o_w_wid                ( axid_o_w_wid            ), 
    .o_w_wdata              ( axid_o_w_wdata          ), 
    .o_w_wstrb              ( axid_o_w_wstrb          ), 
    .o_w_wlast              ( axid_o_w_wlast          ), 
    .o_w_wvalid             ( axid_o_w_wvalid         ), 
    .i_w_wready             ( axid_i_w_wready         ), 
    .i_w_bid                ( axid_i_w_bid            ), 
    .i_w_bresp              ( axid_i_w_bresp          ), 
    .i_w_bvalid             ( axid_i_w_bvalid         ), 
    .o_w_bready             ( axid_o_w_bready         ), 
    .o_r_arbar              ( axid_o_r_arbar          ), 
    .o_r_arsnoop            ( axid_o_r_arsnoop        ), 
    .o_r_ardomain           ( axid_o_r_ardomain       ), 
    .o_r_arid               ( axid_o_r_arid           ), 
    .o_r_araddr             ( axid_o_r_araddr         ), 
    .o_r_arlen              ( axid_o_r_arlen          ), 
    .o_r_arsize             ( axid_o_r_arsize         ), 
    .o_r_arburst            ( axid_o_r_arburst        ), 
    .o_r_arlock             ( axid_o_r_arlock         ), 
    .o_r_arcache            ( axid_o_r_arcache        ), 
    .o_r_arprot             ( axid_o_r_arprot         ), 
    .o_r_arqos              ( axid_o_r_arqos          ), 
    .o_r_arregion           ( axid_o_r_arregion       ), 
    .o_r_arvalid            ( axid_o_r_arvalid        ), 
    .i_r_arready            ( axid_i_r_arready        ), 
    .i_r_rid                ( axid_i_r_rid            ), 
    .i_r_rdata              ( axid_i_r_rdata          ), 
    .i_r_rresp              ( axid_i_r_rresp          ), 
    .i_r_rlast              ( axid_i_r_rlast          ), 
    .i_r_rvalid             ( axid_i_r_rvalid         ), 
    .o_r_rready             ( axid_o_r_rready         )  
    ); 

osr_ahb_dmac_top #(
    .p_DATA_WIDTH              ( AHBD_p_DATA_WIDTH          ), 
    .p_ADDR_WIDTH              ( AHBD_p_ADDR_WIDTH          ), 
    .p_CH_FIFO_EXIST           ( AHBD_p_CH_FIFO_EXIST       ), 
    .p_LINK_LIST_EN            ( AHBD_p_LINK_LIST_EN        ), 
    .p_AHB_SIMPLE              ( AHBD_p_AHB_SIMPLE          ), 
    .p_CDC_MS_EN               ( AHBD_p_CDC_MS_EN           ), 
    .p_CDC_MP_EN               ( AHBD_p_CDC_MP_EN           ), 
    .p_R_FIFO_DEPTH_WIDTH      ( AHBD_p_R_FIFO_DEPTH_WIDTH  ), 
    .p_W_FIFO_DEPTH_WIDTH      ( AHBD_p_W_FIFO_DEPTH_WIDTH  ), 
    .p_CH_FIFO_DEPTH_WIDTH     ( AHBD_p_CH_FIFO_DEPTH_WIDTH )  
    ) u_ahb_dmac (
    .i_s_clk                ( ahbd_i_s_clk            ), 
    .i_s_rst_n              ( ahbd_i_s_rst_n          ), 
    .i_m_clk                ( ahbd_i_m_clk            ), 
    .i_m_rst_n              ( ahbd_i_m_rst_n          ), 
    .i_p_clk                ( ahbd_i_p_clk            ), 
    .i_p_rst_n              ( ahbd_i_p_rst_n          ), 
    .i_w_fifo_data          ( ahbd_i_w_fifo_data      ), 
    .o_w_fifo_rd_enable     ( ahbd_o_w_fifo_rd_enable ), 
    .i_w_fifo_empty         ( ahbd_i_w_fifo_empty     ), 
    .i_w_fifo_num_count     ( ahbd_i_w_fifo_num_count ), 
    .o_w_fifo_clear         ( ahbd_o_w_fifo_clear     ), 
    .o_r_fifo_data          ( ahbd_o_r_fifo_data      ), 
    .o_r_fifo_wr_enable     ( ahbd_o_r_fifo_wr_enable ), 
    .i_r_fifo_full          ( ahbd_i_r_fifo_full      ), 
    .i_r_fifo_num_count     ( ahbd_i_r_fifo_num_count ), 
    .o_r_fifo_clear         ( ahbd_o_r_fifo_clear     ), 
    .i_w_req                ( ahbd_i_w_req            ), 
    .o_w_ack                ( ahbd_o_w_ack            ), 
    .o_w_finish             ( ahbd_o_w_finish         ), 
    .i_r_req                ( ahbd_i_r_req            ), 
    .o_r_ack                ( ahbd_o_r_ack            ), 
    .o_r_finish             ( ahbd_o_r_finish         ), 
    .i_reg_wr               ( ahbd_i_reg_wr           ), 
    .i_reg_addr             ( ahbd_i_reg_addr         ), 
    .i_reg_wdata            ( ahbd_i_reg_wdata        ), 
    .o_reg_rdata            ( ahbd_o_reg_rdata        ), 
    .i_dma_start            ( ahbd_i_dma_start        ), 
    .i_suspend              ( ahbd_i_suspend          ), 
    .i_lli_en               ( ahbd_i_lli_en           ), 
    .i_ch_to                ( ahbd_i_ch_to            ), 
    .i_rd_endian            ( ahbd_i_rd_endian        ), 
    .i_rd_max_byte          ( ahbd_i_rd_max_byte      ), 
    .i_wr_endian            ( ahbd_i_wr_endian        ), 
    .i_wr_max_byte          ( ahbd_i_wr_max_byte      ), 
    .i_saddr                ( ahbd_i_saddr            ), 
    .i_daddr                ( ahbd_i_daddr            ), 
    .i_rlen                 ( ahbd_i_rlen             ), 
    .i_wlen                 ( ahbd_i_wlen             ), 
    .i_llp                  ( ahbd_i_llp              ), 
    .i_w_to_int_en          ( ahbd_i_w_to_int_en      ), 
    .i_r_to_int_en          ( ahbd_i_r_to_int_en      ), 
    .i_w_dma_int_en         ( ahbd_i_w_dma_int_en     ), 
    .i_r_dma_int_en         ( ahbd_i_r_dma_int_en     ), 
    .o_w_dma_done           ( ahbd_o_w_dma_done       ), 
    .o_w_dma_done_lvl       ( ahbd_o_w_dma_done_lvl   ), 
    .o_r_dma_done           ( ahbd_o_r_dma_done       ), 
    .o_r_dma_done_lvl       ( ahbd_o_r_dma_done_lvl   ), 
    .o_suspend_done         ( ahbd_o_suspend_done     ), 
    .i_w_to_clr             ( ahbd_i_w_to_clr         ), 
    .i_r_to_clr             ( ahbd_i_r_to_clr         ), 
    .o_to                   ( ahbd_o_to               ), 
    .i_w_dma_done_clr       ( ahbd_i_w_dma_done_clr   ), 
    .i_r_dma_done_clr       ( ahbd_i_r_dma_done_clr   ), 
    .o_w_haddr              ( ahbd_o_w_haddr          ), 
    .o_w_hburst             ( ahbd_o_w_hburst         ), 
    .o_w_hsize              ( ahbd_o_w_hsize          ), 
    .o_w_htrans             ( ahbd_o_w_htrans         ), 
    .o_w_hwdata             ( ahbd_o_w_hwdata         ), 
    .o_w_hprot              ( ahbd_o_w_hprot          ), 
    .o_w_hwrite             ( ahbd_o_w_hwrite         ), 
    .i_w_hrdata             ( ahbd_i_w_hrdata         ), 
    .i_w_hready             ( ahbd_i_w_hready         ), 
    .o_r_haddr              ( ahbd_o_r_haddr          ), 
    .o_r_hburst             ( ahbd_o_r_hburst         ), 
    .o_r_hsize              ( ahbd_o_r_hsize          ), 
    .o_r_htrans             ( ahbd_o_r_htrans         ), 
    .o_r_hwdata             ( ahbd_o_r_hwdata         ), 
    .o_r_hprot              ( ahbd_o_r_hprot          ), 
    .o_r_hwrite             ( ahbd_o_r_hwrite         ), 
    .i_r_hrdata             ( ahbd_i_r_hrdata         ), 
    .i_r_hready             ( ahbd_i_r_hready         ), 
    .o_int                  ( ahbd_o_int              )  
    ); 

osr_ahb_master_mux #(
    .P_PORT0_ENABLE     ( AMUX_P_PORT0_ENABLE ), 
    .P_PORT1_ENABLE     ( AMUX_P_PORT1_ENABLE ), 
    .P_PORT2_ENABLE     ( AMUX_P_PORT2_ENABLE ), 
    .P_AWIDTH           ( AMUX_P_AWIDTH       ), 
    .P_DWIDTH           ( AMUX_P_DWIDTH       )  
    ) u_ahb_master_mux (
    .i_hclk            ( amux_i_hclk        ), 
    .i_hreset_n        ( amux_i_hreset_n    ), 
    .i_hsels0          ( amux_i_hsels0      ), 
    .i_haddrs0         ( amux_i_haddrs0     ), 
    .i_htranss0        ( amux_i_htranss0    ), 
    .i_hsizes0         ( amux_i_hsizes0     ), 
    .i_hwrites0        ( amux_i_hwrites0    ), 
    .i_hreadys0        ( amux_i_hreadys0    ), 
    .i_hprots0         ( amux_i_hprots0     ), 
    .i_hbursts0        ( amux_i_hbursts0    ), 
    .i_hmastlocks0     ( amux_i_hmastlocks0 ), 
    .i_hwdatas0        ( amux_i_hwdatas0    ), 
    .o_hreadyouts0     ( amux_o_hreadyouts0 ), 
    .o_hresps0         ( amux_o_hresps0     ), 
    .o_hrdatas0        ( amux_o_hrdatas0    ), 
    .i_hsels1          ( amux_i_hsels1      ), 
    .i_haddrs1         ( amux_i_haddrs1     ), 
    .i_htranss1        ( amux_i_htranss1    ), 
    .i_hsizes1         ( amux_i_hsizes1     ), 
    .i_hwrites1        ( amux_i_hwrites1    ), 
    .i_hreadys1        ( amux_i_hreadys1    ), 
    .i_hprots1         ( amux_i_hprots1     ), 
    .i_hbursts1        ( amux_i_hbursts1    ), 
    .i_hmastlocks1     ( amux_i_hmastlocks1 ), 
    .i_hwdatas1        ( amux_i_hwdatas1    ), 
    .o_hreadyouts1     ( amux_o_hreadyouts1 ), 
    .o_hresps1         ( amux_o_hresps1     ), 
    .o_hrdatas1        ( amux_o_hrdatas1    ), 
    .i_hsels2          ( amux_i_hsels2      ), 
    .i_haddrs2         ( amux_i_haddrs2     ), 
    .i_htranss2        ( amux_i_htranss2    ), 
    .i_hsizes2         ( amux_i_hsizes2     ), 
    .i_hwrites2        ( amux_i_hwrites2    ), 
    .i_hreadys2        ( amux_i_hreadys2    ), 
    .i_hprots2         ( amux_i_hprots2     ), 
    .i_hbursts2        ( amux_i_hbursts2    ), 
    .i_hmastlocks2     ( amux_i_hmastlocks2 ), 
    .i_hwdatas2        ( amux_i_hwdatas2    ), 
    .o_hreadyouts2     ( amux_o_hreadyouts2 ), 
    .o_hresps2         ( amux_o_hresps2     ), 
    .o_hrdatas2        ( amux_o_hrdatas2    ), 
    .o_hselm           ( amux_o_hselm       ), 
    .o_haddrm          ( amux_o_haddrm      ), 
    .o_htransm         ( amux_o_htransm     ), 
    .o_hsizem          ( amux_o_hsizem      ), 
    .o_hwritem         ( amux_o_hwritem     ), 
    .o_hreadym         ( amux_o_hreadym     ), 
    .o_hprotm          ( amux_o_hprotm      ), 
    .o_hburstm         ( amux_o_hburstm     ), 
    .o_hmastlockm      ( amux_o_hmastlockm  ), 
    .o_hwdatam         ( amux_o_hwdatam     ), 
    .i_hreadyoutm      ( amux_i_hreadyoutm  ), 
    .i_hrespm          ( amux_i_hrespm      ), 
    .i_hrdatam         ( amux_i_hrdatam     ), 
    .o_hmasterm        ( amux_o_hmasterm    )  
    ); 

osr_ahb_downsizer64to32 #(
    .HMASTER_WIDTH           ( ADNSIZ_HMASTER_WIDTH       ), 
    .ERR_BURST_BLOCK_ALL     ( ADNSIZ_ERR_BURST_BLOCK_ALL )  
    ) u_ahb_64to32 (
    .i_hclk             ( adnsiz_i_hclk       ), 
    .i_hreset_n         ( adnsiz_i_hreset_n   ), 
    .i_hsels            ( adnsiz_i_hsels      ), 
    .i_haddrs           ( adnsiz_i_haddrs     ), 
    .i_htranss          ( adnsiz_i_htranss    ), 
    .i_hbursts          ( adnsiz_i_hbursts    ), 
    .i_hprots           ( adnsiz_i_hprots     ), 
    .i_hwrites          ( adnsiz_i_hwrites    ), 
    .i_hsizes           ( adnsiz_i_hsizes     ), 
    .i_hmastlocks       ( adnsiz_i_hmastlocks ), 
    .i_hreadys          ( adnsiz_i_hreadys    ), 
    .i_hwdatas          ( adnsiz_i_hwdatas    ), 
    .o_hrdatas          ( adnsiz_o_hrdatas    ), 
    .o_hresps           ( adnsiz_o_hresps     ), 
    .o_hreadyouts       ( adnsiz_o_hreadyouts ), 
    .o_hselm            ( adnsiz_o_hselm      ), 
    .o_haddrm           ( adnsiz_o_haddrm     ), 
    .o_htransm          ( adnsiz_o_htransm    ), 
    .o_hburstm          ( adnsiz_o_hburstm    ), 
    .o_hprotm           ( adnsiz_o_hprotm     ), 
    .o_hwritem          ( adnsiz_o_hwritem    ), 
    .o_hsizem           ( adnsiz_o_hsizem     ), 
    .o_hmastlockm       ( adnsiz_o_hmastlockm ), 
    .o_hwdatam          ( adnsiz_o_hwdatam    ), 
    .o_hreadym          ( adnsiz_o_hreadym    ), 
    .i_hrdatam          ( adnsiz_i_hrdatam    ), 
    .i_hrespm           ( adnsiz_i_hrespm     ), 
    .i_hreadyoutm       ( adnsiz_i_hreadyoutm )  
    ); 

always @(posedge i_sys_clk or negedge i_sys_rst_n) begin
    if(!i_sys_rst_n) begin
        sys_o_dma_wr_ctl_buf <= 2'b0                      ;
        sys_o_dc_dma_sel_buf <= 1'b0                      ;
    end else begin
        sys_o_dma_wr_ctl_buf <= sys_o_dma_wr_ctl_buf_next ;
        sys_o_dc_dma_sel_buf <= sys_o_dc_dma_sel_buf_next ;
    end
end

endmodule
