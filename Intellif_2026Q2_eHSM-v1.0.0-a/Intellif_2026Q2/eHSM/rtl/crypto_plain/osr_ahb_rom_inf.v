//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_rom_inf #(
    parameter  P_BUS_AW         = 25    ,
    parameter  P_BUS_DW         = 32    ,
    parameter  P_BUS_NO_WORD    = 1     ,
    parameter  P_AHB2RAM_TIMING = 0     ,
    parameter  P_CIPHER_EN      = 1     ,
    parameter  P_ECC_EN         = 1     ,
    parameter  P_ECC_TM_CHB     = (P_BUS_DW == 32) ? 7 : (P_BUS_DW == 64) ? 8 : (P_BUS_DW == 128) ? 9 : 10,
    parameter  P_ROM_DW         = (P_ECC_EN == 0)? P_BUS_DW : (P_BUS_DW == 32) ? 7+32 : (P_BUS_DW == 64) ? 8+64 : 32   
    )(

    input  wire                           i_hclk              ,
    input  wire                           i_hresetn           ,

    input  wire [7:0]                     i_ecc_ck_en         , 
    input  wire [7:0]                     i_ecc_err_rsp_en    , 
    input  wire [7:0]                     i_ecc_tm_en         , 
    input  wire [P_ECC_TM_CHB-1:0]        i_ecc_tm_ckbits     , 

    output wire                           o_ecc_dec_sec       ,
    output wire                           o_ecc_dec_ded       ,
    output wire [P_BUS_AW-3:0]            o_ecc_err_addr      ,

    input  wire                           i_hsel              ,
    input  wire                           i_hready            ,
    input  wire [P_BUS_AW-1:0]            i_haddr             ,
    input  wire [1:0]                     i_htrans            ,
    input  wire                           i_hwrite            ,
    input  wire [2:0]                     i_hsize             ,
    input  wire [2:0]                     i_hburst            ,
    input  wire [3:0]                     i_hprot             ,
    input  wire [3:0]                     i_hmaster           ,
    input  wire [31:0]                    i_hwdata            ,
    input  wire                           i_hmastlock         ,

    output wire [31:0]                    o_hrdata            ,
    output wire                           o_hreadyout         ,
    output wire [1:0]                     o_hresp             ,

    output wire                           o_rom_cs            , 
    output wire [P_BUS_AW-3:0]            o_rom_addr          ,
    input  wire [P_ROM_DW-1:0]            i_rom_rdata          

);

localparam     AHB2ROM_P_BUS_AW         = P_BUS_AW; 
localparam     AHB2ROM_P_BUS_NO_WORD    = P_BUS_NO_WORD; 
localparam     AHB2ROM_P_AHB2RAM_TIMING = P_AHB2RAM_TIMING;
localparam     AHB2ROM_P_AW_DEL         = 0; 

localparam     ROM_CIPHER_ECC_P_ROM_AW         = P_BUS_AW    ; 
localparam     ROM_CIPHER_ECC_P_ROM_DW         = P_BUS_DW    ; 
localparam     ROM_CIPHER_ECC_P_ROM_CIPHER_EN  = P_CIPHER_EN ;
localparam     ROM_CIPHER_ECC_P_ROM_CIPHER_RN  = 2;
localparam     ROM_CIPHER_ECC_P_ROM_ECC_EN     = P_ECC_EN;
localparam     ROM_CIPHER_ECC_P_ROM_ECC_TM_CHB = (ROM_CIPHER_ECC_P_ROM_DW == 32) ? 7 : (ROM_CIPHER_ECC_P_ROM_DW == 64) ? 8 : (ROM_CIPHER_ECC_P_ROM_DW == 128) ? 9 : 10;
localparam     ROM_CIPHER_ECC_P_ROM_ECC_DW     = P_ROM_DW;

wire [31:0]                                     ahb2rom_o_hrdata         ;
wire                                            ahb2rom_o_hreadyout      ;
wire [1:0]                                      ahb2rom_o_hresp          ;
wire                                            ahb2rom_o_ram_cs         ;
wire [3:0]                                      ahb2rom_o_ram_wen        ;
wire [AHB2ROM_P_BUS_AW-AHB2ROM_P_AW_DEL-1:0]    ahb2rom_o_ram_addr       ;
wire [31:0]                                     ahb2rom_o_ram_wdata      ;

wire                                          rom_cipher_ecc_o_ecc_dec_sec       ;
wire                                          rom_cipher_ecc_o_ecc_dec_ded       ;
wire [ROM_CIPHER_ECC_P_ROM_AW-3:0]            rom_cipher_ecc_o_ecc_err_addr      ;
wire                                          rom_cipher_ecc_o_ecc_dec_sec_nd    ;
wire                                          rom_cipher_ecc_o_ecc_dec_ded_nd    ;
wire [ROM_CIPHER_ECC_P_ROM_DW-1:0]            rom_cipher_ecc_o_p_rom_rdata       ;
wire                                          rom_cipher_ecc_o_c_rom_cs          ;
wire [ROM_CIPHER_ECC_P_ROM_AW-3:0]            rom_cipher_ecc_o_c_rom_addr        ;

wire rom_cipher_ecc_clk     = i_hclk           ;
wire rom_cipher_ecc_hresetn = i_hresetn        ;
wire rom_cipher_ecc_ram_cs  = ahb2rom_o_ram_cs;
wire [ROM_CIPHER_ECC_P_ROM_AW-1:0] rom_cipher_ecc_ram_addr = ahb2rom_o_ram_addr;
wire [31:0] rom_o_hrdata = ahb2rom_o_hrdata    ; 
wire rom_o_hreadyout     = ahb2rom_o_hreadyout ;
wire [1:0] rom_o_hresp   = ahb2rom_o_hresp     ;

wire [7:0] ecc_err_rsp_en = i_ecc_err_rsp_en;

wire                                            ahb2rom_i_hclk           = i_hclk;
wire                                            ahb2rom_i_hresetn        = i_hresetn;
wire [7:0]                                      ahb2rom_i_ecc_err_rsp_en = ecc_err_rsp_en;
wire                                            ahb2rom_i_ecc_dec_sec    = rom_cipher_ecc_o_ecc_dec_sec_nd;
wire                                            ahb2rom_i_ecc_dec_ded    = rom_cipher_ecc_o_ecc_dec_ded_nd;
wire                                            ahb2rom_i_hsel           = i_hsel;
wire                                            ahb2rom_i_hready         = i_hready;   
wire [AHB2ROM_P_BUS_AW-1:0]                     ahb2rom_i_haddr          = i_haddr;  
wire [1:0]                                      ahb2rom_i_htrans         = i_htrans;   
wire                                            ahb2rom_i_hwrite         = 1'b0;   
wire [2:0]                                      ahb2rom_i_hsize          = i_hsize;    
wire [2:0]                                      ahb2rom_i_hburst         = i_hburst;   
wire [3:0]                                      ahb2rom_i_hprot          = i_hprot;    
wire [3:0]                                      ahb2rom_i_hmaster        = i_hmaster;  
wire [31:0]                                     ahb2rom_i_hwdata         = i_hwdata;   
wire                                            ahb2rom_i_hmastlock      = i_hmastlock;
wire [31:0]                                     ahb2rom_i_ram_rdata      = rom_cipher_ecc_o_p_rom_rdata; 

wire                                          rom_cipher_ecc_i_hclk              = rom_cipher_ecc_clk;    
wire                                          rom_cipher_ecc_i_hresetn           = rom_cipher_ecc_hresetn;

wire [7:0]                                    rom_cipher_ecc_i_ecc_ck_en         = i_ecc_ck_en;        
wire [7:0]                                    rom_cipher_ecc_i_ecc_err_rsp_en    = i_ecc_err_rsp_en; 
wire [7:0]                                    rom_cipher_ecc_i_ecc_tm_en         = i_ecc_tm_en;        
wire [ROM_CIPHER_ECC_P_ROM_ECC_TM_CHB-1:0]    rom_cipher_ecc_i_ecc_tm_ckbits     = i_ecc_tm_ckbits;    

wire                                          rom_cipher_ecc_i_p_rom_cs          = rom_cipher_ecc_ram_cs;   
wire [ROM_CIPHER_ECC_P_ROM_AW-1:0]            rom_cipher_ecc_i_p_rom_addr        = {rom_cipher_ecc_ram_addr[P_BUS_AW-1:2],2'b0}; 
wire [ROM_CIPHER_ECC_P_ROM_ECC_DW-1:0]        rom_cipher_ecc_i_c_rom_rdata       = i_rom_rdata;       

assign o_ecc_dec_sec       = rom_cipher_ecc_o_ecc_dec_sec; 
assign o_ecc_dec_ded       = rom_cipher_ecc_o_ecc_dec_ded;
assign o_ecc_err_addr      = rom_cipher_ecc_o_ecc_err_addr;

assign o_hrdata            = rom_o_hrdata    ; 
assign o_hreadyout         = rom_o_hreadyout ;
assign o_hresp             = rom_o_hresp     ;
assign o_rom_cs            = rom_cipher_ecc_o_c_rom_cs; 
assign o_rom_addr          = rom_cipher_ecc_o_c_rom_addr;

osr_ahb_to_ram #(
    .P_BUS_AW             ( AHB2ROM_P_BUS_AW         ), 
    .P_BUS_NO_WORD        ( AHB2ROM_P_BUS_NO_WORD    ), 
    .P_AHB2RAM_TIMING     ( AHB2ROM_P_AHB2RAM_TIMING ), 
    .P_AW_DEL             ( AHB2ROM_P_AW_DEL         )  
    ) u_osr_ahb_to_rom (
    .i_hclk                  ( ahb2rom_i_hclk           ), 
    .i_hresetn               ( ahb2rom_i_hresetn        ), 
    .i_ecc_err_rsp_en        ( ahb2rom_i_ecc_err_rsp_en ), 
    .i_ecc_dec_sec           ( ahb2rom_i_ecc_dec_sec    ), 
    .i_ecc_dec_ded           ( ahb2rom_i_ecc_dec_ded    ), 
    .i_hsel                  ( ahb2rom_i_hsel           ), 
    .i_hready                ( ahb2rom_i_hready         ), 
    .i_haddr                 ( ahb2rom_i_haddr          ), 
    .i_htrans                ( ahb2rom_i_htrans         ), 
    .i_hwrite                ( ahb2rom_i_hwrite         ), 
    .i_hsize                 ( ahb2rom_i_hsize          ), 
    .i_hburst                ( ahb2rom_i_hburst         ), 
    .i_hprot                 ( ahb2rom_i_hprot          ), 
    .i_hmaster               ( ahb2rom_i_hmaster        ), 
    .i_hwdata                ( ahb2rom_i_hwdata         ), 
    .i_hmastlock             ( ahb2rom_i_hmastlock      ), 
    .o_hrdata                ( ahb2rom_o_hrdata         ), 
    .o_hreadyout             ( ahb2rom_o_hreadyout      ), 
    .o_hresp                 ( ahb2rom_o_hresp          ), 
    .o_ram_cs                ( ahb2rom_o_ram_cs         ), 
    .o_ram_wen               ( ahb2rom_o_ram_wen        ), 
    .o_ram_addr              ( ahb2rom_o_ram_addr       ), 
    .o_ram_wdata             ( ahb2rom_o_ram_wdata      ), 
    .i_ram_rdata             ( ahb2rom_i_ram_rdata      )  
    ); 

osr_rom_cipher_ecc_top #(
    .P_ROM_AW             ( ROM_CIPHER_ECC_P_ROM_AW         ), 
    .P_ROM_DW             ( ROM_CIPHER_ECC_P_ROM_DW         ), 
    .P_ROM_CIPHER_EN      ( ROM_CIPHER_ECC_P_ROM_CIPHER_EN  ), 
    .P_ROM_CIPHER_RN      ( ROM_CIPHER_ECC_P_ROM_CIPHER_RN  ), 
    .P_ROM_ECC_EN         ( ROM_CIPHER_ECC_P_ROM_ECC_EN     ), 
    .P_ROM_ECC_TM_CHB     ( ROM_CIPHER_ECC_P_ROM_ECC_TM_CHB ), 
    .P_ROM_ECC_DW         ( ROM_CIPHER_ECC_P_ROM_ECC_DW     )  
    ) u_osr_rom_cipher_ecc_top (
    .i_hclk                            ( rom_cipher_ecc_i_hclk              ), 
    .i_hresetn                         ( rom_cipher_ecc_i_hresetn           ), 
    .i_ecc_ck_en                       ( rom_cipher_ecc_i_ecc_ck_en         ), 
    .i_ecc_err_rsp_en                  ( rom_cipher_ecc_i_ecc_err_rsp_en    ), 
    .i_ecc_tm_en                       ( rom_cipher_ecc_i_ecc_tm_en         ), 
    .i_ecc_tm_ckbits                   ( rom_cipher_ecc_i_ecc_tm_ckbits     ), 
    .o_ecc_dec_sec                     ( rom_cipher_ecc_o_ecc_dec_sec       ), 
    .o_ecc_dec_ded                     ( rom_cipher_ecc_o_ecc_dec_ded       ), 
    .o_ecc_err_addr                    ( rom_cipher_ecc_o_ecc_err_addr      ), 
    .o_ecc_dec_sec_nd                  ( rom_cipher_ecc_o_ecc_dec_sec_nd    ), 
    .o_ecc_dec_ded_nd                  ( rom_cipher_ecc_o_ecc_dec_ded_nd    ), 
    .i_p_rom_cs                        ( rom_cipher_ecc_i_p_rom_cs          ), 
    .i_p_rom_addr                      ( rom_cipher_ecc_i_p_rom_addr        ), 
    .o_p_rom_rdata                     ( rom_cipher_ecc_o_p_rom_rdata       ), 
    .o_c_rom_cs                        ( rom_cipher_ecc_o_c_rom_cs          ), 
    .o_c_rom_addr                      ( rom_cipher_ecc_o_c_rom_addr        ), 
    .i_c_rom_rdata                     ( rom_cipher_ecc_i_c_rom_rdata       )  
    ); 

endmodule 
