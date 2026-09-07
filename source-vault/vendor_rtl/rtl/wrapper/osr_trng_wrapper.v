//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_trng_wrapper (
    input   wire                    i_clk            , 
    input   wire                    i_rst_n          , 
    input   wire                    i_endian         , 
    input   wire                    i_s_hsel         , 
    input   wire  [11:0]            i_s_haddr        , 
    input   wire  [1:0]             i_s_htrans       , 
    input   wire                    i_s_hwrite       , 
    input   wire  [2:0]             i_s_hburst       , 
    input   wire  [2:0]             i_s_hsize        , 
    input   wire  [3:0]             i_s_hprot        , 
    input   wire                    i_s_hmastlock    , 
    input   wire                    i_s_hready       , 
    input   wire  [31:0]            i_s_hwdata       , 
    output  wire  [31:0]            o_s_hrdata       , 
    output  wire                    o_s_hresp        , 
    output  wire                    o_s_hreadyout    , 
    output  wire                    o_irq            , 
    input   wire                    i_trng_pop       , 
    output  wire                    o_trng_drdy      , 
    output  wire  [31:0]            o_trng_data      , 
    input   wire                    i_scan_mode      , 

    input  wire                     i_drbg_mode      ,   
    input  wire   [63: 0]           i_ro_src_en      ,   
    input  wire   [3:0]             i_ro_clk_en      , 
    input  wire   [1:0]             i_ro_src_fsel    , 
    input   wire                    i_skip_startup   , 
    input   wire                    i_sclk_sel       , 
    output  wire                    o_trng_rdy       , 
    output  wire                    o_alarm          , 
    output  wire  [3:0]             o_ro_clk         , 
    output  wire  [3:0]             o_ro_out         , 
    output  wire                    o_tero_busy      , 
    output  wire  [3:0]             o_tero_es_out        
);

    localparam P_CONFIG_DRBG_AES_EN = 1;

    localparam P_CONFIG_DRBG_SM4_EN = 1;

    localparam P_CONFIG_DRBG_LFSR_EN = 0;

`ifdef OSR_TRNG_FPGA
    localparam P_CONFIG_FPGA = 1;
`else
    localparam P_CONFIG_FPGA = 0;
`endif


wire        trng_drdy   ;
wire [31:0] trng_data   ;
wire [1:0]  osc_en_clk = {1'b1,i_clk};

assign o_trng_drdy = trng_drdy  ;
assign o_trng_data = trng_data  ;

osr_trng_top #(
    .p_CONFIG_DRBG_AES_EN  (P_CONFIG_DRBG_AES_EN),
    .p_CONFIG_DRBG_SM4_EN  (P_CONFIG_DRBG_SM4_EN),
    .p_CONFIG_DRBG_LFSR_EN (P_CONFIG_DRBG_LFSR_EN),
    .p_CONFIG_FPGA         (P_CONFIG_FPGA)
)u_trng (
    .clk                ( i_clk          ), 
`ifdef OSR_TRNG_FPGA            
    .i_osc_en_clk       (   osc_en_clk   ), 
`endif
    .rst_n              ( i_rst_n        ), 
    .i_s_hsel           ( i_s_hsel       ), 
    .i_s_htrans         ( i_s_htrans     ), 
    .i_s_hburst         ( i_s_hburst     ), 
    .i_s_hsize          ( i_s_hsize      ), 
    .i_s_haddr          ( i_s_haddr      ), 
    .i_s_hmastlock      ( i_s_hmastlock  ), 
    .i_s_hprot          ( i_s_hprot      ), 
    .i_s_hwdata         ( i_s_hwdata     ), 
    .i_s_hwrite         ( i_s_hwrite     ), 
    .i_s_hready         ( i_s_hready     ), 
    .o_s_hrdata         ( o_s_hrdata     ), 
    .o_s_hresp          ( o_s_hresp      ), 
    .o_s_hreadyout      ( o_s_hreadyout  ), 
    .o_irq              ( o_irq          ), 
    .i_trng_pop         ( i_trng_pop     ), 
    .o_trng_drdy        (   trng_drdy    ), 
    .o_trng_data        (   trng_data    ), 
    .i_scan_mode        ( i_scan_mode    ), 
    .i_skip_startup     ( i_skip_startup ), 
    .i_drbg_mode        ( i_drbg_mode    ), 
    .i_cbc_sm4          ( 1'b0           ), 
    .i_sclk_sel         ( i_sclk_sel     ), 
    .i_ro_src_en        ( i_ro_src_en    ), 
    .i_ro_clk_en        ( i_ro_clk_en    ), 
    .i_ro_src_fsel      ( i_ro_src_fsel  ), 
    .o_trng_rdy         ( o_trng_rdy     ), 
    .o_alarm            ( o_alarm        ), 
    .o_ro_clk           ( o_ro_clk       ), 
    .o_ro_out           ( o_ro_out       )  
    ); 

endmodule 
