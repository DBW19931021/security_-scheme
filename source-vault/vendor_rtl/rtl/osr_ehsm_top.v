//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ehsm_top(
    input  wire                 i_clk                   ,
    input  wire                 i_rst_n                 ,

    input  wire                 i_scan_mode             ,
    input  wire                 i_scan_icg_enable       ,

    input  wire [ 31:0]         i_soc_status            ,
    input  wire [ 31:0]         i_soc_err               ,

    output wire [ 63:0]         o_hsm_status            ,
    output wire [ 63:0]         o_hsm_err_hw            ,
    output wire [ 63:0]         o_hsm_err_fw            ,

    output wire [ 15:0]         o_mbox_irq              ,

    input  wire                 i_s_hsel                ,
    input  wire [ 31:0]         i_s_haddr               ,
    input  wire [  1:0]         i_s_htrans              ,
    input  wire                 i_s_hwrite              ,
    input  wire [  2:0]         i_s_hburst              ,
    input  wire [  2:0]         i_s_hsize               ,
    input  wire [  3:0]         i_s_hprot               ,
    input  wire                 i_s_hmastlock           ,
    input  wire                 i_s_hready              ,
    input  wire [ 31:0]         i_s_hwdata              ,
    output wire [ 31:0]         o_s_hrdata              ,
    output wire [  1:0]         o_s_hresp               ,
    output wire                 o_s_hreadyout           ,

    output wire                 o_m_hsel                ,
    output wire [ 63:0]         o_m_haddr               ,
    output wire [  1:0]         o_m_htrans              ,
    output wire                 o_m_hwrite              ,
    output wire [  2:0]         o_m_hburst              ,
    output wire [  2:0]         o_m_hsize               ,
    output wire [  3:0]         o_m_hprot               ,
    output wire                 o_m_hmastlock           ,
    output wire                 o_m_hready              ,
    output wire [ 31:0]         o_m_hwdata              ,
    input  wire [ 31:0]         i_m_hrdata              ,
    input  wire [  1:0]         i_m_hresp               ,
    input  wire                 i_m_hreadyout           ,

    output wire [  3:0]         o_dma_m_awid            ,
    output wire [ 63:0]         o_dma_m_awaddr          ,
    output wire [  7:0]         o_dma_m_awlen           ,
    output wire [  2:0]         o_dma_m_awsize          ,
    output wire [  1:0]         o_dma_m_awburst         ,
    output wire                 o_dma_m_awlock          ,
    output wire [  3:0]         o_dma_m_awcache         ,
    output wire [  2:0]         o_dma_m_awprot          ,
    output wire [  3:0]         o_dma_m_awqos           ,
    output wire [  3:0]         o_dma_m_awregion        ,
    output wire                 o_dma_m_awvalid         ,
    input  wire                 i_dma_m_awready         ,

    output wire [ 63:0]         o_dma_m_wdata           ,
    output wire [  7:0]         o_dma_m_wstrb           ,
    output wire                 o_dma_m_wlast           ,
    output wire                 o_dma_m_wvalid          ,
    input  wire                 i_dma_m_wready          ,

    input  wire [  3:0]         i_dma_m_bid             ,
    input  wire [  1:0]         i_dma_m_bresp           ,
    input  wire                 i_dma_m_bvalid          ,
    output wire                 o_dma_m_bready          ,

    output wire [  3:0]         o_dma_m_arid            ,
    output wire [ 63:0]         o_dma_m_araddr          ,
    output wire [  7:0]         o_dma_m_arlen           ,
    output wire [  2:0]         o_dma_m_arsize          ,
    output wire [  1:0]         o_dma_m_arburst         ,
    output wire                 o_dma_m_arlock          ,
    output wire [  3:0]         o_dma_m_arcache         ,
    output wire [  2:0]         o_dma_m_arprot          ,
    output wire [  3:0]         o_dma_m_arqos           ,
    output wire [  3:0]         o_dma_m_arregion        ,
    output wire                 o_dma_m_arvalid         ,
    input  wire                 i_dma_m_arready         ,

    input  wire [  3:0]         i_dma_m_rid             ,
    input  wire [ 63:0]         i_dma_m_rdata           ,
    input  wire [  1:0]         i_dma_m_rresp           ,
    input  wire                 i_dma_m_rlast           ,
    input  wire                 i_dma_m_rvalid          ,
    output wire                 o_dma_m_rready          ,

    output wire                 o_cfg_hsel              ,
    output wire [ 31:0]         o_cfg_haddr             ,
    output wire [  1:0]         o_cfg_htrans            ,
    output wire                 o_cfg_hwrite            ,
    output wire [  2:0]         o_cfg_hburst            ,
    output wire [  2:0]         o_cfg_hsize             ,
    output wire [  3:0]         o_cfg_hprot             ,
    output wire                 o_cfg_hmastlock         ,
    output wire                 o_cfg_hready            ,
    output wire [ 31:0]         o_cfg_hwdata            ,
    input  wire [ 31:0]         i_cfg_hrdata            ,
    input  wire [  1:0]         i_cfg_hresp             ,
    input  wire                 i_cfg_hreadyout         ,

    output wire                 o_otp_hsel              ,
    output wire [ 31:0]         o_otp_haddr             ,
    output wire [  1:0]         o_otp_htrans            ,
    output wire                 o_otp_hwrite            ,
    output wire [  2:0]         o_otp_hburst            ,
    output wire [  2:0]         o_otp_hsize             ,
    output wire [  3:0]         o_otp_hprot             ,
    output wire                 o_otp_hmastlock         ,
    output wire                 o_otp_hready            ,
    output wire [ 31:0]         o_otp_hwdata            ,
    input  wire [ 31:0]         i_otp_hrdata            ,
    input  wire [  1:0]         i_otp_hresp             ,
    input  wire                 i_otp_hreadyout         ,

    output wire                 o_irom_CK               ,
    output wire                 o_irom_CSB              ,
    output wire [ 13:0]         o_irom_A                ,

    input  wire [ 38:0]         i_irom_DO               ,

    `ifdef OSR_FPGA_ROM 
    output wire [ 38:0]         o_irom_DI               ,
    output wire                 o_irom_WEB              ,
    `endif 

    output wire                 o_iram_CK               ,
    output wire [ 15:0]         o_iram_A                ,
    output wire                 o_iram_CSB              ,
    output wire                 o_iram_WEB              ,
    output wire [ 38:0]         o_iram_DI               ,
    input  wire [ 38:0]         i_iram_DO               ,

    output wire                 o_dram_CK               ,
    output wire [ 13:0]         o_dram_A                ,
    output wire                 o_dram_CSB              ,
    output wire                 o_dram_WEB              ,
    output wire [ 38:0]         o_dram_DI               ,
    input  wire [ 38:0]         i_dram_DO               ,

    output wire                 o_kbuf_CK               ,
    output wire                 o_kbuf_CSB              ,
    output wire [  7:0]         o_kbuf_A                ,
    output wire                 o_kbuf_WEB              ,
    output wire [ 38:0]         o_kbuf_DI               ,
    input  wire [ 38:0]         i_kbuf_DO               ,

    output wire                 o_pke_sram0_CK          ,
    output wire                 o_pke_sram0_CSAN        ,
    output wire [  8:0]         o_pke_sram0_A           ,
    input  wire [ 71:0]         i_pke_sram0_DO          ,
    output wire                 o_pke_sram0_CSBN        ,
    output wire [  8:0]         o_pke_sram0_B           ,
    output wire [ 71:0]         o_pke_sram0_DI          ,

    output wire                 o_pke_sram1_CK          ,
    output wire                 o_pke_sram1_CSAN        ,
    output wire [  8:0]         o_pke_sram1_A           ,
    input  wire [ 71:0]         i_pke_sram1_DO          ,
    output wire                 o_pke_sram1_CSBN        ,
    output wire [  8:0]         o_pke_sram1_B           ,
    output wire [ 71:0]         o_pke_sram1_DI          ,

    output wire                 o_pke_sram2_CK          ,
    output wire                 o_pke_sram2_CSAN        ,
    output wire [  8:0]         o_pke_sram2_A           ,
    input  wire [ 71:0]         i_pke_sram2_DO          ,
    output wire                 o_pke_sram2_CSBN        ,
    output wire [  8:0]         o_pke_sram2_B           ,
    output wire [ 71:0]         o_pke_sram2_DI          ,

    output wire                 o_pke_sram3_CK          ,
    output wire                 o_pke_sram3_CSAN        ,
    output wire [  8:0]         o_pke_sram3_A           ,
    input  wire [ 71:0]         i_pke_sram3_DO          ,
    output wire                 o_pke_sram3_CSBN        ,
    output wire [  8:0]         o_pke_sram3_B           ,
    output wire [ 71:0]         o_pke_sram3_DI          ,

    input  wire                 i_trst_n                ,
    input  wire                 i_tck                   ,
    input  wire                 i_tms                   ,
    input  wire                 i_tdi                   ,
    output wire                 o_tdo_oe                ,
    output wire                 o_tdo                   ,

    input  wire                 i_uart_rxd              ,
    output wire                 o_uart_txd_oe           ,
    output wire                 o_uart_txd              ,

    output wire [127:0]         o_soc_dbg_en_128b       ,

    output wire                 o_trng_rdy              ,
    output wire                 o_trng_alarm            ,

    output wire [  3:0]         o_trng_ro_clk           ,
    output wire [  3:0]         o_trng_ro_out            
);

localparam P_IROM_CIPHER_EN = 0 ;

localparam P_IROM_ECC_EN = 1 ;

localparam  P_IROM_BYTE_OPT_EN = 1;

localparam P_IRAM_CIPHER_EN = 0 ;

localparam P_IRAM_ECC_EN = 1 ;

localparam  P_IRAM_BYTE_OPT_EN = 1;

localparam P_CM4_DRAM_CIPHER_EN = 0 ;

localparam P_CM4_DRAM_ECC_EN = 1 ;

localparam  P_CM4_DRAM_BYTE_OPT_EN = 1;

localparam P_KMU_RAM_CIPHER_EN = 0 ;

localparam P_KMU_RAM_ECC_EN = 1 ;

localparam P_IROM_DIRECT_TO_ILM = 0 ;

localparam P_DRAM_DIRECT_TO_CPU = 0 ;

localparam P_OSR_CPU_CLK_DIV =0;

localparam SOC_HBUS_AW_WIDTH = 64 ;

localparam OTP_AHB_EN = 1;

localparam IRAM_INIT  = 1;

localparam CM4_DRAM_INIT  = 1;
localparam OSR_SOC_AHB_BUS_TIMING_ISOLATION = `OSR_SOC_AHB_BUS_TIMING_ISOLATION;
localparam OSR_NVM_AHB_BUS_TIMING_ISOLATION = `OSR_NVM_AHB_BUS_TIMING_ISOLATION;
localparam OSR_OTP_AHB_BUS_TIMING_ISOLATION = `OSR_OTP_AHB_BUS_TIMING_ISOLATION;
localparam OSR_ABUS_AHB_BUS_TIMING_ISOLATION = `OSR_ABUS_AHB_BUS_TIMING_ISOLATION;
localparam OSR_AHB_DMA_TIMING_ISOLATION = `OSR_AHB_DMA_TIMING_ISOLATION;

localparam     ROMCFG_SYN_WIDTH = 32;

localparam P_RAM_ECC_CHKBIT_32 = 7;

localparam A1OSR_NVM_AHB_BUS_TIMING_ISOLATION     = (OSR_NVM_AHB_BUS_TIMING_ISOLATION    > 0) ? OSR_NVM_AHB_BUS_TIMING_ISOLATION     : 1 ;
localparam A1OSR_SOC_AHB_BUS_TIMING_ISOLATION     = (OSR_SOC_AHB_BUS_TIMING_ISOLATION    > 0) ? OSR_SOC_AHB_BUS_TIMING_ISOLATION     : 1 ;
localparam A1OSR_OTP_AHB_BUS_TIMING_ISOLATION     = (OSR_OTP_AHB_BUS_TIMING_ISOLATION    > 0) ? OSR_OTP_AHB_BUS_TIMING_ISOLATION     : 1 ;
localparam A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION    = (OSR_ABUS_AHB_BUS_TIMING_ISOLATION   > 0) ? OSR_ABUS_AHB_BUS_TIMING_ISOLATION    : 1 ;
localparam A1OSR_AHB_DMA_TIMING_ISOLATION         = (OSR_AHB_DMA_TIMING_ISOLATION        > 0) ? OSR_AHB_DMA_TIMING_ISOLATION         : 1 ;

localparam     AHBSYN_P_AW = 32;
localparam     AHBSYN_P_DW = 36;
localparam     AHBSYN_P_MW = 8 ;

localparam     AHB_P_DMA_AW = 64;
localparam     AHB_P_DMA_DW = 64;

localparam     OTP2APB_ADDRWIDTH      = 32  ; 
localparam     OTP2APB_REGISTER_RDATA = 1   ; 
localparam     OTP2APB_REGISTER_WDATA = 1   ; 

localparam     CFG2APB_ADDRWIDTH      = 32  ;
localparam     CFG2APB_REGISTER_RDATA = 1   ;
localparam     CFG2APB_REGISTER_WDATA = 1   ;

localparam     MBM2APB_ADDRWIDTH      = 32  ;
localparam     MBM2APB_REGISTER_RDATA = 1   ;
localparam     MBM2APB_REGISTER_WDATA = 1   ;

localparam     MB2AHB_ADDRWIDTH      = 32  ;
localparam     MB2AHB_REGISTER_RDATA = 1   ;
localparam     MB2AHB_REGISTER_WDATA = 1   ;

localparam     NVMHMSYNC_P_AW    = 32 ;
localparam     NVMHMSYNC_P_DW    = 32 ;
localparam     NVMHMSYNC_P_MW    = 2  ;
localparam     NVMHMSYNC_P_BURST = 1  ;

localparam     SOCHMSYNC_P_AW    = SOC_HBUS_AW_WIDTH;
localparam     SOCHMSYNC_P_DW    = 32 ;
localparam     SOCHMSYNC_P_MW    = 2  ;
localparam     SOCHMSYNC_P_BURST = 1  ;

localparam     OTPHMSYNC_P_AW    = 32 ; 
localparam     OTPHMSYNC_P_DW    = 32 ; 
localparam     OTPHMSYNC_P_MW    = 2  ; 
localparam     OTPHMSYNC_P_BURST = 1  ; 

localparam     ABUSHMSYNC_P_AW    = 32 ;
localparam     ABUSHMSYNC_P_DW    = 32 ;
localparam     ABUSHMSYNC_P_MW    = 2  ;
localparam     ABUSHMSYNC_P_BURST = 1  ;

localparam     AHBDMASYNC_P_AW    = 32 ;
localparam     AHBDMASYNC_P_DW    = 36 ;
localparam     AHBDMASYNC_P_MW    = 8  ;
localparam     AHBDMASYNC_P_BURST = 1  ;

localparam     IRAM_P_BUS_AW         = 18 ;
localparam     IRAM_P_BUS_DW         = 32;
localparam     IRAM_P_BUS_NO_WORD    = P_IRAM_BYTE_OPT_EN;
localparam     IRAM_P_AHB_TIMING     = 0;
localparam     IRAM_P_AHB2RAM_TIMING = `OSR_IRAM_AHB_BUS_TIMING_ISOLATION;
localparam     IRAM_P_CIPHER_EN      = P_IRAM_CIPHER_EN;
localparam     IRAM_P_RAM_BLOCK_AW   = 4;
localparam     IRAM_P_ECC_EN         = P_IRAM_ECC_EN;
localparam     IRAM_P_ECC_TM_CHB     = (IRAM_P_BUS_DW == 32) ? P_RAM_ECC_CHKBIT_32 : (IRAM_P_BUS_DW == 64) ? 8 : 7;
localparam     IRAM_P_RAM_DW         = (IRAM_P_ECC_EN == 0) ? IRAM_P_BUS_DW : (IRAM_P_BUS_DW == 32) ? P_RAM_ECC_CHKBIT_32+32 : (IRAM_P_BUS_DW == 64) ? 8+64 : 32;
localparam     IRAM_P_MEM_INIT       = IRAM_INIT;
localparam     IRAM_P_MEM_INIT_DEEP  = 256*1024/4;

localparam     DRAM_P_BUS_AW         = 16 ;
localparam     DRAM_P_BUS_DW         = 32;
localparam     DRAM_P_BUS_NO_WORD    = P_CM4_DRAM_BYTE_OPT_EN;
localparam     DRAM_P_AHB_TIMING     = 0;
localparam     DRAM_P_AHB2RAM_TIMING = `OSR_DRAM_AHB_BUS_TIMING_ISOLATION;
localparam     DRAM_P_CIPHER_EN      = P_CM4_DRAM_CIPHER_EN;
localparam     DRAM_P_RAM_BLOCK_AW   = 2;
localparam     DRAM_P_ECC_EN         = P_CM4_DRAM_ECC_EN;
localparam     DRAM_P_ECC_TM_CHB     = (DRAM_P_BUS_DW == 32) ? 7 : (DRAM_P_BUS_DW == 64) ? 8 : 7;
localparam     DRAM_P_RAM_DW         = (DRAM_P_ECC_EN == 0) ? DRAM_P_BUS_DW : (DRAM_P_BUS_DW == 32) ? 7+32 : (DRAM_P_BUS_DW == 64) ? 8+64 : 32;
localparam     DRAM_P_MEM_INIT       = CM4_DRAM_INIT;
localparam     DRAM_P_MEM_INIT_DEEP  = 64*1024/4;

localparam     KBUF_P_RAM_AW         = 8;
localparam     KBUF_P_RAM_DW         = 32;
localparam     KBUF_P_RAM_CIPHER_EN  = P_KMU_RAM_CIPHER_EN;
localparam     KBUF_P_RAM_CIPHER_RN  = 2;
localparam     KBUF_P_RAM_BLOCK_AW   = 0; 
localparam     KBUF_P_RAM_ECC_EN     = P_KMU_RAM_ECC_EN;
localparam     KBUF_P_RAM_ECC_TM_CHB = (KBUF_P_RAM_DW == 32) ? 7 : (KBUF_P_RAM_DW== 64) ? 8 : 7 ;
localparam     KBUF_P_RAM_ECC_DW     = (KBUF_P_RAM_ECC_EN == 0) ? KBUF_P_RAM_DW : (KBUF_P_RAM_DW == 32) ? 7+32 : (KBUF_P_RAM_DW== 64) ? 8+64 : 32 ;

localparam     NVMABUSCIPHER_P_BUS_AW    = 32 ;
localparam     NVMABUSCIPHER_P_ENC_AW    = 19 ;
localparam     NVMABUSCIPHER_P_ENC_AR    = 20 ;
localparam     NVMABUSCIPHER_P_ADDR_BW   = 5  ;
localparam     NVMABUSCIPHER_P_TIMING_EN = 0  ;

localparam     EXSEN_WIDTH = 32 ;

localparam     EXSTA_WIDTH = 32 ; 

localparam     EXERR_WIDTH = 32 ;

localparam     EXIRQ_WIDTH = 32 ;

localparam     ROMAHBSYN_P_AW    = 32   ;
localparam     ROMAHBSYN_P_DW    = 36   ;
localparam     ROMAHBSYN_P_MW    = 8    ;
localparam     ROMAHBSYN_P_BURST = 0    ;
localparam     ROMAHBSYN_P_WB    = 0    ;

localparam     IROM_P_BUS_AW         = 16 + 0  ;
localparam     IROM_P_BUS_DW         = 32                  ;
localparam     IROM_P_BUS_NO_WORD    = P_IROM_BYTE_OPT_EN  ; 
localparam     IROM_P_AHB2RAM_TIMING = `OSR_IROM_AHB_BUS_TIMING_ISOLATION;
localparam     IROM_P_CIPHER_EN      = P_IROM_CIPHER_EN    ;
localparam     IROM_P_ECC_EN         = P_IROM_ECC_EN       ;
localparam     IROM_P_ECC_TM_CHB     = (IROM_P_BUS_DW== 32) ? 7 : (IROM_P_BUS_DW == 64) ? 8 : 7;
localparam     IROM_P_ROM_DW         = (IROM_P_ECC_EN == 0) ? IROM_P_BUS_DW : (IROM_P_BUS_DW== 32) ? 7+32 : (IROM_P_BUS_DW == 64) ? 8+64 : 32;

localparam     FIROM_P_BUS_AW         = 16 + 0 ;
localparam     FIROM_P_BUS_DW         = 32                  ;
localparam     FIROM_P_BUS_NO_WORD    = P_IROM_BYTE_OPT_EN  ;
localparam     FIROM_P_AHB2RAM_TIMING = 0                   ;
localparam     FIROM_P_CIPHER_EN      = P_IROM_CIPHER_EN    ;
localparam     FIROM_P_ECC_EN         = P_IROM_ECC_EN       ;
localparam     FIROM_P_ECC_TM_CHB     = (IROM_P_BUS_DW== 32) ? 7 : (IROM_P_BUS_DW == 64) ? 8 : 7;
localparam     FIROM_P_ROM_DW         = (IROM_P_ECC_EN == 0) ? IROM_P_BUS_DW : (IROM_P_BUS_DW== 32) ? 7+32 : (IROM_P_BUS_DW == 64) ? 8+64 : 32;

wire            ahb_dma_m_hsel      ;
wire [32-1:0]   ahb_dma_m_haddr     ;
wire [1:0]      ahb_dma_m_htrans    ;
wire            ahb_dma_m_hwrite    ;
wire [2:0]      ahb_dma_m_hburst    ;
wire [2:0]      ahb_dma_m_hsize     ;
wire [3:0]      ahb_dma_m_hprot     ;
wire            ahb_dma_m_hmastlock ;
wire [35:0]     ahb_dma_m_hwdata    ;
wire [7:0]      ahb_dma_m_hmaster   ;
wire            otp_ahb_sel_boot    ;

wire [31:0]     w_otp_hrdata        ;  
wire [1:0]      w_otp_hresp         ;
wire            w_otp_hreadyout     ;

wire [31:0]     w_cfg_hrdata        ; 
wire [1:0]      w_cfg_hresp         ; 
wire            w_cfg_hreadyout     ; 

wire [31:0]     w_m_hrdata          ;
wire [1:0]      w_m_hresp           ;
wire            w_m_hreadyout       ;

wire            w_s_hsel            ; 
wire [31:0]     w_s_haddr           ; 
wire [1:0]      w_s_htrans          ; 
wire            w_s_hwrite          ; 
wire [2:0]      w_s_hburst          ; 
wire [2:0]      w_s_hsize           ; 
wire [3:0]      w_s_hprot           ; 
wire            w_s_hmastlock       ; 
wire            w_s_hready          ; 
wire [31:0]     w_s_hwdata          ; 

wire [31:0]     w_mtx_o_haddrm4     ; 

wire            ahbdmasync_o_hready ;

wire [31:0] w_mtx_i_nvm_hrdatam4    ;
wire        w_mtx_i_nvm_hreadyoutm4 ;
wire [1:0]  w_mtx_i_nvm_hrespm4     ;

`ifdef OSR_FPGA
wire                                              syn_firom_ecc_dec_sec                    ; 
wire                                              syn_firom_ecc_dec_ded                    ; 
wire [16 + 0-3:0] syn_firom_ecc_err_addr                   ; 
`endif

reg                                              soc_err_ahb_mem          ;
reg                                              soc_err_ahb_otp          ;
reg                                              soc_err_ahb_cfg          ;
reg                                              soc_err_axi_dma_wr       ;
reg                                              soc_err_axi_dma_rd       ;

wire                                              soc_err_ahb_nvm          = 1'b0;


wire                                              soc_err_axi_dma_wr_r     = 1'b0  ; 
wire                                              soc_err_axi_dma_rd_r     = 1'b0  ;

 wire [31:0]          i_nvm_hrdata            = 32'b0;
 wire [1:0]           i_nvm_hresp             = 2'b0 ;
 wire                 i_nvm_hreadyout         = 1'b1 ;

wire [31:0]    soc_hrdata              ;
wire           soc_hreadyout           ;
wire [1:0]     soc_hresp               ;

wire [31:0]    otp_hrdata              ;
wire           otp_hreadyout           ;
wire [1:0]     otp_hresp               ;

wire [31:0]    nvm_hrdata              ;
wire           nvm_hreadyout           ;
wire [1:0]     nvm_hresp               ;

wire [31:0]    cfg_hrdata              ;
wire           cfg_hreadyout           ;
wire [1:0]     cfg_hresp               ;

wire sochmsync_o_hready;
wire nvmhmsync_o_hready;
wire otphmsync_o_hready;
wire abushmsync_o_hready;

`ifndef OSR_FPGA_ROM
        wire                            irom_o_ecc_dec_sec       ;
        wire                            irom_o_ecc_dec_ded       ;
        wire [IROM_P_BUS_AW-3:0]        irom_o_ecc_err_addr      ;
    wire [31:0]                     irom_o_hrdata            ;
    wire                            irom_o_hreadyout         ;
    wire [1:0]                      irom_o_hresp             ;
    wire                            irom_o_rom_cs            ;
    wire [IROM_P_BUS_AW-3:0]        irom_o_rom_addr          ;
`else
        wire                            irom_o_ecc_dec_sec       = 1'h0     ;
        wire                            irom_o_ecc_dec_ded       = 1'h0     ;
        wire [IROM_P_BUS_AW-3:0]        irom_o_ecc_err_addr      = {{IROM_P_BUS_AW-2}{1'h0}};
    wire [31:0]                     irom_o_hrdata            = 32'h0    ;
    wire                            irom_o_hreadyout         = 1'h1     ;
    wire [1:0]                      irom_o_hresp             = 2'h0     ;
    wire                            irom_o_rom_cs            = 1'h0     ;
    wire [IROM_P_BUS_AW-3:0]        irom_o_rom_addr          = {{IROM_P_BUS_AW-2}{1'h0}}   ;

`endif

wire                                              syn_irom_ecc_dec_sec                      = irom_o_ecc_dec_sec    ; 
wire                                              syn_irom_ecc_dec_ded                      = irom_o_ecc_dec_ded    ; 
wire [16 + 0-3:0] syn_irom_ecc_err_addr                     = irom_o_ecc_err_addr   ; 

`ifdef OSR_FPGA_ROM 
wire                             firom_o_ecc_dec_sec       ;
wire                             firom_o_ecc_dec_ded       ;
wire [FIROM_P_BUS_AW-3:0]        firom_o_ecc_err_addr      ;
wire [31:0]                      firom_o_hrdata            ;
wire                             firom_o_hreadyout         ;
wire [1:0]                       firom_o_hresp             ;
wire                             firom_o_rom_cs            ;
wire [3:0]                       firom_o_rom_wen           ;
wire [FIROM_P_BUS_AW-3:0]        firom_o_rom_addr          ;
wire [FIROM_P_ROM_DW-1:0]        firom_o_rom_wdata         ;
`else 
wire                             firom_o_ecc_dec_sec       =  1'h0   ;
wire                             firom_o_ecc_dec_ded       =  1'h0   ;
wire [FIROM_P_BUS_AW-3:0]        firom_o_ecc_err_addr      =  {FIROM_P_BUS_AW-2{1'b0}};
wire [31:0]                      firom_o_hrdata            =  32'h0  ;
wire                             firom_o_hreadyout         =  1'h1   ;
wire [1:0]                       firom_o_hresp             =  2'h0   ;
wire                             firom_o_rom_cs            =  1'h0   ;
wire [3:0]                       firom_o_rom_wen           =  4'h0   ;
wire [FIROM_P_BUS_AW-3:0]        firom_o_rom_addr          =  {FIROM_P_BUS_AW-2{1'b0}};
wire [FIROM_P_ROM_DW-1:0]        firom_o_rom_wdata         =  {FIROM_P_ROM_DW{1'b0}};
`endif 
`ifdef OSR_FPGA_ROM 
wire                             ilmrom_o_ecc_dec_sec       = syn_firom_ecc_dec_sec ;
wire                             ilmrom_o_ecc_dec_ded       = syn_firom_ecc_dec_ded ;
wire [FIROM_P_BUS_AW-3:0]        ilmrom_o_ecc_err_addr      = syn_firom_ecc_err_addr;
wire [31:0]                      ilmrom_o_hrdata            = firom_o_hrdata        ;
wire                             ilmrom_o_hreadyout         = firom_o_hreadyout     ;
wire [1:0]                       ilmrom_o_hresp             = firom_o_hresp         ;
wire                             ilmrom_o_rom_cs            = firom_o_rom_cs        ;
wire [3:0]                       ilmrom_o_ram_wen           = firom_o_rom_wen       ;
wire [FIROM_P_BUS_AW-3:0]        ilmrom_o_rom_addr          = firom_o_rom_addr      ;
wire [FIROM_P_ROM_DW-1:0]        ilmrom_o_ram_wdata         = firom_o_rom_wdata     ;
`else 
wire                             ilmrom_o_ecc_dec_sec       = syn_irom_ecc_dec_sec  ;
wire                             ilmrom_o_ecc_dec_ded       = syn_irom_ecc_dec_ded  ;
wire [IROM_P_BUS_AW-3:0]         ilmrom_o_ecc_err_addr      = syn_irom_ecc_err_addr ;
wire [31:0]                      ilmrom_o_hrdata            = irom_o_hrdata         ;
wire                             ilmrom_o_hreadyout         = irom_o_hreadyout      ;
wire [1:0]                       ilmrom_o_hresp             = irom_o_hresp          ;
wire                             ilmrom_o_rom_cs            = irom_o_rom_cs         ;
wire [IROM_P_BUS_AW-3:0]         ilmrom_o_rom_addr          = irom_o_rom_addr       ;
`endif

wire           cgu_o_sclk               ;
wire           cgu_o_gclk               ;
wire           cgu_o_dbg_toggle_a       ;
wire           cgu_o_mtime_toggle_a     ;
wire           cgu_o_cpu_clk            ;
wire           cgu_o_sys_clk            ;
wire           cgu_o_emu_clk            ;
wire           cgu_o_boot_clk           ;
wire           cgu_o_kmu_clk            ;
wire           cgu_o_kmuram_clk         ;
wire           cgu_o_mbox_clk           ;
wire           cgu_o_trng_clk           ;
wire           cgu_o_hash0_clk          ;
wire           cgu_o_hash0_dma_clk      ;
wire           cgu_o_ske0_clk           ;
wire           cgu_o_ske0_dma_clk       ;
wire           cgu_o_pke_clk            ;
wire           cgu_o_ahb_dma_clk        ;
wire           cgu_o_axi_dma_clk        ;
wire           cgu_o_irom_clk           ;
wire           cgu_o_crc_clk            ;
wire           cgu_o_rdc_clk            ;
wire           cgu_o_wdt_clk            ;
wire           cgu_o_uart_clk           ;
wire           cgu_o_tim_clk            ;

wire           rgu_o_srst_n                 ;
wire           rgu_o_sgrst_n                ;
wire           rgu_o_hwrst_n                ;
wire           rgu_o_cpu_rst_n              ;
wire           rgu_o_dm_ndmreset_n          ;
wire           rgu_o_sys_rst_n              ;
wire           rgu_o_emu_rst_n              ;
wire           rgu_o_boot_rst_n             ;
wire           rgu_o_kmu_rst_n              ;
wire           rgu_o_kmuram_rst_n           ;
wire           rgu_o_mbox_rst_n             ;
wire           rgu_o_trng_rst_n             ;
wire           rgu_o_hash0_rst_n            ;
wire           rgu_o_hash0_dma_rst_n        ;
wire           rgu_o_ske0_rst_n             ;
wire           rgu_o_ske0_dma_rst_n         ;
wire           rgu_o_pke_rst_n              ;
wire           rgu_o_ahb_dma_rst_n          ;
wire           rgu_o_axi_dma_rst_n          ;
wire           rgu_o_irom_rst_n             ;
wire           rgu_o_crc_rst_n              ;
wire           rgu_o_rdc_rst_n              ;
wire           rgu_o_wdt_rst_n              ;
wire           rgu_o_uart_rst_n             ;
wire           rgu_o_tim_rst_n              ;
wire           rgu_o_cpu_boot_rst_n         ;


wire                                cpu_o_dm_ndmreset            ;
wire                                cpu_o_tdo_oe                 ;
wire                                cpu_o_tdo                    ;
wire [1:0]                          cpu_o_ilm_htrans             ;
wire                                cpu_o_ilm_hwrite             ;
wire                                cpu_o_ilm_hmastlock          ;
wire [35:0]                         cpu_o_ilm_hwdata             ;
wire [31:0]                         cpu_o_ilm_haddr              ;
wire [2:0]                          cpu_o_ilm_hsize              ;
wire [2:0]                          cpu_o_ilm_hburst             ;
wire [3:0]                          cpu_o_ilm_hprot              ;
wire [7:0]                          cpu_o_ilm_hmaster            ;
wire                                cpu_o_dlm_hsel               ;
wire [1:0]                          cpu_o_dlm_htrans             ;
wire                                cpu_o_dlm_hwrite             ;
wire [31:0]                         cpu_o_dlm_haddr              ;
wire [2:0]                          cpu_o_dlm_hsize              ;
wire [2:0]                          cpu_o_dlm_hburst             ;
wire                                cpu_o_dlm_hmastlock          ;
wire [35:0]                         cpu_o_dlm_hwdata             ;
wire [3:0]                          cpu_o_dlm_hprot              ;
wire [7:0]                          cpu_o_dlm_hmaster            ;
wire                                cpu_o_hsel                   ;
wire [1:0]                          cpu_o_htrans                 ;
wire                                cpu_o_hwrite                 ;
wire [31:0]                         cpu_o_haddr                  ;
wire [2:0]                          cpu_o_hsize                  ;
wire [2:0]                          cpu_o_hburst                 ;
wire                                cpu_o_hmastlock              ;
wire [35:0]                         cpu_o_hwdata                 ;
wire [3:0]                          cpu_o_hprot                  ;
wire [7:0]                          cpu_o_master                 ;
wire                                cpu_o_dram_ecc_dec_sec       ;
wire                                cpu_o_dram_ecc_dec_ded       ;
wire [14-1:0]    cpu_o_dram_ecc_err_addr      ;
wire                                cpu_o_dram_web               ;
wire [38:0]                         cpu_o_dram_wdata             ;
wire [14-1:0]    cpu_o_dram_addr              ;
wire                                cpu_o_dram_csb               ;
wire                                cpu_o_hart_halted            ;
wire                                cpu_o_wfi                    ;
wire                                cpu_o_ipre_set_bus_pr_alarm  ;
wire                                cpu_o_dpre_set_bus_pr_alarm  ;
wire                                cpu_o_spre_set_bus_pr_alarm  ;
wire                                cpu_o_mem_init_done          ;

wire                      ahbsyn_o_hreadyouts = 1'h1;
wire                      ahbsyn_o_hresps     = 1'h0;
wire [AHBSYN_P_DW-1:0]    ahbsyn_o_hrdatas    = {AHBSYN_P_DW{1'b0}};
wire [AHBSYN_P_AW-1:0]    ahbsyn_o_haddrm     = {AHBSYN_P_AW{1'b0}};
wire [1:0]                ahbsyn_o_htransm    = 2'h0;
wire [2:0]                ahbsyn_o_hsizem     = 3'h0;
wire                      ahbsyn_o_hwritem    = 1'h0;
wire [3:0]                ahbsyn_o_hprotm     = 4'h0;
wire [AHBSYN_P_MW-1:0]    ahbsyn_o_hmasterm   = {AHBSYN_P_MW{1'b0}};
wire                      ahbsyn_o_hmastlockm = 1'h0;
wire [AHBSYN_P_DW-1:0]    ahbsyn_o_hwdatam    = {AHBSYN_P_DW{1'b0}};
wire [2:0]                ahbsyn_o_hburstm    = 3'h0;

wire [35:0]    mtx_o_hrdatas0    ;
wire           mtx_o_hreadyouts0 ;
wire [1:0]     mtx_o_hresps0     ;
wire [35:0]    mtx_o_hrdatas1    ;
wire           mtx_o_hreadyouts1 ;
wire [1:0]     mtx_o_hresps1     ;
wire [35:0]    mtx_o_hrdatas2    ;
wire           mtx_o_hreadyouts2 ;
wire [1:0]     mtx_o_hresps2     ;
wire [35:0]    mtx_o_hrdatas3    ;
wire           mtx_o_hreadyouts3 ;
wire [1:0]     mtx_o_hresps3     ;
wire [35:0]    mtx_o_hrdatas4    ;
wire           mtx_o_hreadyouts4 ;
wire [1:0]     mtx_o_hresps4     ;
wire           mtx_o_hselm0      ;
wire [31:0]    mtx_o_haddrm0     ;
wire [1:0]     mtx_o_htransm0    ;
wire           mtx_o_hwritem0    ;
wire [2:0]     mtx_o_hsizem0     ;
wire [2:0]     mtx_o_hburstm0    ;
wire [3:0]     mtx_o_hprotm0     ;
wire [7:0]     mtx_o_hmasterm0   ;
wire [35:0]    mtx_o_hwdatam0    ;
wire           mtx_o_hmastlockm0 ;
wire           mtx_o_hreadymuxm0 ;
wire           mtx_o_hselm1      ;
wire [31:0]    mtx_o_haddrm1     ;
wire [1:0]     mtx_o_htransm1    ;
wire           mtx_o_hwritem1    ;
wire [2:0]     mtx_o_hsizem1     ;
wire [2:0]     mtx_o_hburstm1    ;
wire [3:0]     mtx_o_hprotm1     ;
wire [7:0]     mtx_o_hmasterm1   ;
wire [35:0]    mtx_o_hwdatam1    ;
wire           mtx_o_hmastlockm1 ;
wire           mtx_o_hreadymuxm1 ;
wire           mtx_o_hselm2      ;
wire [31:0]    mtx_o_haddrm2     ;
wire [1:0]     mtx_o_htransm2    ;
wire           mtx_o_hwritem2    ;
wire [2:0]     mtx_o_hsizem2     ;
wire [2:0]     mtx_o_hburstm2    ;
wire [3:0]     mtx_o_hprotm2     ;
wire [7:0]     mtx_o_hmasterm2   ;
wire [35:0]    mtx_o_hwdatam2    ;
wire           mtx_o_hmastlockm2 ;
wire           mtx_o_hreadymuxm2 ;
wire           mtx_o_hselm3      ;
wire [31:0]    mtx_o_haddrm3     ;
wire [1:0]     mtx_o_htransm3    ;
wire           mtx_o_hwritem3    ;
wire [2:0]     mtx_o_hsizem3     ;
wire [2:0]     mtx_o_hburstm3    ;
wire [3:0]     mtx_o_hprotm3     ;
wire [7:0]     mtx_o_hmasterm3   ;
wire [35:0]    mtx_o_hwdatam3    ;
wire           mtx_o_hmastlockm3 ;
wire           mtx_o_hreadymuxm3 ;
wire           mtx_o_hselm4      ;
wire [31:0]    mtx_o_haddrm4     ;
wire [1:0]     mtx_o_htransm4    ;
wire           mtx_o_hwritem4    ;
wire [2:0]     mtx_o_hsizem4     ;
wire [2:0]     mtx_o_hburstm4    ;
wire [3:0]     mtx_o_hprotm4     ;
wire [7:0]     mtx_o_hmasterm4   ;
wire [35:0]    mtx_o_hwdatam4    ;
wire           mtx_o_hmastlockm4 ;
wire           mtx_o_hreadymuxm4 ;
wire           mtx_o_hselm5      ;
wire [31:0]    mtx_o_haddrm5     ;
wire [1:0]     mtx_o_htransm5    ;
wire           mtx_o_hwritem5    ;
wire [2:0]     mtx_o_hsizem5     ;
wire [2:0]     mtx_o_hburstm5    ;
wire [3:0]     mtx_o_hprotm5     ;
wire [7:0]     mtx_o_hmasterm5   ;
wire [35:0]    mtx_o_hwdatam5    ;
wire           mtx_o_hmastlockm5 ;
wire           mtx_o_hreadymuxm5 ;

wire                                ahb_o_clock_en                    ;
wire                                ahb_o_reset_trng_n                ;
wire                                ahb_o_keep_logic                  ;
wire [31:0]                         ahb_o_m_hrdata                    ;
wire [1:0]                          ahb_o_m_hresp                     ;
wire                                ahb_o_m_hreadyout                 ;
wire                                ahb_o_otp_hsel                    ;
wire [31:0]                         ahb_o_otp_haddr                   ;
wire [1:0]                          ahb_o_otp_htrans                  ;
wire                                ahb_o_otp_hwrite                  ;
wire [2:0]                          ahb_o_otp_hburst                  ;
wire [2:0]                          ahb_o_otp_hsize                   ;
wire [3:0]                          ahb_o_otp_hprot                   ;
wire                                ahb_o_otp_hmastlock               ;
wire                                ahb_o_otp_hready                  ;
wire [31:0]                         ahb_o_otp_hwdata                  ;
wire [31:0]                         ahb_o_mbox_soc_hrdata             ;
wire [1:0]                          ahb_o_mbox_soc_hresp              ;
wire                                ahb_o_mbox_soc_hreadyout          ;
wire                                ahb_o_cfg_hsel                    ;
wire [31:0]                         ahb_o_cfg_haddr                   ;
wire [1:0]                          ahb_o_cfg_htrans                  ;
wire                                ahb_o_cfg_hwrite                  ;
wire [2:0]                          ahb_o_cfg_hburst                  ;
wire [2:0]                          ahb_o_cfg_hsize                   ;
wire [3:0]                          ahb_o_cfg_hprot                   ;
wire                                ahb_o_cfg_hmastlock               ;
wire                                ahb_o_cfg_hready                  ;
wire [31:0]                         ahb_o_cfg_hwdata                  ;
wire [3:0]                          ahb_o_dma_m_awid                  ;
wire [AHB_P_DMA_AW-1:0]             ahb_o_dma_m_awaddr                ;
wire [7:0]                          ahb_o_dma_m_awlen                 ;
wire [2:0]                          ahb_o_dma_m_awsize                ;
wire [1:0]                          ahb_o_dma_m_awburst               ;
wire [1:0]                          ahb_o_dma_m_awlock                ;
wire [3:0]                          ahb_o_dma_m_awcache               ;
wire [2:0]                          ahb_o_dma_m_awprot                ;
wire [3:0]                          ahb_o_dma_m_awqos                 ;
wire [3:0]                          ahb_o_dma_m_awregion              ;
wire                                ahb_o_dma_m_awvalid               ;
wire [3:0]                          ahb_o_dma_m_wid                   ;
wire [AHB_P_DMA_DW-1:0]             ahb_o_dma_m_wdata                 ;
wire [AHB_P_DMA_DW/8-1:0]           ahb_o_dma_m_wstrb                 ;
wire                                ahb_o_dma_m_wlast                 ;
wire                                ahb_o_dma_m_wvalid                ;
wire                                ahb_o_dma_m_bready                ;
wire [3:0]                          ahb_o_dma_m_arid                  ;
wire [AHB_P_DMA_AW-1:0]             ahb_o_dma_m_araddr                ;
wire [7:0]                          ahb_o_dma_m_arlen                 ;
wire [2:0]                          ahb_o_dma_m_arsize                ;
wire [1:0]                          ahb_o_dma_m_arburst               ;
wire [1:0]                          ahb_o_dma_m_arlock                ;
wire [3:0]                          ahb_o_dma_m_arcache               ;
wire [2:0]                          ahb_o_dma_m_arprot                ;
wire [3:0]                          ahb_o_dma_m_arqos                 ;
wire [3:0]                          ahb_o_dma_m_arregion              ;
wire                                ahb_o_dma_m_arvalid               ;
wire                                ahb_o_dma_m_rready                ;
wire [1:0]                          ahb_o_dma_m_awbar                 ;
wire [2:0]                          ahb_o_dma_m_awsnoop               ;
wire [1:0]                          ahb_o_dma_m_awdomain              ;
wire [1:0]                          ahb_o_dma_m_arbar                 ;
wire [3:0]                          ahb_o_dma_m_arsnoop               ;
wire [1:0]                          ahb_o_dma_m_ardomain              ;
wire                                ahb_o_dma_m_hsel                  ;
wire [32-1:0]                       ahb_o_dma_m_haddr                 ;
wire [1:0]                          ahb_o_dma_m_htrans                ;
wire                                ahb_o_dma_m_hwrite                ;
wire [2:0]                          ahb_o_dma_m_hburst                ;
wire [2:0]                          ahb_o_dma_m_hsize                 ;
wire [3:0]                          ahb_o_dma_m_hprot                 ;
wire                                ahb_o_dma_m_hmastlock             ;
wire [32-1:0]                       ahb_o_dma_m_hwdata                ;
wire                                ahb_o_kbuf_CSB                    ;
wire [7:0]                          ahb_o_kbuf_A                      ;
wire [3:0]                          ahb_o_kbuf_WEB                    ;
wire [31:0]                         ahb_o_kbuf_DI                     ;
wire                                ahb_o_pke_sram0_CK                ;
wire                                ahb_o_pke_sram0_CSAN              ;
wire [8:0]                          ahb_o_pke_sram0_A                 ;
wire                                ahb_o_pke_sram0_CSBN              ;
wire [8:0]                          ahb_o_pke_sram0_B                 ;
wire                                ahb_o_pke_sram1_CK                ;
wire                                ahb_o_pke_sram1_CSAN              ;
wire [8:0]                          ahb_o_pke_sram1_A                 ;
wire                                ahb_o_pke_sram1_CSBN              ;
wire [8:0]                          ahb_o_pke_sram1_B                 ;
wire                                ahb_o_pke_sram2_CK                ;
wire                                ahb_o_pke_sram2_CSAN              ;
wire [8:0]                          ahb_o_pke_sram2_A                 ;
wire                                ahb_o_pke_sram2_CSBN              ;
wire [8:0]                          ahb_o_pke_sram2_B                 ;
wire                                ahb_o_pke_sram3_CK                ;
wire                                ahb_o_pke_sram3_CSAN              ;
wire [8:0]                          ahb_o_pke_sram3_A                 ;
wire                                ahb_o_pke_sram3_CSBN              ;
wire [8:0]                          ahb_o_pke_sram3_B                 ;
wire [71:0]                         ahb_o_pke_sram0_DI                ;
wire [71:0]                         ahb_o_pke_sram1_DI                ;
wire [71:0]                         ahb_o_pke_sram2_DI                ;
wire [71:0]                         ahb_o_pke_sram3_DI                ;
wire [31:0]                         ahb_o_clk_ctrl0                   ;
wire [31:0]                         ahb_o_clk_ctrl1                   ;
wire [31:0]                         ahb_o_alg_clk_ctrl0               ;
wire [31:0]                         ahb_o_alg_clk_ctrl1               ;
wire [31:0]                         ahb_o_alg_clk_ctrl2               ;
wire [31:0]                         ahb_o_alg_clk_ctrl3               ;
wire [31:0]                         ahb_o_alg_clk_ctrl4               ;
wire [31:0]                         ahb_o_alg_clk_ctrl5               ;
wire [31:0]                         ahb_o_alg_clk_ctrl6               ;
wire [31:0]                         ahb_o_rst_ctrl0                   ;
wire [31:0]                         ahb_o_rst_ctrl1                   ;
wire [31:0]                         ahb_o_alg_rst_ctrl0               ;
wire [31:0]                         ahb_o_alg_rst_ctrl1               ;
wire [31:0]                         ahb_o_alg_rst_ctrl2               ;
wire [31:0]                         ahb_o_alg_rst_ctrl3               ;
wire [31:0]                         ahb_o_alg_rst_ctrl4               ;
wire [31:0]                         ahb_o_alg_rst_ctrl5               ;
wire [31:0]                         ahb_o_alg_rst_ctrl6               ;
wire [63:0]                         ahb_o_hsm_status                  ;
wire                                ahb_o_ahb_dma_en                  ;
wire                                ahb_o_boot_hw_ok                  ;
wire                                ahb_o_boot_hw_err                 ;
wire                                ahb_o_otp_ahb_if_sel              ;
wire                                ahb_o_ahb_nvm_rsp_err_en          ;
wire                                ahb_o_ahb_cfg_rsp_err_en          ;
wire                                ahb_o_ahb_otp_rsp_err_en          ;
wire                                ahb_o_ahb_soc_rsp_err_en          ;
wire [15:0]                         ahb_o_mbox_soc_irq                ;
wire [63:0]                         ahb_o_mbox_ram_ba                 ;
wire                                ahb_o_uart_txd_oe                 ;
wire                                ahb_o_uart_txd                    ;
wire [63:0]                         ahb_o_irq2cpu                     ;
wire [31:0]                         ahb_o_emu_err_sensor              ;
wire [63:0]                         ahb_o_emu_err_hw                  ;
wire [63:0]                         ahb_o_emu_err_fw                  ;
wire                                ahb_o_emu_resetn_all              ;
wire                                ahb_o_emu_resetn_noboot           ;
wire                                ahb_o_emu_resetn_cpu              ;
wire [3:0]                          ahb_o_trng_ro_clk                 ;
wire [3:0]                          ahb_o_trng_ro_out                 ;
wire                                ahb_o_bus_cipher_en               ;
wire [31:0]                         ahb_o_bus_cipher_key              ;
wire [31:0]                         ahb_o_bus_cipher_nce              ;
wire                                ahb_o_ram_cipher_en               ;
wire [31:0]                         ahb_o_ram_cipher_key              ;
wire [31:0]                         ahb_o_ram_cipher_nce              ;
wire                                ahb_o_nvm_cipher_bypass           ;
wire [31:0]                         ahb_o_irom_ecc_cfg                ;
wire [31:0]                         ahb_o_iram_ecc_cfg                ;
wire [31:0]                         ahb_o_dram_ecc_cfg                ;
wire [31:0]                         ahb_o_kmu_ram_ecc_cfg             ;
wire [31:0]                         ahb_o_cpu_cfg                     ;
wire                                ahb_o_ahb_bus_pr_en               ;
wire                                ahb_o_trng_rdy                    ;
wire                                ahb_o_trng_alarm                  ;
wire                                ahb_o_patch_en                    ;
wire                                ahb_o_patch_info_vld              ;
wire [11:0]                         ahb_o_patch_info_addr             ;
wire [31:0]                         ahb_o_patch_otp_rdata             ;
wire [127:0]                        ahb_o_soc_dbg_en_128b             ;
wire                                ahb_o_hsm_dbg_en                  ;





wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION-1:0]                        nvmhmsync_o_hreadyouts ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION-1:0]                        nvmhmsync_o_hresps     ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*NVMHMSYNC_P_DW-1:0]         nvmhmsync_o_hrdatas    ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*NVMHMSYNC_P_AW-1:0]         nvmhmsync_o_haddrm     ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*(1+1)-1:0]                  nvmhmsync_o_htransm    ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                  nvmhmsync_o_hsizem     ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION-1:0]                        nvmhmsync_o_hwritem    ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*(3+1)-1:0]                  nvmhmsync_o_hprotm     ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*NVMHMSYNC_P_MW-1:0]         nvmhmsync_o_hmasterm   ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION-1:0]                        nvmhmsync_o_hmastlockm ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*NVMHMSYNC_P_DW-1:0]         nvmhmsync_o_hwdatam    ;
wire [A1OSR_NVM_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                  nvmhmsync_o_hburstm    ;

wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION-1:0]                        sochmsync_o_hreadyouts ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION-1:0]                        sochmsync_o_hresps     ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*SOCHMSYNC_P_DW-1:0]         sochmsync_o_hrdatas    ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*SOCHMSYNC_P_AW-1:0]         sochmsync_o_haddrm     ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*(1+1)-1:0]                  sochmsync_o_htransm    ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                  sochmsync_o_hsizem     ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION-1:0]                        sochmsync_o_hwritem    ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*(3+1)-1:0]                  sochmsync_o_hprotm     ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*SOCHMSYNC_P_MW-1:0]         sochmsync_o_hmasterm   ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION-1:0]                        sochmsync_o_hmastlockm ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*SOCHMSYNC_P_DW-1:0]         sochmsync_o_hwdatam    ;
wire [A1OSR_SOC_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                  sochmsync_o_hburstm    ;

wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION-1:0]                        otphmsync_o_hreadyouts ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION-1:0]                        otphmsync_o_hresps     ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*OTPHMSYNC_P_DW-1:0]         otphmsync_o_hrdatas    ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*OTPHMSYNC_P_AW-1:0]         otphmsync_o_haddrm     ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*(1+1)-1:0]                  otphmsync_o_htransm    ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                  otphmsync_o_hsizem     ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION-1:0]                        otphmsync_o_hwritem    ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*(3+1)-1:0]                  otphmsync_o_hprotm     ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*OTPHMSYNC_P_MW-1:0]         otphmsync_o_hmasterm   ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION-1:0]                        otphmsync_o_hmastlockm ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*OTPHMSYNC_P_DW-1:0]         otphmsync_o_hwdatam    ;
wire [A1OSR_OTP_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                  otphmsync_o_hburstm    ;

wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION-1:0]                         abushmsync_o_hreadyouts ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION-1:0]                         abushmsync_o_hresps     ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*ABUSHMSYNC_P_DW-1:0]         abushmsync_o_hrdatas    ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*ABUSHMSYNC_P_AW-1:0]         abushmsync_o_haddrm     ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*(1+1)-1:0]                   abushmsync_o_htransm    ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                   abushmsync_o_hsizem     ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION-1:0]                         abushmsync_o_hwritem    ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*(3+1)-1:0]                   abushmsync_o_hprotm     ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*ABUSHMSYNC_P_MW-1:0]         abushmsync_o_hmasterm   ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION-1:0]                         abushmsync_o_hmastlockm ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*ABUSHMSYNC_P_DW-1:0]         abushmsync_o_hwdatam    ;
wire [A1OSR_ABUS_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                   abushmsync_o_hburstm    ;

wire [A1OSR_AHB_DMA_TIMING_ISOLATION-1:0]                         ahbdmasync_o_hreadyouts ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION-1:0]                         ahbdmasync_o_hresps     ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*AHBDMASYNC_P_DW-1:0]         ahbdmasync_o_hrdatas    ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*AHBDMASYNC_P_AW-1:0]         ahbdmasync_o_haddrm     ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*(1+1)-1:0]                   ahbdmasync_o_htransm    ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*(2+1)-1:0]                   ahbdmasync_o_hsizem     ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION-1:0]                         ahbdmasync_o_hwritem    ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*(3+1)-1:0]                   ahbdmasync_o_hprotm     ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*AHBDMASYNC_P_MW-1:0]         ahbdmasync_o_hmasterm   ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION-1:0]                         ahbdmasync_o_hmastlockm ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*AHBDMASYNC_P_DW-1:0]         ahbdmasync_o_hwdatam    ;
wire [A1OSR_AHB_DMA_TIMING_ISOLATION*(2+1)-1:0]                   ahbdmasync_o_hburstm    ;

wire              x2x_awready_m  = i_dma_m_awready ; 
wire              x2x_wready_m   = i_dma_m_wready  ;
wire              x2x_arready_m  = i_dma_m_arready ;

wire                            iram_o_ecc_dec_sec       ;
wire                            iram_o_ecc_dec_ded       ;
wire [IRAM_P_BUS_AW-3:0]        iram_o_ecc_err_addr      ;
wire [31:0]                     iram_o_hrdata            ;
wire                            iram_o_hreadyout         ;
wire [1:0]                      iram_o_hresp             ;
wire                            iram_o_ram_cs            ;
wire [3:0]                      iram_o_ram_wen           ;
wire [IRAM_P_BUS_AW-3:0]        iram_o_ram_addr          ;
wire [IRAM_P_RAM_DW-1:0]        iram_o_ram_wdata         ;
wire                            iram_o_mem_init_done     ;

wire                            dram_o_ecc_dec_sec       ;
wire                            dram_o_ecc_dec_ded       ;
wire [DRAM_P_BUS_AW-3:0]        dram_o_ecc_err_addr      ;
wire [31:0]                     dram_o_hrdata            ;
wire                            dram_o_hreadyout         ;
wire [1:0]                      dram_o_hresp             ;
wire                            dram_o_ram_cs            ;
wire [3:0]                      dram_o_ram_wen           ;
wire [DRAM_P_BUS_AW-3:0]        dram_o_ram_addr          ;
wire [DRAM_P_RAM_DW-1:0]        dram_o_ram_wdata         ;
wire                            dram_o_mem_init_done     ;

wire                                 kbuf_o_ecc_dec_sec       ;
wire                                 kbuf_o_ecc_dec_ded       ;
wire [KBUF_P_RAM_AW-1:0]             kbuf_o_ecc_err_addr      ;
wire                                 kbuf_o_ecc_dec_sec_nd    ;
wire                                 kbuf_o_ecc_dec_ded_nd    ;
wire [KBUF_P_RAM_DW-1:0]             kbuf_o_p_ram_rdata       ;
wire                                 kbuf_o_c_ram_cs          ;
wire [KBUF_P_RAM_DW/8-1:0]           kbuf_o_c_ram_wen         ;
wire [KBUF_P_RAM_AW-1:0]             kbuf_o_c_ram_addr        ;
wire [KBUF_P_RAM_ECC_DW-1:0]         kbuf_o_c_ram_wdata       ;

wire [31:0]                          nvmabuscipher_o_hrdatas          = nvm_hrdata;
wire                                 nvmabuscipher_o_hreadyouts       = nvm_hreadyout;
wire [1:0]                           nvmabuscipher_o_hresps           = nvm_hresp;
wire [1:0]                           nvmabuscipher_o_htransm          = nvmhmsync_o_htransm;

wire           rh2hm_o_s_hsel     = sochmsync_o_htransm[1];
wire [1:0]     rh2hm_o_s_htrans   = sochmsync_o_htransm   ;
wire [31:0]    rh2hm_o_g_hrdata   = w_m_hrdata          ;
wire [1:0]     rh2hm_o_g_hresp    = w_m_hresp           ;

wire           rh2hotp_o_s_hsel     = otphmsync_o_htransm[1];
wire [1:0]     rh2hotp_o_s_htrans   = otphmsync_o_htransm   ;
wire [31:0]    rh2hotp_o_g_hrdata   = w_otp_hrdata        ;
wire [1:0]     rh2hotp_o_g_hresp    = w_otp_hresp         ;

wire [31:0]    rh2hnvm_o_g_hrdata   = i_nvm_hrdata        ;
wire [1:0]     rh2hnvm_o_g_hresp    = i_nvm_hresp         ;

wire           rh2hcfg_o_s_hsel     = ahb_o_cfg_hsel      ;  
wire [1:0]     rh2hcfg_o_s_htrans   = ahb_o_cfg_htrans    ;  
wire [31:0]    rh2hcfg_o_g_hrdata   = w_cfg_hrdata        ;  
wire [1:0]     rh2hcfg_o_g_hresp    = w_cfg_hresp         ;  

wire        rx2xdma_o_s_awvalid = ahb_o_dma_m_awvalid ;
wire        rx2xdma_o_s_wvalid  = ahb_o_dma_m_wvalid  ;
wire        rx2xdma_o_s_arvalid = ahb_o_dma_m_arvalid ;
wire        rx2xdma_o_s_rready  = ahb_o_dma_m_rready  ;
wire        rx2xdma_o_s_bready  = ahb_o_dma_m_bready  ;
wire        rx2xdma_o_g_awready = x2x_awready_m       ;
wire        rx2xdma_o_g_wready  = x2x_wready_m        ;
wire        rx2xdma_o_g_arready = x2x_arready_m       ;

wire [EXSEN_WIDTH-1:0]    exsen_o_sync_expand = {EXSEN_WIDTH{1'h0}};

wire [EXSTA_WIDTH-1:0]    exsta_o_sync_expand ;

wire [EXERR_WIDTH-1:0]    exerr_o_sync_expand ;


wire [31:0]    otp_ipatch_o_hrdatas         ;
wire           otp_ipatch_o_hreadyouts      ;
wire           otp_ipatch_o_hresps          ;
wire           otp_ipatch_o_hselm           ;
wire [31:0]    otp_ipatch_o_haddrm          ;
wire [1:0]     otp_ipatch_o_htransm         ;
wire           otp_ipatch_o_hwritem         ;
wire [2:0]     otp_ipatch_o_hsizem          ;
wire [2:0]     otp_ipatch_o_hburstm         ;
wire [3:0]     otp_ipatch_o_hprotm          ;
wire [31:0]    otp_ipatch_o_hwdatam         ;
wire           otp_ipatch_o_hmastlockm      ;
wire           otp_ipatch_o_hreadym         ;

wire           ahbdpre_o_set_bus_pr_alarm = 1'b0   ;
wire [7:0]     ahbdpre_o_e2s_hmaster      = 8'b0   ;
wire [3:0]     ahbdpre_o_e2s_parity       = 4'b0   ;

wire [7:0]     s4pre_o_e2s_hmaster      = 8'b0 ;
wire [3:0]     s4pre_o_e2s_parity       = 4'b0 ;

wire           iromprc_o_set_bus_pr_alarm = 1'b0;
wire [35:0]    iromprc_o_c2m_hrdata       = {4'b0,otp_ipatch_o_hrdatas};

wire           iramprc_o_set_bus_pr_alarm = 1'b0;
wire [35:0]    iramprc_o_c2m_hrdata       = {4'b0,iram_o_hrdata};

wire           dramprc_o_set_bus_pr_alarm = 1'b0;
wire [35:0]    dramprc_o_c2m_hrdata       = {4'b0,dram_o_hrdata};

wire           ahbprc_o_set_bus_pr_alarm = 1'b0;
wire [35:0]    ahbprc_o_c2m_hrdata       = {4'b0,abushmsync_o_hrdatas};

wire           nvmprc_o_set_bus_pr_alarm = 1'b0;
wire [35:0]    nvmprc_o_c2m_hrdata       = {4'b0,w_mtx_i_nvm_hrdatam4};

wire           socprc_o_set_bus_pr_alarm = 1'b0;
wire [35:0]    socprc_o_c2m_hrdata       = {4'b0,sochmsync_o_hrdatas};

wire                         romahbsyn_o_hreadyouts = otp_ipatch_o_hreadyouts    ;
wire                         romahbsyn_o_hresps     = otp_ipatch_o_hresps        ;
wire [ROMAHBSYN_P_DW-1:0]    romahbsyn_o_hrdatas    = iromprc_o_c2m_hrdata       ;
wire [ROMAHBSYN_P_AW-1:0]    romahbsyn_o_haddrm     = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_haddr[ROMAHBSYN_P_AW-1:0] : mtx_o_haddrm0[ROMAHBSYN_P_AW-1:0];
wire [1:0]                   romahbsyn_o_htransm    = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_htrans                    : mtx_o_htransm0                   ;
wire [2:0]                   romahbsyn_o_hsizem     = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_hsize                     : mtx_o_hsizem0                    ;
wire                         romahbsyn_o_hwritem    = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_hwrite                    : mtx_o_hwritem0                   ;
wire [3:0]                   romahbsyn_o_hprotm     = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_hprot                     : mtx_o_hprotm0                    ;
wire                         romahbsyn_o_hmastlockm = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_hmastlock                 : mtx_o_hmastlockm0                ;
wire [ROMAHBSYN_P_DW-1:0]    romahbsyn_o_hwdatam    = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_hwdata                    : mtx_o_hwdatam0                   ;
wire [2:0]                   romahbsyn_o_hburstm    = P_IROM_DIRECT_TO_ILM ? cpu_o_ilm_hburst                    : mtx_o_hburstm0                   ;

wire cpu_hart_halted       = cpu_o_hart_halted     ;
wire cpu_wfi               = cpu_o_wfi             ;

wire [33:0]          soc_haddr_h = ahb_o_mbox_ram_ba[63:30] + {33'h0,mtx_o_haddrm5[30]};
wire [63:0]          soc_haddr   = {soc_haddr_h,mtx_o_haddrm5[29:0]} ;

generate if(OSR_OTP_AHB_BUS_TIMING_ISOLATION == 0) begin : no_otphmsync
assign otphmsync_o_hreadyouts = otp_hreadyout                                                   ;
assign otphmsync_o_hresps     = otp_hresp[0]                                                    ;
assign otphmsync_o_hrdatas    = otp_hrdata                                                      ;
assign otphmsync_o_haddrm     = ahb_o_otp_haddr                                                 ;
assign otphmsync_o_htransm    = ahb_o_otp_htrans                                                ;
assign otphmsync_o_hsizem     = ahb_o_otp_hsize                                                 ;
assign otphmsync_o_hwritem    = ahb_o_otp_hwrite                                                ;
assign otphmsync_o_hprotm     = ahb_o_otp_hprot                                                 ;
assign otphmsync_o_hmasterm   = 2'b0                                                            ;
assign otphmsync_o_hmastlockm = ahb_o_otp_hmastlock                                             ;
assign otphmsync_o_hwdatam    = ahb_o_otp_hwdata                                                ;
assign otphmsync_o_hburstm    = ahb_o_otp_hburst                                                ;
assign otphmsync_o_hready     = ahb_o_otp_hready                                                ;
end
else begin
assign otphmsync_o_hready     = w_otp_hreadyout                                                 ;
end
endgenerate

generate if(OSR_NVM_AHB_BUS_TIMING_ISOLATION == 0) begin : no_nvmhmsync
assign nvmhmsync_o_hreadyouts = nvmabuscipher_o_hreadyouts                                      ;
assign nvmhmsync_o_hresps     = nvmabuscipher_o_hresps[0]                                       ;
assign nvmhmsync_o_hrdatas    = nvmabuscipher_o_hrdatas                                         ;
assign nvmhmsync_o_haddrm     = otp_ahb_sel_boot ? ahb_o_otp_haddr  : w_mtx_o_haddrm4           ;
assign nvmhmsync_o_htransm    = otp_ahb_sel_boot ? ahb_o_otp_htrans : mtx_o_htransm4            ;
assign nvmhmsync_o_hsizem     = otp_ahb_sel_boot ? ahb_o_otp_hsize  : mtx_o_hsizem4             ;
assign nvmhmsync_o_hwritem    = otp_ahb_sel_boot ? ahb_o_otp_hwrite : mtx_o_hwritem4            ;
assign nvmhmsync_o_hprotm     = otp_ahb_sel_boot ? ahb_o_otp_hprot  : mtx_o_hprotm4             ;
assign nvmhmsync_o_hmasterm   = 2'b0                                                            ;
assign nvmhmsync_o_hmastlockm = otp_ahb_sel_boot ? ahb_o_otp_hmastlock : mtx_o_hmastlockm4      ;
assign nvmhmsync_o_hwdatam    = otp_ahb_sel_boot ? ahb_o_otp_hwdata : mtx_o_hwdatam4[31:0]      ;
assign nvmhmsync_o_hburstm    = otp_ahb_sel_boot ? ahb_o_otp_hburst : mtx_o_hburstm4            ;
assign nvmhmsync_o_hready     = otp_ahb_sel_boot ? ahb_o_otp_hready : mtx_o_hreadymuxm4         ;
end
else begin
assign nvmhmsync_o_hready     = i_nvm_hreadyout                                                 ;
end
endgenerate

generate if(OSR_SOC_AHB_BUS_TIMING_ISOLATION == 0) begin : no_sochmsync
assign sochmsync_o_hreadyouts = soc_hreadyout                                                   ;
assign sochmsync_o_hresps     = soc_hresp[0]                                                    ;
assign sochmsync_o_hrdatas    = soc_hrdata                                                      ;
assign sochmsync_o_haddrm     = soc_haddr                                                       ;
assign sochmsync_o_htransm    = mtx_o_htransm5                                                  ;
assign sochmsync_o_hsizem     = mtx_o_hsizem5                                                   ;
assign sochmsync_o_hwritem    = mtx_o_hwritem5                                                  ;
assign sochmsync_o_hprotm     = mtx_o_hprotm5                                                   ;
assign sochmsync_o_hmasterm   = {SOCHMSYNC_P_MW{1'b0}}                                          ;
assign sochmsync_o_hmastlockm = mtx_o_hmastlockm5                                               ;
assign sochmsync_o_hwdatam    = mtx_o_hwdatam5[31:0]                                            ;
assign sochmsync_o_hburstm    = mtx_o_hburstm5                                                  ;
assign sochmsync_o_hready     = mtx_o_hreadymuxm5                                               ;
end
else begin
assign sochmsync_o_hready     = w_m_hreadyout                                                   ;
end
endgenerate

generate if(OSR_ABUS_AHB_BUS_TIMING_ISOLATION == 0) begin : no_abushmsync
assign abushmsync_o_hreadyouts = ahb_o_m_hreadyout                                              ;
assign abushmsync_o_hresps     = ahb_o_m_hresp[0]                                               ;
assign abushmsync_o_hrdatas    = ahb_o_m_hrdata                                                 ;
assign abushmsync_o_haddrm     = mtx_o_haddrm3                                                  ;
assign abushmsync_o_htransm    = mtx_o_htransm3                                                 ;
assign abushmsync_o_hsizem     = mtx_o_hsizem3                                                  ;
assign abushmsync_o_hwritem    = mtx_o_hwritem3                                                 ;
assign abushmsync_o_hprotm     = mtx_o_hprotm3                                                  ;
assign abushmsync_o_hmasterm   = 2'b0                                                           ;
assign abushmsync_o_hmastlockm = mtx_o_hmastlockm3                                              ;
assign abushmsync_o_hwdatam    = mtx_o_hwdatam3[31:0]                                           ;
assign abushmsync_o_hburstm    = mtx_o_hburstm3                                                 ;
assign abushmsync_o_hready     = mtx_o_hreadymuxm3                                              ;
end
else begin
assign abushmsync_o_hready     = ahb_o_m_hreadyout                                              ;
end
endgenerate

generate if(OSR_AHB_DMA_TIMING_ISOLATION == 0) begin : no_ahbdmasync
assign ahbdmasync_o_hreadyouts = mtx_o_hreadyouts2                                              ;
assign ahbdmasync_o_hresps     = mtx_o_hresps2[0]                                               ;
assign ahbdmasync_o_hrdatas    = mtx_o_hrdatas2                                                 ;
assign ahbdmasync_o_haddrm     = ahb_o_dma_m_haddr                                              ;
assign ahbdmasync_o_htransm    = ahb_o_dma_m_htrans                                             ;
assign ahbdmasync_o_hsizem     = ahb_o_dma_m_hsize                                              ;
assign ahbdmasync_o_hwritem    = ahb_o_dma_m_hwrite                                             ;
assign ahbdmasync_o_hprotm     = ahb_o_dma_m_hprot                                              ;
assign ahbdmasync_o_hmasterm   = ahbdpre_o_e2s_hmaster                                          ;
assign ahbdmasync_o_hmastlockm = ahb_o_dma_m_hmastlock                                          ;
assign ahbdmasync_o_hwdatam    = {ahbdpre_o_e2s_parity,ahb_o_dma_m_hwdata}                      ;
assign ahbdmasync_o_hburstm    = ahb_o_dma_m_hburst                                             ;
assign ahbdmasync_o_hready     = 1'b1                                                           ;
end
else begin
assign ahbdmasync_o_hready     = mtx_o_hreadyouts2                                              ;
end
endgenerate

wire irom_hready      =  P_IROM_DIRECT_TO_ILM ? 1'h1   :     mtx_o_hreadymuxm0                  ;

assign  w_otp_hrdata    = i_otp_hrdata       ;  
assign  w_otp_hresp     = i_otp_hresp        ;
assign  w_otp_hreadyout = i_otp_hreadyout    ;

assign  w_cfg_hrdata    = i_cfg_hrdata            ;  
assign  w_cfg_hresp     = i_cfg_hresp             ;
assign  w_cfg_hreadyout = i_cfg_hreadyout         ;

assign  w_m_hrdata      = i_m_hrdata       ;
assign  w_m_hresp       = i_m_hresp        ;
assign  w_m_hreadyout   = i_m_hreadyout    ;

assign  w_s_hsel        = i_s_hsel           ;
assign  w_s_haddr       = i_s_haddr          ;
assign  w_s_htrans      = i_s_htrans         ;
assign  w_s_hwrite      = i_s_hwrite         ;
assign  w_s_hsize       = i_s_hsize          ;
assign  w_s_hburst      = i_s_hburst         ;
assign  w_s_hprot       = i_s_hprot          ;
assign  w_s_hwdata      = i_s_hwdata         ;      
assign  w_s_hmastlock   = i_s_hmastlock      ;
assign  w_s_hready      = i_s_hready         ;

assign w_mtx_o_haddrm4 = mtx_o_haddrm4;

wire [ROMCFG_SYN_WIDTH-1:0]    romcfg_syn_o_sync  = ahb_o_irom_ecc_cfg;

`ifdef OSR_FPGA
assign  syn_firom_ecc_dec_sec                     = firom_o_ecc_dec_sec   ; 
assign  syn_firom_ecc_dec_ded                     = firom_o_ecc_dec_ded   ; 
assign  syn_firom_ecc_err_addr                    = firom_o_ecc_err_addr  ; 
`endif

wire           w_cpu_o_dlm_hsel      = cpu_o_dlm_hsel      ;
wire [31:0]    w_cpu_o_dlm_haddr     = cpu_o_dlm_haddr     ;
wire [1:0]     w_cpu_o_dlm_htrans    = cpu_o_dlm_htrans    ;
wire           w_cpu_o_dlm_hwrite    = cpu_o_dlm_hwrite    ;
wire [2:0]     w_cpu_o_dlm_hsize     = cpu_o_dlm_hsize     ;
wire [2:0]     w_cpu_o_dlm_hburst    = cpu_o_dlm_hburst    ;
wire [3:0]     w_cpu_o_dlm_hprot     = cpu_o_dlm_hprot     ;
wire [7:0]     w_cpu_o_dlm_hmaster   = cpu_o_dlm_hmaster   ;
wire [35:0]    w_cpu_o_dlm_hwdata    = cpu_o_dlm_hwdata    ;
wire           w_cpu_o_dlm_hmastlock = cpu_o_dlm_hmastlock ;
wire           w_cpu_o_dlm_hready    = mtx_o_hreadyouts3   ;

wire soc_err_ahb_mem_next = (w_m_hreadyout & (w_m_hresp != 2'b00)) || (!ahb_o_clock_en && soc_err_ahb_mem) ;

wire soc_err_ahb_otp_next = (w_otp_hreadyout & (w_otp_hresp != 2'b00)) || (!ahb_o_clock_en && soc_err_ahb_otp);

wire soc_err_ahb_cfg_next = (w_cfg_hreadyout & (w_cfg_hresp != 2'b00)) || (!ahb_o_clock_en && soc_err_ahb_cfg);

wire soc_err_axi_dma_wr_next = rx2xdma_o_s_bready & i_dma_m_bvalid & (i_dma_m_bresp != 2'b0);
wire soc_err_axi_dma_rd_next = rx2xdma_o_s_rready & i_dma_m_rvalid & (i_dma_m_rresp != 2'b0);

assign otp_ahb_sel_boot = 1'b0; 

assign ahb_dma_m_hsel       = ahbdmasync_o_htransm[1] ;
assign ahb_dma_m_haddr      = ahbdmasync_o_haddrm     ;
assign ahb_dma_m_htrans     = ahbdmasync_o_htransm    ;
assign ahb_dma_m_hwrite     = ahbdmasync_o_hwritem    ;
assign ahb_dma_m_hburst     = ahbdmasync_o_hburstm    ;
assign ahb_dma_m_hsize      = ahbdmasync_o_hsizem     ;
assign ahb_dma_m_hprot      = ahbdmasync_o_hprotm     ;
assign ahb_dma_m_hmastlock  = ahbdmasync_o_hmastlockm ;
assign ahb_dma_m_hwdata     = ahbdmasync_o_hwdatam    ;
assign ahb_dma_m_hmaster    = ahbdmasync_o_hmasterm   ;

wire i_rtc_clk   = 1'b0;
wire i_rtc_rst_n = 1'b0;

assign soc_hrdata              = rh2hm_o_g_hrdata       ;
assign soc_hreadyout           = w_m_hreadyout          ;
assign soc_hresp               = ahb_o_ahb_soc_rsp_err_en ? rh2hm_o_g_hresp : 2'b0      ;

assign otp_hrdata              = rh2hotp_o_g_hrdata     ;
assign otp_hreadyout           = w_otp_hreadyout        ;
assign otp_hresp               = ahb_o_ahb_otp_rsp_err_en ? rh2hotp_o_g_hresp : 2'b0;

assign nvm_hrdata              = rh2hnvm_o_g_hrdata     ;
assign nvm_hreadyout           = i_nvm_hreadyout        ;
assign nvm_hresp               = ahb_o_ahb_nvm_rsp_err_en ? rh2hnvm_o_g_hresp : 2'b0;

assign cfg_hrdata              = rh2hcfg_o_g_hrdata     ;
assign cfg_hreadyout           = w_cfg_hreadyout        ;
assign cfg_hresp               = ahb_o_ahb_cfg_rsp_err_en ? rh2hcfg_o_g_hresp : 2'b0;

wire rom_clk  = cgu_o_irom_clk               ;
wire rom_rstn = rgu_o_irom_rst_n             ;



wire [7:0]     dram_ecc_err_rsp_en        = ahb_o_dram_ecc_cfg[15:8]   ;

wire        mtx_i_cpu_o_ilm_hsel        =    cpu_o_ilm_htrans[1] ;
wire [31:0] mtx_i_cpu_o_ilm_haddr       =    cpu_o_ilm_haddr     ;
wire [1:0]  mtx_i_cpu_o_ilm_htrans      =    cpu_o_ilm_htrans    ;
wire        mtx_i_cpu_o_ilm_hwrite      =    cpu_o_ilm_hwrite    ;
wire [2:0]  mtx_i_cpu_o_ilm_hsize       =    cpu_o_ilm_hsize     ;
wire [2:0]  mtx_i_cpu_o_ilm_hburst      =    cpu_o_ilm_hburst    ;
wire [3:0]  mtx_i_cpu_o_ilm_hprot       =    cpu_o_ilm_hprot     ;
wire [7:0]  mtx_i_cpu_o_ilm_hmaster     =    cpu_o_ilm_hmaster   ;
wire [35:0] mtx_i_cpu_o_ilm_hwdata      =    cpu_o_ilm_hwdata    ;
wire        mtx_i_cpu_o_ilm_hmastlock   =    cpu_o_ilm_hmastlock ;
wire        cpu_i_mtx_o_hreadyouts1     =    mtx_o_hreadyouts1   ;
wire [1:0]  cpu_i_mtx_o_hresps1         =    mtx_o_hresps1       ;
wire [35:0] cpu_i_mtx_o_hrdatas1        =    mtx_o_hrdatas1      ;

assign      w_mtx_i_nvm_hrdatam4      = OTP_AHB_EN ? nvmhmsync_o_hrdatas       : otp_ahb_sel_boot ? 32'h0 : nvmhmsync_o_hrdatas       ;
assign      w_mtx_i_nvm_hreadyoutm4   = OTP_AHB_EN ? nvmhmsync_o_hreadyouts    : otp_ahb_sel_boot ? 1'h1  : nvmhmsync_o_hreadyouts    ;
assign      w_mtx_i_nvm_hrespm4       = OTP_AHB_EN ? {1'b0,nvmhmsync_o_hresps} : otp_ahb_sel_boot ? 2'h0  : {1'b0,nvmhmsync_o_hresps} ;

wire           w_mtx_i_hsels4      = 1'h0                           ;
wire [31:0]    w_mtx_i_haddrs4     = 32'h0                          ;
wire [1:0]     w_mtx_i_htranss4    = 2'h0                           ;
wire           w_mtx_i_hwrites4    = 1'h0                           ;
wire [2:0]     w_mtx_i_hsizes4     = 3'h0                           ;
wire [2:0]     w_mtx_i_hbursts4    = 3'h0                           ;
wire [3:0]     w_mtx_i_hprots4     = 4'h0                           ;
wire [31:0]    w_mtx_i_hwdatas4    = 32'h0                          ;
wire           w_mtx_i_hmastlocks4 = 1'h0                           ;
wire           w_mtx_i_hreadys4    = 1'h1                           ;


wire           cgu_i_rtc_clk            = i_rtc_clk             ;
wire           cgu_i_rtc_rst_n          = i_rtc_rst_n           ;

wire           cgu_i_clk                = i_clk                 ;
wire           cgu_i_rst_n              = i_rst_n               ;
wire           cgu_i_scan_mode          = i_scan_mode           ;

wire           cgu_i_scan_enable        = i_scan_icg_enable     ;

wire           cgu_i_boot_hw_ok         = ahb_o_boot_hw_ok      ;
wire [31:0]    cgu_i_clk_ctrl0          = ahb_o_clk_ctrl0       ;
wire [31:0]    cgu_i_clk_ctrl1          = ahb_o_clk_ctrl1       ;
wire [31:0]    cgu_i_alg_clk_ctrl0      =ahb_o_alg_clk_ctrl0    ; 
wire [31:0]    cgu_i_alg_clk_ctrl1      =ahb_o_alg_clk_ctrl1    ; 
wire [31:0]    cgu_i_alg_clk_ctrl2      =ahb_o_alg_clk_ctrl2    ; 
wire [31:0]    cgu_i_alg_clk_ctrl3      =ahb_o_alg_clk_ctrl3    ; 
wire [31:0]    cgu_i_alg_clk_ctrl4      =ahb_o_alg_clk_ctrl4    ; 
wire [31:0]    cgu_i_alg_clk_ctrl5      =ahb_o_alg_clk_ctrl5    ;
wire [31:0]    cgu_i_alg_clk_ctrl6      =ahb_o_alg_clk_ctrl6    ;
wire           cgu_i_sgrst_n            = rgu_o_sgrst_n         ;

wire           rgu_i_clk                    = i_clk                      ;
wire           rgu_i_rst_n                  = i_rst_n                    ;
wire           rgu_i_scan_mode              = i_scan_mode                ;
wire           rgu_i_rtc_clk                = i_rtc_clk                  ;
wire           rgu_i_rtc_rst_n              = i_rtc_rst_n                ;

wire           rgu_i_reset_trng_n           = ahb_o_reset_trng_n         ;

wire           rgu_i_gclk                   = cgu_o_gclk                 ;
wire           rgu_i_cpu_clk                = cgu_o_cpu_clk              ;
wire           rgu_i_sys_clk                = cgu_o_sys_clk              ;
wire           rgu_i_emu_clk                = cgu_o_emu_clk              ;
wire           rgu_i_boot_clk               = cgu_o_boot_clk             ;

wire           rgu_i_kmu_clk                = cgu_o_kmu_clk              ;

wire           rgu_i_kmuram_clk             = cgu_o_kmuram_clk           ;

wire           rgu_i_mbox_clk               = cgu_o_mbox_clk             ;

wire           rgu_i_trng_clk               = cgu_o_trng_clk             ;

wire           rgu_i_hash0_clk              = cgu_o_hash0_clk           ;
wire           rgu_i_hash0_dma_clk          = cgu_o_hash0_dma_clk       ;

wire           rgu_i_ske0_clk               = cgu_o_ske0_clk             ;
wire           rgu_i_ske0_dma_clk           = cgu_o_ske0_dma_clk         ;

wire           rgu_i_pke_clk                = cgu_o_pke_clk              ;

wire           rgu_i_ahb_dma_clk            = cgu_o_ahb_dma_clk          ;

wire           rgu_i_axi_dma_clk            = cgu_o_axi_dma_clk          ;

wire           rgu_i_irom_clk               = cgu_o_irom_clk             ;

wire           rgu_i_rdc_clk                = cgu_o_rdc_clk              ;
wire           rgu_i_wdt_clk                = cgu_o_wdt_clk              ;

wire           rgu_i_uart_clk               = cgu_o_uart_clk             ;
wire           rgu_i_tim_clk                = cgu_o_tim_clk              ;

wire           rgu_i_crc_clk                = cgu_o_crc_clk              ;

wire [31:0]    rgu_i_rst_ctrl0              = ahb_o_rst_ctrl0            ;
wire [31:0]    rgu_i_rst_ctrl1              = ahb_o_rst_ctrl1            ;
wire [31:0]    rgu_i_alg_rst_ctrl0          = ahb_o_alg_rst_ctrl0        ; 
wire [31:0]    rgu_i_alg_rst_ctrl1          = ahb_o_alg_rst_ctrl1        ; 
wire [31:0]    rgu_i_alg_rst_ctrl2          = ahb_o_alg_rst_ctrl2        ; 
wire [31:0]    rgu_i_alg_rst_ctrl3          = ahb_o_alg_rst_ctrl3        ; 
wire [31:0]    rgu_i_alg_rst_ctrl4          = ahb_o_alg_rst_ctrl4        ; 
wire [31:0]    rgu_i_alg_rst_ctrl5          = ahb_o_alg_rst_ctrl5        ;
wire [31:0]    rgu_i_alg_rst_ctrl6          = ahb_o_alg_rst_ctrl6        ;
wire           rgu_i_boot_hw_ok             = ahb_o_boot_hw_ok           ;
wire           rgu_i_emu_resetn_all         = ahb_o_emu_resetn_all       ;
wire           rgu_i_emu_resetn_noboot      = ahb_o_emu_resetn_noboot    ;
wire           rgu_i_emu_resetn_cpu         = ahb_o_emu_resetn_cpu       ;

wire           rgu_i_dm_ndmreset            = cpu_o_dm_ndmreset          ; 

wire                                cpu_i_clk                    = cgu_o_cpu_clk             ;
wire                                cpu_i_rst_n                  = rgu_o_cpu_rst_n           ;
wire                                cpu_i_keep_logic             = ahb_o_keep_logic          ;

wire                                cpu_i_dbg_stop               = !ahb_o_hsm_dbg_en         ; 
wire                                cpu_i_scan_mode              = i_scan_mode               ;

wire                                cpu_i_scan_enable            = i_scan_icg_enable         ;

wire                                cpu_i_dbg_toggle_a           = cgu_o_dbg_toggle_a        ;
wire                                cpu_i_mtime_toggle_a         = cgu_o_mtime_toggle_a      ;
wire [63:0]                         cpu_i_clic_irq               = ahb_o_irq2cpu             ;

wire                                cpu_i_dm_ndmreset_n          = rgu_o_dm_ndmreset_n       ; 

wire                                cpu_i_trst_n                 = i_trst_n                  ; 
wire                                cpu_i_tck                    = i_tck                     ; 
wire                                cpu_i_tms                    = i_tms                     ; 
wire                                cpu_i_tdi                    = i_tdi                     ; 

wire [35:0]                         cpu_i_ilm_hrdata             = P_IROM_DIRECT_TO_ILM ? romahbsyn_o_hrdatas       : cpu_i_mtx_o_hrdatas1           ;
wire [1:0]                          cpu_i_ilm_hresp              = P_IROM_DIRECT_TO_ILM ? {1'b0,romahbsyn_o_hresps} : cpu_i_mtx_o_hresps1            ;
wire                                cpu_i_ilm_hready             = P_IROM_DIRECT_TO_ILM ? romahbsyn_o_hreadyouts    : cpu_i_mtx_o_hreadyouts1        ;

wire [35:0]                         cpu_i_dlm_hrdata             = mtx_o_hrdatas3          ;
wire [1:0]                          cpu_i_dlm_hresp              = mtx_o_hresps3           ;
wire                                cpu_i_dlm_hready             = mtx_o_hreadyouts3       ;

wire [35:0]                         cpu_i_hrdata                 = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hrdatas       : mtx_o_hrdatas0            ;
wire [1:0]                          cpu_i_hresp                  = P_OSR_CPU_CLK_DIV ? {1'b0,ahbsyn_o_hresps} : mtx_o_hresps0             ;
wire                                cpu_i_hready                 = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hreadyouts    : mtx_o_hreadyouts0         ;
wire                                cpu_i_dram_cipher_en         = ahb_o_ram_cipher_en       ;
wire [7:0]                          cpu_i_dram_cipher_key        = ahb_o_ram_cipher_key[7:0] ;
wire [31:0]                         cpu_i_dram_cipher_dnonce     = ahb_o_ram_cipher_nce      ;
wire [14-1:0]    cpu_i_dram_cipher_anonce     = ahb_o_ram_cipher_nce[14-1:0];
wire [7:0]                          cpu_i_dram_ecc_ck_en         = ahb_o_dram_ecc_cfg[7:0]   ;
wire [7:0]                          cpu_i_dram_ecc_err_rsp_en    = ahb_o_dram_ecc_cfg[15:8]  ;
wire [7:0]                          cpu_i_dram_ecc_tm_en         = ahb_o_dram_ecc_cfg[23:16] ;
wire [6:0]                          cpu_i_dram_ecc_tm_ckbits     = ahb_o_dram_ecc_cfg[30:24] ;

wire [38:0]                         cpu_i_dram_rdata             = i_dram_DO                 ;

wire                                cpu_i_ahb_bus_pr_en          = ahb_o_ahb_bus_pr_en ;
wire                                cpu_i_mem_init_rst_n         = rgu_o_cpu_boot_rst_n;

wire           mtx_i_hclk        = cgu_o_gclk            ;
wire           mtx_i_hresetn     = rgu_o_hwrst_n         ;
wire           mtx_i_cipher_en   = ahb_o_bus_cipher_en   ;
wire [31:0]    mtx_i_key         = ahb_o_bus_cipher_key  ;
wire [31:0]    mtx_i_dat_nounce  = ahb_o_bus_cipher_nce  ;
wire [31:0]    mtx_i_adr_nounce  = ahb_o_bus_cipher_nce  ;
wire           mtx_i_nosec_m0    = 1'b0                  ;
wire [4:0]     mtx_i_masterid_m0 = 5'b0                  ;
wire [7:0]     mtx_i_chid_m0     = 8'b0                  ;
wire           mtx_i_nosec_m1    = 1'b0                  ;
wire [4:0]     mtx_i_masterid_m1 = 5'b0                  ;
wire [7:0]     mtx_i_chid_m1     = 8'b0                  ;
wire           mtx_i_nosec_m2    = 1'b0                  ;
wire [4:0]     mtx_i_masterid_m2 = 5'b0                  ;
wire [7:0]     mtx_i_chid_m2     = 8'b0                  ;
wire           mtx_i_nosec_m3    = 1'b0                  ;
wire [4:0]     mtx_i_masterid_m3 = 5'b0                  ;
wire [7:0]     mtx_i_chid_m3     = 8'b0                  ;
wire           mtx_i_hsels0      = P_OSR_CPU_CLK_DIV ? ahbsyn_o_htransm[1]      : cpu_o_hsel               ;
wire [31:0]    mtx_i_haddrs0     = P_OSR_CPU_CLK_DIV ? ahbsyn_o_haddrm          : cpu_o_haddr              ;
wire [1:0]     mtx_i_htranss0    = P_OSR_CPU_CLK_DIV ? ahbsyn_o_htransm         : cpu_o_htrans             ;
wire           mtx_i_hwrites0    = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hwritem         : cpu_o_hwrite             ;
wire [2:0]     mtx_i_hsizes0     = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hsizem          : cpu_o_hsize              ;
wire [2:0]     mtx_i_hbursts0    = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hburstm         : cpu_o_hburst             ;
wire [3:0]     mtx_i_hprots0     = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hprotm          : cpu_o_hprot              ;
wire [7:0]     mtx_i_hmasters0   = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hmasterm        : cpu_o_master             ;
wire [35:0]    mtx_i_hwdatas0    = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hwdatam         : cpu_o_hwdata             ;
wire           mtx_i_hmastlocks0 = P_OSR_CPU_CLK_DIV ? ahbsyn_o_hmastlockm      : cpu_o_hmastlock          ;
wire           mtx_i_hreadys0    = mtx_o_hreadyouts0        ;
wire           mtx_i_hsels1      = P_IROM_DIRECT_TO_ILM ? 1'h0  : mtx_i_cpu_o_ilm_hsel           ;
wire [31:0]    mtx_i_haddrs1     = P_IROM_DIRECT_TO_ILM ? 32'h0 : mtx_i_cpu_o_ilm_haddr          ;
wire [1:0]     mtx_i_htranss1    = P_IROM_DIRECT_TO_ILM ? 2'h0  : mtx_i_cpu_o_ilm_htrans         ;
wire           mtx_i_hwrites1    = P_IROM_DIRECT_TO_ILM ? 1'h0  : mtx_i_cpu_o_ilm_hwrite         ;
wire [2:0]     mtx_i_hsizes1     = P_IROM_DIRECT_TO_ILM ? 3'h0  : mtx_i_cpu_o_ilm_hsize          ;
wire [2:0]     mtx_i_hbursts1    = P_IROM_DIRECT_TO_ILM ? 3'h0  : mtx_i_cpu_o_ilm_hburst         ;
wire [3:0]     mtx_i_hprots1     = P_IROM_DIRECT_TO_ILM ? 4'h0  : mtx_i_cpu_o_ilm_hprot          ;
wire [7:0]     mtx_i_hmasters1   = P_IROM_DIRECT_TO_ILM ? 8'h0  : mtx_i_cpu_o_ilm_hmaster        ;
wire [35:0]    mtx_i_hwdatas1    = P_IROM_DIRECT_TO_ILM ? 36'h0 : mtx_i_cpu_o_ilm_hwdata         ;
wire           mtx_i_hmastlocks1 = P_IROM_DIRECT_TO_ILM ? 1'h0  : mtx_i_cpu_o_ilm_hmastlock      ;
wire           mtx_i_hreadys1    = P_IROM_DIRECT_TO_ILM ? 1'h1  : mtx_o_hreadyouts1        ;
wire           mtx_i_hsels2      = !ahb_o_ahb_dma_en ? 1'h0  : ahb_dma_m_hsel      ;
wire [31:0]    mtx_i_haddrs2     = !ahb_o_ahb_dma_en ? 32'h0 : ahb_dma_m_haddr     ;
wire [1:0]     mtx_i_htranss2    = !ahb_o_ahb_dma_en ? 2'h0  : ahb_dma_m_htrans    ;
wire           mtx_i_hwrites2    = !ahb_o_ahb_dma_en ? 1'h0  : ahb_dma_m_hwrite    ;
wire [2:0]     mtx_i_hsizes2     = !ahb_o_ahb_dma_en ? 3'h0  : ahb_dma_m_hsize     ;
wire [2:0]     mtx_i_hbursts2    = !ahb_o_ahb_dma_en ? 3'h0  : ahb_dma_m_hburst    ;
wire [3:0]     mtx_i_hprots2     = !ahb_o_ahb_dma_en ? 4'h0  : ahb_dma_m_hprot     ;
wire [7:0]     mtx_i_hmasters2   = !ahb_o_ahb_dma_en ? 8'h0  : ahb_dma_m_hmaster   ;
wire [35:0]    mtx_i_hwdatas2    = !ahb_o_ahb_dma_en ? 36'h0 : ahb_dma_m_hwdata    ;
wire           mtx_i_hmastlocks2 = !ahb_o_ahb_dma_en ? 1'h0  : ahb_dma_m_hmastlock ;
wire           mtx_i_hreadys2    = !ahb_o_ahb_dma_en ? 1'h1  : mtx_o_hreadyouts2   ;
wire           mtx_i_hsels3      = P_DRAM_DIRECT_TO_CPU ? 1'h0  : w_cpu_o_dlm_hsel      ;
wire [31:0]    mtx_i_haddrs3     = P_DRAM_DIRECT_TO_CPU ? 32'h0 : w_cpu_o_dlm_haddr     ;
wire [1:0]     mtx_i_htranss3    = P_DRAM_DIRECT_TO_CPU ? 2'h0  : w_cpu_o_dlm_htrans    ;
wire           mtx_i_hwrites3    = P_DRAM_DIRECT_TO_CPU ? 1'h0  : w_cpu_o_dlm_hwrite    ;
wire [2:0]     mtx_i_hsizes3     = P_DRAM_DIRECT_TO_CPU ? 3'h0  : w_cpu_o_dlm_hsize     ;
wire [2:0]     mtx_i_hbursts3    = P_DRAM_DIRECT_TO_CPU ? 3'h0  : w_cpu_o_dlm_hburst    ;
wire [3:0]     mtx_i_hprots3     = P_DRAM_DIRECT_TO_CPU ? 4'h0  : w_cpu_o_dlm_hprot     ;
wire [7:0]     mtx_i_hmasters3   = P_DRAM_DIRECT_TO_CPU ? 8'h0  : w_cpu_o_dlm_hmaster   ;
wire [35:0]    mtx_i_hwdatas3    = P_DRAM_DIRECT_TO_CPU ? 36'h0 : w_cpu_o_dlm_hwdata    ;
wire           mtx_i_hmastlocks3 = P_DRAM_DIRECT_TO_CPU ? 1'h0  : w_cpu_o_dlm_hmastlock ;
wire           mtx_i_hreadys3    = P_DRAM_DIRECT_TO_CPU ? 1'h1  : w_cpu_o_dlm_hready    ;
wire           mtx_i_hsels4      = w_mtx_i_hsels4      ;
wire [31:0]    mtx_i_haddrs4     = w_mtx_i_haddrs4     ;
wire [1:0]     mtx_i_htranss4    = w_mtx_i_htranss4    ;
wire           mtx_i_hwrites4    = w_mtx_i_hwrites4    ;
wire [2:0]     mtx_i_hsizes4     = w_mtx_i_hsizes4     ;
wire [2:0]     mtx_i_hbursts4    = w_mtx_i_hbursts4    ;
wire [3:0]     mtx_i_hprots4     = w_mtx_i_hprots4     ;
wire [7:0]     mtx_i_hmasters4   = s4pre_o_e2s_hmaster ;
wire [35:0]    mtx_i_hwdatas4    = {s4pre_o_e2s_parity,w_mtx_i_hwdatas4} ;
wire           mtx_i_hmastlocks4 = w_mtx_i_hmastlocks4 ;
wire           mtx_i_hreadys4    = w_mtx_i_hreadys4    ;
wire [35:0]    mtx_i_hrdatam0    = P_IROM_DIRECT_TO_ILM ? 36'h0 : romahbsyn_o_hrdatas       ;
wire           mtx_i_hreadyoutm0 = P_IROM_DIRECT_TO_ILM ? 1'h1  : romahbsyn_o_hreadyouts    ;
wire [1:0]     mtx_i_hrespm0     = P_IROM_DIRECT_TO_ILM ? 2'h0  : {1'b0,romahbsyn_o_hresps} ;
wire [35:0]    mtx_i_hrdatam1    = iramprc_o_c2m_hrdata     ;
wire           mtx_i_hreadyoutm1 = iram_o_hreadyout         ;
wire [1:0]     mtx_i_hrespm1     = iram_o_hresp             ;
wire [35:0]    mtx_i_hrdatam2    = dramprc_o_c2m_hrdata     ;
wire           mtx_i_hreadyoutm2 = dram_o_hreadyout         ;
wire [1:0]     mtx_i_hrespm2     = dram_o_hresp             ;
wire [35:0]    mtx_i_hrdatam3    = ahbprc_o_c2m_hrdata      ;
wire           mtx_i_hreadyoutm3 = abushmsync_o_hreadyouts  ;
wire [1:0]     mtx_i_hrespm3     = {1'b0,abushmsync_o_hresps};
wire [35:0]    mtx_i_hrdatam4    = nvmprc_o_c2m_hrdata      ;
wire           mtx_i_hreadyoutm4 = w_mtx_i_nvm_hreadyoutm4  ;
wire [1:0]     mtx_i_hrespm4     = w_mtx_i_nvm_hrespm4      ;
wire [35:0]    mtx_i_hrdatam5    = socprc_o_c2m_hrdata   ;
wire           mtx_i_hreadyoutm5 = sochmsync_o_hreadyouts;
wire [1:0]     mtx_i_hrespm5     = {1'b0,sochmsync_o_hresps};

wire                                ahb_i_sclk                        = cgu_o_sclk          ;
wire                                ahb_i_gclk                        = cgu_o_gclk          ;
wire                                ahb_i_scan_mode                   = i_scan_mode         ;

wire                                ahb_i_cpu_clk                     = cgu_o_cpu_clk       ;
wire                                ahb_i_sys_clk                     = cgu_o_sys_clk       ;
wire                                ahb_i_emu_clk                     = cgu_o_emu_clk       ;
wire                                ahb_i_boot_clk                    = cgu_o_boot_clk      ;
wire                                ahb_i_kmu_clk                     = cgu_o_kmu_clk       ;
wire                                ahb_i_mbox_clk                    = cgu_o_mbox_clk      ;

wire                                ahb_i_trng_clk                    = cgu_o_trng_clk      ;

wire                                ahb_i_hash0_clk                   = cgu_o_hash0_clk      ;
wire                                ahb_i_hash0_dma_clk               = cgu_o_hash0_dma_clk  ;

wire                                ahb_i_ske0_clk                    = cgu_o_ske0_clk       ;
wire                                ahb_i_ske0_dma_clk                = cgu_o_ske0_dma_clk   ;

wire                                ahb_i_pke_clk                     = cgu_o_pke_clk       ;

wire                                ahb_i_ahb_dma_clk                 = cgu_o_ahb_dma_clk   ;

wire                                ahb_i_axi_dma_clk                 = cgu_o_axi_dma_clk   ;

wire                                ahb_i_rdc_clk                     = cgu_o_rdc_clk       ;
wire                                ahb_i_wdt_clk                     = cgu_o_wdt_clk       ;

wire                                ahb_i_uart_clk                    = cgu_o_uart_clk      ;
wire                                ahb_i_tim_clk                     = cgu_o_tim_clk       ;

wire                                ahb_i_crc_clk                     = cgu_o_crc_clk       ;

wire                                ahb_i_srst_n                      = rgu_o_srst_n        ;
wire                                ahb_i_hwrst_n                     = rgu_o_hwrst_n       ;
wire                                ahb_i_cpu_rst_n                   = rgu_o_cpu_rst_n     ;
wire                                ahb_i_sys_rst_n                   = rgu_o_sys_rst_n     ;
wire                                ahb_i_emu_rst_n                   = rgu_o_emu_rst_n     ;
wire                                ahb_i_boot_rst_n                  = rgu_o_boot_rst_n    ;
wire                                ahb_i_kmu_rst_n                   = rgu_o_kmu_rst_n     ;
wire                                ahb_i_mbox_rst_n                  = rgu_o_mbox_rst_n    ;

wire                                ahb_i_trng_rst_n                  = rgu_o_trng_rst_n    ;

wire                                ahb_i_hash0_rst_n                 = rgu_o_hash0_rst_n    ;
wire                                ahb_i_hash0_dma_rst_n             = rgu_o_hash0_dma_rst_n;

wire                                ahb_i_ske0_rst_n                  = rgu_o_ske0_rst_n     ;
wire                                ahb_i_ske0_dma_rst_n              = rgu_o_ske0_dma_rst_n ;

wire                                ahb_i_pke_rst_n                   = rgu_o_pke_rst_n     ;

wire                                ahb_i_ahb_dma_rst_n               = rgu_o_ahb_dma_rst_n ;

wire                                ahb_i_axi_dma_rst_n               = rgu_o_axi_dma_rst_n ;

wire                                ahb_i_rdc_rst_n                   = rgu_o_rdc_rst_n     ;
wire                                ahb_i_wdt_rst_n                   = rgu_o_wdt_rst_n     ;

wire                                ahb_i_uart_rst_n                  = rgu_o_uart_rst_n    ;
wire                                ahb_i_tim_rst_n                   = rgu_o_tim_rst_n     ;

wire                                ahb_i_crc_rst_n                   = rgu_o_crc_rst_n     ;

wire                                ahb_i_m_hsel                      = abushmsync_o_htransm[1]   ;
wire [31:0]                         ahb_i_m_haddr                     = abushmsync_o_haddrm       ;
wire [1:0]                          ahb_i_m_htrans                    = abushmsync_o_htransm      ;
wire                                ahb_i_m_hwrite                    = abushmsync_o_hwritem      ;
wire [2:0]                          ahb_i_m_hburst                    = abushmsync_o_hburstm      ;
wire [2:0]                          ahb_i_m_hsize                     = abushmsync_o_hsizem       ;
wire [3:0]                          ahb_i_m_hprot                     = abushmsync_o_hprotm       ;
wire                                ahb_i_m_hmastlock                 = abushmsync_o_hmastlockm   ;
wire                                ahb_i_m_hready                    = abushmsync_o_hready       ;
wire [31:0]                         ahb_i_m_hwdata                    = abushmsync_o_hwdatam      ;
wire [31:0]                         ahb_i_otp_hrdata                  = OTP_AHB_EN ? otphmsync_o_hrdatas       : otp_ahb_sel_boot ? nvmhmsync_o_hrdatas        : 32'b0 ;
wire [1:0]                          ahb_i_otp_hresp                   = OTP_AHB_EN ? {1'b0,otphmsync_o_hresps} : otp_ahb_sel_boot ? {1'b0,nvmhmsync_o_hresps}  : 2'b0  ;
wire                                ahb_i_otp_hreadyout               = OTP_AHB_EN ? otphmsync_o_hreadyouts    : otp_ahb_sel_boot ? nvmhmsync_o_hreadyouts     : 1'b1  ;

wire [31:0]                         ahb_i_mbox_soc_sta                = 32'h0          ;
wire                                ahb_i_mbox_soc_hsel               = w_s_hsel       ;
wire [31:0]                         ahb_i_mbox_soc_haddr              = w_s_haddr      ;
wire [1:0]                          ahb_i_mbox_soc_htrans             = w_s_htrans     ;
wire                                ahb_i_mbox_soc_hwrite             = w_s_hwrite     ;
wire [2:0]                          ahb_i_mbox_soc_hburst             = w_s_hburst     ;
wire [2:0]                          ahb_i_mbox_soc_hsize              = w_s_hsize      ;
wire [3:0]                          ahb_i_mbox_soc_hprot              = w_s_hprot      ;
wire                                ahb_i_mbox_soc_hmastlock          = w_s_hmastlock  ;
wire                                ahb_i_mbox_soc_hready             = w_s_hready     ;
wire [31:0]                         ahb_i_mbox_soc_hwdata             = w_s_hwdata     ;

wire [31:0]                         ahb_i_cfg_hrdata                  = cfg_hrdata          ; 
wire [1:0]                          ahb_i_cfg_hresp                   = cfg_hresp           ; 
wire                                ahb_i_cfg_hreadyout               = cfg_hreadyout       ; 

wire                                ahb_i_dma_m_awready               = rx2xdma_o_g_awready ; 
wire                                ahb_i_dma_m_wready                = rx2xdma_o_g_wready  ;
wire [3:0]                          ahb_i_dma_m_bid                   = i_dma_m_bid           ;
wire [1:0]                          ahb_i_dma_m_bresp                 = i_dma_m_bresp         ;
wire                                ahb_i_dma_m_bvalid                = i_dma_m_bvalid        ;
wire                                ahb_i_dma_m_arready               = rx2xdma_o_g_arready ;
wire [3:0]                          ahb_i_dma_m_rid                   = i_dma_m_rid           ;
wire [AHB_P_DMA_DW-1:0]             ahb_i_dma_m_rdata                 = i_dma_m_rdata         ;
wire [1:0]                          ahb_i_dma_m_rresp                 = i_dma_m_rresp         ;
wire                                ahb_i_dma_m_rlast                 = i_dma_m_rlast         ;
wire                                ahb_i_dma_m_rvalid                = i_dma_m_rvalid        ;

wire [32-1:0]                       ahb_i_dma_m_hrdata                = !ahb_o_ahb_dma_en ? 32'b0: ahbdmasync_o_hrdatas[31:0]   ;
wire [1:0]                          ahb_i_dma_m_hresp                 = !ahb_o_ahb_dma_en ? 2'b0 : {1'b0,ahbdmasync_o_hresps}    ;
wire                                ahb_i_dma_m_hreadyout             = !ahb_o_ahb_dma_en ? 1'b1 : ahbdmasync_o_hreadyouts;

wire [31:0]                         ahb_i_kbuf_DO                     = kbuf_o_p_ram_rdata  ;

wire [71:0]                         ahb_i_pke_sram0_DO                = i_pke_sram0_DO  ;
wire [71:0]                         ahb_i_pke_sram1_DO                = i_pke_sram1_DO  ;
wire [71:0]                         ahb_i_pke_sram2_DO                = i_pke_sram2_DO  ;
wire [71:0]                         ahb_i_pke_sram3_DO                = i_pke_sram3_DO  ;

wire [31:0]                         ahb_i_sensor                      = exsen_o_sync_expand ;
wire [31:0]                         ahb_i_soc_status                  = exsta_o_sync_expand ; 
wire [31:0]                         ahb_i_soc_err                     = exerr_o_sync_expand ;

wire                                ahb_i_uart_rxd                    = i_uart_rxd      ;

wire                                ahb_i_mem_ecc_1b_irom             = ilmrom_o_ecc_dec_sec;
wire                                ahb_i_mem_ecc_1b_iram             = iram_o_ecc_dec_sec;
wire                                ahb_i_mem_ecc_1b_dram             = dram_o_ecc_dec_sec;
wire                                ahb_i_mem_ecc_1b_kmu              = kbuf_o_ecc_dec_sec;
wire                                ahb_i_mem_ecc_mb_irom             = ilmrom_o_ecc_dec_ded;
wire                                ahb_i_mem_ecc_mb_iram             = iram_o_ecc_dec_ded;
wire                                ahb_i_mem_ecc_mb_dram             = dram_o_ecc_dec_ded;
wire                                ahb_i_mem_ecc_mb_kmu              = kbuf_o_ecc_dec_ded;
wire [14-1:0]    ahb_i_mem_ecc_addr_irom           = ilmrom_o_ecc_err_addr;
wire [16-1:0]    ahb_i_mem_ecc_addr_iram           = iram_o_ecc_err_addr;
wire [14-1:0]    ahb_i_mem_ecc_addr_dram           = dram_o_ecc_err_addr;
wire [16:0]                         ahb_i_mem_ecc_addr_kmu            = {9'b0,kbuf_o_ecc_err_addr};
wire                                ahb_i_soc_err_ahb_mem             = soc_err_ahb_mem;
wire                                ahb_i_soc_err_ahb_otp             = soc_err_ahb_otp;
wire                                ahb_i_soc_err_ahb_nvm             = soc_err_ahb_nvm;
wire                                ahb_i_soc_err_ahb_cfg             = soc_err_ahb_cfg;
wire                                ahb_i_soc_err_axi_dma_wr          = soc_err_axi_dma_wr | soc_err_axi_dma_wr_r;
wire                                ahb_i_soc_err_axi_dma_rd          = soc_err_axi_dma_rd | soc_err_axi_dma_rd_r;
wire                                ahb_i_ipre_pchk_err               = cpu_o_ipre_set_bus_pr_alarm;
wire                                ahb_i_dpre_pchk_err               = cpu_o_dpre_set_bus_pr_alarm;
wire                                ahb_i_spre_pchk_err               = cpu_o_spre_set_bus_pr_alarm;
wire                                ahb_i_ahbdpre_pchk_err            = ahbdpre_o_set_bus_pr_alarm ;
wire                                ahb_i_iromprc_pchk_err            = iromprc_o_set_bus_pr_alarm ;
wire                                ahb_i_iramprc_pchk_err            = iramprc_o_set_bus_pr_alarm ;
wire                                ahb_i_dramprc_pchk_err            = dramprc_o_set_bus_pr_alarm ;
wire                                ahb_i_ahbprc_pchk_err             = ahbprc_o_set_bus_pr_alarm  ;
wire                                ahb_i_nvmprc_pchk_err             = nvmprc_o_set_bus_pr_alarm  ;
wire                                ahb_i_socprc_pchk_err             = socprc_o_set_bus_pr_alarm  ;

wire                                ahb_i_cpu_hart_halted             = cpu_hart_halted       ;
wire                                ahb_i_cpu_wfi                     = cpu_wfi               ;

wire                                ahb_i_mem_init_done               = dram_o_mem_init_done & iram_o_mem_init_done;

wire                         nvmhmsync_i_hclk       = cgu_o_gclk                                                                ;
wire                         nvmhmsync_i_hresetn    = rgu_o_sgrst_n                                                             ;
wire                         nvmhmsync_i_hsels      = otp_ahb_sel_boot ? ahb_o_otp_hsel   : mtx_o_hselm4                        ;
wire [NVMHMSYNC_P_AW-1:0]    nvmhmsync_i_haddrs     = otp_ahb_sel_boot ? ahb_o_otp_haddr  : w_mtx_o_haddrm4                     ;
wire [1:0]                   nvmhmsync_i_htranss    = otp_ahb_sel_boot ? ahb_o_otp_htrans : mtx_o_htransm4                      ;
wire [2:0]                   nvmhmsync_i_hsizes     = otp_ahb_sel_boot ? ahb_o_otp_hsize  : mtx_o_hsizem4                       ;
wire                         nvmhmsync_i_hwrites    = otp_ahb_sel_boot ? ahb_o_otp_hwrite : mtx_o_hwritem4                      ;
wire                         nvmhmsync_i_hreadys    = otp_ahb_sel_boot ? ahb_o_otp_hready : mtx_o_hreadymuxm4                   ;
wire [3:0]                   nvmhmsync_i_hprots     = otp_ahb_sel_boot ? ahb_o_otp_hprot  : mtx_o_hprotm4                       ;
wire [NVMHMSYNC_P_MW-1:0]    nvmhmsync_i_hmasters   = {NVMHMSYNC_P_MW{1'b0}}                                                    ;
wire                         nvmhmsync_i_hmastlocks = otp_ahb_sel_boot ? ahb_o_otp_hmastlock : mtx_o_hmastlockm4                ;
wire [NVMHMSYNC_P_DW-1:0]    nvmhmsync_i_hwdatas    = otp_ahb_sel_boot ? ahb_o_otp_hwdata : mtx_o_hwdatam4[NVMHMSYNC_P_DW-1:0]  ;
wire [2:0]                   nvmhmsync_i_hbursts    = otp_ahb_sel_boot ? ahb_o_otp_hburst : mtx_o_hburstm4                      ;
wire                         nvmhmsync_i_hreadyoutm = nvmabuscipher_o_hreadyouts ;
wire                         nvmhmsync_i_hresps     = nvmabuscipher_o_hresps[0]  ;
wire [NVMHMSYNC_P_DW-1:0]    nvmhmsync_i_hrdatam    = nvmabuscipher_o_hrdatas    ;

wire                         sochmsync_i_hclk       = cgu_o_gclk                                                      ;
wire                         sochmsync_i_hresetn    = rgu_o_sgrst_n                                                   ;
wire                         sochmsync_i_hsels      = mtx_o_hselm5                                                    ;
wire [SOCHMSYNC_P_AW-1:0]    sochmsync_i_haddrs     = soc_haddr                                                       ;
wire [1:0]                   sochmsync_i_htranss    = mtx_o_htransm5                                                  ;
wire [2:0]                   sochmsync_i_hsizes     = mtx_o_hsizem5                                                   ;
wire                         sochmsync_i_hwrites    = mtx_o_hwritem5                                                  ;
wire                         sochmsync_i_hreadys    = mtx_o_hreadymuxm5                                               ;
wire [3:0]                   sochmsync_i_hprots     = mtx_o_hprotm5                                                   ;
wire [SOCHMSYNC_P_MW-1:0]    sochmsync_i_hmasters   = {SOCHMSYNC_P_MW{1'b0}}                                          ;
wire                         sochmsync_i_hmastlocks = mtx_o_hmastlockm5                                               ;
wire [SOCHMSYNC_P_DW-1:0]    sochmsync_i_hwdatas    = mtx_o_hwdatam5[31:0]                                            ;
wire [2:0]                   sochmsync_i_hbursts    = mtx_o_hburstm5                                                  ;
wire                         sochmsync_i_hreadyoutm = soc_hreadyout                                                   ;
wire                         sochmsync_i_hresps     = soc_hresp[0]                                                    ;
wire [SOCHMSYNC_P_DW-1:0]    sochmsync_i_hrdatam    = soc_hrdata                                                      ;

wire                         otphmsync_i_hclk       = cgu_o_gclk                                                      ;   
wire                         otphmsync_i_hresetn    = rgu_o_sgrst_n                                                   ;   
wire                         otphmsync_i_hsels      = ahb_o_otp_hsel                                                  ;
wire [OTPHMSYNC_P_AW-1:0]    otphmsync_i_haddrs     = ahb_o_otp_haddr                                                 ;
wire [1:0]                   otphmsync_i_htranss    = ahb_o_otp_htrans                                                ;
wire [2:0]                   otphmsync_i_hsizes     = ahb_o_otp_hsize                                                 ;
wire                         otphmsync_i_hwrites    = ahb_o_otp_hwrite                                                ;
wire                         otphmsync_i_hreadys    = ahb_o_otp_hready                                                ;
wire [3:0]                   otphmsync_i_hprots     = ahb_o_otp_hprot                                                 ;
wire [OTPHMSYNC_P_MW-1:0]    otphmsync_i_hmasters   = {OTPHMSYNC_P_MW{1'b0}}                                          ;   
wire                         otphmsync_i_hmastlocks = ahb_o_otp_hmastlock                                             ;   
wire [OTPHMSYNC_P_DW-1:0]    otphmsync_i_hwdatas    = ahb_o_otp_hwdata                                                ;   
wire [2:0]                   otphmsync_i_hbursts    = ahb_o_otp_hburst                                                ;   
wire                         otphmsync_i_hreadyoutm = otp_hreadyout                                                   ; 
wire                         otphmsync_i_hresps     = otp_hresp[0]                                                    ; 
wire [OTPHMSYNC_P_DW-1:0]    otphmsync_i_hrdatam    = otp_hrdata                                                      ; 

wire                          abushmsync_i_hclk       = cgu_o_gclk                                                    ;
wire                          abushmsync_i_hresetn    = rgu_o_sgrst_n                                                 ;
wire                          abushmsync_i_hsels      = mtx_o_hselm3                                                  ;
wire [ABUSHMSYNC_P_AW-1:0]    abushmsync_i_haddrs     = mtx_o_haddrm3                                                 ;
wire [1:0]                    abushmsync_i_htranss    = mtx_o_htransm3                                                ;
wire [2:0]                    abushmsync_i_hsizes     = mtx_o_hsizem3                                                 ;
wire                          abushmsync_i_hwrites    = mtx_o_hwritem3                                                ;
wire                          abushmsync_i_hreadys    = mtx_o_hreadymuxm3                                             ;
wire [3:0]                    abushmsync_i_hprots     = mtx_o_hprotm3                                                 ;
wire [ABUSHMSYNC_P_MW-1:0]    abushmsync_i_hmasters   = {ABUSHMSYNC_P_MW{1'b0}}                                       ;
wire                          abushmsync_i_hmastlocks = mtx_o_hmastlockm3                                             ;
wire [ABUSHMSYNC_P_DW-1:0]    abushmsync_i_hwdatas    = mtx_o_hwdatam3[ABUSHMSYNC_P_DW-1:0]                           ;
wire [2:0]                    abushmsync_i_hbursts    = mtx_o_hburstm3                                                ;
wire                          abushmsync_i_hreadyoutm = ahb_o_m_hreadyout                                             ;
wire                          abushmsync_i_hresps     = ahb_o_m_hresp[0]                                              ;
wire [ABUSHMSYNC_P_DW-1:0]    abushmsync_i_hrdatam    = ahb_o_m_hrdata                                                ;

wire                          ahbdmasync_i_hclk       = cgu_o_gclk                                                    ;
wire                          ahbdmasync_i_hresetn    = rgu_o_sgrst_n                                                 ;
wire                          ahbdmasync_i_hsels      = ahb_o_dma_m_hsel                                              ;
wire [AHBDMASYNC_P_AW-1:0]    ahbdmasync_i_haddrs     = ahb_o_dma_m_haddr                                             ;
wire [1:0]                    ahbdmasync_i_htranss    = ahb_o_dma_m_htrans                                            ;
wire [2:0]                    ahbdmasync_i_hsizes     = ahb_o_dma_m_hsize                                             ;
wire                          ahbdmasync_i_hwrites    = ahb_o_dma_m_hwrite                                            ;
wire                          ahbdmasync_i_hreadys    = ahbdmasync_o_hreadyouts                                       ;
wire [3:0]                    ahbdmasync_i_hprots     = ahb_o_dma_m_hprot                                             ;
wire [AHBDMASYNC_P_MW-1:0]    ahbdmasync_i_hmasters   = ahbdpre_o_e2s_hmaster                                         ;
wire                          ahbdmasync_i_hmastlocks = ahb_o_dma_m_hmastlock                                         ;
wire [AHBDMASYNC_P_DW-1:0]    ahbdmasync_i_hwdatas    = {ahbdpre_o_e2s_parity,ahb_o_dma_m_hwdata}                     ;
wire [2:0]                    ahbdmasync_i_hbursts    = ahb_o_dma_m_hburst                                            ;
wire                          ahbdmasync_i_hreadyoutm = mtx_o_hreadyouts2                                             ;
wire                          ahbdmasync_i_hresps     = mtx_o_hresps2[0]                                              ;
wire [AHBDMASYNC_P_DW-1:0]    ahbdmasync_i_hrdatam    = mtx_o_hrdatas2                                                ;

wire                            iram_i_hclk              = cgu_o_cpu_clk  ;
wire                            iram_i_hresetn           = rgu_o_cpu_rst_n;
wire                            iram_i_ram_cipher_en     = ahb_o_ram_cipher_en;
wire [7:0]                      iram_i_ram_cipher_key    = ahb_o_ram_cipher_key[23:16];
wire [IRAM_P_BUS_DW-1:0]        iram_i_ram_cipher_dnonce = ahb_o_ram_cipher_nce[IRAM_P_BUS_DW-1:0];
wire [IRAM_P_BUS_AW-3:0]        iram_i_ram_cipher_anonce = ahb_o_ram_cipher_nce[IRAM_P_BUS_AW-3:0];
wire [7:0]                      iram_i_ecc_ck_en         = ahb_o_iram_ecc_cfg[7:0];
wire [7:0]                      iram_i_ecc_err_rsp_en    = ahb_o_iram_ecc_cfg[15:8];
wire [7:0]                      iram_i_ecc_tm_en         = ahb_o_iram_ecc_cfg[23:16];
wire [IRAM_P_ECC_TM_CHB-1:0]    iram_i_ecc_tm_ckbits     = ahb_o_iram_ecc_cfg[23+IRAM_P_ECC_TM_CHB:24];
wire                            iram_i_hsel              = mtx_o_hselm1;
wire                            iram_i_hready            = mtx_o_hreadymuxm1;
wire [IRAM_P_BUS_AW-1:0]        iram_i_haddr             = mtx_o_haddrm1[IRAM_P_BUS_AW-1:0];
wire [1:0]                      iram_i_htrans            = mtx_o_htransm1;
wire                            iram_i_hwrite            = mtx_o_hwritem1;
wire [2:0]                      iram_i_hsize             = mtx_o_hsizem1;
wire [2:0]                      iram_i_hburst            = mtx_o_hburstm1;
wire [3:0]                      iram_i_hprot             = mtx_o_hprotm1;
wire [3:0]                      iram_i_hmaster           = 4'b0;
wire [31:0]                     iram_i_hwdata            = mtx_o_hwdatam1[31:0];
wire                            iram_i_hmastlock         = mtx_o_hmastlockm1;
wire [IRAM_P_RAM_DW-1:0]        iram_i_ram_rdata         = i_iram_DO;
wire                            iram_i_mem_init_rst_n    = rgu_o_cpu_boot_rst_n;

wire                            dram_i_hclk              = cgu_o_cpu_clk  ;
wire                            dram_i_hresetn           = rgu_o_cpu_rst_n;
wire                            dram_i_ram_cipher_en     = ahb_o_ram_cipher_en;
wire [7:0]                      dram_i_ram_cipher_key    = ahb_o_ram_cipher_key[7:0];
wire [DRAM_P_BUS_DW-1:0]        dram_i_ram_cipher_dnonce = ahb_o_ram_cipher_nce[DRAM_P_BUS_DW-1:0];
wire [DRAM_P_BUS_AW-3:0]        dram_i_ram_cipher_anonce = ahb_o_ram_cipher_nce[DRAM_P_BUS_AW-3:0];
wire [7:0]                      dram_i_ecc_ck_en         = ahb_o_dram_ecc_cfg[7:0];   
wire [7:0]                      dram_i_ecc_err_rsp_en    = dram_ecc_err_rsp_en;
wire [7:0]                      dram_i_ecc_tm_en         = ahb_o_dram_ecc_cfg[23:16];
wire [DRAM_P_ECC_TM_CHB-1:0]    dram_i_ecc_tm_ckbits     = ahb_o_dram_ecc_cfg[30:24];
wire                            dram_i_hsel              = mtx_o_hselm2;
wire                            dram_i_hready            = mtx_o_hreadymuxm2;
wire [DRAM_P_BUS_AW-1:0]        dram_i_haddr             = mtx_o_haddrm2[DRAM_P_BUS_AW-1:0];
wire [1:0]                      dram_i_htrans            = mtx_o_htransm2;
wire                            dram_i_hwrite            = mtx_o_hwritem2;
wire [2:0]                      dram_i_hsize             = mtx_o_hsizem2;
wire [2:0]                      dram_i_hburst            = mtx_o_hburstm2;
wire [3:0]                      dram_i_hprot             = mtx_o_hprotm2;
wire [3:0]                      dram_i_hmaster           = 4'b0;
wire [31:0]                     dram_i_hwdata            = mtx_o_hwdatam2[31:0];
wire                            dram_i_hmastlock         = mtx_o_hmastlockm2;
wire [DRAM_P_RAM_DW-1:0]        dram_i_ram_rdata         = i_dram_DO;
wire                            dram_i_mem_init_rst_n    = rgu_o_cpu_boot_rst_n;

wire                                 kbuf_i_hclk              = cgu_o_kmuram_clk;
wire                                 kbuf_i_hresetn           = rgu_o_kmuram_rst_n;
wire                                 kbuf_i_ram_cipher_en     = ahb_o_ram_cipher_en;
wire [KBUF_P_RAM_CIPHER_RN*4-1:0]    kbuf_i_ram_cipher_key    = ahb_o_ram_cipher_key[31:24];
wire [KBUF_P_RAM_DW-1:0]             kbuf_i_ram_cipher_dnonce = ahb_o_ram_cipher_nce;
wire [KBUF_P_RAM_AW-1:0]             kbuf_i_ram_cipher_anonce = ahb_o_ram_cipher_nce[7:0];
wire [7:0]                           kbuf_i_ecc_ck_en         = ahb_o_kmu_ram_ecc_cfg[7:0];
wire [7:0]                           kbuf_i_ecc_err_rsp_en    = 8'h5a;
wire [7:0]                           kbuf_i_ecc_tm_en         = ahb_o_kmu_ram_ecc_cfg[15:8];
wire [KBUF_P_RAM_ECC_TM_CHB-1:0]     kbuf_i_ecc_tm_ckbits     = ahb_o_kmu_ram_ecc_cfg[22:16];
wire                                 kbuf_i_p_ram_cs          = ~ahb_o_kbuf_CSB;
wire [KBUF_P_RAM_DW/8-1:0]           kbuf_i_p_ram_wen         = ~ahb_o_kbuf_WEB;
wire [KBUF_P_RAM_AW-1:0]             kbuf_i_p_ram_addr        = ahb_o_kbuf_A;
wire [KBUF_P_RAM_DW-1:0]             kbuf_i_p_ram_wdata       = ahb_o_kbuf_DI;
wire [KBUF_P_RAM_ECC_DW-1:0]         kbuf_i_c_ram_rdata       = i_kbuf_DO;

wire                      exsta_i_clk         = cgu_o_sclk      ; 
wire                      exsta_i_rst_n       = rgu_o_srst_n    ;
wire [EXSTA_WIDTH-1:0]    exsta_i_async       = i_soc_status    ;

wire                      exerr_i_clk         = cgu_o_sclk      ;
wire                      exerr_i_rst_n       = rgu_o_srst_n    ;
wire [EXERR_WIDTH-1:0]    exerr_i_async       = i_soc_err       ;

wire           otp_ipatch_i_patch_en        = ahb_o_patch_en            ;
wire           otp_ipatch_i_patch_info_vld  = ahb_o_patch_info_vld      ;
wire [11:0]    otp_ipatch_i_patch_info_addr = ahb_o_patch_info_addr     ;
wire [31:0]    otp_ipatch_i_patch_otp_rdata = ahb_o_patch_otp_rdata     ;
wire           otp_ipatch_i_hclk            = cgu_o_boot_clk            ;
wire           otp_ipatch_i_hresetn         = rgu_o_boot_rst_n          ;
wire           otp_ipatch_i_hsels           = romahbsyn_o_htransm[1]    ;
wire [31:0]    otp_ipatch_i_haddrs          = romahbsyn_o_haddrm        ;
wire [1:0]     otp_ipatch_i_htranss         = romahbsyn_o_htransm       ;
wire           otp_ipatch_i_hwrites         = romahbsyn_o_hwritem       ;
wire [2:0]     otp_ipatch_i_hsizes          = romahbsyn_o_hsizem        ;
wire [2:0]     otp_ipatch_i_hbursts         = romahbsyn_o_hburstm       ;
wire [3:0]     otp_ipatch_i_hprots          = romahbsyn_o_hprotm        ;
wire [31:0]    otp_ipatch_i_hwdatas         = romahbsyn_o_hwdatam[31:0] ;
wire           otp_ipatch_i_hmastlocks      = romahbsyn_o_hmastlockm    ;
wire           otp_ipatch_i_hreadys         = irom_hready               ;
wire [31:0]    otp_ipatch_i_hrdatam         = ilmrom_o_hrdata           ; 
wire           otp_ipatch_i_hreadyoutm      = ilmrom_o_hreadyout        ; 
wire           otp_ipatch_i_hrespm          = ilmrom_o_hresp[0]         ;

`ifndef OSR_FPGA_ROM
wire                            irom_i_hclk              = rom_clk                      ;
wire                            irom_i_hresetn           = rom_rstn                     ;

wire [7:0]                      irom_i_ecc_ck_en         = romcfg_syn_o_sync[7:0]      ;
wire [7:0]                      irom_i_ecc_err_rsp_en    = romcfg_syn_o_sync[15:8]     ;
wire [7:0]                      irom_i_ecc_tm_en         = romcfg_syn_o_sync[23:16]    ;
wire [IROM_P_ECC_TM_CHB-1:0]    irom_i_ecc_tm_ckbits     = romcfg_syn_o_sync[30:24]    ;

wire                            irom_i_hsel              = otp_ipatch_o_hselm                       ;
wire                            irom_i_hready            = otp_ipatch_o_hreadym                     ;
wire [IROM_P_BUS_AW-1:0]        irom_i_haddr             = otp_ipatch_o_haddrm[IROM_P_BUS_AW-1:0]   ;
wire [1:0]                      irom_i_htrans            = otp_ipatch_o_htransm                     ;
wire                            irom_i_hwrite            = otp_ipatch_o_hwritem                     ;
wire [2:0]                      irom_i_hsize             = otp_ipatch_o_hsizem                      ;
wire [2:0]                      irom_i_hburst            = otp_ipatch_o_hburstm                     ;
wire [3:0]                      irom_i_hprot             = otp_ipatch_o_hprotm                      ;
wire [3:0]                      irom_i_hmaster           = 4'h0                                     ;
wire [31:0]                     irom_i_hwdata            = otp_ipatch_o_hwdatam                     ;
wire                            irom_i_hmastlock         = otp_ipatch_o_hmastlockm                  ;
wire [IROM_P_ROM_DW-1:0]        irom_i_rom_rdata         = i_irom_DO                                ;
`endif

`ifdef OSR_FPGA_ROM 
wire                             firom_i_hclk              = rom_clk                                  ;
wire                             firom_i_hresetn           = rom_rstn                                 ;
wire                             firom_i_ram_cipher_en     = 1'b0;
wire [7:0]                       firom_i_ram_cipher_key    = 8'b0                                     ;
wire [FIROM_P_BUS_DW-1:0]        firom_i_ram_cipher_dnonce = {FIROM_P_BUS_DW{1'b0}}                   ;
wire [FIROM_P_BUS_AW-3:0]        firom_i_ram_cipher_anonce = {FIROM_P_BUS_AW-2{1'b0}}                 ;
wire [7:0]                       firom_i_ecc_ck_en         = romcfg_syn_o_sync[7:0]                   ;
wire [7:0]                       firom_i_ecc_err_rsp_en    = romcfg_syn_o_sync[15:8]                  ;
wire [7:0]                       firom_i_ecc_tm_en         = romcfg_syn_o_sync[23:16]                 ;
wire [FIROM_P_ECC_TM_CHB-1:0]    firom_i_ecc_tm_ckbits     = romcfg_syn_o_sync[30:24]                 ;
wire                             firom_i_hsel              = otp_ipatch_o_hselm                       ;
wire                             firom_i_hready            = otp_ipatch_o_hreadym                     ;
wire [FIROM_P_BUS_AW-1:0]        firom_i_haddr             = otp_ipatch_o_haddrm[FIROM_P_BUS_AW-1:0]  ;
wire [1:0]                       firom_i_htrans            = otp_ipatch_o_htransm                     ;
wire                             firom_i_hwrite            = otp_ipatch_o_hwritem                     ;
wire [2:0]                       firom_i_hsize             = otp_ipatch_o_hsizem                      ;
wire [2:0]                       firom_i_hburst            = otp_ipatch_o_hburstm                     ;
wire [3:0]                       firom_i_hprot             = otp_ipatch_o_hprotm                      ;
wire [3:0]                       firom_i_hmaster           = 4'b0                                     ;
wire [31:0]                      firom_i_hwdata            = otp_ipatch_o_hwdatam                     ;
wire                             firom_i_hmastlock         = otp_ipatch_o_hmastlockm                  ;
wire [FIROM_P_ROM_DW-1:0]        firom_i_rom_rdata         = i_irom_DO                                ;
`endif 

assign o_hsm_status             = ahb_o_hsm_status          ;

assign o_hsm_err_hw             = ahb_o_emu_err_hw          ;
assign o_hsm_err_fw             = ahb_o_emu_err_fw          ;
assign o_mbox_irq               = ahb_o_mbox_soc_irq[16-1:0];

assign o_s_hrdata               = ahb_o_mbox_soc_hrdata    ;
assign o_s_hresp                = ahb_o_mbox_soc_hresp     ;
assign o_s_hreadyout            = ahb_o_mbox_soc_hreadyout ;

assign o_m_hsel                 = rh2hm_o_s_hsel    ;
assign o_m_haddr                = sochmsync_o_haddrm ;
assign o_m_htrans               = rh2hm_o_s_htrans      ;
assign o_m_hwrite               = sochmsync_o_hwritem   ;
assign o_m_hburst               = sochmsync_o_hburstm   ;
assign o_m_hsize                = sochmsync_o_hsizem    ;
assign o_m_hprot                = sochmsync_o_hprotm    ;
assign o_m_hmastlock            = sochmsync_o_hmastlockm;
assign o_m_hready               = sochmsync_o_hready    ;
assign o_m_hwdata               = sochmsync_o_hwdatam   ;

assign o_dma_m_awid             = ahb_o_dma_m_awid          ;
assign o_dma_m_awaddr           = ahb_o_dma_m_awaddr        ; 
assign o_dma_m_awlen            = ahb_o_dma_m_awlen         ;
assign o_dma_m_awsize           = ahb_o_dma_m_awsize        ;
assign o_dma_m_awburst          = ahb_o_dma_m_awburst       ;
assign o_dma_m_awlock           = ahb_o_dma_m_awlock[0]     ;
assign o_dma_m_awcache          = ahb_o_dma_m_awcache       ;
assign o_dma_m_awprot           = ahb_o_dma_m_awprot        ;
assign o_dma_m_awqos            = ahb_o_dma_m_awqos         ;
assign o_dma_m_awregion         = ahb_o_dma_m_awregion      ;
assign o_dma_m_awvalid          = rx2xdma_o_s_awvalid       ;
assign o_dma_m_wdata            = ahb_o_dma_m_wdata         ;
assign o_dma_m_wstrb            = ahb_o_dma_m_wstrb         ;
assign o_dma_m_wlast            = ahb_o_dma_m_wlast         ;
assign o_dma_m_wvalid           = rx2xdma_o_s_wvalid        ;
assign o_dma_m_bready           = rx2xdma_o_s_bready        ;
assign o_dma_m_arid             = ahb_o_dma_m_arid          ;
assign o_dma_m_araddr           = ahb_o_dma_m_araddr        ;
assign o_dma_m_arlen            = ahb_o_dma_m_arlen         ;
assign o_dma_m_arsize           = ahb_o_dma_m_arsize        ;
assign o_dma_m_arburst          = ahb_o_dma_m_arburst       ;
assign o_dma_m_arlock           = ahb_o_dma_m_arlock[0]     ;
assign o_dma_m_arcache          = ahb_o_dma_m_arcache       ;
assign o_dma_m_arprot           = ahb_o_dma_m_arprot        ;
assign o_dma_m_arqos            = ahb_o_dma_m_arqos         ;
assign o_dma_m_arregion         = ahb_o_dma_m_arregion      ;
assign o_dma_m_arvalid          = rx2xdma_o_s_arvalid       ;
assign o_dma_m_rready           = rx2xdma_o_s_rready        ;

assign o_cfg_hsel               = rh2hcfg_o_s_hsel   ;                                              
assign o_cfg_haddr              = ahb_o_cfg_haddr    ; 
assign o_cfg_htrans             = rh2hcfg_o_s_htrans ;                                             
assign o_cfg_hwrite             = ahb_o_cfg_hwrite   ; 
assign o_cfg_hburst             = ahb_o_cfg_hburst   ; 
assign o_cfg_hsize              = ahb_o_cfg_hsize    ; 
assign o_cfg_hprot              = ahb_o_cfg_hprot    ; 
assign o_cfg_hmastlock          = ahb_o_cfg_hmastlock; 
assign o_cfg_hready             = ahb_o_cfg_hready   ; 
assign o_cfg_hwdata             = ahb_o_cfg_hwdata   ; 

assign o_otp_hsel               = rh2hotp_o_s_hsel     ;
assign o_otp_haddr              = otphmsync_o_haddrm    ; 
assign o_otp_htrans             = rh2hotp_o_s_htrans   ;
assign o_otp_hwrite             = otphmsync_o_hwritem   ; 
assign o_otp_hburst             = otphmsync_o_hburstm   ; 
assign o_otp_hsize              = otphmsync_o_hsizem    ; 
assign o_otp_hprot              = otphmsync_o_hprotm    ; 
assign o_otp_hmastlock          = otphmsync_o_hmastlockm; 
assign o_otp_hready             = otphmsync_o_hready    ; 
assign o_otp_hwdata             = otphmsync_o_hwdatam   ; 

assign o_irom_CK                = rom_clk           ;
assign o_irom_CSB               = ~ilmrom_o_rom_cs  ;                            
assign o_irom_A                 = ilmrom_o_rom_addr[14-1:0];  
`ifdef OSR_FPGA_ROM  
assign o_irom_DI                = ilmrom_o_ram_wdata          ;
assign o_irom_WEB               = ~(|ilmrom_o_ram_wen)        ;
`endif

assign o_iram_CK                = iram_i_hclk               ;
assign o_iram_A                 = iram_o_ram_addr           ;
assign o_iram_CSB               = ~iram_o_ram_cs            ;
assign o_iram_WEB               = ~(|iram_o_ram_wen)        ;
assign o_iram_DI                = iram_o_ram_wdata          ;

assign o_dram_CK                = cgu_o_cpu_clk             ;
assign o_dram_A                 = P_DRAM_DIRECT_TO_CPU ? cpu_o_dram_addr : dram_o_ram_addr;
assign o_dram_CSB               = P_DRAM_DIRECT_TO_CPU ? cpu_o_dram_csb  : ~dram_o_ram_cs ;

assign o_dram_WEB               = P_DRAM_DIRECT_TO_CPU ? |cpu_o_dram_web  : ~(|dram_o_ram_wen);
assign o_dram_DI                = P_DRAM_DIRECT_TO_CPU ? cpu_o_dram_wdata : dram_o_ram_wdata  ;

assign o_kbuf_CK                = cgu_o_kmuram_clk          ;
assign o_kbuf_CSB               = ~kbuf_o_c_ram_cs          ;
assign o_kbuf_A                 = kbuf_o_c_ram_addr         ;
assign o_kbuf_WEB               = ~(|kbuf_o_c_ram_wen)      ;
assign o_kbuf_DI                = kbuf_o_c_ram_wdata        ;

assign o_pke_sram0_CK           = ahb_o_pke_sram0_CK        ; 
assign o_pke_sram0_CSAN         = ahb_o_pke_sram0_CSAN      ; 
assign o_pke_sram0_A            = ahb_o_pke_sram0_A         ; 
assign o_pke_sram0_CSBN         = ahb_o_pke_sram0_CSBN      ;
assign o_pke_sram0_B            = ahb_o_pke_sram0_B         ;
assign o_pke_sram0_DI           = ahb_o_pke_sram0_DI        ;
assign o_pke_sram1_CK           = ahb_o_pke_sram1_CK        ;
assign o_pke_sram1_CSAN         = ahb_o_pke_sram1_CSAN      ;
assign o_pke_sram1_A            = ahb_o_pke_sram1_A         ;
assign o_pke_sram1_CSBN         = ahb_o_pke_sram1_CSBN      ;
assign o_pke_sram1_B            = ahb_o_pke_sram1_B         ;
assign o_pke_sram1_DI           = ahb_o_pke_sram1_DI        ;
assign o_pke_sram2_CK           = ahb_o_pke_sram2_CK        ;
assign o_pke_sram2_CSAN         = ahb_o_pke_sram2_CSAN      ;
assign o_pke_sram2_A            = ahb_o_pke_sram2_A         ;
assign o_pke_sram2_CSBN         = ahb_o_pke_sram2_CSBN      ;
assign o_pke_sram2_B            = ahb_o_pke_sram2_B         ;
assign o_pke_sram2_DI           = ahb_o_pke_sram2_DI        ;
assign o_pke_sram3_CK           = ahb_o_pke_sram3_CK        ;
assign o_pke_sram3_CSAN         = ahb_o_pke_sram3_CSAN      ;
assign o_pke_sram3_A            = ahb_o_pke_sram3_A         ;
assign o_pke_sram3_CSBN         = ahb_o_pke_sram3_CSBN      ;
assign o_pke_sram3_B            = ahb_o_pke_sram3_B         ;
assign o_pke_sram3_DI           = ahb_o_pke_sram3_DI        ;

assign o_tdo_oe                 = cpu_o_tdo_oe              ; 
assign o_tdo                    = cpu_o_tdo                 ;

assign o_uart_txd_oe            = ahb_o_uart_txd_oe         ;
assign o_uart_txd               = ahb_o_uart_txd            ;
assign o_soc_dbg_en_128b        = ahb_o_soc_dbg_en_128b     ; 
assign o_trng_rdy               = ahb_o_trng_rdy            ;
assign o_trng_alarm             = ahb_o_trng_alarm          ;
assign o_trng_ro_clk            = ahb_o_trng_ro_clk         ;
assign o_trng_ro_out            = ahb_o_trng_ro_out         ;

osr_cgu u_cgu (
    .i_rtc_clk               ( cgu_i_rtc_clk            ), 
    .i_rtc_rst_n             ( cgu_i_rtc_rst_n          ), 
    .i_clk                   ( cgu_i_clk                ), 
    .i_rst_n                 ( cgu_i_rst_n              ), 
    .i_scan_mode             ( cgu_i_scan_mode          ), 
    .i_scan_enable           ( cgu_i_scan_enable        ), 
    .i_boot_hw_ok            ( cgu_i_boot_hw_ok         ), 
    .i_clk_ctrl0             ( cgu_i_clk_ctrl0          ), 
    .i_clk_ctrl1             ( cgu_i_clk_ctrl1          ), 
    .i_alg_clk_ctrl0         ( cgu_i_alg_clk_ctrl0      ), 
    .i_alg_clk_ctrl1         ( cgu_i_alg_clk_ctrl1      ), 
    .i_alg_clk_ctrl2         ( cgu_i_alg_clk_ctrl2      ), 
    .i_alg_clk_ctrl3         ( cgu_i_alg_clk_ctrl3      ), 
    .i_alg_clk_ctrl4         ( cgu_i_alg_clk_ctrl4      ), 
    .i_alg_clk_ctrl5         ( cgu_i_alg_clk_ctrl5      ), 
    .i_alg_clk_ctrl6         ( cgu_i_alg_clk_ctrl6      ), 
    .i_sgrst_n               ( cgu_i_sgrst_n            ), 
    .o_sclk                  ( cgu_o_sclk               ), 
    .o_gclk                  ( cgu_o_gclk               ), 
    .o_dbg_toggle_a          ( cgu_o_dbg_toggle_a       ), 
    .o_mtime_toggle_a        ( cgu_o_mtime_toggle_a     ), 
    .o_cpu_clk               ( cgu_o_cpu_clk            ), 
    .o_sys_clk               ( cgu_o_sys_clk            ), 
    .o_emu_clk               ( cgu_o_emu_clk            ), 
    .o_boot_clk              ( cgu_o_boot_clk           ), 
    .o_kmu_clk               ( cgu_o_kmu_clk            ), 
    .o_kmuram_clk            ( cgu_o_kmuram_clk         ), 
    .o_mbox_clk              ( cgu_o_mbox_clk           ), 
    .o_trng_clk              ( cgu_o_trng_clk           ), 
    .o_hash0_clk             ( cgu_o_hash0_clk          ), 
    .o_hash0_dma_clk         ( cgu_o_hash0_dma_clk      ), 
    .o_ske0_clk              ( cgu_o_ske0_clk           ), 
    .o_ske0_dma_clk          ( cgu_o_ske0_dma_clk       ), 
    .o_pke_clk               ( cgu_o_pke_clk            ), 
    .o_ahb_dma_clk           ( cgu_o_ahb_dma_clk        ), 
    .o_axi_dma_clk           ( cgu_o_axi_dma_clk        ), 
    .o_irom_clk              ( cgu_o_irom_clk           ), 
    .o_crc_clk               ( cgu_o_crc_clk            ), 
    .o_rdc_clk               ( cgu_o_rdc_clk            ), 
    .o_wdt_clk               ( cgu_o_wdt_clk            ), 
    .o_uart_clk              ( cgu_o_uart_clk           ), 
    .o_tim_clk               ( cgu_o_tim_clk            )  
    ); 

osr_rgu u_rgu (
    .i_clk                       ( rgu_i_clk                    ), 
    .i_rst_n                     ( rgu_i_rst_n                  ), 
    .i_scan_mode                 ( rgu_i_scan_mode              ), 
    .i_rtc_clk                   ( rgu_i_rtc_clk                ), 
    .i_rtc_rst_n                 ( rgu_i_rtc_rst_n              ), 
    .i_reset_trng_n              ( rgu_i_reset_trng_n           ), 
    .i_gclk                      ( rgu_i_gclk                   ), 
    .i_cpu_clk                   ( rgu_i_cpu_clk                ), 
    .i_sys_clk                   ( rgu_i_sys_clk                ), 
    .i_emu_clk                   ( rgu_i_emu_clk                ), 
    .i_boot_clk                  ( rgu_i_boot_clk               ), 
    .i_kmu_clk                   ( rgu_i_kmu_clk                ), 
    .i_kmuram_clk                ( rgu_i_kmuram_clk             ), 
    .i_mbox_clk                  ( rgu_i_mbox_clk               ), 
    .i_trng_clk                  ( rgu_i_trng_clk               ), 
    .i_hash0_clk                 ( rgu_i_hash0_clk              ), 
    .i_hash0_dma_clk             ( rgu_i_hash0_dma_clk          ), 
    .i_ske0_clk                  ( rgu_i_ske0_clk               ), 
    .i_ske0_dma_clk              ( rgu_i_ske0_dma_clk           ), 
    .i_pke_clk                   ( rgu_i_pke_clk                ), 
    .i_ahb_dma_clk               ( rgu_i_ahb_dma_clk            ), 
    .i_axi_dma_clk               ( rgu_i_axi_dma_clk            ), 
    .i_irom_clk                  ( rgu_i_irom_clk               ), 
    .i_rdc_clk                   ( rgu_i_rdc_clk                ), 
    .i_wdt_clk                   ( rgu_i_wdt_clk                ), 
    .i_uart_clk                  ( rgu_i_uart_clk               ), 
    .i_tim_clk                   ( rgu_i_tim_clk                ), 
    .i_crc_clk                   ( rgu_i_crc_clk                ), 
    .i_rst_ctrl0                 ( rgu_i_rst_ctrl0              ), 
    .i_rst_ctrl1                 ( rgu_i_rst_ctrl1              ), 
    .i_alg_rst_ctrl0             ( rgu_i_alg_rst_ctrl0          ), 
    .i_alg_rst_ctrl1             ( rgu_i_alg_rst_ctrl1          ), 
    .i_alg_rst_ctrl2             ( rgu_i_alg_rst_ctrl2          ), 
    .i_alg_rst_ctrl3             ( rgu_i_alg_rst_ctrl3          ), 
    .i_alg_rst_ctrl4             ( rgu_i_alg_rst_ctrl4          ), 
    .i_alg_rst_ctrl5             ( rgu_i_alg_rst_ctrl5          ), 
    .i_alg_rst_ctrl6             ( rgu_i_alg_rst_ctrl6          ), 
    .i_boot_hw_ok                ( rgu_i_boot_hw_ok             ), 
    .i_emu_resetn_all            ( rgu_i_emu_resetn_all         ), 
    .i_emu_resetn_noboot         ( rgu_i_emu_resetn_noboot      ), 
    .i_emu_resetn_cpu            ( rgu_i_emu_resetn_cpu         ), 
    .o_srst_n                    ( rgu_o_srst_n                 ), 
    .o_sgrst_n                   ( rgu_o_sgrst_n                ), 
    .o_hwrst_n                   ( rgu_o_hwrst_n                ), 
    .o_cpu_rst_n                 ( rgu_o_cpu_rst_n              ), 
    .i_dm_ndmreset               ( rgu_i_dm_ndmreset            ), 
    .o_dm_ndmreset_n             ( rgu_o_dm_ndmreset_n          ), 
    .o_sys_rst_n                 ( rgu_o_sys_rst_n              ), 
    .o_emu_rst_n                 ( rgu_o_emu_rst_n              ), 
    .o_boot_rst_n                ( rgu_o_boot_rst_n             ), 
    .o_kmu_rst_n                 ( rgu_o_kmu_rst_n              ), 
    .o_kmuram_rst_n              ( rgu_o_kmuram_rst_n           ), 
    .o_mbox_rst_n                ( rgu_o_mbox_rst_n             ), 
    .o_trng_rst_n                ( rgu_o_trng_rst_n             ), 
    .o_hash0_rst_n               ( rgu_o_hash0_rst_n            ), 
    .o_hash0_dma_rst_n           ( rgu_o_hash0_dma_rst_n        ), 
    .o_ske0_rst_n                ( rgu_o_ske0_rst_n             ), 
    .o_ske0_dma_rst_n            ( rgu_o_ske0_dma_rst_n         ), 
    .o_pke_rst_n                 ( rgu_o_pke_rst_n              ), 
    .o_ahb_dma_rst_n             ( rgu_o_ahb_dma_rst_n          ), 
    .o_axi_dma_rst_n             ( rgu_o_axi_dma_rst_n          ), 
    .o_irom_rst_n                ( rgu_o_irom_rst_n             ), 
    .o_crc_rst_n                 ( rgu_o_crc_rst_n              ), 
    .o_rdc_rst_n                 ( rgu_o_rdc_rst_n              ), 
    .o_wdt_rst_n                 ( rgu_o_wdt_rst_n              ), 
    .o_uart_rst_n                ( rgu_o_uart_rst_n             ), 
    .o_tim_rst_n                 ( rgu_o_tim_rst_n              ), 
    .o_cpu_boot_rst_n            ( rgu_o_cpu_boot_rst_n         )  
    ); 

osr_cpu_wrapper u_cpu_wrapper (
    .i_clk                       ( cpu_i_clk                    ), 
    .i_rst_n                     ( cpu_i_rst_n                  ), 
    .i_keep_logic                ( cpu_i_keep_logic             ), 
    .i_dbg_stop                  ( cpu_i_dbg_stop               ), 
    .i_scan_mode                 ( cpu_i_scan_mode              ), 
    .i_scan_enable               ( cpu_i_scan_enable            ), 
    .i_dbg_toggle_a              ( cpu_i_dbg_toggle_a           ), 
    .i_mtime_toggle_a            ( cpu_i_mtime_toggle_a         ), 
    .i_clic_irq                  ( cpu_i_clic_irq               ), 
    .o_dm_ndmreset               ( cpu_o_dm_ndmreset            ), 
    .i_dm_ndmreset_n             ( cpu_i_dm_ndmreset_n          ), 
    .i_trst_n                    ( cpu_i_trst_n                 ), 
    .i_tck                       ( cpu_i_tck                    ), 
    .i_tms                       ( cpu_i_tms                    ), 
    .i_tdi                       ( cpu_i_tdi                    ), 
    .o_tdo_oe                    ( cpu_o_tdo_oe                 ), 
    .o_tdo                       ( cpu_o_tdo                    ), 
    .o_ilm_htrans                ( cpu_o_ilm_htrans             ), 
    .o_ilm_hwrite                ( cpu_o_ilm_hwrite             ), 
    .o_ilm_hmastlock             ( cpu_o_ilm_hmastlock          ), 
    .o_ilm_hwdata                ( cpu_o_ilm_hwdata             ), 
    .o_ilm_haddr                 ( cpu_o_ilm_haddr              ), 
    .o_ilm_hsize                 ( cpu_o_ilm_hsize              ), 
    .o_ilm_hburst                ( cpu_o_ilm_hburst             ), 
    .o_ilm_hprot                 ( cpu_o_ilm_hprot              ), 
    .o_ilm_hmaster               ( cpu_o_ilm_hmaster            ), 
    .i_ilm_hrdata                ( cpu_i_ilm_hrdata             ), 
    .i_ilm_hresp                 ( cpu_i_ilm_hresp              ), 
    .i_ilm_hready                ( cpu_i_ilm_hready             ), 
    .o_dlm_hsel                  ( cpu_o_dlm_hsel               ), 
    .o_dlm_htrans                ( cpu_o_dlm_htrans             ), 
    .o_dlm_hwrite                ( cpu_o_dlm_hwrite             ), 
    .o_dlm_haddr                 ( cpu_o_dlm_haddr              ), 
    .o_dlm_hsize                 ( cpu_o_dlm_hsize              ), 
    .o_dlm_hburst                ( cpu_o_dlm_hburst             ), 
    .o_dlm_hmastlock             ( cpu_o_dlm_hmastlock          ), 
    .o_dlm_hwdata                ( cpu_o_dlm_hwdata             ), 
    .o_dlm_hprot                 ( cpu_o_dlm_hprot              ), 
    .o_dlm_hmaster               ( cpu_o_dlm_hmaster            ), 
    .i_dlm_hrdata                ( cpu_i_dlm_hrdata             ), 
    .i_dlm_hresp                 ( cpu_i_dlm_hresp              ), 
    .i_dlm_hready                ( cpu_i_dlm_hready             ), 
    .o_hsel                      ( cpu_o_hsel                   ), 
    .o_htrans                    ( cpu_o_htrans                 ), 
    .o_hwrite                    ( cpu_o_hwrite                 ), 
    .o_haddr                     ( cpu_o_haddr                  ), 
    .o_hsize                     ( cpu_o_hsize                  ), 
    .o_hburst                    ( cpu_o_hburst                 ), 
    .o_hmastlock                 ( cpu_o_hmastlock              ), 
    .o_hwdata                    ( cpu_o_hwdata                 ), 
    .o_hprot                     ( cpu_o_hprot                  ), 
    .o_master                    ( cpu_o_master                 ), 
    .i_hrdata                    ( cpu_i_hrdata                 ), 
    .i_hresp                     ( cpu_i_hresp                  ), 
    .i_hready                    ( cpu_i_hready                 ), 
    .i_dram_cipher_en            ( cpu_i_dram_cipher_en         ), 
    .i_dram_cipher_key           ( cpu_i_dram_cipher_key        ), 
    .i_dram_cipher_dnonce        ( cpu_i_dram_cipher_dnonce     ), 
    .i_dram_cipher_anonce        ( cpu_i_dram_cipher_anonce     ), 
    .i_dram_ecc_ck_en            ( cpu_i_dram_ecc_ck_en         ), 
    .i_dram_ecc_err_rsp_en       ( cpu_i_dram_ecc_err_rsp_en    ), 
    .i_dram_ecc_tm_en            ( cpu_i_dram_ecc_tm_en         ), 
    .i_dram_ecc_tm_ckbits        ( cpu_i_dram_ecc_tm_ckbits     ), 
    .o_dram_ecc_dec_sec          ( cpu_o_dram_ecc_dec_sec       ), 
    .o_dram_ecc_dec_ded          ( cpu_o_dram_ecc_dec_ded       ), 
    .o_dram_ecc_err_addr         ( cpu_o_dram_ecc_err_addr      ), 
    .o_dram_web                  ( cpu_o_dram_web               ), 
    .o_dram_wdata                ( cpu_o_dram_wdata             ), 
    .i_dram_rdata                ( cpu_i_dram_rdata             ), 
    .o_dram_addr                 ( cpu_o_dram_addr              ), 
    .o_dram_csb                  ( cpu_o_dram_csb               ), 
    .o_hart_halted               ( cpu_o_hart_halted            ), 
    .o_wfi                       ( cpu_o_wfi                    ), 
    .i_ahb_bus_pr_en             ( cpu_i_ahb_bus_pr_en          ), 
    .o_ipre_set_bus_pr_alarm     ( cpu_o_ipre_set_bus_pr_alarm  ), 
    .o_dpre_set_bus_pr_alarm     ( cpu_o_dpre_set_bus_pr_alarm  ), 
    .o_spre_set_bus_pr_alarm     ( cpu_o_spre_set_bus_pr_alarm  ), 
    .i_mem_init_rst_n            ( cpu_i_mem_init_rst_n         ), 
    .o_mem_init_done             ( cpu_o_mem_init_done          )  
    ); 

osr_ahb_mtx_wrapper u_ahb_matrix (
    .i_hclk           ( mtx_i_hclk        ), 
    .i_hresetn        ( mtx_i_hresetn     ), 
    .i_cipher_en      ( mtx_i_cipher_en   ), 
    .i_key            ( mtx_i_key         ), 
    .i_dat_nounce     ( mtx_i_dat_nounce  ), 
    .i_adr_nounce     ( mtx_i_adr_nounce  ), 
    .i_nosec_m0       ( mtx_i_nosec_m0    ), 
    .i_masterid_m0    ( mtx_i_masterid_m0 ), 
    .i_chid_m0        ( mtx_i_chid_m0     ), 
    .i_nosec_m1       ( mtx_i_nosec_m1    ), 
    .i_masterid_m1    ( mtx_i_masterid_m1 ), 
    .i_chid_m1        ( mtx_i_chid_m1     ), 
    .i_nosec_m2       ( mtx_i_nosec_m2    ), 
    .i_masterid_m2    ( mtx_i_masterid_m2 ), 
    .i_chid_m2        ( mtx_i_chid_m2     ), 
    .i_nosec_m3       ( mtx_i_nosec_m3    ), 
    .i_masterid_m3    ( mtx_i_masterid_m3 ), 
    .i_chid_m3        ( mtx_i_chid_m3     ), 
    .i_hsels0         ( mtx_i_hsels0      ), 
    .i_haddrs0        ( mtx_i_haddrs0     ), 
    .i_htranss0       ( mtx_i_htranss0    ), 
    .i_hwrites0       ( mtx_i_hwrites0    ), 
    .i_hsizes0        ( mtx_i_hsizes0     ), 
    .i_hbursts0       ( mtx_i_hbursts0    ), 
    .i_hprots0        ( mtx_i_hprots0     ), 
    .i_hmasters0      ( mtx_i_hmasters0   ), 
    .i_hwdatas0       ( mtx_i_hwdatas0    ), 
    .i_hmastlocks0    ( mtx_i_hmastlocks0 ), 
    .i_hreadys0       ( mtx_i_hreadys0    ), 
    .o_hrdatas0       ( mtx_o_hrdatas0    ), 
    .o_hreadyouts0    ( mtx_o_hreadyouts0 ), 
    .o_hresps0        ( mtx_o_hresps0     ), 
    .i_hsels1         ( mtx_i_hsels1      ), 
    .i_haddrs1        ( mtx_i_haddrs1     ), 
    .i_htranss1       ( mtx_i_htranss1    ), 
    .i_hwrites1       ( mtx_i_hwrites1    ), 
    .i_hsizes1        ( mtx_i_hsizes1     ), 
    .i_hbursts1       ( mtx_i_hbursts1    ), 
    .i_hprots1        ( mtx_i_hprots1     ), 
    .i_hmasters1      ( mtx_i_hmasters1   ), 
    .i_hwdatas1       ( mtx_i_hwdatas1    ), 
    .i_hmastlocks1    ( mtx_i_hmastlocks1 ), 
    .i_hreadys1       ( mtx_i_hreadys1    ), 
    .o_hrdatas1       ( mtx_o_hrdatas1    ), 
    .o_hreadyouts1    ( mtx_o_hreadyouts1 ), 
    .o_hresps1        ( mtx_o_hresps1     ), 
    .i_hsels2         ( mtx_i_hsels2      ), 
    .i_haddrs2        ( mtx_i_haddrs2     ), 
    .i_htranss2       ( mtx_i_htranss2    ), 
    .i_hwrites2       ( mtx_i_hwrites2    ), 
    .i_hsizes2        ( mtx_i_hsizes2     ), 
    .i_hbursts2       ( mtx_i_hbursts2    ), 
    .i_hprots2        ( mtx_i_hprots2     ), 
    .i_hmasters2      ( mtx_i_hmasters2   ), 
    .i_hwdatas2       ( mtx_i_hwdatas2    ), 
    .i_hmastlocks2    ( mtx_i_hmastlocks2 ), 
    .i_hreadys2       ( mtx_i_hreadys2    ), 
    .o_hrdatas2       ( mtx_o_hrdatas2    ), 
    .o_hreadyouts2    ( mtx_o_hreadyouts2 ), 
    .o_hresps2        ( mtx_o_hresps2     ), 
    .i_hsels3         ( mtx_i_hsels3      ), 
    .i_haddrs3        ( mtx_i_haddrs3     ), 
    .i_htranss3       ( mtx_i_htranss3    ), 
    .i_hwrites3       ( mtx_i_hwrites3    ), 
    .i_hsizes3        ( mtx_i_hsizes3     ), 
    .i_hbursts3       ( mtx_i_hbursts3    ), 
    .i_hprots3        ( mtx_i_hprots3     ), 
    .i_hmasters3      ( mtx_i_hmasters3   ), 
    .i_hwdatas3       ( mtx_i_hwdatas3    ), 
    .i_hmastlocks3    ( mtx_i_hmastlocks3 ), 
    .i_hreadys3       ( mtx_i_hreadys3    ), 
    .o_hrdatas3       ( mtx_o_hrdatas3    ), 
    .o_hreadyouts3    ( mtx_o_hreadyouts3 ), 
    .o_hresps3        ( mtx_o_hresps3     ), 
    .i_hsels4         ( mtx_i_hsels4      ), 
    .i_haddrs4        ( mtx_i_haddrs4     ), 
    .i_htranss4       ( mtx_i_htranss4    ), 
    .i_hwrites4       ( mtx_i_hwrites4    ), 
    .i_hsizes4        ( mtx_i_hsizes4     ), 
    .i_hbursts4       ( mtx_i_hbursts4    ), 
    .i_hprots4        ( mtx_i_hprots4     ), 
    .i_hmasters4      ( mtx_i_hmasters4   ), 
    .i_hwdatas4       ( mtx_i_hwdatas4    ), 
    .i_hmastlocks4    ( mtx_i_hmastlocks4 ), 
    .i_hreadys4       ( mtx_i_hreadys4    ), 
    .o_hrdatas4       ( mtx_o_hrdatas4    ), 
    .o_hreadyouts4    ( mtx_o_hreadyouts4 ), 
    .o_hresps4        ( mtx_o_hresps4     ), 
    .o_hselm0         ( mtx_o_hselm0      ), 
    .o_haddrm0        ( mtx_o_haddrm0     ), 
    .o_htransm0       ( mtx_o_htransm0    ), 
    .o_hwritem0       ( mtx_o_hwritem0    ), 
    .o_hsizem0        ( mtx_o_hsizem0     ), 
    .o_hburstm0       ( mtx_o_hburstm0    ), 
    .o_hprotm0        ( mtx_o_hprotm0     ), 
    .o_hmasterm0      ( mtx_o_hmasterm0   ), 
    .o_hwdatam0       ( mtx_o_hwdatam0    ), 
    .o_hmastlockm0    ( mtx_o_hmastlockm0 ), 
    .o_hreadymuxm0    ( mtx_o_hreadymuxm0 ), 
    .i_hrdatam0       ( mtx_i_hrdatam0    ), 
    .i_hreadyoutm0    ( mtx_i_hreadyoutm0 ), 
    .i_hrespm0        ( mtx_i_hrespm0     ), 
    .o_hselm1         ( mtx_o_hselm1      ), 
    .o_haddrm1        ( mtx_o_haddrm1     ), 
    .o_htransm1       ( mtx_o_htransm1    ), 
    .o_hwritem1       ( mtx_o_hwritem1    ), 
    .o_hsizem1        ( mtx_o_hsizem1     ), 
    .o_hburstm1       ( mtx_o_hburstm1    ), 
    .o_hprotm1        ( mtx_o_hprotm1     ), 
    .o_hmasterm1      ( mtx_o_hmasterm1   ), 
    .o_hwdatam1       ( mtx_o_hwdatam1    ), 
    .o_hmastlockm1    ( mtx_o_hmastlockm1 ), 
    .o_hreadymuxm1    ( mtx_o_hreadymuxm1 ), 
    .i_hrdatam1       ( mtx_i_hrdatam1    ), 
    .i_hreadyoutm1    ( mtx_i_hreadyoutm1 ), 
    .i_hrespm1        ( mtx_i_hrespm1     ), 
    .o_hselm2         ( mtx_o_hselm2      ), 
    .o_haddrm2        ( mtx_o_haddrm2     ), 
    .o_htransm2       ( mtx_o_htransm2    ), 
    .o_hwritem2       ( mtx_o_hwritem2    ), 
    .o_hsizem2        ( mtx_o_hsizem2     ), 
    .o_hburstm2       ( mtx_o_hburstm2    ), 
    .o_hprotm2        ( mtx_o_hprotm2     ), 
    .o_hmasterm2      ( mtx_o_hmasterm2   ), 
    .o_hwdatam2       ( mtx_o_hwdatam2    ), 
    .o_hmastlockm2    ( mtx_o_hmastlockm2 ), 
    .o_hreadymuxm2    ( mtx_o_hreadymuxm2 ), 
    .i_hrdatam2       ( mtx_i_hrdatam2    ), 
    .i_hreadyoutm2    ( mtx_i_hreadyoutm2 ), 
    .i_hrespm2        ( mtx_i_hrespm2     ), 
    .o_hselm3         ( mtx_o_hselm3      ), 
    .o_haddrm3        ( mtx_o_haddrm3     ), 
    .o_htransm3       ( mtx_o_htransm3    ), 
    .o_hwritem3       ( mtx_o_hwritem3    ), 
    .o_hsizem3        ( mtx_o_hsizem3     ), 
    .o_hburstm3       ( mtx_o_hburstm3    ), 
    .o_hprotm3        ( mtx_o_hprotm3     ), 
    .o_hmasterm3      ( mtx_o_hmasterm3   ), 
    .o_hwdatam3       ( mtx_o_hwdatam3    ), 
    .o_hmastlockm3    ( mtx_o_hmastlockm3 ), 
    .o_hreadymuxm3    ( mtx_o_hreadymuxm3 ), 
    .i_hrdatam3       ( mtx_i_hrdatam3    ), 
    .i_hreadyoutm3    ( mtx_i_hreadyoutm3 ), 
    .i_hrespm3        ( mtx_i_hrespm3     ), 
    .o_hselm4         ( mtx_o_hselm4      ), 
    .o_haddrm4        ( mtx_o_haddrm4     ), 
    .o_htransm4       ( mtx_o_htransm4    ), 
    .o_hwritem4       ( mtx_o_hwritem4    ), 
    .o_hsizem4        ( mtx_o_hsizem4     ), 
    .o_hburstm4       ( mtx_o_hburstm4    ), 
    .o_hprotm4        ( mtx_o_hprotm4     ), 
    .o_hmasterm4      ( mtx_o_hmasterm4   ), 
    .o_hwdatam4       ( mtx_o_hwdatam4    ), 
    .o_hmastlockm4    ( mtx_o_hmastlockm4 ), 
    .o_hreadymuxm4    ( mtx_o_hreadymuxm4 ), 
    .i_hrdatam4       ( mtx_i_hrdatam4    ), 
    .i_hreadyoutm4    ( mtx_i_hreadyoutm4 ), 
    .i_hrespm4        ( mtx_i_hrespm4     ), 
    .o_hselm5         ( mtx_o_hselm5      ), 
    .o_haddrm5        ( mtx_o_haddrm5     ), 
    .o_htransm5       ( mtx_o_htransm5    ), 
    .o_hwritem5       ( mtx_o_hwritem5    ), 
    .o_hsizem5        ( mtx_o_hsizem5     ), 
    .o_hburstm5       ( mtx_o_hburstm5    ), 
    .o_hprotm5        ( mtx_o_hprotm5     ), 
    .o_hmasterm5      ( mtx_o_hmasterm5   ), 
    .o_hwdatam5       ( mtx_o_hwdatam5    ), 
    .o_hmastlockm5    ( mtx_o_hmastlockm5 ), 
    .o_hreadymuxm5    ( mtx_o_hreadymuxm5 ), 
    .i_hrdatam5       ( mtx_i_hrdatam5    ), 
    .i_hreadyoutm5    ( mtx_i_hreadyoutm5 ), 
    .i_hrespm5        ( mtx_i_hrespm5     )  
    ); 

osr_ahb_bus_wrapper #(
    .P_DMA_AW     ( AHB_P_DMA_AW ), 
    .P_DMA_DW     ( AHB_P_DMA_DW )  
    ) u_ahb_wrapper (
    .i_sclk                           ( ahb_i_sclk                        ), 
    .i_gclk                           ( ahb_i_gclk                        ), 
    .i_scan_mode                      ( ahb_i_scan_mode                   ), 
    .i_cpu_clk                        ( ahb_i_cpu_clk                     ), 
    .i_sys_clk                        ( ahb_i_sys_clk                     ), 
    .i_emu_clk                        ( ahb_i_emu_clk                     ), 
    .i_boot_clk                       ( ahb_i_boot_clk                    ), 
    .i_kmu_clk                        ( ahb_i_kmu_clk                     ), 
    .i_mbox_clk                       ( ahb_i_mbox_clk                    ), 
    .i_trng_clk                       ( ahb_i_trng_clk                    ), 
    .i_hash0_clk                      ( ahb_i_hash0_clk                   ), 
    .i_hash0_dma_clk                  ( ahb_i_hash0_dma_clk               ), 
    .i_ske0_clk                       ( ahb_i_ske0_clk                    ), 
    .i_ske0_dma_clk                   ( ahb_i_ske0_dma_clk                ), 
    .i_pke_clk                        ( ahb_i_pke_clk                     ), 
    .i_ahb_dma_clk                    ( ahb_i_ahb_dma_clk                 ), 
    .i_axi_dma_clk                    ( ahb_i_axi_dma_clk                 ), 
    .i_rdc_clk                        ( ahb_i_rdc_clk                     ), 
    .i_wdt_clk                        ( ahb_i_wdt_clk                     ), 
    .i_uart_clk                       ( ahb_i_uart_clk                    ), 
    .i_tim_clk                        ( ahb_i_tim_clk                     ), 
    .i_crc_clk                        ( ahb_i_crc_clk                     ), 
    .i_srst_n                         ( ahb_i_srst_n                      ), 
    .i_hwrst_n                        ( ahb_i_hwrst_n                     ), 
    .i_cpu_rst_n                      ( ahb_i_cpu_rst_n                   ), 
    .i_sys_rst_n                      ( ahb_i_sys_rst_n                   ), 
    .i_emu_rst_n                      ( ahb_i_emu_rst_n                   ), 
    .i_boot_rst_n                     ( ahb_i_boot_rst_n                  ), 
    .i_kmu_rst_n                      ( ahb_i_kmu_rst_n                   ), 
    .i_mbox_rst_n                     ( ahb_i_mbox_rst_n                  ), 
    .i_trng_rst_n                     ( ahb_i_trng_rst_n                  ), 
    .i_hash0_rst_n                    ( ahb_i_hash0_rst_n                 ), 
    .i_hash0_dma_rst_n                ( ahb_i_hash0_dma_rst_n             ), 
    .i_ske0_rst_n                     ( ahb_i_ske0_rst_n                  ), 
    .i_ske0_dma_rst_n                 ( ahb_i_ske0_dma_rst_n              ), 
    .i_pke_rst_n                      ( ahb_i_pke_rst_n                   ), 
    .i_ahb_dma_rst_n                  ( ahb_i_ahb_dma_rst_n               ), 
    .i_axi_dma_rst_n                  ( ahb_i_axi_dma_rst_n               ), 
    .i_rdc_rst_n                      ( ahb_i_rdc_rst_n                   ), 
    .i_wdt_rst_n                      ( ahb_i_wdt_rst_n                   ), 
    .i_uart_rst_n                     ( ahb_i_uart_rst_n                  ), 
    .i_tim_rst_n                      ( ahb_i_tim_rst_n                   ), 
    .i_crc_rst_n                      ( ahb_i_crc_rst_n                   ), 
    .o_clock_en                       ( ahb_o_clock_en                    ), 
    .o_reset_trng_n                   ( ahb_o_reset_trng_n                ), 
    .o_keep_logic                     ( ahb_o_keep_logic                  ), 
    .i_m_hsel                         ( ahb_i_m_hsel                      ), 
    .i_m_haddr                        ( ahb_i_m_haddr                     ), 
    .i_m_htrans                       ( ahb_i_m_htrans                    ), 
    .i_m_hwrite                       ( ahb_i_m_hwrite                    ), 
    .i_m_hburst                       ( ahb_i_m_hburst                    ), 
    .i_m_hsize                        ( ahb_i_m_hsize                     ), 
    .i_m_hprot                        ( ahb_i_m_hprot                     ), 
    .i_m_hmastlock                    ( ahb_i_m_hmastlock                 ), 
    .i_m_hready                       ( ahb_i_m_hready                    ), 
    .i_m_hwdata                       ( ahb_i_m_hwdata                    ), 
    .o_m_hrdata                       ( ahb_o_m_hrdata                    ), 
    .o_m_hresp                        ( ahb_o_m_hresp                     ), 
    .o_m_hreadyout                    ( ahb_o_m_hreadyout                 ), 
    .o_otp_hsel                       ( ahb_o_otp_hsel                    ), 
    .o_otp_haddr                      ( ahb_o_otp_haddr                   ), 
    .o_otp_htrans                     ( ahb_o_otp_htrans                  ), 
    .o_otp_hwrite                     ( ahb_o_otp_hwrite                  ), 
    .o_otp_hburst                     ( ahb_o_otp_hburst                  ), 
    .o_otp_hsize                      ( ahb_o_otp_hsize                   ), 
    .o_otp_hprot                      ( ahb_o_otp_hprot                   ), 
    .o_otp_hmastlock                  ( ahb_o_otp_hmastlock               ), 
    .o_otp_hready                     ( ahb_o_otp_hready                  ), 
    .o_otp_hwdata                     ( ahb_o_otp_hwdata                  ), 
    .i_otp_hrdata                     ( ahb_i_otp_hrdata                  ), 
    .i_otp_hresp                      ( ahb_i_otp_hresp                   ), 
    .i_otp_hreadyout                  ( ahb_i_otp_hreadyout               ), 
    .i_mbox_soc_sta                   ( ahb_i_mbox_soc_sta                ), 
    .i_mbox_soc_hsel                  ( ahb_i_mbox_soc_hsel               ), 
    .i_mbox_soc_haddr                 ( ahb_i_mbox_soc_haddr              ), 
    .i_mbox_soc_htrans                ( ahb_i_mbox_soc_htrans             ), 
    .i_mbox_soc_hwrite                ( ahb_i_mbox_soc_hwrite             ), 
    .i_mbox_soc_hburst                ( ahb_i_mbox_soc_hburst             ), 
    .i_mbox_soc_hsize                 ( ahb_i_mbox_soc_hsize              ), 
    .i_mbox_soc_hprot                 ( ahb_i_mbox_soc_hprot              ), 
    .i_mbox_soc_hmastlock             ( ahb_i_mbox_soc_hmastlock          ), 
    .i_mbox_soc_hready                ( ahb_i_mbox_soc_hready             ), 
    .i_mbox_soc_hwdata                ( ahb_i_mbox_soc_hwdata             ), 
    .o_mbox_soc_hrdata                ( ahb_o_mbox_soc_hrdata             ), 
    .o_mbox_soc_hresp                 ( ahb_o_mbox_soc_hresp              ), 
    .o_mbox_soc_hreadyout             ( ahb_o_mbox_soc_hreadyout          ), 
    .o_cfg_hsel                       ( ahb_o_cfg_hsel                    ), 
    .o_cfg_haddr                      ( ahb_o_cfg_haddr                   ), 
    .o_cfg_htrans                     ( ahb_o_cfg_htrans                  ), 
    .o_cfg_hwrite                     ( ahb_o_cfg_hwrite                  ), 
    .o_cfg_hburst                     ( ahb_o_cfg_hburst                  ), 
    .o_cfg_hsize                      ( ahb_o_cfg_hsize                   ), 
    .o_cfg_hprot                      ( ahb_o_cfg_hprot                   ), 
    .o_cfg_hmastlock                  ( ahb_o_cfg_hmastlock               ), 
    .o_cfg_hready                     ( ahb_o_cfg_hready                  ), 
    .o_cfg_hwdata                     ( ahb_o_cfg_hwdata                  ), 
    .i_cfg_hrdata                     ( ahb_i_cfg_hrdata                  ), 
    .i_cfg_hresp                      ( ahb_i_cfg_hresp                   ), 
    .i_cfg_hreadyout                  ( ahb_i_cfg_hreadyout               ), 
    .o_dma_m_awid                     ( ahb_o_dma_m_awid                  ), 
    .o_dma_m_awaddr                   ( ahb_o_dma_m_awaddr                ), 
    .o_dma_m_awlen                    ( ahb_o_dma_m_awlen                 ), 
    .o_dma_m_awsize                   ( ahb_o_dma_m_awsize                ), 
    .o_dma_m_awburst                  ( ahb_o_dma_m_awburst               ), 
    .o_dma_m_awlock                   ( ahb_o_dma_m_awlock                ), 
    .o_dma_m_awcache                  ( ahb_o_dma_m_awcache               ), 
    .o_dma_m_awprot                   ( ahb_o_dma_m_awprot                ), 
    .o_dma_m_awqos                    ( ahb_o_dma_m_awqos                 ), 
    .o_dma_m_awregion                 ( ahb_o_dma_m_awregion              ), 
    .o_dma_m_awvalid                  ( ahb_o_dma_m_awvalid               ), 
    .i_dma_m_awready                  ( ahb_i_dma_m_awready               ), 
    .o_dma_m_wid                      ( ahb_o_dma_m_wid                   ), 
    .o_dma_m_wdata                    ( ahb_o_dma_m_wdata                 ), 
    .o_dma_m_wstrb                    ( ahb_o_dma_m_wstrb                 ), 
    .o_dma_m_wlast                    ( ahb_o_dma_m_wlast                 ), 
    .o_dma_m_wvalid                   ( ahb_o_dma_m_wvalid                ), 
    .i_dma_m_wready                   ( ahb_i_dma_m_wready                ), 
    .i_dma_m_bid                      ( ahb_i_dma_m_bid                   ), 
    .i_dma_m_bresp                    ( ahb_i_dma_m_bresp                 ), 
    .i_dma_m_bvalid                   ( ahb_i_dma_m_bvalid                ), 
    .o_dma_m_bready                   ( ahb_o_dma_m_bready                ), 
    .o_dma_m_arid                     ( ahb_o_dma_m_arid                  ), 
    .o_dma_m_araddr                   ( ahb_o_dma_m_araddr                ), 
    .o_dma_m_arlen                    ( ahb_o_dma_m_arlen                 ), 
    .o_dma_m_arsize                   ( ahb_o_dma_m_arsize                ), 
    .o_dma_m_arburst                  ( ahb_o_dma_m_arburst               ), 
    .o_dma_m_arlock                   ( ahb_o_dma_m_arlock                ), 
    .o_dma_m_arcache                  ( ahb_o_dma_m_arcache               ), 
    .o_dma_m_arprot                   ( ahb_o_dma_m_arprot                ), 
    .o_dma_m_arqos                    ( ahb_o_dma_m_arqos                 ), 
    .o_dma_m_arregion                 ( ahb_o_dma_m_arregion              ), 
    .o_dma_m_arvalid                  ( ahb_o_dma_m_arvalid               ), 
    .i_dma_m_arready                  ( ahb_i_dma_m_arready               ), 
    .i_dma_m_rid                      ( ahb_i_dma_m_rid                   ), 
    .i_dma_m_rdata                    ( ahb_i_dma_m_rdata                 ), 
    .i_dma_m_rresp                    ( ahb_i_dma_m_rresp                 ), 
    .i_dma_m_rlast                    ( ahb_i_dma_m_rlast                 ), 
    .i_dma_m_rvalid                   ( ahb_i_dma_m_rvalid                ), 
    .o_dma_m_rready                   ( ahb_o_dma_m_rready                ), 
    .o_dma_m_awbar                    ( ahb_o_dma_m_awbar                 ), 
    .o_dma_m_awsnoop                  ( ahb_o_dma_m_awsnoop               ), 
    .o_dma_m_awdomain                 ( ahb_o_dma_m_awdomain              ), 
    .o_dma_m_arbar                    ( ahb_o_dma_m_arbar                 ), 
    .o_dma_m_arsnoop                  ( ahb_o_dma_m_arsnoop               ), 
    .o_dma_m_ardomain                 ( ahb_o_dma_m_ardomain              ), 
    .o_dma_m_hsel                     ( ahb_o_dma_m_hsel                  ), 
    .o_dma_m_haddr                    ( ahb_o_dma_m_haddr                 ), 
    .o_dma_m_htrans                   ( ahb_o_dma_m_htrans                ), 
    .o_dma_m_hwrite                   ( ahb_o_dma_m_hwrite                ), 
    .o_dma_m_hburst                   ( ahb_o_dma_m_hburst                ), 
    .o_dma_m_hsize                    ( ahb_o_dma_m_hsize                 ), 
    .o_dma_m_hprot                    ( ahb_o_dma_m_hprot                 ), 
    .o_dma_m_hmastlock                ( ahb_o_dma_m_hmastlock             ), 
    .o_dma_m_hwdata                   ( ahb_o_dma_m_hwdata                ), 
    .i_dma_m_hrdata                   ( ahb_i_dma_m_hrdata                ), 
    .i_dma_m_hresp                    ( ahb_i_dma_m_hresp                 ), 
    .i_dma_m_hreadyout                ( ahb_i_dma_m_hreadyout             ), 
    .o_kbuf_CSB                       ( ahb_o_kbuf_CSB                    ), 
    .o_kbuf_A                         ( ahb_o_kbuf_A                      ), 
    .o_kbuf_WEB                       ( ahb_o_kbuf_WEB                    ), 
    .o_kbuf_DI                        ( ahb_o_kbuf_DI                     ), 
    .i_kbuf_DO                        ( ahb_i_kbuf_DO                     ), 
    .o_pke_sram0_CK                   ( ahb_o_pke_sram0_CK                ), 
    .o_pke_sram0_CSAN                 ( ahb_o_pke_sram0_CSAN              ), 
    .o_pke_sram0_A                    ( ahb_o_pke_sram0_A                 ), 
    .o_pke_sram0_CSBN                 ( ahb_o_pke_sram0_CSBN              ), 
    .o_pke_sram0_B                    ( ahb_o_pke_sram0_B                 ), 
    .o_pke_sram1_CK                   ( ahb_o_pke_sram1_CK                ), 
    .o_pke_sram1_CSAN                 ( ahb_o_pke_sram1_CSAN              ), 
    .o_pke_sram1_A                    ( ahb_o_pke_sram1_A                 ), 
    .o_pke_sram1_CSBN                 ( ahb_o_pke_sram1_CSBN              ), 
    .o_pke_sram1_B                    ( ahb_o_pke_sram1_B                 ), 
    .o_pke_sram2_CK                   ( ahb_o_pke_sram2_CK                ), 
    .o_pke_sram2_CSAN                 ( ahb_o_pke_sram2_CSAN              ), 
    .o_pke_sram2_A                    ( ahb_o_pke_sram2_A                 ), 
    .o_pke_sram2_CSBN                 ( ahb_o_pke_sram2_CSBN              ), 
    .o_pke_sram2_B                    ( ahb_o_pke_sram2_B                 ), 
    .o_pke_sram3_CK                   ( ahb_o_pke_sram3_CK                ), 
    .o_pke_sram3_CSAN                 ( ahb_o_pke_sram3_CSAN              ), 
    .o_pke_sram3_A                    ( ahb_o_pke_sram3_A                 ), 
    .o_pke_sram3_CSBN                 ( ahb_o_pke_sram3_CSBN              ), 
    .o_pke_sram3_B                    ( ahb_o_pke_sram3_B                 ), 
    .i_pke_sram0_DO                   ( ahb_i_pke_sram0_DO                ), 
    .o_pke_sram0_DI                   ( ahb_o_pke_sram0_DI                ), 
    .i_pke_sram1_DO                   ( ahb_i_pke_sram1_DO                ), 
    .o_pke_sram1_DI                   ( ahb_o_pke_sram1_DI                ), 
    .i_pke_sram2_DO                   ( ahb_i_pke_sram2_DO                ), 
    .o_pke_sram2_DI                   ( ahb_o_pke_sram2_DI                ), 
    .i_pke_sram3_DO                   ( ahb_i_pke_sram3_DO                ), 
    .o_pke_sram3_DI                   ( ahb_o_pke_sram3_DI                ), 
    .o_clk_ctrl0                      ( ahb_o_clk_ctrl0                   ), 
    .o_clk_ctrl1                      ( ahb_o_clk_ctrl1                   ), 
    .o_alg_clk_ctrl0                  ( ahb_o_alg_clk_ctrl0               ), 
    .o_alg_clk_ctrl1                  ( ahb_o_alg_clk_ctrl1               ), 
    .o_alg_clk_ctrl2                  ( ahb_o_alg_clk_ctrl2               ), 
    .o_alg_clk_ctrl3                  ( ahb_o_alg_clk_ctrl3               ), 
    .o_alg_clk_ctrl4                  ( ahb_o_alg_clk_ctrl4               ), 
    .o_alg_clk_ctrl5                  ( ahb_o_alg_clk_ctrl5               ), 
    .o_alg_clk_ctrl6                  ( ahb_o_alg_clk_ctrl6               ), 
    .o_rst_ctrl0                      ( ahb_o_rst_ctrl0                   ), 
    .o_rst_ctrl1                      ( ahb_o_rst_ctrl1                   ), 
    .o_alg_rst_ctrl0                  ( ahb_o_alg_rst_ctrl0               ), 
    .o_alg_rst_ctrl1                  ( ahb_o_alg_rst_ctrl1               ), 
    .o_alg_rst_ctrl2                  ( ahb_o_alg_rst_ctrl2               ), 
    .o_alg_rst_ctrl3                  ( ahb_o_alg_rst_ctrl3               ), 
    .o_alg_rst_ctrl4                  ( ahb_o_alg_rst_ctrl4               ), 
    .o_alg_rst_ctrl5                  ( ahb_o_alg_rst_ctrl5               ), 
    .o_alg_rst_ctrl6                  ( ahb_o_alg_rst_ctrl6               ), 
    .i_sensor                         ( ahb_i_sensor                      ), 
    .i_soc_status                     ( ahb_i_soc_status                  ), 
    .i_soc_err                        ( ahb_i_soc_err                     ), 
    .o_hsm_status                     ( ahb_o_hsm_status                  ), 
    .o_ahb_dma_en                     ( ahb_o_ahb_dma_en                  ), 
    .o_boot_hw_ok                     ( ahb_o_boot_hw_ok                  ), 
    .o_boot_hw_err                    ( ahb_o_boot_hw_err                 ), 
    .o_otp_ahb_if_sel                 ( ahb_o_otp_ahb_if_sel              ), 
    .o_ahb_nvm_rsp_err_en             ( ahb_o_ahb_nvm_rsp_err_en          ), 
    .o_ahb_cfg_rsp_err_en             ( ahb_o_ahb_cfg_rsp_err_en          ), 
    .o_ahb_otp_rsp_err_en             ( ahb_o_ahb_otp_rsp_err_en          ), 
    .o_ahb_soc_rsp_err_en             ( ahb_o_ahb_soc_rsp_err_en          ), 
    .o_mbox_soc_irq                   ( ahb_o_mbox_soc_irq                ), 
    .o_mbox_ram_ba                    ( ahb_o_mbox_ram_ba                 ), 
    .i_uart_rxd                       ( ahb_i_uart_rxd                    ), 
    .o_uart_txd_oe                    ( ahb_o_uart_txd_oe                 ), 
    .o_uart_txd                       ( ahb_o_uart_txd                    ), 
    .o_irq2cpu                        ( ahb_o_irq2cpu                     ), 
    .o_emu_err_sensor                 ( ahb_o_emu_err_sensor              ), 
    .o_emu_err_hw                     ( ahb_o_emu_err_hw                  ), 
    .o_emu_err_fw                     ( ahb_o_emu_err_fw                  ), 
    .o_emu_resetn_all                 ( ahb_o_emu_resetn_all              ), 
    .o_emu_resetn_noboot              ( ahb_o_emu_resetn_noboot           ), 
    .o_emu_resetn_cpu                 ( ahb_o_emu_resetn_cpu              ), 
    .o_trng_ro_clk                    ( ahb_o_trng_ro_clk                 ), 
    .o_trng_ro_out                    ( ahb_o_trng_ro_out                 ), 
    .o_bus_cipher_en                  ( ahb_o_bus_cipher_en               ), 
    .o_bus_cipher_key                 ( ahb_o_bus_cipher_key              ), 
    .o_bus_cipher_nce                 ( ahb_o_bus_cipher_nce              ), 
    .o_ram_cipher_en                  ( ahb_o_ram_cipher_en               ), 
    .o_ram_cipher_key                 ( ahb_o_ram_cipher_key              ), 
    .o_ram_cipher_nce                 ( ahb_o_ram_cipher_nce              ), 
    .o_nvm_cipher_bypass              ( ahb_o_nvm_cipher_bypass           ), 
    .o_irom_ecc_cfg                   ( ahb_o_irom_ecc_cfg                ), 
    .o_iram_ecc_cfg                   ( ahb_o_iram_ecc_cfg                ), 
    .o_dram_ecc_cfg                   ( ahb_o_dram_ecc_cfg                ), 
    .o_kmu_ram_ecc_cfg                ( ahb_o_kmu_ram_ecc_cfg             ), 
    .o_cpu_cfg                        ( ahb_o_cpu_cfg                     ), 
    .i_mem_ecc_1b_irom                ( ahb_i_mem_ecc_1b_irom             ), 
    .i_mem_ecc_1b_iram                ( ahb_i_mem_ecc_1b_iram             ), 
    .i_mem_ecc_1b_dram                ( ahb_i_mem_ecc_1b_dram             ), 
    .i_mem_ecc_1b_kmu                 ( ahb_i_mem_ecc_1b_kmu              ), 
    .i_mem_ecc_mb_irom                ( ahb_i_mem_ecc_mb_irom             ), 
    .i_mem_ecc_mb_iram                ( ahb_i_mem_ecc_mb_iram             ), 
    .i_mem_ecc_mb_dram                ( ahb_i_mem_ecc_mb_dram             ), 
    .i_mem_ecc_mb_kmu                 ( ahb_i_mem_ecc_mb_kmu              ), 
    .i_mem_ecc_addr_irom              ( ahb_i_mem_ecc_addr_irom           ), 
    .i_mem_ecc_addr_iram              ( ahb_i_mem_ecc_addr_iram           ), 
    .i_mem_ecc_addr_dram              ( ahb_i_mem_ecc_addr_dram           ), 
    .i_mem_ecc_addr_kmu               ( ahb_i_mem_ecc_addr_kmu            ), 
    .i_soc_err_ahb_mem                ( ahb_i_soc_err_ahb_mem             ), 
    .i_soc_err_ahb_otp                ( ahb_i_soc_err_ahb_otp             ), 
    .i_soc_err_ahb_nvm                ( ahb_i_soc_err_ahb_nvm             ), 
    .i_soc_err_ahb_cfg                ( ahb_i_soc_err_ahb_cfg             ), 
    .i_soc_err_axi_dma_wr             ( ahb_i_soc_err_axi_dma_wr          ), 
    .i_soc_err_axi_dma_rd             ( ahb_i_soc_err_axi_dma_rd          ), 
    .o_ahb_bus_pr_en                  ( ahb_o_ahb_bus_pr_en               ), 
    .i_ipre_pchk_err                  ( ahb_i_ipre_pchk_err               ), 
    .i_dpre_pchk_err                  ( ahb_i_dpre_pchk_err               ), 
    .i_spre_pchk_err                  ( ahb_i_spre_pchk_err               ), 
    .i_ahbdpre_pchk_err               ( ahb_i_ahbdpre_pchk_err            ), 
    .i_iromprc_pchk_err               ( ahb_i_iromprc_pchk_err            ), 
    .i_iramprc_pchk_err               ( ahb_i_iramprc_pchk_err            ), 
    .i_dramprc_pchk_err               ( ahb_i_dramprc_pchk_err            ), 
    .i_ahbprc_pchk_err                ( ahb_i_ahbprc_pchk_err             ), 
    .i_nvmprc_pchk_err                ( ahb_i_nvmprc_pchk_err             ), 
    .i_socprc_pchk_err                ( ahb_i_socprc_pchk_err             ), 
    .i_cpu_hart_halted                ( ahb_i_cpu_hart_halted             ), 
    .i_cpu_wfi                        ( ahb_i_cpu_wfi                     ), 
    .o_trng_rdy                       ( ahb_o_trng_rdy                    ), 
    .o_trng_alarm                     ( ahb_o_trng_alarm                  ), 
    .o_patch_en                       ( ahb_o_patch_en                    ), 
    .o_patch_info_vld                 ( ahb_o_patch_info_vld              ), 
    .o_patch_info_addr                ( ahb_o_patch_info_addr             ), 
    .o_patch_otp_rdata                ( ahb_o_patch_otp_rdata             ), 
    .i_mem_init_done                  ( ahb_i_mem_init_done               ), 
    .o_soc_dbg_en_128b                ( ahb_o_soc_dbg_en_128b             ), 
    .o_hsm_dbg_en                     ( ahb_o_hsm_dbg_en                  )  
    ); 

generate if(OSR_NVM_AHB_BUS_TIMING_ISOLATION) begin : g_nvmhmsync
osr_ahb_to_ahb_sync #(
    .P_AW        ( NVMHMSYNC_P_AW    ), 
    .P_DW        ( NVMHMSYNC_P_DW    ), 
    .P_MW        ( NVMHMSYNC_P_MW    ), 
    .P_BURST     ( NVMHMSYNC_P_BURST )  
    ) u_nvmhmsync (
    .i_hclk                ( nvmhmsync_i_hclk       ), 
    .i_hresetn             ( nvmhmsync_i_hresetn    ), 
    .i_hsels               ( nvmhmsync_i_hsels      ), 
    .i_haddrs              ( nvmhmsync_i_haddrs     ), 
    .i_htranss             ( nvmhmsync_i_htranss    ), 
    .i_hsizes              ( nvmhmsync_i_hsizes     ), 
    .i_hwrites             ( nvmhmsync_i_hwrites    ), 
    .i_hreadys             ( nvmhmsync_i_hreadys    ), 
    .i_hprots              ( nvmhmsync_i_hprots     ), 
    .i_hmasters            ( nvmhmsync_i_hmasters   ), 
    .i_hmastlocks          ( nvmhmsync_i_hmastlocks ), 
    .i_hwdatas             ( nvmhmsync_i_hwdatas    ), 
    .i_hbursts             ( nvmhmsync_i_hbursts    ), 
    .o_hreadyouts          ( nvmhmsync_o_hreadyouts ), 
    .o_hresps              ( nvmhmsync_o_hresps     ), 
    .o_hrdatas             ( nvmhmsync_o_hrdatas    ), 
    .o_haddrm              ( nvmhmsync_o_haddrm     ), 
    .o_htransm             ( nvmhmsync_o_htransm    ), 
    .o_hsizem              ( nvmhmsync_o_hsizem     ), 
    .o_hwritem             ( nvmhmsync_o_hwritem    ), 
    .o_hprotm              ( nvmhmsync_o_hprotm     ), 
    .o_hmasterm            ( nvmhmsync_o_hmasterm   ), 
    .o_hmastlockm          ( nvmhmsync_o_hmastlockm ), 
    .o_hwdatam             ( nvmhmsync_o_hwdatam    ), 
    .o_hburstm             ( nvmhmsync_o_hburstm    ), 
    .i_hreadyoutm          ( nvmhmsync_i_hreadyoutm ), 
    .i_hresps              ( nvmhmsync_i_hresps     ), 
    .i_hrdatam             ( nvmhmsync_i_hrdatam    )  
    ); 
end
endgenerate

generate if(OSR_SOC_AHB_BUS_TIMING_ISOLATION) begin : g_sochmsync
osr_ahb_to_ahb_sync #(
    .P_AW        ( SOCHMSYNC_P_AW    ), 
    .P_DW        ( SOCHMSYNC_P_DW    ), 
    .P_MW        ( SOCHMSYNC_P_MW    ), 
    .P_BURST     ( SOCHMSYNC_P_BURST )  
    ) u_sochmsync (
    .i_hclk                ( sochmsync_i_hclk       ), 
    .i_hresetn             ( sochmsync_i_hresetn    ), 
    .i_hsels               ( sochmsync_i_hsels      ), 
    .i_haddrs              ( sochmsync_i_haddrs     ), 
    .i_htranss             ( sochmsync_i_htranss    ), 
    .i_hsizes              ( sochmsync_i_hsizes     ), 
    .i_hwrites             ( sochmsync_i_hwrites    ), 
    .i_hreadys             ( sochmsync_i_hreadys    ), 
    .i_hprots              ( sochmsync_i_hprots     ), 
    .i_hmasters            ( sochmsync_i_hmasters   ), 
    .i_hmastlocks          ( sochmsync_i_hmastlocks ), 
    .i_hwdatas             ( sochmsync_i_hwdatas    ), 
    .i_hbursts             ( sochmsync_i_hbursts    ), 
    .o_hreadyouts          ( sochmsync_o_hreadyouts ), 
    .o_hresps              ( sochmsync_o_hresps     ), 
    .o_hrdatas             ( sochmsync_o_hrdatas    ), 
    .o_haddrm              ( sochmsync_o_haddrm     ), 
    .o_htransm             ( sochmsync_o_htransm    ), 
    .o_hsizem              ( sochmsync_o_hsizem     ), 
    .o_hwritem             ( sochmsync_o_hwritem    ), 
    .o_hprotm              ( sochmsync_o_hprotm     ), 
    .o_hmasterm            ( sochmsync_o_hmasterm   ), 
    .o_hmastlockm          ( sochmsync_o_hmastlockm ), 
    .o_hwdatam             ( sochmsync_o_hwdatam    ), 
    .o_hburstm             ( sochmsync_o_hburstm    ), 
    .i_hreadyoutm          ( sochmsync_i_hreadyoutm ), 
    .i_hresps              ( sochmsync_i_hresps     ), 
    .i_hrdatam             ( sochmsync_i_hrdatam    )  
    ); 
end
endgenerate

generate if(OSR_OTP_AHB_BUS_TIMING_ISOLATION) begin : g_otphmsync
osr_ahb_to_ahb_sync #(
    .P_AW        ( OTPHMSYNC_P_AW    ), 
    .P_DW        ( OTPHMSYNC_P_DW    ), 
    .P_MW        ( OTPHMSYNC_P_MW    ), 
    .P_BURST     ( OTPHMSYNC_P_BURST )  
    ) u_otphmsync (
    .i_hclk                ( otphmsync_i_hclk       ), 
    .i_hresetn             ( otphmsync_i_hresetn    ), 
    .i_hsels               ( otphmsync_i_hsels      ), 
    .i_haddrs              ( otphmsync_i_haddrs     ), 
    .i_htranss             ( otphmsync_i_htranss    ), 
    .i_hsizes              ( otphmsync_i_hsizes     ), 
    .i_hwrites             ( otphmsync_i_hwrites    ), 
    .i_hreadys             ( otphmsync_i_hreadys    ), 
    .i_hprots              ( otphmsync_i_hprots     ), 
    .i_hmasters            ( otphmsync_i_hmasters   ), 
    .i_hmastlocks          ( otphmsync_i_hmastlocks ), 
    .i_hwdatas             ( otphmsync_i_hwdatas    ), 
    .i_hbursts             ( otphmsync_i_hbursts    ), 
    .o_hreadyouts          ( otphmsync_o_hreadyouts ), 
    .o_hresps              ( otphmsync_o_hresps     ), 
    .o_hrdatas             ( otphmsync_o_hrdatas    ), 
    .o_haddrm              ( otphmsync_o_haddrm     ), 
    .o_htransm             ( otphmsync_o_htransm    ), 
    .o_hsizem              ( otphmsync_o_hsizem     ), 
    .o_hwritem             ( otphmsync_o_hwritem    ), 
    .o_hprotm              ( otphmsync_o_hprotm     ), 
    .o_hmasterm            ( otphmsync_o_hmasterm   ), 
    .o_hmastlockm          ( otphmsync_o_hmastlockm ), 
    .o_hwdatam             ( otphmsync_o_hwdatam    ), 
    .o_hburstm             ( otphmsync_o_hburstm    ), 
    .i_hreadyoutm          ( otphmsync_i_hreadyoutm ), 
    .i_hresps              ( otphmsync_i_hresps     ), 
    .i_hrdatam             ( otphmsync_i_hrdatam    )  
    ); 
end
endgenerate

generate if(OSR_ABUS_AHB_BUS_TIMING_ISOLATION) begin : g_abushmsync
osr_ahb_to_ahb_sync #(
    .P_AW        ( ABUSHMSYNC_P_AW    ), 
    .P_DW        ( ABUSHMSYNC_P_DW    ), 
    .P_MW        ( ABUSHMSYNC_P_MW    ), 
    .P_BURST     ( ABUSHMSYNC_P_BURST )  
    ) u_abushmsync (
    .i_hclk                 ( abushmsync_i_hclk       ), 
    .i_hresetn              ( abushmsync_i_hresetn    ), 
    .i_hsels                ( abushmsync_i_hsels      ), 
    .i_haddrs               ( abushmsync_i_haddrs     ), 
    .i_htranss              ( abushmsync_i_htranss    ), 
    .i_hsizes               ( abushmsync_i_hsizes     ), 
    .i_hwrites              ( abushmsync_i_hwrites    ), 
    .i_hreadys              ( abushmsync_i_hreadys    ), 
    .i_hprots               ( abushmsync_i_hprots     ), 
    .i_hmasters             ( abushmsync_i_hmasters   ), 
    .i_hmastlocks           ( abushmsync_i_hmastlocks ), 
    .i_hwdatas              ( abushmsync_i_hwdatas    ), 
    .i_hbursts              ( abushmsync_i_hbursts    ), 
    .o_hreadyouts           ( abushmsync_o_hreadyouts ), 
    .o_hresps               ( abushmsync_o_hresps     ), 
    .o_hrdatas              ( abushmsync_o_hrdatas    ), 
    .o_haddrm               ( abushmsync_o_haddrm     ), 
    .o_htransm              ( abushmsync_o_htransm    ), 
    .o_hsizem               ( abushmsync_o_hsizem     ), 
    .o_hwritem              ( abushmsync_o_hwritem    ), 
    .o_hprotm               ( abushmsync_o_hprotm     ), 
    .o_hmasterm             ( abushmsync_o_hmasterm   ), 
    .o_hmastlockm           ( abushmsync_o_hmastlockm ), 
    .o_hwdatam              ( abushmsync_o_hwdatam    ), 
    .o_hburstm              ( abushmsync_o_hburstm    ), 
    .i_hreadyoutm           ( abushmsync_i_hreadyoutm ), 
    .i_hresps               ( abushmsync_i_hresps     ), 
    .i_hrdatam              ( abushmsync_i_hrdatam    )  
    ); 
end
endgenerate

generate if(OSR_AHB_DMA_TIMING_ISOLATION) begin : g_ahbdmasync
osr_ahb_to_ahb_sync #(
    .P_AW        ( AHBDMASYNC_P_AW    ), 
    .P_DW        ( AHBDMASYNC_P_DW    ), 
    .P_MW        ( AHBDMASYNC_P_MW    ), 
    .P_BURST     ( AHBDMASYNC_P_BURST )  
    ) u_ahbdmasync (
    .i_hclk                 ( ahbdmasync_i_hclk       ), 
    .i_hresetn              ( ahbdmasync_i_hresetn    ), 
    .i_hsels                ( ahbdmasync_i_hsels      ), 
    .i_haddrs               ( ahbdmasync_i_haddrs     ), 
    .i_htranss              ( ahbdmasync_i_htranss    ), 
    .i_hsizes               ( ahbdmasync_i_hsizes     ), 
    .i_hwrites              ( ahbdmasync_i_hwrites    ), 
    .i_hreadys              ( ahbdmasync_i_hreadys    ), 
    .i_hprots               ( ahbdmasync_i_hprots     ), 
    .i_hmasters             ( ahbdmasync_i_hmasters   ), 
    .i_hmastlocks           ( ahbdmasync_i_hmastlocks ), 
    .i_hwdatas              ( ahbdmasync_i_hwdatas    ), 
    .i_hbursts              ( ahbdmasync_i_hbursts    ), 
    .o_hreadyouts           ( ahbdmasync_o_hreadyouts ), 
    .o_hresps               ( ahbdmasync_o_hresps     ), 
    .o_hrdatas              ( ahbdmasync_o_hrdatas    ), 
    .o_haddrm               ( ahbdmasync_o_haddrm     ), 
    .o_htransm              ( ahbdmasync_o_htransm    ), 
    .o_hsizem               ( ahbdmasync_o_hsizem     ), 
    .o_hwritem              ( ahbdmasync_o_hwritem    ), 
    .o_hprotm               ( ahbdmasync_o_hprotm     ), 
    .o_hmasterm             ( ahbdmasync_o_hmasterm   ), 
    .o_hmastlockm           ( ahbdmasync_o_hmastlockm ), 
    .o_hwdatam              ( ahbdmasync_o_hwdatam    ), 
    .o_hburstm              ( ahbdmasync_o_hburstm    ), 
    .i_hreadyoutm           ( ahbdmasync_i_hreadyoutm ), 
    .i_hresps               ( ahbdmasync_i_hresps     ), 
    .i_hrdatam              ( ahbdmasync_i_hrdatam    )  
    ); 
end
endgenerate

osr_ahb_ram_inf_extend #(
    .P_BUS_AW             ( IRAM_P_BUS_AW         ), 
    .P_BUS_DW             ( IRAM_P_BUS_DW         ), 
    .P_BUS_NO_WORD        ( IRAM_P_BUS_NO_WORD    ), 
    .P_AHB_TIMING         ( IRAM_P_AHB_TIMING     ), 
    .P_AHB2RAM_TIMING     ( IRAM_P_AHB2RAM_TIMING ), 
    .P_CIPHER_EN          ( IRAM_P_CIPHER_EN      ), 
    .P_RAM_BLOCK_AW       ( IRAM_P_RAM_BLOCK_AW   ), 
    .P_ECC_EN             ( IRAM_P_ECC_EN         ), 
    .P_ECC_TM_CHB         ( IRAM_P_ECC_TM_CHB     ), 
    .P_RAM_DW             ( IRAM_P_RAM_DW         ), 
    .P_MEM_INIT           ( IRAM_P_MEM_INIT       ), 
    .P_MEM_INIT_DEEP      ( IRAM_P_MEM_INIT_DEEP  )  
    ) u_h2iram (
    .i_hclk                  ( iram_i_hclk              ), 
    .i_hresetn               ( iram_i_hresetn           ), 
    .i_ram_cipher_en         ( iram_i_ram_cipher_en     ), 
    .i_ram_cipher_key        ( iram_i_ram_cipher_key    ), 
    .i_ram_cipher_dnonce     ( iram_i_ram_cipher_dnonce ), 
    .i_ram_cipher_anonce     ( iram_i_ram_cipher_anonce ), 
    .i_ecc_ck_en             ( iram_i_ecc_ck_en         ), 
    .i_ecc_err_rsp_en        ( iram_i_ecc_err_rsp_en    ), 
    .i_ecc_tm_en             ( iram_i_ecc_tm_en         ), 
    .i_ecc_tm_ckbits         ( iram_i_ecc_tm_ckbits     ), 
    .o_ecc_dec_sec           ( iram_o_ecc_dec_sec       ), 
    .o_ecc_dec_ded           ( iram_o_ecc_dec_ded       ), 
    .o_ecc_err_addr          ( iram_o_ecc_err_addr      ), 
    .i_hsel                  ( iram_i_hsel              ), 
    .i_hready                ( iram_i_hready            ), 
    .i_haddr                 ( iram_i_haddr             ), 
    .i_htrans                ( iram_i_htrans            ), 
    .i_hwrite                ( iram_i_hwrite            ), 
    .i_hsize                 ( iram_i_hsize             ), 
    .i_hburst                ( iram_i_hburst            ), 
    .i_hprot                 ( iram_i_hprot             ), 
    .i_hmaster               ( iram_i_hmaster           ), 
    .i_hwdata                ( iram_i_hwdata            ), 
    .i_hmastlock             ( iram_i_hmastlock         ), 
    .o_hrdata                ( iram_o_hrdata            ), 
    .o_hreadyout             ( iram_o_hreadyout         ), 
    .o_hresp                 ( iram_o_hresp             ), 
    .o_ram_cs                ( iram_o_ram_cs            ), 
    .o_ram_wen               ( iram_o_ram_wen           ), 
    .o_ram_addr              ( iram_o_ram_addr          ), 
    .o_ram_wdata             ( iram_o_ram_wdata         ), 
    .i_ram_rdata             ( iram_i_ram_rdata         ), 
    .i_mem_init_rst_n        ( iram_i_mem_init_rst_n    ), 
    .o_mem_init_done         ( iram_o_mem_init_done     )  
    ); 

osr_ahb_ram_inf #(
    .P_BUS_AW             ( DRAM_P_BUS_AW         ), 
    .P_BUS_DW             ( DRAM_P_BUS_DW         ), 
    .P_BUS_NO_WORD        ( DRAM_P_BUS_NO_WORD    ), 
    .P_AHB_TIMING         ( DRAM_P_AHB_TIMING     ), 
    .P_AHB2RAM_TIMING     ( DRAM_P_AHB2RAM_TIMING ), 
    .P_CIPHER_EN          ( DRAM_P_CIPHER_EN      ), 
    .P_RAM_BLOCK_AW       ( DRAM_P_RAM_BLOCK_AW   ), 
    .P_ECC_EN             ( DRAM_P_ECC_EN         ), 
    .P_ECC_TM_CHB         ( DRAM_P_ECC_TM_CHB     ), 
    .P_RAM_DW             ( DRAM_P_RAM_DW         ), 
    .P_MEM_INIT           ( DRAM_P_MEM_INIT       ), 
    .P_MEM_INIT_DEEP      ( DRAM_P_MEM_INIT_DEEP  )  
    ) u_h2dram (
    .i_hclk                  ( dram_i_hclk              ), 
    .i_hresetn               ( dram_i_hresetn           ), 
    .i_ram_cipher_en         ( dram_i_ram_cipher_en     ), 
    .i_ram_cipher_key        ( dram_i_ram_cipher_key    ), 
    .i_ram_cipher_dnonce     ( dram_i_ram_cipher_dnonce ), 
    .i_ram_cipher_anonce     ( dram_i_ram_cipher_anonce ), 
    .i_ecc_ck_en             ( dram_i_ecc_ck_en         ), 
    .i_ecc_err_rsp_en        ( dram_i_ecc_err_rsp_en    ), 
    .i_ecc_tm_en             ( dram_i_ecc_tm_en         ), 
    .i_ecc_tm_ckbits         ( dram_i_ecc_tm_ckbits     ), 
    .o_ecc_dec_sec           ( dram_o_ecc_dec_sec       ), 
    .o_ecc_dec_ded           ( dram_o_ecc_dec_ded       ), 
    .o_ecc_err_addr          ( dram_o_ecc_err_addr      ), 
    .i_hsel                  ( dram_i_hsel              ), 
    .i_hready                ( dram_i_hready            ), 
    .i_haddr                 ( dram_i_haddr             ), 
    .i_htrans                ( dram_i_htrans            ), 
    .i_hwrite                ( dram_i_hwrite            ), 
    .i_hsize                 ( dram_i_hsize             ), 
    .i_hburst                ( dram_i_hburst            ), 
    .i_hprot                 ( dram_i_hprot             ), 
    .i_hmaster               ( dram_i_hmaster           ), 
    .i_hwdata                ( dram_i_hwdata            ), 
    .i_hmastlock             ( dram_i_hmastlock         ), 
    .o_hrdata                ( dram_o_hrdata            ), 
    .o_hreadyout             ( dram_o_hreadyout         ), 
    .o_hresp                 ( dram_o_hresp             ), 
    .o_ram_cs                ( dram_o_ram_cs            ), 
    .o_ram_wen               ( dram_o_ram_wen           ), 
    .o_ram_addr              ( dram_o_ram_addr          ), 
    .o_ram_wdata             ( dram_o_ram_wdata         ), 
    .i_ram_rdata             ( dram_i_ram_rdata         ), 
    .i_mem_init_rst_n        ( dram_i_mem_init_rst_n    ), 
    .o_mem_init_done         ( dram_o_mem_init_done     )  
    ); 

osr_ram_cipher_ecc_top #(
    .P_RAM_AW             ( KBUF_P_RAM_AW         ), 
    .P_RAM_DW             ( KBUF_P_RAM_DW         ), 
    .P_RAM_CIPHER_EN      ( KBUF_P_RAM_CIPHER_EN  ), 
    .P_RAM_CIPHER_RN      ( KBUF_P_RAM_CIPHER_RN  ), 
    .P_RAM_BLOCK_AW       ( KBUF_P_RAM_BLOCK_AW   ), 
    .P_RAM_ECC_EN         ( KBUF_P_RAM_ECC_EN     ), 
    .P_RAM_ECC_TM_CHB     ( KBUF_P_RAM_ECC_TM_CHB ), 
    .P_RAM_ECC_DW         ( KBUF_P_RAM_ECC_DW     )  
    ) u_kbuf_cipher_ecc (
    .i_hclk                  ( kbuf_i_hclk              ), 
    .i_hresetn               ( kbuf_i_hresetn           ), 
    .i_ram_cipher_en         ( kbuf_i_ram_cipher_en     ), 
    .i_ram_cipher_key        ( kbuf_i_ram_cipher_key    ), 
    .i_ram_cipher_dnonce     ( kbuf_i_ram_cipher_dnonce ), 
    .i_ram_cipher_anonce     ( kbuf_i_ram_cipher_anonce ), 
    .i_ecc_ck_en             ( kbuf_i_ecc_ck_en         ), 
    .i_ecc_err_rsp_en        ( kbuf_i_ecc_err_rsp_en    ), 
    .i_ecc_tm_en             ( kbuf_i_ecc_tm_en         ), 
    .i_ecc_tm_ckbits         ( kbuf_i_ecc_tm_ckbits     ), 
    .o_ecc_dec_sec           ( kbuf_o_ecc_dec_sec       ), 
    .o_ecc_dec_ded           ( kbuf_o_ecc_dec_ded       ), 
    .o_ecc_err_addr          ( kbuf_o_ecc_err_addr      ), 
    .o_ecc_dec_sec_nd        ( kbuf_o_ecc_dec_sec_nd    ), 
    .o_ecc_dec_ded_nd        ( kbuf_o_ecc_dec_ded_nd    ), 
    .i_p_ram_cs              ( kbuf_i_p_ram_cs          ), 
    .i_p_ram_wen             ( kbuf_i_p_ram_wen         ), 
    .i_p_ram_addr            ( kbuf_i_p_ram_addr        ), 
    .i_p_ram_wdata           ( kbuf_i_p_ram_wdata       ), 
    .o_p_ram_rdata           ( kbuf_o_p_ram_rdata       ), 
    .o_c_ram_cs              ( kbuf_o_c_ram_cs          ), 
    .o_c_ram_wen             ( kbuf_o_c_ram_wen         ), 
    .o_c_ram_addr            ( kbuf_o_c_ram_addr        ), 
    .o_c_ram_wdata           ( kbuf_o_c_ram_wdata       ), 
    .i_c_ram_rdata           ( kbuf_i_c_ram_rdata       )  
    ); 

osr_sync_expand #(
    .WIDTH     ( EXSTA_WIDTH )  
    ) u_exsta (
    .i_clk              ( exsta_i_clk         ), 
    .i_rst_n            ( exsta_i_rst_n       ), 
    .i_async            ( exsta_i_async       ), 
    .o_sync_expand      ( exsta_o_sync_expand )  
    ); 

osr_sync_expand #(
    .WIDTH     ( EXERR_WIDTH )  
    ) u_exerr (
    .i_clk              ( exerr_i_clk         ), 
    .i_rst_n            ( exerr_i_rst_n       ), 
    .i_async            ( exerr_i_async       ), 
    .o_sync_expand      ( exerr_o_sync_expand )  
    ); 

osr_otp_ipatch u_otp_ipatch (
    .i_patch_en                  ( otp_ipatch_i_patch_en        ), 
    .i_patch_info_vld            ( otp_ipatch_i_patch_info_vld  ), 
    .i_patch_info_addr           ( otp_ipatch_i_patch_info_addr ), 
    .i_patch_otp_rdata           ( otp_ipatch_i_patch_otp_rdata ), 
    .i_hclk                      ( otp_ipatch_i_hclk            ), 
    .i_hresetn                   ( otp_ipatch_i_hresetn         ), 
    .i_hsels                     ( otp_ipatch_i_hsels           ), 
    .i_haddrs                    ( otp_ipatch_i_haddrs          ), 
    .i_htranss                   ( otp_ipatch_i_htranss         ), 
    .i_hwrites                   ( otp_ipatch_i_hwrites         ), 
    .i_hsizes                    ( otp_ipatch_i_hsizes          ), 
    .i_hbursts                   ( otp_ipatch_i_hbursts         ), 
    .i_hprots                    ( otp_ipatch_i_hprots          ), 
    .i_hwdatas                   ( otp_ipatch_i_hwdatas         ), 
    .i_hmastlocks                ( otp_ipatch_i_hmastlocks      ), 
    .i_hreadys                   ( otp_ipatch_i_hreadys         ), 
    .o_hrdatas                   ( otp_ipatch_o_hrdatas         ), 
    .o_hreadyouts                ( otp_ipatch_o_hreadyouts      ), 
    .o_hresps                    ( otp_ipatch_o_hresps          ), 
    .o_hselm                     ( otp_ipatch_o_hselm           ), 
    .o_haddrm                    ( otp_ipatch_o_haddrm          ), 
    .o_htransm                   ( otp_ipatch_o_htransm         ), 
    .o_hwritem                   ( otp_ipatch_o_hwritem         ), 
    .o_hsizem                    ( otp_ipatch_o_hsizem          ), 
    .o_hburstm                   ( otp_ipatch_o_hburstm         ), 
    .o_hprotm                    ( otp_ipatch_o_hprotm          ), 
    .o_hwdatam                   ( otp_ipatch_o_hwdatam         ), 
    .o_hmastlockm                ( otp_ipatch_o_hmastlockm      ), 
    .o_hreadym                   ( otp_ipatch_o_hreadym         ), 
    .i_hrdatam                   ( otp_ipatch_i_hrdatam         ), 
    .i_hreadyoutm                ( otp_ipatch_i_hreadyoutm      ), 
    .i_hrespm                    ( otp_ipatch_i_hrespm          )  
    ); 

`ifndef OSR_FPGA_ROM
osr_ahb_rom_inf #(
    .P_BUS_AW          ( IROM_P_BUS_AW        ), 
    .P_BUS_DW          ( IROM_P_BUS_DW        ), 
    .P_BUS_NO_WORD     ( IROM_P_BUS_NO_WORD   ), 
    .P_AHB2RAM_TIMING  ( IROM_P_AHB2RAM_TIMING), 
    .P_CIPHER_EN       ( IROM_P_CIPHER_EN     ), 
    .P_ECC_EN          ( IROM_P_ECC_EN        ), 
    .P_ECC_TM_CHB      ( IROM_P_ECC_TM_CHB    ), 
    .P_ROM_DW          ( IROM_P_ROM_DW        )  
    ) u_h2irom (
    .i_hclk                  ( irom_i_hclk              ), 
    .i_hresetn               ( irom_i_hresetn           ), 
    .i_ecc_ck_en             ( irom_i_ecc_ck_en         ), 
    .i_ecc_err_rsp_en        ( irom_i_ecc_err_rsp_en    ), 
    .i_ecc_tm_en             ( irom_i_ecc_tm_en         ), 
    .i_ecc_tm_ckbits         ( irom_i_ecc_tm_ckbits     ), 
    .o_ecc_dec_sec           ( irom_o_ecc_dec_sec       ), 
    .o_ecc_dec_ded           ( irom_o_ecc_dec_ded       ), 
    .o_ecc_err_addr          ( irom_o_ecc_err_addr      ), 
    .i_hsel                  ( irom_i_hsel              ), 
    .i_hready                ( irom_i_hready            ), 
    .i_haddr                 ( irom_i_haddr             ), 
    .i_htrans                ( irom_i_htrans            ), 
    .i_hwrite                ( irom_i_hwrite            ), 
    .i_hsize                 ( irom_i_hsize             ), 
    .i_hburst                ( irom_i_hburst            ), 
    .i_hprot                 ( irom_i_hprot             ), 
    .i_hmaster               ( irom_i_hmaster           ), 
    .i_hwdata                ( irom_i_hwdata            ), 
    .i_hmastlock             ( irom_i_hmastlock         ), 
    .o_hrdata                ( irom_o_hrdata            ), 
    .o_hreadyout             ( irom_o_hreadyout         ), 
    .o_hresp                 ( irom_o_hresp             ), 
    .o_rom_cs                ( irom_o_rom_cs            ), 
    .o_rom_addr              ( irom_o_rom_addr          ), 
    .i_rom_rdata             ( irom_i_rom_rdata         )  
    ); 
`endif

`ifdef OSR_FPGA_ROM
osr_fpga_ahb2rom_wrapper #(
    .P_BUS_AW          ( FIROM_P_BUS_AW      ), 
    .P_BUS_DW          ( FIROM_P_BUS_DW      ), 
    .P_BUS_NO_WORD     ( FIROM_P_BUS_NO_WORD ), 
    .P_CIPHER_EN       ( FIROM_P_CIPHER_EN   ), 
    .P_ECC_EN          ( FIROM_P_ECC_EN      ), 
    .P_ECC_TM_CHB      ( FIROM_P_ECC_TM_CHB  ), 
    .P_ROM_DW          ( FIROM_P_ROM_DW      )  
    ) u_h2irom (
    .i_hclk                   ( firom_i_hclk              ), 
    .i_hresetn                ( firom_i_hresetn           ), 
    .i_ram_cipher_en          ( firom_i_ram_cipher_en     ), 
    .i_ram_cipher_key         ( firom_i_ram_cipher_key    ), 
    .i_ram_cipher_dnonce      ( firom_i_ram_cipher_dnonce ), 
    .i_ram_cipher_anonce      ( firom_i_ram_cipher_anonce ), 
    .i_ecc_ck_en              ( firom_i_ecc_ck_en         ), 
    .i_ecc_err_rsp_en         ( firom_i_ecc_err_rsp_en    ), 
    .i_ecc_tm_en              ( firom_i_ecc_tm_en         ), 
    .i_ecc_tm_ckbits          ( firom_i_ecc_tm_ckbits     ), 
    .o_ecc_dec_sec            ( firom_o_ecc_dec_sec       ), 
    .o_ecc_dec_ded            ( firom_o_ecc_dec_ded       ), 
    .o_ecc_err_addr           ( firom_o_ecc_err_addr      ), 
    .i_hsel                   ( firom_i_hsel              ), 
    .i_hready                 ( firom_i_hready            ), 
    .i_haddr                  ( firom_i_haddr             ), 
    .i_htrans                 ( firom_i_htrans            ), 
    .i_hwrite                 ( firom_i_hwrite            ), 
    .i_hsize                  ( firom_i_hsize             ), 
    .i_hburst                 ( firom_i_hburst            ), 
    .i_hprot                  ( firom_i_hprot             ), 
    .i_hmaster                ( firom_i_hmaster           ), 
    .i_hwdata                 ( firom_i_hwdata            ), 
    .i_hmastlock              ( firom_i_hmastlock         ), 
    .o_hrdata                 ( firom_o_hrdata            ), 
    .o_hreadyout              ( firom_o_hreadyout         ), 
    .o_hresp                  ( firom_o_hresp             ), 
    .o_rom_cs                 ( firom_o_rom_cs            ), 
    .o_rom_wen                ( firom_o_rom_wen           ), 
    .o_rom_addr               ( firom_o_rom_addr          ), 
    .o_rom_wdata              ( firom_o_rom_wdata         ), 
    .i_rom_rdata              ( firom_i_rom_rdata         )  
    ); 
`endif 

always @(posedge ahb_i_sclk or negedge ahb_i_srst_n) begin
    if(!ahb_i_srst_n) begin
        soc_err_ahb_mem          <= 1'b0                          ; 
    end else begin
        soc_err_ahb_mem          <= soc_err_ahb_mem_next          ; 
    end
end

always @(posedge ahb_i_sclk or negedge ahb_i_srst_n) begin
    if(!ahb_i_srst_n) begin
        soc_err_ahb_otp          <= 1'b0                          ; 
    end else begin
        soc_err_ahb_otp          <= soc_err_ahb_otp_next          ; 
    end
end

always @(posedge ahb_i_sclk or negedge ahb_i_srst_n) begin
    if(!ahb_i_srst_n) begin
        soc_err_ahb_cfg          <= 1'b0                          ; 
    end else begin
        soc_err_ahb_cfg          <= soc_err_ahb_cfg_next          ; 
    end
end

always @(posedge ahb_i_axi_dma_clk or negedge ahb_i_axi_dma_rst_n) begin
    if(!ahb_i_axi_dma_rst_n) begin
        soc_err_axi_dma_wr       <= 1'b0                          ; 
        soc_err_axi_dma_rd       <= 1'b0                          ; 
    end else begin
        soc_err_axi_dma_wr       <= soc_err_axi_dma_wr_next       ; 
        soc_err_axi_dma_rd       <= soc_err_axi_dma_rd_next       ; 
    end
end

endmodule
