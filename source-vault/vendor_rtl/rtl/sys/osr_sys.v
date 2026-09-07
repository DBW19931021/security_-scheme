//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_sys (
    input  wire                 i_hclk                  ,
    input  wire                 i_hresetn               ,
    input  wire                 i_hsel                  ,
    input  wire [31:0]          i_haddr                 ,
    input  wire [ 1:0]          i_htrans                ,
    input  wire                 i_hwrite                ,
    input  wire [ 2:0]          i_hburst                ,
    input  wire [ 2:0]          i_hsize                 ,
    input  wire [ 3:0]          i_hprot                 ,
    input  wire                 i_hmastlock             ,
    input  wire                 i_hready                ,
    input  wire [31:0]          i_hwdata                ,
    output wire [31:0]          o_hrdata                ,
    output wire [ 1:0]          o_hresp                 ,
    output wire                 o_hreadyout             ,

    input  wire                 i_scan_mode             ,

    input  wire [31:0]          i_sensor                ,
    input  wire [31:0]          i_soc_status            ,
    input  wire [31:0]          i_boot_mode             ,           
    input  wire [31:0]          i_boot_sta              ,           

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

    output wire                 o_bus_cipher_en         , 
    output wire                 o_ram_cipher_en         ,
    output wire                 o_nvm_cipher_bypass     ,

    output wire [31:0]          o_axi_dma_cfg0          ,           
    output wire [31:0]          o_axi_dma_cfg1          ,           
    output wire [31:0]          o_axi_dma_ch0_cfg0      ,           
    output wire [31:0]          o_axi_dma_ch0_cfg1      ,           
    output wire [31:0]          o_axi_dma_ch0_cfg2      ,          
    output wire [31:0]          o_axi_dma_ch1_cfg0      ,           
    output wire [31:0]          o_axi_dma_ch1_cfg1      ,           
    output wire [31:0]          o_axi_dma_ch1_cfg2      ,           
    output wire [31:0]          o_axi_dma_ch2_cfg0      ,   
    output wire [31:0]          o_axi_dma_ch2_cfg1      ,           
    output wire [31:0]          o_axi_dma_ch2_cfg2      ,           
    output wire [31:0]          o_axi_dma_ch3_cfg0      ,           
    output wire [31:0]          o_axi_dma_ch3_cfg1      ,           
    output wire [31:0]          o_axi_dma_ch3_cfg2      ,         

    output wire [31:0]          o_irom_ecc_cfg          ,
    output wire [31:0]          o_iram_ecc_cfg          ,
    output wire [31:0]          o_dram_ecc_cfg          ,
    output wire [31:0]          o_kmu_ram_ecc_cfg       ,
    output wire [31:0]          o_pke_ram0_ecc_cfg      ,
    output wire [31:0]          o_pke_ram1_ecc_cfg      ,
    output wire [31:0]          o_pke_ram2_ecc_cfg      ,
    output wire [31:0]          o_pke_ram3_ecc_cfg      ,

    output wire [63:0]          o_mbox_ram_ba           ,

    output wire [31:0]          o_cpu_cfg               ,

    output wire [7:0]           o_ahb_dma_rd_wr_to      ,          
    output wire                 o_dc_dma_sel            ,          
    output wire [1:0]           o_dma_wr_ctl            ,          

    output wire                 o_otp_ahb_if_sel        ,

    output wire                 o_ahb_cfg_rsp_err_en    ,
    output wire                 o_ahb_nvm_rsp_err_en    ,
    output wire                 o_ahb_otp_rsp_err_en    ,
    output wire                 o_ahb_soc_rsp_err_en    ,

    output wire                 o_ahb_bus_pr_en         ,

    output wire [63:0]          o_hsm_status            , 

    input  wire                 i_lc_undef              , 
    input  wire                 i_lc_destroy            , 
    input  wire                 i_lc_debug              , 
    input  wire                 i_lc_user               , 
    input  wire                 i_lc_manu               , 
    input  wire                 i_lc_dev2               ,
    input  wire                 i_lc_dev                , 
    input  wire                 i_lc_test               , 

    input  wire [31:0]          i_life_cycle            ,
    input  wire [31:0]          i_con_field0            ,
    input  wire [31:0]          i_con_field1            ,
    input  wire [31:0]          i_con_field2            ,
    input  wire [31:0]          i_con_field3            ,
    input  wire [31:0]          i_con_field4            ,
    input  wire [31:0]          i_con_field5            ,

    input  wire [7:0]           i_hw_rd_otp_end         ,

    input  wire                 i_cpu_hart_halted       , 
    input  wire                 i_cpu_wfi               , 

    input  wire                 i_hw_boot_done_pulse    ,

    output wire                 o_hsm_dbg_en            ,
    output wire [127:0]         o_soc_dbg_en_128b       ,

    output wire                 o_trng_drbg_alg_sel     ,

    output wire                 o_sys_irq                
);

localparam     P_SYS_HW_INF0                    = 20'h00000 ;
localparam     P_SYS_HW_INF1                    = 20'h00004 ;
localparam     P_SYS_HW_INF2                    = 20'h00008 ;
localparam     P_SYS_HW_INF3                    = 20'h0000c ;
localparam     P_SYS_HW_INF4                    = 20'h00010 ;
localparam     P_SYS_HW_INF5                    = 20'h00014 ;
localparam     P_SYS_VER0                       = 20'h00080 ;
localparam     P_SYS_VER1                       = 20'h00084 ;
localparam     P_SYS_HSM_STA0                   = 20'h01000 ;
localparam     P_SYS_HSM_STA1                   = 20'h01004 ;
localparam     P_SYS_HSM_STA2                   = 20'h01008 ;
localparam     P_SYS_HSM_STA3                   = 20'h0100c ;
localparam     P_SYS_WR_HSM_DBG_EN              = 20'h01080 ;
localparam     P_SYS_WR_SOC_DBG_EN              = 20'h01100 ;
localparam     P_SYS_WR_SOC_DBG_EN_128B0        = 20'h01104 ;
localparam     P_SYS_WR_SOC_DBG_EN_128B1        = 20'h01108 ;
localparam     P_SYS_WR_SOC_DBG_EN_128B2        = 20'h0110c ;
localparam     P_SYS_WR_SOC_DBG_EN_128B3        = 20'h01110 ;
localparam     P_SYS_WR_SOC_CPU_RELEASE         = 20'h01200 ;
localparam     P_SYS_WR_SOC_CPU_RESET           = 20'h01240 ;
localparam     P_SYS_WR_SOC_RESET               = 20'h01280 ;
localparam     P_SYS_SOC_STA                    = 20'h02000 ;
localparam     P_SYS_SOC_STA_INT                = 20'h02800 ;
localparam     P_SYS_SOC_STA_INT_EN             = 20'h02880 ;
localparam     P_SYS_CLK_CTRL0                  = 20'h03000 ;
localparam     P_SYS_CLK_CTRL1                  = 20'h03004 ;
localparam     P_SYS_ALG_CLK_CTRL0              = 20'h03100 ;
localparam     P_SYS_ALG_CLK_CTRL1              = 20'h03104 ;
localparam     P_SYS_ALG_CLK_CTRL2              = 20'h03108 ;
localparam     P_SYS_ALG_CLK_CTRL3              = 20'h0310c ;
localparam     P_SYS_ALG_CLK_CTRL4              = 20'h03110 ;
localparam     P_SYS_ALG_CLK_CTRL5              = 20'h03114 ;
localparam     P_SYS_ALG_CLK_CTRL6              = 20'h03118 ;
localparam     P_SYS_RST_CTRL0                  = 20'h03200 ;
localparam     P_SYS_RST_CTRL1                  = 20'h03204 ;
localparam     P_SYS_ALG_RST_CTRL0              = 20'h03300 ;
localparam     P_SYS_ALG_RST_CTRL1              = 20'h03304 ;
localparam     P_SYS_ALG_RST_CTRL2              = 20'h03308 ;
localparam     P_SYS_ALG_RST_CTRL3              = 20'h0330c ;
localparam     P_SYS_ALG_RST_CTRL4              = 20'h03310 ;
localparam     P_SYS_ALG_RST_CTRL5              = 20'h03314 ;
localparam     P_SYS_ALG_RST_CTRL6              = 20'h03318 ;
localparam     P_SYS_CIPHER_CTRL                = 20'h03400 ;
localparam     P_SYS_SOC_MEM_BAL                = 20'h03800 ;
localparam     P_SYS_SOC_MEM_BAH                = 20'h03804 ;
localparam     P_SYS_TRNG_DRBG_ALG              = 20'h03900 ;
localparam     P_SYS_CPU_CFG                    = 20'h04000 ;
localparam     P_SYS_BUS_ERR_CFG                = 20'h05000 ;
localparam     P_SYS_BUS_PR_CFG                 = 20'h05100 ;
localparam     P_SYS_AHB_DMA_CFG                = 20'h06000 ;
localparam     P_SYS_AXI_DMA_CFG0               = 20'h06800 ;
localparam     P_SYS_AXI_DMA_CFG1               = 20'h06804 ;
localparam     P_SYS_AXI_DMA_CH0_CFG0           = 20'h06880 ;
localparam     P_SYS_AXI_DMA_CH0_CFG1           = 20'h06884 ;
localparam     P_SYS_AXI_DMA_CH0_CFG2           = 20'h06888 ;
localparam     P_SYS_AXI_DMA_CH1_CFG0           = 20'h068a0 ;
localparam     P_SYS_AXI_DMA_CH1_CFG1           = 20'h068a4 ;
localparam     P_SYS_AXI_DMA_CH1_CFG2           = 20'h068a8 ;
localparam     P_SYS_AXI_DMA_CH2_CFG0           = 20'h068c0 ;
localparam     P_SYS_AXI_DMA_CH2_CFG1           = 20'h068c4 ;
localparam     P_SYS_AXI_DMA_CH2_CFG2           = 20'h068c8 ;
localparam     P_SYS_AXI_DMA_CH3_CFG0           = 20'h068e0 ;
localparam     P_SYS_AXI_DMA_CH3_CFG1           = 20'h068e4 ;
localparam     P_SYS_AXI_DMA_CH3_CFG2           = 20'h068e8 ;
localparam     P_SYS_CPU_ECC_CTRL0              = 20'h07000 ;
localparam     P_SYS_CPU_ECC_CTRL1              = 20'h07004 ;
localparam     P_SYS_CPU_ECC_CTRL2              = 20'h07008 ;
localparam     P_SYS_KMU_ECC_CTRL               = 20'h07100 ;
localparam     P_SYS_PKE_ECC_CTRL0              = 20'h07200 ;
localparam     P_SYS_PKE_ECC_CTRL1              = 20'h07204 ;
localparam     P_SYS_PKE_ECC_CTRL2              = 20'h07208 ;
localparam     P_SYS_PKE_ECC_CTRL3              = 20'h0720c ;
localparam     P_LIFE_CYCLE                     = 20'h08000 ;
localparam     P_HW_CONTROL0                    = 20'h08080 ;
localparam     P_HW_CONTROL1                    = 20'h08084 ;
localparam     P_FW_CONTROL0                    = 20'h08100 ;
localparam     P_FW_CONTROL1                    = 20'h08104 ;
localparam     P_FW_CONTROL2                    = 20'h08108 ;
localparam     P_FW_CONTROL3                    = 20'h0810c ;
localparam     P_SYS_GEN_REG0                   = 20'hf0800 ;
localparam     P_SYS_GEN_REG1                   = 20'hf0804 ;
localparam     P_SYS_GEN_REG2                   = 20'hf0808 ;
localparam     P_SYS_GEN_REG3                   = 20'hf080c ;
localparam     P_SYS_GEN_REG4                   = 20'hf0810 ;
localparam     P_SYS_GEN_REG5                   = 20'hf0814 ;
localparam     P_SYS_GEN_REG6                   = 20'hf0818 ;
localparam     P_SYS_GEN_REG7                   = 20'hf081c ;
localparam     P_SYS_GEN_REG8                   = 20'hf0820 ;
localparam     P_SYS_GEN_REG9                   = 20'hf0824 ;
localparam     P_SYS_GEN_REG10                  = 20'hf0828 ;
localparam     P_SYS_GEN_REG11                  = 20'hf082c ;
localparam     P_SYS_GEN_REG12                  = 20'hf0830 ;
localparam     P_SYS_GEN_REG13                  = 20'hf0834 ;
localparam     P_SYS_GEN_REG14                  = 20'hf0838 ;
localparam     P_SYS_GEN_REG15                  = 20'hf083c ;
localparam     P_SYS_RUN_STEP                   = 20'hf0f00 ;

wire [4:0]     w_cpu_dram_width = 16 ;
wire [4:0]     w_cpu_irom_width = 16 ;
wire [4:0]     w_cpu_iram_width = 18 ;

reg             r_wr_en                 ;
reg [17:0]      r_addr                  ;
reg [31:0]      r_hrdata                ;
reg             r_nvm_cipher_bypass     ;
reg             r_bus_cipher_en         ;
reg             r_ram_cipher_en         ;
reg             hsm_fw_sta0             ;
reg             soc_verify_err          ;
reg             soc_verify_done         ;
reg             firmware_err            ;
reg             firmware_done           ;
reg             bootloader_err          ;
reg             bootloader_done         ;
reg [3:0]       hsm_fw_sta1             ;
reg [10:0]      hsm_fw_sta2             ;
reg             fw_upgrade_working      ;
reg             fw_backup_error         ;
reg             fw_backup_done          ;
reg             fw_upgrade_error        ;
reg             fw_upgrade_done         ;
reg [15:0]      hsm_fw_sta3             ;
reg [31:0]      wr_hsm_dbg_en           ;
reg [31:0]      wr_soc_dbg_en           ;
reg [31:0]      wr_soc_dbg_en_128b0     ;
reg [31:0]      wr_soc_dbg_en_128b1     ;
reg [31:0]      wr_soc_dbg_en_128b2     ;
reg [31:0]      wr_soc_dbg_en_128b3     ;
reg [31:0]      wr_soc_cpu_release      ;
reg [31:0]      wr_soc_cpu_reset        ;
reg [31:0]      wr_soc_reset            ;
reg [31:0]      sys_soc_sta_int         ;
reg [31:0]      sys_soc_sta_int_en      ;
reg             axi_dma_clk_en          ;
reg             ahb_dma_clk_en          ;
reg             debug_mailbox_clk_en    ;
reg             mailbox_clk_en          ;
reg             kmu_clk_en              ;
reg             irom_clk_en             ;
reg             emu_clk_en              ;
reg             boot_clk_en             ;
reg             mono_cnt_clk_en         ;
reg             utc_tim_clk_en          ;
reg             tim_clk_en              ;
reg             apb_nvm_cipher_clk_en   ;
reg             crc_clk_en              ;
reg             uart_clk_en             ;
reg             wdt1_clk_en             ;
reg             wdt_clk_en              ;
reg             hash0_clk_dma_en        ;
reg             hash0_clk_en            ;
reg             ske0_clk_dma_en         ;
reg             ske0_clk_en             ;
reg             pke_clk_en              ;
reg             trng_clk_en             ;
reg             chacha_clk_dma_en       ;
reg             chacha_clk_en           ;
reg             pqc_spxp_clk_dma_en     ;
reg             pqc_spxp_clk_en         ;
reg             pqc_kd_clk_en           ;
reg             hsm_with_boot_rst       ;
reg             hsm_rst                 ;
reg             cpu_rst                 ;
reg             axi_dma_rst             ;
reg             ahb_dma_rst             ;
reg             debug_mailbox_rst       ;
reg             mailbox_rst             ;
reg             kmu_rst                 ;
reg             irom_rst                ;
reg             emu_rst                 ;
reg             sys_rst                 ;
reg             boot_rst                ;
reg             mono_cnt_rst            ;
reg             utc_tim_rst             ;
reg             tim_rst                 ;
reg             apb_nvm_cipher_rst      ;
reg             crc_rst                 ;
reg             uart_rst                ;
reg             wdt1_rst                ;
reg             wdt_rst                 ;
reg             hash0_dma_rst           ;
reg             hash0_rst               ;
reg             ske0_dma_rst            ;
reg             ske0_rst                ;
reg             pke_rst                 ;
reg             trng_rst                ;
reg             chacha_dma_rst          ;
reg             chacha_rst              ;
reg             pqc_spxp_dma_rst        ;
reg             pqc_spxp_rst            ;
reg             pqc_kd_rst              ;
reg [31:0]      soc_mem_bal             ;
reg [31:0]      soc_mem_bah             ;
reg             trng_drbg_alg           ;
reg [25:0]      cpu_stcalib_cfg         ;
reg             ahb_cfg_rsp_err_en      ;
reg             ahb_nvm_rsp_err_en      ;
reg             ahb_otp_rsp_err_en      ;
reg             ahb_soc_rsp_err_en      ;
reg             ahb_bus_pr_en           ;
reg [7:0]       ahb_dma_rd_wr_to        ;
reg             otp_ahb_if_sel          ;
reg [1:0]       dma_wr_ctl              ;
reg             dc_dma_sel              ;
reg [15:0]      axi_dma_rd_wr_to        ;
reg [3:0]       axi_dma_ch0_arsnoop     ;
reg [1:0]       axi_dma_ch0_arbar       ;
reg [1:0]       axi_dma_ch0_ardomain    ;
reg [1:0]       axi_dma_ch0_arlock      ;
reg [3:0]       axi_dma_ch0_arcache     ;
reg [2:0]       axi_dma_ch0_arprot      ;
reg [3:0]       axi_dma_ch0_arqos       ;
reg [3:0]       axi_dma_ch0_arregion    ;
reg [2:0]       axi_dma_ch0_awsnoop     ;
reg [1:0]       axi_dma_ch0_awbar       ;
reg [1:0]       axi_dma_ch0_awdomain    ;
reg [1:0]       axi_dma_ch0_awlock      ;
reg [3:0]       axi_dma_ch0_awcache     ;
reg [2:0]       axi_dma_ch0_awprot      ;
reg [3:0]       axi_dma_ch0_awqos       ;
reg [3:0]       axi_dma_ch0_awregion    ;
reg [3:0]       axi_dma_ch1_arsnoop     ;
reg [1:0]       axi_dma_ch1_arbar       ;
reg [1:0]       axi_dma_ch1_ardomain    ;
reg [1:0]       axi_dma_ch1_arlock      ;
reg [3:0]       axi_dma_ch1_arcache     ;
reg [2:0]       axi_dma_ch1_arprot      ;
reg [3:0]       axi_dma_ch1_arqos       ;
reg [3:0]       axi_dma_ch1_arregion    ;
reg [2:0]       axi_dma_ch1_awsnoop     ;
reg [1:0]       axi_dma_ch1_awbar       ;
reg [1:0]       axi_dma_ch1_awdomain    ;
reg [1:0]       axi_dma_ch1_awlock      ;
reg [3:0]       axi_dma_ch1_awcache     ;
reg [2:0]       axi_dma_ch1_awprot      ;
reg [3:0]       axi_dma_ch1_awqos       ;
reg [3:0]       axi_dma_ch1_awregion    ;
reg [3:0]       axi_dma_ch2_arsnoop     ;
reg [1:0]       axi_dma_ch2_arbar       ;
reg [1:0]       axi_dma_ch2_ardomain    ;
reg [1:0]       axi_dma_ch2_arlock      ;
reg [3:0]       axi_dma_ch2_arcache     ;
reg [2:0]       axi_dma_ch2_arprot      ;
reg [3:0]       axi_dma_ch2_arqos       ;
reg [3:0]       axi_dma_ch2_arregion    ;
reg [2:0]       axi_dma_ch2_awsnoop     ;
reg [1:0]       axi_dma_ch2_awbar       ;
reg [1:0]       axi_dma_ch2_awdomain    ;
reg [1:0]       axi_dma_ch2_awlock      ;
reg [3:0]       axi_dma_ch2_awcache     ;
reg [2:0]       axi_dma_ch2_awprot      ;
reg [3:0]       axi_dma_ch2_awqos       ;
reg [3:0]       axi_dma_ch2_awregion    ;
reg [3:0]       axi_dma_ch3_arsnoop     ;
reg [1:0]       axi_dma_ch3_arbar       ;
reg [1:0]       axi_dma_ch3_ardomain    ;
reg [1:0]       axi_dma_ch3_arlock      ;
reg [3:0]       axi_dma_ch3_arcache     ;
reg [2:0]       axi_dma_ch3_arprot      ;
reg [3:0]       axi_dma_ch3_arqos       ;
reg [3:0]       axi_dma_ch3_arregion    ;
reg [2:0]       axi_dma_ch3_awsnoop     ;
reg [1:0]       axi_dma_ch3_awbar       ;
reg [1:0]       axi_dma_ch3_awdomain    ;
reg [1:0]       axi_dma_ch3_awlock      ;
reg [3:0]       axi_dma_ch3_awcache     ;
reg [2:0]       axi_dma_ch3_awprot      ;
reg [3:0]       axi_dma_ch3_awqos       ;
reg [3:0]       axi_dma_ch3_awregion    ;
reg [7:0]       cpu_irom_ecc_ckbits     ;
reg [7:0]       cpu_irom_ecc_tm_en      ;
reg [7:0]       cpu_irom_ecc_err_rsp_en ;
reg [7:0]       cpu_irom_ecc_ck_en      ;
reg [7:0]       cpu_iram_ecc_ckbits     ;
reg [7:0]       cpu_iram_ecc_tm_en      ;
reg [7:0]       cpu_iram_ecc_err_rsp_en ;
reg [7:0]       cpu_iram_ecc_ck_en      ;
reg [7:0]       cpu_dram_ecc_ckbits     ;
reg [7:0]       cpu_dram_ecc_tm_en      ;
reg [7:0]       cpu_dram_ecc_err_rsp_en ;
reg [7:0]       cpu_dram_ecc_ck_en      ;
reg [9:0]       kmu_ram_ecc_ckbits      ;
reg [7:0]       kmu_ram_ecc_tm_en       ;
reg [7:0]       kmu_ram_ecc_ck_en       ;
reg [9:0]       pke_ram0_ecc_ckbits     ;
reg [7:0]       pke_ram0_ecc_tm_en      ;
reg [7:0]       pke_ram0_ecc_ck_en      ;
reg [9:0]       pke_ram1_ecc_ckbits     ;
reg [7:0]       pke_ram1_ecc_tm_en      ;
reg [7:0]       pke_ram1_ecc_ck_en      ;
reg [9:0]       pke_ram2_ecc_ckbits     ;
reg [7:0]       pke_ram2_ecc_tm_en      ;
reg [7:0]       pke_ram2_ecc_ck_en      ;
reg [9:0]       pke_ram3_ecc_ckbits     ;
reg [7:0]       pke_ram3_ecc_tm_en      ;
reg [7:0]       pke_ram3_ecc_ck_en      ;
reg [31:0]      gen_reg0                ;
reg [31:0]      gen_reg1                ;
reg [31:0]      gen_reg2                ;
reg [31:0]      gen_reg3                ;
reg [31:0]      gen_reg4                ;
reg [31:0]      gen_reg5                ;
reg [31:0]      gen_reg6                ;
reg [31:0]      gen_reg7                ;
reg [31:0]      gen_reg8                ;
reg [31:0]      gen_reg9                ;
reg [31:0]      gen_reg10               ;
reg [31:0]      gen_reg11               ;
reg [31:0]      gen_reg12               ;
reg [31:0]      gen_reg13               ;
reg [31:0]      gen_reg14               ;
reg [31:0]      gen_reg15               ;
reg [31:0]      sys_run_step            ;
reg             r_soc_cpu_release       ;
reg             r_soc_cpu_reset         ;
reg             r_soc_reset             ;
reg             r_soc_dbg_en            ;
reg             r_hsm_dbg_en            ;

wire             tdt_dm_core_unavail     = 1'h0 ; 

wire         bus_vld       = i_hsel & i_htrans[1] & i_hready & o_hreadyout & (i_hsize == 3'h2);
wire         r_wr_en_next  = bus_vld &  i_hwrite;
wire         rd_en         = bus_vld & ~i_hwrite;
wire [17:0]  r_addr_next   = i_haddr[19:2] ; 
wire [19:0]  addr_wr       = {r_addr,2'h0};
wire [19:0]  addr_rd       = {i_haddr[19:2],2'h0};
reg  [31:0]  r_hrdata_next ; 

wire     wr_sys_hsm_sta0                  = ( addr_wr == P_SYS_HSM_STA0                 ) & r_wr_en ;
wire     wr_sys_hsm_sta1                  = ( addr_wr == P_SYS_HSM_STA1                 ) & r_wr_en ;
wire     wr_sys_hsm_sta2                  = ( addr_wr == P_SYS_HSM_STA2                 ) & r_wr_en ;
wire     wr_sys_hsm_sta3                  = ( addr_wr == P_SYS_HSM_STA3                 ) & r_wr_en ;
wire     wr_sys_wr_hsm_dbg_en             = ( addr_wr == P_SYS_WR_HSM_DBG_EN            ) & r_wr_en ;
wire     wr_sys_wr_soc_dbg_en             = ( addr_wr == P_SYS_WR_SOC_DBG_EN            ) & r_wr_en ;
wire     wr_sys_wr_soc_dbg_en_128B0       = ( addr_wr == P_SYS_WR_SOC_DBG_EN_128B0      ) & r_wr_en ;
wire     wr_sys_wr_soc_dbg_en_128B1       = ( addr_wr == P_SYS_WR_SOC_DBG_EN_128B1      ) & r_wr_en ;
wire     wr_sys_wr_soc_dbg_en_128B2       = ( addr_wr == P_SYS_WR_SOC_DBG_EN_128B2      ) & r_wr_en ;
wire     wr_sys_wr_soc_dbg_en_128B3       = ( addr_wr == P_SYS_WR_SOC_DBG_EN_128B3      ) & r_wr_en ;
wire     wr_sys_wr_soc_cpu_release        = ( addr_wr == P_SYS_WR_SOC_CPU_RELEASE       ) & r_wr_en ;
wire     wr_sys_wr_soc_cpu_reset          = ( addr_wr == P_SYS_WR_SOC_CPU_RESET         ) & r_wr_en ;
wire     wr_sys_wr_soc_reset              = ( addr_wr == P_SYS_WR_SOC_RESET             ) & r_wr_en ;
wire     wr_sys_soc_sta_int               = ( addr_wr == P_SYS_SOC_STA_INT              ) & r_wr_en ;
wire     wr_sys_soc_sta_int_en            = ( addr_wr == P_SYS_SOC_STA_INT_EN           ) & r_wr_en ;
wire     wr_sys_clk_ctrl0                 = ( addr_wr == P_SYS_CLK_CTRL0                ) & r_wr_en ;
wire     wr_sys_clk_ctrl1                 = ( addr_wr == P_SYS_CLK_CTRL1                ) & r_wr_en ;
wire     wr_sys_alg_clk_ctrl0             = ( addr_wr == P_SYS_ALG_CLK_CTRL0            ) & r_wr_en ;
wire     wr_sys_alg_clk_ctrl1             = ( addr_wr == P_SYS_ALG_CLK_CTRL1            ) & r_wr_en ;
wire     wr_sys_alg_clk_ctrl2             = ( addr_wr == P_SYS_ALG_CLK_CTRL2            ) & r_wr_en ;
wire     wr_sys_alg_clk_ctrl3             = ( addr_wr == P_SYS_ALG_CLK_CTRL3            ) & r_wr_en ;
wire     wr_sys_alg_clk_ctrl4             = ( addr_wr == P_SYS_ALG_CLK_CTRL4            ) & r_wr_en ;
wire     wr_sys_alg_clk_ctrl5             = ( addr_wr == P_SYS_ALG_CLK_CTRL5            ) & r_wr_en ;
wire     wr_sys_alg_clk_ctrl6             = ( addr_wr == P_SYS_ALG_CLK_CTRL6            ) & r_wr_en ;
wire     wr_sys_rst_ctrl0                 = ( addr_wr == P_SYS_RST_CTRL0                ) & r_wr_en ;
wire     wr_sys_rst_ctrl1                 = ( addr_wr == P_SYS_RST_CTRL1                ) & r_wr_en ;
wire     wr_sys_alg_rst_ctrl0             = ( addr_wr == P_SYS_ALG_RST_CTRL0            ) & r_wr_en ;
wire     wr_sys_alg_rst_ctrl1             = ( addr_wr == P_SYS_ALG_RST_CTRL1            ) & r_wr_en ;
wire     wr_sys_alg_rst_ctrl2             = ( addr_wr == P_SYS_ALG_RST_CTRL2            ) & r_wr_en ;
wire     wr_sys_alg_rst_ctrl3             = ( addr_wr == P_SYS_ALG_RST_CTRL3            ) & r_wr_en ;
wire     wr_sys_alg_rst_ctrl4             = ( addr_wr == P_SYS_ALG_RST_CTRL4            ) & r_wr_en ;
wire     wr_sys_alg_rst_ctrl5             = ( addr_wr == P_SYS_ALG_RST_CTRL5            ) & r_wr_en ;
wire     wr_sys_alg_rst_ctrl6             = ( addr_wr == P_SYS_ALG_RST_CTRL6            ) & r_wr_en ;
wire     wr_sys_cipher_ctrl               = ( addr_wr == P_SYS_CIPHER_CTRL              ) & r_wr_en ;
wire     wr_sys_soc_mem_bal               = ( addr_wr == P_SYS_SOC_MEM_BAL              ) & r_wr_en ;
wire     wr_sys_soc_mem_bah               = ( addr_wr == P_SYS_SOC_MEM_BAH              ) & r_wr_en ;
wire     wr_sys_trng_drbg_alg             = ( addr_wr == P_SYS_TRNG_DRBG_ALG            ) & r_wr_en ;
wire     wr_sys_cpu_cfg                   = ( addr_wr == P_SYS_CPU_CFG                  ) & r_wr_en ;
wire     wr_sys_bus_err_cfg               = ( addr_wr == P_SYS_BUS_ERR_CFG              ) & r_wr_en ;
wire     wr_sys_bus_pr_cfg                = ( addr_wr == P_SYS_BUS_PR_CFG               ) & r_wr_en ;
wire     wr_sys_ahb_dma_cfg               = ( addr_wr == P_SYS_AHB_DMA_CFG              ) & r_wr_en ;
wire     wr_sys_axi_dma_cfg1              = ( addr_wr == P_SYS_AXI_DMA_CFG1             ) & r_wr_en ;
wire     wr_sys_axi_dma_ch0_cfg1          = ( addr_wr == P_SYS_AXI_DMA_CH0_CFG1         ) & r_wr_en ;
wire     wr_sys_axi_dma_ch0_cfg2          = ( addr_wr == P_SYS_AXI_DMA_CH0_CFG2         ) & r_wr_en ;
wire     wr_sys_axi_dma_ch1_cfg1          = ( addr_wr == P_SYS_AXI_DMA_CH1_CFG1         ) & r_wr_en ;
wire     wr_sys_axi_dma_ch1_cfg2          = ( addr_wr == P_SYS_AXI_DMA_CH1_CFG2         ) & r_wr_en ;
wire     wr_sys_axi_dma_ch2_cfg1          = ( addr_wr == P_SYS_AXI_DMA_CH2_CFG1         ) & r_wr_en ;
wire     wr_sys_axi_dma_ch2_cfg2          = ( addr_wr == P_SYS_AXI_DMA_CH2_CFG2         ) & r_wr_en ;
wire     wr_sys_axi_dma_ch3_cfg1          = ( addr_wr == P_SYS_AXI_DMA_CH3_CFG1         ) & r_wr_en ;
wire     wr_sys_axi_dma_ch3_cfg2          = ( addr_wr == P_SYS_AXI_DMA_CH3_CFG2         ) & r_wr_en ;
wire     wr_sys_cpu_ecc_ctrl0             = ( addr_wr == P_SYS_CPU_ECC_CTRL0            ) & r_wr_en ;
wire     wr_sys_cpu_ecc_ctrl1             = ( addr_wr == P_SYS_CPU_ECC_CTRL1            ) & r_wr_en ;
wire     wr_sys_cpu_ecc_ctrl2             = ( addr_wr == P_SYS_CPU_ECC_CTRL2            ) & r_wr_en ;
wire     wr_sys_kmu_ecc_ctrl              = ( addr_wr == P_SYS_KMU_ECC_CTRL             ) & r_wr_en ;
wire     wr_sys_pke_ecc_ctrl0             = ( addr_wr == P_SYS_PKE_ECC_CTRL0            ) & r_wr_en ;
wire     wr_sys_pke_ecc_ctrl1             = ( addr_wr == P_SYS_PKE_ECC_CTRL1            ) & r_wr_en ;
wire     wr_sys_pke_ecc_ctrl2             = ( addr_wr == P_SYS_PKE_ECC_CTRL2            ) & r_wr_en ;
wire     wr_sys_pke_ecc_ctrl3             = ( addr_wr == P_SYS_PKE_ECC_CTRL3            ) & r_wr_en ;
wire     wr_sys_gen_reg0                  = ( addr_wr == P_SYS_GEN_REG0                 ) & r_wr_en ;
wire     wr_sys_gen_reg1                  = ( addr_wr == P_SYS_GEN_REG1                 ) & r_wr_en ;
wire     wr_sys_gen_reg2                  = ( addr_wr == P_SYS_GEN_REG2                 ) & r_wr_en ;
wire     wr_sys_gen_reg3                  = ( addr_wr == P_SYS_GEN_REG3                 ) & r_wr_en ;
wire     wr_sys_gen_reg4                  = ( addr_wr == P_SYS_GEN_REG4                 ) & r_wr_en ;
wire     wr_sys_gen_reg5                  = ( addr_wr == P_SYS_GEN_REG5                 ) & r_wr_en ;
wire     wr_sys_gen_reg6                  = ( addr_wr == P_SYS_GEN_REG6                 ) & r_wr_en ;
wire     wr_sys_gen_reg7                  = ( addr_wr == P_SYS_GEN_REG7                 ) & r_wr_en ;
wire     wr_sys_gen_reg8                  = ( addr_wr == P_SYS_GEN_REG8                 ) & r_wr_en ;
wire     wr_sys_gen_reg9                  = ( addr_wr == P_SYS_GEN_REG9                 ) & r_wr_en ;
wire     wr_sys_gen_reg10                 = ( addr_wr == P_SYS_GEN_REG10                ) & r_wr_en ;
wire     wr_sys_gen_reg11                 = ( addr_wr == P_SYS_GEN_REG11                ) & r_wr_en ;
wire     wr_sys_gen_reg12                 = ( addr_wr == P_SYS_GEN_REG12                ) & r_wr_en ;
wire     wr_sys_gen_reg13                 = ( addr_wr == P_SYS_GEN_REG13                ) & r_wr_en ;
wire     wr_sys_gen_reg14                 = ( addr_wr == P_SYS_GEN_REG14                ) & r_wr_en ;
wire     wr_sys_gen_reg15                 = ( addr_wr == P_SYS_GEN_REG15                ) & r_wr_en ;
wire     wr_sys_run_step                  = ( addr_wr == P_SYS_RUN_STEP                 ) & r_wr_en ;

wire write_sta0_valid_trig        = wr_sys_hsm_sta0                & (i_hwdata[7:0]      == 8'h65);

wire write_sta1_valid_trig        = wr_sys_hsm_sta1                & (i_hwdata[7:0]      == 8'h93);

wire write_sta2_valid_trig        = wr_sys_hsm_sta2                & (i_hwdata[7:0]      == 8'hA6);

wire write_sta3_valid_trig        = wr_sys_hsm_sta3                & (i_hwdata[7:0]      == 8'hC9);

wire write_cc0_valid_trig         = wr_sys_clk_ctrl0               & (i_hwdata[7:0]      == 8'h5A);

wire write_cc1_valid_trig         = wr_sys_clk_ctrl1               & (i_hwdata[7:0]      == 8'h5A);

wire write_acc0_valid_trig        = wr_sys_alg_clk_ctrl0           & (i_hwdata[7:0]      == 8'h5A);

wire write_acc1_valid_trig        = wr_sys_alg_clk_ctrl1           & (i_hwdata[7:0]      == 8'h5A);

wire write_acc2_valid_trig        = wr_sys_alg_clk_ctrl2           & (i_hwdata[7:0]      == 8'h5A);

wire write_acc3_valid_trig        = wr_sys_alg_clk_ctrl3           & (i_hwdata[7:0]      == 8'h5A);

wire write_acc4_valid_trig        = wr_sys_alg_clk_ctrl4           & (i_hwdata[7:0]      == 8'h5A);

wire write_acc5_valid_trig        = wr_sys_alg_clk_ctrl5           & (i_hwdata[7:0]      == 8'h5A);

wire write_acc6_valid_trig        = wr_sys_alg_clk_ctrl6           & (i_hwdata[7:0]      == 8'h5A);

wire write_rc0_valid_trig         = wr_sys_rst_ctrl0               & (i_hwdata[7:0]      == 8'h5A);

wire write_rc1_valid_trig         = wr_sys_rst_ctrl1               & (i_hwdata[7:0]      == 8'h5A);

wire write_arc0_valid_trig        = wr_sys_alg_rst_ctrl0           & (i_hwdata[7:0]      == 8'h5A);

wire write_arc1_valid_trig        = wr_sys_alg_rst_ctrl1           & (i_hwdata[7:0]      == 8'h5A);

wire write_arc2_valid_trig        = wr_sys_alg_rst_ctrl2           & (i_hwdata[7:0]      == 8'h5A);

wire write_arc3_valid_trig        = wr_sys_alg_rst_ctrl3           & (i_hwdata[7:0]      == 8'h5A);

wire write_arc4_valid_trig        = wr_sys_alg_rst_ctrl4           & (i_hwdata[7:0]      == 8'h5A);

wire write_arc5_valid_trig        = wr_sys_alg_rst_ctrl5           & (i_hwdata[7:0]      == 8'h5A);

wire write_arc6_valid_trig        = wr_sys_alg_rst_ctrl6           & (i_hwdata[7:0]      == 8'h5A);

wire nvm_cipher_bypass_trig       = wr_sys_cipher_ctrl             & i_hwdata[2]        ;
wire bus_cipher_en_trig           = wr_sys_cipher_ctrl             & i_hwdata[1]        ;
wire ram_cipher_en_trig           = wr_sys_cipher_ctrl             & i_hwdata[0]        ;

wire r_nvm_cipher_bypass_next = r_nvm_cipher_bypass | nvm_cipher_bypass_trig ; 
wire r_bus_cipher_en_next = r_bus_cipher_en | bus_cipher_en_trig ; 
wire r_ram_cipher_en_next = r_ram_cipher_en | ram_cipher_en_trig ; 

wire [15:0] w_pc = 16'h4019 ;

wire w_otp_default_1 = 1'h0 ;

`ifdef OSR_FPGA    
wire w_define_fpga = 1'h1 ; 
`else                               
wire w_define_fpga = 1'h0 ; 
`endif

wire w_osr_cpu_tmr = 1'h1 ;

wire w_osr_cpu_dbg = 1'h1 ;    

wire w_cpu_dram_byte_opt = 1'h1 ;    

wire w_cpu_iram_byte_opt = 1'h1 ;    

wire w_cpu_irom_byte_opt = 1'h1 ;    

wire w_pke_ram_ecc = 1'h1 ; 

wire w_kmu_ram_cipher = 1'h0 ;

wire w_cpu_dram_cipher = 1'h0 ;

wire w_cpu_iram_cipher = 1'h0 ;

wire w_cpu_irom_cipher = 1'h0 ;

wire w_kmu_ram_ecc = 1'h1 ; 

wire w_cpu_dram_ecc = 1'h1 ; 

wire w_cpu_iram_ecc = 1'h1 ; 

wire w_cpu_irom_ecc = 1'h1 ; 

wire [31:0] w_cpu_reset_vector = 32'h1000_0000 ;

wire            otp_polarity            = w_otp_default_1               ; 
wire            define_fpga             = w_define_fpga                 ;
wire [4:0]      cpu_dram_width          = w_cpu_dram_width              ; 
wire [4:0]      cpu_irom_width          = w_cpu_irom_width              ;
wire [4:0]      cpu_iram_width          = w_cpu_iram_width              ;
wire            cpu_dram_byte           = w_cpu_dram_byte_opt           ;
wire            cpu_iram_byte           = w_cpu_iram_byte_opt           ;
wire            cpu_irom_byte           = w_cpu_irom_byte_opt           ;
wire            cpu_tmr                 = w_osr_cpu_tmr                 ; 
wire            cpu_dbg                 = w_osr_cpu_dbg                 ; 

wire [31:0]     cpu_reset_vector        = w_cpu_reset_vector            ; 

wire            kmu_ram_cipher          = w_kmu_ram_cipher              ; 
wire            cpu_dram_cipher         = w_cpu_dram_cipher             ; 
wire            cpu_iram_cipher         = w_cpu_iram_cipher             ; 
wire            cpu_irom_cipher         = w_cpu_irom_cipher             ; 

wire            kmu_ram_ecc             = w_kmu_ram_ecc                 ; 
wire            cpu_dram_ecc            = w_cpu_dram_ecc                ; 
wire            cpu_iram_ecc            = w_cpu_iram_ecc                ; 
wire            cpu_irom_ecc            = w_cpu_irom_ecc                ; 

wire            pke_ecc_en              = w_pke_ram_ecc                 ; 
wire            pke_cdc_en              = 1'h0                ; 

wire [7:0]      hw_rd_otp_end           = i_hw_rd_otp_end               ;

wire [11:0]     con_yq                  = 12'h0                         ; 
wire [3:0]      qnum                    = 4'h0                          ; 

wire [15:0]     client_code             = w_pc                          ; 
wire [15:0]     git_tag                 = 16'h110a                      ; 

wire            hsm_lc_undef            = i_lc_undef                    ; 
wire            hsm_lc_destroy          = i_lc_destroy                  ;
wire            hsm_lc_debug            = i_lc_debug                    ;
wire            hsm_lc_user             = i_lc_user                     ;
wire            hsm_lc_manu             = i_lc_manu                     ;
wire            hsm_lc_dev0_or_dev      = i_lc_dev | i_lc_dev2          ;
wire            hsm_lc_test             = i_lc_test                     ;
wire            hw_boot_err             = i_boot_sta[1]                 ; 
wire            hw_boot_done            = i_boot_sta[0]                 ; 
wire [7:0]      write_sta0_valid        = 8'h0                          ;

wire            hsm_lc_dev              = i_lc_dev2                     ;
wire            cpu_hart_halted         = i_cpu_hart_halted             ; 
wire            cpu_wfi                 = i_cpu_wfi                     ; 
wire            soc_cpu_release         = r_soc_cpu_release             ; 
wire            soc_cpu_reset           = r_soc_cpu_reset               ; 
wire            soc_reset               = r_soc_reset                   ; 
wire            soc_dbg_en              = r_soc_dbg_en                  ; 
wire            hsm_dbg_en              = r_hsm_dbg_en                  ; 
wire [7:0]      write_sta1_valid        = 8'h0                          ; 

wire [7:0]      write_sta2_valid        = 8'h0                          ; 

wire [7:0]      write_sta3_valid        = 8'h0                          ; 

wire [31:0]     sys_soc_sta             = i_soc_status                  ;

wire [7:0]      write_cc0_valid         = 8'h0                          ; 

wire [7:0]      write_cc1_valid         = 8'h0                          ; 

wire [7:0]      write_acc0_valid        = 8'h0                          ;  

wire [7:0]      write_acc1_valid        = 8'h0                          ;  

wire [7:0]      write_acc2_valid        = 8'h0                          ;  

wire [7:0]      write_acc3_valid        = 8'h0                          ;  

wire [7:0]      write_acc4_valid        = 8'h0                          ;  

wire [7:0]      write_acc5_valid        = 8'h0                          ;  

wire [7:0]      write_acc6_valid        = 8'h0                          ;  

wire [7:0]      write_rc0_valid         = 8'h0                          ; 

wire [7:0]      write_rc1_valid         = 8'h0                          ; 

wire [7:0]      write_arc0_valid        = 8'h0                          ;  

wire [7:0]      write_arc1_valid        = 8'h0                          ;  

wire [7:0]      write_arc2_valid        = 8'h0                          ;  

wire [7:0]      write_arc3_valid        = 8'h0                          ;  

wire [7:0]      write_arc4_valid        = 8'h0                          ;  

wire [7:0]      write_arc5_valid        = 8'h0                          ;  

wire [7:0]      write_arc6_valid        = 8'h0                          ;  

wire            nvm_cipher_bypass       = r_nvm_cipher_bypass           ;
wire            bus_cipher_en           = r_bus_cipher_en               ; 
wire            ram_cipher_en           = r_ram_cipher_en               ; 

wire [7:0]      axi_dma_wost_max        = 8'h07             ;
wire [7:0]      axi_dma_rost_max        = 8'h07             ; 
wire [7:0]      axi_dma_wburst_max      = 8'h07           ; 
wire [7:0]      axi_dma_rburst_max      = 8'h07           ; 

wire [15:0]     axi_dma_ch0_rtrans_max  = 16'h20       ;
wire [15:0]     axi_dma_ch0_wtrans_max  = 16'h20       ;

wire [15:0]     axi_dma_ch1_rtrans_max  = 16'h20       ;
wire [15:0]     axi_dma_ch1_wtrans_max  = 16'h20       ;

wire [15:0]     axi_dma_ch2_rtrans_max  = 16'h20       ;
wire [15:0]     axi_dma_ch2_wtrans_max  = 16'h20       ;

wire [15:0]     axi_dma_ch3_rtrans_max  = 16'h20       ; 
wire [15:0]     axi_dma_ch3_wtrans_max  = 16'h20       ; 

wire [31:0]     life_cycle              = i_life_cycle                  ;

wire [31:0]     hw_control0             = i_con_field0                  ;

wire [31:0]     hw_control1             = i_con_field1                  ;

wire [31:0]     fw_control0             = i_con_field2                  ;

wire [31:0]     fw_control1             = i_con_field3                  ;

wire [31:0]     fw_control2             = i_con_field4                  ;

wire [31:0]     fw_control3             = i_con_field5                  ;

wire clc_soc_dbg_en    = i_lc_undef | i_lc_destroy ;
wire set_soc_dbg_en    = i_lc_test | ((i_lc_dev | i_lc_dev2 | i_lc_manu) & i_hw_boot_done_pulse) ;

wire clc_hsm_dbg_en    = i_lc_undef | i_lc_destroy ;
wire set_hsm_dbg_en    = i_lc_test | ((i_lc_dev | i_lc_dev2) & i_hw_boot_done_pulse) ;

wire            hsm_fw_sta0_nxt              = write_sta0_valid_trig ? i_hwdata[31] : hsm_fw_sta0       ;
wire            soc_verify_err_nxt           = write_sta0_valid_trig ? i_hwdata[23] : soc_verify_err    ;
wire            soc_verify_done_nxt          = write_sta0_valid_trig ? i_hwdata[22] : soc_verify_done   ;
wire            firmware_err_nxt             = write_sta0_valid_trig ? i_hwdata[21] : firmware_err      ;
wire            firmware_done_nxt            = write_sta0_valid_trig ? i_hwdata[20] : firmware_done     ;
wire            bootloader_err_nxt           = write_sta0_valid_trig ? i_hwdata[19] : bootloader_err    ;
wire            bootloader_done_nxt          = write_sta0_valid_trig ? i_hwdata[18] : bootloader_done   ;

wire [3:0]      hsm_fw_sta1_nxt              = write_sta1_valid_trig ? i_hwdata[24:21] : hsm_fw_sta1 ;

wire [10:0]     hsm_fw_sta2_nxt              = write_sta2_valid_trig ? i_hwdata[31:21] : hsm_fw_sta2 ;
wire            fw_upgrade_working_nxt       = write_sta2_valid_trig ? i_hwdata[20] : fw_upgrade_working;
wire            fw_backup_error_nxt          = write_sta2_valid_trig ? i_hwdata[19] : fw_backup_error   ;
wire            fw_backup_done_nxt           = write_sta2_valid_trig ? i_hwdata[18] : fw_backup_done    ;
wire            fw_upgrade_error_nxt         = write_sta2_valid_trig ? i_hwdata[17] : fw_upgrade_error  ;
wire            fw_upgrade_done_nxt          = write_sta2_valid_trig ? i_hwdata[16] : fw_upgrade_done   ;

wire [15:0]     hsm_fw_sta3_nxt              = write_sta3_valid_trig ? i_hwdata[31:16] : hsm_fw_sta3 ;

wire [31:0]     wr_hsm_dbg_en_nxt            = clc_hsm_dbg_en       ? 32'h0             :
                                               set_hsm_dbg_en       ? 32'h265C1A93      :
                                               wr_sys_wr_hsm_dbg_en ? i_hwdata[31:0]    : wr_hsm_dbg_en ;

wire [31:0]     wr_soc_dbg_en_nxt            = clc_soc_dbg_en       ? 32'h0             :
                                               set_soc_dbg_en       ? 32'h6F3C0A95      :
                                               wr_sys_wr_soc_dbg_en ? i_hwdata[31:0]    : wr_soc_dbg_en ;

wire [31:0]     wr_soc_dbg_en_128b0_nxt      = clc_soc_dbg_en             ? 32'h0                 :
                                               set_soc_dbg_en             ? `OSR_SOC_DBG_EN_128B0 :
                                               wr_sys_wr_soc_dbg_en_128B0 ? i_hwdata[31:0]        : wr_soc_dbg_en_128b0 ;

wire [31:0]     wr_soc_dbg_en_128b1_nxt      = clc_soc_dbg_en             ? 32'h0                 :
                                               set_soc_dbg_en             ? `OSR_SOC_DBG_EN_128B1 :
                                               wr_sys_wr_soc_dbg_en_128B1 ? i_hwdata[31:0]        : wr_soc_dbg_en_128b1 ;

wire [31:0]     wr_soc_dbg_en_128b2_nxt      = clc_soc_dbg_en             ? 32'h0                 :
                                               set_soc_dbg_en             ? `OSR_SOC_DBG_EN_128B2 :
                                               wr_sys_wr_soc_dbg_en_128B2 ? i_hwdata[31:0]        : wr_soc_dbg_en_128b2 ;

wire [31:0]     wr_soc_dbg_en_128b3_nxt      = clc_soc_dbg_en             ? 32'h0                 :
                                               set_soc_dbg_en             ? `OSR_SOC_DBG_EN_128B3 :
                                               wr_sys_wr_soc_dbg_en_128B3 ? i_hwdata[31:0]        : wr_soc_dbg_en_128b3 ;

wire [31:0]     wr_soc_cpu_release_nxt       = (wr_sys_wr_soc_cpu_release & (i_hwdata[31:0] == 32'h59AC65A3)) ? 32'h59AC65A3 :  
                                               (wr_sys_wr_soc_cpu_release & (i_hwdata[31:0] == 32'h7F99A765)) ? 32'h7F99A765 : wr_soc_cpu_release ;

wire [31:0]     wr_soc_cpu_reset_nxt         = (wr_sys_wr_soc_cpu_reset & (i_hwdata[31:0] == 32'hA93C65F4)) ? 32'hA93C65F4 :  
                                               (wr_sys_wr_soc_cpu_reset & (i_hwdata[31:0] == 32'hB0DC2ECA)) ? 32'hB0DC2ECA : wr_soc_cpu_reset ;

wire [31:0]     wr_soc_reset_nxt             = (wr_sys_wr_soc_reset     & (i_hwdata[31:0] == 32'h5C28639D)) ? 32'h5C28639D : 
                                               (wr_sys_wr_soc_reset     & (i_hwdata[31:0] == 32'hAE1F05B7)) ? 32'hAE1F05B7 : wr_soc_reset     ;

wire [31:0]     sys_soc_sta_int_nxt          = i_soc_status | ( wr_sys_soc_sta_int ? (~i_hwdata[31:0] & sys_soc_sta_int[31:0]) : sys_soc_sta_int[31:0] );

wire            axi_dma_clk_en_nxt           = write_cc0_valid_trig ? i_hwdata[26] : axi_dma_clk_en    ;
wire            ahb_dma_clk_en_nxt           = write_cc0_valid_trig ? i_hwdata[24] : ahb_dma_clk_en    ;
wire            debug_mailbox_clk_en_nxt     = write_cc0_valid_trig ? i_hwdata[23] : debug_mailbox_clk_en ;
wire            mailbox_clk_en_nxt           = write_cc0_valid_trig ? i_hwdata[20] : mailbox_clk_en    ;
wire            kmu_clk_en_nxt               = write_cc0_valid_trig ? i_hwdata[16] : kmu_clk_en        ;
wire            irom_clk_en_nxt              = write_cc0_valid_trig ? i_hwdata[15] : irom_clk_en       ;
wire            emu_clk_en_nxt               = write_cc0_valid_trig ? i_hwdata[12] : emu_clk_en        ;
wire            boot_clk_en_nxt              = write_cc0_valid_trig ? i_hwdata[8]  : boot_clk_en       ;

wire            mono_cnt_clk_en_nxt          = write_cc1_valid_trig ? i_hwdata[21] : mono_cnt_clk_en   ;
wire            utc_tim_clk_en_nxt           = write_cc1_valid_trig ? i_hwdata[20] : utc_tim_clk_en    ;
wire            tim_clk_en_nxt               = write_cc1_valid_trig ? i_hwdata[16] : tim_clk_en        ;
wire            apb_nvm_cipher_clk_en_nxt    = write_cc1_valid_trig ? i_hwdata[14] : apb_nvm_cipher_clk_en;
wire            crc_clk_en_nxt               = write_cc1_valid_trig ? i_hwdata[13] : crc_clk_en        ;
wire            uart_clk_en_nxt              = write_cc1_valid_trig ? i_hwdata[12] : uart_clk_en       ;
wire            wdt1_clk_en_nxt              = write_cc1_valid_trig ? i_hwdata[9]  : wdt1_clk_en       ;
wire            wdt_clk_en_nxt               = write_cc1_valid_trig ? i_hwdata[8]  : wdt_clk_en        ;

wire            hash0_clk_dma_en_nxt         = write_acc0_valid_trig ? i_hwdata[9]  : hash0_clk_dma_en ;
wire            hash0_clk_en_nxt             = write_acc0_valid_trig ? i_hwdata[8]  : hash0_clk_en     ;

wire            ske0_clk_dma_en_nxt          = write_acc1_valid_trig ? i_hwdata[9]  : ske0_clk_dma_en   ;
wire            ske0_clk_en_nxt              = write_acc1_valid_trig ? i_hwdata[8]  : ske0_clk_en       ;

wire            pke_clk_en_nxt               = write_acc2_valid_trig ? i_hwdata[8] : pke_clk_en        ;

wire            trng_clk_en_nxt              = write_acc3_valid_trig ? i_hwdata[8] : trng_clk_en       ;

wire            chacha_clk_dma_en_nxt        = write_acc4_valid_trig ? i_hwdata[9] : chacha_clk_dma_en ;
wire            chacha_clk_en_nxt            = write_acc4_valid_trig ? i_hwdata[8] : chacha_clk_en     ;

wire            pqc_spxp_clk_dma_en_nxt      = write_acc5_valid_trig ? i_hwdata[9] : pqc_spxp_clk_dma_en ;
wire            pqc_spxp_clk_en_nxt          = write_acc5_valid_trig ? i_hwdata[8] : pqc_spxp_clk_en     ;

wire            pqc_kd_clk_en_nxt            = write_acc6_valid_trig ? i_hwdata[8] : pqc_kd_clk_en     ;

wire            hsm_with_boot_rst_nxt        = write_rc0_valid_trig ? i_hwdata[31] : 1'h1              ;
wire            hsm_rst_nxt                  = write_rc0_valid_trig ? i_hwdata[30] : 1'h1              ;
wire            cpu_rst_nxt                  = write_rc0_valid_trig ? i_hwdata[29] : 1'h1              ;
wire            axi_dma_rst_nxt              = write_rc0_valid_trig ? i_hwdata[26] : axi_dma_rst       ;
wire            ahb_dma_rst_nxt              = write_rc0_valid_trig ? i_hwdata[24] : ahb_dma_rst       ;
wire            debug_mailbox_rst_nxt        = write_rc0_valid_trig ? i_hwdata[23] : debug_mailbox_rst ;
wire            mailbox_rst_nxt              = write_rc0_valid_trig ? i_hwdata[20] : mailbox_rst       ;
wire            kmu_rst_nxt                  = write_rc0_valid_trig ? i_hwdata[16] : kmu_rst           ;
wire            irom_rst_nxt                 = write_rc0_valid_trig ? i_hwdata[15] : irom_rst          ;
wire            emu_rst_nxt                  = write_rc0_valid_trig ? i_hwdata[12] : emu_rst           ;
wire            sys_rst_nxt                  = write_rc0_valid_trig ? i_hwdata[9]  : 1'h1              ;
wire            boot_rst_nxt                 = write_rc0_valid_trig ? i_hwdata[8]  : 1'h1              ;

wire            mono_cnt_rst_nxt             = write_rc1_valid_trig ? i_hwdata[21] : mono_cnt_rst      ;
wire            utc_tim_rst_nxt              = write_rc1_valid_trig ? i_hwdata[20] : utc_tim_rst       ;
wire            tim_rst_nxt                  = write_rc1_valid_trig ? i_hwdata[16] : tim_rst           ;
wire            apb_nvm_cipher_rst_nxt       = write_rc1_valid_trig ? i_hwdata[14] : apb_nvm_cipher_rst;
wire            crc_rst_nxt                  = write_rc1_valid_trig ? i_hwdata[13] : crc_rst           ;
wire            uart_rst_nxt                 = write_rc1_valid_trig ? i_hwdata[12] : uart_rst          ;
wire            wdt1_rst_nxt                 = write_rc1_valid_trig ? i_hwdata[9]  : wdt1_rst          ;
wire            wdt_rst_nxt                  = write_rc1_valid_trig ? i_hwdata[8]  : wdt_rst           ;

wire            hash0_dma_rst_nxt            = write_arc0_valid_trig ? i_hwdata[9]  : hash0_dma_rst    ;
wire            hash0_rst_nxt                = write_arc0_valid_trig ? i_hwdata[8]  : hash0_rst        ;

wire            ske0_dma_rst_nxt             = write_arc1_valid_trig ? i_hwdata[9]  : ske0_dma_rst     ;
wire            ske0_rst_nxt                 = write_arc1_valid_trig ? i_hwdata[8]  : ske0_rst         ;

wire            pke_rst_nxt                  = write_arc2_valid_trig ? i_hwdata[8] : pke_rst           ;

wire            trng_rst_nxt                 = write_arc3_valid_trig ? i_hwdata[8] : trng_rst          ;

wire            chacha_dma_rst_nxt           = write_arc4_valid_trig ? i_hwdata[9] : chacha_dma_rst    ;
wire            chacha_rst_nxt               = write_arc4_valid_trig ? i_hwdata[8] : chacha_rst        ;

wire            pqc_spxp_dma_rst_nxt         = write_arc5_valid_trig ? i_hwdata[9] : pqc_spxp_dma_rst  ;
wire            pqc_spxp_rst_nxt             = write_arc5_valid_trig ? i_hwdata[8] : pqc_spxp_rst      ;

wire            pqc_kd_rst_nxt               = write_arc6_valid_trig ? i_hwdata[8] : pqc_kd_rst        ;

wire            hsm_fw_sta0_next             = hsm_fw_sta0_nxt              ; 
wire            soc_verify_err_next          = soc_verify_err_nxt           ; 
wire            soc_verify_done_next         = soc_verify_done_nxt          ; 
wire            firmware_err_next            = firmware_err_nxt             ; 
wire            firmware_done_next           = firmware_done_nxt            ; 
wire            bootloader_err_next          = bootloader_err_nxt           ; 
wire            bootloader_done_next         = bootloader_done_nxt          ; 

wire [3:0]      hsm_fw_sta1_next             = hsm_fw_sta1_nxt              ; 

wire [10:0]     hsm_fw_sta2_next             = hsm_fw_sta2_nxt              ; 
wire            fw_upgrade_working_next      = fw_upgrade_working_nxt       ; 
wire            fw_backup_error_next         = fw_backup_error_nxt          ; 
wire            fw_backup_done_next          = fw_backup_done_nxt           ; 
wire            fw_upgrade_error_next        = fw_upgrade_error_nxt         ; 
wire            fw_upgrade_done_next         = fw_upgrade_done_nxt          ; 

wire [15:0]     hsm_fw_sta3_next             = hsm_fw_sta3_nxt              ; 

wire [31:0]     wr_hsm_dbg_en_next           = wr_hsm_dbg_en_nxt            ; 

wire [31:0]     wr_soc_dbg_en_next           = wr_soc_dbg_en_nxt            ; 

wire [31:0]     wr_soc_dbg_en_128b0_next     = i_scan_mode ? `OSR_SOC_DBG_EN_128B0 : wr_soc_dbg_en_128b0_nxt      ; 

wire [31:0]     wr_soc_dbg_en_128b1_next     = i_scan_mode ? `OSR_SOC_DBG_EN_128B1 : wr_soc_dbg_en_128b1_nxt      ; 

wire [31:0]     wr_soc_dbg_en_128b2_next     = i_scan_mode ? `OSR_SOC_DBG_EN_128B2 : wr_soc_dbg_en_128b2_nxt      ; 

wire [31:0]     wr_soc_dbg_en_128b3_next     = i_scan_mode ? `OSR_SOC_DBG_EN_128B3 : wr_soc_dbg_en_128b3_nxt      ; 

wire [31:0]     wr_soc_cpu_release_next      = wr_soc_cpu_release_nxt       ; 

wire [31:0]     wr_soc_cpu_reset_next        = wr_soc_cpu_reset_nxt         ; 

wire [31:0]     wr_soc_reset_next            = wr_soc_reset_nxt             ; 

wire [31:0]     sys_soc_sta_int_next         = sys_soc_sta_int_nxt          ; 

wire [31:0]     sys_soc_sta_int_en_next      = wr_sys_soc_sta_int_en          ?  i_hwdata[31:0]     :  sys_soc_sta_int_en           ; 

wire            axi_dma_clk_en_next          = axi_dma_clk_en_nxt           ; 
wire            ahb_dma_clk_en_next          = ahb_dma_clk_en_nxt           ; 
wire            debug_mailbox_clk_en_next    = debug_mailbox_clk_en_nxt     ; 
wire            mailbox_clk_en_next          = mailbox_clk_en_nxt           ; 
wire            kmu_clk_en_next              = kmu_clk_en_nxt               ; 
wire            irom_clk_en_next             = irom_clk_en_nxt              ; 
wire            emu_clk_en_next              = emu_clk_en_nxt               ; 
wire            boot_clk_en_next             = boot_clk_en_nxt              ; 

wire            mono_cnt_clk_en_next         = mono_cnt_clk_en_nxt          ; 
wire            utc_tim_clk_en_next          = utc_tim_clk_en_nxt           ; 
wire            tim_clk_en_next              = tim_clk_en_nxt               ; 
wire            apb_nvm_cipher_clk_en_next   = apb_nvm_cipher_clk_en_nxt    ; 
wire            crc_clk_en_next              = crc_clk_en_nxt               ; 
wire            uart_clk_en_next             = uart_clk_en_nxt              ; 
wire            wdt1_clk_en_next             = wdt1_clk_en_nxt              ; 
wire            wdt_clk_en_next              = wdt_clk_en_nxt               ; 

wire            hash0_clk_dma_en_next        = hash0_clk_dma_en_nxt         ; 
wire            hash0_clk_en_next            = hash0_clk_en_nxt             ; 

wire            ske0_clk_dma_en_next         = ske0_clk_dma_en_nxt          ; 
wire            ske0_clk_en_next             = ske0_clk_en_nxt              ; 

wire            pke_clk_en_next              = pke_clk_en_nxt               ; 

wire            trng_clk_en_next             = trng_clk_en_nxt              ; 

wire            chacha_clk_dma_en_next       = chacha_clk_dma_en_nxt        ; 
wire            chacha_clk_en_next           = chacha_clk_en_nxt            ; 

wire            pqc_spxp_clk_dma_en_next     = pqc_spxp_clk_dma_en_nxt      ; 
wire            pqc_spxp_clk_en_next         = pqc_spxp_clk_en_nxt          ; 

wire            pqc_kd_clk_en_next           = pqc_kd_clk_en_nxt            ; 

wire            hsm_with_boot_rst_next       = hsm_with_boot_rst_nxt        ; 
wire            hsm_rst_next                 = hsm_rst_nxt                  ; 
wire            cpu_rst_next                 = cpu_rst_nxt                  ; 
wire            axi_dma_rst_next             = axi_dma_rst_nxt              ; 
wire            ahb_dma_rst_next             = ahb_dma_rst_nxt              ; 
wire            debug_mailbox_rst_next       = debug_mailbox_rst_nxt        ; 
wire            mailbox_rst_next             = mailbox_rst_nxt              ; 
wire            kmu_rst_next                 = kmu_rst_nxt                  ; 
wire            irom_rst_next                = irom_rst_nxt                 ; 
wire            emu_rst_next                 = emu_rst_nxt                  ; 
wire            sys_rst_next                 = sys_rst_nxt                  ; 
wire            boot_rst_next                = boot_rst_nxt                 ; 

wire            mono_cnt_rst_next            = mono_cnt_rst_nxt             ; 
wire            utc_tim_rst_next             = utc_tim_rst_nxt              ; 
wire            tim_rst_next                 = tim_rst_nxt                  ; 
wire            apb_nvm_cipher_rst_next      = apb_nvm_cipher_rst_nxt       ; 
wire            crc_rst_next                 = crc_rst_nxt                  ; 
wire            uart_rst_next                = uart_rst_nxt                 ; 
wire            wdt1_rst_next                = wdt1_rst_nxt                 ; 
wire            wdt_rst_next                 = wdt_rst_nxt                  ; 

wire            hash0_dma_rst_next           = hash0_dma_rst_nxt            ; 
wire            hash0_rst_next               = hash0_rst_nxt                ; 

wire            ske0_dma_rst_next            = ske0_dma_rst_nxt             ; 
wire            ske0_rst_next                = ske0_rst_nxt                 ; 

wire            pke_rst_next                 = pke_rst_nxt                  ; 

wire            trng_rst_next                = trng_rst_nxt                 ; 

wire            chacha_dma_rst_next          = chacha_dma_rst_nxt           ; 
wire            chacha_rst_next              = chacha_rst_nxt               ; 

wire            pqc_spxp_dma_rst_next        = pqc_spxp_dma_rst_nxt         ; 
wire            pqc_spxp_rst_next            = pqc_spxp_rst_nxt             ; 

wire            pqc_kd_rst_next              = pqc_kd_rst_nxt               ; 

wire [31:0]     soc_mem_bal_next             = wr_sys_soc_mem_bal              ?  i_hwdata[31:0]     :  soc_mem_bal                  ; 

wire [31:0]     soc_mem_bah_next             = wr_sys_soc_mem_bah              ?  i_hwdata[31:0]     :  soc_mem_bah                  ; 

wire            trng_drbg_alg_next           = wr_sys_trng_drbg_alg            ?  i_hwdata[0]        :  trng_drbg_alg                ; 

wire [25:0]     cpu_stcalib_cfg_next         = wr_sys_cpu_cfg                  ?  i_hwdata[25:0]     :  cpu_stcalib_cfg              ; 

wire            ahb_cfg_rsp_err_en_next      = wr_sys_bus_err_cfg              ?  i_hwdata[10]       :  ahb_cfg_rsp_err_en           ; 
wire            ahb_nvm_rsp_err_en_next      = wr_sys_bus_err_cfg              ?  i_hwdata[6]        :  ahb_nvm_rsp_err_en           ; 
wire            ahb_otp_rsp_err_en_next      = wr_sys_bus_err_cfg              ?  i_hwdata[4]        :  ahb_otp_rsp_err_en           ; 
wire            ahb_soc_rsp_err_en_next      = wr_sys_bus_err_cfg              ?  i_hwdata[0]        :  ahb_soc_rsp_err_en           ; 
wire            ahb_bus_pr_en_next           = wr_sys_bus_pr_cfg               ?  i_hwdata[0]        :  ahb_bus_pr_en                ; 

wire [7:0]      ahb_dma_rd_wr_to_next        = wr_sys_ahb_dma_cfg              ?  i_hwdata[15:8]     :  ahb_dma_rd_wr_to             ; 
wire            otp_ahb_if_sel_next          = wr_sys_ahb_dma_cfg              ?  i_hwdata[3]        :  otp_ahb_if_sel               ; 
wire [1:0]      dma_wr_ctl_next              = wr_sys_ahb_dma_cfg              ?  i_hwdata[2:1]      :  dma_wr_ctl                   ; 
wire            dc_dma_sel_next              = wr_sys_ahb_dma_cfg              ?  i_hwdata[0]        :  dc_dma_sel                   ; 

wire [15:0]     axi_dma_rd_wr_to_next        = wr_sys_axi_dma_cfg1             ?  i_hwdata[15:0]     :  axi_dma_rd_wr_to             ; 

wire [3:0]      axi_dma_ch0_arsnoop_next     = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[31:28]    :  axi_dma_ch0_arsnoop          ; 
wire [1:0]      axi_dma_ch0_arbar_next       = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[27:26]    :  axi_dma_ch0_arbar            ; 
wire [1:0]      axi_dma_ch0_ardomain_next    = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[25:24]    :  axi_dma_ch0_ardomain         ; 
wire [1:0]      axi_dma_ch0_arlock_next      = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[17:16]    :  axi_dma_ch0_arlock           ; 
wire [3:0]      axi_dma_ch0_arcache_next     = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[15:12]    :  axi_dma_ch0_arcache          ; 
wire [2:0]      axi_dma_ch0_arprot_next      = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[10:8]     :  axi_dma_ch0_arprot           ; 
wire [3:0]      axi_dma_ch0_arqos_next       = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[7:4]      :  axi_dma_ch0_arqos            ; 
wire [3:0]      axi_dma_ch0_arregion_next    = wr_sys_axi_dma_ch0_cfg1         ?  i_hwdata[3:0]      :  axi_dma_ch0_arregion         ; 

wire [2:0]      axi_dma_ch0_awsnoop_next     = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[30:28]    :  axi_dma_ch0_awsnoop          ; 
wire [1:0]      axi_dma_ch0_awbar_next       = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[27:26]    :  axi_dma_ch0_awbar            ; 
wire [1:0]      axi_dma_ch0_awdomain_next    = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[25:24]    :  axi_dma_ch0_awdomain         ; 
wire [1:0]      axi_dma_ch0_awlock_next      = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[17:16]    :  axi_dma_ch0_awlock           ; 
wire [3:0]      axi_dma_ch0_awcache_next     = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[15:12]    :  axi_dma_ch0_awcache          ; 
wire [2:0]      axi_dma_ch0_awprot_next      = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[10:8]     :  axi_dma_ch0_awprot           ; 
wire [3:0]      axi_dma_ch0_awqos_next       = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[7:4]      :  axi_dma_ch0_awqos            ; 
wire [3:0]      axi_dma_ch0_awregion_next    = wr_sys_axi_dma_ch0_cfg2         ?  i_hwdata[3:0]      :  axi_dma_ch0_awregion         ; 

wire [3:0]      axi_dma_ch1_arsnoop_next     = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[31:28]    :  axi_dma_ch1_arsnoop          ; 
wire [1:0]      axi_dma_ch1_arbar_next       = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[27:26]    :  axi_dma_ch1_arbar            ; 
wire [1:0]      axi_dma_ch1_ardomain_next    = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[25:24]    :  axi_dma_ch1_ardomain         ; 
wire [1:0]      axi_dma_ch1_arlock_next      = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[17:16]    :  axi_dma_ch1_arlock           ; 
wire [3:0]      axi_dma_ch1_arcache_next     = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[15:12]    :  axi_dma_ch1_arcache          ; 
wire [2:0]      axi_dma_ch1_arprot_next      = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[10:8]     :  axi_dma_ch1_arprot           ; 
wire [3:0]      axi_dma_ch1_arqos_next       = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[7:4]      :  axi_dma_ch1_arqos            ; 
wire [3:0]      axi_dma_ch1_arregion_next    = wr_sys_axi_dma_ch1_cfg1         ?  i_hwdata[3:0]      :  axi_dma_ch1_arregion         ; 

wire [2:0]      axi_dma_ch1_awsnoop_next     = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[30:28]    :  axi_dma_ch1_awsnoop          ; 
wire [1:0]      axi_dma_ch1_awbar_next       = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[27:26]    :  axi_dma_ch1_awbar            ; 
wire [1:0]      axi_dma_ch1_awdomain_next    = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[25:24]    :  axi_dma_ch1_awdomain         ; 
wire [1:0]      axi_dma_ch1_awlock_next      = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[17:16]    :  axi_dma_ch1_awlock           ; 
wire [3:0]      axi_dma_ch1_awcache_next     = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[15:12]    :  axi_dma_ch1_awcache          ; 
wire [2:0]      axi_dma_ch1_awprot_next      = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[10:8]     :  axi_dma_ch1_awprot           ; 
wire [3:0]      axi_dma_ch1_awqos_next       = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[7:4]      :  axi_dma_ch1_awqos            ; 
wire [3:0]      axi_dma_ch1_awregion_next    = wr_sys_axi_dma_ch1_cfg2         ?  i_hwdata[3:0]      :  axi_dma_ch1_awregion         ; 

wire [3:0]      axi_dma_ch2_arsnoop_next     = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[31:28]    :  axi_dma_ch2_arsnoop          ; 
wire [1:0]      axi_dma_ch2_arbar_next       = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[27:26]    :  axi_dma_ch2_arbar            ; 
wire [1:0]      axi_dma_ch2_ardomain_next    = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[25:24]    :  axi_dma_ch2_ardomain         ; 
wire [1:0]      axi_dma_ch2_arlock_next      = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[17:16]    :  axi_dma_ch2_arlock           ; 
wire [3:0]      axi_dma_ch2_arcache_next     = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[15:12]    :  axi_dma_ch2_arcache          ; 
wire [2:0]      axi_dma_ch2_arprot_next      = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[10:8]     :  axi_dma_ch2_arprot           ; 
wire [3:0]      axi_dma_ch2_arqos_next       = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[7:4]      :  axi_dma_ch2_arqos            ; 
wire [3:0]      axi_dma_ch2_arregion_next    = wr_sys_axi_dma_ch2_cfg1         ?  i_hwdata[3:0]      :  axi_dma_ch2_arregion         ; 

wire [2:0]      axi_dma_ch2_awsnoop_next     = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[30:28]    :  axi_dma_ch2_awsnoop          ; 
wire [1:0]      axi_dma_ch2_awbar_next       = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[27:26]    :  axi_dma_ch2_awbar            ; 
wire [1:0]      axi_dma_ch2_awdomain_next    = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[25:24]    :  axi_dma_ch2_awdomain         ; 
wire [1:0]      axi_dma_ch2_awlock_next      = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[17:16]    :  axi_dma_ch2_awlock           ; 
wire [3:0]      axi_dma_ch2_awcache_next     = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[15:12]    :  axi_dma_ch2_awcache          ; 
wire [2:0]      axi_dma_ch2_awprot_next      = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[10:8]     :  axi_dma_ch2_awprot           ; 
wire [3:0]      axi_dma_ch2_awqos_next       = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[7:4]      :  axi_dma_ch2_awqos            ; 
wire [3:0]      axi_dma_ch2_awregion_next    = wr_sys_axi_dma_ch2_cfg2         ?  i_hwdata[3:0]      :  axi_dma_ch2_awregion         ; 

wire [3:0]      axi_dma_ch3_arsnoop_next     = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[31:28]    :  axi_dma_ch3_arsnoop          ; 
wire [1:0]      axi_dma_ch3_arbar_next       = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[27:26]    :  axi_dma_ch3_arbar            ; 
wire [1:0]      axi_dma_ch3_ardomain_next    = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[25:24]    :  axi_dma_ch3_ardomain         ; 
wire [1:0]      axi_dma_ch3_arlock_next      = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[17:16]    :  axi_dma_ch3_arlock           ; 
wire [3:0]      axi_dma_ch3_arcache_next     = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[15:12]    :  axi_dma_ch3_arcache          ; 
wire [2:0]      axi_dma_ch3_arprot_next      = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[10:8]     :  axi_dma_ch3_arprot           ; 
wire [3:0]      axi_dma_ch3_arqos_next       = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[7:4]      :  axi_dma_ch3_arqos            ; 
wire [3:0]      axi_dma_ch3_arregion_next    = wr_sys_axi_dma_ch3_cfg1         ?  i_hwdata[3:0]      :  axi_dma_ch3_arregion         ; 

wire [2:0]      axi_dma_ch3_awsnoop_next     = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[30:28]    :  axi_dma_ch3_awsnoop          ; 
wire [1:0]      axi_dma_ch3_awbar_next       = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[27:26]    :  axi_dma_ch3_awbar            ; 
wire [1:0]      axi_dma_ch3_awdomain_next    = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[25:24]    :  axi_dma_ch3_awdomain         ; 
wire [1:0]      axi_dma_ch3_awlock_next      = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[17:16]    :  axi_dma_ch3_awlock           ; 
wire [3:0]      axi_dma_ch3_awcache_next     = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[15:12]    :  axi_dma_ch3_awcache          ; 
wire [2:0]      axi_dma_ch3_awprot_next      = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[10:8]     :  axi_dma_ch3_awprot           ; 
wire [3:0]      axi_dma_ch3_awqos_next       = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[7:4]      :  axi_dma_ch3_awqos            ; 
wire [3:0]      axi_dma_ch3_awregion_next    = wr_sys_axi_dma_ch3_cfg2         ?  i_hwdata[3:0]      :  axi_dma_ch3_awregion         ; 

wire [7:0]      cpu_irom_ecc_ckbits_next     = wr_sys_cpu_ecc_ctrl0            ?  i_hwdata[31:24]    :  cpu_irom_ecc_ckbits          ; 
wire [7:0]      cpu_irom_ecc_tm_en_next      = wr_sys_cpu_ecc_ctrl0            ?  i_hwdata[23:16]    :  cpu_irom_ecc_tm_en           ; 
wire [7:0]      cpu_irom_ecc_err_rsp_en_next = wr_sys_cpu_ecc_ctrl0            ?  i_hwdata[15:8]     :  cpu_irom_ecc_err_rsp_en      ; 
wire [7:0]      cpu_irom_ecc_ck_en_next      = wr_sys_cpu_ecc_ctrl0            ?  i_hwdata[7:0]      :  cpu_irom_ecc_ck_en           ; 

wire [7:0]      cpu_iram_ecc_ckbits_next     = wr_sys_cpu_ecc_ctrl1            ?  i_hwdata[31:24]    :  cpu_iram_ecc_ckbits          ; 
wire [7:0]      cpu_iram_ecc_tm_en_next      = wr_sys_cpu_ecc_ctrl1            ?  i_hwdata[23:16]    :  cpu_iram_ecc_tm_en           ; 
wire [7:0]      cpu_iram_ecc_err_rsp_en_next = wr_sys_cpu_ecc_ctrl1            ?  i_hwdata[15:8]     :  cpu_iram_ecc_err_rsp_en      ; 
wire [7:0]      cpu_iram_ecc_ck_en_next      = wr_sys_cpu_ecc_ctrl1            ?  i_hwdata[7:0]      :  cpu_iram_ecc_ck_en           ; 

wire [7:0]      cpu_dram_ecc_ckbits_next     = wr_sys_cpu_ecc_ctrl2            ?  i_hwdata[31:24]    :  cpu_dram_ecc_ckbits          ; 
wire [7:0]      cpu_dram_ecc_tm_en_next      = wr_sys_cpu_ecc_ctrl2            ?  i_hwdata[23:16]    :  cpu_dram_ecc_tm_en           ; 
wire [7:0]      cpu_dram_ecc_err_rsp_en_next = wr_sys_cpu_ecc_ctrl2            ?  i_hwdata[15:8]     :  cpu_dram_ecc_err_rsp_en      ; 
wire [7:0]      cpu_dram_ecc_ck_en_next      = wr_sys_cpu_ecc_ctrl2            ?  i_hwdata[7:0]      :  cpu_dram_ecc_ck_en           ; 

wire [9:0]      kmu_ram_ecc_ckbits_next      = wr_sys_kmu_ecc_ctrl             ?  i_hwdata[25:16]    :  kmu_ram_ecc_ckbits           ; 
wire [7:0]      kmu_ram_ecc_tm_en_next       = wr_sys_kmu_ecc_ctrl             ?  i_hwdata[15:8]     :  kmu_ram_ecc_tm_en            ; 
wire [7:0]      kmu_ram_ecc_ck_en_next       = wr_sys_kmu_ecc_ctrl             ?  i_hwdata[7:0]      :  kmu_ram_ecc_ck_en            ; 

wire [9:0]      pke_ram0_ecc_ckbits_next     = wr_sys_pke_ecc_ctrl0            ?  i_hwdata[25:16]    :  pke_ram0_ecc_ckbits          ; 
wire [7:0]      pke_ram0_ecc_tm_en_next      = wr_sys_pke_ecc_ctrl0            ?  i_hwdata[15:8]     :  pke_ram0_ecc_tm_en           ; 
wire [7:0]      pke_ram0_ecc_ck_en_next      = wr_sys_pke_ecc_ctrl0            ?  i_hwdata[7:0]      :  pke_ram0_ecc_ck_en           ; 

wire [9:0]      pke_ram1_ecc_ckbits_next     = wr_sys_pke_ecc_ctrl1            ?  i_hwdata[25:16]    :  pke_ram1_ecc_ckbits          ; 
wire [7:0]      pke_ram1_ecc_tm_en_next      = wr_sys_pke_ecc_ctrl1            ?  i_hwdata[15:8]     :  pke_ram1_ecc_tm_en           ; 
wire [7:0]      pke_ram1_ecc_ck_en_next      = wr_sys_pke_ecc_ctrl1            ?  i_hwdata[7:0]      :  pke_ram1_ecc_ck_en           ; 

wire [9:0]      pke_ram2_ecc_ckbits_next     = wr_sys_pke_ecc_ctrl2            ?  i_hwdata[25:16]    :  pke_ram2_ecc_ckbits          ; 
wire [7:0]      pke_ram2_ecc_tm_en_next      = wr_sys_pke_ecc_ctrl2            ?  i_hwdata[15:8]     :  pke_ram2_ecc_tm_en           ; 
wire [7:0]      pke_ram2_ecc_ck_en_next      = wr_sys_pke_ecc_ctrl2            ?  i_hwdata[7:0]      :  pke_ram2_ecc_ck_en           ; 

wire [9:0]      pke_ram3_ecc_ckbits_next     = wr_sys_pke_ecc_ctrl3            ?  i_hwdata[25:16]    :  pke_ram3_ecc_ckbits          ; 
wire [7:0]      pke_ram3_ecc_tm_en_next      = wr_sys_pke_ecc_ctrl3            ?  i_hwdata[15:8]     :  pke_ram3_ecc_tm_en           ; 
wire [7:0]      pke_ram3_ecc_ck_en_next      = wr_sys_pke_ecc_ctrl3            ?  i_hwdata[7:0]      :  pke_ram3_ecc_ck_en           ; 

wire [31:0]     gen_reg0_next                = wr_sys_gen_reg0                 ?  i_hwdata[31:0]     :  gen_reg0                     ; 

wire [31:0]     gen_reg1_next                = wr_sys_gen_reg1                 ?  i_hwdata[31:0]     :  gen_reg1                     ; 

wire [31:0]     gen_reg2_next                = wr_sys_gen_reg2                 ?  i_hwdata[31:0]     :  gen_reg2                     ; 

wire [31:0]     gen_reg3_next                = wr_sys_gen_reg3                 ?  i_hwdata[31:0]     :  gen_reg3                     ; 

wire [31:0]     gen_reg4_next                = wr_sys_gen_reg4                 ?  i_hwdata[31:0]     :  gen_reg4                     ; 

wire [31:0]     gen_reg5_next                = wr_sys_gen_reg5                 ?  i_hwdata[31:0]     :  gen_reg5                     ; 

wire [31:0]     gen_reg6_next                = wr_sys_gen_reg6                 ?  i_hwdata[31:0]     :  gen_reg6                     ; 

wire [31:0]     gen_reg7_next                = wr_sys_gen_reg7                 ?  i_hwdata[31:0]     :  gen_reg7                     ; 

wire [31:0]     gen_reg8_next                = wr_sys_gen_reg8                 ?  i_hwdata[31:0]     :  gen_reg8                     ; 

wire [31:0]     gen_reg9_next                = wr_sys_gen_reg9                 ?  i_hwdata[31:0]     :  gen_reg9                     ; 

wire [31:0]     gen_reg10_next               = wr_sys_gen_reg10                ?  i_hwdata[31:0]     :  gen_reg10                    ; 

wire [31:0]     gen_reg11_next               = wr_sys_gen_reg11                ?  i_hwdata[31:0]     :  gen_reg11                    ; 

wire [31:0]     gen_reg12_next               = wr_sys_gen_reg12                ?  i_hwdata[31:0]     :  gen_reg12                    ; 

wire [31:0]     gen_reg13_next               = wr_sys_gen_reg13                ?  i_hwdata[31:0]     :  gen_reg13                    ; 

wire [31:0]     gen_reg14_next               = wr_sys_gen_reg14                ?  i_hwdata[31:0]     :  gen_reg14                    ; 

wire [31:0]     gen_reg15_next               = wr_sys_gen_reg15                ?  i_hwdata[31:0]     :  gen_reg15                    ; 

wire [31:0]     sys_run_step_next            = wr_sys_run_step                 ?  i_hwdata[31:0]     :  sys_run_step                 ; 

wire            r_soc_cpu_release_next    = (wr_soc_cpu_release_nxt == 32'h59AC65A3) ;
wire            r_soc_cpu_reset_next      = (wr_soc_cpu_reset_nxt   == 32'hA93C65F4) ;
wire            r_soc_reset_next          = (wr_soc_reset_nxt       == 32'h5C28639D) ;
wire            r_soc_dbg_en_next         = (wr_soc_dbg_en_nxt      == 32'h6F3C0A95) ;
wire            r_hsm_dbg_en_next         = (wr_hsm_dbg_en_nxt      == 32'h265C1A93) ;

wire [31:0] pack_sys_hw_inf0                   = {otp_polarity,define_fpga,4'h0,cpu_dram_width[4:0],cpu_irom_width[4:0],cpu_iram_width[4:0],cpu_dram_byte,cpu_iram_byte,cpu_irom_byte,6'h0,cpu_tmr,cpu_dbg};
wire [31:0] pack_sys_hw_inf1                   = {cpu_reset_vector[31:0]};
wire [31:0] pack_sys_hw_inf2                   = {15'h0,kmu_ram_cipher,13'h0,cpu_dram_cipher,cpu_iram_cipher,cpu_irom_cipher};
wire [31:0] pack_sys_hw_inf3                   = {15'h0,kmu_ram_ecc,13'h0,cpu_dram_ecc,cpu_iram_ecc,cpu_irom_ecc};
wire [31:0] pack_sys_hw_inf4                   = {30'h0,pke_ecc_en,pke_cdc_en};
wire [31:0] pack_sys_hw_inf5                   = {24'h0,hw_rd_otp_end[7:0]};
wire [31:0] pack_sys_ver0                      = {4'h0,con_yq[11:0],12'h0,qnum[3:0]};
wire [31:0] pack_sys_ver1                      = {client_code[15:0],git_tag[15:0]};
wire [31:0] pack_sys_hsm_sta0                  = {hsm_fw_sta0,hsm_lc_undef,hsm_lc_destroy,hsm_lc_debug,hsm_lc_user,hsm_lc_manu,hsm_lc_dev0_or_dev,hsm_lc_test,soc_verify_err,soc_verify_done,firmware_err,firmware_done,bootloader_err,bootloader_done,hw_boot_err,hw_boot_done,8'h0,write_sta0_valid[7:0]};
wire [31:0] pack_sys_hsm_sta1                  = {hsm_lc_dev,3'h0,cpu_hart_halted,1'h0,cpu_wfi,hsm_fw_sta1[3:0],soc_cpu_release,soc_cpu_reset,soc_reset,soc_dbg_en,hsm_dbg_en,8'h0,write_sta1_valid[7:0]};
wire [31:0] pack_sys_hsm_sta2                  = {hsm_fw_sta2[10:0],fw_upgrade_working,fw_backup_error,fw_backup_done,fw_upgrade_error,fw_upgrade_done,8'h0,write_sta2_valid[7:0]};
wire [31:0] pack_sys_hsm_sta3                  = {hsm_fw_sta3[15:0],8'h0,write_sta3_valid[7:0]};
wire [31:0] pack_sys_wr_hsm_dbg_en             = {wr_hsm_dbg_en[31:0]};
wire [31:0] pack_sys_wr_soc_dbg_en             = {wr_soc_dbg_en[31:0]};
wire [31:0] pack_sys_wr_soc_dbg_en_128b0       = {wr_soc_dbg_en_128b0[31:0]};
wire [31:0] pack_sys_wr_soc_dbg_en_128b1       = {wr_soc_dbg_en_128b1[31:0]};
wire [31:0] pack_sys_wr_soc_dbg_en_128b2       = {wr_soc_dbg_en_128b2[31:0]};
wire [31:0] pack_sys_wr_soc_dbg_en_128b3       = {wr_soc_dbg_en_128b3[31:0]};
wire [31:0] pack_sys_wr_soc_cpu_release        = {wr_soc_cpu_release[31:0]};
wire [31:0] pack_sys_wr_soc_cpu_reset          = {wr_soc_cpu_reset[31:0]};
wire [31:0] pack_sys_wr_soc_reset              = {wr_soc_reset[31:0]};
wire [31:0] pack_sys_soc_sta                   = {sys_soc_sta[31:0]};
wire [31:0] pack_sys_soc_sta_int               = {sys_soc_sta_int[31:0]};
wire [31:0] pack_sys_soc_sta_int_en            = {sys_soc_sta_int_en[31:0]};
wire [31:0] pack_sys_clk_ctrl0                 = {5'h0,axi_dma_clk_en,1'h0,ahb_dma_clk_en,debug_mailbox_clk_en,2'h0,mailbox_clk_en,3'h0,kmu_clk_en,irom_clk_en,2'h0,emu_clk_en,3'h0,boot_clk_en,write_cc0_valid[7:0]};
wire [31:0] pack_sys_clk_ctrl1                 = {10'h0,mono_cnt_clk_en,utc_tim_clk_en,3'h0,tim_clk_en,1'h0,apb_nvm_cipher_clk_en,crc_clk_en,uart_clk_en,2'h0,wdt1_clk_en,wdt_clk_en,write_cc1_valid[7:0]};
wire [31:0] pack_sys_alg_clk_ctrl0             = {22'h0,hash0_clk_dma_en,hash0_clk_en,write_acc0_valid[7:0]};
wire [31:0] pack_sys_alg_clk_ctrl1             = {22'h0,ske0_clk_dma_en,ske0_clk_en,write_acc1_valid[7:0]};
wire [31:0] pack_sys_alg_clk_ctrl2             = {23'h0,pke_clk_en,write_acc2_valid[7:0]};
wire [31:0] pack_sys_alg_clk_ctrl3             = {23'h0,trng_clk_en,write_acc3_valid[7:0]};
wire [31:0] pack_sys_alg_clk_ctrl4             = {22'h0,chacha_clk_dma_en,chacha_clk_en,write_acc4_valid[7:0]};
wire [31:0] pack_sys_alg_clk_ctrl5             = {22'h0,pqc_spxp_clk_dma_en,pqc_spxp_clk_en,write_acc5_valid[7:0]};
wire [31:0] pack_sys_alg_clk_ctrl6             = {23'h0,pqc_kd_clk_en,write_acc6_valid[7:0]};
wire [31:0] pack_sys_rst_ctrl0                 = {hsm_with_boot_rst,hsm_rst,cpu_rst,2'h0,axi_dma_rst,1'h0,ahb_dma_rst,debug_mailbox_rst,2'h0,mailbox_rst,3'h0,kmu_rst,irom_rst,2'h0,emu_rst,2'h0,sys_rst,boot_rst,write_rc0_valid[7:0]};
wire [31:0] pack_sys_rst_ctrl1                 = {10'h0,mono_cnt_rst,utc_tim_rst,3'h0,tim_rst,1'h0,apb_nvm_cipher_rst,crc_rst,uart_rst,2'h0,wdt1_rst,wdt_rst,write_rc1_valid[7:0]};
wire [31:0] pack_sys_alg_rst_ctrl0             = {22'h0,hash0_dma_rst,hash0_rst,write_arc0_valid[7:0]};
wire [31:0] pack_sys_alg_rst_ctrl1             = {22'h0,ske0_dma_rst,ske0_rst,write_arc1_valid[7:0]};
wire [31:0] pack_sys_alg_rst_ctrl2             = {23'h0,pke_rst,write_arc2_valid[7:0]};
wire [31:0] pack_sys_alg_rst_ctrl3             = {23'h0,trng_rst,write_arc3_valid[7:0]};
wire [31:0] pack_sys_alg_rst_ctrl4             = {22'h0,chacha_dma_rst,chacha_rst,write_arc4_valid[7:0]};
wire [31:0] pack_sys_alg_rst_ctrl5             = {22'h0,pqc_spxp_dma_rst,pqc_spxp_rst,write_arc5_valid[7:0]};
wire [31:0] pack_sys_alg_rst_ctrl6             = {23'h0,pqc_kd_rst,write_arc6_valid[7:0]};
wire [31:0] pack_sys_cipher_ctrl               = {29'h0,nvm_cipher_bypass,bus_cipher_en,ram_cipher_en};
wire [31:0] pack_sys_soc_mem_bal               = {soc_mem_bal[31:0]};
wire [31:0] pack_sys_soc_mem_bah               = {soc_mem_bah[31:0]};
wire [31:0] pack_sys_trng_drbg_alg             = {31'h0,trng_drbg_alg};
wire [31:0] pack_sys_cpu_cfg                   = {6'h0,cpu_stcalib_cfg[25:0]};
wire [31:0] pack_sys_bus_err_cfg               = {21'h0,ahb_cfg_rsp_err_en,3'h0,ahb_nvm_rsp_err_en,1'h0,ahb_otp_rsp_err_en,3'h0,ahb_soc_rsp_err_en};
wire [31:0] pack_sys_bus_pr_cfg                = {31'h0,ahb_bus_pr_en};
wire [31:0] pack_sys_ahb_dma_cfg               = {16'h0,ahb_dma_rd_wr_to[7:0],4'h0,otp_ahb_if_sel,dma_wr_ctl[1:0],dc_dma_sel};
wire [31:0] pack_sys_axi_dma_cfg0              = {axi_dma_wost_max[7:0],axi_dma_rost_max[7:0],axi_dma_wburst_max[7:0],axi_dma_rburst_max[7:0]};
wire [31:0] pack_sys_axi_dma_cfg1              = {16'h0,axi_dma_rd_wr_to[15:0]};
wire [31:0] pack_sys_axi_dma_ch0_cfg0          = {axi_dma_ch0_rtrans_max[15:0],axi_dma_ch0_wtrans_max[15:0]};
wire [31:0] pack_sys_axi_dma_ch0_cfg1          = {axi_dma_ch0_arsnoop[3:0],axi_dma_ch0_arbar[1:0],axi_dma_ch0_ardomain[1:0],6'h0,axi_dma_ch0_arlock[1:0],axi_dma_ch0_arcache[3:0],1'h0,axi_dma_ch0_arprot[2:0],axi_dma_ch0_arqos[3:0],axi_dma_ch0_arregion[3:0]};
wire [31:0] pack_sys_axi_dma_ch0_cfg2          = {1'h0,axi_dma_ch0_awsnoop[2:0],axi_dma_ch0_awbar[1:0],axi_dma_ch0_awdomain[1:0],6'h0,axi_dma_ch0_awlock[1:0],axi_dma_ch0_awcache[3:0],1'h0,axi_dma_ch0_awprot[2:0],axi_dma_ch0_awqos[3:0],axi_dma_ch0_awregion[3:0]};
wire [31:0] pack_sys_axi_dma_ch1_cfg0          = {axi_dma_ch1_rtrans_max[15:0],axi_dma_ch1_wtrans_max[15:0]};
wire [31:0] pack_sys_axi_dma_ch1_cfg1          = {axi_dma_ch1_arsnoop[3:0],axi_dma_ch1_arbar[1:0],axi_dma_ch1_ardomain[1:0],6'h0,axi_dma_ch1_arlock[1:0],axi_dma_ch1_arcache[3:0],1'h0,axi_dma_ch1_arprot[2:0],axi_dma_ch1_arqos[3:0],axi_dma_ch1_arregion[3:0]};
wire [31:0] pack_sys_axi_dma_ch1_cfg2          = {1'h0,axi_dma_ch1_awsnoop[2:0],axi_dma_ch1_awbar[1:0],axi_dma_ch1_awdomain[1:0],6'h0,axi_dma_ch1_awlock[1:0],axi_dma_ch1_awcache[3:0],1'h0,axi_dma_ch1_awprot[2:0],axi_dma_ch1_awqos[3:0],axi_dma_ch1_awregion[3:0]};
wire [31:0] pack_sys_axi_dma_ch2_cfg0          = {axi_dma_ch2_rtrans_max[15:0],axi_dma_ch2_wtrans_max[15:0]};
wire [31:0] pack_sys_axi_dma_ch2_cfg1          = {axi_dma_ch2_arsnoop[3:0],axi_dma_ch2_arbar[1:0],axi_dma_ch2_ardomain[1:0],6'h0,axi_dma_ch2_arlock[1:0],axi_dma_ch2_arcache[3:0],1'h0,axi_dma_ch2_arprot[2:0],axi_dma_ch2_arqos[3:0],axi_dma_ch2_arregion[3:0]};
wire [31:0] pack_sys_axi_dma_ch2_cfg2          = {1'h0,axi_dma_ch2_awsnoop[2:0],axi_dma_ch2_awbar[1:0],axi_dma_ch2_awdomain[1:0],6'h0,axi_dma_ch2_awlock[1:0],axi_dma_ch2_awcache[3:0],1'h0,axi_dma_ch2_awprot[2:0],axi_dma_ch2_awqos[3:0],axi_dma_ch2_awregion[3:0]};
wire [31:0] pack_sys_axi_dma_ch3_cfg0          = {axi_dma_ch3_rtrans_max[15:0],axi_dma_ch3_wtrans_max[15:0]};
wire [31:0] pack_sys_axi_dma_ch3_cfg1          = {axi_dma_ch3_arsnoop[3:0],axi_dma_ch3_arbar[1:0],axi_dma_ch3_ardomain[1:0],6'h0,axi_dma_ch3_arlock[1:0],axi_dma_ch3_arcache[3:0],1'h0,axi_dma_ch3_arprot[2:0],axi_dma_ch3_arqos[3:0],axi_dma_ch3_arregion[3:0]};
wire [31:0] pack_sys_axi_dma_ch3_cfg2          = {1'h0,axi_dma_ch3_awsnoop[2:0],axi_dma_ch3_awbar[1:0],axi_dma_ch3_awdomain[1:0],6'h0,axi_dma_ch3_awlock[1:0],axi_dma_ch3_awcache[3:0],1'h0,axi_dma_ch3_awprot[2:0],axi_dma_ch3_awqos[3:0],axi_dma_ch3_awregion[3:0]};
wire [31:0] pack_sys_cpu_ecc_ctrl0             = {cpu_irom_ecc_ckbits[7:0],cpu_irom_ecc_tm_en[7:0],cpu_irom_ecc_err_rsp_en[7:0],cpu_irom_ecc_ck_en[7:0]};
wire [31:0] pack_sys_cpu_ecc_ctrl1             = {cpu_iram_ecc_ckbits[7:0],cpu_iram_ecc_tm_en[7:0],cpu_iram_ecc_err_rsp_en[7:0],cpu_iram_ecc_ck_en[7:0]};
wire [31:0] pack_sys_cpu_ecc_ctrl2             = {cpu_dram_ecc_ckbits[7:0],cpu_dram_ecc_tm_en[7:0],cpu_dram_ecc_err_rsp_en[7:0],cpu_dram_ecc_ck_en[7:0]};
wire [31:0] pack_sys_kmu_ecc_ctrl              = {6'h0,kmu_ram_ecc_ckbits[9:0],kmu_ram_ecc_tm_en[7:0],kmu_ram_ecc_ck_en[7:0]};
wire [31:0] pack_sys_pke_ecc_ctrl0             = {6'h0,pke_ram0_ecc_ckbits[9:0],pke_ram0_ecc_tm_en[7:0],pke_ram0_ecc_ck_en[7:0]};
wire [31:0] pack_sys_pke_ecc_ctrl1             = {6'h0,pke_ram1_ecc_ckbits[9:0],pke_ram1_ecc_tm_en[7:0],pke_ram1_ecc_ck_en[7:0]};
wire [31:0] pack_sys_pke_ecc_ctrl2             = {6'h0,pke_ram2_ecc_ckbits[9:0],pke_ram2_ecc_tm_en[7:0],pke_ram2_ecc_ck_en[7:0]};
wire [31:0] pack_sys_pke_ecc_ctrl3             = {6'h0,pke_ram3_ecc_ckbits[9:0],pke_ram3_ecc_tm_en[7:0],pke_ram3_ecc_ck_en[7:0]};
wire [31:0] pack_life_cycle                    = {life_cycle[31:0]};
wire [31:0] pack_hw_control0                   = {hw_control0[31:0]};
wire [31:0] pack_hw_control1                   = {hw_control1[31:0]};
wire [31:0] pack_fw_control0                   = {fw_control0[31:0]};
wire [31:0] pack_fw_control1                   = {fw_control1[31:0]};
wire [31:0] pack_fw_control2                   = {fw_control2[31:0]};
wire [31:0] pack_fw_control3                   = {fw_control3[31:0]};
wire [31:0] pack_sys_gen_reg0                  = {gen_reg0[31:0]};
wire [31:0] pack_sys_gen_reg1                  = {gen_reg1[31:0]};
wire [31:0] pack_sys_gen_reg2                  = {gen_reg2[31:0]};
wire [31:0] pack_sys_gen_reg3                  = {gen_reg3[31:0]};
wire [31:0] pack_sys_gen_reg4                  = {gen_reg4[31:0]};
wire [31:0] pack_sys_gen_reg5                  = {gen_reg5[31:0]};
wire [31:0] pack_sys_gen_reg6                  = {gen_reg6[31:0]};
wire [31:0] pack_sys_gen_reg7                  = {gen_reg7[31:0]};
wire [31:0] pack_sys_gen_reg8                  = {gen_reg8[31:0]};
wire [31:0] pack_sys_gen_reg9                  = {gen_reg9[31:0]};
wire [31:0] pack_sys_gen_reg10                 = {gen_reg10[31:0]};
wire [31:0] pack_sys_gen_reg11                 = {gen_reg11[31:0]};
wire [31:0] pack_sys_gen_reg12                 = {gen_reg12[31:0]};
wire [31:0] pack_sys_gen_reg13                 = {gen_reg13[31:0]};
wire [31:0] pack_sys_gen_reg14                 = {gen_reg14[31:0]};
wire [31:0] pack_sys_gen_reg15                 = {gen_reg15[31:0]};
wire [31:0] pack_sys_run_step                  = {sys_run_step[31:0]};

wire [63:0] hsm_status = {pack_sys_hsm_sta3[31:16],pack_sys_hsm_sta2[31:16],pack_sys_hsm_sta1[31:16],pack_sys_hsm_sta0[31:16]};

assign o_hrdata             = r_hrdata                    ;
assign o_hresp              = 2'h0                        ;
assign o_hreadyout          = 1'h1                        ;
assign o_clk_ctrl0          = pack_sys_clk_ctrl0          ; 
assign o_clk_ctrl1          = pack_sys_clk_ctrl1          ; 
assign o_alg_clk_ctrl0      = pack_sys_alg_clk_ctrl0      ;
assign o_alg_clk_ctrl1      = pack_sys_alg_clk_ctrl1      ;
assign o_alg_clk_ctrl2      = pack_sys_alg_clk_ctrl2      ;
assign o_alg_clk_ctrl3      = pack_sys_alg_clk_ctrl3      ;
assign o_alg_clk_ctrl4      = pack_sys_alg_clk_ctrl4      ;
assign o_alg_clk_ctrl5      = pack_sys_alg_clk_ctrl5      ;
assign o_alg_clk_ctrl6      = pack_sys_alg_clk_ctrl6      ;
assign o_rst_ctrl0          = pack_sys_rst_ctrl0          ; 
assign o_rst_ctrl1          = pack_sys_rst_ctrl1          ; 
assign o_alg_rst_ctrl0      = pack_sys_alg_rst_ctrl0      ; 
assign o_alg_rst_ctrl1      = pack_sys_alg_rst_ctrl1      ; 
assign o_alg_rst_ctrl2      = pack_sys_alg_rst_ctrl2      ; 
assign o_alg_rst_ctrl3      = pack_sys_alg_rst_ctrl3      ; 
assign o_alg_rst_ctrl4      = pack_sys_alg_rst_ctrl4      ; 
assign o_alg_rst_ctrl5      = pack_sys_alg_rst_ctrl5      ; 
assign o_alg_rst_ctrl6      = pack_sys_alg_rst_ctrl6      ; 
assign o_bus_cipher_en      = r_bus_cipher_en             ; 
assign o_ram_cipher_en      = r_ram_cipher_en             ;
assign o_nvm_cipher_bypass  = r_nvm_cipher_bypass         ;
assign o_axi_dma_cfg0       = pack_sys_axi_dma_cfg0       ; 
assign o_axi_dma_cfg1       = pack_sys_axi_dma_cfg1       ;
assign o_axi_dma_ch0_cfg0   = pack_sys_axi_dma_ch0_cfg0   ;
assign o_axi_dma_ch0_cfg1   = pack_sys_axi_dma_ch0_cfg1   ;
assign o_axi_dma_ch0_cfg2   = pack_sys_axi_dma_ch0_cfg2   ;

assign o_axi_dma_ch1_cfg0   = pack_sys_axi_dma_ch1_cfg0   ;
assign o_axi_dma_ch1_cfg1   = pack_sys_axi_dma_ch1_cfg1   ;
assign o_axi_dma_ch1_cfg2   = pack_sys_axi_dma_ch1_cfg2   ;
assign o_axi_dma_ch2_cfg0   = pack_sys_axi_dma_ch2_cfg0   ;     
assign o_axi_dma_ch2_cfg1   = pack_sys_axi_dma_ch2_cfg1   ;
assign o_axi_dma_ch2_cfg2   = pack_sys_axi_dma_ch2_cfg2   ;
assign o_axi_dma_ch3_cfg0   = pack_sys_axi_dma_ch3_cfg0   ;
assign o_axi_dma_ch3_cfg1   = pack_sys_axi_dma_ch3_cfg1   ;
assign o_axi_dma_ch3_cfg2   = pack_sys_axi_dma_ch3_cfg2   ;

assign o_irom_ecc_cfg       = pack_sys_cpu_ecc_ctrl0      ; 
assign o_iram_ecc_cfg       = pack_sys_cpu_ecc_ctrl1      ;
assign o_dram_ecc_cfg       = pack_sys_cpu_ecc_ctrl2      ;
assign o_kmu_ram_ecc_cfg    = pack_sys_kmu_ecc_ctrl       ;
assign o_pke_ram0_ecc_cfg   = pack_sys_pke_ecc_ctrl0      ;
assign o_pke_ram1_ecc_cfg   = pack_sys_pke_ecc_ctrl1      ;
assign o_pke_ram2_ecc_cfg   = pack_sys_pke_ecc_ctrl2      ;
assign o_pke_ram3_ecc_cfg   = pack_sys_pke_ecc_ctrl3      ;
assign o_mbox_ram_ba        = {soc_mem_bah,soc_mem_bal}   ;
assign o_cpu_cfg            = pack_sys_cpu_cfg            ;

assign o_ahb_dma_rd_wr_to   = ahb_dma_rd_wr_to            ; 
assign o_dc_dma_sel         = dc_dma_sel                  ; 
assign o_dma_wr_ctl         = dma_wr_ctl                  ;
assign o_otp_ahb_if_sel     = otp_ahb_if_sel              ; 
assign o_ahb_cfg_rsp_err_en = ahb_cfg_rsp_err_en          ;  
assign o_ahb_nvm_rsp_err_en = ahb_nvm_rsp_err_en          ;  
assign o_ahb_otp_rsp_err_en = ahb_otp_rsp_err_en          ; 
assign o_ahb_soc_rsp_err_en = ahb_soc_rsp_err_en          ; 
assign o_ahb_bus_pr_en      = ahb_bus_pr_en               ;
assign o_hsm_status         = hsm_status                  ;
assign o_hsm_dbg_en         = hsm_dbg_en                  ;
assign o_soc_dbg_en_128b    = {wr_soc_dbg_en_128b3,wr_soc_dbg_en_128b2,wr_soc_dbg_en_128b1,wr_soc_dbg_en_128b0};
assign o_trng_drbg_alg_sel  = trng_drbg_alg               ;
assign o_sys_irq            = |(sys_soc_sta_int & sys_soc_sta_int_en);

always @(posedge i_hclk or negedge i_hresetn) begin : ahb_decode
    if(!i_hresetn) begin
        r_wr_en                 <= 1'h0                         ; 
        r_addr                  <= 18'h0                        ; 
        r_hrdata                <= 32'h0                        ; 
    end else begin
        r_wr_en                 <= r_wr_en_next                 ; 
        r_addr                  <= r_addr_next                  ; 
        r_hrdata                <= r_hrdata_next                ; 
    end
end

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        r_nvm_cipher_bypass     <= 1'h0                         ; 
        r_bus_cipher_en         <= 1'h0                         ; 
        r_ram_cipher_en         <= 1'h0                         ; 
    end else begin
        r_nvm_cipher_bypass     <= r_nvm_cipher_bypass_next     ; 
        r_bus_cipher_en         <= r_bus_cipher_en_next         ; 
        r_ram_cipher_en         <= r_ram_cipher_en_next         ; 
    end
end

always @(posedge i_hclk or negedge i_hresetn) begin : ahb_reg
    if(!i_hresetn) begin
        hsm_fw_sta0             <= 1'h0                         ; 
        soc_verify_err          <= 1'h0                         ; 
        soc_verify_done         <= 1'h0                         ; 
        firmware_err            <= 1'h0                         ; 
        firmware_done           <= 1'h0                         ; 
        bootloader_err          <= 1'h0                         ; 
        bootloader_done         <= 1'h0                         ; 
        hsm_fw_sta1             <= 4'h0                         ; 
        hsm_fw_sta2             <= 11'h0                        ; 
        fw_upgrade_working      <= 1'h0                         ; 
        fw_backup_error         <= 1'h0                         ; 
        fw_backup_done          <= 1'h0                         ; 
        fw_upgrade_error        <= 1'h0                         ; 
        fw_upgrade_done         <= 1'h0                         ; 
        hsm_fw_sta3             <= 16'h0                        ; 
        wr_hsm_dbg_en           <= 32'h0                        ; 
        wr_soc_dbg_en           <= 32'h0                        ; 
        wr_soc_dbg_en_128b0     <= 32'h0                        ; 
        wr_soc_dbg_en_128b1     <= 32'h0                        ; 
        wr_soc_dbg_en_128b2     <= 32'h0                        ; 
        wr_soc_dbg_en_128b3     <= 32'h0                        ; 
        wr_soc_cpu_release      <= 32'h0                        ; 
        wr_soc_cpu_reset        <= 32'h0                        ; 
        wr_soc_reset            <= 32'h0                        ; 
        sys_soc_sta_int         <= 32'h0                        ; 
        sys_soc_sta_int_en      <= 32'h0                        ; 
        axi_dma_clk_en          <= 1'h1                         ; 
        ahb_dma_clk_en          <= 1'h1                         ; 
        debug_mailbox_clk_en    <= 1'h1                         ; 
        mailbox_clk_en          <= 1'h1                         ; 
        kmu_clk_en              <= 1'h1                         ; 
        irom_clk_en             <= 1'h1                         ; 
        emu_clk_en              <= 1'h1                         ; 
        boot_clk_en             <= 1'h1                         ; 
        mono_cnt_clk_en         <= 1'h1                         ; 
        utc_tim_clk_en          <= 1'h1                         ; 
        tim_clk_en              <= 1'h1                         ; 
        apb_nvm_cipher_clk_en   <= 1'h1                         ; 
        crc_clk_en              <= 1'h1                         ; 
        uart_clk_en             <= 1'h1                         ; 
        wdt1_clk_en             <= 1'h1                         ; 
        wdt_clk_en              <= 1'h1                         ; 
        hash0_clk_dma_en        <= 1'h1                         ; 
        hash0_clk_en            <= 1'h1                         ; 
        ske0_clk_dma_en         <= 1'h1                         ; 
        ske0_clk_en             <= 1'h1                         ; 
        pke_clk_en              <= 1'h1                         ; 
        trng_clk_en             <= 1'h1                         ; 
        chacha_clk_dma_en       <= 1'h1                         ; 
        chacha_clk_en           <= 1'h1                         ; 
        pqc_spxp_clk_dma_en     <= 1'h1                         ; 
        pqc_spxp_clk_en         <= 1'h1                         ; 
        pqc_kd_clk_en           <= 1'h1                         ; 
        hsm_with_boot_rst       <= 1'h1                         ; 
        hsm_rst                 <= 1'h1                         ; 
        cpu_rst                 <= 1'h1                         ; 
        axi_dma_rst             <= 1'h1                         ; 
        ahb_dma_rst             <= 1'h1                         ; 
        debug_mailbox_rst       <= 1'h1                         ; 
        mailbox_rst             <= 1'h1                         ; 
        kmu_rst                 <= 1'h1                         ; 
        irom_rst                <= 1'h1                         ; 
        emu_rst                 <= 1'h1                         ; 
        sys_rst                 <= 1'h1                         ; 
        boot_rst                <= 1'h1                         ; 
        mono_cnt_rst            <= 1'h1                         ; 
        utc_tim_rst             <= 1'h1                         ; 
        tim_rst                 <= 1'h1                         ; 
        apb_nvm_cipher_rst      <= 1'h1                         ; 
        crc_rst                 <= 1'h1                         ; 
        uart_rst                <= 1'h1                         ; 
        wdt1_rst                <= 1'h1                         ; 
        wdt_rst                 <= 1'h1                         ; 
        hash0_dma_rst           <= 1'h1                         ; 
        hash0_rst               <= 1'h1                         ; 
        ske0_dma_rst            <= 1'h1                         ; 
        ske0_rst                <= 1'h1                         ; 
        pke_rst                 <= 1'h1                         ; 
        trng_rst                <= 1'h1                         ; 
        chacha_dma_rst          <= 1'h1                         ; 
        chacha_rst              <= 1'h1                         ; 
        pqc_spxp_dma_rst        <= 1'h1                         ; 
        pqc_spxp_rst            <= 1'h1                         ; 
        pqc_kd_rst              <= 1'h1                         ; 
        soc_mem_bal             <= 32'h0                        ; 
        soc_mem_bah             <= 32'h0                        ; 
        trng_drbg_alg           <= 1'h0                         ; 
        cpu_stcalib_cfg         <= 26'h0                        ; 
        ahb_cfg_rsp_err_en      <= 1'h0                         ; 
        ahb_nvm_rsp_err_en      <= 1'h0                         ; 
        ahb_otp_rsp_err_en      <= 1'h0                         ; 
        ahb_soc_rsp_err_en      <= 1'h0                         ; 
        ahb_bus_pr_en           <= 1'b0                         ; 
        ahb_dma_rd_wr_to        <= 8'h0                         ; 
        otp_ahb_if_sel          <= 1'h0                         ; 
        dma_wr_ctl              <= 2'h3                         ; 
        dc_dma_sel              <= 1'h0                         ; 
        axi_dma_rd_wr_to        <= 16'h0                        ; 
        axi_dma_ch0_arsnoop     <= 4'h0                         ; 
        axi_dma_ch0_arbar       <= 2'h0                         ; 
        axi_dma_ch0_ardomain    <= 2'h0                         ; 
        axi_dma_ch0_arlock      <= 2'h0                         ; 
        axi_dma_ch0_arcache     <= 4'h0                         ; 
        axi_dma_ch0_arprot      <= 3'h0                         ; 
        axi_dma_ch0_arqos       <= 4'h0                         ; 
        axi_dma_ch0_arregion    <= 4'h0                         ; 
        axi_dma_ch0_awsnoop     <= 3'h0                         ; 
        axi_dma_ch0_awbar       <= 2'h0                         ; 
        axi_dma_ch0_awdomain    <= 2'h0                         ; 
        axi_dma_ch0_awlock      <= 2'h0                         ; 
        axi_dma_ch0_awcache     <= 4'h0                         ; 
        axi_dma_ch0_awprot      <= 3'h0                         ; 
        axi_dma_ch0_awqos       <= 4'h0                         ; 
        axi_dma_ch0_awregion    <= 4'h0                         ; 
        axi_dma_ch1_arsnoop     <= 4'h0                         ; 
        axi_dma_ch1_arbar       <= 2'h0                         ; 
        axi_dma_ch1_ardomain    <= 2'h0                         ; 
        axi_dma_ch1_arlock      <= 2'h0                         ; 
        axi_dma_ch1_arcache     <= 4'h0                         ; 
        axi_dma_ch1_arprot      <= 3'h0                         ; 
        axi_dma_ch1_arqos       <= 4'h0                         ; 
        axi_dma_ch1_arregion    <= 4'h0                         ; 
        axi_dma_ch1_awsnoop     <= 3'h0                         ; 
        axi_dma_ch1_awbar       <= 2'h0                         ; 
        axi_dma_ch1_awdomain    <= 2'h0                         ; 
        axi_dma_ch1_awlock      <= 2'h0                         ; 
        axi_dma_ch1_awcache     <= 4'h0                         ; 
        axi_dma_ch1_awprot      <= 3'h0                         ; 
        axi_dma_ch1_awqos       <= 4'h0                         ; 
        axi_dma_ch1_awregion    <= 4'h0                         ; 
        axi_dma_ch2_arsnoop     <= 4'h0                         ; 
        axi_dma_ch2_arbar       <= 2'h0                         ; 
        axi_dma_ch2_ardomain    <= 2'h0                         ; 
        axi_dma_ch2_arlock      <= 2'h0                         ; 
        axi_dma_ch2_arcache     <= 4'h0                         ; 
        axi_dma_ch2_arprot      <= 3'h0                         ; 
        axi_dma_ch2_arqos       <= 4'h0                         ; 
        axi_dma_ch2_arregion    <= 4'h0                         ; 
        axi_dma_ch2_awsnoop     <= 3'h0                         ; 
        axi_dma_ch2_awbar       <= 2'h0                         ; 
        axi_dma_ch2_awdomain    <= 2'h0                         ; 
        axi_dma_ch2_awlock      <= 2'h0                         ; 
        axi_dma_ch2_awcache     <= 4'h0                         ; 
        axi_dma_ch2_awprot      <= 3'h0                         ; 
        axi_dma_ch2_awqos       <= 4'h0                         ; 
        axi_dma_ch2_awregion    <= 4'h0                         ; 
        axi_dma_ch3_arsnoop     <= 4'h0                         ; 
        axi_dma_ch3_arbar       <= 2'h0                         ; 
        axi_dma_ch3_ardomain    <= 2'h0                         ; 
        axi_dma_ch3_arlock      <= 2'h0                         ; 
        axi_dma_ch3_arcache     <= 4'h0                         ; 
        axi_dma_ch3_arprot      <= 3'h0                         ; 
        axi_dma_ch3_arqos       <= 4'h0                         ; 
        axi_dma_ch3_arregion    <= 4'h0                         ; 
        axi_dma_ch3_awsnoop     <= 3'h0                         ; 
        axi_dma_ch3_awbar       <= 2'h0                         ; 
        axi_dma_ch3_awdomain    <= 2'h0                         ; 
        axi_dma_ch3_awlock      <= 2'h0                         ; 
        axi_dma_ch3_awcache     <= 4'h0                         ; 
        axi_dma_ch3_awprot      <= 3'h0                         ; 
        axi_dma_ch3_awqos       <= 4'h0                         ; 
        axi_dma_ch3_awregion    <= 4'h0                         ; 
        cpu_irom_ecc_ckbits     <= 8'h0                         ; 
        cpu_irom_ecc_tm_en      <= 8'h0                         ; 
        cpu_irom_ecc_err_rsp_en <= 8'h5a                        ; 
        cpu_irom_ecc_ck_en      <= 8'h0                         ; 
        cpu_iram_ecc_ckbits     <= 8'h0                         ; 
        cpu_iram_ecc_tm_en      <= 8'h0                         ; 
        cpu_iram_ecc_err_rsp_en <= 8'h5a                        ; 
        cpu_iram_ecc_ck_en      <= 8'h0                         ; 
        cpu_dram_ecc_ckbits     <= 8'h0                         ; 
        cpu_dram_ecc_tm_en      <= 8'h0                         ; 
        cpu_dram_ecc_err_rsp_en <= 8'h5a                        ; 
        cpu_dram_ecc_ck_en      <= 8'h0                         ; 
        kmu_ram_ecc_ckbits      <= 10'h0                        ; 
        kmu_ram_ecc_tm_en       <= 8'h0                         ; 
        kmu_ram_ecc_ck_en       <= 8'h0                         ; 
        pke_ram0_ecc_ckbits     <= 10'h0                        ; 
        pke_ram0_ecc_tm_en      <= 8'h0                         ; 
        pke_ram0_ecc_ck_en      <= 8'h0                         ; 
        pke_ram1_ecc_ckbits     <= 10'h0                        ; 
        pke_ram1_ecc_tm_en      <= 8'h0                         ; 
        pke_ram1_ecc_ck_en      <= 8'h0                         ; 
        pke_ram2_ecc_ckbits     <= 10'h0                        ; 
        pke_ram2_ecc_tm_en      <= 8'h0                         ; 
        pke_ram2_ecc_ck_en      <= 8'h0                         ; 
        pke_ram3_ecc_ckbits     <= 10'h0                        ; 
        pke_ram3_ecc_tm_en      <= 8'h0                         ; 
        pke_ram3_ecc_ck_en      <= 8'h0                         ; 
        gen_reg0                <= 32'h0                        ; 
        gen_reg1                <= 32'h0                        ; 
        gen_reg2                <= 32'h0                        ; 
        gen_reg3                <= 32'h0                        ; 
        gen_reg4                <= 32'h0                        ; 
        gen_reg5                <= 32'h0                        ; 
        gen_reg6                <= 32'h0                        ; 
        gen_reg7                <= 32'h0                        ; 
        gen_reg8                <= 32'h0                        ; 
        gen_reg9                <= 32'h0                        ; 
        gen_reg10               <= 32'h0                        ; 
        gen_reg11               <= 32'h0                        ; 
        gen_reg12               <= 32'h0                        ; 
        gen_reg13               <= 32'h0                        ; 
        gen_reg14               <= 32'h0                        ; 
        gen_reg15               <= 32'h0                        ; 
        sys_run_step            <= 32'h0                        ; 
    end else begin
        hsm_fw_sta0             <= hsm_fw_sta0_next             ; 
        soc_verify_err          <= soc_verify_err_next          ; 
        soc_verify_done         <= soc_verify_done_next         ; 
        firmware_err            <= firmware_err_next            ; 
        firmware_done           <= firmware_done_next           ; 
        bootloader_err          <= bootloader_err_next          ; 
        bootloader_done         <= bootloader_done_next         ; 
        hsm_fw_sta1             <= hsm_fw_sta1_next             ; 
        hsm_fw_sta2             <= hsm_fw_sta2_next             ; 
        fw_upgrade_working      <= fw_upgrade_working_next      ; 
        fw_backup_error         <= fw_backup_error_next         ; 
        fw_backup_done          <= fw_backup_done_next          ; 
        fw_upgrade_error        <= fw_upgrade_error_next        ; 
        fw_upgrade_done         <= fw_upgrade_done_next         ; 
        hsm_fw_sta3             <= hsm_fw_sta3_next             ; 
        wr_hsm_dbg_en           <= wr_hsm_dbg_en_next           ; 
        wr_soc_dbg_en           <= wr_soc_dbg_en_next           ; 
        wr_soc_dbg_en_128b0     <= wr_soc_dbg_en_128b0_next     ; 
        wr_soc_dbg_en_128b1     <= wr_soc_dbg_en_128b1_next     ; 
        wr_soc_dbg_en_128b2     <= wr_soc_dbg_en_128b2_next     ; 
        wr_soc_dbg_en_128b3     <= wr_soc_dbg_en_128b3_next     ; 
        wr_soc_cpu_release      <= wr_soc_cpu_release_next      ; 
        wr_soc_cpu_reset        <= wr_soc_cpu_reset_next        ; 
        wr_soc_reset            <= wr_soc_reset_next            ; 
        sys_soc_sta_int         <= sys_soc_sta_int_next         ; 
        sys_soc_sta_int_en      <= sys_soc_sta_int_en_next      ; 
        axi_dma_clk_en          <= axi_dma_clk_en_next          ; 
        ahb_dma_clk_en          <= ahb_dma_clk_en_next          ; 
        debug_mailbox_clk_en    <= debug_mailbox_clk_en_next    ; 
        mailbox_clk_en          <= mailbox_clk_en_next          ; 
        kmu_clk_en              <= kmu_clk_en_next              ; 
        irom_clk_en             <= irom_clk_en_next             ; 
        emu_clk_en              <= emu_clk_en_next              ; 
        boot_clk_en             <= boot_clk_en_next             ; 
        mono_cnt_clk_en         <= mono_cnt_clk_en_next         ; 
        utc_tim_clk_en          <= utc_tim_clk_en_next          ; 
        tim_clk_en              <= tim_clk_en_next              ; 
        apb_nvm_cipher_clk_en   <= apb_nvm_cipher_clk_en_next   ; 
        crc_clk_en              <= crc_clk_en_next              ; 
        uart_clk_en             <= uart_clk_en_next             ; 
        wdt1_clk_en             <= wdt1_clk_en_next             ; 
        wdt_clk_en              <= wdt_clk_en_next              ; 
        hash0_clk_dma_en        <= hash0_clk_dma_en_next        ; 
        hash0_clk_en            <= hash0_clk_en_next            ; 
        ske0_clk_dma_en         <= ske0_clk_dma_en_next         ; 
        ske0_clk_en             <= ske0_clk_en_next             ; 
        pke_clk_en              <= pke_clk_en_next              ; 
        trng_clk_en             <= trng_clk_en_next             ; 
        chacha_clk_dma_en       <= chacha_clk_dma_en_next       ; 
        chacha_clk_en           <= chacha_clk_en_next           ; 
        pqc_spxp_clk_dma_en     <= pqc_spxp_clk_dma_en_next     ; 
        pqc_spxp_clk_en         <= pqc_spxp_clk_en_next         ; 
        pqc_kd_clk_en           <= pqc_kd_clk_en_next           ; 
        hsm_with_boot_rst       <= hsm_with_boot_rst_next       ; 
        hsm_rst                 <= hsm_rst_next                 ; 
        cpu_rst                 <= cpu_rst_next                 ; 
        axi_dma_rst             <= axi_dma_rst_next             ; 
        ahb_dma_rst             <= ahb_dma_rst_next             ; 
        debug_mailbox_rst       <= debug_mailbox_rst_next       ; 
        mailbox_rst             <= mailbox_rst_next             ; 
        kmu_rst                 <= kmu_rst_next                 ; 
        irom_rst                <= irom_rst_next                ; 
        emu_rst                 <= emu_rst_next                 ; 
        sys_rst                 <= sys_rst_next                 ; 
        boot_rst                <= boot_rst_next                ; 
        mono_cnt_rst            <= mono_cnt_rst_next            ; 
        utc_tim_rst             <= utc_tim_rst_next             ; 
        tim_rst                 <= tim_rst_next                 ; 
        apb_nvm_cipher_rst      <= apb_nvm_cipher_rst_next      ; 
        crc_rst                 <= crc_rst_next                 ; 
        uart_rst                <= uart_rst_next                ; 
        wdt1_rst                <= wdt1_rst_next                ; 
        wdt_rst                 <= wdt_rst_next                 ; 
        hash0_dma_rst           <= hash0_dma_rst_next           ; 
        hash0_rst               <= hash0_rst_next               ; 
        ske0_dma_rst            <= ske0_dma_rst_next            ; 
        ske0_rst                <= ske0_rst_next                ; 
        pke_rst                 <= pke_rst_next                 ; 
        trng_rst                <= trng_rst_next                ; 
        chacha_dma_rst          <= chacha_dma_rst_next          ; 
        chacha_rst              <= chacha_rst_next              ; 
        pqc_spxp_dma_rst        <= pqc_spxp_dma_rst_next        ; 
        pqc_spxp_rst            <= pqc_spxp_rst_next            ; 
        pqc_kd_rst              <= pqc_kd_rst_next              ; 
        soc_mem_bal             <= soc_mem_bal_next             ; 
        soc_mem_bah             <= soc_mem_bah_next             ; 
        trng_drbg_alg           <= trng_drbg_alg_next           ; 
        cpu_stcalib_cfg         <= cpu_stcalib_cfg_next         ; 
        ahb_cfg_rsp_err_en      <= ahb_cfg_rsp_err_en_next      ; 
        ahb_nvm_rsp_err_en      <= ahb_nvm_rsp_err_en_next      ; 
        ahb_otp_rsp_err_en      <= ahb_otp_rsp_err_en_next      ; 
        ahb_soc_rsp_err_en      <= ahb_soc_rsp_err_en_next      ; 
        ahb_bus_pr_en           <= ahb_bus_pr_en_next           ; 
        ahb_dma_rd_wr_to        <= ahb_dma_rd_wr_to_next        ; 
        otp_ahb_if_sel          <= otp_ahb_if_sel_next          ; 
        dma_wr_ctl              <= dma_wr_ctl_next              ; 
        dc_dma_sel              <= dc_dma_sel_next              ; 
        axi_dma_rd_wr_to        <= axi_dma_rd_wr_to_next        ; 
        axi_dma_ch0_arsnoop     <= axi_dma_ch0_arsnoop_next     ; 
        axi_dma_ch0_arbar       <= axi_dma_ch0_arbar_next       ; 
        axi_dma_ch0_ardomain    <= axi_dma_ch0_ardomain_next    ; 
        axi_dma_ch0_arlock      <= axi_dma_ch0_arlock_next      ; 
        axi_dma_ch0_arcache     <= axi_dma_ch0_arcache_next     ; 
        axi_dma_ch0_arprot      <= axi_dma_ch0_arprot_next      ; 
        axi_dma_ch0_arqos       <= axi_dma_ch0_arqos_next       ; 
        axi_dma_ch0_arregion    <= axi_dma_ch0_arregion_next    ; 
        axi_dma_ch0_awsnoop     <= axi_dma_ch0_awsnoop_next     ; 
        axi_dma_ch0_awbar       <= axi_dma_ch0_awbar_next       ; 
        axi_dma_ch0_awdomain    <= axi_dma_ch0_awdomain_next    ; 
        axi_dma_ch0_awlock      <= axi_dma_ch0_awlock_next      ; 
        axi_dma_ch0_awcache     <= axi_dma_ch0_awcache_next     ; 
        axi_dma_ch0_awprot      <= axi_dma_ch0_awprot_next      ; 
        axi_dma_ch0_awqos       <= axi_dma_ch0_awqos_next       ; 
        axi_dma_ch0_awregion    <= axi_dma_ch0_awregion_next    ; 
        axi_dma_ch1_arsnoop     <= axi_dma_ch1_arsnoop_next     ; 
        axi_dma_ch1_arbar       <= axi_dma_ch1_arbar_next       ; 
        axi_dma_ch1_ardomain    <= axi_dma_ch1_ardomain_next    ; 
        axi_dma_ch1_arlock      <= axi_dma_ch1_arlock_next      ; 
        axi_dma_ch1_arcache     <= axi_dma_ch1_arcache_next     ; 
        axi_dma_ch1_arprot      <= axi_dma_ch1_arprot_next      ; 
        axi_dma_ch1_arqos       <= axi_dma_ch1_arqos_next       ; 
        axi_dma_ch1_arregion    <= axi_dma_ch1_arregion_next    ; 
        axi_dma_ch1_awsnoop     <= axi_dma_ch1_awsnoop_next     ; 
        axi_dma_ch1_awbar       <= axi_dma_ch1_awbar_next       ; 
        axi_dma_ch1_awdomain    <= axi_dma_ch1_awdomain_next    ; 
        axi_dma_ch1_awlock      <= axi_dma_ch1_awlock_next      ; 
        axi_dma_ch1_awcache     <= axi_dma_ch1_awcache_next     ; 
        axi_dma_ch1_awprot      <= axi_dma_ch1_awprot_next      ; 
        axi_dma_ch1_awqos       <= axi_dma_ch1_awqos_next       ; 
        axi_dma_ch1_awregion    <= axi_dma_ch1_awregion_next    ; 
        axi_dma_ch2_arsnoop     <= axi_dma_ch2_arsnoop_next     ; 
        axi_dma_ch2_arbar       <= axi_dma_ch2_arbar_next       ; 
        axi_dma_ch2_ardomain    <= axi_dma_ch2_ardomain_next    ; 
        axi_dma_ch2_arlock      <= axi_dma_ch2_arlock_next      ; 
        axi_dma_ch2_arcache     <= axi_dma_ch2_arcache_next     ; 
        axi_dma_ch2_arprot      <= axi_dma_ch2_arprot_next      ; 
        axi_dma_ch2_arqos       <= axi_dma_ch2_arqos_next       ; 
        axi_dma_ch2_arregion    <= axi_dma_ch2_arregion_next    ; 
        axi_dma_ch2_awsnoop     <= axi_dma_ch2_awsnoop_next     ; 
        axi_dma_ch2_awbar       <= axi_dma_ch2_awbar_next       ; 
        axi_dma_ch2_awdomain    <= axi_dma_ch2_awdomain_next    ; 
        axi_dma_ch2_awlock      <= axi_dma_ch2_awlock_next      ; 
        axi_dma_ch2_awcache     <= axi_dma_ch2_awcache_next     ; 
        axi_dma_ch2_awprot      <= axi_dma_ch2_awprot_next      ; 
        axi_dma_ch2_awqos       <= axi_dma_ch2_awqos_next       ; 
        axi_dma_ch2_awregion    <= axi_dma_ch2_awregion_next    ; 
        axi_dma_ch3_arsnoop     <= axi_dma_ch3_arsnoop_next     ; 
        axi_dma_ch3_arbar       <= axi_dma_ch3_arbar_next       ; 
        axi_dma_ch3_ardomain    <= axi_dma_ch3_ardomain_next    ; 
        axi_dma_ch3_arlock      <= axi_dma_ch3_arlock_next      ; 
        axi_dma_ch3_arcache     <= axi_dma_ch3_arcache_next     ; 
        axi_dma_ch3_arprot      <= axi_dma_ch3_arprot_next      ; 
        axi_dma_ch3_arqos       <= axi_dma_ch3_arqos_next       ; 
        axi_dma_ch3_arregion    <= axi_dma_ch3_arregion_next    ; 
        axi_dma_ch3_awsnoop     <= axi_dma_ch3_awsnoop_next     ; 
        axi_dma_ch3_awbar       <= axi_dma_ch3_awbar_next       ; 
        axi_dma_ch3_awdomain    <= axi_dma_ch3_awdomain_next    ; 
        axi_dma_ch3_awlock      <= axi_dma_ch3_awlock_next      ; 
        axi_dma_ch3_awcache     <= axi_dma_ch3_awcache_next     ; 
        axi_dma_ch3_awprot      <= axi_dma_ch3_awprot_next      ; 
        axi_dma_ch3_awqos       <= axi_dma_ch3_awqos_next       ; 
        axi_dma_ch3_awregion    <= axi_dma_ch3_awregion_next    ; 
        cpu_irom_ecc_ckbits     <= cpu_irom_ecc_ckbits_next     ; 
        cpu_irom_ecc_tm_en      <= cpu_irom_ecc_tm_en_next      ; 
        cpu_irom_ecc_err_rsp_en <= cpu_irom_ecc_err_rsp_en_next ; 
        cpu_irom_ecc_ck_en      <= cpu_irom_ecc_ck_en_next      ; 
        cpu_iram_ecc_ckbits     <= cpu_iram_ecc_ckbits_next     ; 
        cpu_iram_ecc_tm_en      <= cpu_iram_ecc_tm_en_next      ; 
        cpu_iram_ecc_err_rsp_en <= cpu_iram_ecc_err_rsp_en_next ; 
        cpu_iram_ecc_ck_en      <= cpu_iram_ecc_ck_en_next      ; 
        cpu_dram_ecc_ckbits     <= cpu_dram_ecc_ckbits_next     ; 
        cpu_dram_ecc_tm_en      <= cpu_dram_ecc_tm_en_next      ; 
        cpu_dram_ecc_err_rsp_en <= cpu_dram_ecc_err_rsp_en_next ; 
        cpu_dram_ecc_ck_en      <= cpu_dram_ecc_ck_en_next      ; 
        kmu_ram_ecc_ckbits      <= kmu_ram_ecc_ckbits_next      ; 
        kmu_ram_ecc_tm_en       <= kmu_ram_ecc_tm_en_next       ; 
        kmu_ram_ecc_ck_en       <= kmu_ram_ecc_ck_en_next       ; 
        pke_ram0_ecc_ckbits     <= pke_ram0_ecc_ckbits_next     ; 
        pke_ram0_ecc_tm_en      <= pke_ram0_ecc_tm_en_next      ; 
        pke_ram0_ecc_ck_en      <= pke_ram0_ecc_ck_en_next      ; 
        pke_ram1_ecc_ckbits     <= pke_ram1_ecc_ckbits_next     ; 
        pke_ram1_ecc_tm_en      <= pke_ram1_ecc_tm_en_next      ; 
        pke_ram1_ecc_ck_en      <= pke_ram1_ecc_ck_en_next      ; 
        pke_ram2_ecc_ckbits     <= pke_ram2_ecc_ckbits_next     ; 
        pke_ram2_ecc_tm_en      <= pke_ram2_ecc_tm_en_next      ; 
        pke_ram2_ecc_ck_en      <= pke_ram2_ecc_ck_en_next      ; 
        pke_ram3_ecc_ckbits     <= pke_ram3_ecc_ckbits_next     ; 
        pke_ram3_ecc_tm_en      <= pke_ram3_ecc_tm_en_next      ; 
        pke_ram3_ecc_ck_en      <= pke_ram3_ecc_ck_en_next      ; 
        gen_reg0                <= gen_reg0_next                ; 
        gen_reg1                <= gen_reg1_next                ; 
        gen_reg2                <= gen_reg2_next                ; 
        gen_reg3                <= gen_reg3_next                ; 
        gen_reg4                <= gen_reg4_next                ; 
        gen_reg5                <= gen_reg5_next                ; 
        gen_reg6                <= gen_reg6_next                ; 
        gen_reg7                <= gen_reg7_next                ; 
        gen_reg8                <= gen_reg8_next                ; 
        gen_reg9                <= gen_reg9_next                ; 
        gen_reg10               <= gen_reg10_next               ; 
        gen_reg11               <= gen_reg11_next               ; 
        gen_reg12               <= gen_reg12_next               ; 
        gen_reg13               <= gen_reg13_next               ; 
        gen_reg14               <= gen_reg14_next               ; 
        gen_reg15               <= gen_reg15_next               ; 
        sys_run_step            <= sys_run_step_next            ; 
    end
end

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        r_soc_cpu_release       <= 1'h0                         ; 
        r_soc_cpu_reset         <= 1'h0                         ; 
        r_soc_reset             <= 1'h0                         ; 
        r_soc_dbg_en            <= 1'h0                         ; 
        r_hsm_dbg_en            <= 1'h0                         ; 
    end else begin
        r_soc_cpu_release       <= r_soc_cpu_release_next       ; 
        r_soc_cpu_reset         <= r_soc_cpu_reset_next         ; 
        r_soc_reset             <= r_soc_reset_next             ; 
        r_soc_dbg_en            <= r_soc_dbg_en_next            ; 
        r_hsm_dbg_en            <= r_hsm_dbg_en_next            ; 
    end
end

always @(*) begin : ahb_hrdata
    if(!rd_en) begin
        r_hrdata_next = r_hrdata;
    end else begin
        case(addr_rd)
            P_SYS_HW_INF0                   : r_hrdata_next = pack_sys_hw_inf0                   ;
            P_SYS_HW_INF1                   : r_hrdata_next = pack_sys_hw_inf1                   ;
            P_SYS_HW_INF2                   : r_hrdata_next = pack_sys_hw_inf2                   ;
            P_SYS_HW_INF3                   : r_hrdata_next = pack_sys_hw_inf3                   ;
            P_SYS_HW_INF4                   : r_hrdata_next = pack_sys_hw_inf4                   ;
            P_SYS_HW_INF5                   : r_hrdata_next = pack_sys_hw_inf5                   ;
            P_SYS_VER0                      : r_hrdata_next = pack_sys_ver0                      ;
            P_SYS_VER1                      : r_hrdata_next = pack_sys_ver1                      ;
            P_SYS_HSM_STA0                  : r_hrdata_next = pack_sys_hsm_sta0                  ;
            P_SYS_HSM_STA1                  : r_hrdata_next = pack_sys_hsm_sta1                  ;
            P_SYS_HSM_STA2                  : r_hrdata_next = pack_sys_hsm_sta2                  ;
            P_SYS_HSM_STA3                  : r_hrdata_next = pack_sys_hsm_sta3                  ;
            P_SYS_WR_HSM_DBG_EN             : r_hrdata_next = pack_sys_wr_hsm_dbg_en             ;
            P_SYS_WR_SOC_DBG_EN             : r_hrdata_next = pack_sys_wr_soc_dbg_en             ;
            P_SYS_WR_SOC_DBG_EN_128B0       : r_hrdata_next = pack_sys_wr_soc_dbg_en_128b0       ;
            P_SYS_WR_SOC_DBG_EN_128B1       : r_hrdata_next = pack_sys_wr_soc_dbg_en_128b1       ;
            P_SYS_WR_SOC_DBG_EN_128B2       : r_hrdata_next = pack_sys_wr_soc_dbg_en_128b2       ;
            P_SYS_WR_SOC_DBG_EN_128B3       : r_hrdata_next = pack_sys_wr_soc_dbg_en_128b3       ;            
            P_SYS_WR_SOC_CPU_RELEASE        : r_hrdata_next = pack_sys_wr_soc_cpu_release        ;
            P_SYS_WR_SOC_CPU_RESET          : r_hrdata_next = pack_sys_wr_soc_cpu_reset          ;
            P_SYS_WR_SOC_RESET              : r_hrdata_next = pack_sys_wr_soc_reset              ;
            P_SYS_SOC_STA                   : r_hrdata_next = pack_sys_soc_sta                   ;
            P_SYS_SOC_STA_INT               : r_hrdata_next = pack_sys_soc_sta_int               ;
            P_SYS_SOC_STA_INT_EN            : r_hrdata_next = pack_sys_soc_sta_int_en            ;
            P_SYS_CLK_CTRL0                 : r_hrdata_next = pack_sys_clk_ctrl0                 ;
            P_SYS_CLK_CTRL1                 : r_hrdata_next = pack_sys_clk_ctrl1                 ;
            P_SYS_ALG_CLK_CTRL0             : r_hrdata_next = pack_sys_alg_clk_ctrl0             ;
            P_SYS_ALG_CLK_CTRL1             : r_hrdata_next = pack_sys_alg_clk_ctrl1             ;
            P_SYS_ALG_CLK_CTRL2             : r_hrdata_next = pack_sys_alg_clk_ctrl2             ;
            P_SYS_ALG_CLK_CTRL3             : r_hrdata_next = pack_sys_alg_clk_ctrl3             ;
            P_SYS_ALG_CLK_CTRL4             : r_hrdata_next = pack_sys_alg_clk_ctrl4             ;
            P_SYS_ALG_CLK_CTRL5             : r_hrdata_next = pack_sys_alg_clk_ctrl5             ;
            P_SYS_ALG_CLK_CTRL6             : r_hrdata_next = pack_sys_alg_clk_ctrl6             ;
            P_SYS_RST_CTRL0                 : r_hrdata_next = pack_sys_rst_ctrl0                 ;
            P_SYS_RST_CTRL1                 : r_hrdata_next = pack_sys_rst_ctrl1                 ;
            P_SYS_ALG_RST_CTRL0             : r_hrdata_next = pack_sys_alg_rst_ctrl0             ;
            P_SYS_ALG_RST_CTRL1             : r_hrdata_next = pack_sys_alg_rst_ctrl1             ;
            P_SYS_ALG_RST_CTRL2             : r_hrdata_next = pack_sys_alg_rst_ctrl2             ;
            P_SYS_ALG_RST_CTRL3             : r_hrdata_next = pack_sys_alg_rst_ctrl3             ;
            P_SYS_ALG_RST_CTRL4             : r_hrdata_next = pack_sys_alg_rst_ctrl4             ;
            P_SYS_ALG_RST_CTRL5             : r_hrdata_next = pack_sys_alg_rst_ctrl5             ;
            P_SYS_ALG_RST_CTRL6             : r_hrdata_next = pack_sys_alg_rst_ctrl6             ;
            P_SYS_CIPHER_CTRL               : r_hrdata_next = pack_sys_cipher_ctrl               ;
            P_SYS_SOC_MEM_BAL               : r_hrdata_next = pack_sys_soc_mem_bal               ;
            P_SYS_SOC_MEM_BAH               : r_hrdata_next = pack_sys_soc_mem_bah               ;
            P_SYS_TRNG_DRBG_ALG             : r_hrdata_next = pack_sys_trng_drbg_alg             ;
            P_SYS_CPU_CFG                   : r_hrdata_next = pack_sys_cpu_cfg                   ;
            P_SYS_BUS_ERR_CFG               : r_hrdata_next = pack_sys_bus_err_cfg               ;
            P_SYS_BUS_PR_CFG                : r_hrdata_next = pack_sys_bus_pr_cfg                ;
            P_SYS_AHB_DMA_CFG               : r_hrdata_next = pack_sys_ahb_dma_cfg               ;
            P_SYS_AXI_DMA_CFG0              : r_hrdata_next = pack_sys_axi_dma_cfg0              ;
            P_SYS_AXI_DMA_CFG1              : r_hrdata_next = pack_sys_axi_dma_cfg1              ;
            P_SYS_AXI_DMA_CH0_CFG0          : r_hrdata_next = pack_sys_axi_dma_ch0_cfg0          ;
            P_SYS_AXI_DMA_CH0_CFG1          : r_hrdata_next = pack_sys_axi_dma_ch0_cfg1          ;
            P_SYS_AXI_DMA_CH0_CFG2          : r_hrdata_next = pack_sys_axi_dma_ch0_cfg2          ;
            P_SYS_AXI_DMA_CH1_CFG0          : r_hrdata_next = pack_sys_axi_dma_ch1_cfg0          ;
            P_SYS_AXI_DMA_CH1_CFG1          : r_hrdata_next = pack_sys_axi_dma_ch1_cfg1          ;
            P_SYS_AXI_DMA_CH1_CFG2          : r_hrdata_next = pack_sys_axi_dma_ch1_cfg2          ;
            P_SYS_AXI_DMA_CH2_CFG0          : r_hrdata_next = pack_sys_axi_dma_ch2_cfg0          ;
            P_SYS_AXI_DMA_CH2_CFG1          : r_hrdata_next = pack_sys_axi_dma_ch2_cfg1          ;
            P_SYS_AXI_DMA_CH2_CFG2          : r_hrdata_next = pack_sys_axi_dma_ch2_cfg2          ;
            P_SYS_AXI_DMA_CH3_CFG0          : r_hrdata_next = pack_sys_axi_dma_ch3_cfg0          ;
            P_SYS_AXI_DMA_CH3_CFG1          : r_hrdata_next = pack_sys_axi_dma_ch3_cfg1          ;
            P_SYS_AXI_DMA_CH3_CFG2          : r_hrdata_next = pack_sys_axi_dma_ch3_cfg2          ;
            P_SYS_CPU_ECC_CTRL0             : r_hrdata_next = pack_sys_cpu_ecc_ctrl0             ;
            P_SYS_CPU_ECC_CTRL1             : r_hrdata_next = pack_sys_cpu_ecc_ctrl1             ;
            P_SYS_CPU_ECC_CTRL2             : r_hrdata_next = pack_sys_cpu_ecc_ctrl2             ;
            P_SYS_KMU_ECC_CTRL              : r_hrdata_next = pack_sys_kmu_ecc_ctrl              ;
            P_SYS_PKE_ECC_CTRL0             : r_hrdata_next = pack_sys_pke_ecc_ctrl0             ;
            P_SYS_PKE_ECC_CTRL1             : r_hrdata_next = pack_sys_pke_ecc_ctrl1             ;
            P_SYS_PKE_ECC_CTRL2             : r_hrdata_next = pack_sys_pke_ecc_ctrl2             ;
            P_SYS_PKE_ECC_CTRL3             : r_hrdata_next = pack_sys_pke_ecc_ctrl3             ;
            P_LIFE_CYCLE                    : r_hrdata_next = pack_life_cycle                    ;
            P_HW_CONTROL0                   : r_hrdata_next = pack_hw_control0                   ;
            P_HW_CONTROL1                   : r_hrdata_next = pack_hw_control1                   ;
            P_FW_CONTROL0                   : r_hrdata_next = pack_fw_control0                   ;
            P_FW_CONTROL1                   : r_hrdata_next = pack_fw_control1                   ;
            P_FW_CONTROL2                   : r_hrdata_next = pack_fw_control2                   ;
            P_FW_CONTROL3                   : r_hrdata_next = pack_fw_control3                   ;
            P_SYS_GEN_REG0                  : r_hrdata_next = pack_sys_gen_reg0                  ;
            P_SYS_GEN_REG1                  : r_hrdata_next = pack_sys_gen_reg1                  ;
            P_SYS_GEN_REG2                  : r_hrdata_next = pack_sys_gen_reg2                  ;
            P_SYS_GEN_REG3                  : r_hrdata_next = pack_sys_gen_reg3                  ;
            P_SYS_GEN_REG4                  : r_hrdata_next = pack_sys_gen_reg4                  ;
            P_SYS_GEN_REG5                  : r_hrdata_next = pack_sys_gen_reg5                  ;
            P_SYS_GEN_REG6                  : r_hrdata_next = pack_sys_gen_reg6                  ;
            P_SYS_GEN_REG7                  : r_hrdata_next = pack_sys_gen_reg7                  ;
            P_SYS_GEN_REG8                  : r_hrdata_next = pack_sys_gen_reg8                  ;
            P_SYS_GEN_REG9                  : r_hrdata_next = pack_sys_gen_reg9                  ;
            P_SYS_GEN_REG10                 : r_hrdata_next = pack_sys_gen_reg10                 ;
            P_SYS_GEN_REG11                 : r_hrdata_next = pack_sys_gen_reg11                 ;
            P_SYS_GEN_REG12                 : r_hrdata_next = pack_sys_gen_reg12                 ;
            P_SYS_GEN_REG13                 : r_hrdata_next = pack_sys_gen_reg13                 ;
            P_SYS_GEN_REG14                 : r_hrdata_next = pack_sys_gen_reg14                 ;
            P_SYS_GEN_REG15                 : r_hrdata_next = pack_sys_gen_reg15                 ;
            P_SYS_RUN_STEP                  : r_hrdata_next = pack_sys_run_step                  ;
            default                         : r_hrdata_next = 32'h0                              ;
        endcase
    end
end

endmodule 
