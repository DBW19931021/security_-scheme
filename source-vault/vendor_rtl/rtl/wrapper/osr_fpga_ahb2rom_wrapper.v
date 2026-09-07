//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_fpga_ahb2rom_wrapper #(
    parameter  P_BUS_AW         = 19    ,
    parameter  P_BUS_DW         = 32    ,
    parameter  P_BUS_NO_WORD    = 1     ,
    parameter  P_AHB2RAM_TIMING = 0     ,
    parameter  P_CIPHER_EN      = 0     ,
    parameter  P_ECC_EN         = 0     ,
    parameter  P_ECC_TM_CHB     = (P_BUS_DW == 32) ? 7 : (P_BUS_DW == 64) ? 8 : (P_BUS_DW == 128) ? 9 : 10,
    parameter  P_ROM_DW         = (P_ECC_EN == 0)? P_BUS_DW : (P_BUS_DW == 32) ? 7+32 : (P_BUS_DW == 64) ? 8+64 : 32   
    )(

    input  wire                           i_hclk              ,
    input  wire                           i_hresetn           ,

    input  wire                           i_ram_cipher_en     , 
    input  wire [7:0]                     i_ram_cipher_key    , 
    input  wire [P_BUS_DW-1:0]            i_ram_cipher_dnonce , 
    input  wire [P_BUS_AW-3:0]            i_ram_cipher_anonce , 

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
    output wire [3:0]                     o_rom_wen           ,  
    output wire [P_BUS_AW-3:0]            o_rom_addr          , 
    output wire [P_ROM_DW-1:0]            o_rom_wdata         , 
    input  wire [P_ROM_DW-1:0]            i_rom_rdata          

);

localparam     AHB2RAM_P_BUS_AW         = P_BUS_AW; 
localparam     AHB2RAM_P_BUS_NO_WORD    = P_BUS_NO_WORD;
localparam     AHB2RAM_P_AHB2RAM_TIMING = P_AHB2RAM_TIMING; 
localparam     AHB2RAM_P_AW_DEL         = 2; 

localparam     RAM_CIPHER_ECC_P_RAM_AW         = P_BUS_AW - 2; 
localparam     RAM_CIPHER_ECC_P_RAM_DW         = P_BUS_DW    ; 
localparam     RAM_CIPHER_ECC_P_RAM_CIPHER_EN  = P_CIPHER_EN ;
localparam     RAM_CIPHER_ECC_P_RAM_CIPHER_RN  = 2;
localparam     RAM_CIPHER_ECC_P_RAM_ECC_EN     = P_ECC_EN;
localparam     RAM_CIPHER_ECC_P_RAM_ECC_TM_CHB = P_ECC_TM_CHB;
localparam     RAM_CIPHER_ECC_P_RAM_ECC_DW     = P_ROM_DW;

wire [31:0]                                     ahb2ram_o_hrdata         ;
wire                                            ahb2ram_o_hreadyout      ;
wire [1:0]                                      ahb2ram_o_hresp          ;
wire                                            ahb2ram_o_ram_cs         ;
wire [3:0]                                      ahb2ram_o_ram_wen        ;
wire [AHB2RAM_P_BUS_AW-AHB2RAM_P_AW_DEL-1:0]    ahb2ram_o_ram_addr       ;
wire [31:0]                                     ahb2ram_o_ram_wdata      ;

wire                                           ram_cipher_ecc_o_ecc_dec_sec       ;
wire                                           ram_cipher_ecc_o_ecc_dec_ded       ;
wire [RAM_CIPHER_ECC_P_RAM_AW-1:0]             ram_cipher_ecc_o_ecc_err_addr      ;
wire                                           ram_cipher_ecc_o_ecc_dec_sec_nd    ;
wire                                           ram_cipher_ecc_o_ecc_dec_ded_nd    ;
wire [RAM_CIPHER_ECC_P_RAM_DW-1:0]             ram_cipher_ecc_o_p_ram_rdata       ;
wire                                           ram_cipher_ecc_o_c_ram_cs          ;
wire [RAM_CIPHER_ECC_P_RAM_DW/8-1:0]           ram_cipher_ecc_o_c_ram_wen         ;
wire [RAM_CIPHER_ECC_P_RAM_AW-1:0]             ram_cipher_ecc_o_c_ram_addr        ;
wire [RAM_CIPHER_ECC_P_RAM_ECC_DW-1:0]         ram_cipher_ecc_o_c_ram_wdata       ;

wire rom_cipher_ecc_clk     = i_hclk           ;
wire rom_cipher_ecc_hresetn = i_hresetn        ;
wire rom_cipher_ecc_ram_cs  = ahb2ram_o_ram_cs ;
wire [3:0]rom_cipher_ecc_ram_wen = ahb2ram_o_ram_wen;
wire [RAM_CIPHER_ECC_P_RAM_AW-1:0] rom_cipher_ecc_ram_addr = ahb2ram_o_ram_addr;
wire [31:0] rom_cipher_ecc_ram_wdata = ahb2ram_o_ram_wdata;
wire [31:0] rom_o_hrdata = ahb2ram_o_hrdata    ; 
wire rom_o_hreadyout     = ahb2ram_o_hreadyout ;
wire [1:0] rom_o_hresp   = ahb2ram_o_hresp     ;

wire                                            ahb2ram_i_hclk           = i_hclk;
wire                                            ahb2ram_i_hresetn        = i_hresetn;
wire [7:0]                                      ahb2ram_i_ecc_err_rsp_en = i_ecc_err_rsp_en;
wire                                            ahb2ram_i_ecc_dec_sec    = ram_cipher_ecc_o_ecc_dec_sec_nd;
wire                                            ahb2ram_i_ecc_dec_ded    = ram_cipher_ecc_o_ecc_dec_ded_nd;
wire                                            ahb2ram_i_hsel           = i_hsel;
wire                                            ahb2ram_i_hready         = i_hready;   
wire [AHB2RAM_P_BUS_AW-1:0]                     ahb2ram_i_haddr          = i_haddr;  
wire [1:0]                                      ahb2ram_i_htrans         = i_htrans;   
wire                                            ahb2ram_i_hwrite         = i_hwrite;   
wire [2:0]                                      ahb2ram_i_hsize          = i_hsize;    
wire [2:0]                                      ahb2ram_i_hburst         = i_hburst;   
wire [3:0]                                      ahb2ram_i_hprot          = i_hprot;    
wire [3:0]                                      ahb2ram_i_hmaster        = i_hmaster;  
wire [31:0]                                     ahb2ram_i_hwdata         = i_hwdata;   
wire                                            ahb2ram_i_hmastlock      = i_hmastlock;
wire [31:0]                                     ahb2ram_i_ram_rdata      = ram_cipher_ecc_o_p_ram_rdata; 

wire                                           ram_cipher_ecc_i_hclk              = rom_cipher_ecc_clk;     
wire                                           ram_cipher_ecc_i_hresetn           = rom_cipher_ecc_hresetn; 
wire                                           ram_cipher_ecc_i_ram_cipher_en     = i_ram_cipher_en;    
wire [RAM_CIPHER_ECC_P_RAM_CIPHER_RN*4-1:0]    ram_cipher_ecc_i_ram_cipher_key    = i_ram_cipher_key;   
wire [RAM_CIPHER_ECC_P_RAM_DW-1:0]             ram_cipher_ecc_i_ram_cipher_dnonce = i_ram_cipher_dnonce;
wire [RAM_CIPHER_ECC_P_RAM_AW-1:0]             ram_cipher_ecc_i_ram_cipher_anonce = i_ram_cipher_anonce;
wire [7:0]                                     ram_cipher_ecc_i_ecc_ck_en         = i_ecc_ck_en;        
wire [7:0]                                     ram_cipher_ecc_i_ecc_err_rsp_en    = i_ecc_err_rsp_en; 
wire [7:0]                                     ram_cipher_ecc_i_ecc_tm_en         = i_ecc_tm_en;        
wire [RAM_CIPHER_ECC_P_RAM_ECC_TM_CHB-1:0]     ram_cipher_ecc_i_ecc_tm_ckbits     = i_ecc_tm_ckbits;    
wire                                           ram_cipher_ecc_i_p_ram_cs          = rom_cipher_ecc_ram_cs;   
wire [RAM_CIPHER_ECC_P_RAM_DW/8-1:0]           ram_cipher_ecc_i_p_ram_wen         = rom_cipher_ecc_ram_wen;  
wire [RAM_CIPHER_ECC_P_RAM_AW-1:0]             ram_cipher_ecc_i_p_ram_addr        = rom_cipher_ecc_ram_addr; 
wire [RAM_CIPHER_ECC_P_RAM_DW-1:0]             ram_cipher_ecc_i_p_ram_wdata       = rom_cipher_ecc_ram_wdata; 
wire [RAM_CIPHER_ECC_P_RAM_ECC_DW-1:0]         ram_cipher_ecc_i_c_ram_rdata       = i_rom_rdata;       

assign o_ecc_dec_sec       = ram_cipher_ecc_o_ecc_dec_sec  ;
assign o_ecc_dec_ded       = ram_cipher_ecc_o_ecc_dec_ded  ;
assign o_ecc_err_addr      = ram_cipher_ecc_o_ecc_err_addr ;
assign o_hrdata            = rom_o_hrdata    ;            
assign o_hreadyout         = rom_o_hreadyout ;
assign o_hresp             = rom_o_hresp     ;
assign o_rom_cs            = ram_cipher_ecc_o_c_ram_cs;   
assign o_rom_wen           = ram_cipher_ecc_o_c_ram_wen;
assign o_rom_addr          = ram_cipher_ecc_o_c_ram_addr; 
assign o_rom_wdata         = ram_cipher_ecc_o_c_ram_wdata;

osr_ahb_to_ram #(
    .P_BUS_AW             ( AHB2RAM_P_BUS_AW         ), 
    .P_BUS_NO_WORD        ( AHB2RAM_P_BUS_NO_WORD    ), 
    .P_AHB2RAM_TIMING     ( AHB2RAM_P_AHB2RAM_TIMING ), 
    .P_AW_DEL             ( AHB2RAM_P_AW_DEL         )  
    ) u_osr_ahb_to_ram (
    .i_hclk                  ( ahb2ram_i_hclk           ), 
    .i_hresetn               ( ahb2ram_i_hresetn        ), 
    .i_ecc_err_rsp_en        ( ahb2ram_i_ecc_err_rsp_en ), 
    .i_ecc_dec_sec           ( ahb2ram_i_ecc_dec_sec    ), 
    .i_ecc_dec_ded           ( ahb2ram_i_ecc_dec_ded    ), 
    .i_hsel                  ( ahb2ram_i_hsel           ), 
    .i_hready                ( ahb2ram_i_hready         ), 
    .i_haddr                 ( ahb2ram_i_haddr          ), 
    .i_htrans                ( ahb2ram_i_htrans         ), 
    .i_hwrite                ( ahb2ram_i_hwrite         ), 
    .i_hsize                 ( ahb2ram_i_hsize          ), 
    .i_hburst                ( ahb2ram_i_hburst         ), 
    .i_hprot                 ( ahb2ram_i_hprot          ), 
    .i_hmaster               ( ahb2ram_i_hmaster        ), 
    .i_hwdata                ( ahb2ram_i_hwdata         ), 
    .i_hmastlock             ( ahb2ram_i_hmastlock      ), 
    .o_hrdata                ( ahb2ram_o_hrdata         ), 
    .o_hreadyout             ( ahb2ram_o_hreadyout      ), 
    .o_hresp                 ( ahb2ram_o_hresp          ), 
    .o_ram_cs                ( ahb2ram_o_ram_cs         ), 
    .o_ram_wen               ( ahb2ram_o_ram_wen        ), 
    .o_ram_addr              ( ahb2ram_o_ram_addr       ), 
    .o_ram_wdata             ( ahb2ram_o_ram_wdata      ), 
    .i_ram_rdata             ( ahb2ram_i_ram_rdata      )  
    ); 

osr_ram_cipher_ecc_top #(
    .P_RAM_AW             ( RAM_CIPHER_ECC_P_RAM_AW         ), 
    .P_RAM_DW             ( RAM_CIPHER_ECC_P_RAM_DW         ), 
    .P_RAM_CIPHER_EN      ( RAM_CIPHER_ECC_P_RAM_CIPHER_EN  ), 
    .P_RAM_CIPHER_RN      ( RAM_CIPHER_ECC_P_RAM_CIPHER_RN  ), 
    .P_RAM_ECC_EN         ( RAM_CIPHER_ECC_P_RAM_ECC_EN     ), 
    .P_RAM_ECC_TM_CHB     ( RAM_CIPHER_ECC_P_RAM_ECC_TM_CHB ), 
    .P_RAM_ECC_DW         ( RAM_CIPHER_ECC_P_RAM_ECC_DW     )  
    ) u_osr_ram_cipher_ecc_top (
    .i_hclk                            ( ram_cipher_ecc_i_hclk              ), 
    .i_hresetn                         ( ram_cipher_ecc_i_hresetn           ), 
    .i_ram_cipher_en                   ( ram_cipher_ecc_i_ram_cipher_en     ), 
    .i_ram_cipher_key                  ( ram_cipher_ecc_i_ram_cipher_key    ), 
    .i_ram_cipher_dnonce               ( ram_cipher_ecc_i_ram_cipher_dnonce ), 
    .i_ram_cipher_anonce               ( ram_cipher_ecc_i_ram_cipher_anonce ), 
    .i_ecc_ck_en                       ( ram_cipher_ecc_i_ecc_ck_en         ), 
    .i_ecc_err_rsp_en                  ( ram_cipher_ecc_i_ecc_err_rsp_en    ), 
    .i_ecc_tm_en                       ( ram_cipher_ecc_i_ecc_tm_en         ), 
    .i_ecc_tm_ckbits                   ( ram_cipher_ecc_i_ecc_tm_ckbits     ), 
    .o_ecc_dec_sec                     ( ram_cipher_ecc_o_ecc_dec_sec       ), 
    .o_ecc_dec_ded                     ( ram_cipher_ecc_o_ecc_dec_ded       ), 
    .o_ecc_err_addr                    ( ram_cipher_ecc_o_ecc_err_addr      ), 
    .o_ecc_dec_sec_nd                  ( ram_cipher_ecc_o_ecc_dec_sec_nd    ), 
    .o_ecc_dec_ded_nd                  ( ram_cipher_ecc_o_ecc_dec_ded_nd    ), 
    .i_p_ram_cs                        ( ram_cipher_ecc_i_p_ram_cs          ), 
    .i_p_ram_wen                       ( ram_cipher_ecc_i_p_ram_wen         ), 
    .i_p_ram_addr                      ( ram_cipher_ecc_i_p_ram_addr        ), 
    .i_p_ram_wdata                     ( ram_cipher_ecc_i_p_ram_wdata       ), 
    .o_p_ram_rdata                     ( ram_cipher_ecc_o_p_ram_rdata       ), 
    .o_c_ram_cs                        ( ram_cipher_ecc_o_c_ram_cs          ), 
    .o_c_ram_wen                       ( ram_cipher_ecc_o_c_ram_wen         ), 
    .o_c_ram_addr                      ( ram_cipher_ecc_o_c_ram_addr        ), 
    .o_c_ram_wdata                     ( ram_cipher_ecc_o_c_ram_wdata       ), 
    .i_c_ram_rdata                     ( ram_cipher_ecc_i_c_ram_rdata       )  
    ); 

endmodule 
