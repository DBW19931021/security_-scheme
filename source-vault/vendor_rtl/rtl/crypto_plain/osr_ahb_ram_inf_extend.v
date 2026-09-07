//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_ram_inf_extend #(
    parameter  P_BUS_AW         = 25    ,
    parameter  P_BUS_DW         = 32    ,
    parameter  P_BUS_NO_WORD    = 1     ,
    parameter  P_AHB_TIMING     = 0     ,
    parameter  P_AHB2RAM_TIMING = 0     ,
    parameter  P_CIPHER_EN      = 1     ,
    parameter  P_RAM_BLOCK_AW   = 5     ,
    parameter  P_ECC_EN         = 1     ,
    parameter  P_ECC_TM_CHB     = (P_BUS_DW == 32) ? 7 : (P_BUS_DW == 64) ? 8 : (P_BUS_DW == 128) ? 9 : 10,
    parameter  P_RAM_DW         = (P_ECC_EN == 0)? P_BUS_DW : (P_BUS_DW == 32) ? 7+32 : (P_BUS_DW == 64) ? 8+64 : 32 , 
    parameter  P_MEM_INIT       = 1     ,
    parameter  P_MEM_INIT_DEEP  = 512*1024/4
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

    output wire                           o_ram_cs            , 
    output wire [3:0]                     o_ram_wen           ,  
    output wire [P_BUS_AW-3:0]            o_ram_addr          , 
    output wire [P_RAM_DW-1:0]            o_ram_wdata         , 
    input  wire [P_RAM_DW-1:0]            i_ram_rdata         ,

    input  wire                           i_mem_init_rst_n    , 
    output wire                           o_mem_init_done       

);

localparam OSR_AHB_BUS_TIMING_ISOLATION = P_AHB_TIMING; 

localparam A1OSR_AHB_BUS_TIMING_ISOLATION    = (OSR_AHB_BUS_TIMING_ISOLATION   > 0) ? OSR_AHB_BUS_TIMING_ISOLATION    : 1 ;
localparam A1P_MEM_INIT                      = (P_MEM_INIT                     > 0) ? P_MEM_INIT                      : 1 ;

localparam     AHBSYNC_P_AW    = P_BUS_AW;
localparam     AHBSYNC_P_DW    = P_BUS_DW;
localparam     AHBSYNC_P_MW    = 4; 
localparam     AHBSYNC_P_BURST = 1; 

localparam     AHB2RAM_P_BUS_AW         = P_BUS_AW; 
localparam     AHB2RAM_P_BUS_NO_WORD    = P_BUS_NO_WORD;
localparam     AHB2RAM_P_AHB2RAM_TIMING = P_AHB2RAM_TIMING; 
localparam     AHB2RAM_P_AW_DEL         = 2; 

localparam     RAM_CIPHER_ECC_P_RAM_AW         = P_BUS_AW - 2; 
localparam     RAM_CIPHER_ECC_P_RAM_DW         = P_BUS_DW    ; 
localparam     RAM_CIPHER_ECC_P_RAM_CIPHER_EN  = P_CIPHER_EN ;
localparam     RAM_CIPHER_ECC_P_RAM_CIPHER_RN  = 2;
localparam     RAM_CIPHER_ECC_P_RAM_BLOCK_AW   = P_RAM_BLOCK_AW;
localparam     RAM_CIPHER_ECC_P_RAM_ECC_EN     = P_ECC_EN;
localparam     RAM_CIPHER_ECC_P_RAM_ECC_TM_CHB = P_ECC_TM_CHB;
localparam     RAM_CIPHER_ECC_P_RAM_ECC_DW     = P_RAM_DW;

localparam     MEM_INIT_P_MEM_DATA_WIDTH = P_BUS_DW;
localparam     MEM_INIT_P_MEM_ADDR_WIDTH = AHB2RAM_P_BUS_AW-AHB2RAM_P_AW_DEL;
localparam     MEM_INIT_P_MEM_INIT_DEEP  = P_MEM_INIT_DEEP;

wire  ahbsync_o_hready   ;

wire [A1OSR_AHB_BUS_TIMING_ISOLATION-1:0]                      ahbsync_o_hreadyouts ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION-1:0]                      ahbsync_o_hresps     ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*AHBSYNC_P_DW-1:0]         ahbsync_o_hrdatas    ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*AHBSYNC_P_AW-1:0]         ahbsync_o_haddrm     ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*(1+1)-1:0]                ahbsync_o_htransm    ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                ahbsync_o_hsizem     ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION-1:0]                      ahbsync_o_hwritem    ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*(3+1)-1:0]                ahbsync_o_hprotm     ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*AHBSYNC_P_MW-1:0]         ahbsync_o_hmasterm   ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION-1:0]                      ahbsync_o_hmastlockm ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*AHBSYNC_P_DW-1:0]         ahbsync_o_hwdatam    ;
wire [A1OSR_AHB_BUS_TIMING_ISOLATION*(2+1)-1:0]                ahbsync_o_hburstm    ;

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

wire [A1P_MEM_INIT-1:0]                                   mem_init_o_ram_wren  ;
wire [A1P_MEM_INIT*MEM_INIT_P_MEM_ADDR_WIDTH-1:0]         mem_init_o_ram_waddr ;
wire [A1P_MEM_INIT*MEM_INIT_P_MEM_DATA_WIDTH-1:0]         mem_init_o_ram_wdata ;
wire [A1P_MEM_INIT-1:0]                                   mem_init_o_init_done ;

generate if(OSR_AHB_BUS_TIMING_ISOLATION == 0) begin : no_ahbsync
assign ahbsync_o_hreadyouts = ahb2ram_o_hreadyout                                             ;
assign ahbsync_o_hresps     = ahb2ram_o_hresp[0]                                              ;
assign ahbsync_o_hrdatas    = ahb2ram_o_hrdata                                                ;
assign ahbsync_o_haddrm     = i_haddr                                                         ;
assign ahbsync_o_htransm    = i_htrans                                                        ;
assign ahbsync_o_hsizem     = i_hsize                                                         ;
assign ahbsync_o_hwritem    = i_hwrite                                                        ;
assign ahbsync_o_hprotm     = i_hprot                                                         ;
assign ahbsync_o_hmasterm   = 4'b0                                                            ;
assign ahbsync_o_hmastlockm = i_hmastlock                                                     ;
assign ahbsync_o_hwdatam    = i_hwdata                                                        ;
assign ahbsync_o_hburstm    = i_hburst                                                        ;
assign ahbsync_o_hready     = i_hready                                                        ;
end
else begin
assign ahbsync_o_hready     = ahb2ram_o_hreadyout                                             ;
end
endgenerate

wire                       ahbsync_i_hclk       = i_hclk              ;    
wire                       ahbsync_i_hresetn    = i_hresetn           ; 
wire                       ahbsync_i_hsels      = i_hsel              ;          
wire [AHBSYNC_P_AW-1:0]    ahbsync_i_haddrs     = i_haddr             ;               
wire [1:0]                 ahbsync_i_htranss    = i_htrans            ;            
wire [2:0]                 ahbsync_i_hsizes     = i_hsize             ;               
wire                       ahbsync_i_hwrites    = i_hwrite            ;               
wire                       ahbsync_i_hreadys    = i_hready            ;               
wire [3:0]                 ahbsync_i_hprots     = i_hprot             ;               
wire [AHBSYNC_P_MW-1:0]    ahbsync_i_hmasters   = i_hmaster           ;               
wire                       ahbsync_i_hmastlocks = i_hmastlock         ;               
wire [AHBSYNC_P_DW-1:0]    ahbsync_i_hwdatas    = i_hwdata            ;               
wire [2:0]                 ahbsync_i_hbursts    = i_hburst            ;               
wire                       ahbsync_i_hreadyoutm = ahb2ram_o_hreadyout ;  
wire                       ahbsync_i_hresps     = ahb2ram_o_hresp[0]  ;     
wire [AHBSYNC_P_DW-1:0]    ahbsync_i_hrdatam    = ahb2ram_o_hrdata    ;     

wire                                            ahb2ram_i_hclk           = i_hclk;
wire                                            ahb2ram_i_hresetn        = i_hresetn;
wire [7:0]                                      ahb2ram_i_ecc_err_rsp_en = i_ecc_err_rsp_en;
wire                                            ahb2ram_i_ecc_dec_sec    = ram_cipher_ecc_o_ecc_dec_sec_nd;
wire                                            ahb2ram_i_ecc_dec_ded    = ram_cipher_ecc_o_ecc_dec_ded_nd;
wire                                            ahb2ram_i_hsel           = ahbsync_o_htransm[1];
wire                                            ahb2ram_i_hready         = ahbsync_o_hready;   
wire [AHB2RAM_P_BUS_AW-1:0]                     ahb2ram_i_haddr          = ahbsync_o_haddrm;  
wire [1:0]                                      ahb2ram_i_htrans         = ahbsync_o_htransm;   
wire                                            ahb2ram_i_hwrite         = ahbsync_o_hwritem;   
wire [2:0]                                      ahb2ram_i_hsize          = ahbsync_o_hsizem;    
wire [2:0]                                      ahb2ram_i_hburst         = ahbsync_o_hburstm;   
wire [3:0]                                      ahb2ram_i_hprot          = ahbsync_o_hprotm;    
wire [3:0]                                      ahb2ram_i_hmaster        = ahbsync_o_hmasterm;  
wire [31:0]                                     ahb2ram_i_hwdata         = ahbsync_o_hwdatam;   
wire                                            ahb2ram_i_hmastlock      = ahbsync_o_hmastlockm;
wire [31:0]                                     ahb2ram_i_ram_rdata      = ram_cipher_ecc_o_p_ram_rdata; 

wire                                           ram_cipher_ecc_i_hclk              = i_hclk; 
wire                                           ram_cipher_ecc_i_hresetn           = i_hresetn;
wire                                           ram_cipher_ecc_i_ram_cipher_en     = i_ram_cipher_en & mem_init_o_init_done;    
wire [RAM_CIPHER_ECC_P_RAM_CIPHER_RN*4-1:0]    ram_cipher_ecc_i_ram_cipher_key    = i_ram_cipher_key;   
wire [RAM_CIPHER_ECC_P_RAM_DW-1:0]             ram_cipher_ecc_i_ram_cipher_dnonce = i_ram_cipher_dnonce;
wire [RAM_CIPHER_ECC_P_RAM_AW-1:0]             ram_cipher_ecc_i_ram_cipher_anonce = i_ram_cipher_anonce;
wire [7:0]                                     ram_cipher_ecc_i_ecc_ck_en         = i_ecc_ck_en;        
wire [7:0]                                     ram_cipher_ecc_i_ecc_err_rsp_en    = i_ecc_err_rsp_en; 
wire [7:0]                                     ram_cipher_ecc_i_ecc_tm_en         = i_ecc_tm_en;        
wire [RAM_CIPHER_ECC_P_RAM_ECC_TM_CHB-1:0]     ram_cipher_ecc_i_ecc_tm_ckbits     = i_ecc_tm_ckbits;    
wire                                           ram_cipher_ecc_i_p_ram_cs          = mem_init_o_init_done ? ahb2ram_o_ram_cs    : mem_init_o_ram_wren;   
wire [RAM_CIPHER_ECC_P_RAM_DW/8-1:0]           ram_cipher_ecc_i_p_ram_wen         = mem_init_o_init_done ? ahb2ram_o_ram_wen   : 4'hf      ;  
wire [RAM_CIPHER_ECC_P_RAM_AW-1:0]             ram_cipher_ecc_i_p_ram_addr        = mem_init_o_init_done ? ahb2ram_o_ram_addr  : mem_init_o_ram_waddr; 
wire [RAM_CIPHER_ECC_P_RAM_DW-1:0]             ram_cipher_ecc_i_p_ram_wdata       = mem_init_o_init_done ? ahb2ram_o_ram_wdata : mem_init_o_ram_wdata; 
wire [RAM_CIPHER_ECC_P_RAM_ECC_DW-1:0]         ram_cipher_ecc_i_c_ram_rdata       = i_ram_rdata;       

wire                                    mem_init_clk         = i_hclk          ;
wire                                    mem_init_rst_n       = i_mem_init_rst_n;

assign o_ecc_dec_sec       = ram_cipher_ecc_o_ecc_dec_sec; 
assign o_ecc_dec_ded       = ram_cipher_ecc_o_ecc_dec_ded;
assign o_ecc_err_addr      = ram_cipher_ecc_o_ecc_err_addr;
assign o_hrdata            = ahbsync_o_hrdatas;
assign o_hreadyout         = ahbsync_o_hreadyouts;
assign o_hresp             = {1'b0,ahbsync_o_hresps};
assign o_ram_cs            = ram_cipher_ecc_o_c_ram_cs; 
assign o_ram_wen           = ram_cipher_ecc_o_c_ram_wen;
assign o_ram_addr          = ram_cipher_ecc_o_c_ram_addr;
assign o_ram_wdata         = ram_cipher_ecc_o_c_ram_wdata;
assign o_mem_init_done     = mem_init_o_init_done;

generate if(OSR_AHB_BUS_TIMING_ISOLATION) begin : g_ahbsync
osr_ahb_to_ahb_sync #(
    .P_AW        ( AHBSYNC_P_AW    ), 
    .P_DW        ( AHBSYNC_P_DW    ), 
    .P_MW        ( AHBSYNC_P_MW    ), 
    .P_BURST     ( AHBSYNC_P_BURST )  
    ) u_ahbsync (
    .i_hclk              ( ahbsync_i_hclk       ), 
    .i_hresetn           ( ahbsync_i_hresetn    ), 
    .i_hsels             ( ahbsync_i_hsels      ), 
    .i_haddrs            ( ahbsync_i_haddrs     ), 
    .i_htranss           ( ahbsync_i_htranss    ), 
    .i_hsizes            ( ahbsync_i_hsizes     ), 
    .i_hwrites           ( ahbsync_i_hwrites    ), 
    .i_hreadys           ( ahbsync_i_hreadys    ), 
    .i_hprots            ( ahbsync_i_hprots     ), 
    .i_hmasters          ( ahbsync_i_hmasters   ), 
    .i_hmastlocks        ( ahbsync_i_hmastlocks ), 
    .i_hwdatas           ( ahbsync_i_hwdatas    ), 
    .i_hbursts           ( ahbsync_i_hbursts    ), 
    .o_hreadyouts        ( ahbsync_o_hreadyouts ), 
    .o_hresps            ( ahbsync_o_hresps     ), 
    .o_hrdatas           ( ahbsync_o_hrdatas    ), 
    .o_haddrm            ( ahbsync_o_haddrm     ), 
    .o_htransm           ( ahbsync_o_htransm    ), 
    .o_hsizem            ( ahbsync_o_hsizem     ), 
    .o_hwritem           ( ahbsync_o_hwritem    ), 
    .o_hprotm            ( ahbsync_o_hprotm     ), 
    .o_hmasterm          ( ahbsync_o_hmasterm   ), 
    .o_hmastlockm        ( ahbsync_o_hmastlockm ), 
    .o_hwdatam           ( ahbsync_o_hwdatam    ), 
    .o_hburstm           ( ahbsync_o_hburstm    ), 
    .i_hreadyoutm        ( ahbsync_i_hreadyoutm ), 
    .i_hresps            ( ahbsync_i_hresps     ), 
    .i_hrdatam           ( ahbsync_i_hrdatam    )  
    ); 
end
endgenerate

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

osr_ram_cipher_ecc_top_extend #(
    .P_RAM_AW             ( RAM_CIPHER_ECC_P_RAM_AW         ), 
    .P_RAM_DW             ( RAM_CIPHER_ECC_P_RAM_DW         ), 
    .P_RAM_CIPHER_EN      ( RAM_CIPHER_ECC_P_RAM_CIPHER_EN  ), 
    .P_RAM_CIPHER_RN      ( RAM_CIPHER_ECC_P_RAM_CIPHER_RN  ), 
    .P_RAM_BLOCK_AW       ( RAM_CIPHER_ECC_P_RAM_BLOCK_AW   ), 
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

generate if(P_MEM_INIT) begin : g_osr_mem_init
osr_mem_init #(
    .P_MEM_DATA_WIDTH     ( MEM_INIT_P_MEM_DATA_WIDTH ), 
    .P_MEM_ADDR_WIDTH     ( MEM_INIT_P_MEM_ADDR_WIDTH ), 
    .P_MEM_INIT_DEEP      ( MEM_INIT_P_MEM_INIT_DEEP  )  
    ) u_osr_mem_init (
    .clk                 ( mem_init_clk         ), 
    .rst_n               ( mem_init_rst_n       ), 
    .o_ram_wren          ( mem_init_o_ram_wren  ), 
    .o_ram_waddr         ( mem_init_o_ram_waddr ), 
    .o_ram_wdata         ( mem_init_o_ram_wdata ), 
    .o_init_done         ( mem_init_o_init_done )  
    ); 
end
endgenerate

generate if(!P_MEM_INIT) begin : g_no_osr_mem_init
    assign mem_init_o_ram_wren  = 1'b0;
    assign mem_init_o_ram_waddr = {MEM_INIT_P_MEM_ADDR_WIDTH{1'b0}};
    assign mem_init_o_ram_wdata = 'b0;
    assign mem_init_o_init_done = 1'b1;
end
endgenerate

endmodule 
