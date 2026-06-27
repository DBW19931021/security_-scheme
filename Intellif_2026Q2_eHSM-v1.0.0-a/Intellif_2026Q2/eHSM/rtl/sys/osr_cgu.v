//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_cgu (
    input  wire                 i_rtc_clk               ,
    input  wire                 i_rtc_rst_n             ,

    input  wire                 i_clk                   ,
    input  wire                 i_rst_n                 ,
    input  wire                 i_scan_mode             ,
    input   wire                i_scan_enable           ,

    input  wire                 i_boot_hw_ok            ,

    input  wire [31:0]          i_clk_ctrl0             ,
    input  wire [31:0]          i_clk_ctrl1             ,
    input  wire [31:0]          i_alg_clk_ctrl0         ,
    input  wire [31:0]          i_alg_clk_ctrl1         ,
    input  wire [31:0]          i_alg_clk_ctrl2         ,
    input  wire [31:0]          i_alg_clk_ctrl3         ,
    input  wire [31:0]          i_alg_clk_ctrl4         ,
    input  wire [31:0]          i_alg_clk_ctrl5         ,
    input  wire [31:0]          i_alg_clk_ctrl6         ,

    input  wire                 i_sgrst_n               ,

    output wire                 o_sclk                  ,
    output wire                 o_gclk                  ,

    output wire                 o_dbg_toggle_a          , 
    output wire                 o_mtime_toggle_a        ,

    output wire                 o_cpu_clk               ,
    output wire                 o_sys_clk               ,
    output wire                 o_emu_clk               ,
    output wire                 o_boot_clk              ,

    output wire                 o_kmu_clk               ,
    output wire                 o_kmuram_clk            ,

    output wire                 o_mbox_clk              ,

    output wire                 o_trng_clk              ,

    output wire                 o_hash0_clk              ,
    output wire                 o_hash0_dma_clk          ,

    output wire                 o_ske0_clk               ,
    output wire                 o_ske0_dma_clk           ,

    output wire                 o_pke_clk               ,

    output wire                 o_ahb_dma_clk           ,

    output wire                 o_axi_dma_clk           ,

    output wire                 o_irom_clk              ,

    output wire                 o_crc_clk               ,

    output wire                 o_rdc_clk               ,
    output wire                 o_wdt_clk               ,
    output wire                 o_uart_clk              ,
    output wire                 o_tim_clk
);

localparam     UTIM_SYN_WIDTH = 1;

localparam     MCNT_SYN_WIDTH = 1;

wire w_icg_se = i_scan_enable   ;

wire             scan_rtc_clk   ;
wire             scan_rtc_rst_n ;

reg [3:0]       cnt        ;
reg             low_clk    ;


wire        irdc_GCK = i_clk;

wire        iboot_GCK ;

wire        iemu_GCK ;

wire        ikmu_GCK ;

wire        ikmuram_GCK ;

wire        imbox_GCK ;

wire        itrng_GCK ;

wire        ihash0_GCK ;

wire        ihash0_dma_GCK ;

wire        iske0_GCK ;

wire        iske0_dma_GCK ;



wire        ipke_GCK ;




wire        iahb_dma_GCK ;

wire        iaxi_dma_GCK ;

wire        irom_GCK ;

wire        iwdt_GCK ;


wire        iuart_GCK ;

wire        icrc_GCK ;


wire        itim_GCK ;







wire        iclkd_GCK = 1'b0;


wire [3:0]  cnt_next        = (cnt + 4'h1); 
wire        low_clk_next    = (cnt == 4'hf) ? ~low_clk : low_clk; 

osr_mux2 scan_rtc_clk_mux(.A(i_rtc_clk ),     .B(i_clk),     .S(i_scan_mode), .Z(scan_rtc_clk));
osr_mux2 scan_rtc_rstn_mux(.A(i_rtc_rst_n),   .B(i_rst_n),   .S(i_scan_mode), .Z(scan_rtc_rst_n));

wire        iboot_CK  = irdc_GCK        ;
wire        iboot_E   = i_clk_ctrl0[8]  ;
wire        iboot_SE  = w_icg_se        ;

wire        iemu_CK  = irdc_GCK         ;
wire        iemu_E   = i_clk_ctrl0[12]  ;
wire        iemu_SE  = w_icg_se         ;

wire        ikmu_CK  = irdc_GCK         ;
wire        ikmu_E   = i_clk_ctrl0[16]  ;
wire        ikmu_SE  = w_icg_se         ;

wire        ikmuram_CK  = irdc_GCK      ;
wire        ikmuram_E   = i_boot_hw_ok  ? i_clk_ctrl0[16] : i_clk_ctrl0[8] ;
wire        ikmuram_SE  = w_icg_se      ;

wire        imbox_CK  = i_clk           ;
wire        imbox_E   = i_clk_ctrl0[20] ;
wire        imbox_SE  = w_icg_se        ;

wire        itrng_CK  = irdc_GCK        ;
wire        itrng_E   = i_alg_clk_ctrl3[8];
wire        itrng_SE  = w_icg_se        ;

wire        ihash0_CK  =  irdc_GCK        ;
wire        ihash0_E   =  i_alg_clk_ctrl0[8] ;
wire        ihash0_SE  =  w_icg_se        ;

wire        ihash0_dma_CK  =  irdc_GCK        ;
wire        ihash0_dma_E   =  i_alg_clk_ctrl0[9] ;
wire        ihash0_dma_SE  =  w_icg_se        ;

wire        iske0_CK  =  irdc_GCK         ;
wire        iske0_E   =  i_alg_clk_ctrl1[8]  ;
wire        iske0_SE  =  w_icg_se         ;

wire        iske0_dma_CK  =  irdc_GCK         ;
wire        iske0_dma_E   =  i_alg_clk_ctrl1[9]  ;
wire        iske0_dma_SE  =  w_icg_se         ;

wire        ipke_CK  = irdc_GCK         ;
wire        ipke_E   = i_alg_clk_ctrl2[8]  ;
wire        ipke_SE  = w_icg_se         ;

wire        iahb_dma_CK  = irdc_GCK         ;
wire        iahb_dma_E   = i_clk_ctrl0[24]  ;
wire        iahb_dma_SE  = w_icg_se         ;

wire        iaxi_dma_CK  = irdc_GCK         ;
wire        iaxi_dma_E   = i_clk_ctrl0[26]  ;
wire        iaxi_dma_SE  = w_icg_se         ;

wire        irom_CK  = irdc_GCK         ;
wire        irom_E   = i_clk_ctrl0[15]  ;
wire        irom_SE  = w_icg_se         ;

wire        iwdt_CK  = i_clk            ;
wire        iwdt_E   = i_clk_ctrl1[8]   ;
wire        iwdt_SE  = w_icg_se         ;

wire        iuart_CK  = i_clk            ;
wire        iuart_E   = i_clk_ctrl1[12]  ;
wire        iuart_SE  = w_icg_se         ;

wire        icrc_CK  = i_clk            ;
wire        icrc_E   = i_clk_ctrl1[13]  ;
wire        icrc_SE  = w_icg_se         ;

wire        itim_CK  = i_clk            ;
wire        itim_E   = i_clk_ctrl1[16]  ;
wire        itim_SE  = w_icg_se         ;


wire cpu_clk_ctl = irdc_GCK          ;

assign o_sclk               = i_clk         ;
assign o_gclk               = irdc_GCK      ;

assign o_dbg_toggle_a       = 1'h0          ;
assign o_mtime_toggle_a     = low_clk       ;
assign o_cpu_clk            = cpu_clk_ctl   ;
assign o_sys_clk            = irdc_GCK      ;
assign o_emu_clk            = iemu_GCK      ;
assign o_boot_clk           = iboot_GCK     ;
assign o_kmu_clk            = ikmu_GCK      ;

assign o_kmuram_clk         = ikmuram_GCK   ;

assign o_mbox_clk           = imbox_GCK     ;

assign o_trng_clk           = itrng_GCK     ; 

assign o_hash0_clk          = ihash0_GCK     ;
assign o_hash0_dma_clk      = ihash0_dma_GCK ;

assign o_ske0_clk           = iske0_GCK      ;
assign o_ske0_dma_clk       = iske0_dma_GCK  ;

assign o_pke_clk            = ipke_GCK      ;

assign o_ahb_dma_clk        = iahb_dma_GCK  ;

assign o_axi_dma_clk        = iaxi_dma_GCK  ;

assign o_irom_clk           = irom_GCK      ;

assign o_crc_clk            = icrc_GCK      ;

assign o_rdc_clk            = i_clk         ;
assign o_wdt_clk            = iwdt_GCK      ;

assign o_uart_clk           = iuart_GCK     ;
assign o_tim_clk            = itim_GCK      ;

osr_icg OSR_DONTTOUCH_u_icg_boot (
    .CK       ( iboot_CK  ), 
    .E        ( iboot_E   ), 
    .SE       ( iboot_SE  ), 
    .GCK      ( iboot_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_emu (
    .CK      ( iemu_CK  ), 
    .E       ( iemu_E   ), 
    .SE      ( iemu_SE  ), 
    .GCK     ( iemu_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_kmu (
    .CK      ( ikmu_CK  ), 
    .E       ( ikmu_E   ), 
    .SE      ( ikmu_SE  ), 
    .GCK     ( ikmu_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_kmuram (
    .CK         ( ikmuram_CK  ), 
    .E          ( ikmuram_E   ), 
    .SE         ( ikmuram_SE  ), 
    .GCK        ( ikmuram_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_mbox (
    .CK       ( imbox_CK  ), 
    .E        ( imbox_E   ), 
    .SE       ( imbox_SE  ), 
    .GCK      ( imbox_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_trng (
    .CK       ( itrng_CK  ), 
    .E        ( itrng_E   ), 
    .SE       ( itrng_SE  ), 
    .GCK      ( itrng_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_hash0 (
    .CK        ( ihash0_CK  ), 
    .E         ( ihash0_E   ), 
    .SE        ( ihash0_SE  ), 
    .GCK       ( ihash0_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_hash0_dma (
    .CK            ( ihash0_dma_CK  ), 
    .E             ( ihash0_dma_E   ), 
    .SE            ( ihash0_dma_SE  ), 
    .GCK           ( ihash0_dma_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_ske0 (
    .CK       ( iske0_CK  ), 
    .E        ( iske0_E   ), 
    .SE       ( iske0_SE  ), 
    .GCK      ( iske0_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_ske0_dma (
    .CK           ( iske0_dma_CK  ), 
    .E            ( iske0_dma_E   ), 
    .SE           ( iske0_dma_SE  ), 
    .GCK          ( iske0_dma_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_pke (
    .CK      ( ipke_CK  ), 
    .E       ( ipke_E   ), 
    .SE      ( ipke_SE  ), 
    .GCK     ( ipke_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_ahb_dma (
    .CK          ( iahb_dma_CK  ), 
    .E           ( iahb_dma_E   ), 
    .SE          ( iahb_dma_SE  ), 
    .GCK         ( iahb_dma_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_axi_dma (
    .CK          ( iaxi_dma_CK  ), 
    .E           ( iaxi_dma_E   ), 
    .SE          ( iaxi_dma_SE  ), 
    .GCK         ( iaxi_dma_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_irom (
    .CK      ( irom_CK  ), 
    .E       ( irom_E   ), 
    .SE      ( irom_SE  ), 
    .GCK     ( irom_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_wdt (
    .CK      ( iwdt_CK  ), 
    .E       ( iwdt_E   ), 
    .SE      ( iwdt_SE  ), 
    .GCK     ( iwdt_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_uart (
    .CK       ( iuart_CK  ), 
    .E        ( iuart_E   ), 
    .SE       ( iuart_SE  ), 
    .GCK      ( iuart_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_crc (
    .CK      ( icrc_CK  ), 
    .E       ( icrc_E   ), 
    .SE      ( icrc_SE  ), 
    .GCK     ( icrc_GCK )  
    ); 

osr_icg OSR_DONTTOUCH_u_icg_tim (
    .CK      ( itim_CK  ), 
    .E       ( itim_E   ), 
    .SE      ( itim_SE  ), 
    .GCK     ( itim_GCK )  
    ); 

always @(posedge irdc_GCK or negedge i_sgrst_n) begin : lowclk_32d
    if(!i_sgrst_n) begin
        cnt        <= 4'h0            ; 
        low_clk    <= 1'h0            ; 
    end else begin
        cnt        <= cnt_next        ; 
        low_clk    <= low_clk_next    ; 
    end
end

endmodule
