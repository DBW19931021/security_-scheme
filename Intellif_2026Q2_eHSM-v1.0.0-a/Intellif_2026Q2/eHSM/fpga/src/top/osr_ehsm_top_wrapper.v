//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ehsm_top_wrapper (
    input  wire                 i_clk                   ,   
    input  wire                 i_rst_n                 ,
    input  wire                 i_scan_mode             ,   
    input  wire                 i_scan_icg_enable       ,   

    input  wire [31:0]          i_soc_status            ,   
    input  wire [31:0]          i_soc_err               ,   

    output wire [63:0]          o_hsm_status            ,   
    output wire [63:0]          o_hsm_err_hw            ,   
    output wire [63:0]          o_hsm_err_fw            ,   

    output wire [16-1:0] o_mbox_irq              ,   

    input  wire                 i_s_hsel                ,   
    input  wire [31:0]          i_s_haddr               ,
    input  wire [1:0]           i_s_htrans              ,
    input  wire                 i_s_hwrite              ,
    input  wire [2:0]           i_s_hburst              ,
    input  wire [2:0]           i_s_hsize               ,
    input  wire [3:0]           i_s_hprot               ,
    input  wire                 i_s_hmastlock           ,
    input  wire                 i_s_hready              ,
    input  wire [31:0]          i_s_hwdata              ,
    output wire [31:0]          o_s_hrdata              ,
    output wire [1:0]           o_s_hresp               ,
    output wire                 o_s_hreadyout           ,

    output wire                 o_m_hsel                ,   
    output wire [63:0]          o_m_haddr               ,
    output wire [1:0]           o_m_htrans              ,
    output wire                 o_m_hwrite              ,
    output wire [2:0]           o_m_hburst              ,
    output wire [2:0]           o_m_hsize               ,
    output wire [3:0]           o_m_hprot               ,
    output wire                 o_m_hmastlock           ,
    output wire                 o_m_hready              ,
    output wire [31:0]          o_m_hwdata              ,
    input  wire [31:0]          i_m_hrdata              ,
    input  wire [1:0]           i_m_hresp               ,
    input  wire                 i_m_hreadyout           ,

    output wire [3:0]           o_dma_m_awid            ,  
    output wire [64-1:0]o_dma_m_awaddr         ,
    output wire [7:0]           o_dma_m_awlen           ,
    output wire [2:0]           o_dma_m_awsize          ,
    output wire [1:0]           o_dma_m_awburst         ,
    output wire                 o_dma_m_awlock          ,
    output wire [3:0]           o_dma_m_awcache         ,
    output wire [2:0]           o_dma_m_awprot          ,
    output wire [3:0]           o_dma_m_awqos           ,
    output wire [3:0]           o_dma_m_awregion        ,
    output wire                 o_dma_m_awvalid         ,
    input  wire                 i_dma_m_awready         ,

    output wire [64-1:0]o_dma_m_wdata          ,
    output wire [64/8-1:0]o_dma_m_wstrb        ,
    output wire                 o_dma_m_wlast           ,
    output wire                 o_dma_m_wvalid          ,
    input  wire                 i_dma_m_wready          ,

    input  wire [3:0]           i_dma_m_bid             ,
    input  wire [1:0]           i_dma_m_bresp           ,
    input  wire                 i_dma_m_bvalid          ,
    output wire                 o_dma_m_bready          ,

    output wire [3:0]           o_dma_m_arid            ,
    output wire [64-1:0]o_dma_m_araddr         ,
    output wire [7:0]           o_dma_m_arlen           ,
    output wire [2:0]           o_dma_m_arsize          ,
    output wire [1:0]           o_dma_m_arburst         ,
    output wire                 o_dma_m_arlock          ,
    output wire [3:0]           o_dma_m_arcache         ,
    output wire [2:0]           o_dma_m_arprot          ,
    output wire [3:0]           o_dma_m_arqos           ,
    output wire [3:0]           o_dma_m_arregion        ,
    output wire                 o_dma_m_arvalid         ,
    input  wire                 i_dma_m_arready         ,

    input  wire [3:0]           i_dma_m_rid             ,
    input  wire [64-1:0]i_dma_m_rdata          ,
    input  wire [1:0]           i_dma_m_rresp           ,
    input  wire                 i_dma_m_rlast           ,
    input  wire                 i_dma_m_rvalid          ,
    output wire                 o_dma_m_rready          ,

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

    output wire                 o_irom_CK               ,   
    output wire                 o_irom_CSB              ,
    output wire [14-1:0] o_irom_A    ,

    input  wire [38:0]          i_irom_DO               ,

    `ifdef OSR_FPGA_ROM 
    output wire [38:0]          o_irom_DI               ,
    output wire                 o_irom_WEB              ,
    `endif 

    output wire                 o_iram_CK               ,   
    output wire [16-1:0] o_iram_A    ,
    output wire                 o_iram_CSB              ,
    output wire                 o_iram_WEB              ,
    output wire [38:0]          o_iram_DI               ,
    input  wire [38:0]          i_iram_DO               ,

    output wire                 o_dram_CK               ,   
    output wire [14-1:0] o_dram_A    ,
    output wire                 o_dram_CSB              ,
    output wire                 o_dram_WEB              ,
    output wire [38:0]          o_dram_DI               ,
    input  wire [38:0]          i_dram_DO               ,

    output wire                 o_kbuf_CK               ,   
    output wire                 o_kbuf_CSB              ,
    output wire [7:0]           o_kbuf_A                ,
    output wire                 o_kbuf_WEB              ,
    output wire [38:0]          o_kbuf_DI               ,
    input  wire [38:0]          i_kbuf_DO               ,

    output wire                 o_pke_sram0_CK          ,   
    output wire                 o_pke_sram0_CSAN        ,
    output wire [8:0]           o_pke_sram0_A           ,
    input  wire [71:0]          i_pke_sram0_DO          ,
    output wire                 o_pke_sram0_CSBN        ,
    output wire [8:0]           o_pke_sram0_B           ,
    output wire [71:0]          o_pke_sram0_DI          ,

    output wire                 o_pke_sram1_CK          ,
    output wire                 o_pke_sram1_CSAN        ,
    output wire [8:0]           o_pke_sram1_A           ,
    input  wire [71:0]          i_pke_sram1_DO          ,
    output wire                 o_pke_sram1_CSBN        ,
    output wire [8:0]           o_pke_sram1_B           ,
    output wire [71:0]          o_pke_sram1_DI          ,

    output wire                 o_pke_sram2_CK          ,
    output wire                 o_pke_sram2_CSAN        ,
    output wire [8:0]           o_pke_sram2_A           ,
    input  wire [71:0]          i_pke_sram2_DO          ,
    output wire                 o_pke_sram2_CSBN        ,
    output wire [8:0]           o_pke_sram2_B           ,
    output wire [71:0]          o_pke_sram2_DI          ,

    output wire                 o_pke_sram3_CK          ,
    output wire                 o_pke_sram3_CSAN        ,
    output wire [8:0]           o_pke_sram3_A           ,
    input  wire [71:0]          i_pke_sram3_DO          ,
    output wire                 o_pke_sram3_CSBN        ,
    output wire [8:0]           o_pke_sram3_B           ,
    output wire [71:0]          o_pke_sram3_DI          ,

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

    output wire [3:0]           o_trng_ro_clk           ,   
    output wire [3:0]           o_trng_ro_out

);

localparam     OTP2H_ADDRWIDTH      = 32    ; 
localparam     OTP2H_REGISTER_RDATA = 1     ; 
localparam     OTP2H_REGISTER_WDATA = 1     ; 

localparam     CFG2H_ADDRWIDTH      = 32   ; 
localparam     CFG2H_REGISTER_RDATA = 1    ; 
localparam     CFG2H_REGISTER_WDATA = 1    ; 

localparam     MBM2H_ADDRWIDTH      = 32   ; 
localparam     MBM2H_REGISTER_RDATA = 1    ; 
localparam     MBM2H_REGISTER_WDATA = 1    ; 

localparam     MB2H_ADDRWIDTH      = 32   ; 
localparam     MB2H_REGISTER_RDATA = 1    ; 
localparam     MB2H_REGISTER_WDATA = 1    ; 

wire [63:0]                         dut_o_hsm_status             ;
wire [63:0]                         dut_o_hsm_err_hw             ;
wire [63:0]                         dut_o_hsm_err_fw             ;
wire [16-1:0]                dut_o_mbox_irq               ;
wire [31:0]                         dut_o_s_hrdata               ;
wire [1:0]                          dut_o_s_hresp                ;
wire                                dut_o_s_hreadyout            ;
wire                                dut_o_m_hsel                 ;
wire [63:0]                         dut_o_m_haddr                ;
wire [1:0]                          dut_o_m_htrans               ;
wire                                dut_o_m_hwrite               ;
wire [2:0]                          dut_o_m_hburst               ;
wire [2:0]                          dut_o_m_hsize                ;
wire [3:0]                          dut_o_m_hprot                ;
wire                                dut_o_m_hmastlock            ;
wire                                dut_o_m_hready               ;
wire [31:0]                         dut_o_m_hwdata               ;
wire [3:0]                          dut_o_dma_m_awid             ;
wire [64-1:0]              dut_o_dma_m_awaddr           ;
wire [7:0]                          dut_o_dma_m_awlen            ;
wire [2:0]                          dut_o_dma_m_awsize           ;
wire [1:0]                          dut_o_dma_m_awburst          ;
wire                                dut_o_dma_m_awlock           ;
wire [3:0]                          dut_o_dma_m_awcache          ;
wire [2:0]                          dut_o_dma_m_awprot           ;
wire [3:0]                          dut_o_dma_m_awqos            ;
wire [3:0]                          dut_o_dma_m_awregion         ;
wire                                dut_o_dma_m_awvalid          ;
wire [64-1:0]              dut_o_dma_m_wdata            ;
wire [64/8-1:0]            dut_o_dma_m_wstrb            ;
wire                                dut_o_dma_m_wlast            ;
wire                                dut_o_dma_m_wvalid           ;
wire                                dut_o_dma_m_bready           ;
wire [3:0]                          dut_o_dma_m_arid             ;
wire [64-1:0]              dut_o_dma_m_araddr           ;
wire [7:0]                          dut_o_dma_m_arlen            ;
wire [2:0]                          dut_o_dma_m_arsize           ;
wire [1:0]                          dut_o_dma_m_arburst          ;
wire                                dut_o_dma_m_arlock           ;
wire [3:0]                          dut_o_dma_m_arcache          ;
wire [2:0]                          dut_o_dma_m_arprot           ;
wire [3:0]                          dut_o_dma_m_arqos            ;
wire [3:0]                          dut_o_dma_m_arregion         ;
wire                                dut_o_dma_m_arvalid          ;
wire                                dut_o_dma_m_rready           ;
wire                                dut_o_cfg_hsel               ;
wire [31:0]                         dut_o_cfg_haddr              ;
wire [1:0]                          dut_o_cfg_htrans             ;
wire                                dut_o_cfg_hwrite             ;
wire [2:0]                          dut_o_cfg_hburst             ;
wire [2:0]                          dut_o_cfg_hsize              ;
wire [3:0]                          dut_o_cfg_hprot              ;
wire                                dut_o_cfg_hmastlock          ;
wire                                dut_o_cfg_hready             ;
wire [31:0]                         dut_o_cfg_hwdata             ;
wire                                dut_o_otp_hsel               ;
wire [31:0]                         dut_o_otp_haddr              ;
wire [1:0]                          dut_o_otp_htrans             ;
wire                                dut_o_otp_hwrite             ;
wire [2:0]                          dut_o_otp_hburst             ;
wire [2:0]                          dut_o_otp_hsize              ;
wire [3:0]                          dut_o_otp_hprot              ;
wire                                dut_o_otp_hmastlock          ;
wire                                dut_o_otp_hready             ;
wire [31:0]                         dut_o_otp_hwdata             ;
wire                                dut_o_irom_CK                ;
wire                                dut_o_irom_CSB               ;
wire [14-1:0]    dut_o_irom_A                 ;
`ifdef OSR_FPGA_ROM  
wire [38:0]                         dut_o_irom_DI                ;
wire                                dut_o_irom_WEB               ;
`endif
wire                                dut_o_iram_CK                ;
wire [16-1:0]    dut_o_iram_A                 ;
wire                                dut_o_iram_CSB               ;
wire                                dut_o_iram_WEB               ;
wire [38:0]                         dut_o_iram_DI                ;
wire                                dut_o_dram_CK                ;
wire [14-1:0]    dut_o_dram_A                 ;
wire                                dut_o_dram_CSB               ;
wire                                dut_o_dram_WEB               ;
wire [38:0]                         dut_o_dram_DI                ;
wire                                dut_o_kbuf_CK                ;
wire                                dut_o_kbuf_CSB               ;
wire [7:0]                          dut_o_kbuf_A                 ;
wire                                dut_o_kbuf_WEB               ;
wire [38:0]                         dut_o_kbuf_DI                ;
wire                                dut_o_pke_sram0_CK           ;
wire                                dut_o_pke_sram0_CSAN         ;
wire [8:0]                          dut_o_pke_sram0_A            ;
wire                                dut_o_pke_sram0_CSBN         ;
wire [8:0]                          dut_o_pke_sram0_B            ;
wire [71:0]                         dut_o_pke_sram0_DI           ;
wire                                dut_o_pke_sram1_CK           ;
wire                                dut_o_pke_sram1_CSAN         ;
wire [8:0]                          dut_o_pke_sram1_A            ;
wire                                dut_o_pke_sram1_CSBN         ;
wire [8:0]                          dut_o_pke_sram1_B            ;
wire [71:0]                         dut_o_pke_sram1_DI           ;
wire                                dut_o_pke_sram2_CK           ;
wire                                dut_o_pke_sram2_CSAN         ;
wire [8:0]                          dut_o_pke_sram2_A            ;
wire                                dut_o_pke_sram2_CSBN         ;
wire [8:0]                          dut_o_pke_sram2_B            ;
wire [71:0]                         dut_o_pke_sram2_DI           ;
wire                                dut_o_pke_sram3_CK           ;
wire                                dut_o_pke_sram3_CSAN         ;
wire [8:0]                          dut_o_pke_sram3_A            ;
wire                                dut_o_pke_sram3_CSBN         ;
wire [8:0]                          dut_o_pke_sram3_B            ;
wire [71:0]                         dut_o_pke_sram3_DI           ;
wire                                dut_o_tdo_oe                 ;
wire                                dut_o_tdo                    ;
wire                                dut_o_uart_txd_oe            ;
wire                                dut_o_uart_txd               ;
wire [127:0]                        dut_o_soc_dbg_en_128b        ;
wire                                dut_o_trng_rdy               ;
wire                                dut_o_trng_alarm             ;
wire [3:0]                          dut_o_trng_ro_clk            ;
wire [3:0]                          dut_o_trng_ro_out            ;

wire                          otp2h_o_hsel      = 32'h0 ; 
wire [OTP2H_ADDRWIDTH-1:0]    otp2h_o_haddr     = 32'h0 ; 
wire [1:0]                    otp2h_o_htrans    = 32'h0 ; 
wire [2:0]                    otp2h_o_hsize     = 32'h0 ; 
wire [3:0]                    otp2h_o_hprot     = 32'h0 ; 
wire                          otp2h_o_hwrite    = 32'h0 ; 
wire                          otp2h_o_hready    = 32'h0 ; 
wire [31:0]                   otp2h_o_hwdata    = 32'h0 ; 
wire [31:0]                   otp2h_o_prdata    = 32'h0 ; 
wire                          otp2h_o_pready    = 32'h1 ; 
wire                          otp2h_o_pslverr   = 32'h0 ; 

wire                          cfg2h_o_hsel      = 32'h0 ; 
wire [CFG2H_ADDRWIDTH-1:0]    cfg2h_o_haddr     = 32'h0 ; 
wire [1:0]                    cfg2h_o_htrans    = 32'h0 ; 
wire [2:0]                    cfg2h_o_hsize     = 32'h0 ; 
wire [3:0]                    cfg2h_o_hprot     = 32'h0 ; 
wire                          cfg2h_o_hwrite    = 32'h0 ; 
wire                          cfg2h_o_hready    = 32'h0 ; 
wire [31:0]                   cfg2h_o_hwdata    = 32'h0 ; 
wire [31:0]                   cfg2h_o_prdata    = 32'h0 ; 
wire                          cfg2h_o_pready    = 32'h1 ; 
wire                          cfg2h_o_pslverr   = 32'h0 ; 

wire                          mbm2h_o_hsel      = 32'h0 ; 
wire [MBM2H_ADDRWIDTH-1:0]    mbm2h_o_haddr     = 32'h0 ; 
wire [1:0]                    mbm2h_o_htrans    = 32'h0 ; 
wire [2:0]                    mbm2h_o_hsize     = 32'h0 ; 
wire [3:0]                    mbm2h_o_hprot     = 32'h0 ; 
wire                          mbm2h_o_hwrite    = 32'h0 ; 
wire                          mbm2h_o_hready    = 32'h0 ; 
wire [31:0]                   mbm2h_o_hwdata    = 32'h0 ; 
wire [31:0]                   mbm2h_o_prdata    = 32'h0 ; 
wire                          mbm2h_o_pready    = 32'h1 ; 
wire                          mbm2h_o_pslverr   = 32'h0 ; 

wire                         mb2h_o_hreadyout = 32'h0   ; 
wire [31:0]                  mb2h_o_hrdata    = 32'h0   ; 
wire [1:0]                   mb2h_o_hresp     = 32'h0   ; 
wire [MB2H_ADDRWIDTH-1:0]    mb2h_o_paddr     = 32'h0   ; 
wire                         mb2h_o_penable   = 32'h0   ; 
wire                         mb2h_o_pwrite    = 32'h0   ; 
wire [3:0]                   mb2h_o_pstrb     = 32'h0   ; 
wire [2:0]                   mb2h_o_pprot     = 32'h0   ; 
wire [31:0]                  mb2h_o_pwdata    = 32'h0   ; 
wire                         mb2h_o_psel      = 32'h0   ; 
wire                         mb2h_o_apbactive = 32'h0   ; 

wire [31:0]  w_s_hrdata     = dut_o_s_hrdata    ; 
wire [1:0]   w_s_hresp      = dut_o_s_hresp     ; 
wire         w_s_hreadyout  = dut_o_s_hreadyout ; 

wire [31:0] w_otp_haddr_mux = dut_o_otp_haddr;

wire                                dut_i_clk                    = i_clk                 ; 
wire                                dut_i_rst_n                  = i_rst_n               ; 

wire                                dut_i_scan_mode              = i_scan_mode           ; 

wire                                dut_i_scan_icg_enable        = i_scan_icg_enable     ; 

wire [31:0]                         dut_i_soc_status             = i_soc_status          ; 
wire [31:0]                         dut_i_soc_err                = i_soc_err             ;

wire                                dut_i_s_hsel                 = i_s_hsel              ; 
wire [31:0]                         dut_i_s_haddr                = i_s_haddr             ; 
wire [1:0]                          dut_i_s_htrans               = i_s_htrans            ; 
wire                                dut_i_s_hwrite               = i_s_hwrite            ; 
wire [2:0]                          dut_i_s_hburst               = i_s_hburst            ; 
wire [2:0]                          dut_i_s_hsize                = i_s_hsize             ; 
wire [3:0]                          dut_i_s_hprot                = i_s_hprot             ; 
wire                                dut_i_s_hmastlock            = i_s_hmastlock         ; 
wire                                dut_i_s_hready               = i_s_hready            ; 
wire [31:0]                         dut_i_s_hwdata               = i_s_hwdata            ; 

wire [31:0]                         dut_i_m_hrdata               = i_m_hrdata            ; 
wire [1:0]                          dut_i_m_hresp                = i_m_hresp             ; 
wire                                dut_i_m_hreadyout            = i_m_hreadyout         ; 

wire                                dut_i_dma_m_awready          = i_dma_m_awready ;
wire                                dut_i_dma_m_wready           = i_dma_m_wready  ;
wire [3:0]                          dut_i_dma_m_bid              = i_dma_m_bid     ;
wire [1:0]                          dut_i_dma_m_bresp            = i_dma_m_bresp   ;
wire                                dut_i_dma_m_bvalid           = i_dma_m_bvalid  ;
wire                                dut_i_dma_m_arready          = i_dma_m_arready ;
wire [3:0]                          dut_i_dma_m_rid              = i_dma_m_rid     ;   
wire [64-1:0]              dut_i_dma_m_rdata            = i_dma_m_rdata   ;  
wire [1:0]                          dut_i_dma_m_rresp            = i_dma_m_rresp   ;  
wire                                dut_i_dma_m_rlast            = i_dma_m_rlast   ;  
wire                                dut_i_dma_m_rvalid           = i_dma_m_rvalid  ;  

wire [31:0]                         dut_i_cfg_hrdata             = i_cfg_hrdata          ;
wire [1:0]                          dut_i_cfg_hresp              = i_cfg_hresp           ;
wire                                dut_i_cfg_hreadyout          = i_cfg_hreadyout       ;

wire [31:0]                         dut_i_otp_hrdata             = i_otp_hrdata          ; 
wire [1:0]                          dut_i_otp_hresp              = i_otp_hresp           ; 
wire                                dut_i_otp_hreadyout          = i_otp_hreadyout       ; 

wire [38:0]                         dut_i_irom_DO                = i_irom_DO             ; 
`ifdef OSR_FPGA_ROM  
`endif

wire [38:0]                         dut_i_iram_DO                = i_iram_DO             ; 

wire [38:0]                         dut_i_dram_DO                = i_dram_DO             ; 

wire [38:0]                         dut_i_kbuf_DO                = i_kbuf_DO             ; 

wire [71:0]                         dut_i_pke_sram0_DO           = i_pke_sram0_DO        ; 
wire [71:0]                         dut_i_pke_sram1_DO           = i_pke_sram1_DO        ; 
wire [71:0]                         dut_i_pke_sram2_DO           = i_pke_sram2_DO        ; 
wire [71:0]                         dut_i_pke_sram3_DO           = i_pke_sram3_DO        ; 

wire                                dut_i_trst_n                 = i_trst_n              ; 
wire                                dut_i_tck                    = i_tck                 ;
wire                                dut_i_tms                    = i_tms                 ;
wire                                dut_i_tdi                    = i_tdi                 ;

wire                                dut_i_uart_rxd               = i_uart_rxd            ; 

    wire                 w_otp_hsel              = dut_o_otp_hsel      ;
    wire [31:0]          w_otp_haddr             = w_otp_haddr_mux     ;
    wire [1:0]           w_otp_htrans            = dut_o_otp_htrans    ;
    wire                 w_otp_hwrite            = dut_o_otp_hwrite    ;
    wire [2:0]           w_otp_hburst            = dut_o_otp_hburst    ;
    wire [2:0]           w_otp_hsize             = dut_o_otp_hsize     ;
    wire [3:0]           w_otp_hprot             = dut_o_otp_hprot     ;
    wire                 w_otp_hmastlock         = dut_o_otp_hmastlock ;
    wire                 w_otp_hready            = dut_o_otp_hready    ;
    wire [31:0]          w_otp_hwdata            = dut_o_otp_hwdata    ;

    wire                 w_cfg_hsel              = dut_o_cfg_hsel      ;
    wire [31:0]          w_cfg_haddr             = dut_o_cfg_haddr     ;
    wire [1:0]           w_cfg_htrans            = dut_o_cfg_htrans    ;
    wire                 w_cfg_hwrite            = dut_o_cfg_hwrite    ;
    wire [2:0]           w_cfg_hburst            = dut_o_cfg_hburst    ;
    wire [2:0]           w_cfg_hsize             = dut_o_cfg_hsize     ;
    wire [3:0]           w_cfg_hprot             = dut_o_cfg_hprot     ;
    wire                 w_cfg_hmastlock         = dut_o_cfg_hmastlock ;
    wire                 w_cfg_hready            = dut_o_cfg_hready    ;
    wire [31:0]          w_cfg_hwdata            = dut_o_cfg_hwdata    ;

    wire                 w_m_hsel                =dut_o_m_hsel      ;   
    wire [63:0]          w_m_haddr               =dut_o_m_haddr     ;
    wire [1:0]           w_m_htrans              =dut_o_m_htrans    ;
    wire                 w_m_hwrite              =dut_o_m_hwrite    ;
    wire [2:0]           w_m_hburst              =dut_o_m_hburst    ;
    wire [2:0]           w_m_hsize               =dut_o_m_hsize     ;
    wire [3:0]           w_m_hprot               =dut_o_m_hprot     ;
    wire                 w_m_hmastlock           =dut_o_m_hmastlock ;
    wire                 w_m_hready              =dut_o_m_hready    ;
    wire [31:0]          w_m_hwdata              =dut_o_m_hwdata    ;

assign o_hsm_status             = dut_o_hsm_status          ; 

assign o_hsm_err_hw             = dut_o_hsm_err_hw          ; 
assign o_hsm_err_fw             = dut_o_hsm_err_fw          ; 
assign o_mbox_irq               = dut_o_mbox_irq            ; 

assign o_s_hrdata               = w_s_hrdata            ; 
assign o_s_hresp                = w_s_hresp             ; 
assign o_s_hreadyout            = w_s_hreadyout         ; 
assign o_m_hsel                 = w_m_hsel              ; 

assign o_m_haddr                = w_m_haddr             ; 

assign o_m_htrans               = w_m_htrans            ; 
assign o_m_hwrite               = w_m_hwrite            ; 
assign o_m_hburst               = w_m_hburst            ; 
assign o_m_hsize                = w_m_hsize             ; 
assign o_m_hprot                = w_m_hprot             ; 
assign o_m_hmastlock            = w_m_hmastlock         ; 
assign o_m_hready               = w_m_hready            ; 
assign o_m_hwdata               = w_m_hwdata            ; 

assign o_dma_m_awid             = dut_o_dma_m_awid        ;
assign o_dma_m_awaddr           = dut_o_dma_m_awaddr      ;
assign o_dma_m_awlen            = dut_o_dma_m_awlen       ;
assign o_dma_m_awsize           = dut_o_dma_m_awsize      ;
assign o_dma_m_awburst          = dut_o_dma_m_awburst     ;
assign o_dma_m_awlock           = dut_o_dma_m_awlock      ;
assign o_dma_m_awcache          = dut_o_dma_m_awcache     ;
assign o_dma_m_awprot           = dut_o_dma_m_awprot      ;
assign o_dma_m_awqos            = dut_o_dma_m_awqos       ;
assign o_dma_m_awregion         = dut_o_dma_m_awregion    ;
assign o_dma_m_awvalid          = dut_o_dma_m_awvalid     ; 
assign o_dma_m_wdata            = dut_o_dma_m_wdata       ;
assign o_dma_m_wstrb            = dut_o_dma_m_wstrb       ;
assign o_dma_m_wlast            = dut_o_dma_m_wlast       ;
assign o_dma_m_wvalid           = dut_o_dma_m_wvalid      ;
assign o_dma_m_bready           = dut_o_dma_m_bready      ;
assign o_dma_m_arid             = dut_o_dma_m_arid        ;
assign o_dma_m_araddr           = dut_o_dma_m_araddr      ; 
assign o_dma_m_arlen            = dut_o_dma_m_arlen       ;
assign o_dma_m_arsize           = dut_o_dma_m_arsize      ;
assign o_dma_m_arburst          = dut_o_dma_m_arburst     ;
assign o_dma_m_arlock           = dut_o_dma_m_arlock      ; 
assign o_dma_m_arcache          = dut_o_dma_m_arcache     ; 
assign o_dma_m_arprot           = dut_o_dma_m_arprot      ; 
assign o_dma_m_arqos            = dut_o_dma_m_arqos       ; 
assign o_dma_m_arregion         = dut_o_dma_m_arregion    ; 
assign o_dma_m_arvalid          = dut_o_dma_m_arvalid     ; 
assign o_dma_m_rready           = dut_o_dma_m_rready      ;

assign o_cfg_hsel               = w_cfg_hsel               ; 
assign o_cfg_haddr              = w_cfg_haddr              ; 
assign o_cfg_htrans             = w_cfg_htrans             ; 
assign o_cfg_hwrite             = w_cfg_hwrite             ; 
assign o_cfg_hburst             = w_cfg_hburst             ; 
assign o_cfg_hsize              = w_cfg_hsize              ; 
assign o_cfg_hprot              = w_cfg_hprot              ; 
assign o_cfg_hmastlock          = w_cfg_hmastlock          ; 
assign o_cfg_hready             = w_cfg_hready             ; 
assign o_cfg_hwdata             = w_cfg_hwdata             ; 

assign o_otp_hsel               = w_otp_hsel               ;
assign o_otp_haddr              = w_otp_haddr              ;
assign o_otp_htrans             = w_otp_htrans             ;
assign o_otp_hwrite             = w_otp_hwrite             ;
assign o_otp_hburst             = w_otp_hburst             ;
assign o_otp_hsize              = w_otp_hsize              ;
assign o_otp_hprot              = w_otp_hprot              ;
assign o_otp_hmastlock          = w_otp_hmastlock          ;
assign o_otp_hready             = w_otp_hready             ;
assign o_otp_hwdata             = w_otp_hwdata             ;

assign o_irom_CK                = dut_o_irom_CK             ; 
assign o_irom_CSB               = dut_o_irom_CSB            ; 
assign o_irom_A                 = dut_o_irom_A              ; 
`ifdef OSR_FPGA_ROM  
assign o_irom_DI                = dut_o_irom_DI             ; 
assign o_irom_WEB               = dut_o_irom_WEB            ; 
`endif

assign o_iram_CK                = dut_o_iram_CK             ; 
assign o_iram_A                 = dut_o_iram_A              ; 
assign o_iram_CSB               = dut_o_iram_CSB            ; 
assign o_iram_WEB               = dut_o_iram_WEB            ; 
assign o_iram_DI                = dut_o_iram_DI             ; 

assign o_dram_CK                = dut_o_dram_CK             ; 
assign o_dram_A                 = dut_o_dram_A              ; 
assign o_dram_CSB               = dut_o_dram_CSB            ; 

assign o_dram_WEB               = dut_o_dram_WEB            ; 
assign o_dram_DI                = dut_o_dram_DI             ; 

assign o_kbuf_CK                = dut_o_kbuf_CK             ; 
assign o_kbuf_CSB               = dut_o_kbuf_CSB            ; 
assign o_kbuf_A                 = dut_o_kbuf_A              ; 
assign o_kbuf_WEB               = dut_o_kbuf_WEB            ; 
assign o_kbuf_DI                = dut_o_kbuf_DI             ; 

assign o_pke_sram0_CK           = dut_o_pke_sram0_CK        ; 
assign o_pke_sram0_CSAN         = dut_o_pke_sram0_CSAN      ; 
assign o_pke_sram0_A            = dut_o_pke_sram0_A         ; 
assign o_pke_sram0_CSBN         = dut_o_pke_sram0_CSBN      ; 
assign o_pke_sram0_B            = dut_o_pke_sram0_B         ; 
assign o_pke_sram0_DI           = dut_o_pke_sram0_DI        ; 
assign o_pke_sram1_CK           = dut_o_pke_sram1_CK        ; 
assign o_pke_sram1_CSAN         = dut_o_pke_sram1_CSAN      ; 
assign o_pke_sram1_A            = dut_o_pke_sram1_A         ; 
assign o_pke_sram1_CSBN         = dut_o_pke_sram1_CSBN      ; 
assign o_pke_sram1_B            = dut_o_pke_sram1_B         ; 
assign o_pke_sram1_DI           = dut_o_pke_sram1_DI        ; 
assign o_pke_sram2_CK           = dut_o_pke_sram2_CK        ; 
assign o_pke_sram2_CSAN         = dut_o_pke_sram2_CSAN      ; 
assign o_pke_sram2_A            = dut_o_pke_sram2_A         ; 
assign o_pke_sram2_CSBN         = dut_o_pke_sram2_CSBN      ; 
assign o_pke_sram2_B            = dut_o_pke_sram2_B         ; 
assign o_pke_sram2_DI           = dut_o_pke_sram2_DI        ; 
assign o_pke_sram3_CK           = dut_o_pke_sram3_CK        ; 
assign o_pke_sram3_CSAN         = dut_o_pke_sram3_CSAN      ; 
assign o_pke_sram3_A            = dut_o_pke_sram3_A         ; 
assign o_pke_sram3_CSBN         = dut_o_pke_sram3_CSBN      ; 
assign o_pke_sram3_B            = dut_o_pke_sram3_B         ; 
assign o_pke_sram3_DI           = dut_o_pke_sram3_DI        ; 

assign o_tdo_oe                 = dut_o_tdo_oe     ; 
assign o_tdo                    = dut_o_tdo        ;

assign o_uart_txd_oe            = dut_o_uart_txd_oe         ; 
assign o_uart_txd               = dut_o_uart_txd            ; 
assign o_soc_dbg_en_128b        = dut_o_soc_dbg_en_128b     ; 
assign o_trng_ro_clk            = dut_o_trng_ro_clk         ; 
assign o_trng_ro_out            = dut_o_trng_ro_out         ; 

osr_ehsm_top u_osr_ehsm_top (
    .i_clk                       ( dut_i_clk                    ), 
    .i_rst_n                     ( dut_i_rst_n                  ), 
    .i_scan_mode                 ( dut_i_scan_mode              ), 
    .i_scan_icg_enable           ( dut_i_scan_icg_enable        ), 
    .i_soc_status                ( dut_i_soc_status             ), 
    .i_soc_err                   ( dut_i_soc_err                ), 
    .o_hsm_status                ( dut_o_hsm_status             ), 
    .o_hsm_err_hw                ( dut_o_hsm_err_hw             ), 
    .o_hsm_err_fw                ( dut_o_hsm_err_fw             ), 
    .o_mbox_irq                  ( dut_o_mbox_irq               ), 
    .i_s_hsel                    ( dut_i_s_hsel                 ), 
    .i_s_haddr                   ( dut_i_s_haddr                ), 
    .i_s_htrans                  ( dut_i_s_htrans               ), 
    .i_s_hwrite                  ( dut_i_s_hwrite               ), 
    .i_s_hburst                  ( dut_i_s_hburst               ), 
    .i_s_hsize                   ( dut_i_s_hsize                ), 
    .i_s_hprot                   ( dut_i_s_hprot                ), 
    .i_s_hmastlock               ( dut_i_s_hmastlock            ), 
    .i_s_hready                  ( dut_i_s_hready               ), 
    .i_s_hwdata                  ( dut_i_s_hwdata               ), 
    .o_s_hrdata                  ( dut_o_s_hrdata               ), 
    .o_s_hresp                   ( dut_o_s_hresp                ), 
    .o_s_hreadyout               ( dut_o_s_hreadyout            ), 
    .o_m_hsel                    ( dut_o_m_hsel                 ), 
    .o_m_haddr                   ( dut_o_m_haddr                ), 
    .o_m_htrans                  ( dut_o_m_htrans               ), 
    .o_m_hwrite                  ( dut_o_m_hwrite               ), 
    .o_m_hburst                  ( dut_o_m_hburst               ), 
    .o_m_hsize                   ( dut_o_m_hsize                ), 
    .o_m_hprot                   ( dut_o_m_hprot                ), 
    .o_m_hmastlock               ( dut_o_m_hmastlock            ), 
    .o_m_hready                  ( dut_o_m_hready               ), 
    .o_m_hwdata                  ( dut_o_m_hwdata               ), 
    .i_m_hrdata                  ( dut_i_m_hrdata               ), 
    .i_m_hresp                   ( dut_i_m_hresp                ), 
    .i_m_hreadyout               ( dut_i_m_hreadyout            ), 
    .o_dma_m_awid                ( dut_o_dma_m_awid             ), 
    .o_dma_m_awaddr              ( dut_o_dma_m_awaddr           ), 
    .o_dma_m_awlen               ( dut_o_dma_m_awlen            ), 
    .o_dma_m_awsize              ( dut_o_dma_m_awsize           ), 
    .o_dma_m_awburst             ( dut_o_dma_m_awburst          ), 
    .o_dma_m_awlock              ( dut_o_dma_m_awlock           ), 
    .o_dma_m_awcache             ( dut_o_dma_m_awcache          ), 
    .o_dma_m_awprot              ( dut_o_dma_m_awprot           ), 
    .o_dma_m_awqos               ( dut_o_dma_m_awqos            ), 
    .o_dma_m_awregion            ( dut_o_dma_m_awregion         ), 
    .o_dma_m_awvalid             ( dut_o_dma_m_awvalid          ), 
    .i_dma_m_awready             ( dut_i_dma_m_awready          ), 
    .o_dma_m_wdata               ( dut_o_dma_m_wdata            ), 
    .o_dma_m_wstrb               ( dut_o_dma_m_wstrb            ), 
    .o_dma_m_wlast               ( dut_o_dma_m_wlast            ), 
    .o_dma_m_wvalid              ( dut_o_dma_m_wvalid           ), 
    .i_dma_m_wready              ( dut_i_dma_m_wready           ), 
    .i_dma_m_bid                 ( dut_i_dma_m_bid              ), 
    .i_dma_m_bresp               ( dut_i_dma_m_bresp            ), 
    .i_dma_m_bvalid              ( dut_i_dma_m_bvalid           ), 
    .o_dma_m_bready              ( dut_o_dma_m_bready           ), 
    .o_dma_m_arid                ( dut_o_dma_m_arid             ), 
    .o_dma_m_araddr              ( dut_o_dma_m_araddr           ), 
    .o_dma_m_arlen               ( dut_o_dma_m_arlen            ), 
    .o_dma_m_arsize              ( dut_o_dma_m_arsize           ), 
    .o_dma_m_arburst             ( dut_o_dma_m_arburst          ), 
    .o_dma_m_arlock              ( dut_o_dma_m_arlock           ), 
    .o_dma_m_arcache             ( dut_o_dma_m_arcache          ), 
    .o_dma_m_arprot              ( dut_o_dma_m_arprot           ), 
    .o_dma_m_arqos               ( dut_o_dma_m_arqos            ), 
    .o_dma_m_arregion            ( dut_o_dma_m_arregion         ), 
    .o_dma_m_arvalid             ( dut_o_dma_m_arvalid          ), 
    .i_dma_m_arready             ( dut_i_dma_m_arready          ), 
    .i_dma_m_rid                 ( dut_i_dma_m_rid              ), 
    .i_dma_m_rdata               ( dut_i_dma_m_rdata            ), 
    .i_dma_m_rresp               ( dut_i_dma_m_rresp            ), 
    .i_dma_m_rlast               ( dut_i_dma_m_rlast            ), 
    .i_dma_m_rvalid              ( dut_i_dma_m_rvalid           ), 
    .o_dma_m_rready              ( dut_o_dma_m_rready           ), 
    .o_cfg_hsel                  ( dut_o_cfg_hsel               ), 
    .o_cfg_haddr                 ( dut_o_cfg_haddr              ), 
    .o_cfg_htrans                ( dut_o_cfg_htrans             ), 
    .o_cfg_hwrite                ( dut_o_cfg_hwrite             ), 
    .o_cfg_hburst                ( dut_o_cfg_hburst             ), 
    .o_cfg_hsize                 ( dut_o_cfg_hsize              ), 
    .o_cfg_hprot                 ( dut_o_cfg_hprot              ), 
    .o_cfg_hmastlock             ( dut_o_cfg_hmastlock          ), 
    .o_cfg_hready                ( dut_o_cfg_hready             ), 
    .o_cfg_hwdata                ( dut_o_cfg_hwdata             ), 
    .i_cfg_hrdata                ( dut_i_cfg_hrdata             ), 
    .i_cfg_hresp                 ( dut_i_cfg_hresp              ), 
    .i_cfg_hreadyout             ( dut_i_cfg_hreadyout          ), 
    .o_otp_hsel                  ( dut_o_otp_hsel               ), 
    .o_otp_haddr                 ( dut_o_otp_haddr              ), 
    .o_otp_htrans                ( dut_o_otp_htrans             ), 
    .o_otp_hwrite                ( dut_o_otp_hwrite             ), 
    .o_otp_hburst                ( dut_o_otp_hburst             ), 
    .o_otp_hsize                 ( dut_o_otp_hsize              ), 
    .o_otp_hprot                 ( dut_o_otp_hprot              ), 
    .o_otp_hmastlock             ( dut_o_otp_hmastlock          ), 
    .o_otp_hready                ( dut_o_otp_hready             ), 
    .o_otp_hwdata                ( dut_o_otp_hwdata             ), 
    .i_otp_hrdata                ( dut_i_otp_hrdata             ), 
    .i_otp_hresp                 ( dut_i_otp_hresp              ), 
    .i_otp_hreadyout             ( dut_i_otp_hreadyout          ), 
    .o_irom_CK                   ( dut_o_irom_CK                ), 
    .o_irom_CSB                  ( dut_o_irom_CSB               ), 
    .o_irom_A                    ( dut_o_irom_A                 ), 
    .i_irom_DO                   ( dut_i_irom_DO                ), 
    `ifdef OSR_FPGA_ROM  
    .o_irom_DI                   ( dut_o_irom_DI                ), 
    .o_irom_WEB                  ( dut_o_irom_WEB               ), 
    `endif
    .o_iram_CK                   ( dut_o_iram_CK                ), 
    .o_iram_A                    ( dut_o_iram_A                 ), 
    .o_iram_CSB                  ( dut_o_iram_CSB               ), 
    .o_iram_WEB                  ( dut_o_iram_WEB               ), 
    .o_iram_DI                   ( dut_o_iram_DI                ), 
    .i_iram_DO                   ( dut_i_iram_DO                ), 
    .o_dram_CK                   ( dut_o_dram_CK                ), 
    .o_dram_A                    ( dut_o_dram_A                 ), 
    .o_dram_CSB                  ( dut_o_dram_CSB               ), 
    .o_dram_WEB                  ( dut_o_dram_WEB               ), 
    .o_dram_DI                   ( dut_o_dram_DI                ), 
    .i_dram_DO                   ( dut_i_dram_DO                ), 
    .o_kbuf_CK                   ( dut_o_kbuf_CK                ), 
    .o_kbuf_CSB                  ( dut_o_kbuf_CSB               ), 
    .o_kbuf_A                    ( dut_o_kbuf_A                 ), 
    .o_kbuf_WEB                  ( dut_o_kbuf_WEB               ), 
    .o_kbuf_DI                   ( dut_o_kbuf_DI                ), 
    .i_kbuf_DO                   ( dut_i_kbuf_DO                ), 
    .o_pke_sram0_CK              ( dut_o_pke_sram0_CK           ), 
    .o_pke_sram0_CSAN            ( dut_o_pke_sram0_CSAN         ), 
    .o_pke_sram0_A               ( dut_o_pke_sram0_A            ), 
    .i_pke_sram0_DO              ( dut_i_pke_sram0_DO           ), 
    .o_pke_sram0_CSBN            ( dut_o_pke_sram0_CSBN         ), 
    .o_pke_sram0_B               ( dut_o_pke_sram0_B            ), 
    .o_pke_sram0_DI              ( dut_o_pke_sram0_DI           ), 
    .o_pke_sram1_CK              ( dut_o_pke_sram1_CK           ), 
    .o_pke_sram1_CSAN            ( dut_o_pke_sram1_CSAN         ), 
    .o_pke_sram1_A               ( dut_o_pke_sram1_A            ), 
    .i_pke_sram1_DO              ( dut_i_pke_sram1_DO           ), 
    .o_pke_sram1_CSBN            ( dut_o_pke_sram1_CSBN         ), 
    .o_pke_sram1_B               ( dut_o_pke_sram1_B            ), 
    .o_pke_sram1_DI              ( dut_o_pke_sram1_DI           ), 
    .o_pke_sram2_CK              ( dut_o_pke_sram2_CK           ), 
    .o_pke_sram2_CSAN            ( dut_o_pke_sram2_CSAN         ), 
    .o_pke_sram2_A               ( dut_o_pke_sram2_A            ), 
    .i_pke_sram2_DO              ( dut_i_pke_sram2_DO           ), 
    .o_pke_sram2_CSBN            ( dut_o_pke_sram2_CSBN         ), 
    .o_pke_sram2_B               ( dut_o_pke_sram2_B            ), 
    .o_pke_sram2_DI              ( dut_o_pke_sram2_DI           ), 
    .o_pke_sram3_CK              ( dut_o_pke_sram3_CK           ), 
    .o_pke_sram3_CSAN            ( dut_o_pke_sram3_CSAN         ), 
    .o_pke_sram3_A               ( dut_o_pke_sram3_A            ), 
    .i_pke_sram3_DO              ( dut_i_pke_sram3_DO           ), 
    .o_pke_sram3_CSBN            ( dut_o_pke_sram3_CSBN         ), 
    .o_pke_sram3_B               ( dut_o_pke_sram3_B            ), 
    .o_pke_sram3_DI              ( dut_o_pke_sram3_DI           ), 
    .i_trst_n                    ( dut_i_trst_n                 ), 
    .i_tck                       ( dut_i_tck                    ), 
    .i_tms                       ( dut_i_tms                    ), 
    .i_tdi                       ( dut_i_tdi                    ), 
    .o_tdo_oe                    ( dut_o_tdo_oe                 ), 
    .o_tdo                       ( dut_o_tdo                    ), 
    .i_uart_rxd                  ( dut_i_uart_rxd               ), 
    .o_uart_txd_oe               ( dut_o_uart_txd_oe            ), 
    .o_uart_txd                  ( dut_o_uart_txd               ), 
    .o_soc_dbg_en_128b           ( dut_o_soc_dbg_en_128b        ), 
    .o_trng_rdy                  ( dut_o_trng_rdy               ), 
    .o_trng_alarm                ( dut_o_trng_alarm             ), 
    .o_trng_ro_clk               ( dut_o_trng_ro_clk            ), 
    .o_trng_ro_out               ( dut_o_trng_ro_out            )  
    ); 

endmodule 
