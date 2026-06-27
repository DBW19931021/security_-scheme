//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_fpga_top (
    input  wire                 OSCCLK              ,
    input  wire                 OSCCLK_N            ,
    input  wire                 RST_N               ,

    `ifdef OSR_FPGA_PWR_CTL
    output wire                 PWR_TEST_EN         , 
    output wire                 PWR_EN              ,
    inout  wire                 PWR_SCL             ,
    inout  wire                 PWR_SDA             ,
    `endif

    input  wire                 i_upper_CS_TCK      ,               
    input  wire                 i_upper_CS_TMS      ,               
    input  wire                 i_upper_tdi         ,
    output wire                 o_upper_tdo         ,

    input  wire                 i_upper_uart_rxd    ,
    output wire                 o_upper_uart_txd    ,

    input  wire                 i_upper_uart1_rxd   ,
    output wire                 o_upper_uart1_txd   ,

    output wire [3:0]           o_hsm_led           ,

    input  wire                 i_hsm_tck           ,
    input  wire                 i_hsm_trst_n        ,        
    input  wire                 i_hsm_tms           ,
    input  wire                 i_hsm_tdi           ,
    output wire                 o_hsm_tdo           ,

    input  wire                 i_hsm_uart_rxd      ,
    output wire                 o_hsm_uart_txd
);

localparam CLK_OUT0_MHZ  = 30 ; 
localparam CLK_OUT1_MHZ  = 60 ; 
localparam CLK_IN_MHZ    = 200;
localparam CLK_IN_DIVIDE = 1;
localparam VCO_MULT      = 6;                
localparam VCO_MHZ       = CLK_IN_MHZ * VCO_MULT / CLK_IN_DIVIDE;
localparam DIV0          = VCO_MHZ / CLK_OUT0_MHZ;  
localparam DIV1          = VCO_MHZ / CLK_OUT1_MHZ;  

localparam UPPER_P_DMA_AW = 64;
localparam UPPER_P_DMA_DW = 64;

           localparam     IROM_ECC_ID0 = "../../romcode/ehsm_bootloader_ecc.romcode";
           localparam     IROM_ECC_I0 = 1'b0;

        localparam IRAM_ECC_I0  = 1;
        localparam IRAM_ECC_ID0 = "init.data";

           localparam     DRAM_ECC_I0 = 1'b1;

       localparam UPPER_IRAM_AW = 19;
       localparam UPPER_IRAM_ID0 = "../../fpga/src/uppercode/upper.romcode";

       localparam UPPER_DRAM_AW = 18;

       localparam UPPER_RAM_I0 = 0;
       localparam UPPER_RAM_SA0 = 0;

       localparam UPPER_RAM_ID0 = "../../romcode/ehsm_firmware_secure_boot_aes128.romcode" ;

       localparam NVM_RAM_AW = 13 ;
       localparam NVM_RAM_ID0 = "init.data" ;
       localparam NVM_RAM_SA0 = 4096;
       localparam NVM_RAM_ID1 = "init.data" ;
       localparam NVM_RAM_SA1 = 4096;
       localparam NVM_RAM_I0 = 0;

localparam P_BUS_ODD_PARITY_CHK_EN = 0;

localparam SOC_HBUS_AW_WIDTH = 64 ;

localparam     AHB2KRAM_P_BUS_AW         = 10 ;
localparam     AHB2KRAM_P_BUS_NO_WORD    = 0  ;
localparam     AHB2KRAM_P_AHB2RAM_TIMING = 0  ;
localparam     AHB2KRAM_P_AW_DEL         = 2  ;

wire clk_buf            ;
wire clkfbout           ;
wire clkfbout_buf       ;
wire clk_out0           ;
wire clk_out1           ;
wire pll_clk            ;
wire test_clk           ;
wire pll_locked         ;
wire PWR_PWR_TEST_EN    ;
wire PWR_PWR_EN         ;
wire cpu_clk_div        ;
wire cpu_clk_div_en     ;
wire w_hsm_o_mboxdbg_irq;
wire            upper_o_clk              ;
wire            upper_o_rst_n            ;
wire            upper_o_rtc_clk          ;
wire            upper_o_rtc_rst_n        ;
wire            upper_o_scan_mode        ;
wire [31:0]     upper_o_sensor           ;
wire [31:0]     upper_o_soc_status       ;
wire [31:0]     upper_o_soc_err          ;
wire            upper_o_s_hsel           ;
wire [31:0]     upper_o_s_haddr          ;
wire [1:0]      upper_o_s_htrans         ;
wire            upper_o_s_hwrite         ;
wire [2:0]      upper_o_s_hburst         ;
wire [2:0]      upper_o_s_hsize          ;
wire [3:0]      upper_o_s_hprot          ;
wire            upper_o_s_hmastlock      ;
wire            upper_o_s_hready         ;
wire [31:0]     upper_o_s_hwdata         ;
wire [UPPER_P_DMA_DW-1:0]     upper_o_m_hrdata         ;
wire [1:0]      upper_o_m_hresp          ;
wire            upper_o_m_hreadyout      ;
wire            upper_o_dma_m_awready    ;
wire            upper_o_dma_m_wready     ;
wire [3:0]      upper_o_dma_m_bid        ;
wire [1:0]      upper_o_dma_m_bresp      ;
wire            upper_o_dma_m_bvalid     ;
wire            upper_o_dma_m_arready    ;
wire [3:0]      upper_o_dma_m_rid        ;
wire [63:0]     upper_o_dma_m_rdata      ;
wire [1:0]      upper_o_dma_m_rresp      ;
wire            upper_o_dma_m_rlast      ;
wire            upper_o_dma_m_rvalid     ;
wire [31:0]     upper_o_prdata           ;
wire            upper_o_pslverr          ;
wire            upper_o_pready           ;
wire [31:0]     upper_o_otp_hrdata       ;
wire [1:0]      upper_o_otp_hresp        ;
wire            upper_o_otp_hreadyout    ;
wire [31:0]     upper_o_nvm_hrdata       ;
wire [1:0]      upper_o_nvm_hresp        ;
wire            upper_o_nvm_hreadyout    ;

wire [38:0]     ahb2kram_o_ram_rdata  ;

reg pll_clkr            ;
reg pll_lockedr         ;

reg             pll_rst_r           ;
reg             pll_rst_n           ;
reg             soc_mem_err_en      ;
reg             otp_err_en          ;
reg             soc_mem_err_respond ;
reg             otp_err_respond     ;
reg [1:0]       clk_div_cnt0        ;
reg             cpu_clk_2_div       ;
reg             cpu_clk_2_div_en    ;
reg             cpu_clk_3_div0      ;
reg             cpu_clk_3_div_en    ;
reg             cpu_clk_3_div       ;
reg [1:0]       clk_div_cnt1        ;
reg             cpu_clk_3_div1      ;

wire                                hsm_o_nvm_hsel       = 1'b0;
wire [31:0]                         hsm_o_nvm_haddr      = 32'b0;
wire [1:0]                          hsm_o_nvm_htrans     = 2'b0;
wire                                hsm_o_nvm_hwrite     = 1'b0;
wire [2:0]                          hsm_o_nvm_hburst     = 3'b0;
wire [2:0]                          hsm_o_nvm_hsize      = 3'b0;
wire [3:0]                          hsm_o_nvm_hprot      = 4'b0;
wire                                hsm_o_nvm_hmastlock  = 1'b0;
wire                                hsm_o_nvm_hready     = 1'b0;
wire [31:0]                         hsm_o_nvm_hwdata     = 32'b0;

wire [3:0]                          hsm_o_dma_m_wid      = 4'b0;

wire [63:0]                         hsm_o_hsm_status             ;
wire [63:0]                         hsm_o_hsm_err_hw             ;
wire [63:0]                         hsm_o_hsm_err_fw             ;
wire [16-1:0]                hsm_o_mbox_irq               ;
wire [31:0]                         hsm_o_s_hrdata               ;
wire [1:0]                          hsm_o_s_hresp                ;
wire                                hsm_o_s_hreadyout            ;
wire                                hsm_o_m_hsel                 ;
wire [63:0]                         hsm_o_m_haddr                ;
wire [1:0]                          hsm_o_m_htrans               ;
wire                                hsm_o_m_hwrite               ;
wire [2:0]                          hsm_o_m_hburst               ;
wire [2:0]                          hsm_o_m_hsize                ;
wire [3:0]                          hsm_o_m_hprot                ;
wire                                hsm_o_m_hmastlock            ;
wire                                hsm_o_m_hready               ;
wire [31:0]                         hsm_o_m_hwdata               ;
wire [3:0]                          hsm_o_dma_m_awid             ;
wire [64-1:0]              hsm_o_dma_m_awaddr           ;
wire [7:0]                          hsm_o_dma_m_awlen            ;
wire [2:0]                          hsm_o_dma_m_awsize           ;
wire [1:0]                          hsm_o_dma_m_awburst          ;
wire                                hsm_o_dma_m_awlock           ;
wire [3:0]                          hsm_o_dma_m_awcache          ;
wire [2:0]                          hsm_o_dma_m_awprot           ;
wire [3:0]                          hsm_o_dma_m_awqos            ;
wire [3:0]                          hsm_o_dma_m_awregion         ;
wire                                hsm_o_dma_m_awvalid          ;
wire [64-1:0]              hsm_o_dma_m_wdata            ;
wire [64/8-1:0]            hsm_o_dma_m_wstrb            ;
wire                                hsm_o_dma_m_wlast            ;
wire                                hsm_o_dma_m_wvalid           ;
wire                                hsm_o_dma_m_bready           ;
wire [3:0]                          hsm_o_dma_m_arid             ;
wire [64-1:0]              hsm_o_dma_m_araddr           ;
wire [7:0]                          hsm_o_dma_m_arlen            ;
wire [2:0]                          hsm_o_dma_m_arsize           ;
wire [1:0]                          hsm_o_dma_m_arburst          ;
wire                                hsm_o_dma_m_arlock           ;
wire [3:0]                          hsm_o_dma_m_arcache          ;
wire [2:0]                          hsm_o_dma_m_arprot           ;
wire [3:0]                          hsm_o_dma_m_arqos            ;
wire [3:0]                          hsm_o_dma_m_arregion         ;
wire                                hsm_o_dma_m_arvalid          ;
wire                                hsm_o_dma_m_rready           ;
wire                                hsm_o_cfg_hsel               ;
wire [31:0]                         hsm_o_cfg_haddr              ;
wire [1:0]                          hsm_o_cfg_htrans             ;
wire                                hsm_o_cfg_hwrite             ;
wire [2:0]                          hsm_o_cfg_hburst             ;
wire [2:0]                          hsm_o_cfg_hsize              ;
wire [3:0]                          hsm_o_cfg_hprot              ;
wire                                hsm_o_cfg_hmastlock          ;
wire                                hsm_o_cfg_hready             ;
wire [31:0]                         hsm_o_cfg_hwdata             ;
wire                                hsm_o_otp_hsel               ;
wire [31:0]                         hsm_o_otp_haddr              ;
wire [1:0]                          hsm_o_otp_htrans             ;
wire                                hsm_o_otp_hwrite             ;
wire [2:0]                          hsm_o_otp_hburst             ;
wire [2:0]                          hsm_o_otp_hsize              ;
wire [3:0]                          hsm_o_otp_hprot              ;
wire                                hsm_o_otp_hmastlock          ;
wire                                hsm_o_otp_hready             ;
wire [31:0]                         hsm_o_otp_hwdata             ;
wire                                hsm_o_irom_CK                ;
wire                                hsm_o_irom_CSB               ;
wire [14-1:0]    hsm_o_irom_A                 ;
`ifdef OSR_FPGA_ROM  
wire [38:0]                         hsm_o_irom_DI                ;
wire                                hsm_o_irom_WEB               ;
`endif
wire                                hsm_o_iram_CK                ;
wire [16-1:0]    hsm_o_iram_A                 ;
wire                                hsm_o_iram_CSB               ;
wire                                hsm_o_iram_WEB               ;
wire [38:0]                         hsm_o_iram_DI                ;
wire                                hsm_o_dram_CK                ;
wire [14-1:0]    hsm_o_dram_A                 ;
wire                                hsm_o_dram_CSB               ;
wire                                hsm_o_dram_WEB               ;
wire [38:0]                         hsm_o_dram_DI                ;
wire                                hsm_o_kbuf_CK                ;
wire                                hsm_o_kbuf_CSB               ;
wire [7:0]                          hsm_o_kbuf_A                 ;
wire                                hsm_o_kbuf_WEB               ;
wire [38:0]                         hsm_o_kbuf_DI                ;
wire                                hsm_o_pke_sram0_CK           ;
wire                                hsm_o_pke_sram0_CSAN         ;
wire [8:0]                          hsm_o_pke_sram0_A            ;
wire                                hsm_o_pke_sram0_CSBN         ;
wire [8:0]                          hsm_o_pke_sram0_B            ;
wire [71:0]                         hsm_o_pke_sram0_DI           ;
wire                                hsm_o_pke_sram1_CK           ;
wire                                hsm_o_pke_sram1_CSAN         ;
wire [8:0]                          hsm_o_pke_sram1_A            ;
wire                                hsm_o_pke_sram1_CSBN         ;
wire [8:0]                          hsm_o_pke_sram1_B            ;
wire [71:0]                         hsm_o_pke_sram1_DI           ;
wire                                hsm_o_pke_sram2_CK           ;
wire                                hsm_o_pke_sram2_CSAN         ;
wire [8:0]                          hsm_o_pke_sram2_A            ;
wire                                hsm_o_pke_sram2_CSBN         ;
wire [8:0]                          hsm_o_pke_sram2_B            ;
wire [71:0]                         hsm_o_pke_sram2_DI           ;
wire                                hsm_o_pke_sram3_CK           ;
wire                                hsm_o_pke_sram3_CSAN         ;
wire [8:0]                          hsm_o_pke_sram3_A            ;
wire                                hsm_o_pke_sram3_CSBN         ;
wire [8:0]                          hsm_o_pke_sram3_B            ;
wire [71:0]                         hsm_o_pke_sram3_DI           ;
wire                                hsm_o_tdo_oe                 ;
wire                                hsm_o_tdo                    ;
wire                                hsm_o_uart_txd_oe            ;
wire                                hsm_o_uart_txd               ;
wire [127:0]                        hsm_o_soc_dbg_en_128b        ;
wire [3:0]                          hsm_o_trng_ro_clk            ;
wire [3:0]                          hsm_o_trng_ro_out            ;

wire [11:0]    dbg_tdt_dmi_paddr       = 12'h0  ; 
wire           dbg_tdt_dmi_penable     = 1'h0   ;
wire           dbg_tdt_dmi_psel        = 1'h0   ;
wire [31:0]    dbg_tdt_dmi_pwdata      = 32'h0  ;
wire           dbg_tdt_dmi_pwrite      = 1'h0   ;
wire           dbg_tdt_dtm_pad_tdo     = 1'h0   ;
wire           dbg_tdt_dtm_pad_tdo_en  = 1'h0   ;

wire              x2x_awready_m  = 1'h0 ;   
wire              x2x_wready_m   = 1'h0 ; 
wire              x2x_bvalid_m   = 1'h0 ; 
wire [4-1:0]      x2x_bid_m      = 4'h0 ; 
wire [2-1:0]      x2x_bresp_m    = 2'h0 ; 
wire              x2x_arready_m  = 1'h0 ; 
wire              x2x_rvalid_m   = 1'h0 ; 
wire [4-1:0]      x2x_rid_m      = 4'h0 ; 
wire [128-1:0]    x2x_rdata_m    = 128'h0 ;
wire              x2x_rlast_m    = 1'h0 ; 
wire [2-1:0]      x2x_rresp_m    = 2'h0 ; 
wire              x2x_awvalid_s1 = 1'h0 ; 
wire [64-1:0]     x2x_awaddr_s1  = 64'h0 ;
wire [4-1:0]      x2x_awid_s1    = 4'h0 ; 
wire [4-1:0]      x2x_awlen_s1   = 4'h0 ; 
wire [3-1:0]      x2x_awsize_s1  = 3'h0 ; 
wire [2-1:0]      x2x_awburst_s1 = 2'h0 ; 
wire [2-1:0]      x2x_awlock_s1  = 2'h0 ; 
wire [4-1:0]      x2x_awcache_s1 = 4'h0 ; 
wire [3-1:0]      x2x_awprot_s1  = 3'h0 ; 
wire              x2x_wvalid_s1  = 1'h0 ; 
wire [4-1:0]      x2x_wid_s1     = 4'h0 ; 
wire [64-1:0]     x2x_wdata_s1   = 64'h0 ;
wire [8-1:0]      x2x_wstrb_s1   = 8'h0 ; 
wire              x2x_wlast_s1   = 1'h0 ; 
wire              x2x_bready_s1  = 1'h0 ; 
wire              x2x_arvalid_s  = 1'h0 ; 
wire [4-1:0]      x2x_arid_s     = 4'h0 ; 
wire [64-1:0]     x2x_araddr_s   = 64'h0 ;
wire [4-1:0]      x2x_arlen_s    = 4'h0 ; 
wire [3-1:0]      x2x_arsize_s   = 3'h0 ; 
wire [2-1:0]      x2x_arburst_s  = 2'h0 ; 
wire [2-1:0]      x2x_arlock_s   = 2'h0 ; 
wire [4-1:0]      x2x_arcache_s  = 4'h0 ; 
wire [3-1:0]      x2x_arprot_s   = 3'h0 ; 
wire              x2x_rready_s   = 1'h0 ; 

wire [31:0]                                       ahb2kram_o_hrdata         ;
wire                                              ahb2kram_o_hreadyout      ;
wire [1:0]                                        ahb2kram_o_hresp          ;
wire                                              ahb2kram_o_ram_cs         ;
wire [3:0]                                        ahb2kram_o_ram_wen        ;
wire [AHB2KRAM_P_BUS_AW-AHB2KRAM_P_AW_DEL-1:0]    ahb2kram_o_ram_addr       ;
wire [31:0]                                       ahb2kram_o_ram_wdata      ;

wire [31:0]    upper_n205_o_sensor         ;
wire [31:0]    upper_n205_o_soc_status     ;
wire [31:0]    upper_n205_o_soc_err        ;
wire           upper_n205_o_tdo_oe         ;
wire           upper_n205_o_tdo            ;
wire           upper_n205_o_m_hsel         ;
wire [63:0]    upper_n205_o_m_haddr        ;
wire [1:0]     upper_n205_o_m_htrans       ;
wire           upper_n205_o_m_hwrite       ;
wire [2:0]     upper_n205_o_m_hburst       ;
wire [2:0]     upper_n205_o_m_hsize        ;
wire [3:0]     upper_n205_o_m_hprot        ;
wire           upper_n205_o_m_hmastlock    ;
wire           upper_n205_o_m_hready       ;
wire [31:0]    upper_n205_o_m_hwdata       ;
wire [31:0]    upper_n205_o_otp_hrdata     ;
wire [1:0]     upper_n205_o_otp_hresp      ;
wire           upper_n205_o_otp_hreadyout  ;
wire [31:0]    upper_n205_o_soc_hrdata     ;
wire [1:0]     upper_n205_o_soc_hresp      ;
wire           upper_n205_o_soc_hreadyout  ;
wire           upper_n205_s_axi_awready    ;
wire           upper_n205_s_axi_wready     ;
wire [7:0]     upper_n205_s_axi_bid        ;
wire [1:0]     upper_n205_s_axi_bresp      ;
wire           upper_n205_s_axi_bvalid     ;
wire           upper_n205_s_axi_arready    ;
wire [7:0]     upper_n205_s_axi_rid        ;
wire [63:0]    upper_n205_s_axi_rdata      ;
wire [1:0]     upper_n205_s_axi_rresp      ;
wire           upper_n205_s_axi_rvalid     ;
wire           upper_n205_s_axi_rlast      ;
wire           upper_n205_o_dut_clk        ;
wire           upper_n205_o_dut_rst_n      ;

wire            upper_o_swdo             ;
wire            upper_o_swdoen           ;
wire            upper_o_tdo              ;
wire            upper_o_tdo_oen          ;
wire            upper_o_jtagnsw          ;
wire            upper_o_swv              ;
wire            upper_o_traceclk         ;
wire [3:0]      upper_o_tracedata        ;
wire [31:0]     upper_o_hrdata_s_ext0    ;
wire            upper_o_hreadyout_s_ext0 ;
wire [1:0]      upper_o_hresp_s_ext0     ;
wire [2:0]      upper_o_hruser_s_ext0    ;
wire [31:0]     upper_o_hrdata_s_ext1    ;
wire            upper_o_hreadyout_s_ext1 ;
wire [1:0]      upper_o_hresp_s_ext1     ;
wire [2:0]      upper_o_hruser_s_ext1    ;
wire [31:0]     upper_o_hrdata_s_ext2    ;
wire            upper_o_hreadyout_s_ext2 ;
wire [1:0]      upper_o_hresp_s_ext2     ;
wire [2:0]      upper_o_hruser_s_ext2    ;
wire [31:0]     upper_o_hrdata_s_ext3    ;
wire            upper_o_hreadyout_s_ext3 ;
wire [1:0]      upper_o_hresp_s_ext3     ;
wire [2:0]      upper_o_hruser_s_ext3    ;
wire [31:0]     upper_o_hrdata_s_ext4    ;
wire            upper_o_hreadyout_s_ext4 ;
wire [1:0]      upper_o_hresp_s_ext4     ;
wire [2:0]      upper_o_hruser_s_ext4    ;
wire            upper_o_hsel_m_ext0      ;
wire [63:0]     upper_o_haddr_m_ext0     ;
wire [1:0]      upper_o_htrans_m_ext0    ;
wire            upper_o_hwrite_m_ext0    ;
wire [2:0]      upper_o_hsize_m_ext0     ;
wire [2:0]      upper_o_hburst_m_ext0    ;
wire [3:0]      upper_o_hprot_m_ext0     ;
wire [3:0]      upper_o_hmaster_m_ext0   ;
wire [31:0]     upper_o_hwdata_m_ext0    ;
wire            upper_o_hmastlock_m_ext0 ;
wire            upper_o_hreadymux_m_ext0 ;
wire [2:0]      upper_o_hauser_m_ext0    ;
wire [2:0]      upper_o_hwuser_m_ext0    ;
wire            upper_s_axi_awready      ;
wire            upper_s_axi_wready       ;
wire [7:0]      upper_s_axi_bid          ;
wire [1:0]      upper_s_axi_bresp        ;
wire            upper_s_axi_bvalid       ;
wire            upper_s_axi_arready      ;
wire [7:0]      upper_s_axi_rid          ;
wire [UPPER_P_DMA_DW-1:0]     upper_s_axi_rdata        ;
wire [1:0]      upper_s_axi_rresp        ;
wire            upper_s_axi_rvalid       ;
wire            upper_s_axi_rlast        ;
wire            upper_o_hsel_m_ext2      ;
wire [63:0]     upper_o_haddr_m_ext2     ;
wire [1:0]      upper_o_htrans_m_ext2    ;
wire            upper_o_hwrite_m_ext2    ;
wire [2:0]      upper_o_hsize_m_ext2     ;
wire [2:0]      upper_o_hburst_m_ext2    ;
wire [3:0]      upper_o_hprot_m_ext2     ;
wire [3:0]      upper_o_hmaster_m_ext2   ;
wire [31:0]     upper_o_hwdata_m_ext2    ;
wire            upper_o_hmastlock_m_ext2 ;
wire            upper_o_hreadymux_m_ext2 ;
wire [2:0]      upper_o_hauser_m_ext2    ;
wire [2:0]      upper_o_hwuser_m_ext2    ;
wire [15:0]     upper_o_portout          ;
wire [15:0]     upper_o_porten           ;
wire [15:0]     upper_o_portfunc         ;
wire            upper_o_spi_ss_o         ;
wire            upper_o_spi_clk_o        ;
wire            upper_o_spi_ctl_oen      ;
wire            upper_o_spi_dout         ;
wire            upper_o_spi_dout_oen     ;
wire            upper_o_uart_txd         ;
wire            upper_o_uart1_txd        ;
wire            upper_o_uart_txd_oe      ;
wire            upper_o_uart1_txd_oe     ;
wire            upper_o_dut_clk          ;
wire            upper_o_dut_rst_n        ;
wire [255:0]    upper_o_dut_ctrl         ;
wire [255:0]    upper_o_dut_ctrl_ext     ;

wire [31:0]    irom_DO  ;

wire [31:0]    iram_DO  ;

wire [31:0]    dram_DO  ;

wire [31:0]    kram_DO  ;

wire [63:0]    pram0_DO   ;
wire [63:0]    pram1_DO   ;
wire [63:0]    pram2_DO   ;
wire [63:0]    pram3_DO   ;

wire [38:0]    irom_ecc_DO  ;

wire [38:0]    iram_ecc_DO  ;

wire [38:0]    dram_ecc_DO  ;

wire [38:0]    kram_ecc_DO  ;

wire [71:0]    pram0_ecc_DO   ;
wire [71:0]    pram1_ecc_DO   ;
wire [71:0]    pram2_ecc_DO   ;
wire [71:0]    pram3_ecc_DO   ;

wire [63:0] nvm_addr = {32'h0,20'h6007C,hsm_o_nvm_haddr[11:0]};

wire pll_rst_r_next = pll_locked; 
wire pll_rst_n_next = pll_rst_r; 

wire soc_mem_err_en_next = (upper_o_dut_ctrl[71:64] == 8'h5a); 
wire otp_err_en_next = (upper_o_dut_ctrl[79:72] == 8'h5a); 
wire soc_mem_err_respond_next = (soc_mem_err_en & hsm_o_m_htrans[1] & (upper_o_dut_ctrl[127:96] == hsm_o_m_haddr[31:0])) ? 1'b1: (soc_mem_err_en & upper_o_m_hreadyout) ? 1'b0 : soc_mem_err_en ? soc_mem_err_respond: 1'b0;
wire otp_err_respond_next = (otp_err_en & hsm_o_otp_htrans[1] & (upper_o_dut_ctrl[159:128] == hsm_o_otp_haddr[31:0])) ? 1'b1 :(otp_err_en & upper_o_otp_hreadyout) ? 1'b0 : otp_err_en ? otp_err_respond: 1'b0;

wire [1:0]clk_div_cnt0_next =(clk_div_cnt0 == 2'h2) ? 2'b0 : (clk_div_cnt0+1'b1);
wire cpu_clk_2_div_next = ~cpu_clk_2_div;
wire cpu_clk_2_div_en_next = ~cpu_clk_2_div_en;
wire cpu_clk_3_div0_next = (clk_div_cnt0 == 0) ? ~cpu_clk_3_div0 : cpu_clk_3_div0;
wire cpu_clk_3_div_en_next = (clk_div_cnt0 == 2'h2) ? 1'b1 : 1'b0;
wire cpu_clk_3_div_next = cpu_clk_3_div_en ? 1'b1 : 1'b0;

wire [1:0]clk_div_cnt1_next =(clk_div_cnt1 == 2'h2) ? 2'b0 : (clk_div_cnt1+1'b1);
wire cpu_clk_3_div1_next = (clk_div_cnt1 == 2'h2) ? ~cpu_clk_3_div1 : cpu_clk_3_div1;

assign cpu_clk_div_en = cpu_clk_3_div_en;

`ifndef OSR_SIM_PLL

IBUFDS #(
    .DIFF_TERM("FALSE"),
    .IOSTANDARD("DIFF_SSTL2")
) IBUFDS_inst (
    .I(OSCCLK),
    .IB(OSCCLK_N),
    .O(clk_buf)
);

MMCME2_ADV #(    .CLKIN1_PERIOD(5),                 
                 .DIVCLK_DIVIDE(CLK_IN_DIVIDE),     
                 .CLKFBOUT_MULT_F(VCO_MULT),        
                 .CLKOUT0_DIVIDE_F(DIV0),           
                 .CLKOUT0_PHASE(0.0), 
                 .CLKOUT1_DIVIDE(DIV1),             
                 .CLKOUT1_PHASE(0.0),   
                 .COMPENSATION("ZHOLD")             
            ) mmcm_inst (    
                 .CLKIN1(clk_buf),                  
                 .CLKFBOUT(clkfbout),               
                 .CLKFBIN(clkfbout_buf),            
                 .CLKOUT0(clk_out0),                
                 .CLKOUT1(clk_out1),                
                 .LOCKED(pll_locked),               
                 .RST(~RST_N)                       
            );

BUFG bufg_feedback (
    .I(clkfbout),
    .O(clkfbout_buf)
);

BUFG bufg_clkout0 (
    .I(clk_out0),
    .O(pll_clk)
);

BUFG bufg_clkout1 (
    .I(clk_out1),
    .O(test_clk)
);

`else
    initial
    begin
        pll_clkr= 1'b0;
        forever
        begin
            #((1_000_000_000.000/`OSR_SIM_CLK_FREQ)/2.000 * 1ns) pll_clkr = ~pll_clkr;     
        end
    end

    initial begin
        force pll_locked = 1'h0;
        #11ns;
        release pll_locked;
    end

    assign pll_clk = pll_clkr;

    assign #10ns pll_locked = 1'b1;

assign cpu_clk_div = cpu_clk_3_div;

`endif

`ifdef OSR_FPGA_PWR_CTL
    pwr_i2c_top u_pwr_i2c_top (
        .sys_clk   (pll_clk         ),
        .rst_n     (RST_N           ),
        .test_en1  (PWR_PWR_TEST_EN ),
        .pwr_en    (PWR_PWR_EN      ),
        .pwr_scl   (PWR_SCL         ),
        .pwr_sda   (PWR_SDA         )
    );
`endif

assign upper_o_clk             = upper_o_dut_clk  ;
assign upper_o_rst_n           = upper_o_dut_rst_n;
assign upper_o_rtc_clk         = upper_o_dut_clk  ;
assign upper_o_rtc_rst_n       = upper_o_dut_rst_n;
assign upper_o_scan_mode       = 'h0              ;
assign upper_o_icg_scan_en     = 'h0              ;
assign upper_o_sensor          = upper_o_dut_ctrl[191:160];
assign upper_o_soc_status      = upper_o_dut_ctrl[223:192];
assign upper_o_soc_err         = upper_o_dut_ctrl[255:224];
assign upper_o_s_hsel          = upper_o_hsel_m_ext0     ;
assign upper_o_s_haddr         = upper_o_haddr_m_ext0    ;
assign upper_o_s_htrans        = upper_o_htrans_m_ext0   ;
assign upper_o_s_hwrite        = upper_o_hwrite_m_ext0   ;
assign upper_o_s_hburst        = upper_o_hburst_m_ext0   ;
assign upper_o_s_hsize         = upper_o_hsize_m_ext0    ;
assign upper_o_s_hprot         = upper_o_hprot_m_ext0    ;
assign upper_o_s_hmastlock     = upper_o_hmastlock_m_ext0;
assign upper_o_s_hready        = upper_o_hreadymux_m_ext0;
assign upper_o_s_hwdata        = upper_o_hwdata_m_ext0   ;
assign upper_o_m_hrdata        = upper_o_hrdata_s_ext1   ;
assign upper_o_m_hresp         = upper_o_hresp_s_ext1    ;
assign upper_o_m_hreadyout     = upper_o_hreadyout_s_ext1;
assign upper_o_dma_m_awready   = upper_s_axi_awready     ;
assign upper_o_dma_m_wready    = upper_s_axi_wready      ;
assign upper_o_dma_m_bid       = upper_s_axi_bid         ;
assign upper_o_dma_m_bresp     = upper_s_axi_bresp       ;
assign upper_o_dma_m_bvalid    = upper_s_axi_bvalid      ;
assign upper_o_dma_m_arready   = upper_s_axi_arready     ;
assign upper_o_dma_m_rid       = upper_s_axi_rid         ;
assign upper_o_dma_m_rdata     = upper_s_axi_rdata       ;
assign upper_o_dma_m_rresp     = upper_s_axi_rresp       ;
assign upper_o_dma_m_rlast     = upper_s_axi_rlast       ;
assign upper_o_dma_m_rvalid    = upper_s_axi_rvalid      ;
assign upper_o_prdata          = 'h0  ;
assign upper_o_pslverr         = 'h0  ;
assign upper_o_pready          = 1'h1 ;
assign upper_o_otp_hrdata      = upper_o_hrdata_s_ext2    ;
assign upper_o_otp_hresp       = upper_o_hresp_s_ext2     ;
assign upper_o_otp_hreadyout   = upper_o_hreadyout_s_ext2 ;
assign upper_o_nvm_hrdata      = upper_o_hrdata_s_ext3    ;
assign upper_o_nvm_hresp       = upper_o_hresp_s_ext3     ;
assign upper_o_nvm_hreadyout   = upper_o_hreadyout_s_ext3 ;

wire                                hsm_i_clk                    = upper_o_clk              ;
wire                                hsm_i_rst_n                  = upper_o_rst_n            ;

wire                                hsm_i_scan_mode              = upper_o_scan_mode        ;

wire                                hsm_i_scan_icg_enable        = upper_o_icg_scan_en      ; 

wire [31:0]                         hsm_i_soc_status             = upper_o_soc_status       ;
wire [31:0]                         hsm_i_soc_err                = upper_o_soc_err          ;

wire                                hsm_i_s_hsel                 = upper_o_s_hsel           ;
wire [31:0]                         hsm_i_s_haddr                = upper_o_s_haddr          ;
wire [1:0]                          hsm_i_s_htrans               = upper_o_s_htrans         ;
wire                                hsm_i_s_hwrite               = upper_o_s_hwrite         ;
wire [2:0]                          hsm_i_s_hburst               = upper_o_s_hburst         ;
wire [2:0]                          hsm_i_s_hsize                = upper_o_s_hsize          ;
wire [3:0]                          hsm_i_s_hprot                = upper_o_s_hprot          ;
wire                                hsm_i_s_hmastlock            = upper_o_s_hmastlock      ;
wire                                hsm_i_s_hready               = upper_o_s_hready         ;
wire [31:0]                         hsm_i_s_hwdata               = upper_o_s_hwdata         ;

wire [31:0]                         hsm_i_m_hrdata               = upper_o_m_hrdata         ;
wire [1:0]                          hsm_i_m_hresp                = !soc_mem_err_en ? upper_o_m_hresp : {1'b0,soc_mem_err_respond}  ;
wire                                hsm_i_m_hreadyout            = upper_o_m_hreadyout      ;

wire                                hsm_i_dma_m_awready          = upper_o_dma_m_awready ;
wire                                hsm_i_dma_m_wready           = upper_o_dma_m_wready     ;
wire [3:0]                          hsm_i_dma_m_bid              = upper_o_dma_m_bid        ;
wire [1:0]                          hsm_i_dma_m_bresp            = upper_o_dma_m_bresp      ;
wire                                hsm_i_dma_m_bvalid           = upper_o_dma_m_bvalid     ;
wire                                hsm_i_dma_m_arready          = upper_o_dma_m_arready    ;
wire [3:0]                          hsm_i_dma_m_rid              = upper_o_dma_m_rid        ;
wire [64-1:0]              hsm_i_dma_m_rdata            = upper_o_dma_m_rdata      ;
wire [1:0]                          hsm_i_dma_m_rresp            = upper_o_dma_m_rresp      ;
wire                                hsm_i_dma_m_rlast            = upper_o_dma_m_rlast      ;
wire                                hsm_i_dma_m_rvalid           = upper_o_dma_m_rvalid     ;

wire [31:0]                         hsm_i_cfg_hrdata             = upper_o_hrdata_s_ext0;
wire [1:0]                          hsm_i_cfg_hresp              = upper_o_hresp_s_ext0;
wire                                hsm_i_cfg_hreadyout          = upper_o_hreadyout_s_ext0;

wire [31:0]                         hsm_i_otp_hrdata             = upper_o_otp_hrdata        ;
wire [1:0]                          hsm_i_otp_hresp              = !otp_err_en ? upper_o_otp_hresp : {1'b0,otp_err_respond}  ;
wire                                hsm_i_otp_hreadyout          = upper_o_otp_hreadyout     ;

wire [38:0]                         hsm_i_irom_DO                = irom_ecc_DO              ;
`ifdef OSR_FPGA_ROM  
`endif

wire [38:0]                         hsm_i_iram_DO                = iram_ecc_DO ;

wire [38:0]                         hsm_i_dram_DO                = dram_ecc_DO              ;

wire [38:0]                         hsm_i_kbuf_DO                = kram_ecc_DO              ;

wire [71:0]                         hsm_i_pke_sram0_DO           = pram0_ecc_DO             ;
wire [71:0]                         hsm_i_pke_sram1_DO           = pram1_ecc_DO             ;
wire [71:0]                         hsm_i_pke_sram2_DO           = pram2_ecc_DO             ;
wire [71:0]                         hsm_i_pke_sram3_DO           = pram3_ecc_DO             ;

wire                                hsm_i_trst_n                 = 1'h1                     ; 
wire                                hsm_i_tck                    = i_hsm_tck                ;
wire                                hsm_i_tms                    = i_hsm_tms                ;
wire                                hsm_i_tdi                    = i_hsm_tdi                ;

wire                                hsm_i_uart_rxd               = i_hsm_uart_rxd           ;

wire                                              ahb2kram_i_hclk           = pll_clk                    ;
wire                                              ahb2kram_i_hresetn        = pll_rst_n                  ;
wire [7:0]                                        ahb2kram_i_ecc_err_rsp_en = 8'h0                       ;
wire                                              ahb2kram_i_ecc_dec_sec    = 1'h0                       ;
wire                                              ahb2kram_i_ecc_dec_ded    = 1'h0                       ;
wire                                              ahb2kram_i_hsel           = upper_o_hsel_m_ext2        ;
wire                                              ahb2kram_i_hready         = upper_o_hreadymux_m_ext2   ;
wire [AHB2KRAM_P_BUS_AW-1:0]                      ahb2kram_i_haddr          = upper_o_haddr_m_ext2       ;
wire [1:0]                                        ahb2kram_i_htrans         = upper_o_htrans_m_ext2      ;
wire                                              ahb2kram_i_hwrite         = upper_o_hwrite_m_ext2      ;
wire [2:0]                                        ahb2kram_i_hsize          = upper_o_hsize_m_ext2       ;
wire [2:0]                                        ahb2kram_i_hburst         = 3'h0                       ;
wire [3:0]                                        ahb2kram_i_hprot          = 4'h0                       ;
wire [3:0]                                        ahb2kram_i_hmaster        = 4'h0                       ;
wire [31:0]                                       ahb2kram_i_hwdata         = upper_o_hwdata_m_ext2      ;
wire                                              ahb2kram_i_hmastlock      = 1'h0                       ;
wire [31:0]                                       ahb2kram_i_ram_rdata      = ahb2kram_o_ram_rdata[31:0] ;

reg r_dut_rst;
reg r_pll_lock;
always @(posedge upper_o_dut_clk or negedge upper_o_dut_rst_n) begin
    if(!upper_o_dut_rst_n) begin
        r_dut_rst   <= 1'h0;
        r_pll_lock  <= 1'h0;
    end else begin
        r_dut_rst   <= 1'h1;
        r_pll_lock  <= pll_locked;
    end
end

wire            upper_i_osc_clk          = pll_clk  ;
wire            upper_i_por_rst_n        = pll_rst_n;
wire            upper_i_soc_rst_n        = pll_rst_n;
wire            upper_i_ext_rst_n        = pll_rst_n;
wire            upper_i_pll_locked       = pll_locked;
wire            upper_i_trst_n           = 1'h1;
wire            upper_i_tms_swdi         = i_upper_CS_TMS;
wire            upper_i_tck_swclk        = i_upper_CS_TCK;
wire            upper_i_tdi              = i_upper_tdi   ;
wire            upper_i_hsel_s_ext0      = hsm_o_cfg_hsel;
wire [63:0]     upper_i_haddr_s_ext0     = {32'h0,14'h1801,hsm_o_cfg_haddr[17:0]};
wire [1:0]      upper_i_htrans_s_ext0    = hsm_o_cfg_htrans;
wire            upper_i_hwrite_s_ext0    = hsm_o_cfg_hwrite;
wire [2:0]      upper_i_hsize_s_ext0     = hsm_o_cfg_hsize;
wire [2:0]      upper_i_hburst_s_ext0    = hsm_o_cfg_hburst;
wire [3:0]      upper_i_hprot_s_ext0     = hsm_o_cfg_hprot;
wire [3:0]      upper_i_hmaster_s_ext0   = 4'h0;
wire [31:0]     upper_i_hwdata_s_ext0    = hsm_o_cfg_hwdata;
wire            upper_i_hmastlock_s_ext0 = hsm_o_cfg_hmastlock;
wire            upper_i_hready_s_ext0    = hsm_o_cfg_hready;
wire [2:0]      upper_i_hauser_s_ext0    = 3'h0;
wire [2:0]      upper_i_hwuser_s_ext0    = 3'h0;
wire            upper_i_hsel_s_ext1      = hsm_o_m_hsel      ;
wire [63:0]     upper_i_haddr_s_ext1     = hsm_o_m_haddr[31:0]     ;
wire [1:0]      upper_i_htrans_s_ext1    = hsm_o_m_htrans    ;
wire            upper_i_hwrite_s_ext1    = hsm_o_m_hwrite    ;
wire [2:0]      upper_i_hsize_s_ext1     = hsm_o_m_hsize     ;
wire [2:0]      upper_i_hburst_s_ext1    = hsm_o_m_hburst    ;
wire [3:0]      upper_i_hprot_s_ext1     = hsm_o_m_hprot     ;
wire [3:0]      upper_i_hmaster_s_ext1   = 4'h0              ;
wire [31:0]     upper_i_hwdata_s_ext1    = hsm_o_m_hwdata    ;
wire            upper_i_hmastlock_s_ext1 = hsm_o_m_hmastlock ;
wire            upper_i_hready_s_ext1    = hsm_o_m_hready    ;
wire [2:0]      upper_i_hauser_s_ext1    = 3'h0;
wire [2:0]      upper_i_hwuser_s_ext1    = 3'h0;
wire            upper_i_hsel_s_ext2      = hsm_o_otp_hsel      ;
wire [63:0]     upper_i_haddr_s_ext2     = {32'h0,20'h6007C,hsm_o_otp_haddr[11:0]};
wire [1:0]      upper_i_htrans_s_ext2    = hsm_o_otp_htrans    ;
wire            upper_i_hwrite_s_ext2    = hsm_o_otp_hwrite    ;
wire [2:0]      upper_i_hsize_s_ext2     = hsm_o_otp_hsize     ;
wire [2:0]      upper_i_hburst_s_ext2    = hsm_o_otp_hburst    ;
wire [3:0]      upper_i_hprot_s_ext2     = hsm_o_otp_hprot     ;
wire [3:0]      upper_i_hmaster_s_ext2   = 'h0                 ;
wire [31:0]     upper_i_hwdata_s_ext2    = hsm_o_otp_hwdata    ;
wire            upper_i_hmastlock_s_ext2 = hsm_o_otp_hmastlock ;
wire            upper_i_hready_s_ext2    = hsm_o_otp_hready    ;
wire [2:0]      upper_i_hauser_s_ext2    = 'h0;
wire [2:0]      upper_i_hwuser_s_ext2    = 'h0;
wire            upper_i_hsel_s_ext3      = hsm_o_nvm_hsel      ;
wire [63:0]     upper_i_haddr_s_ext3     = nvm_addr            ;
wire [1:0]      upper_i_htrans_s_ext3    = hsm_o_nvm_htrans    ;
wire            upper_i_hwrite_s_ext3    = hsm_o_nvm_hwrite    ;
wire [2:0]      upper_i_hsize_s_ext3     = hsm_o_nvm_hsize     ;
wire [2:0]      upper_i_hburst_s_ext3    = hsm_o_nvm_hburst    ;
wire [3:0]      upper_i_hprot_s_ext3     = hsm_o_nvm_hprot     ;
wire [3:0]      upper_i_hmaster_s_ext3   = 'h0;
wire [31:0]     upper_i_hwdata_s_ext3    = hsm_o_nvm_hwdata    ;
wire            upper_i_hmastlock_s_ext3 = hsm_o_nvm_hmastlock ;
wire            upper_i_hready_s_ext3    = hsm_o_nvm_hready    ;
wire [2:0]      upper_i_hauser_s_ext3    = 'h0;
wire [2:0]      upper_i_hwuser_s_ext3    = 'h0;
wire            upper_i_hsel_s_ext4      = 'h0;
wire [63:0]     upper_i_haddr_s_ext4     = 'h0;
wire [1:0]      upper_i_htrans_s_ext4    = 'h0;
wire            upper_i_hwrite_s_ext4    = 'h0;
wire [2:0]      upper_i_hsize_s_ext4     = 'h0;
wire [2:0]      upper_i_hburst_s_ext4    = 'h0;
wire [3:0]      upper_i_hprot_s_ext4     = 'h0;
wire [3:0]      upper_i_hmaster_s_ext4   = 'h0;
wire [31:0]     upper_i_hwdata_s_ext4    = 'h0;
wire            upper_i_hmastlock_s_ext4 = 'h0;
wire            upper_i_hready_s_ext4    = 'h1;
wire [2:0]      upper_i_hauser_s_ext4    = 'h0;
wire [2:0]      upper_i_hwuser_s_ext4    = 'h0;
wire [31:0]     upper_i_hrdata_m_ext0    = hsm_o_s_hrdata    ;
wire            upper_i_hreadyout_m_ext0 = hsm_o_s_hreadyout ;
wire [1:0]      upper_i_hresp_m_ext0     = hsm_o_s_hresp     ;
wire [2:0]      upper_i_hruser_m_ext0    = 'h0;
wire [7:0]      upper_s_axi_awid         = hsm_o_dma_m_awid   ;
wire [7:0]      upper_s_axi_awlen        = hsm_o_dma_m_awlen  ;
wire [2:0]      upper_s_axi_awsize       = hsm_o_dma_m_awsize ;
wire [1:0]      upper_s_axi_awburst      = hsm_o_dma_m_awburst;
wire [3:0]      upper_s_axi_awcache      = hsm_o_dma_m_awcache;
wire [UPPER_P_DMA_AW-1:0]     upper_s_axi_awaddr       ;
generate
if(64 == 64)begin
assign upper_s_axi_awaddr       = {hsm_o_dma_m_awaddr[63:32],4'h6,hsm_o_dma_m_awaddr[27:0]};
end
else begin
assign upper_s_axi_awaddr       = {4'h6,hsm_o_dma_m_awaddr[27:0]};
end
endgenerate
wire [2:0]      upper_s_axi_awprot       = hsm_o_dma_m_awprot ;
wire            upper_s_axi_awvalid      = hsm_o_dma_m_awvalid;
wire            upper_s_axi_awlock       = hsm_o_dma_m_awlock ;
wire [7:0]      upper_s_axi_wid          = hsm_o_dma_m_wid    ;
wire [UPPER_P_DMA_DW-1:0]     upper_s_axi_wdata        = hsm_o_dma_m_wdata  ;
wire [UPPER_P_DMA_DW/8-1:0]   upper_s_axi_wstrb        = hsm_o_dma_m_wstrb  ;
wire            upper_s_axi_wlast        = hsm_o_dma_m_wlast  ;
wire            upper_s_axi_wvalid       = hsm_o_dma_m_wvalid ;
wire            upper_s_axi_bready       = hsm_o_dma_m_bready ;
wire [7:0]      upper_s_axi_arid         = hsm_o_dma_m_arid   ;
wire [UPPER_P_DMA_AW-1:0]     upper_s_axi_araddr       ;
generate
if(64 == 64)begin
assign upper_s_axi_araddr       = {hsm_o_dma_m_araddr[63:32],4'h6,hsm_o_dma_m_araddr[27:0]};
end
else begin
assign upper_s_axi_araddr       = {4'h6,hsm_o_dma_m_araddr[27:0]};
end
endgenerate
wire [2:0]      upper_s_axi_arprot       = hsm_o_dma_m_arprot ;
wire [3:0]      upper_s_axi_arcache      = hsm_o_dma_m_arcache;
wire            upper_s_axi_arvalid      = hsm_o_dma_m_arvalid;
wire [7:0]      upper_s_axi_arlen        = hsm_o_dma_m_arlen  ;
wire [2:0]      upper_s_axi_arsize       = hsm_o_dma_m_arsize ;
wire [1:0]      upper_s_axi_arburst      = hsm_o_dma_m_arburst;
wire            upper_s_axi_arlock       = hsm_o_dma_m_arlock ;
wire            upper_s_axi_rready       = hsm_o_dma_m_rready ;
wire [31:0]     upper_i_hrdata_m_ext2    = ahb2kram_o_hrdata    ;
wire            upper_i_hreadyout_m_ext2 = ahb2kram_o_hreadyout ;
wire [1:0]      upper_i_hresp_m_ext2     = ahb2kram_o_hresp     ;
wire [2:0]      upper_i_hruser_m_ext2    = 'h0                  ;
wire [15:0]     upper_i_portin           = 'h0                  ;
wire            upper_i_spi_ss_i         = 'h0                  ;
wire            upper_i_spi_clk_i        = 'h0                  ;
wire            upper_i_spi_din          = 'h0                  ;
wire            upper_i_uart_rxd         = i_upper_uart_rxd     ;
wire            upper_i_uart1_rxd        = i_upper_uart1_rxd    ;
wire [447:0]    upper_i_dut_capture      = {hsm_o_soc_dbg_en_128b,64'h0,{(32-16){1'b0}},hsm_o_mbox_irq,hsm_o_hsm_err_fw,hsm_o_hsm_err_hw,32'h0,hsm_o_hsm_status};

wire [31:0]     upper_i_pll_freq         = CLK_OUT0_MHZ ;

`ifdef OSR_FPGA_ROM
wire           irom_ecc_CK  = hsm_o_irom_CK  ;
wire           irom_ecc_CSB = hsm_o_irom_CSB ;
wire           irom_ecc_WEB = hsm_o_irom_WEB ;
wire [14-1:0]    irom_ecc_A   = hsm_o_irom_A   ;
wire [38:0]    irom_ecc_DI  = hsm_o_irom_DI  ;
`else
wire           irom_ecc_CK  = hsm_o_irom_CK  ;
wire           irom_ecc_CSB = hsm_o_irom_CSB ;
wire           irom_ecc_WEB = 1'b1           ;
wire [14-1:0]    irom_ecc_A   = hsm_o_irom_A   ;
wire [38:0]    irom_ecc_DI  = 39'h0          ;
`endif

wire           iram_ecc_CK  = hsm_o_iram_CK  ;
wire           iram_ecc_CSB = hsm_o_iram_CSB ;
wire           iram_ecc_WEB = hsm_o_iram_WEB ;
wire [16-1:0]    iram_ecc_A   = hsm_o_iram_A   ;
wire [38:0]    iram_ecc_DI  = hsm_o_iram_DI  ;

wire           dram_ecc_CK  = hsm_o_dram_CK  ;
wire           dram_ecc_CSB = hsm_o_dram_CSB ;
wire           dram_ecc_WEB = hsm_o_dram_WEB ;
wire [14-1:0]    dram_ecc_A   = hsm_o_dram_A   ;
wire [38:0]    dram_ecc_DI  = hsm_o_dram_DI  ;

wire           kram_ecc_CK  = hsm_o_kbuf_CK  ;
wire           kram_ecc_CSB = hsm_o_kbuf_CSB ;
wire           kram_ecc_WEB = hsm_o_kbuf_WEB ;
wire [7:0]     kram_ecc_A   = hsm_o_kbuf_A   ;
wire [38:0]    kram_ecc_DI  = hsm_o_kbuf_DI  ;

wire           pram0_ecc_CK   = hsm_o_pke_sram0_CK   ;
wire           pram0_ecc_CSAN = hsm_o_pke_sram0_CSAN ;
wire [8:0]     pram0_ecc_A    = hsm_o_pke_sram0_A    ;
wire           pram0_ecc_CSBN = hsm_o_pke_sram0_CSBN ;
wire [8:0]     pram0_ecc_B    = hsm_o_pke_sram0_B    ;
wire           pram0_ecc_WEB  = hsm_o_pke_sram0_CSBN ;
wire [71:0]    pram0_ecc_DI   = hsm_o_pke_sram0_DI   ;

wire           pram1_ecc_CK   = hsm_o_pke_sram1_CK   ;
wire           pram1_ecc_CSAN = hsm_o_pke_sram1_CSAN ;
wire [8:0]     pram1_ecc_A    = hsm_o_pke_sram1_A    ;
wire           pram1_ecc_CSBN = hsm_o_pke_sram1_CSBN ;
wire [8:0]     pram1_ecc_B    = hsm_o_pke_sram1_B    ;
wire           pram1_ecc_WEB  = hsm_o_pke_sram1_CSBN ;
wire [71:0]    pram1_ecc_DI   = hsm_o_pke_sram1_DI   ;

wire           pram2_ecc_CK   = hsm_o_pke_sram2_CK   ;
wire           pram2_ecc_CSAN = hsm_o_pke_sram2_CSAN ;
wire [8:0]     pram2_ecc_A    = hsm_o_pke_sram2_A    ;
wire           pram2_ecc_CSBN = hsm_o_pke_sram2_CSBN ;
wire [8:0]     pram2_ecc_B    = hsm_o_pke_sram2_B    ;
wire           pram2_ecc_WEB  = hsm_o_pke_sram2_CSBN ;
wire [71:0]    pram2_ecc_DI   = hsm_o_pke_sram2_DI   ;

wire           pram3_ecc_CK   = hsm_o_pke_sram3_CK   ;
wire           pram3_ecc_CSAN = hsm_o_pke_sram3_CSAN ;
wire [8:0]     pram3_ecc_A    = hsm_o_pke_sram3_A    ;
wire           pram3_ecc_CSBN = hsm_o_pke_sram3_CSBN ;
wire [8:0]     pram3_ecc_B    = hsm_o_pke_sram3_B    ;
wire           pram3_ecc_WEB  = hsm_o_pke_sram3_CSBN ;
wire [71:0]    pram3_ecc_DI   = hsm_o_pke_sram3_DI   ;

wire hsm_tdo = hsm_o_tdo_oe ? hsm_o_tdo : 1'hz;

`ifdef OSR_FPGA_PWR_CTL 
assign PWR_TEST_EN       = PWR_PWR_TEST_EN   ;
assign PWR_EN            = PWR_PWR_EN        ;
`endif

assign o_upper_tdo       = upper_o_tdo_oen ? upper_o_tdo : 1'hz ; 

assign o_upper_uart_txd  = upper_o_uart_txd_oe ? upper_o_uart_txd : 1'hz;
assign o_upper_uart1_txd = upper_o_uart1_txd_oe ? upper_o_uart1_txd : 1'hz;
assign o_hsm_led         =  {hsm_o_hsm_status[3],hsm_o_hsm_status[2],hsm_o_hsm_status[1],hsm_o_hsm_status[0]};

assign o_hsm_tdo         = hsm_tdo; 

assign o_hsm_uart_txd    = hsm_o_uart_txd_oe ? hsm_o_uart_txd : 1'bz;

osr_ehsm_top_wrapper u_osr_ehsm_top_wrapper (
    .i_clk                       ( hsm_i_clk                    ), 
    .i_rst_n                     ( hsm_i_rst_n                  ), 
    .i_scan_mode                 ( hsm_i_scan_mode              ), 
    .i_scan_icg_enable           ( hsm_i_scan_icg_enable        ), 
    .i_soc_status                ( hsm_i_soc_status             ), 
    .i_soc_err                   ( hsm_i_soc_err                ), 
    .o_hsm_status                ( hsm_o_hsm_status             ), 
    .o_hsm_err_hw                ( hsm_o_hsm_err_hw             ), 
    .o_hsm_err_fw                ( hsm_o_hsm_err_fw             ), 
    .o_mbox_irq                  ( hsm_o_mbox_irq               ), 
    .i_s_hsel                    ( hsm_i_s_hsel                 ), 
    .i_s_haddr                   ( hsm_i_s_haddr                ), 
    .i_s_htrans                  ( hsm_i_s_htrans               ), 
    .i_s_hwrite                  ( hsm_i_s_hwrite               ), 
    .i_s_hburst                  ( hsm_i_s_hburst               ), 
    .i_s_hsize                   ( hsm_i_s_hsize                ), 
    .i_s_hprot                   ( hsm_i_s_hprot                ), 
    .i_s_hmastlock               ( hsm_i_s_hmastlock            ), 
    .i_s_hready                  ( hsm_i_s_hready               ), 
    .i_s_hwdata                  ( hsm_i_s_hwdata               ), 
    .o_s_hrdata                  ( hsm_o_s_hrdata               ), 
    .o_s_hresp                   ( hsm_o_s_hresp                ), 
    .o_s_hreadyout               ( hsm_o_s_hreadyout            ), 
    .o_m_hsel                    ( hsm_o_m_hsel                 ), 
    .o_m_haddr                   ( hsm_o_m_haddr                ), 
    .o_m_htrans                  ( hsm_o_m_htrans               ), 
    .o_m_hwrite                  ( hsm_o_m_hwrite               ), 
    .o_m_hburst                  ( hsm_o_m_hburst               ), 
    .o_m_hsize                   ( hsm_o_m_hsize                ), 
    .o_m_hprot                   ( hsm_o_m_hprot                ), 
    .o_m_hmastlock               ( hsm_o_m_hmastlock            ), 
    .o_m_hready                  ( hsm_o_m_hready               ), 
    .o_m_hwdata                  ( hsm_o_m_hwdata               ), 
    .i_m_hrdata                  ( hsm_i_m_hrdata               ), 
    .i_m_hresp                   ( hsm_i_m_hresp                ), 
    .i_m_hreadyout               ( hsm_i_m_hreadyout            ), 
    .o_dma_m_awid                ( hsm_o_dma_m_awid             ), 
    .o_dma_m_awaddr              ( hsm_o_dma_m_awaddr           ), 
    .o_dma_m_awlen               ( hsm_o_dma_m_awlen            ), 
    .o_dma_m_awsize              ( hsm_o_dma_m_awsize           ), 
    .o_dma_m_awburst             ( hsm_o_dma_m_awburst          ), 
    .o_dma_m_awlock              ( hsm_o_dma_m_awlock           ), 
    .o_dma_m_awcache             ( hsm_o_dma_m_awcache          ), 
    .o_dma_m_awprot              ( hsm_o_dma_m_awprot           ), 
    .o_dma_m_awqos               ( hsm_o_dma_m_awqos            ), 
    .o_dma_m_awregion            ( hsm_o_dma_m_awregion         ), 
    .o_dma_m_awvalid             ( hsm_o_dma_m_awvalid          ), 
    .i_dma_m_awready             ( hsm_i_dma_m_awready          ), 
    .o_dma_m_wdata               ( hsm_o_dma_m_wdata            ), 
    .o_dma_m_wstrb               ( hsm_o_dma_m_wstrb            ), 
    .o_dma_m_wlast               ( hsm_o_dma_m_wlast            ), 
    .o_dma_m_wvalid              ( hsm_o_dma_m_wvalid           ), 
    .i_dma_m_wready              ( hsm_i_dma_m_wready           ), 
    .i_dma_m_bid                 ( hsm_i_dma_m_bid              ), 
    .i_dma_m_bresp               ( hsm_i_dma_m_bresp            ), 
    .i_dma_m_bvalid              ( hsm_i_dma_m_bvalid           ), 
    .o_dma_m_bready              ( hsm_o_dma_m_bready           ), 
    .o_dma_m_arid                ( hsm_o_dma_m_arid             ), 
    .o_dma_m_araddr              ( hsm_o_dma_m_araddr           ), 
    .o_dma_m_arlen               ( hsm_o_dma_m_arlen            ), 
    .o_dma_m_arsize              ( hsm_o_dma_m_arsize           ), 
    .o_dma_m_arburst             ( hsm_o_dma_m_arburst          ), 
    .o_dma_m_arlock              ( hsm_o_dma_m_arlock           ), 
    .o_dma_m_arcache             ( hsm_o_dma_m_arcache          ), 
    .o_dma_m_arprot              ( hsm_o_dma_m_arprot           ), 
    .o_dma_m_arqos               ( hsm_o_dma_m_arqos            ), 
    .o_dma_m_arregion            ( hsm_o_dma_m_arregion         ), 
    .o_dma_m_arvalid             ( hsm_o_dma_m_arvalid          ), 
    .i_dma_m_arready             ( hsm_i_dma_m_arready          ), 
    .i_dma_m_rid                 ( hsm_i_dma_m_rid              ), 
    .i_dma_m_rdata               ( hsm_i_dma_m_rdata            ), 
    .i_dma_m_rresp               ( hsm_i_dma_m_rresp            ), 
    .i_dma_m_rlast               ( hsm_i_dma_m_rlast            ), 
    .i_dma_m_rvalid              ( hsm_i_dma_m_rvalid           ), 
    .o_dma_m_rready              ( hsm_o_dma_m_rready           ), 
    .o_cfg_hsel                  ( hsm_o_cfg_hsel               ), 
    .o_cfg_haddr                 ( hsm_o_cfg_haddr              ), 
    .o_cfg_htrans                ( hsm_o_cfg_htrans             ), 
    .o_cfg_hwrite                ( hsm_o_cfg_hwrite             ), 
    .o_cfg_hburst                ( hsm_o_cfg_hburst             ), 
    .o_cfg_hsize                 ( hsm_o_cfg_hsize              ), 
    .o_cfg_hprot                 ( hsm_o_cfg_hprot              ), 
    .o_cfg_hmastlock             ( hsm_o_cfg_hmastlock          ), 
    .o_cfg_hready                ( hsm_o_cfg_hready             ), 
    .o_cfg_hwdata                ( hsm_o_cfg_hwdata             ), 
    .i_cfg_hrdata                ( hsm_i_cfg_hrdata             ), 
    .i_cfg_hresp                 ( hsm_i_cfg_hresp              ), 
    .i_cfg_hreadyout             ( hsm_i_cfg_hreadyout          ), 
    .o_otp_hsel                  ( hsm_o_otp_hsel               ), 
    .o_otp_haddr                 ( hsm_o_otp_haddr              ), 
    .o_otp_htrans                ( hsm_o_otp_htrans             ), 
    .o_otp_hwrite                ( hsm_o_otp_hwrite             ), 
    .o_otp_hburst                ( hsm_o_otp_hburst             ), 
    .o_otp_hsize                 ( hsm_o_otp_hsize              ), 
    .o_otp_hprot                 ( hsm_o_otp_hprot              ), 
    .o_otp_hmastlock             ( hsm_o_otp_hmastlock          ), 
    .o_otp_hready                ( hsm_o_otp_hready             ), 
    .o_otp_hwdata                ( hsm_o_otp_hwdata             ), 
    .i_otp_hrdata                ( hsm_i_otp_hrdata             ), 
    .i_otp_hresp                 ( hsm_i_otp_hresp              ), 
    .i_otp_hreadyout             ( hsm_i_otp_hreadyout          ), 
    .o_irom_CK                   ( hsm_o_irom_CK                ), 
    .o_irom_CSB                  ( hsm_o_irom_CSB               ), 
    .o_irom_A                    ( hsm_o_irom_A                 ), 
    .i_irom_DO                   ( hsm_i_irom_DO                ), 
    `ifdef OSR_FPGA_ROM  
    .o_irom_DI                   ( hsm_o_irom_DI                ), 
    .o_irom_WEB                  ( hsm_o_irom_WEB               ), 
    `endif
    .o_iram_CK                   ( hsm_o_iram_CK                ), 
    .o_iram_A                    ( hsm_o_iram_A                 ), 
    .o_iram_CSB                  ( hsm_o_iram_CSB               ), 
    .o_iram_WEB                  ( hsm_o_iram_WEB               ), 
    .o_iram_DI                   ( hsm_o_iram_DI                ), 
    .i_iram_DO                   ( hsm_i_iram_DO                ), 
    .o_dram_CK                   ( hsm_o_dram_CK                ), 
    .o_dram_A                    ( hsm_o_dram_A                 ), 
    .o_dram_CSB                  ( hsm_o_dram_CSB               ), 
    .o_dram_WEB                  ( hsm_o_dram_WEB               ), 
    .o_dram_DI                   ( hsm_o_dram_DI                ), 
    .i_dram_DO                   ( hsm_i_dram_DO                ), 
    .o_kbuf_CK                   ( hsm_o_kbuf_CK                ), 
    .o_kbuf_CSB                  ( hsm_o_kbuf_CSB               ), 
    .o_kbuf_A                    ( hsm_o_kbuf_A                 ), 
    .o_kbuf_WEB                  ( hsm_o_kbuf_WEB               ), 
    .o_kbuf_DI                   ( hsm_o_kbuf_DI                ), 
    .i_kbuf_DO                   ( hsm_i_kbuf_DO                ), 
    .o_pke_sram0_CK              ( hsm_o_pke_sram0_CK           ), 
    .o_pke_sram0_CSAN            ( hsm_o_pke_sram0_CSAN         ), 
    .o_pke_sram0_A               ( hsm_o_pke_sram0_A            ), 
    .i_pke_sram0_DO              ( hsm_i_pke_sram0_DO           ), 
    .o_pke_sram0_CSBN            ( hsm_o_pke_sram0_CSBN         ), 
    .o_pke_sram0_B               ( hsm_o_pke_sram0_B            ), 
    .o_pke_sram0_DI              ( hsm_o_pke_sram0_DI           ), 
    .o_pke_sram1_CK              ( hsm_o_pke_sram1_CK           ), 
    .o_pke_sram1_CSAN            ( hsm_o_pke_sram1_CSAN         ), 
    .o_pke_sram1_A               ( hsm_o_pke_sram1_A            ), 
    .i_pke_sram1_DO              ( hsm_i_pke_sram1_DO           ), 
    .o_pke_sram1_CSBN            ( hsm_o_pke_sram1_CSBN         ), 
    .o_pke_sram1_B               ( hsm_o_pke_sram1_B            ), 
    .o_pke_sram1_DI              ( hsm_o_pke_sram1_DI           ), 
    .o_pke_sram2_CK              ( hsm_o_pke_sram2_CK           ), 
    .o_pke_sram2_CSAN            ( hsm_o_pke_sram2_CSAN         ), 
    .o_pke_sram2_A               ( hsm_o_pke_sram2_A            ), 
    .i_pke_sram2_DO              ( hsm_i_pke_sram2_DO           ), 
    .o_pke_sram2_CSBN            ( hsm_o_pke_sram2_CSBN         ), 
    .o_pke_sram2_B               ( hsm_o_pke_sram2_B            ), 
    .o_pke_sram2_DI              ( hsm_o_pke_sram2_DI           ), 
    .o_pke_sram3_CK              ( hsm_o_pke_sram3_CK           ), 
    .o_pke_sram3_CSAN            ( hsm_o_pke_sram3_CSAN         ), 
    .o_pke_sram3_A               ( hsm_o_pke_sram3_A            ), 
    .i_pke_sram3_DO              ( hsm_i_pke_sram3_DO           ), 
    .o_pke_sram3_CSBN            ( hsm_o_pke_sram3_CSBN         ), 
    .o_pke_sram3_B               ( hsm_o_pke_sram3_B            ), 
    .o_pke_sram3_DI              ( hsm_o_pke_sram3_DI           ), 
    .i_trst_n                    ( hsm_i_trst_n                 ), 
    .i_tck                       ( hsm_i_tck                    ), 
    .i_tms                       ( hsm_i_tms                    ), 
    .i_tdi                       ( hsm_i_tdi                    ), 
    .o_tdo_oe                    ( hsm_o_tdo_oe                 ), 
    .o_tdo                       ( hsm_o_tdo                    ), 
    .i_uart_rxd                  ( hsm_i_uart_rxd               ), 
    .o_uart_txd_oe               ( hsm_o_uart_txd_oe            ), 
    .o_uart_txd                  ( hsm_o_uart_txd               ), 
    .o_soc_dbg_en_128b           ( hsm_o_soc_dbg_en_128b        ), 
    .o_trng_ro_clk               ( hsm_o_trng_ro_clk            ), 
    .o_trng_ro_out               ( hsm_o_trng_ro_out            )  
    ); 

osr_ahb_to_ram #(
    .P_BUS_AW             ( AHB2KRAM_P_BUS_AW         ), 
    .P_BUS_NO_WORD        ( AHB2KRAM_P_BUS_NO_WORD    ), 
    .P_AHB2RAM_TIMING     ( AHB2KRAM_P_AHB2RAM_TIMING ), 
    .P_AW_DEL             ( AHB2KRAM_P_AW_DEL         )  
    ) u_ahb2kram (
    .i_hclk                   ( ahb2kram_i_hclk           ), 
    .i_hresetn                ( ahb2kram_i_hresetn        ), 
    .i_ecc_err_rsp_en         ( ahb2kram_i_ecc_err_rsp_en ), 
    .i_ecc_dec_sec            ( ahb2kram_i_ecc_dec_sec    ), 
    .i_ecc_dec_ded            ( ahb2kram_i_ecc_dec_ded    ), 
    .i_hsel                   ( ahb2kram_i_hsel           ), 
    .i_hready                 ( ahb2kram_i_hready         ), 
    .i_haddr                  ( ahb2kram_i_haddr          ), 
    .i_htrans                 ( ahb2kram_i_htrans         ), 
    .i_hwrite                 ( ahb2kram_i_hwrite         ), 
    .i_hsize                  ( ahb2kram_i_hsize          ), 
    .i_hburst                 ( ahb2kram_i_hburst         ), 
    .i_hprot                  ( ahb2kram_i_hprot          ), 
    .i_hmaster                ( ahb2kram_i_hmaster        ), 
    .i_hwdata                 ( ahb2kram_i_hwdata         ), 
    .i_hmastlock              ( ahb2kram_i_hmastlock      ), 
    .o_hrdata                 ( ahb2kram_o_hrdata         ), 
    .o_hreadyout              ( ahb2kram_o_hreadyout      ), 
    .o_hresp                  ( ahb2kram_o_hresp          ), 
    .o_ram_cs                 ( ahb2kram_o_ram_cs         ), 
    .o_ram_wen                ( ahb2kram_o_ram_wen        ), 
    .o_ram_addr               ( ahb2kram_o_ram_addr       ), 
    .o_ram_wdata              ( ahb2kram_o_ram_wdata      ), 
    .i_ram_rdata              ( ahb2kram_i_ram_rdata      )  
    ); 

 osr_upper_top_m130a_no_fusa #(
    .P_DMA_AW                (UPPER_P_DMA_AW            ),
    .P_DMA_DW                (UPPER_P_DMA_DW            ),
    .UPPER_IRAM_AW           (UPPER_IRAM_AW             ),
    .UPPER_IRAM_ID0          (UPPER_IRAM_ID0            ),
    .UPPER_DRAM_AW           (UPPER_DRAM_AW             ),
    .UPPER_RAM_ID0           (UPPER_RAM_ID0             ),
    .UPPER_RAM_SA0           (UPPER_RAM_SA0             ),
    .UPPER_RAM_I0            (UPPER_RAM_I0              ),
    .NVM_RAM_AW              (NVM_RAM_AW                ),
    .NVM_RAM_ID0             (NVM_RAM_ID0               ),
    .NVM_RAM_SA0             (NVM_RAM_SA0               ),
    .NVM_RAM_ID1             (NVM_RAM_ID1               ),
    .NVM_RAM_SA1             (NVM_RAM_SA1               ),
    .NVM_RAM_I0              (NVM_RAM_I0                )
    )u_upper(
    .i_osc_clk               ( upper_i_osc_clk          ), 
    .i_por_rst_n             ( upper_i_por_rst_n        ), 
    .i_soc_rst_n             ( upper_i_soc_rst_n        ), 
    .i_ext_rst_n             ( upper_i_ext_rst_n        ), 
    .i_pll_locked            ( upper_i_pll_locked       ), 
    .i_trst_n                ( upper_i_trst_n           ), 
    .i_tck                   ( upper_i_tck_swclk        ), 
    .i_tms                   ( upper_i_tms_swdi         ), 
    .i_tdi                   ( upper_i_tdi              ), 
    .o_tdo                   ( upper_o_tdo              ), 
    .o_tdo_oen               ( upper_o_tdo_oen          ), 
    .i_hsel_s_ext0           ( upper_i_hsel_s_ext0      ), 
    .i_haddr_s_ext0          ( upper_i_haddr_s_ext0     ), 
    .i_htrans_s_ext0         ( upper_i_htrans_s_ext0    ), 
    .i_hwrite_s_ext0         ( upper_i_hwrite_s_ext0    ), 
    .i_hsize_s_ext0          ( upper_i_hsize_s_ext0     ), 
    .i_hburst_s_ext0         ( upper_i_hburst_s_ext0    ), 
    .i_hprot_s_ext0          ( upper_i_hprot_s_ext0     ), 
    .i_hmaster_s_ext0        ( upper_i_hmaster_s_ext0   ), 
    .i_hwdata_s_ext0         ( upper_i_hwdata_s_ext0    ), 
    .i_hmastlock_s_ext0      ( upper_i_hmastlock_s_ext0 ), 
    .i_hready_s_ext0         ( upper_i_hready_s_ext0    ), 
    .i_hauser_s_ext0         ( upper_i_hauser_s_ext0    ), 
    .i_hwuser_s_ext0         ( upper_i_hwuser_s_ext0    ), 
    .o_hrdata_s_ext0         ( upper_o_hrdata_s_ext0    ), 
    .o_hreadyout_s_ext0      ( upper_o_hreadyout_s_ext0 ), 
    .o_hresp_s_ext0          ( upper_o_hresp_s_ext0     ), 
    .o_hruser_s_ext0         ( upper_o_hruser_s_ext0    ), 
    .i_hsel_s_ext1           ( upper_i_hsel_s_ext1      ), 
    .i_haddr_s_ext1          ( upper_i_haddr_s_ext1     ), 
    .i_htrans_s_ext1         ( upper_i_htrans_s_ext1    ), 
    .i_hwrite_s_ext1         ( upper_i_hwrite_s_ext1    ), 
    .i_hsize_s_ext1          ( upper_i_hsize_s_ext1     ), 
    .i_hburst_s_ext1         ( upper_i_hburst_s_ext1    ), 
    .i_hprot_s_ext1          ( upper_i_hprot_s_ext1     ), 
    .i_hmaster_s_ext1        ( upper_i_hmaster_s_ext1   ), 
    .i_hwdata_s_ext1         ( upper_i_hwdata_s_ext1    ), 
    .i_hmastlock_s_ext1      ( upper_i_hmastlock_s_ext1 ), 
    .i_hready_s_ext1         ( upper_i_hready_s_ext1    ), 
    .i_hauser_s_ext1         ( upper_i_hauser_s_ext1    ), 
    .i_hwuser_s_ext1         ( upper_i_hwuser_s_ext1    ), 
    .o_hrdata_s_ext1         ( upper_o_hrdata_s_ext1    ), 
    .o_hreadyout_s_ext1      ( upper_o_hreadyout_s_ext1 ), 
    .o_hresp_s_ext1          ( upper_o_hresp_s_ext1     ), 
    .o_hruser_s_ext1         ( upper_o_hruser_s_ext1    ), 
    .i_hsel_s_ext2           ( upper_i_hsel_s_ext2      ), 
    .i_haddr_s_ext2          ( upper_i_haddr_s_ext2     ), 
    .i_htrans_s_ext2         ( upper_i_htrans_s_ext2    ), 
    .i_hwrite_s_ext2         ( upper_i_hwrite_s_ext2    ), 
    .i_hsize_s_ext2          ( upper_i_hsize_s_ext2     ), 
    .i_hburst_s_ext2         ( upper_i_hburst_s_ext2    ), 
    .i_hprot_s_ext2          ( upper_i_hprot_s_ext2     ), 
    .i_hmaster_s_ext2        ( upper_i_hmaster_s_ext2   ), 
    .i_hwdata_s_ext2         ( upper_i_hwdata_s_ext2    ), 
    .i_hmastlock_s_ext2      ( upper_i_hmastlock_s_ext2 ), 
    .i_hready_s_ext2         ( upper_i_hready_s_ext2    ), 
    .i_hauser_s_ext2         ( upper_i_hauser_s_ext2    ), 
    .i_hwuser_s_ext2         ( upper_i_hwuser_s_ext2    ), 
    .o_hrdata_s_ext2         ( upper_o_hrdata_s_ext2    ), 
    .o_hreadyout_s_ext2      ( upper_o_hreadyout_s_ext2 ), 
    .o_hresp_s_ext2          ( upper_o_hresp_s_ext2     ), 
    .o_hruser_s_ext2         ( upper_o_hruser_s_ext2    ), 
    .i_hsel_s_ext3           ( upper_i_hsel_s_ext3      ), 
    .i_haddr_s_ext3          ( upper_i_haddr_s_ext3     ), 
    .i_htrans_s_ext3         ( upper_i_htrans_s_ext3    ), 
    .i_hwrite_s_ext3         ( upper_i_hwrite_s_ext3    ), 
    .i_hsize_s_ext3          ( upper_i_hsize_s_ext3     ), 
    .i_hburst_s_ext3         ( upper_i_hburst_s_ext3    ), 
    .i_hprot_s_ext3          ( upper_i_hprot_s_ext3     ), 
    .i_hmaster_s_ext3        ( upper_i_hmaster_s_ext3   ), 
    .i_hwdata_s_ext3         ( upper_i_hwdata_s_ext3    ), 
    .i_hmastlock_s_ext3      ( upper_i_hmastlock_s_ext3 ), 
    .i_hready_s_ext3         ( upper_i_hready_s_ext3    ), 
    .i_hauser_s_ext3         ( upper_i_hauser_s_ext3    ), 
    .i_hwuser_s_ext3         ( upper_i_hwuser_s_ext3    ), 
    .o_hrdata_s_ext3         ( upper_o_hrdata_s_ext3    ), 
    .o_hreadyout_s_ext3      ( upper_o_hreadyout_s_ext3 ), 
    .o_hresp_s_ext3          ( upper_o_hresp_s_ext3     ), 
    .o_hruser_s_ext3         ( upper_o_hruser_s_ext3    ), 
    .i_hsel_s_ext4           ( upper_i_hsel_s_ext4      ), 
    .i_haddr_s_ext4          ( upper_i_haddr_s_ext4     ), 
    .i_htrans_s_ext4         ( upper_i_htrans_s_ext4    ), 
    .i_hwrite_s_ext4         ( upper_i_hwrite_s_ext4    ), 
    .i_hsize_s_ext4          ( upper_i_hsize_s_ext4     ), 
    .i_hburst_s_ext4         ( upper_i_hburst_s_ext4    ), 
    .i_hprot_s_ext4          ( upper_i_hprot_s_ext4     ), 
    .i_hmaster_s_ext4        ( upper_i_hmaster_s_ext4   ), 
    .i_hwdata_s_ext4         ( upper_i_hwdata_s_ext4    ), 
    .i_hmastlock_s_ext4      ( upper_i_hmastlock_s_ext4 ), 
    .i_hready_s_ext4         ( upper_i_hready_s_ext4    ), 
    .i_hauser_s_ext4         ( upper_i_hauser_s_ext4    ), 
    .i_hwuser_s_ext4         ( upper_i_hwuser_s_ext4    ), 
    .o_hrdata_s_ext4         ( upper_o_hrdata_s_ext4    ), 
    .o_hreadyout_s_ext4      ( upper_o_hreadyout_s_ext4 ), 
    .o_hresp_s_ext4          ( upper_o_hresp_s_ext4     ), 
    .o_hruser_s_ext4         ( upper_o_hruser_s_ext4    ), 
    .o_hsel_m_ext0           ( upper_o_hsel_m_ext0      ), 
    .o_haddr_m_ext0          ( upper_o_haddr_m_ext0     ), 
    .o_htrans_m_ext0         ( upper_o_htrans_m_ext0    ), 
    .o_hwrite_m_ext0         ( upper_o_hwrite_m_ext0    ), 
    .o_hsize_m_ext0          ( upper_o_hsize_m_ext0     ), 
    .o_hburst_m_ext0         ( upper_o_hburst_m_ext0    ), 
    .o_hprot_m_ext0          ( upper_o_hprot_m_ext0     ), 
    .o_hmaster_m_ext0        ( upper_o_hmaster_m_ext0   ), 
    .o_hwdata_m_ext0         ( upper_o_hwdata_m_ext0    ), 
    .o_hmastlock_m_ext0      ( upper_o_hmastlock_m_ext0 ), 
    .o_hreadymux_m_ext0      ( upper_o_hreadymux_m_ext0 ), 
    .o_hauser_m_ext0         ( upper_o_hauser_m_ext0    ), 
    .o_hwuser_m_ext0         ( upper_o_hwuser_m_ext0    ), 
    .i_hrdata_m_ext0         ( upper_i_hrdata_m_ext0    ), 
    .i_hreadyout_m_ext0      ( upper_i_hreadyout_m_ext0 ), 
    .i_hresp_m_ext0          ( upper_i_hresp_m_ext0     ), 
    .i_hruser_m_ext0         ( upper_i_hruser_m_ext0    ), 
    .s_axi_awid              ( upper_s_axi_awid         ), 
    .s_axi_awlen             ( upper_s_axi_awlen        ), 
    .s_axi_awsize            ( upper_s_axi_awsize       ), 
    .s_axi_awburst           ( upper_s_axi_awburst      ), 
    .s_axi_awcache           ( upper_s_axi_awcache      ), 
    .s_axi_awaddr            ( upper_s_axi_awaddr       ), 
    .s_axi_awprot            ( upper_s_axi_awprot       ), 
    .s_axi_awvalid           ( upper_s_axi_awvalid      ), 
    .s_axi_awready           ( upper_s_axi_awready      ), 
    .s_axi_awlock            ( upper_s_axi_awlock       ), 
    .s_axi_wid               ( upper_s_axi_wid          ), 
    .s_axi_wdata             ( upper_s_axi_wdata        ), 
    .s_axi_wstrb             ( upper_s_axi_wstrb        ), 
    .s_axi_wlast             ( upper_s_axi_wlast        ), 
    .s_axi_wvalid            ( upper_s_axi_wvalid       ), 
    .s_axi_wready            ( upper_s_axi_wready       ), 
    .s_axi_bid               ( upper_s_axi_bid          ), 
    .s_axi_bresp             ( upper_s_axi_bresp        ), 
    .s_axi_bvalid            ( upper_s_axi_bvalid       ), 
    .s_axi_bready            ( upper_s_axi_bready       ), 
    .s_axi_arid              ( upper_s_axi_arid         ), 
    .s_axi_araddr            ( upper_s_axi_araddr       ), 
    .s_axi_arprot            ( upper_s_axi_arprot       ), 
    .s_axi_arcache           ( upper_s_axi_arcache      ), 
    .s_axi_arvalid           ( upper_s_axi_arvalid      ), 
    .s_axi_arlen             ( upper_s_axi_arlen        ), 
    .s_axi_arsize            ( upper_s_axi_arsize       ), 
    .s_axi_arburst           ( upper_s_axi_arburst      ), 
    .s_axi_arlock            ( upper_s_axi_arlock       ), 
    .s_axi_arready           ( upper_s_axi_arready      ), 
    .s_axi_rid               ( upper_s_axi_rid          ), 
    .s_axi_rdata             ( upper_s_axi_rdata        ), 
    .s_axi_rresp             ( upper_s_axi_rresp        ), 
    .s_axi_rvalid            ( upper_s_axi_rvalid       ), 
    .s_axi_rlast             ( upper_s_axi_rlast        ), 
    .s_axi_rready            ( upper_s_axi_rready       ), 
    .o_hsel_m_ext2           ( upper_o_hsel_m_ext2      ), 
    .o_haddr_m_ext2          ( upper_o_haddr_m_ext2     ), 
    .o_htrans_m_ext2         ( upper_o_htrans_m_ext2    ), 
    .o_hwrite_m_ext2         ( upper_o_hwrite_m_ext2    ), 
    .o_hsize_m_ext2          ( upper_o_hsize_m_ext2     ), 
    .o_hburst_m_ext2         ( upper_o_hburst_m_ext2    ), 
    .o_hprot_m_ext2          ( upper_o_hprot_m_ext2     ), 
    .o_hmaster_m_ext2        ( upper_o_hmaster_m_ext2   ), 
    .o_hwdata_m_ext2         ( upper_o_hwdata_m_ext2    ), 
    .o_hmastlock_m_ext2      ( upper_o_hmastlock_m_ext2 ), 
    .o_hreadymux_m_ext2      ( upper_o_hreadymux_m_ext2 ), 
    .o_hauser_m_ext2         ( upper_o_hauser_m_ext2    ), 
    .o_hwuser_m_ext2         ( upper_o_hwuser_m_ext2    ), 
    .i_hrdata_m_ext2         ( upper_i_hrdata_m_ext2    ), 
    .i_hreadyout_m_ext2      ( upper_i_hreadyout_m_ext2 ), 
    .i_hresp_m_ext2          ( upper_i_hresp_m_ext2     ), 
    .i_hruser_m_ext2         ( upper_i_hruser_m_ext2    ), 
    .i_portin                ( upper_i_portin           ), 
    .o_portout               ( upper_o_portout          ), 
    .o_porten                ( upper_o_porten           ), 
    .o_portfunc              ( upper_o_portfunc         ), 
    .i_spi_ss_i              ( upper_i_spi_ss_i         ), 
    .o_spi_ss_o              ( upper_o_spi_ss_o         ), 
    .i_spi_clk_i             ( upper_i_spi_clk_i        ), 
    .o_spi_clk_o             ( upper_o_spi_clk_o        ), 
    .o_spi_ctl_oen           ( upper_o_spi_ctl_oen      ), 
    .i_spi_din               ( upper_i_spi_din          ), 
    .o_spi_dout              ( upper_o_spi_dout         ), 
    .o_spi_dout_oen          ( upper_o_spi_dout_oen     ), 
    .i_uart_rxd              ( upper_i_uart_rxd         ), 
    .o_uart_txd              ( upper_o_uart_txd         ), 
    .o_uart_txd_oe           ( upper_o_uart_txd_oe      ), 
    .i_uart1_rxd             ( upper_i_uart1_rxd        ), 
    .o_uart1_txd             ( upper_o_uart1_txd        ), 
    .o_uart1_txd_oe          ( upper_o_uart1_txd_oe     ), 
    .o_dut_clk               ( upper_o_dut_clk          ), 
    .o_dut_rst_n             ( upper_o_dut_rst_n        ), 
    .o_dut_ctrl              ( upper_o_dut_ctrl         ), 
    .o_dut_ctrl_ext          ( upper_o_dut_ctrl_ext     ), 
    .i_pll_freq              ( upper_i_pll_freq         ), 
    .i_dut_capture           ( upper_i_dut_capture      )  
    ); 

tsp_bw_sc_ram #(
    .DW           (39           ),
    .AW           (14),
    .CW           (39           ),
    .ID0          (IROM_ECC_ID0 ),
    .I0           (IROM_ECC_I0  ),
    .DP           (64*1024/4)
    ) u_irom_ecc  (
    .CK           (irom_ecc_CK  ), 
    .CSB          (irom_ecc_CSB ), 
    .WEB          (irom_ecc_WEB ), 
    .A            (irom_ecc_A   ), 
    .DI           (irom_ecc_DI  ), 
    .DO           (irom_ecc_DO  )  
);

tsp_bw_sc_ram #(
    .DW           (39           ),
    .AW           (16),
    .CW           (39           ),
    .ID0          (IRAM_ECC_ID0 ),
    .I0           (IRAM_ECC_I0  ),
    .DP           (256*1024/4)
    ) u_iram_ecc (
    .CK           (iram_ecc_CK  ), 
    .CSB          (iram_ecc_CSB ), 
    .WEB          (iram_ecc_WEB ), 
    .A            (iram_ecc_A   ), 
    .DI           (iram_ecc_DI  ), 
    .DO           (iram_ecc_DO  )  
);

tsp_bw_sc_ram #(
    .DW           (39           ),
    .AW           (14),
    .CW           (39           ),
    .I0           (DRAM_ECC_I0  )
    ) u_dram_ecc (
    .CK           (dram_ecc_CK  ), 
    .CSB          (dram_ecc_CSB ), 
    .WEB          (dram_ecc_WEB ), 
    .A            (dram_ecc_A   ), 
    .DI           (dram_ecc_DI  ), 
    .DO           (dram_ecc_DO  )  
);

tdp_bw_dc_ram #(
    .DW           (39           ),      
    .AW           (8            ),    
    .CW           (39           )    
    ) u_kram_ecc (
    .CKA          ( kram_ecc_CK            ),
    .CSAN         ( kram_ecc_CSB           ),
    .WEAN         ( kram_ecc_WEB           ),
    .A            ( kram_ecc_A             ),
    .DIA          ( kram_ecc_DI            ),
    .DOA          ( kram_ecc_DO            ),
    .CKB          ( pll_clk                ),
    .CSBN         ( ~ahb2kram_o_ram_cs     ),
    .WEBN         ( 1'h1                   ),
    .B            ( ahb2kram_o_ram_addr    ),
    .DIB          ( 39'h0                  ),
    .DOB          ( ahb2kram_o_ram_rdata   )  
);

sdp_bw_dc_ram #( 
    .DW           (72               ), 
    .AW           (9                ), 
    .CW           (72               ),
    .DP           (384              ) 
    ) u_pram0_ecc (     
  .CKA            ( pram0_ecc_CK    ),
  .CSAN           ( pram0_ecc_CSAN  ),
  .A              ( pram0_ecc_A     ),
  .DO             ( pram0_ecc_DO    ),
  .CKB            ( pram0_ecc_CK    ),
  .CSBN           ( pram0_ecc_CSBN  ),
  .B              ( pram0_ecc_B     ),
  .WEB            ( pram0_ecc_WEB   ),
  .DI             ( pram0_ecc_DI    ) 
);

sdp_bw_dc_ram #( 
    .DW           (72               ), 
    .AW           (9                ), 
    .CW           (72               ), 
    .DP           (384              ) 
    ) u_pram1_ecc (     
  .CKA            ( pram1_ecc_CK    ),
  .CSAN           ( pram1_ecc_CSAN  ),
  .A              ( pram1_ecc_A     ),
  .DO             ( pram1_ecc_DO    ),
  .CKB            ( pram1_ecc_CK    ),
  .CSBN           ( pram1_ecc_CSBN  ),
  .B              ( pram1_ecc_B     ),
  .WEB            ( pram1_ecc_WEB   ),
  .DI             ( pram1_ecc_DI    ) 
);

sdp_bw_dc_ram #( 
    .DW           (72               ), 
    .AW           (9                ), 
    .CW           (72               ), 
    .DP           (384              ) 
    ) u_pram2_ecc (     
  .CKA            ( pram2_ecc_CK    ),
  .CSAN           ( pram2_ecc_CSAN  ),
  .A              ( pram2_ecc_A     ),
  .DO             ( pram2_ecc_DO    ),
  .CKB            ( pram2_ecc_CK    ),
  .CSBN           ( pram2_ecc_CSBN  ),
  .B              ( pram2_ecc_B     ),
  .WEB            ( pram2_ecc_WEB   ),
  .DI             ( pram2_ecc_DI    ) 
);

sdp_bw_dc_ram #( 
    .DW           (72               ), 
    .AW           (9                ), 
    .CW           (72               ),
    .DP           (384              ) 
    ) u_pram3_ecc (     
  .CKA            ( pram3_ecc_CK    ),
  .CSAN           ( pram3_ecc_CSAN  ),
  .A              ( pram3_ecc_A     ),
  .DO             ( pram3_ecc_DO    ),
  .CKB            ( pram3_ecc_CK    ),
  .CSBN           ( pram3_ecc_CSBN  ),
  .B              ( pram3_ecc_B     ),
  .WEB            ( pram3_ecc_WEB   ),
  .DI             ( pram3_ecc_DI    ) 
);

always @(posedge pll_clk or negedge RST_N) begin
    if(!RST_N) begin
        pll_rst_r                <= 1'b0                     ;
        pll_rst_n                <= 1'b0                     ;
    end else begin
        pll_rst_r                <= pll_rst_r_next           ;
        pll_rst_n                <= pll_rst_n_next           ;
    end
end

always @(posedge upper_o_dut_clk or negedge upper_o_dut_rst_n) begin
    if(!upper_o_dut_rst_n) begin
        soc_mem_err_en           <= 1'b0                     ;
        otp_err_en               <= 1'b0                     ;
        soc_mem_err_respond      <= 1'b0                     ;
        otp_err_respond          <= 1'b0                     ;
    end else begin
        soc_mem_err_en           <= soc_mem_err_en_next      ;
        otp_err_en               <= otp_err_en_next          ;
        soc_mem_err_respond      <= soc_mem_err_respond_next ;
        otp_err_respond          <= otp_err_respond_next     ;
    end
end

always @(posedge upper_o_dut_clk or negedge upper_o_dut_rst_n) begin
    if(!upper_o_dut_rst_n) begin
        clk_div_cnt0             <= 2'h0                     ;
        cpu_clk_2_div            <= 1'b0                     ;
        cpu_clk_2_div_en         <= 1'b1                     ;
        cpu_clk_3_div0           <= 1'b0                     ;
        cpu_clk_3_div_en         <= 1'b0                     ;
        cpu_clk_3_div            <= 1'b0                     ;
    end else begin
        clk_div_cnt0             <= clk_div_cnt0_next        ;
        cpu_clk_2_div            <= cpu_clk_2_div_next       ;
        cpu_clk_2_div_en         <= cpu_clk_2_div_en_next    ;
        cpu_clk_3_div0           <= cpu_clk_3_div0_next      ;
        cpu_clk_3_div_en         <= cpu_clk_3_div_en_next    ;
        cpu_clk_3_div            <= cpu_clk_3_div_next       ;
    end
end

always @(negedge upper_o_dut_clk or negedge upper_o_dut_rst_n) begin
    if(!upper_o_dut_rst_n) begin
        clk_div_cnt1             <= 2'h0                     ;
        cpu_clk_3_div1           <= 1'b0                     ;
    end else begin
        clk_div_cnt1             <= clk_div_cnt1_next        ;
        cpu_clk_3_div1           <= cpu_clk_3_div1_next      ;
    end
end

endmodule 
