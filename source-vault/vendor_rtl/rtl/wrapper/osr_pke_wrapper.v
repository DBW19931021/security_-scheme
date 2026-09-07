//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_pke_wrapper #(

    parameter    P_RAM_WIDTH     = 72 ,

    parameter    P_NONE_         = 0

    )(
    input   wire                                  i_clk_ahb       ,
    input   wire                                  i_rst_n_ahb     ,
    input   wire                                  i_clk_core      ,
    input   wire                                  i_rst_n_core    ,
    input   wire                                  i_ahb_endian    ,
    output  wire                                  o_irq           ,
    input   wire                                  i_s_hsel        ,
    input   wire   [16:0]                         i_s_haddr       ,
    input   wire                                  i_s_hwrite      ,
    input   wire   [2:0]                          i_s_hsize       ,
    input   wire   [2:0]                          i_s_hburst      ,
    input   wire   [3:0]                          i_s_hprot       ,
    input   wire   [1:0]                          i_s_htrans      ,
    input   wire                                  i_s_hmastlock   ,
    input   wire                                  i_s_hready      ,
    input   wire   [32-1:0]                       i_s_hwdata      ,
    output  wire                                  o_s_hreadyout   ,
    output  wire                                  o_s_hresp       ,
    output  wire   [32-1:0]                       o_s_hrdata      ,

    output  wire                                  o_ram0_wren     ,
    output  wire   [8:0]                          o_ram0_waddr    ,
    output  wire   [P_RAM_WIDTH-1:0]              o_ram0_wdata    ,
    output  wire                                  o_ram0_rden     ,
    output  wire   [8:0]                          o_ram0_raddr    ,
    input   wire   [P_RAM_WIDTH-1:0]              i_ram0_rdata    ,
    output  wire                                  o_ram1_wren     ,
    output  wire   [8:0]                          o_ram1_waddr    ,
    output  wire   [P_RAM_WIDTH-1:0]              o_ram1_wdata    ,
    output  wire                                  o_ram1_rden     ,
    output  wire   [8:0]                          o_ram1_raddr    ,
    input   wire   [P_RAM_WIDTH-1:0]              i_ram1_rdata    ,
    output  wire                                  o_ram2_wren     ,
    output  wire   [8:0]                          o_ram2_waddr    ,
    output  wire   [P_RAM_WIDTH-1:0]              o_ram2_wdata    ,
    output  wire                                  o_ram2_rden     ,
    output  wire   [8:0]                          o_ram2_raddr    ,
    input   wire   [P_RAM_WIDTH-1:0]              i_ram2_rdata    ,
    output  wire                                  o_ram3_wren     ,
    output  wire   [8:0]                          o_ram3_waddr    ,
    output  wire   [P_RAM_WIDTH-1:0]              o_ram3_wdata    ,
    output  wire                                  o_ram3_rden     ,
    output  wire   [8:0]                          o_ram3_raddr    ,
    input   wire   [P_RAM_WIDTH-1:0]              i_ram3_rdata    ,

    input   wire   [32-1:0]                       i_ecc_ck_en     ,
    input   wire   [32-1:0]                       i_ecc_tm_en     ,
    input   wire   [40-1:0]                       i_ecc_chkbits   ,
    output  wire   [3:0]                          o_ecc_dec_sec   ,
    output  wire   [3:0]                          o_ecc_dec_ded   ,
    output  wire   [36-1:0]                       o_ecc_err_addr
);

localparam P_CDC_EN = 0 ;
localparam P_ECC_ENC_DIN_DW    = 81;
localparam P_ECC_ENC_CHKBIT_DW = 8 ;
localparam P_ECC_ENC_DOUT_DW   = 89;
localparam P_ECC_DEC_DIN_DW    = 89;
localparam P_ECC_DEC_DOUT_DW   = 81;

localparam P_SECURE_ALG_EN     = 0;

localparam P_PKE_HP_SM9_EN     = 0;

localparam        PKE_UHP_ECC_ALG_DP_RAM_p_AHB_ADDR_WIDTH = 17;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_AHB_DATA_WIDTH = 32;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_RAM_CIPHER_EN  = P_SECURE_ALG_EN;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_CORE_NUM       = 1;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_CDC_EN         = P_CDC_EN;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_ECC_EN         = 1;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_SM9_EN         = 0;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_SECURE_ALG_EN  = P_SECURE_ALG_EN;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_MUL_DWIDTH     = 256;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_RAM_DATA_WIDTH = 256;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_RAM_ADDR_WIDTH = 6;
localparam        PKE_UHP_ECC_ALG_DP_RAM_p_RAM_DBYTE_NUM  = 32;
localparam [15:0] PKE_UHP_ECC_ALG_DP_RAM_p_PRO_NUM        = 16'h004c;
localparam [3:0]  PKE_UHP_ECC_ALG_DP_RAM_p_MAR_VER        = 4'h1;
localparam [3:0]  PKE_UHP_ECC_ALG_DP_RAM_p_MIR_VER        = 4'h0;

localparam        PKE_UHP_ECC_ALG_p_AHB_ADDR_WIDTH = 20; 
localparam        PKE_UHP_ECC_ALG_p_AHB_DATA_WIDTH = 32; 
localparam        PKE_UHP_ECC_ALG_p_CDC_EN         = P_CDC_EN; 
localparam        PKE_UHP_ECC_ALG_p_ECC_EN         = 1; 
localparam        PKE_UHP_ECC_ALG_p_SECURE_ALG_EN  = P_SECURE_ALG_EN; 
localparam        PKE_UHP_ECC_ALG_p_MUL_DWIDTH     = 256; 
localparam [15:0] PKE_UHP_ECC_ALG_p_PRO_NUM        = 16'h004C; 
localparam [3:0]  PKE_UHP_ECC_ALG_p_MAR_VER        = 4'h1; 
localparam [3:0]  PKE_UHP_ECC_ALG_p_MIR_VER        = 4'h0; 

localparam        PKE_UHP_ALG_DP_RAM_p_AHB_ADDR_WIDTH = 13;
localparam        PKE_UHP_ALG_DP_RAM_p_AHB_DATA_WIDTH = 32;
localparam        PKE_UHP_ALG_DP_RAM_p_PRIME_LEN      = 4;
localparam        PKE_UHP_ALG_DP_RAM_p_RAM_CIPHER_EN  = P_SECURE_ALG_EN;
localparam        PKE_UHP_ALG_DP_RAM_p_CORE_NUM       = 1;
localparam        PKE_UHP_ALG_DP_RAM_p_CDC_EN         = P_CDC_EN;
localparam        PKE_UHP_ALG_DP_RAM_p_SM9_EN         = 0;
localparam        PKE_UHP_ALG_DP_RAM_p_SECURE_ALG_EN  = P_SECURE_ALG_EN;
localparam        PKE_UHP_ALG_DP_RAM_p_MUL_DWIDTH     = 256;
localparam        PKE_UHP_ALG_DP_RAM_p_RAM_DATA_WIDTH = 256;
localparam        PKE_UHP_ALG_DP_RAM_p_RAM_ADDR_WIDTH = 7;
localparam        PKE_UHP_ALG_DP_RAM_p_RAM_DBYTE_NUM  = 32;
localparam        PKE_UHP_ALG_DP_RAM_p_ROUND_NUM      = 2;
localparam        PKE_UHP_ALG_DP_RAM_p_RAM_INIT_EN    = 1;
localparam [15:0] PKE_UHP_ALG_DP_RAM_p_PRO_NUM        = 16'h0029;
localparam [3:0]  PKE_UHP_ALG_DP_RAM_p_MAR_VER        = 4'h1;
localparam [3:0]  PKE_UHP_ALG_DP_RAM_p_MIR_VER        = 4'h0;

localparam        PKE_HP_ALG_DP_RAM_p_AHB_ADDR_WIDTH = 13;
localparam        PKE_HP_ALG_DP_RAM_p_AHB_DATA_WIDTH = 32;
localparam        PKE_HP_ALG_DP_RAM_p_PRIME_LEN      = 4;
localparam        PKE_HP_ALG_DP_RAM_p_RAM_CIPHER_EN  = P_SECURE_ALG_EN;
localparam        PKE_HP_ALG_DP_RAM_p_CDC_EN         = P_CDC_EN;
localparam        PKE_HP_ALG_DP_RAM_p_SECURE_ALG_EN  = P_SECURE_ALG_EN || P_PKE_HP_SM9_EN;
localparam        PKE_HP_ALG_DP_RAM_p_RAM_DATA_WIDTH = 64;
localparam        PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH = 9;
localparam        PKE_HP_ALG_DP_RAM_p_MAX_DWIDTH     = 4096;
localparam        PKE_HP_ALG_DP_RAM_p_MEM_ECC_EN     = 1'b0;
localparam        PKE_HP_ALG_DP_RAM_p_RAM_WIDTH      = 64;
localparam        PKE_HP_ALG_DP_RAM_p_ROUND_NUM      = 4;
localparam        PKE_HP_ALG_DP_RAM_p_SM9_EN         = P_PKE_HP_SM9_EN;
localparam [15:0] PKE_HP_ALG_DP_RAM_p_PRO_NUM        = 16'h001C;
localparam [3:0]  PKE_HP_ALG_DP_RAM_p_MAR_VER        = 4'h1;
localparam [3:0]  PKE_HP_ALG_DP_RAM_p_MIR_VER        = 4'h0;

localparam        PKE_HP_ALG_SP_RAM_p_AHB_ADDR_WIDTH = 13;
localparam        PKE_HP_ALG_SP_RAM_p_AHB_DATA_WIDTH = 32;
localparam        PKE_HP_ALG_SP_RAM_p_PRIME_LEN      = 4;
localparam        PKE_HP_ALG_SP_RAM_p_RAM_CIPHER_EN  = P_SECURE_ALG_EN;
localparam        PKE_HP_ALG_SP_RAM_p_CDC_EN         = P_CDC_EN;
localparam        PKE_HP_ALG_SP_RAM_p_SECURE_ALG_EN  = P_SECURE_ALG_EN || P_PKE_HP_SM9_EN;
localparam        PKE_HP_ALG_SP_RAM_p_RAM_DATA_WIDTH = 64;
localparam        PKE_HP_ALG_SP_RAM_p_RAM_ADDR_WIDTH = 9;
localparam        PKE_HP_ALG_SP_RAM_p_MAX_DWIDTH     = 4096;
localparam        PKE_HP_ALG_SP_RAM_p_MEM_ECC_EN     = 1'b0;
localparam        PKE_HP_ALG_SP_RAM_p_RAM_WIDTH      = 64;
localparam        PKE_HP_ALG_SP_RAM_p_ROUND_NUM      = 4;
localparam        PKE_HP_ALG_SP_RAM_p_SM9_EN         = P_PKE_HP_SM9_EN;
localparam [15:0] PKE_HP_ALG_SP_RAM_p_PRO_NUM        = 16'h001C;
localparam [3:0]  PKE_HP_ALG_SP_RAM_p_MAR_VER        = 4'h1;
localparam [3:0]  PKE_HP_ALG_SP_RAM_p_MIR_VER        = 4'h0;

localparam          PKE_LP_ALG_DP_RAM_p_AHB_ADDR_WIDTH   = 13        ;
localparam          PKE_LP_ALG_DP_RAM_p_AHB_DATA_WIDTH   = 32        ;
localparam          PKE_LP_ALG_DP_RAM_p_PRIME_LEN        = 4         ;
localparam          PKE_LP_ALG_DP_RAM_p_RAM_DATA_WIDTH   = 32        ;
localparam          PKE_LP_ALG_DP_RAM_p_RAM_ADDR_WIDTH   = 13        ;
localparam [12-1:0] PKE_LP_ALG_DP_RAM_p_BASE_ADDR        = 12'h300   ;
localparam          PKE_LP_ALG_DP_RAM_p_RAM_O_ADDR_WIDTH = 11        ;
localparam [15:0]   PKE_LP_ALG_DP_RAM_p_PRO_NUM          = 16'h0006  ;
localparam [3:0]    PKE_LP_ALG_DP_RAM_p_MAR_VER          = 4'h1      ;
localparam [3:0]    PKE_LP_ALG_DP_RAM_p_MIR_VER          = 4'h0      ;

localparam        PKE_LP_ALG_SP_RAM_p_AHB_ADDR_WIDTH   = 13        ;
localparam        PKE_LP_ALG_SP_RAM_p_AHB_DATA_WIDTH   = 32        ;
localparam        PKE_LP_ALG_SP_RAM_p_PRIME_LEN        = 4         ;
localparam        PKE_LP_ALG_SP_RAM_p_RAM_DATA_WIDTH   = 32        ;
localparam        PKE_LP_ALG_SP_RAM_p_RAM_ADDR_WIDTH   = 13        ;
localparam [15:0] PKE_LP_ALG_SP_RAM_p_BASE_ADDR        = 16'h300   ;
localparam        PKE_LP_ALG_SP_RAM_p_RAM_O_ADDR_WIDTH = 11        ;
localparam [15:0] PKE_LP_ALG_SP_RAM_p_PRO_NUM          = 16'hC406  ;
localparam [3:0]  PKE_LP_ALG_SP_RAM_p_MAR_VER          = 4'h1      ;
localparam [3:0]  PKE_LP_ALG_SP_RAM_p_MIR_VER          = 4'h0      ;

localparam        PKE_LP_SE_ALG_p_RAM_CIPHER_EN    = 1         ;
localparam        PKE_LP_SE_ALG_p_AHB_ADDR_WIDTH   = 13        ;
localparam        PKE_LP_SE_ALG_p_AHB_DATA_WIDTH   = 32        ;
localparam        PKE_LP_SE_ALG_p_PRIME_LEN        = 4         ;
localparam        PKE_LP_SE_ALG_p_RAM_DATA_WIDTH   = 32        ;
localparam        PKE_LP_SE_ALG_p_RAM_ADDR_WIDTH   = 13        ;
localparam [15:0] PKE_LP_SE_ALG_p_BASE_ADDR        = 16'h0308  ;
localparam        PKE_LP_SE_ALG_p_RAM_O_ADDR_WIDTH = 11        ;
localparam [15:0] PKE_LP_SE_ALG_p_PRO_NUM          = 16'h0097  ;
localparam [3:0]  PKE_LP_SE_ALG_p_MAR_VER          = 4'h1      ;
localparam [3:0]  PKE_LP_SE_ALG_p_MIR_VER          = 4'h0      ;
localparam        PKE_LP_SE_ALG_p_ROUND_NUM        = 4         ;

localparam     MEM_INIT_PRAM_P_MEM_DATA_WIDTH = 32            ;
localparam     MEM_INIT_PRAM_P_MEM_ADDR_WIDTH = 11            ;
localparam     MEM_INIT_PRAM_P_MEM_INIT_DEEP  = 6*1024/4      ;

localparam     PKE_RAM0_ENC_WIDTH_DIN    = P_ECC_ENC_DIN_DW   ;
localparam     PKE_RAM0_ENC_WIDTH_CHKBIT = P_ECC_ENC_CHKBIT_DW;
localparam     PKE_RAM0_ENC_WIDTH_DOUT   = P_ECC_ENC_DOUT_DW  ;

localparam     PKE_RAM1_ENC_WIDTH_DIN    = P_ECC_ENC_DIN_DW   ;
localparam     PKE_RAM1_ENC_WIDTH_CHKBIT = P_ECC_ENC_CHKBIT_DW;
localparam     PKE_RAM1_ENC_WIDTH_DOUT   = P_ECC_ENC_DOUT_DW  ;

localparam     PKE_RAM2_ENC_WIDTH_DIN    = P_ECC_ENC_DIN_DW   ;
localparam     PKE_RAM2_ENC_WIDTH_CHKBIT = P_ECC_ENC_CHKBIT_DW;
localparam     PKE_RAM2_ENC_WIDTH_DOUT   = P_ECC_ENC_DOUT_DW  ;

localparam     PKE_RAM3_ENC_WIDTH_DIN    = P_ECC_ENC_DIN_DW   ;
localparam     PKE_RAM3_ENC_WIDTH_CHKBIT = P_ECC_ENC_CHKBIT_DW;
localparam     PKE_RAM3_ENC_WIDTH_DOUT   = P_ECC_ENC_DOUT_DW  ;

localparam     PKE_RAM0_DEC_WIDTH_DOUT = P_ECC_DEC_DOUT_DW  ;
localparam     PKE_RAM0_DEC_WIDTH_DIN  = P_ECC_DEC_DIN_DW   ;

localparam     PKE_RAM1_DEC_WIDTH_DOUT = P_ECC_DEC_DOUT_DW  ;
localparam     PKE_RAM1_DEC_WIDTH_DIN  = P_ECC_DEC_DIN_DW   ;

localparam     PKE_RAM2_DEC_WIDTH_DOUT = P_ECC_DEC_DOUT_DW  ;
localparam     PKE_RAM2_DEC_WIDTH_DIN  = P_ECC_DEC_DIN_DW   ;

localparam     PKE_RAM3_DEC_WIDTH_DOUT = P_ECC_DEC_DOUT_DW  ;
localparam     PKE_RAM3_DEC_WIDTH_DIN  = P_ECC_DEC_DIN_DW   ;

wire   [3:0]                          pke_ecc_dec_sec   ;
wire   [3:0]                          pke_ecc_dec_ded   ;
wire   [36-1:0]                       pke_ecc_err_addr  ;

wire                                  pke_ram0_wren     ;
wire   [8:0]                          pke_ram0_waddr    ;
wire   [P_RAM_WIDTH-1:0]              pke_ram0_wdata    ;
wire                                  pke_ram0_rden     ;
wire   [8:0]                          pke_ram0_raddr    ;
wire                                  pke_ram1_wren     ;
wire   [8:0]                          pke_ram1_waddr    ;
wire   [P_RAM_WIDTH-1:0]              pke_ram1_wdata    ;
wire                                  pke_ram1_rden     ;
wire   [8:0]                          pke_ram1_raddr    ;
wire                                  pke_ram2_wren     ;
wire   [8:0]                          pke_ram2_waddr    ;
wire   [P_RAM_WIDTH-1:0]              pke_ram2_wdata    ;
wire                                  pke_ram2_rden     ;
wire   [8:0]                          pke_ram2_raddr    ;
wire                                  pke_ram3_wren     ;
wire   [8:0]                          pke_ram3_waddr    ;
wire   [P_RAM_WIDTH-1:0]              pke_ram3_wdata    ;
wire                                  pke_ram3_rden     ;
wire   [8:0]                          pke_ram3_raddr    ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]pke_i_ram0_rdata;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]pke_i_ram1_rdata;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]pke_i_ram2_rdata;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]pke_i_ram3_rdata;

wire                                  pke_o_irq         ;
wire                                  pke_o_s_hreadyout ;
wire                                  pke_o_s_hresp     ;
wire  [31:0]                          pke_o_s_hrdata    ;

reg [8:0]       ram0_raddr       ;
reg [8:0]       ram1_raddr       ;
reg [8:0]       ram2_raddr       ;
reg [8:0]       ram3_raddr       ;
reg [8:0]       ram0_raddr_buf   ;
reg [8:0]       ram1_raddr_buf   ;
reg [8:0]       ram2_raddr_buf   ;
reg [8:0]       ram3_raddr_buf   ;
reg [3:0]       err_flg_vail     ;
reg [3:0]       err_flg_vail_buf ;
reg [3:0]       ecc_dec_sec      ;
reg [3:0]       ecc_dec_ded      ;




wire                                             pke_hp_alg_dp_ram_o_irq         ;
wire                                             pke_hp_alg_dp_ram_o_s_hreadyout ;
wire                                             pke_hp_alg_dp_ram_o_s_hresp     ;
wire [PKE_HP_ALG_DP_RAM_p_AHB_DATA_WIDTH-1:0]    pke_hp_alg_dp_ram_o_s_hrdata    ;
wire                                             pke_hp_alg_dp_ram_o_ram0_wren   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram0_waddr  ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_o_ram0_wdata  ;
wire                                             pke_hp_alg_dp_ram_o_ram0_rden   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram0_raddr  ;
wire                                             pke_hp_alg_dp_ram_o_ram1_wren   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram1_waddr  ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_o_ram1_wdata  ;
wire                                             pke_hp_alg_dp_ram_o_ram1_rden   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram1_raddr  ;
wire                                             pke_hp_alg_dp_ram_o_ram2_wren   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram2_waddr  ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_o_ram2_wdata  ;
wire                                             pke_hp_alg_dp_ram_o_ram2_rden   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram2_raddr  ;
wire                                             pke_hp_alg_dp_ram_o_ram3_wren   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram3_waddr  ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_o_ram3_wdata  ;
wire                                             pke_hp_alg_dp_ram_o_ram3_rden   ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_o_ram3_raddr  ;






wire [PKE_RAM0_ENC_WIDTH_DOUT-1:0]      pke_ram0_enc_encode_dout     ;

wire [PKE_RAM1_ENC_WIDTH_DOUT-1:0]      pke_ram1_enc_encode_dout     ;

wire [PKE_RAM2_ENC_WIDTH_DOUT-1:0]      pke_ram2_enc_encode_dout     ;

wire [PKE_RAM3_ENC_WIDTH_DOUT-1:0]      pke_ram3_enc_encode_dout     ;

wire                                  pke_ram0_dec_ecc_sec     ;
wire                                  pke_ram0_dec_ecc_ded     ;
wire [PKE_RAM0_DEC_WIDTH_DOUT-1:0]    pke_ram0_dec_decode_dout ;

wire                                  pke_ram1_dec_ecc_sec     ;
wire                                  pke_ram1_dec_ecc_ded     ;
wire [PKE_RAM1_DEC_WIDTH_DOUT-1:0]    pke_ram1_dec_decode_dout ;

wire                                  pke_ram2_dec_ecc_sec     ;
wire                                  pke_ram2_dec_ecc_ded     ;
wire [PKE_RAM2_DEC_WIDTH_DOUT-1:0]    pke_ram2_dec_decode_dout ;

wire                                  pke_ram3_dec_ecc_sec     ;
wire                                  pke_ram3_dec_ecc_ded     ;
wire [PKE_RAM3_DEC_WIDTH_DOUT-1:0]    pke_ram3_dec_decode_dout ;

wire [8:0] ram0_raddr_next = pke_hp_alg_dp_ram_o_ram0_raddr;
wire [8:0] ram1_raddr_next = pke_hp_alg_dp_ram_o_ram1_raddr;
wire [8:0] ram2_raddr_next = pke_hp_alg_dp_ram_o_ram2_raddr;
wire [8:0] ram3_raddr_next = pke_hp_alg_dp_ram_o_ram3_raddr;
wire [8:0] ram0_raddr_buf_next = ram0_raddr;
wire [8:0] ram1_raddr_buf_next = ram1_raddr;
wire [8:0] ram2_raddr_buf_next = ram2_raddr;
wire [8:0] ram3_raddr_buf_next = ram3_raddr;
wire [3:0] err_flg_vail_next = {pke_hp_alg_dp_ram_o_ram3_rden,pke_hp_alg_dp_ram_o_ram2_rden,pke_hp_alg_dp_ram_o_ram1_rden,pke_hp_alg_dp_ram_o_ram0_rden};
wire [3:0] err_flg_vail_buf_next = err_flg_vail;
wire [3:0] ecc_dec_sec_next = {pke_ram3_dec_ecc_sec,pke_ram2_dec_ecc_sec,pke_ram1_dec_ecc_sec,pke_ram0_dec_ecc_sec};
wire [3:0] ecc_dec_ded_next = {pke_ram3_dec_ecc_ded,pke_ram2_dec_ecc_ded,pke_ram1_dec_ecc_ded,pke_ram0_dec_ecc_ded};

assign pke_ecc_dec_sec    = ecc_dec_sec & err_flg_vail_buf;
assign pke_ecc_dec_ded    = ecc_dec_ded & err_flg_vail_buf;
assign pke_ecc_err_addr   = {ram3_raddr_buf,ram2_raddr_buf,ram1_raddr_buf,ram0_raddr_buf};
assign pke_ram0_wren      = pke_hp_alg_dp_ram_o_ram0_wren;
assign pke_ram0_waddr     = pke_hp_alg_dp_ram_o_ram0_waddr;
assign pke_ram0_wdata     = pke_ram0_enc_encode_dout[88:17];
assign pke_ram0_rden      = pke_hp_alg_dp_ram_o_ram0_rden;
assign pke_ram0_raddr     = pke_hp_alg_dp_ram_o_ram0_raddr;
assign pke_ram1_wren      = pke_hp_alg_dp_ram_o_ram1_wren;
assign pke_ram1_waddr     = pke_hp_alg_dp_ram_o_ram1_waddr;
assign pke_ram1_wdata     = pke_ram1_enc_encode_dout[88:17];
assign pke_ram1_rden      = pke_hp_alg_dp_ram_o_ram1_rden;
assign pke_ram1_raddr     = pke_hp_alg_dp_ram_o_ram1_raddr;
assign pke_ram2_wren      = pke_hp_alg_dp_ram_o_ram2_wren;
assign pke_ram2_waddr     = pke_hp_alg_dp_ram_o_ram2_waddr;
assign pke_ram2_wdata     = pke_ram2_enc_encode_dout[88:17];
assign pke_ram2_rden      = pke_hp_alg_dp_ram_o_ram2_rden;
assign pke_ram2_raddr     = pke_hp_alg_dp_ram_o_ram2_raddr;
assign pke_ram3_wren      = pke_hp_alg_dp_ram_o_ram3_wren;
assign pke_ram3_waddr     = pke_hp_alg_dp_ram_o_ram3_waddr;
assign pke_ram3_wdata     = pke_ram3_enc_encode_dout[88:17];
assign pke_ram3_rden      = pke_hp_alg_dp_ram_o_ram3_rden;
assign pke_ram3_raddr     = pke_hp_alg_dp_ram_o_ram3_raddr;
assign pke_i_ram0_rdata   = pke_ram0_dec_decode_dout[80:17];
assign pke_i_ram1_rdata   = pke_ram1_dec_decode_dout[80:17];
assign pke_i_ram2_rdata   = pke_ram2_dec_decode_dout[80:17];
assign pke_i_ram3_rdata   = pke_ram3_dec_decode_dout[80:17];

assign pke_o_irq         = pke_hp_alg_dp_ram_o_irq             ;
assign pke_o_s_hreadyout = pke_hp_alg_dp_ram_o_s_hreadyout     ;
assign pke_o_s_hresp     = pke_hp_alg_dp_ram_o_s_hresp         ;
assign pke_o_s_hrdata    = pke_hp_alg_dp_ram_o_s_hrdata        ;

wire                                             pke_hp_alg_dp_ram_clk_ahb       = i_clk_ahb     ;
wire                                             pke_hp_alg_dp_ram_rst_n_ahb     = i_rst_n_ahb   ;
wire                                             pke_hp_alg_dp_ram_clk_core      = i_clk_core    ;
wire                                             pke_hp_alg_dp_ram_rst_n_core    = i_rst_n_core  ;
wire                                             pke_hp_alg_dp_ram_i_ahb_endian  = i_ahb_endian  ;
wire                                             pke_hp_alg_dp_ram_i_s_hsel      = i_s_hsel      ;
wire [PKE_HP_ALG_DP_RAM_p_AHB_ADDR_WIDTH-1:0]    pke_hp_alg_dp_ram_i_s_haddr     = i_s_haddr[12:0];
wire                                             pke_hp_alg_dp_ram_i_s_hwrite    = i_s_hwrite    ;
wire [2:0]                                       pke_hp_alg_dp_ram_i_s_hsize     = i_s_hsize     ;
wire [2:0]                                       pke_hp_alg_dp_ram_i_s_hburst    = i_s_hburst    ;
wire [3:0]                                       pke_hp_alg_dp_ram_i_s_hprot     = i_s_hprot     ;
wire [1:0]                                       pke_hp_alg_dp_ram_i_s_htrans    = i_s_htrans    ;
wire                                             pke_hp_alg_dp_ram_i_s_hmastlock = i_s_hmastlock ;
wire                                             pke_hp_alg_dp_ram_i_s_hready    = i_s_hready    ;
wire [PKE_HP_ALG_DP_RAM_p_AHB_DATA_WIDTH-1:0]    pke_hp_alg_dp_ram_i_s_hwdata    = i_s_hwdata    ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_i_ram0_rdata  = pke_i_ram0_rdata ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_i_ram1_rdata  = pke_i_ram1_rdata ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_i_ram2_rdata  = pke_i_ram2_rdata ;
wire [PKE_HP_ALG_DP_RAM_p_RAM_WIDTH-1:0]         pke_hp_alg_dp_ram_i_ram3_rdata  = pke_i_ram3_rdata ;

wire [PKE_RAM0_ENC_WIDTH_DIN-1:0]       pke_ram0_enc_encode_din      = {pke_hp_alg_dp_ram_o_ram0_wdata,8'b0,pke_hp_alg_dp_ram_o_ram0_waddr};
wire [PKE_RAM0_ENC_WIDTH_CHKBIT-1:0]    pke_ram0_enc_tm_chkbits      = i_ecc_chkbits[7:0];
wire [7:0]                              pke_ram0_enc_tm_sel_ecc_code = i_ecc_tm_en[7:0];

wire [PKE_RAM1_ENC_WIDTH_DIN-1:0]       pke_ram1_enc_encode_din      = {pke_hp_alg_dp_ram_o_ram1_wdata,8'b0,pke_hp_alg_dp_ram_o_ram1_waddr};
wire [PKE_RAM1_ENC_WIDTH_CHKBIT-1:0]    pke_ram1_enc_tm_chkbits      = i_ecc_chkbits[17:10];
wire [7:0]                              pke_ram1_enc_tm_sel_ecc_code = i_ecc_tm_en[15:8];

wire [PKE_RAM2_ENC_WIDTH_DIN-1:0]       pke_ram2_enc_encode_din      = {pke_hp_alg_dp_ram_o_ram2_wdata,8'b0,pke_hp_alg_dp_ram_o_ram2_waddr};
wire [PKE_RAM2_ENC_WIDTH_CHKBIT-1:0]    pke_ram2_enc_tm_chkbits      = i_ecc_chkbits[27:20];
wire [7:0]                              pke_ram2_enc_tm_sel_ecc_code = i_ecc_tm_en[23:16];

wire [PKE_RAM3_ENC_WIDTH_DIN-1:0]       pke_ram3_enc_encode_din      = {pke_hp_alg_dp_ram_o_ram3_wdata,8'b0,pke_hp_alg_dp_ram_o_ram3_waddr};
wire [PKE_RAM3_ENC_WIDTH_CHKBIT-1:0]    pke_ram3_enc_tm_chkbits      = i_ecc_chkbits[37:30];
wire [7:0]                              pke_ram3_enc_tm_sel_ecc_code = i_ecc_tm_en[31:24];

wire [PKE_RAM0_DEC_WIDTH_DIN-1:0]     pke_ram0_dec_decode_din  = {i_ram0_rdata,8'b0,ram0_raddr};
wire [7:0]                            pke_ram0_dec_ecc_en      = i_ecc_ck_en[7:0];
wire [PKE_RAM0_DEC_WIDTH_DIN-1:0]     pke_ram0_dec_xor_dedin   = 89'h0;

wire [PKE_RAM1_DEC_WIDTH_DIN-1:0]     pke_ram1_dec_decode_din  = {i_ram1_rdata,8'b0,ram1_raddr};
wire [7:0]                            pke_ram1_dec_ecc_en      = i_ecc_ck_en[15:8];
wire [PKE_RAM1_DEC_WIDTH_DIN-1:0]     pke_ram1_dec_xor_dedin   = 89'h0;

wire [PKE_RAM2_DEC_WIDTH_DIN-1:0]     pke_ram2_dec_decode_din  = {i_ram2_rdata,8'b0,ram2_raddr};
wire [7:0]                            pke_ram2_dec_ecc_en      = i_ecc_ck_en[23:16];
wire [PKE_RAM2_DEC_WIDTH_DIN-1:0]     pke_ram2_dec_xor_dedin   = 89'h0;

wire [PKE_RAM3_DEC_WIDTH_DIN-1:0]     pke_ram3_dec_decode_din  = {i_ram3_rdata,8'b0,ram3_raddr};
wire [7:0]                            pke_ram3_dec_ecc_en      = i_ecc_ck_en[31:24];
wire [PKE_RAM3_DEC_WIDTH_DIN-1:0]     pke_ram3_dec_xor_dedin   = 89'h0;

assign o_irq              = pke_o_irq;
assign o_s_hreadyout      = pke_o_s_hreadyout;
assign o_s_hresp          = pke_o_s_hresp;
assign o_s_hrdata         = pke_o_s_hrdata;

assign o_ram0_wren        = pke_ram0_wren;
assign o_ram0_waddr       = pke_ram0_waddr;
assign o_ram0_wdata       = pke_ram0_wdata;
assign o_ram0_rden        = pke_ram0_rden;
assign o_ram0_raddr       = pke_ram0_raddr;
assign o_ram1_wren        = pke_ram1_wren;
assign o_ram1_waddr       = pke_ram1_waddr;
assign o_ram1_wdata       = pke_ram1_wdata;
assign o_ram1_rden        = pke_ram1_rden;
assign o_ram1_raddr       = pke_ram1_raddr;
assign o_ram2_wren        = pke_ram2_wren;
assign o_ram2_waddr       = pke_ram2_waddr;
assign o_ram2_wdata       = pke_ram2_wdata;
assign o_ram2_rden        = pke_ram2_rden;
assign o_ram2_raddr       = pke_ram2_raddr;
assign o_ram3_wren        = pke_ram3_wren;
assign o_ram3_waddr       = pke_ram3_waddr;
assign o_ram3_wdata       = pke_ram3_wdata;
assign o_ram3_rden        = pke_ram3_rden;
assign o_ram3_raddr       = pke_ram3_raddr;

assign o_ecc_dec_sec      = pke_ecc_dec_sec;
assign o_ecc_dec_ded      = pke_ecc_dec_ded;
assign o_ecc_err_addr     = pke_ecc_err_addr;

osr_pke_hp_wrapper #(
    .p_AHB_ADDR_WIDTH     ( PKE_HP_ALG_DP_RAM_p_AHB_ADDR_WIDTH ), 
    .p_AHB_DATA_WIDTH     ( PKE_HP_ALG_DP_RAM_p_AHB_DATA_WIDTH ), 
    .p_PRIME_LEN          ( PKE_HP_ALG_DP_RAM_p_PRIME_LEN      ), 
    .p_RAM_CIPHER_EN      ( PKE_HP_ALG_DP_RAM_p_RAM_CIPHER_EN  ), 
    .p_CDC_EN             ( PKE_HP_ALG_DP_RAM_p_CDC_EN         ), 
    .p_SECURE_ALG_EN      ( PKE_HP_ALG_DP_RAM_p_SECURE_ALG_EN  ), 
    .p_RAM_DATA_WIDTH     ( PKE_HP_ALG_DP_RAM_p_RAM_DATA_WIDTH ), 
    .p_RAM_ADDR_WIDTH     ( PKE_HP_ALG_DP_RAM_p_RAM_ADDR_WIDTH ), 
    .p_MAX_DWIDTH         ( PKE_HP_ALG_DP_RAM_p_MAX_DWIDTH     ), 
    .p_MEM_ECC_EN         ( PKE_HP_ALG_DP_RAM_p_MEM_ECC_EN     ), 
    .p_RAM_WIDTH          ( PKE_HP_ALG_DP_RAM_p_RAM_WIDTH      ), 
    .p_ROUND_NUM          ( PKE_HP_ALG_DP_RAM_p_ROUND_NUM      ), 
    .p_SM9_EN             ( PKE_HP_ALG_DP_RAM_p_SM9_EN         ), 
    .p_PRO_NUM            ( PKE_HP_ALG_DP_RAM_p_PRO_NUM        ), 
    .p_MAR_VER            ( PKE_HP_ALG_DP_RAM_p_MAR_VER        ), 
    .p_MIR_VER            ( PKE_HP_ALG_DP_RAM_p_MIR_VER        )  
    ) u_osr_pke_hp_top_dp_ram (
    .clk_ahb                        ( pke_hp_alg_dp_ram_clk_ahb       ), 
    .rst_n_ahb                      ( pke_hp_alg_dp_ram_rst_n_ahb     ), 
    .clk_core                       ( pke_hp_alg_dp_ram_clk_core      ), 
    .rst_n_core                     ( pke_hp_alg_dp_ram_rst_n_core    ), 
    .i_ahb_endian                   ( pke_hp_alg_dp_ram_i_ahb_endian  ), 
    .o_irq                          ( pke_hp_alg_dp_ram_o_irq         ), 
    .i_s_hsel                       ( pke_hp_alg_dp_ram_i_s_hsel      ), 
    .i_s_haddr                      ( pke_hp_alg_dp_ram_i_s_haddr     ), 
    .i_s_hwrite                     ( pke_hp_alg_dp_ram_i_s_hwrite    ), 
    .i_s_hsize                      ( pke_hp_alg_dp_ram_i_s_hsize     ), 
    .i_s_hburst                     ( pke_hp_alg_dp_ram_i_s_hburst    ), 
    .i_s_hprot                      ( pke_hp_alg_dp_ram_i_s_hprot     ), 
    .i_s_htrans                     ( pke_hp_alg_dp_ram_i_s_htrans    ), 
    .i_s_hmastlock                  ( pke_hp_alg_dp_ram_i_s_hmastlock ), 
    .i_s_hready                     ( pke_hp_alg_dp_ram_i_s_hready    ), 
    .i_s_hwdata                     ( pke_hp_alg_dp_ram_i_s_hwdata    ), 
    .o_s_hreadyout                  ( pke_hp_alg_dp_ram_o_s_hreadyout ), 
    .o_s_hresp                      ( pke_hp_alg_dp_ram_o_s_hresp     ), 
    .o_s_hrdata                     ( pke_hp_alg_dp_ram_o_s_hrdata    ), 
    .o_ram0_wren                    ( pke_hp_alg_dp_ram_o_ram0_wren   ), 
    .o_ram0_waddr                   ( pke_hp_alg_dp_ram_o_ram0_waddr  ), 
    .o_ram0_wdata                   ( pke_hp_alg_dp_ram_o_ram0_wdata  ), 
    .o_ram0_rden                    ( pke_hp_alg_dp_ram_o_ram0_rden   ), 
    .o_ram0_raddr                   ( pke_hp_alg_dp_ram_o_ram0_raddr  ), 
    .i_ram0_rdata                   ( pke_hp_alg_dp_ram_i_ram0_rdata  ), 
    .o_ram1_wren                    ( pke_hp_alg_dp_ram_o_ram1_wren   ), 
    .o_ram1_waddr                   ( pke_hp_alg_dp_ram_o_ram1_waddr  ), 
    .o_ram1_wdata                   ( pke_hp_alg_dp_ram_o_ram1_wdata  ), 
    .o_ram1_rden                    ( pke_hp_alg_dp_ram_o_ram1_rden   ), 
    .o_ram1_raddr                   ( pke_hp_alg_dp_ram_o_ram1_raddr  ), 
    .i_ram1_rdata                   ( pke_hp_alg_dp_ram_i_ram1_rdata  ), 
    .o_ram2_wren                    ( pke_hp_alg_dp_ram_o_ram2_wren   ), 
    .o_ram2_waddr                   ( pke_hp_alg_dp_ram_o_ram2_waddr  ), 
    .o_ram2_wdata                   ( pke_hp_alg_dp_ram_o_ram2_wdata  ), 
    .o_ram2_rden                    ( pke_hp_alg_dp_ram_o_ram2_rden   ), 
    .o_ram2_raddr                   ( pke_hp_alg_dp_ram_o_ram2_raddr  ), 
    .i_ram2_rdata                   ( pke_hp_alg_dp_ram_i_ram2_rdata  ), 
    .o_ram3_wren                    ( pke_hp_alg_dp_ram_o_ram3_wren   ), 
    .o_ram3_waddr                   ( pke_hp_alg_dp_ram_o_ram3_waddr  ), 
    .o_ram3_wdata                   ( pke_hp_alg_dp_ram_o_ram3_wdata  ), 
    .o_ram3_rden                    ( pke_hp_alg_dp_ram_o_ram3_rden   ), 
    .o_ram3_raddr                   ( pke_hp_alg_dp_ram_o_ram3_raddr  ), 
    .i_ram3_rdata                   ( pke_hp_alg_dp_ram_i_ram3_rdata  )  
    ); 

osr_ecc_encode #(
    .WIDTH_DIN        ( PKE_RAM0_ENC_WIDTH_DIN    ), 
    .WIDTH_CHKBIT     ( PKE_RAM0_ENC_WIDTH_CHKBIT ), 
    .WIDTH_DOUT       ( PKE_RAM0_ENC_WIDTH_DOUT   )  
    ) u_pke_ram0_ecc_encode (
    .encode_din                  ( pke_ram0_enc_encode_din      ), 
    .tm_chkbits                  ( pke_ram0_enc_tm_chkbits      ), 
    .tm_sel_ecc_code             ( pke_ram0_enc_tm_sel_ecc_code ), 
    .encode_dout                 ( pke_ram0_enc_encode_dout     )  
    ); 

osr_ecc_encode #(
    .WIDTH_DIN        ( PKE_RAM1_ENC_WIDTH_DIN    ), 
    .WIDTH_CHKBIT     ( PKE_RAM1_ENC_WIDTH_CHKBIT ), 
    .WIDTH_DOUT       ( PKE_RAM1_ENC_WIDTH_DOUT   )  
    ) u_pke_ram1_ecc_encode (
    .encode_din                  ( pke_ram1_enc_encode_din      ), 
    .tm_chkbits                  ( pke_ram1_enc_tm_chkbits      ), 
    .tm_sel_ecc_code             ( pke_ram1_enc_tm_sel_ecc_code ), 
    .encode_dout                 ( pke_ram1_enc_encode_dout     )  
    ); 

osr_ecc_encode #(
    .WIDTH_DIN        ( PKE_RAM2_ENC_WIDTH_DIN    ), 
    .WIDTH_CHKBIT     ( PKE_RAM2_ENC_WIDTH_CHKBIT ), 
    .WIDTH_DOUT       ( PKE_RAM2_ENC_WIDTH_DOUT   )  
    ) u_pke_ram2_ecc_encode (
    .encode_din                  ( pke_ram2_enc_encode_din      ), 
    .tm_chkbits                  ( pke_ram2_enc_tm_chkbits      ), 
    .tm_sel_ecc_code             ( pke_ram2_enc_tm_sel_ecc_code ), 
    .encode_dout                 ( pke_ram2_enc_encode_dout     )  
    ); 

osr_ecc_encode #(
    .WIDTH_DIN        ( PKE_RAM3_ENC_WIDTH_DIN    ), 
    .WIDTH_CHKBIT     ( PKE_RAM3_ENC_WIDTH_CHKBIT ), 
    .WIDTH_DOUT       ( PKE_RAM3_ENC_WIDTH_DOUT   )  
    ) u_pke_ram3_ecc_encode (
    .encode_din                  ( pke_ram3_enc_encode_din      ), 
    .tm_chkbits                  ( pke_ram3_enc_tm_chkbits      ), 
    .tm_sel_ecc_code             ( pke_ram3_enc_tm_sel_ecc_code ), 
    .encode_dout                 ( pke_ram3_enc_encode_dout     )  
    ); 

osr_ecc_decode #(
    .WIDTH_DOUT     ( PKE_RAM0_DEC_WIDTH_DOUT ), 
    .WIDTH_DIN      ( PKE_RAM0_DEC_WIDTH_DIN  )  
    ) u_pke_ram0_ecc_decode (
    .decode_din              ( pke_ram0_dec_decode_din  ), 
    .ecc_en                  ( pke_ram0_dec_ecc_en      ), 
    .xor_dedin               ( pke_ram0_dec_xor_dedin   ), 
    .ecc_sec                 ( pke_ram0_dec_ecc_sec     ), 
    .ecc_ded                 ( pke_ram0_dec_ecc_ded     ), 
    .decode_dout             ( pke_ram0_dec_decode_dout )  
    ); 

osr_ecc_decode #(
    .WIDTH_DOUT     ( PKE_RAM1_DEC_WIDTH_DOUT ), 
    .WIDTH_DIN      ( PKE_RAM1_DEC_WIDTH_DIN  )  
    ) u_pke_ram1_ecc_decode (
    .decode_din              ( pke_ram1_dec_decode_din  ), 
    .ecc_en                  ( pke_ram1_dec_ecc_en      ), 
    .xor_dedin               ( pke_ram1_dec_xor_dedin   ), 
    .ecc_sec                 ( pke_ram1_dec_ecc_sec     ), 
    .ecc_ded                 ( pke_ram1_dec_ecc_ded     ), 
    .decode_dout             ( pke_ram1_dec_decode_dout )  
    ); 

osr_ecc_decode #(
    .WIDTH_DOUT     ( PKE_RAM2_DEC_WIDTH_DOUT ), 
    .WIDTH_DIN      ( PKE_RAM2_DEC_WIDTH_DIN  )  
    ) u_pke_ram2_ecc_decode (
    .decode_din              ( pke_ram2_dec_decode_din  ), 
    .ecc_en                  ( pke_ram2_dec_ecc_en      ), 
    .xor_dedin               ( pke_ram2_dec_xor_dedin   ), 
    .ecc_sec                 ( pke_ram2_dec_ecc_sec     ), 
    .ecc_ded                 ( pke_ram2_dec_ecc_ded     ), 
    .decode_dout             ( pke_ram2_dec_decode_dout )  
    ); 

osr_ecc_decode #(
    .WIDTH_DOUT     ( PKE_RAM3_DEC_WIDTH_DOUT ), 
    .WIDTH_DIN      ( PKE_RAM3_DEC_WIDTH_DIN  )  
    ) u_pke_ram3_ecc_decode (
    .decode_din              ( pke_ram3_dec_decode_din  ), 
    .ecc_en                  ( pke_ram3_dec_ecc_en      ), 
    .xor_dedin               ( pke_ram3_dec_xor_dedin   ), 
    .ecc_sec                 ( pke_ram3_dec_ecc_sec     ), 
    .ecc_ded                 ( pke_ram3_dec_ecc_ded     ), 
    .decode_dout             ( pke_ram3_dec_decode_dout )  
    ); 

always @(posedge i_clk_ahb or negedge i_rst_n_ahb) begin
    if(!i_rst_n_ahb) begin
        ram0_raddr            <= 9'h0                  ;
        ram1_raddr            <= 9'h0                  ;
        ram2_raddr            <= 9'h0                  ;
        ram3_raddr            <= 9'h0                  ;
        ram0_raddr_buf        <= 9'h0                  ;
        ram1_raddr_buf        <= 9'h0                  ;
        ram2_raddr_buf        <= 9'h0                  ;
        ram3_raddr_buf        <= 9'h0                  ;
        err_flg_vail          <= 4'h0                  ;
        err_flg_vail_buf      <= 4'h0                  ;
        ecc_dec_sec           <= 4'h0                  ;
        ecc_dec_ded           <= 4'h0                  ;
    end else begin
        ram0_raddr            <= ram0_raddr_next       ;
        ram1_raddr            <= ram1_raddr_next       ;
        ram2_raddr            <= ram2_raddr_next       ;
        ram3_raddr            <= ram3_raddr_next       ;
        ram0_raddr_buf        <= ram0_raddr_buf_next   ;
        ram1_raddr_buf        <= ram1_raddr_buf_next   ;
        ram2_raddr_buf        <= ram2_raddr_buf_next   ;
        ram3_raddr_buf        <= ram3_raddr_buf_next   ;
        err_flg_vail          <= err_flg_vail_next     ;
        err_flg_vail_buf      <= err_flg_vail_buf_next ;
        ecc_dec_sec           <= ecc_dec_sec_next      ;
        ecc_dec_ded           <= ecc_dec_ded_next      ;
    end
end

endmodule
