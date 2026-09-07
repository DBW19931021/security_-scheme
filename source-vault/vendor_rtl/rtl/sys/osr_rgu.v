//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_rgu (
    input  wire                 i_clk                   ,
    input  wire                 i_rst_n                 ,
    input  wire                 i_scan_mode             ,

    input  wire                 i_rtc_clk               ,
    input  wire                 i_rtc_rst_n             ,

    input  wire                 i_reset_trng_n          ,

    input  wire                 i_gclk                  ,

    input  wire                 i_cpu_clk               ,
    input  wire                 i_sys_clk               ,
    input  wire                 i_emu_clk               ,
    input  wire                 i_boot_clk              ,

    input  wire                 i_kmu_clk               ,
    input  wire                 i_kmuram_clk            ,

    input  wire                 i_mbox_clk              ,

    input wire                  i_trng_clk              ,

    input wire                  i_hash0_clk              ,
    input wire                  i_hash0_dma_clk          ,

    input wire                  i_ske0_clk               ,
    input wire                  i_ske0_dma_clk           ,

    input wire                  i_pke_clk               ,

    input wire                  i_ahb_dma_clk           ,

    input wire                  i_axi_dma_clk           ,

    input wire                  i_irom_clk              ,

    input  wire                 i_rdc_clk               ,
    input  wire                 i_wdt_clk               ,
    input  wire                 i_uart_clk              ,
    input  wire                 i_tim_clk               ,

    input  wire                 i_crc_clk               ,

    input  wire [31:0]          i_rst_ctrl0             ,
    input  wire [31:0]          i_rst_ctrl1             ,
    input  wire [31:0]          i_alg_rst_ctrl0         ,
    input  wire [31:0]          i_alg_rst_ctrl1         ,
    input  wire [31:0]          i_alg_rst_ctrl2         ,
    input  wire [31:0]          i_alg_rst_ctrl3         ,
    input  wire [31:0]          i_alg_rst_ctrl4         ,
    input  wire [31:0]          i_alg_rst_ctrl5         ,
    input  wire [31:0]          i_alg_rst_ctrl6         ,

    input  wire                 i_boot_hw_ok            ,
    input  wire                 i_emu_resetn_all        ,
    input  wire                 i_emu_resetn_noboot     ,
    input  wire                 i_emu_resetn_cpu        ,

    output wire                 o_srst_n                ,
    output wire                 o_sgrst_n               ,
    output wire                 o_hwrst_n               ,

    output wire                 o_cpu_rst_n             ,

    input  wire                 i_dm_ndmreset           ,
    output wire                 o_dm_ndmreset_n         ,

    output wire                 o_sys_rst_n             ,
    output wire                 o_emu_rst_n             ,
    output wire                 o_boot_rst_n            ,

    output wire                 o_kmu_rst_n             ,
    output wire                 o_kmuram_rst_n          ,

    output wire                 o_mbox_rst_n            ,

    output wire                 o_trng_rst_n            ,

    output wire                 o_hash0_rst_n            ,
    output wire                 o_hash0_dma_rst_n        ,

    output wire                 o_ske0_rst_n             ,
    output wire                 o_ske0_dma_rst_n         ,

    output wire                 o_pke_rst_n             ,

    output wire                 o_ahb_dma_rst_n         ,

    output wire                 o_axi_dma_rst_n         ,

    output wire                 o_irom_rst_n            ,

    output wire                 o_crc_rst_n             ,

    output wire                 o_rdc_rst_n             ,
    output wire                 o_wdt_rst_n             ,
    output wire                 o_uart_rst_n            ,
    output wire                 o_tim_rst_n             ,

    output wire                 o_cpu_boot_rst_n
);

localparam     RST_SYN_SYNC_TIMES = 2 ;

localparam     GRST_SYN_SYNC_TIMES = 2 ;

localparam     ETRALL_P_POLARITY = 1;

localparam     ETRNBOOT_P_POLARITY = 1;

localparam     ETRCPU_P_POLARITY = 1;

localparam     HWRST_SYN_SYNC_TIMES = 2 ;

localparam     CPU_SYN_SYNC_TIMES = 2 ;

localparam     CPU_SYN_D2_SYNC_TIMES = 2 ;

localparam     ROMETRALL_P_POLARITY = 1; 

localparam     ROMETRNBOOT_P_POLARITY = 1; 

localparam     ROMCLKD_SYN_SYNC_TIMES = 2; 

localparam     ROMRSTCTL_SYN_WIDTH = 3; 

localparam     NDMRESET_N_SYN_WIDTH = 1; 

localparam     HARTRESET_N_SYN_WIDTH = 1;

localparam     BOOT_SYN_SYNC_TIMES = 2 ;

localparam     SYS_SYN_SYNC_TIMES = 2 ;

localparam     EMU_SYN_SYNC_TIMES = 2 ;

localparam     KMU_SYN_SYNC_TIMES = 2 ;

localparam     KMURAM_SYN_SYNC_TIMES = 2 ;

localparam     MBOX_SYN_SYNC_TIMES = 2 ;

localparam     TRNG_SYN_SYNC_TIMES = 2 ;

localparam     HASH0_SYN_SYNC_TIMES = 2 ;

localparam     HASH0_DSYN_SYNC_TIMES = 2 ;

localparam     SKE0_SYN_SYNC_TIMES = 2 ;

localparam     SKE0_DSYN_SYNC_TIMES = 2 ; 

localparam     CHACHA_SYN_SYNC_TIMES = 2 ;

localparam     CHACHA_DSYN_SYNC_TIMES = 2 ;

localparam     PKE_SYN_SYNC_TIMES = 2 ;

localparam     PQC_SPXP_SYN_SYNC_TIMES = 2 ;

localparam     PQC_SPXP_DSYN_SYNC_TIMES = 2 ;

localparam     PQC_KD_SYN_SYNC_TIMES = 2 ;

localparam     AHB_SYN_SYNC_TIMES = 2 ;

localparam     AXI_SYN_SYNC_TIMES = 2 ;

localparam     IROM_SYN_SYNC_TIMES = 2 ;

localparam     AXI_64TO128_SYN_SYNC_TIMES = 2 ; 

localparam     WDT_SYN_SYNC_TIMES = 2 ;

localparam     WDT1_SYN_SYNC_TIMES = 2 ; 

localparam     UART_SYN_SYNC_TIMES = 2 ;

localparam     CRC_SYN_SYNC_TIMES = 2 ;

localparam     APB_NVM_CIPHER_SYN_SYNC_TIMES = 2; 

localparam     TIM_SYN_SYNC_TIMES = 2 ;

localparam     UTIM_PSYN_SYNC_TIMES = 2 ;

localparam     MCNT_PSYN_SYNC_TIMES = 2 ;

localparam     UTIM_RSYN_SYNC_TIMES = 2 ;  

localparam     MCNT_RSYN_SYNC_TIMES = 2 ;

localparam     CLKD_RSYN_SYNC_TIMES = 2;

localparam     CPU_BOOT_SYN_SYNC_TIMES = 2;

localparam     PKE_BOOT_SYN_SYNC_TIMES = 2;


reg             r_dm_ndmreset_n ;

wire        rst_syn_o_rst_n      ;

wire        grst_syn_o_rst_n      ;




wire        hwrst_syn_o_rst_n      ;

wire        cpu_syn_o_rst_n      ;








wire        boot_syn_o_rst_n      ;

wire        sys_syn_o_rst_n      ;

wire        emu_syn_o_rst_n      ;

wire        kmu_syn_o_rst_n      ;

wire        kmuram_syn_o_rst_n      ;

wire        mbox_syn_o_rst_n      ;

wire        trng_syn_o_rst_n      ;

wire        hash0_syn_o_rst_n      ;

wire        hash0_dsyn_o_rst_n      ;

wire        ske0_syn_o_rst_n      ;

wire        ske0_dsyn_o_rst_n      ;



wire        pke_syn_o_rst_n      ;




wire        ahb_syn_o_rst_n      ;

wire        axi_syn_o_rst_n      ;

wire        irom_syn_o_rst_n      ;


wire        wdt_syn_o_rst_n      ;


wire        uart_syn_o_rst_n      ;

wire        crc_syn_o_rst_n      ;


wire        tim_syn_o_rst_n      ;






wire        cpu_boot_syn_o_rst_n      ;


wire    cpu_rst = i_rst_n;

wire soc_nbt_rst_n = 1'b1           ;

wire    hsm_rbt_n = i_rst_ctrl0[31] & i_emu_resetn_all; 
wire    hsm_rst_n = hsm_rbt_n & i_rst_ctrl0[30] & i_emu_resetn_noboot & soc_nbt_rst_n; 
wire    cpu_rst_n = hsm_rst_n & i_rst_ctrl0[29]  & i_boot_hw_ok & i_emu_resetn_cpu; 

wire    emu_soft_rst_n = i_rst_ctrl0[12];

wire    r_dm_ndmreset_n_next = ~i_dm_ndmreset    ;

wire        rst_syn_i_scan_rst_n = i_rst_n            ;
wire        rst_syn_i_scan_mode  = i_scan_mode        ;
wire        rst_syn_i_clk        = i_clk              ;
wire        rst_syn_i_rst_n      = i_rst_n            ;
wire        rst_syn_i_soft_rst_n = 1'h1               ;

wire        grst_syn_i_scan_rst_n = i_rst_n            ;
wire        grst_syn_i_scan_mode  = i_scan_mode        ;
wire        grst_syn_i_clk        = i_gclk             ;
wire        grst_syn_i_rst_n      = i_rst_n            ;
wire        grst_syn_i_soft_rst_n = 1'h1               ;

wire        hwrst_syn_i_scan_rst_n = i_rst_n            ;
wire        hwrst_syn_i_scan_mode  = i_scan_mode        ;
wire        hwrst_syn_i_clk        = i_gclk             ;
wire        hwrst_syn_i_rst_n      = i_rst_n            ;
wire        hwrst_syn_i_soft_rst_n = i_boot_hw_ok       ;

wire        cpu_syn_i_scan_rst_n = i_rst_n            ;
wire        cpu_syn_i_scan_mode  = i_scan_mode        ;
wire        cpu_syn_i_clk        = i_cpu_clk          ;
wire        cpu_syn_i_rst_n      = cpu_rst            ;
wire        cpu_syn_i_soft_rst_n = cpu_rst_n          ;

wire        boot_syn_i_scan_rst_n = i_rst_n            ;
wire        boot_syn_i_scan_mode  = i_scan_mode        ;
wire        boot_syn_i_clk        = i_boot_clk         ;
wire        boot_syn_i_rst_n      = i_rst_n            ;
wire        boot_syn_i_soft_rst_n = i_rst_ctrl0[8] & hsm_rbt_n ;

wire        sys_syn_i_scan_rst_n = i_rst_n            ;
wire        sys_syn_i_scan_mode  = i_scan_mode        ;
wire        sys_syn_i_clk        = i_sys_clk          ;
wire        sys_syn_i_rst_n      = i_rst_n            ;
wire        sys_syn_i_soft_rst_n = i_rst_ctrl0[9] & hsm_rbt_n ;

wire        emu_syn_i_scan_rst_n = i_rst_n            ;
wire        emu_syn_i_scan_mode  = i_scan_mode        ;
wire        emu_syn_i_clk        = i_emu_clk          ;
wire        emu_syn_i_rst_n      = i_rst_n            ;
wire        emu_syn_i_soft_rst_n = emu_soft_rst_n     ;

wire        kmu_syn_i_scan_rst_n = i_rst_n            ;
wire        kmu_syn_i_scan_mode  = i_scan_mode        ;
wire        kmu_syn_i_clk        = i_kmu_clk          ;
wire        kmu_syn_i_rst_n      = i_rst_n            ;
wire        kmu_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl0[16] & hsm_rst_n ;

wire        kmuram_syn_i_scan_rst_n = i_rst_n            ;
wire        kmuram_syn_i_scan_mode  = i_scan_mode        ;
wire        kmuram_syn_i_clk        = i_kmuram_clk       ;
wire        kmuram_syn_i_rst_n      = i_rst_n            ;
wire        kmuram_syn_i_soft_rst_n = i_boot_hw_ok ? (i_rst_ctrl0[16] & hsm_rst_n) : boot_syn_i_soft_rst_n ;

wire        mbox_syn_i_scan_rst_n = i_rst_n            ;
wire        mbox_syn_i_scan_mode  = i_scan_mode        ;
wire        mbox_syn_i_clk        = i_mbox_clk         ;
wire        mbox_syn_i_rst_n      = i_rst_n   ;
wire        mbox_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl0[20] & hsm_rst_n     ;

wire        trng_syn_i_scan_rst_n = i_rst_n            ;
wire        trng_syn_i_scan_mode  = i_scan_mode        ;
wire        trng_syn_i_clk        = i_trng_clk         ;
wire        trng_syn_i_rst_n      = i_rst_n            ;
wire        trng_syn_i_soft_rst_n = i_reset_trng_n & i_alg_rst_ctrl3[8] & hsm_rst_n    ;

wire        hash0_syn_i_scan_rst_n = i_rst_n            ;
wire        hash0_syn_i_scan_mode  = i_scan_mode        ;
wire        hash0_syn_i_clk        = i_hash0_clk        ;
wire        hash0_syn_i_rst_n      = i_rst_n            ;
wire        hash0_syn_i_soft_rst_n = i_boot_hw_ok & i_alg_rst_ctrl0[8] & hsm_rst_n    ;

wire        hash0_dsyn_i_scan_rst_n = i_rst_n            ;
wire        hash0_dsyn_i_scan_mode  = i_scan_mode        ;
wire        hash0_dsyn_i_clk        = i_hash0_dma_clk    ;
wire        hash0_dsyn_i_rst_n      = i_rst_n            ;
wire        hash0_dsyn_i_soft_rst_n = i_boot_hw_ok & i_alg_rst_ctrl0[9] & hsm_rst_n   ;

wire        ske0_syn_i_scan_rst_n = i_rst_n            ;
wire        ske0_syn_i_scan_mode  = i_scan_mode        ;
wire        ske0_syn_i_clk        = i_ske0_clk         ;
wire        ske0_syn_i_rst_n      = i_rst_n            ;
wire        ske0_syn_i_soft_rst_n = i_boot_hw_ok & i_alg_rst_ctrl1[8] & hsm_rst_n     ;

wire        ske0_dsyn_i_scan_rst_n = i_rst_n            ;
wire        ske0_dsyn_i_scan_mode  = i_scan_mode        ;
wire        ske0_dsyn_i_clk        = i_ske0_dma_clk     ;
wire        ske0_dsyn_i_rst_n      = i_rst_n            ;
wire        ske0_dsyn_i_soft_rst_n = i_boot_hw_ok & i_alg_rst_ctrl1[9] & hsm_rst_n    ;

wire        pke_syn_i_scan_rst_n = i_rst_n            ;
wire        pke_syn_i_scan_mode  = i_scan_mode        ;
wire        pke_syn_i_clk        = i_pke_clk          ;
wire        pke_syn_i_rst_n      = i_rst_n            ;
wire        pke_syn_i_soft_rst_n = i_boot_hw_ok & i_alg_rst_ctrl2[8] & hsm_rst_n    ;

wire        ahb_syn_i_scan_rst_n = i_rst_n            ;
wire        ahb_syn_i_scan_mode  = i_scan_mode        ;
wire        ahb_syn_i_clk        = i_ahb_dma_clk      ;
wire        ahb_syn_i_rst_n      = i_rst_n            ;
wire        ahb_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl0[24] & hsm_rst_n    ;

wire        axi_syn_i_scan_rst_n = i_rst_n            ;
wire        axi_syn_i_scan_mode  = i_scan_mode        ;
wire        axi_syn_i_clk        = i_axi_dma_clk      ;
wire        axi_syn_i_rst_n      = i_rst_n            ;
wire        axi_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl0[26] & hsm_rst_n    ;

wire        irom_syn_i_scan_rst_n = i_rst_n           ;
wire        irom_syn_i_scan_mode  = i_scan_mode       ; 
wire        irom_syn_i_clk        = i_irom_clk        ;
wire        irom_syn_i_rst_n      = i_rst_n           ;
wire        irom_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl0[15] & hsm_rst_n    ;

wire        wdt_syn_i_scan_rst_n = i_rst_n            ;
wire        wdt_syn_i_scan_mode  = i_scan_mode        ;
wire        wdt_syn_i_clk        = i_wdt_clk          ;
wire        wdt_syn_i_rst_n      = i_rst_n            ;
wire        wdt_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl1[8] & hsm_rst_n     ;

wire        uart_syn_i_scan_rst_n = i_rst_n            ;
wire        uart_syn_i_scan_mode  = i_scan_mode        ;
wire        uart_syn_i_clk        = i_uart_clk         ;
wire        uart_syn_i_rst_n      = i_rst_n            ;
wire        uart_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl1[12] & hsm_rst_n     ;

wire        crc_syn_i_scan_rst_n = i_rst_n            ;
wire        crc_syn_i_scan_mode  = i_scan_mode        ;
wire        crc_syn_i_clk        = i_crc_clk          ;
wire        crc_syn_i_rst_n      = i_rst_n            ;
wire        crc_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl1[13] & hsm_rst_n     ;

wire        tim_syn_i_scan_rst_n = i_rst_n            ;
wire        tim_syn_i_scan_mode  = i_scan_mode        ;
wire        tim_syn_i_clk        = i_tim_clk          ;
wire        tim_syn_i_rst_n      = i_rst_n            ;
wire        tim_syn_i_soft_rst_n = i_boot_hw_ok & i_rst_ctrl1[16] & hsm_rst_n     ;

wire        cpu_boot_syn_i_scan_rst_n = i_rst_n         ;
wire        cpu_boot_syn_i_scan_mode  = i_scan_mode     ;
wire        cpu_boot_syn_i_clk        = i_cpu_clk       ;
wire        cpu_boot_syn_i_rst_n      = i_rst_n         ;
wire        cpu_boot_syn_i_soft_rst_n = boot_syn_o_rst_n;

wire o_cpu_rst_n_w = cpu_syn_o_rst_n    ;

assign o_srst_n                 = rst_syn_o_rst_n       ;
assign o_sgrst_n                = grst_syn_o_rst_n      ;
assign o_hwrst_n                = hwrst_syn_o_rst_n     ;
assign o_cpu_rst_n              = o_cpu_rst_n_w         ;

assign o_dm_ndmreset_n          = r_dm_ndmreset_n       ; 

assign o_sys_rst_n              = sys_syn_o_rst_n       ;
assign o_emu_rst_n              = emu_syn_o_rst_n       ;
assign o_boot_rst_n             = boot_syn_o_rst_n      ;

assign o_kmu_rst_n              = kmu_syn_o_rst_n       ;

assign o_kmuram_rst_n           = kmuram_syn_o_rst_n    ;

assign o_mbox_rst_n             = mbox_syn_o_rst_n      ;

assign o_trng_rst_n             = trng_syn_o_rst_n      ; 

assign o_hash0_rst_n            = hash0_syn_o_rst_n      ;
assign o_hash0_dma_rst_n        = hash0_dsyn_o_rst_n     ;

assign o_ske0_rst_n             = ske0_syn_o_rst_n       ;
assign o_ske0_dma_rst_n         = ske0_dsyn_o_rst_n      ;

assign o_pke_rst_n              = pke_syn_o_rst_n       ; 

assign o_ahb_dma_rst_n          = ahb_syn_o_rst_n       ; 

assign o_axi_dma_rst_n          = axi_syn_o_rst_n       ; 

assign o_irom_rst_n             = irom_syn_o_rst_n      ;

assign o_crc_rst_n              = crc_syn_o_rst_n       ;

assign o_rdc_rst_n              = rst_syn_o_rst_n       ;
assign o_wdt_rst_n              = wdt_syn_o_rst_n       ;

assign o_uart_rst_n             = uart_syn_o_rst_n      ;
assign o_tim_rst_n              = tim_syn_o_rst_n       ;

assign o_cpu_boot_rst_n         = cpu_boot_syn_o_rst_n  ;

osr_sync_rst #(
    .SYNC_TIMES     ( RST_SYN_SYNC_TIMES )  
    ) u_rst_syn (
    .i_scan_rst_n        ( rst_syn_i_scan_rst_n ), 
    .i_scan_mode         ( rst_syn_i_scan_mode  ), 
    .i_clk               ( rst_syn_i_clk        ), 
    .i_rst_n             ( rst_syn_i_rst_n      ), 
    .i_soft_rst_n        ( rst_syn_i_soft_rst_n ), 
    .o_rst_n             ( rst_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( GRST_SYN_SYNC_TIMES )  
    ) u_grst_syn (
    .i_scan_rst_n         ( grst_syn_i_scan_rst_n ), 
    .i_scan_mode          ( grst_syn_i_scan_mode  ), 
    .i_clk                ( grst_syn_i_clk        ), 
    .i_rst_n              ( grst_syn_i_rst_n      ), 
    .i_soft_rst_n         ( grst_syn_i_soft_rst_n ), 
    .o_rst_n              ( grst_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( HWRST_SYN_SYNC_TIMES )  
    ) u_hwrst_syn (
    .i_scan_rst_n          ( hwrst_syn_i_scan_rst_n ), 
    .i_scan_mode           ( hwrst_syn_i_scan_mode  ), 
    .i_clk                 ( hwrst_syn_i_clk        ), 
    .i_rst_n               ( hwrst_syn_i_rst_n      ), 
    .i_soft_rst_n          ( hwrst_syn_i_soft_rst_n ), 
    .o_rst_n               ( hwrst_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( CPU_SYN_SYNC_TIMES )  
    ) u_cpu_syn (
    .i_scan_rst_n        ( cpu_syn_i_scan_rst_n ), 
    .i_scan_mode         ( cpu_syn_i_scan_mode  ), 
    .i_clk               ( cpu_syn_i_clk        ), 
    .i_rst_n             ( cpu_syn_i_rst_n      ), 
    .i_soft_rst_n        ( cpu_syn_i_soft_rst_n ), 
    .o_rst_n             ( cpu_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( BOOT_SYN_SYNC_TIMES )  
    ) u_boot_syn (
    .i_scan_rst_n         ( boot_syn_i_scan_rst_n ), 
    .i_scan_mode          ( boot_syn_i_scan_mode  ), 
    .i_clk                ( boot_syn_i_clk        ), 
    .i_rst_n              ( boot_syn_i_rst_n      ), 
    .i_soft_rst_n         ( boot_syn_i_soft_rst_n ), 
    .o_rst_n              ( boot_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( SYS_SYN_SYNC_TIMES )  
    ) u_sys_syn (
    .i_scan_rst_n        ( sys_syn_i_scan_rst_n ), 
    .i_scan_mode         ( sys_syn_i_scan_mode  ), 
    .i_clk               ( sys_syn_i_clk        ), 
    .i_rst_n             ( sys_syn_i_rst_n      ), 
    .i_soft_rst_n        ( sys_syn_i_soft_rst_n ), 
    .o_rst_n             ( sys_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( EMU_SYN_SYNC_TIMES )  
    ) u_emu_syn (
    .i_scan_rst_n        ( emu_syn_i_scan_rst_n ), 
    .i_scan_mode         ( emu_syn_i_scan_mode  ), 
    .i_clk               ( emu_syn_i_clk        ), 
    .i_rst_n             ( emu_syn_i_rst_n      ), 
    .i_soft_rst_n        ( emu_syn_i_soft_rst_n ), 
    .o_rst_n             ( emu_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( KMU_SYN_SYNC_TIMES )  
    ) u_kmu_syn (
    .i_scan_rst_n        ( kmu_syn_i_scan_rst_n ), 
    .i_scan_mode         ( kmu_syn_i_scan_mode  ), 
    .i_clk               ( kmu_syn_i_clk        ), 
    .i_rst_n             ( kmu_syn_i_rst_n      ), 
    .i_soft_rst_n        ( kmu_syn_i_soft_rst_n ), 
    .o_rst_n             ( kmu_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( KMURAM_SYN_SYNC_TIMES )  
    ) u_kmuram_syn (
    .i_scan_rst_n           ( kmuram_syn_i_scan_rst_n ), 
    .i_scan_mode            ( kmuram_syn_i_scan_mode  ), 
    .i_clk                  ( kmuram_syn_i_clk        ), 
    .i_rst_n                ( kmuram_syn_i_rst_n      ), 
    .i_soft_rst_n           ( kmuram_syn_i_soft_rst_n ), 
    .o_rst_n                ( kmuram_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( MBOX_SYN_SYNC_TIMES )  
    ) u_mbox_syn (
    .i_scan_rst_n         ( mbox_syn_i_scan_rst_n ), 
    .i_scan_mode          ( mbox_syn_i_scan_mode  ), 
    .i_clk                ( mbox_syn_i_clk        ), 
    .i_rst_n              ( mbox_syn_i_rst_n      ), 
    .i_soft_rst_n         ( mbox_syn_i_soft_rst_n ), 
    .o_rst_n              ( mbox_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( TRNG_SYN_SYNC_TIMES )  
    ) u_trng_syn (
    .i_scan_rst_n         ( trng_syn_i_scan_rst_n ), 
    .i_scan_mode          ( trng_syn_i_scan_mode  ), 
    .i_clk                ( trng_syn_i_clk        ), 
    .i_rst_n              ( trng_syn_i_rst_n      ), 
    .i_soft_rst_n         ( trng_syn_i_soft_rst_n ), 
    .o_rst_n              ( trng_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( HASH0_SYN_SYNC_TIMES )  
    ) u_hash0_syn (
    .i_scan_rst_n          ( hash0_syn_i_scan_rst_n ), 
    .i_scan_mode           ( hash0_syn_i_scan_mode  ), 
    .i_clk                 ( hash0_syn_i_clk        ), 
    .i_rst_n               ( hash0_syn_i_rst_n      ), 
    .i_soft_rst_n          ( hash0_syn_i_soft_rst_n ), 
    .o_rst_n               ( hash0_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( HASH0_DSYN_SYNC_TIMES )  
    ) u_hash0_dsyn (
    .i_scan_rst_n           ( hash0_dsyn_i_scan_rst_n ), 
    .i_scan_mode            ( hash0_dsyn_i_scan_mode  ), 
    .i_clk                  ( hash0_dsyn_i_clk        ), 
    .i_rst_n                ( hash0_dsyn_i_rst_n      ), 
    .i_soft_rst_n           ( hash0_dsyn_i_soft_rst_n ), 
    .o_rst_n                ( hash0_dsyn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( SKE0_SYN_SYNC_TIMES )  
    ) u_ske0_syn (
    .i_scan_rst_n         ( ske0_syn_i_scan_rst_n ), 
    .i_scan_mode          ( ske0_syn_i_scan_mode  ), 
    .i_clk                ( ske0_syn_i_clk        ), 
    .i_rst_n              ( ske0_syn_i_rst_n      ), 
    .i_soft_rst_n         ( ske0_syn_i_soft_rst_n ), 
    .o_rst_n              ( ske0_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( SKE0_DSYN_SYNC_TIMES )  
    ) u_ske0_dsyn (
    .i_scan_rst_n          ( ske0_dsyn_i_scan_rst_n ), 
    .i_scan_mode           ( ske0_dsyn_i_scan_mode  ), 
    .i_clk                 ( ske0_dsyn_i_clk        ), 
    .i_rst_n               ( ske0_dsyn_i_rst_n      ), 
    .i_soft_rst_n          ( ske0_dsyn_i_soft_rst_n ), 
    .o_rst_n               ( ske0_dsyn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( PKE_SYN_SYNC_TIMES )  
    ) u_pke_syn (
    .i_scan_rst_n        ( pke_syn_i_scan_rst_n ), 
    .i_scan_mode         ( pke_syn_i_scan_mode  ), 
    .i_clk               ( pke_syn_i_clk        ), 
    .i_rst_n             ( pke_syn_i_rst_n      ), 
    .i_soft_rst_n        ( pke_syn_i_soft_rst_n ), 
    .o_rst_n             ( pke_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( AHB_SYN_SYNC_TIMES )  
    ) u_ahb_syn (
    .i_scan_rst_n        ( ahb_syn_i_scan_rst_n ), 
    .i_scan_mode         ( ahb_syn_i_scan_mode  ), 
    .i_clk               ( ahb_syn_i_clk        ), 
    .i_rst_n             ( ahb_syn_i_rst_n      ), 
    .i_soft_rst_n        ( ahb_syn_i_soft_rst_n ), 
    .o_rst_n             ( ahb_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( AXI_SYN_SYNC_TIMES )  
    ) u_axi_syn (
    .i_scan_rst_n        ( axi_syn_i_scan_rst_n ), 
    .i_scan_mode         ( axi_syn_i_scan_mode  ), 
    .i_clk               ( axi_syn_i_clk        ), 
    .i_rst_n             ( axi_syn_i_rst_n      ), 
    .i_soft_rst_n        ( axi_syn_i_soft_rst_n ), 
    .o_rst_n             ( axi_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( IROM_SYN_SYNC_TIMES )  
    ) u_irom_syn (
    .i_scan_rst_n         ( irom_syn_i_scan_rst_n ), 
    .i_scan_mode          ( irom_syn_i_scan_mode  ), 
    .i_clk                ( irom_syn_i_clk        ), 
    .i_rst_n              ( irom_syn_i_rst_n      ), 
    .i_soft_rst_n         ( irom_syn_i_soft_rst_n ), 
    .o_rst_n              ( irom_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( WDT_SYN_SYNC_TIMES )  
    ) u_wdt_syn (
    .i_scan_rst_n        ( wdt_syn_i_scan_rst_n ), 
    .i_scan_mode         ( wdt_syn_i_scan_mode  ), 
    .i_clk               ( wdt_syn_i_clk        ), 
    .i_rst_n             ( wdt_syn_i_rst_n      ), 
    .i_soft_rst_n        ( wdt_syn_i_soft_rst_n ), 
    .o_rst_n             ( wdt_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( UART_SYN_SYNC_TIMES )  
    ) u_uart_syn (
    .i_scan_rst_n         ( uart_syn_i_scan_rst_n ), 
    .i_scan_mode          ( uart_syn_i_scan_mode  ), 
    .i_clk                ( uart_syn_i_clk        ), 
    .i_rst_n              ( uart_syn_i_rst_n      ), 
    .i_soft_rst_n         ( uart_syn_i_soft_rst_n ), 
    .o_rst_n              ( uart_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( CRC_SYN_SYNC_TIMES )  
    ) u_crc_syn (
    .i_scan_rst_n        ( crc_syn_i_scan_rst_n ), 
    .i_scan_mode         ( crc_syn_i_scan_mode  ), 
    .i_clk               ( crc_syn_i_clk        ), 
    .i_rst_n             ( crc_syn_i_rst_n      ), 
    .i_soft_rst_n        ( crc_syn_i_soft_rst_n ), 
    .o_rst_n             ( crc_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( TIM_SYN_SYNC_TIMES )  
    ) u_tim_syn (
    .i_scan_rst_n        ( tim_syn_i_scan_rst_n ), 
    .i_scan_mode         ( tim_syn_i_scan_mode  ), 
    .i_clk               ( tim_syn_i_clk        ), 
    .i_rst_n             ( tim_syn_i_rst_n      ), 
    .i_soft_rst_n        ( tim_syn_i_soft_rst_n ), 
    .o_rst_n             ( tim_syn_o_rst_n      )  
    ); 

osr_sync_rst #(
    .SYNC_TIMES     ( CPU_BOOT_SYN_SYNC_TIMES )  
    ) u_cpu_boot_syn (
    .i_scan_rst_n             ( cpu_boot_syn_i_scan_rst_n ), 
    .i_scan_mode              ( cpu_boot_syn_i_scan_mode  ), 
    .i_clk                    ( cpu_boot_syn_i_clk        ), 
    .i_rst_n                  ( cpu_boot_syn_i_rst_n      ), 
    .i_soft_rst_n             ( cpu_boot_syn_i_soft_rst_n ), 
    .o_rst_n                  ( cpu_boot_syn_o_rst_n      )  
    ); 

always @(posedge cpu_syn_i_clk or negedge cpu_syn_o_rst_n) begin
    if(!cpu_syn_o_rst_n) begin
        r_dm_ndmreset_n <= 1'h0                 ; 
    end else begin
        r_dm_ndmreset_n <= r_dm_ndmreset_n_next ; 
    end
end

endmodule
