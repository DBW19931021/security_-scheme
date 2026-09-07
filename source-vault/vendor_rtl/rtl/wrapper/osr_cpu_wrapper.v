//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_cpu_wrapper (
    input   wire                i_clk                   ,
    input   wire                i_rst_n                 ,
    input   wire                i_keep_logic            ,
    input   wire                i_dbg_stop              ,
    input   wire                i_scan_mode             ,
    input   wire                i_scan_enable           ,
    input   wire                i_dbg_toggle_a          ,
    input   wire                i_mtime_toggle_a        ,
    input   wire [63:0]         i_clic_irq              ,
    output  wire                o_dm_ndmreset           ,
    input   wire                i_dm_ndmreset_n         ,
    input   wire                i_trst_n                ,
    input   wire                i_tck                   ,
    input   wire                i_tms                   ,
    input   wire                i_tdi                   , 
    output  wire                o_tdo_oe                ,
    output  wire                o_tdo                   ,

    output  wire [1:0]          o_ilm_htrans            ,
    output  wire                o_ilm_hwrite            ,
    output  wire                o_ilm_hmastlock         ,
    output  wire [35:0]         o_ilm_hwdata            ,
    output  wire [31:0]         o_ilm_haddr             ,
    output  wire [2:0]          o_ilm_hsize             ,
    output  wire [2:0]          o_ilm_hburst            ,
    output  wire [3:0]          o_ilm_hprot             ,
    output  wire [7:0]          o_ilm_hmaster           ,

    input   wire [35:0]         i_ilm_hrdata            ,
    input   wire [1:0]          i_ilm_hresp             ,
    input   wire                i_ilm_hready            ,

    output  wire                o_dlm_hsel              ,
    output  wire [1:0]          o_dlm_htrans            ,
    output  wire                o_dlm_hwrite            ,
    output  wire [31:0]         o_dlm_haddr             ,
    output  wire [2:0]          o_dlm_hsize             ,
    output  wire [2:0]          o_dlm_hburst            ,
    output  wire                o_dlm_hmastlock         ,
    output  wire [35:0]         o_dlm_hwdata            ,
    output  wire [3:0]          o_dlm_hprot             ,
    output  wire [7:0]          o_dlm_hmaster           ,
    input   wire [35:0]         i_dlm_hrdata            ,
    input   wire [1:0]          i_dlm_hresp             ,
    input   wire                i_dlm_hready            ,

    output  wire                o_hsel                  ,
    output  wire [1:0]          o_htrans                ,
    output  wire                o_hwrite                ,
    output  wire [31:0]         o_haddr                 ,
    output  wire [2:0]          o_hsize                 ,
    output  wire [2:0]          o_hburst                ,
    output  wire                o_hmastlock             ,
    output  wire [35:0]         o_hwdata                ,
    output  wire [3:0]          o_hprot                 ,
    output  wire [7:0]          o_master                ,
    input   wire [35:0]         i_hrdata                ,
    input   wire [1:0]          i_hresp                 ,
    input   wire                i_hready                ,

    input   wire                i_dram_cipher_en        ,
    input   wire [7:0]          i_dram_cipher_key       ,
    input   wire [31:0]         i_dram_cipher_dnonce    ,
    input   wire [14-1:0] i_dram_cipher_anonce    ,
    input   wire [7:0]          i_dram_ecc_ck_en        ,
    input   wire [7:0]          i_dram_ecc_err_rsp_en   ,
    input   wire [7:0]          i_dram_ecc_tm_en        ,
    input   wire [6:0]          i_dram_ecc_tm_ckbits    ,

    output wire                 o_dram_ecc_dec_sec      , 
    output wire                 o_dram_ecc_dec_ded      , 
    output wire  [14-1:0] o_dram_ecc_err_addr     ,

    output  wire                o_dram_web              , 
    output  wire [38:0]         o_dram_wdata            , 
    input   wire [38:0]         i_dram_rdata            ,
    output  wire [14-1:0] o_dram_addr             , 
    output  wire                o_dram_csb              ,

    output wire                 o_hart_halted           , 
    output wire                 o_wfi                   ,

    input  wire                 i_ahb_bus_pr_en         ,
    output wire                 o_ipre_set_bus_pr_alarm , 
    output wire                 o_dpre_set_bus_pr_alarm , 
    output wire                 o_spre_set_bus_pr_alarm , 

    input  wire                 i_mem_init_rst_n        ,
    output wire                 o_mem_init_done         

);

localparam P_DRAM_CIPHER_EN = 0 ;

localparam P_DRAM_ECC_EN = 1 ; 

localparam P_DRAM_BYTE_OPT_EN = 1;

localparam P_DRAM_TIMING_OPT = 0;

localparam DRAM_INIT     = 1;

localparam     ECC_EN_SYN_WIDTH = 8; 

localparam     HDRAM_P_BUS_AW         = 16; 
localparam     HDRAM_P_BUS_DW         = 32;
localparam     HDRAM_P_BUS_NO_WORD    = P_DRAM_BYTE_OPT_EN;
localparam     HDRAM_P_AHB_TIMING     = 0;
localparam     HDRAM_P_AHB2RAM_TIMING = `OSR_DRAM_AHB_BUS_TIMING_ISOLATION; 
localparam     HDRAM_P_CIPHER_EN      = P_DRAM_CIPHER_EN;
localparam     HDRAM_P_RAM_BLOCK_AW   = 2;
localparam     HDRAM_P_ECC_EN         = P_DRAM_ECC_EN;
localparam     HDRAM_P_ECC_TM_CHB     = (HDRAM_P_BUS_DW == 32) ? 7 : (HDRAM_P_BUS_DW == 64) ? 8 : 7; 
localparam     HDRAM_P_RAM_DW         = (HDRAM_P_ECC_EN == 0)? HDRAM_P_BUS_DW : (HDRAM_P_BUS_DW == 32) ? 7+32 : (HDRAM_P_BUS_DW ==64) ? 8+64 : 32;
localparam     HDRAM_P_MEM_INIT       = DRAM_INIT;
localparam     HDRAM_P_MEM_INIT_DEEP  = 64*1024/4;

wire        w_cpu_jtag_DRV_TDO     ;
wire        w_cpu_jtag_TDO         ;
wire [1:0]  w_cpu_ilm_htrans       ;
wire        w_cpu_ilm_hwrite       ;
wire        w_cpu_ilm_hmastlock    ;
wire [31:0] w_cpu_ilm_hwdata       ;
wire [2:0]  w_cpu_ilm_hsize        ;
wire [2:0]  w_cpu_ilm_hburst       ;
wire [3:0]  w_cpu_ilm_hprot        ;
wire        w_cpu_hsel             ;
wire [1:0]  w_cpu_htrans           ;
wire        w_cpu_hwrite           ;
wire [31:0] w_cpu_haddr            ;
wire [2:0]  w_cpu_hsize            ;
wire [2:0]  w_cpu_hburst           ;
wire        w_cpu_hmastlock        ;
wire [31:0] w_cpu_hwdata           ;
wire [3:0]  w_cpu_hprot            ;
wire [1:0]  w_cpu_master           ;
wire        w_cpu_core_wfi_mode    ;
wire        w_cpu_core_sleep_value ;
wire        w_cpu_trace_interrupt  ;
wire        w_cpu_trace_iexception ;
wire        w_cpu_trace_ivalid     ;
wire        w_cpu_sysrstreq        ;
wire        w_cpu_hart_halted      ;

wire w_dft_icg_scan_en = i_scan_enable;

wire [31:0] w_cpu_reset_vector = 32'h1000_0000 ;

reg             wfi              ;
reg             halted           ;

wire             wcpu_clk_gate_en         ;
wire             wcpu_dm_ndmreset         ;
wire             wcpu_soft_rst_req        ;
wire             wcpu_core_sleeping       ;
wire             wcpu_core_deep_sleeping  ;
wire             wcpu_halted              ;
wire             wcpu_lockup              ;
wire             wcpu_tdo                 ;
wire             wcpu_tdo_en              ;
wire [31:0]      wcpu_i_haddr             ;
wire [2:0]       wcpu_i_hburst            ;
wire [3:0]       wcpu_i_hprot             ;
wire [2:0]       wcpu_i_hsize             ;
wire [1:0]       wcpu_i_htrans            ;
wire             wcpu_i_hwrite            ;
wire [31:0]      wcpu_i_hwdata            ;
wire [1:0]       wcpu_i_hmaster           ;
wire [31:0]      wcpu_d_haddr             ;
wire [2:0]       wcpu_d_hburst            ;
wire [3:0]       wcpu_d_hprot             ;
wire [2:0]       wcpu_d_hsize             ;
wire [1:0]       wcpu_d_htrans            ;
wire             wcpu_d_hwrite            ;
wire [31:0]      wcpu_d_hwdata            ;
wire [1:0]       wcpu_d_hmaster           ;
wire [31:0]      wcpu_m_haddr             ;
wire [2:0]       wcpu_m_hburst            ;
wire [3:0]       wcpu_m_hprot             ;
wire [2:0]       wcpu_m_hsize             ;
wire [1:0]       wcpu_m_htrans            ;
wire             wcpu_m_hwrite            ;
wire [31:0]      wcpu_m_hwdata            ;
wire [1:0]       wcpu_m_hmaster           ;
wire [15:0]      wcpu_err_bus             ;



wire                             hdram_o_ecc_dec_sec       = 1'b0;
wire                             hdram_o_ecc_dec_ded       = 1'b0;
wire [HDRAM_P_BUS_AW-3:0]        hdram_o_ecc_err_addr      = {(HDRAM_P_BUS_AW-2){1'b0}};
wire                             hdram_o_hreadyout         = 1'b1;
wire                             hdram_o_ram_cs            = 1'b0;
wire [3:0]                       hdram_o_ram_wen           = 4'b0;
wire [HDRAM_P_BUS_AW-3:0]        hdram_o_ram_addr          = {(HDRAM_P_BUS_AW-2){1'b0}};
wire [HDRAM_P_RAM_DW-1:0]        hdram_o_ram_wdata         = {(HDRAM_P_RAM_DW){1'b0}};
wire                             hdram_o_mem_init_done     = 1'b0;

wire           ipre_o_set_bus_pr_alarm = 1'b0   ;
wire [7:0]     ipre_o_e2s_hmaster      = 8'b0   ;
wire [3:0]     ipre_o_e2s_parity       = 4'b0   ;

wire           dpre_o_set_bus_pr_alarm = 1'b0   ;
wire [7:0]     dpre_o_e2s_hmaster      = 8'b0   ;
wire [3:0]     dpre_o_e2s_parity       = 4'b0   ;

wire           spre_o_set_bus_pr_alarm = 1'b0   ;
wire [7:0]     spre_o_e2s_hmaster      = 8'b0   ;
wire [3:0]     spre_o_e2s_parity       = 4'b0   ;

wire wfi_next    = wcpu_core_sleeping; 
wire halted_next = wcpu_halted; 

wire [31:0] ilm_haddr = wcpu_i_haddr ;

wire                             w_hdram_i_hsel              = wcpu_d_htrans[1];
wire [HDRAM_P_BUS_AW-1:0]        w_hdram_i_haddr             = wcpu_d_haddr[HDRAM_P_BUS_AW-1:0];
wire [1:0]                       w_hdram_i_htrans            = wcpu_d_htrans;
wire                             w_hdram_i_hwrite            = wcpu_d_hwrite;
wire [2:0]                       w_hdram_i_hsize             = wcpu_d_hsize;
wire [2:0]                       w_hdram_i_hburst            = wcpu_d_hburst;
wire [3:0]                       w_hdram_i_hprot             = wcpu_d_hprot;
wire [31:0]                      w_hdram_i_hwdata            = wcpu_d_hwdata;
wire                             w_hdram_i_hmastlock         = 1'h0;


wire [31:0] w_dlm_hrdata = i_dlm_hrdata[31:0] ;
wire [1:0]  w_dlm_hresp  = i_dlm_hresp  ;
wire        w_dlm_hready = i_dlm_hready ;

assign      w_cpu_jtag_DRV_TDO      = wcpu_tdo_en;
assign      w_cpu_jtag_TDO          = wcpu_tdo;
assign      w_cpu_ilm_htrans        = wcpu_i_htrans;
assign      w_cpu_ilm_hwrite        = wcpu_i_hwrite;
assign      w_cpu_ilm_hmastlock     = 1'h0;
assign      w_cpu_ilm_hwdata        = wcpu_i_hwdata;
assign      w_cpu_ilm_hsize         = wcpu_i_hsize;
assign      w_cpu_ilm_hburst        = wcpu_i_hburst;
assign      w_cpu_ilm_hprot         = wcpu_i_hprot;
assign      w_cpu_hsel              = wcpu_m_htrans[1];
assign      w_cpu_htrans            = wcpu_m_htrans;
assign      w_cpu_hwrite            = wcpu_m_hwrite;
assign      w_cpu_haddr             = wcpu_m_haddr;
assign      w_cpu_hsize             = wcpu_m_hsize;
assign      w_cpu_hburst            = wcpu_m_hburst;
assign      w_cpu_hmastlock         = 1'h0;
assign      w_cpu_hwdata            = wcpu_m_hwdata;
assign      w_cpu_hprot             = wcpu_m_hprot;
assign      w_cpu_master            = 2'h0;
assign      w_cpu_core_wfi_mode     = wfi;
assign      w_cpu_core_sleep_value  = 1'h0;
assign      w_cpu_trace_interrupt   = 1'h0;
assign      w_cpu_trace_iexception  = 1'h0;
assign      w_cpu_trace_ivalid      = 1'h0;
assign      w_cpu_sysrstreq         = 1'h0;
assign      w_cpu_hart_halted       = halted;

wire             wcpu_always_on_clk       = i_clk                  ; 
wire             wcpu_clk                 = i_clk                  ;
wire             wcpu_pwrup_rst_n         = i_rst_n                ;
wire             wcpu_ndm_rst_n           = i_dm_ndmreset_n        ;
wire             wcpu_soft_rst_n          = 1'h1                   ;
wire [32-1:0]    wcpu_boot_pc             = w_cpu_reset_vector     ;
wire [32-1:0]    wcpu_hart_id             = 32'h0                  ;
wire             wcpu_ref_clk             = i_mtime_toggle_a       ;
wire [32-1:0]    wcpu_core_mmr_base_addr  = 32'h0000_0000          ;
wire [15:0]      wcpu_reri_bank_inst_id   = 16'h5A5A               ;
wire             wcpu_core_wait           = 1'h0                   ;
wire             wcpu_endianess           = 1'h0                   ;
wire [32+1:0]    wcpu_timer_calibration   = 34'h0                  ;
wire             wcpu_trst_n              = i_trst_n               ;
wire             wcpu_tck                 = i_tck                  ;
wire             wcpu_tms                 = i_tms                  ;
wire             wcpu_tdi                 = i_tdi                  ;
wire             wcpu_ext_interrupt       = 1'h0                   ;
wire [64-1:0]    wcpu_loc_interrupt       = i_clic_irq             ;
wire [31:0]      wcpu_i_hrdata            = i_ilm_hrdata[31:0]     ;
wire             wcpu_i_hready            = i_ilm_hready           ;
wire             wcpu_i_hresp             = i_ilm_hresp[0]         ;
wire [31:0]      wcpu_d_hrdata            = w_dlm_hrdata           ;
wire             wcpu_d_hready            = w_dlm_hready           ;
wire             wcpu_d_hresp             = w_dlm_hresp[0]         ;
wire [31:0]      wcpu_m_hrdata            = i_hrdata[31:0]         ;
wire             wcpu_m_hready            = i_hready               ;
wire             wcpu_m_hresp             = i_hresp[0]             ;

wire             wcpu_dbg_authen          = ~i_dbg_stop            ;  
wire             wcpu_dft_icg_scan_en     = w_dft_icg_scan_en      ;
wire             wcpu_dft_scan_mode       = i_scan_mode            ;
wire             wcpu_dft_scan_rst_n      = i_rst_n                ;

assign o_dm_ndmreset            = wcpu_dm_ndmreset; 

assign o_tdo_oe                 = w_cpu_jtag_DRV_TDO      ;
assign o_tdo                    = w_cpu_jtag_TDO          ;

assign o_ilm_htrans             = {w_cpu_ilm_htrans[1],1'b0};
assign o_ilm_hwrite             = w_cpu_ilm_hwrite        ;
assign o_ilm_hmastlock          = w_cpu_ilm_hmastlock     ;
assign o_ilm_hwdata             = {ipre_o_e2s_parity,w_cpu_ilm_hwdata};
assign o_ilm_haddr              = ilm_haddr               ;
assign o_ilm_hsize              = w_cpu_ilm_hsize         ;
assign o_ilm_hburst             = 3'b0                    ;
assign o_ilm_hprot              = w_cpu_ilm_hprot         ;
assign o_ilm_hmaster            = ipre_o_e2s_hmaster      ;

assign o_dlm_hsel               = w_hdram_i_hsel          ;
assign o_dlm_htrans             = {w_hdram_i_htrans[1],1'b0};
assign o_dlm_hwrite             = w_hdram_i_hwrite        ;
assign o_dlm_haddr              = {4'h2,{(28-HDRAM_P_BUS_AW){1'b0}},w_hdram_i_haddr};
assign o_dlm_hsize              = w_hdram_i_hsize         ;
assign o_dlm_hburst             = w_hdram_i_hburst        ;
assign o_dlm_hmastlock          = w_hdram_i_hmastlock     ;
assign o_dlm_hwdata             = {dpre_o_e2s_parity,w_hdram_i_hwdata};
assign o_dlm_hprot              = w_hdram_i_hprot         ;
assign o_dlm_hmaster            = dpre_o_e2s_hmaster      ;

assign o_hsel                   = w_cpu_hsel              ;
assign o_htrans                 = {w_cpu_htrans[1],1'b0}  ;
assign o_hwrite                 = w_cpu_hwrite            ;
assign o_haddr                  = w_cpu_haddr             ;
assign o_hsize                  = w_cpu_hsize             ;
assign o_hburst                 = 3'b0                    ;
assign o_hmastlock              = w_cpu_hmastlock         ;
assign o_hwdata                 = {spre_o_e2s_parity,w_cpu_hwdata};
assign o_hprot                  = w_cpu_hprot             ;
assign o_master                 = spre_o_e2s_hmaster      ;
assign o_dram_ecc_dec_sec       = hdram_o_ecc_dec_sec     ;
assign o_dram_ecc_dec_ded       = hdram_o_ecc_dec_ded     ;
assign o_dram_ecc_err_addr      = hdram_o_ecc_err_addr    ;

assign o_dram_web               = ~(|hdram_o_ram_wen)     ; 
assign o_dram_wdata             = hdram_o_ram_wdata       ; 

assign o_dram_addr              = hdram_o_ram_addr        ;
assign o_dram_csb               = ~hdram_o_ram_cs         ;
assign o_hart_halted            = w_cpu_hart_halted       ;
assign o_wfi                    = w_cpu_core_wfi_mode     ;
assign o_ipre_set_bus_pr_alarm  = ipre_o_set_bus_pr_alarm ;
assign o_dpre_set_bus_pr_alarm  = dpre_o_set_bus_pr_alarm ;
assign o_spre_set_bus_pr_alarm  = spre_o_set_bus_pr_alarm ;
assign o_mem_init_done          = hdram_o_mem_init_done   ;

osr_wing_m130a_top_wrapper u_wing_m130a_top_wrapper (
    .always_on_clk           ( wcpu_always_on_clk       ), 
    .clk                     ( wcpu_clk                 ), 
    .clk_gate_en             ( wcpu_clk_gate_en         ), 
    .pwrup_rst_n             ( wcpu_pwrup_rst_n         ), 
    .ndm_rst_n               ( wcpu_ndm_rst_n           ), 
    .dm_ndmreset             ( wcpu_dm_ndmreset         ), 
    .soft_rst_n              ( wcpu_soft_rst_n          ), 
    .soft_rst_req            ( wcpu_soft_rst_req        ), 
    .core_sleeping           ( wcpu_core_sleeping       ), 
    .core_deep_sleeping      ( wcpu_core_deep_sleeping  ), 
    .boot_pc                 ( wcpu_boot_pc             ), 
    .hart_id                 ( wcpu_hart_id             ), 
    .ref_clk                 ( wcpu_ref_clk             ), 
    .core_mmr_base_addr      ( wcpu_core_mmr_base_addr  ), 
    .reri_bank_inst_id       ( wcpu_reri_bank_inst_id   ), 
    .core_wait               ( wcpu_core_wait           ), 
    .endianess               ( wcpu_endianess           ), 
    .halted                  ( wcpu_halted              ), 
    .lockup                  ( wcpu_lockup              ), 
    .timer_calibration       ( wcpu_timer_calibration   ), 
    .trst_n                  ( wcpu_trst_n              ), 
    .tck                     ( wcpu_tck                 ), 
    .tms                     ( wcpu_tms                 ), 
    .tdi                     ( wcpu_tdi                 ), 
    .tdo                     ( wcpu_tdo                 ), 
    .tdo_en                  ( wcpu_tdo_en              ), 
    .ext_interrupt           ( wcpu_ext_interrupt       ), 
    .loc_interrupt           ( wcpu_loc_interrupt       ), 
    .i_haddr                 ( wcpu_i_haddr             ), 
    .i_hburst                ( wcpu_i_hburst            ), 
    .i_hprot                 ( wcpu_i_hprot             ), 
    .i_hsize                 ( wcpu_i_hsize             ), 
    .i_htrans                ( wcpu_i_htrans            ), 
    .i_hwrite                ( wcpu_i_hwrite            ), 
    .i_hwdata                ( wcpu_i_hwdata            ), 
    .i_hrdata                ( wcpu_i_hrdata            ), 
    .i_hready                ( wcpu_i_hready            ), 
    .i_hresp                 ( wcpu_i_hresp             ), 
    .i_hmaster               ( wcpu_i_hmaster           ), 
    .d_haddr                 ( wcpu_d_haddr             ), 
    .d_hburst                ( wcpu_d_hburst            ), 
    .d_hprot                 ( wcpu_d_hprot             ), 
    .d_hsize                 ( wcpu_d_hsize             ), 
    .d_htrans                ( wcpu_d_htrans            ), 
    .d_hwrite                ( wcpu_d_hwrite            ), 
    .d_hwdata                ( wcpu_d_hwdata            ), 
    .d_hrdata                ( wcpu_d_hrdata            ), 
    .d_hready                ( wcpu_d_hready            ), 
    .d_hresp                 ( wcpu_d_hresp             ), 
    .d_hmaster               ( wcpu_d_hmaster           ), 
    .m_haddr                 ( wcpu_m_haddr             ), 
    .m_hburst                ( wcpu_m_hburst            ), 
    .m_hprot                 ( wcpu_m_hprot             ), 
    .m_hsize                 ( wcpu_m_hsize             ), 
    .m_htrans                ( wcpu_m_htrans            ), 
    .m_hwrite                ( wcpu_m_hwrite            ), 
    .m_hwdata                ( wcpu_m_hwdata            ), 
    .m_hrdata                ( wcpu_m_hrdata            ), 
    .m_hready                ( wcpu_m_hready            ), 
    .m_hresp                 ( wcpu_m_hresp             ), 
    .m_hmaster               ( wcpu_m_hmaster           ), 
    .dbg_authen              ( wcpu_dbg_authen          ), 
    .err_bus                 ( wcpu_err_bus             ), 
    .dft_icg_scan_en         ( wcpu_dft_icg_scan_en     ), 
    .dft_scan_mode           ( wcpu_dft_scan_mode       ), 
    .dft_scan_rst_n          ( wcpu_dft_scan_rst_n      )  
    ); 

always @(posedge i_clk or negedge i_rst_n) begin
    if(!i_rst_n) begin
        wfi              <= 1'h0                  ; 
        halted           <= 1'h0                  ; 
    end else begin
        wfi              <= wfi_next              ; 
        halted           <= halted_next           ; 
    end
end

endmodule 
