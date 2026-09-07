//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_otp_ipatch #(
)(
    input   wire                  i_patch_en                ,
    input   wire                  i_patch_info_vld          ,
    input   wire [11:0]           i_patch_info_addr         ,
    input   wire [31:0]           i_patch_otp_rdata         ,

    input   wire                  i_hclk                    ,       
    input   wire                  i_hresetn                 ,       

    input   wire                  i_hsels                   ,
    input   wire [31:0]           i_haddrs                  ,
    input   wire [1:0]            i_htranss                 ,
    input   wire                  i_hwrites                 ,
    input   wire [2:0]            i_hsizes                  ,
    input   wire [2:0]            i_hbursts                 ,
    input   wire [3:0]            i_hprots                  ,
    input   wire [31:0]           i_hwdatas                 ,
    input   wire                  i_hmastlocks              ,
    input   wire                  i_hreadys                 ,
    output  wire [31:0]           o_hrdatas                 ,
    output  wire                  o_hreadyouts              ,
    output  wire                  o_hresps                  ,

    output  wire                  o_hselm                   ,
    output  wire [31:0]           o_haddrm                  ,
    output  wire [1:0]            o_htransm                 ,
    output  wire                  o_hwritem                 ,
    output  wire [2:0]            o_hsizem                  ,
    output  wire [2:0]            o_hburstm                 ,
    output  wire [3:0]            o_hprotm                  ,
    output  wire [31:0]           o_hwdatam                 ,
    output  wire                  o_hmastlockm              ,
    output  wire                  o_hreadym                 ,
    input   wire [31:0]           i_hrdatam                 ,
    input   wire                  i_hreadyoutm              ,
    input   wire                  i_hrespm                 

);

localparam     P_PATCH_CFG_0         = 12'h050 ;
localparam     P_PATCH_CFG_1         = 12'h054 ;

localparam     P_PATCH_ADDR_0        = 12'h058 ;
localparam     P_PATCH_ADDR_1        = 12'h05C ;
localparam     P_PATCH_ADDR_2        = 12'h060 ;
localparam     P_PATCH_ADDR_3        = 12'h064 ;
localparam     P_PATCH_ADDR_4        = 12'h068 ;
localparam     P_PATCH_ADDR_5        = 12'h06C ;
localparam     P_PATCH_ADDR_6        = 12'h070 ;
localparam     P_PATCH_ADDR_7        = 12'h074 ;
localparam     P_PATCH_ADDR_8        = 12'h078 ;
localparam     P_PATCH_ADDR_9        = 12'h07C ;
localparam     P_PATCH_ADDR_10       = 12'h080 ;
localparam     P_PATCH_ADDR_11       = 12'h084 ;
localparam     P_PATCH_ADDR_12       = 12'h088 ;
localparam     P_PATCH_ADDR_13       = 12'h08C ;
localparam     P_PATCH_ADDR_14       = 12'h090 ;
localparam     P_PATCH_ADDR_15       = 12'h094 ;

localparam     P_PATCH_DATA_0        = 12'h098 ;
localparam     P_PATCH_DATA_1        = 12'h09C ;
localparam     P_PATCH_DATA_2        = 12'h0A0 ;
localparam     P_PATCH_DATA_3        = 12'h0A4 ;
localparam     P_PATCH_DATA_4        = 12'h0A8 ;
localparam     P_PATCH_DATA_5        = 12'h0AC ;
localparam     P_PATCH_DATA_6        = 12'h0B0 ;
localparam     P_PATCH_DATA_7        = 12'h0B4 ;
localparam     P_PATCH_DATA_8        = 12'h0B8 ;
localparam     P_PATCH_DATA_9        = 12'h0BC ;
localparam     P_PATCH_DATA_10       = 12'h0C0 ;
localparam     P_PATCH_DATA_11       = 12'h0C4 ;
localparam     P_PATCH_DATA_12       = 12'h0C8 ;
localparam     P_PATCH_DATA_13       = 12'h0CC ;
localparam     P_PATCH_DATA_14       = 12'h0D0 ;
localparam     P_PATCH_DATA_15       = 12'h0D4 ;
localparam     P_PATCH_DATA_16       = 12'h0D8 ;
localparam     P_PATCH_DATA_17       = 12'h0DC ;
localparam     P_PATCH_DATA_18       = 12'h0E0 ;
localparam     P_PATCH_DATA_19       = 12'h0E4 ;
localparam     P_PATCH_DATA_20       = 12'h0E8 ;
localparam     P_PATCH_DATA_21       = 12'h0EC ;
localparam     P_PATCH_DATA_22       = 12'h0F0 ;
localparam     P_PATCH_DATA_23       = 12'h0F4 ;
localparam     P_PATCH_DATA_24       = 12'h0F8 ;
localparam     P_PATCH_DATA_25       = 12'h0FC ;
localparam     P_PATCH_DATA_26       = 12'h100 ;
localparam     P_PATCH_DATA_27       = 12'h104 ;
localparam     P_PATCH_DATA_28       = 12'h108 ;
localparam     P_PATCH_DATA_29       = 12'h10C ;
localparam     P_PATCH_DATA_30       = 12'h110 ;
localparam     P_PATCH_DATA_31       = 12'h114 ;

reg [31:0]      patch_cfg_0   ;
reg [31:0]      patch_cfg_1   ;
reg [15:0]      patch_addr_0  ;
reg [15:0]      patch_addr_1  ;
reg [15:0]      patch_addr_2  ;
reg [15:0]      patch_addr_3  ;
reg [15:0]      patch_addr_4  ;
reg [15:0]      patch_addr_5  ;
reg [15:0]      patch_addr_6  ;
reg [15:0]      patch_addr_7  ;
reg [15:0]      patch_addr_8  ;
reg [15:0]      patch_addr_9  ;
reg [15:0]      patch_addr_10 ;
reg [15:0]      patch_addr_11 ;
reg [15:0]      patch_addr_12 ;
reg [15:0]      patch_addr_13 ;
reg [15:0]      patch_addr_14 ;
reg [15:0]      patch_addr_15 ;
reg [15:0]      patch_addr_16 ;
reg [15:0]      patch_addr_17 ;
reg [15:0]      patch_addr_18 ;
reg [15:0]      patch_addr_19 ;
reg [15:0]      patch_addr_20 ;
reg [15:0]      patch_addr_21 ;
reg [15:0]      patch_addr_22 ;
reg [15:0]      patch_addr_23 ;
reg [15:0]      patch_addr_24 ;
reg [15:0]      patch_addr_25 ;
reg [15:0]      patch_addr_26 ;
reg [15:0]      patch_addr_27 ;
reg [15:0]      patch_addr_28 ;
reg [15:0]      patch_addr_29 ;
reg [15:0]      patch_addr_30 ;
reg [15:0]      patch_addr_31 ;
reg [31:0]      patch_data_0  ;
reg [31:0]      patch_data_1  ;
reg [31:0]      patch_data_2  ;
reg [31:0]      patch_data_3  ;
reg [31:0]      patch_data_4  ;
reg [31:0]      patch_data_5  ;
reg [31:0]      patch_data_6  ;
reg [31:0]      patch_data_7  ;
reg [31:0]      patch_data_8  ;
reg [31:0]      patch_data_9  ;
reg [31:0]      patch_data_10 ;
reg [31:0]      patch_data_11 ;
reg [31:0]      patch_data_12 ;
reg [31:0]      patch_data_13 ;
reg [31:0]      patch_data_14 ;
reg [31:0]      patch_data_15 ;
reg [31:0]      patch_data_16 ;
reg [31:0]      patch_data_17 ;
reg [31:0]      patch_data_18 ;
reg [31:0]      patch_data_19 ;
reg [31:0]      patch_data_20 ;
reg [31:0]      patch_data_21 ;
reg [31:0]      patch_data_22 ;
reg [31:0]      patch_data_23 ;
reg [31:0]      patch_data_24 ;
reg [31:0]      patch_data_25 ;
reg [31:0]      patch_data_26 ;
reg [31:0]      patch_data_27 ;
reg [31:0]      patch_data_28 ;
reg [31:0]      patch_data_29 ;
reg [31:0]      patch_data_30 ;
reg [31:0]      patch_data_31 ;
reg             r_patch_hit   ;
reg [31:0]      patch_rdata   ;

wire    wr_patch_cfg_0 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_CFG_0);
wire    wr_patch_cfg_1 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_CFG_1);

wire    wr_patch_addr_0  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_0 );
wire    wr_patch_addr_1  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_1 );
wire    wr_patch_addr_2  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_2 );
wire    wr_patch_addr_3  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_3 );
wire    wr_patch_addr_4  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_4 );
wire    wr_patch_addr_5  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_5 );
wire    wr_patch_addr_6  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_6 );
wire    wr_patch_addr_7  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_7 );
wire    wr_patch_addr_8  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_8 );
wire    wr_patch_addr_9  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_9 );
wire    wr_patch_addr_10 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_10);
wire    wr_patch_addr_11 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_11);
wire    wr_patch_addr_12 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_12);
wire    wr_patch_addr_13 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_13);
wire    wr_patch_addr_14 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_14);
wire    wr_patch_addr_15 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_ADDR_15);

wire    wr_patch_data_0  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_0 );
wire    wr_patch_data_1  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_1 );
wire    wr_patch_data_2  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_2 );
wire    wr_patch_data_3  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_3 );
wire    wr_patch_data_4  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_4 );
wire    wr_patch_data_5  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_5 );
wire    wr_patch_data_6  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_6 );
wire    wr_patch_data_7  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_7 );
wire    wr_patch_data_8  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_8 );
wire    wr_patch_data_9  = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_9 );
wire    wr_patch_data_10 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_10);
wire    wr_patch_data_11 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_11);
wire    wr_patch_data_12 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_12);
wire    wr_patch_data_13 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_13);
wire    wr_patch_data_14 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_14);
wire    wr_patch_data_15 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_15);
wire    wr_patch_data_16 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_16);
wire    wr_patch_data_17 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_17);
wire    wr_patch_data_18 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_18);
wire    wr_patch_data_19 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_19);
wire    wr_patch_data_20 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_20);
wire    wr_patch_data_21 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_21);
wire    wr_patch_data_22 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_22);
wire    wr_patch_data_23 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_23);
wire    wr_patch_data_24 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_24);
wire    wr_patch_data_25 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_25);
wire    wr_patch_data_26 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_26);
wire    wr_patch_data_27 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_27);
wire    wr_patch_data_28 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_28);
wire    wr_patch_data_29 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_29);
wire    wr_patch_data_30 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_30);
wire    wr_patch_data_31 = i_patch_info_vld & (i_patch_info_addr == P_PATCH_DATA_31);

wire    [31:0] patch_cfg_0_next       = wr_patch_cfg_0 ? i_patch_otp_rdata : patch_cfg_0       ; 
wire    [31:0] patch_cfg_1_next       = wr_patch_cfg_1 ? i_patch_otp_rdata : patch_cfg_1       ; 

wire    [15:0] patch_addr_0_next       = wr_patch_addr_0  ? i_patch_otp_rdata[15:0]  : patch_addr_0    ; 
wire    [15:0] patch_addr_1_next       = wr_patch_addr_0  ? i_patch_otp_rdata[31:16] : patch_addr_1    ; 
wire    [15:0] patch_addr_2_next       = wr_patch_addr_1  ? i_patch_otp_rdata[15:0]  : patch_addr_2    ; 
wire    [15:0] patch_addr_3_next       = wr_patch_addr_1  ? i_patch_otp_rdata[31:16] : patch_addr_3    ; 
wire    [15:0] patch_addr_4_next       = wr_patch_addr_2  ? i_patch_otp_rdata[15:0]  : patch_addr_4    ; 
wire    [15:0] patch_addr_5_next       = wr_patch_addr_2  ? i_patch_otp_rdata[31:16] : patch_addr_5    ; 
wire    [15:0] patch_addr_6_next       = wr_patch_addr_3  ? i_patch_otp_rdata[15:0]  : patch_addr_6    ; 
wire    [15:0] patch_addr_7_next       = wr_patch_addr_3  ? i_patch_otp_rdata[31:16] : patch_addr_7    ; 
wire    [15:0] patch_addr_8_next       = wr_patch_addr_4  ? i_patch_otp_rdata[15:0]  : patch_addr_8    ; 
wire    [15:0] patch_addr_9_next       = wr_patch_addr_4  ? i_patch_otp_rdata[31:16] : patch_addr_9    ; 
wire    [15:0] patch_addr_10_next      = wr_patch_addr_5  ? i_patch_otp_rdata[15:0]  : patch_addr_10   ; 
wire    [15:0] patch_addr_11_next      = wr_patch_addr_5  ? i_patch_otp_rdata[31:16] : patch_addr_11   ; 
wire    [15:0] patch_addr_12_next      = wr_patch_addr_6  ? i_patch_otp_rdata[15:0]  : patch_addr_12   ; 
wire    [15:0] patch_addr_13_next      = wr_patch_addr_6  ? i_patch_otp_rdata[31:16] : patch_addr_13   ; 
wire    [15:0] patch_addr_14_next      = wr_patch_addr_7  ? i_patch_otp_rdata[15:0]  : patch_addr_14   ; 
wire    [15:0] patch_addr_15_next      = wr_patch_addr_7  ? i_patch_otp_rdata[31:16] : patch_addr_15   ; 
wire    [15:0] patch_addr_16_next      = wr_patch_addr_8  ? i_patch_otp_rdata[15:0]  : patch_addr_16   ; 
wire    [15:0] patch_addr_17_next      = wr_patch_addr_8  ? i_patch_otp_rdata[31:16] : patch_addr_17   ; 
wire    [15:0] patch_addr_18_next      = wr_patch_addr_9  ? i_patch_otp_rdata[15:0]  : patch_addr_18   ; 
wire    [15:0] patch_addr_19_next      = wr_patch_addr_9  ? i_patch_otp_rdata[31:16] : patch_addr_19   ; 
wire    [15:0] patch_addr_20_next      = wr_patch_addr_10 ? i_patch_otp_rdata[15:0]  : patch_addr_20   ; 
wire    [15:0] patch_addr_21_next      = wr_patch_addr_10 ? i_patch_otp_rdata[31:16] : patch_addr_21   ; 
wire    [15:0] patch_addr_22_next      = wr_patch_addr_11 ? i_patch_otp_rdata[15:0]  : patch_addr_22   ; 
wire    [15:0] patch_addr_23_next      = wr_patch_addr_11 ? i_patch_otp_rdata[31:16] : patch_addr_23   ; 
wire    [15:0] patch_addr_24_next      = wr_patch_addr_12 ? i_patch_otp_rdata[15:0]  : patch_addr_24   ; 
wire    [15:0] patch_addr_25_next      = wr_patch_addr_12 ? i_patch_otp_rdata[31:16] : patch_addr_25   ; 
wire    [15:0] patch_addr_26_next      = wr_patch_addr_13 ? i_patch_otp_rdata[15:0]  : patch_addr_26   ; 
wire    [15:0] patch_addr_27_next      = wr_patch_addr_13 ? i_patch_otp_rdata[31:16] : patch_addr_27   ; 
wire    [15:0] patch_addr_28_next      = wr_patch_addr_14 ? i_patch_otp_rdata[15:0]  : patch_addr_28   ; 
wire    [15:0] patch_addr_29_next      = wr_patch_addr_14 ? i_patch_otp_rdata[31:16] : patch_addr_29   ; 
wire    [15:0] patch_addr_30_next      = wr_patch_addr_15 ? i_patch_otp_rdata[15:0]  : patch_addr_30   ; 
wire    [15:0] patch_addr_31_next      = wr_patch_addr_15 ? i_patch_otp_rdata[31:16] : patch_addr_31   ; 

wire    [31:0] patch_data_0_next       = wr_patch_data_0  ? i_patch_otp_rdata : patch_data_0    ; 
wire    [31:0] patch_data_1_next       = wr_patch_data_1  ? i_patch_otp_rdata : patch_data_1    ; 
wire    [31:0] patch_data_2_next       = wr_patch_data_2  ? i_patch_otp_rdata : patch_data_2    ; 
wire    [31:0] patch_data_3_next       = wr_patch_data_3  ? i_patch_otp_rdata : patch_data_3    ; 
wire    [31:0] patch_data_4_next       = wr_patch_data_4  ? i_patch_otp_rdata : patch_data_4    ; 
wire    [31:0] patch_data_5_next       = wr_patch_data_5  ? i_patch_otp_rdata : patch_data_5    ; 
wire    [31:0] patch_data_6_next       = wr_patch_data_6  ? i_patch_otp_rdata : patch_data_6    ; 
wire    [31:0] patch_data_7_next       = wr_patch_data_7  ? i_patch_otp_rdata : patch_data_7    ; 
wire    [31:0] patch_data_8_next       = wr_patch_data_8  ? i_patch_otp_rdata : patch_data_8    ; 
wire    [31:0] patch_data_9_next       = wr_patch_data_9  ? i_patch_otp_rdata : patch_data_9    ; 
wire    [31:0] patch_data_10_next      = wr_patch_data_10 ? i_patch_otp_rdata : patch_data_10   ; 
wire    [31:0] patch_data_11_next      = wr_patch_data_11 ? i_patch_otp_rdata : patch_data_11   ; 
wire    [31:0] patch_data_12_next      = wr_patch_data_12 ? i_patch_otp_rdata : patch_data_12   ; 
wire    [31:0] patch_data_13_next      = wr_patch_data_13 ? i_patch_otp_rdata : patch_data_13   ; 
wire    [31:0] patch_data_14_next      = wr_patch_data_14 ? i_patch_otp_rdata : patch_data_14   ; 
wire    [31:0] patch_data_15_next      = wr_patch_data_15 ? i_patch_otp_rdata : patch_data_15   ; 
wire    [31:0] patch_data_16_next      = wr_patch_data_16 ? i_patch_otp_rdata : patch_data_16   ; 
wire    [31:0] patch_data_17_next      = wr_patch_data_17 ? i_patch_otp_rdata : patch_data_17   ; 
wire    [31:0] patch_data_18_next      = wr_patch_data_18 ? i_patch_otp_rdata : patch_data_18   ; 
wire    [31:0] patch_data_19_next      = wr_patch_data_19 ? i_patch_otp_rdata : patch_data_19   ; 
wire    [31:0] patch_data_20_next      = wr_patch_data_20 ? i_patch_otp_rdata : patch_data_20   ; 
wire    [31:0] patch_data_21_next      = wr_patch_data_21 ? i_patch_otp_rdata : patch_data_21   ; 
wire    [31:0] patch_data_22_next      = wr_patch_data_22 ? i_patch_otp_rdata : patch_data_22   ; 
wire    [31:0] patch_data_23_next      = wr_patch_data_23 ? i_patch_otp_rdata : patch_data_23   ; 
wire    [31:0] patch_data_24_next      = wr_patch_data_24 ? i_patch_otp_rdata : patch_data_24   ; 
wire    [31:0] patch_data_25_next      = wr_patch_data_25 ? i_patch_otp_rdata : patch_data_25   ; 
wire    [31:0] patch_data_26_next      = wr_patch_data_26 ? i_patch_otp_rdata : patch_data_26   ; 
wire    [31:0] patch_data_27_next      = wr_patch_data_27 ? i_patch_otp_rdata : patch_data_27   ; 
wire    [31:0] patch_data_28_next      = wr_patch_data_28 ? i_patch_otp_rdata : patch_data_28   ; 
wire    [31:0] patch_data_29_next      = wr_patch_data_29 ? i_patch_otp_rdata : patch_data_29   ; 
wire    [31:0] patch_data_30_next      = wr_patch_data_30 ? i_patch_otp_rdata : patch_data_30   ; 
wire    [31:0] patch_data_31_next      = wr_patch_data_31 ? i_patch_otp_rdata : patch_data_31   ; 

wire    patch_en_0  = i_patch_en & ((patch_cfg_0[1:0]  ==2'b01)|(patch_cfg_0[1:0]  ==2'b10)) ;
wire    patch_en_1  = i_patch_en & ((patch_cfg_0[3:2]  ==2'b01)|(patch_cfg_0[3:2]  ==2'b10)) ;
wire    patch_en_2  = i_patch_en & ((patch_cfg_0[5:4]  ==2'b01)|(patch_cfg_0[5:4]  ==2'b10)) ;
wire    patch_en_3  = i_patch_en & ((patch_cfg_0[7:6]  ==2'b01)|(patch_cfg_0[7:6]  ==2'b10)) ;
wire    patch_en_4  = i_patch_en & ((patch_cfg_0[9:8]  ==2'b01)|(patch_cfg_0[9:8]  ==2'b10)) ;
wire    patch_en_5  = i_patch_en & ((patch_cfg_0[11:10]==2'b01)|(patch_cfg_0[11:10]==2'b10)) ;
wire    patch_en_6  = i_patch_en & ((patch_cfg_0[13:12]==2'b01)|(patch_cfg_0[13:12]==2'b10)) ;
wire    patch_en_7  = i_patch_en & ((patch_cfg_0[15:14]==2'b01)|(patch_cfg_0[15:14]==2'b10)) ;
wire    patch_en_8  = i_patch_en & ((patch_cfg_0[17:16]==2'b01)|(patch_cfg_0[17:16]==2'b10)) ;
wire    patch_en_9  = i_patch_en & ((patch_cfg_0[19:18]==2'b01)|(patch_cfg_0[19:18]==2'b10)) ;
wire    patch_en_10 = i_patch_en & ((patch_cfg_0[21:20]==2'b01)|(patch_cfg_0[21:20]==2'b10)) ;
wire    patch_en_11 = i_patch_en & ((patch_cfg_0[23:22]==2'b01)|(patch_cfg_0[23:22]==2'b10)) ;
wire    patch_en_12 = i_patch_en & ((patch_cfg_0[25:24]==2'b01)|(patch_cfg_0[25:24]==2'b10)) ;
wire    patch_en_13 = i_patch_en & ((patch_cfg_0[27:26]==2'b01)|(patch_cfg_0[27:26]==2'b10)) ;
wire    patch_en_14 = i_patch_en & ((patch_cfg_0[29:28]==2'b01)|(patch_cfg_0[29:28]==2'b10)) ;
wire    patch_en_15 = i_patch_en & ((patch_cfg_0[31:30]==2'b01)|(patch_cfg_0[31:30]==2'b10)) ;
wire    patch_en_16 = i_patch_en & ((patch_cfg_1[1:0]  ==2'b01)|(patch_cfg_1[1:0]  ==2'b10)) ;
wire    patch_en_17 = i_patch_en & ((patch_cfg_1[3:2]  ==2'b01)|(patch_cfg_1[3:2]  ==2'b10)) ;
wire    patch_en_18 = i_patch_en & ((patch_cfg_1[5:4]  ==2'b01)|(patch_cfg_1[5:4]  ==2'b10)) ;
wire    patch_en_19 = i_patch_en & ((patch_cfg_1[7:6]  ==2'b01)|(patch_cfg_1[7:6]  ==2'b10)) ;
wire    patch_en_20 = i_patch_en & ((patch_cfg_1[9:8]  ==2'b01)|(patch_cfg_1[9:8]  ==2'b10)) ;
wire    patch_en_21 = i_patch_en & ((patch_cfg_1[11:10]==2'b01)|(patch_cfg_1[11:10]==2'b10)) ;
wire    patch_en_22 = i_patch_en & ((patch_cfg_1[13:12]==2'b01)|(patch_cfg_1[13:12]==2'b10)) ;
wire    patch_en_23 = i_patch_en & ((patch_cfg_1[15:14]==2'b01)|(patch_cfg_1[15:14]==2'b10)) ;
wire    patch_en_24 = i_patch_en & ((patch_cfg_1[17:16]==2'b01)|(patch_cfg_1[17:16]==2'b10)) ;
wire    patch_en_25 = i_patch_en & ((patch_cfg_1[19:18]==2'b01)|(patch_cfg_1[19:18]==2'b10)) ;
wire    patch_en_26 = i_patch_en & ((patch_cfg_1[21:20]==2'b01)|(patch_cfg_1[21:20]==2'b10)) ;
wire    patch_en_27 = i_patch_en & ((patch_cfg_1[23:22]==2'b01)|(patch_cfg_1[23:22]==2'b10)) ;
wire    patch_en_28 = i_patch_en & ((patch_cfg_1[25:24]==2'b01)|(patch_cfg_1[25:24]==2'b10)) ;
wire    patch_en_29 = i_patch_en & ((patch_cfg_1[27:26]==2'b01)|(patch_cfg_1[27:26]==2'b10)) ;
wire    patch_en_30 = i_patch_en & ((patch_cfg_1[29:28]==2'b01)|(patch_cfg_1[29:28]==2'b10)) ;
wire    patch_en_31 = i_patch_en & ((patch_cfg_1[31:30]==2'b01)|(patch_cfg_1[31:30]==2'b10)) ;

wire    ahb_access  = i_hsels & i_htranss[1] & i_hreadys  ;
wire    checking    = ahb_access & ~i_hwrites;

wire    patch_hit_0         = checking & patch_en_0  & (i_haddrs[17:2] == patch_addr_0 ) ;
wire    patch_hit_1         = checking & patch_en_1  & (i_haddrs[17:2] == patch_addr_1 ) ;
wire    patch_hit_2         = checking & patch_en_2  & (i_haddrs[17:2] == patch_addr_2 ) ;
wire    patch_hit_3         = checking & patch_en_3  & (i_haddrs[17:2] == patch_addr_3 ) ;
wire    patch_hit_4         = checking & patch_en_4  & (i_haddrs[17:2] == patch_addr_4 ) ;
wire    patch_hit_5         = checking & patch_en_5  & (i_haddrs[17:2] == patch_addr_5 ) ;
wire    patch_hit_6         = checking & patch_en_6  & (i_haddrs[17:2] == patch_addr_6 ) ;
wire    patch_hit_7         = checking & patch_en_7  & (i_haddrs[17:2] == patch_addr_7 ) ;
wire    patch_hit_8         = checking & patch_en_8  & (i_haddrs[17:2] == patch_addr_8 ) ;
wire    patch_hit_9         = checking & patch_en_9  & (i_haddrs[17:2] == patch_addr_9 ) ;
wire    patch_hit_10        = checking & patch_en_10 & (i_haddrs[17:2] == patch_addr_10) ;
wire    patch_hit_11        = checking & patch_en_11 & (i_haddrs[17:2] == patch_addr_11) ;
wire    patch_hit_12        = checking & patch_en_12 & (i_haddrs[17:2] == patch_addr_12) ;
wire    patch_hit_13        = checking & patch_en_13 & (i_haddrs[17:2] == patch_addr_13) ;
wire    patch_hit_14        = checking & patch_en_14 & (i_haddrs[17:2] == patch_addr_14) ;
wire    patch_hit_15        = checking & patch_en_15 & (i_haddrs[17:2] == patch_addr_15) ;
wire    patch_hit_16        = checking & patch_en_16 & (i_haddrs[17:2] == patch_addr_16) ;
wire    patch_hit_17        = checking & patch_en_17 & (i_haddrs[17:2] == patch_addr_17) ;
wire    patch_hit_18        = checking & patch_en_18 & (i_haddrs[17:2] == patch_addr_18) ;
wire    patch_hit_19        = checking & patch_en_19 & (i_haddrs[17:2] == patch_addr_19) ;
wire    patch_hit_20        = checking & patch_en_20 & (i_haddrs[17:2] == patch_addr_20) ;
wire    patch_hit_21        = checking & patch_en_21 & (i_haddrs[17:2] == patch_addr_21) ;
wire    patch_hit_22        = checking & patch_en_22 & (i_haddrs[17:2] == patch_addr_22) ;
wire    patch_hit_23        = checking & patch_en_23 & (i_haddrs[17:2] == patch_addr_23) ;
wire    patch_hit_24        = checking & patch_en_24 & (i_haddrs[17:2] == patch_addr_24) ;
wire    patch_hit_25        = checking & patch_en_25 & (i_haddrs[17:2] == patch_addr_25) ;
wire    patch_hit_26        = checking & patch_en_26 & (i_haddrs[17:2] == patch_addr_26) ;
wire    patch_hit_27        = checking & patch_en_27 & (i_haddrs[17:2] == patch_addr_27) ;
wire    patch_hit_28        = checking & patch_en_28 & (i_haddrs[17:2] == patch_addr_28) ;
wire    patch_hit_29        = checking & patch_en_29 & (i_haddrs[17:2] == patch_addr_29) ;
wire    patch_hit_30        = checking & patch_en_30 & (i_haddrs[17:2] == patch_addr_30) ;
wire    patch_hit_31        = checking & patch_en_31 & (i_haddrs[17:2] == patch_addr_31) ;

wire    set_r_patch_hit     = patch_hit_0  | 
                              patch_hit_1  | 
                              patch_hit_2  | 
                              patch_hit_3  | 
                              patch_hit_4  | 
                              patch_hit_5  | 
                              patch_hit_6  | 
                              patch_hit_7  | 
                              patch_hit_8  | 
                              patch_hit_9  | 
                              patch_hit_10 | 
                              patch_hit_11 | 
                              patch_hit_12 | 
                              patch_hit_13 | 
                              patch_hit_14 | 
                              patch_hit_15 | 
                              patch_hit_16 | 
                              patch_hit_17 | 
                              patch_hit_18 | 
                              patch_hit_19 | 
                              patch_hit_20 | 
                              patch_hit_21 | 
                              patch_hit_22 | 
                              patch_hit_23 | 
                              patch_hit_24 | 
                              patch_hit_25 | 
                              patch_hit_26 | 
                              patch_hit_27 | 
                              patch_hit_28 | 
                              patch_hit_29 | 
                              patch_hit_30 | 
                              patch_hit_31 ;

wire        r_patch_hit_next   = set_r_patch_hit ? 1'b1 : i_hreadyoutm ? 1'b0 : r_patch_hit ; 

wire [31:0] patch_rdata_next   = ({32{patch_hit_0     }} & patch_data_0     ) |     
                                 ({32{patch_hit_1     }} & patch_data_1     ) |
                                 ({32{patch_hit_2     }} & patch_data_2     ) |
                                 ({32{patch_hit_3     }} & patch_data_3     ) |
                                 ({32{patch_hit_4     }} & patch_data_4     ) |
                                 ({32{patch_hit_5     }} & patch_data_5     ) |
                                 ({32{patch_hit_6     }} & patch_data_6     ) |
                                 ({32{patch_hit_7     }} & patch_data_7     ) |
                                 ({32{patch_hit_8     }} & patch_data_8     ) |
                                 ({32{patch_hit_9     }} & patch_data_9     ) |
                                 ({32{patch_hit_10    }} & patch_data_10    ) |
                                 ({32{patch_hit_11    }} & patch_data_11    ) |
                                 ({32{patch_hit_12    }} & patch_data_12    ) |
                                 ({32{patch_hit_13    }} & patch_data_13    ) |
                                 ({32{patch_hit_14    }} & patch_data_14    ) |
                                 ({32{patch_hit_15    }} & patch_data_15    ) |
                                 ({32{patch_hit_16    }} & patch_data_16    ) |
                                 ({32{patch_hit_17    }} & patch_data_17    ) |
                                 ({32{patch_hit_18    }} & patch_data_18    ) |
                                 ({32{patch_hit_19    }} & patch_data_19    ) |
                                 ({32{patch_hit_20    }} & patch_data_20    ) |
                                 ({32{patch_hit_21    }} & patch_data_21    ) |
                                 ({32{patch_hit_22    }} & patch_data_22    ) |
                                 ({32{patch_hit_23    }} & patch_data_23    ) |
                                 ({32{patch_hit_24    }} & patch_data_24    ) |
                                 ({32{patch_hit_25    }} & patch_data_25    ) |
                                 ({32{patch_hit_26    }} & patch_data_26    ) |
                                 ({32{patch_hit_27    }} & patch_data_27    ) |
                                 ({32{patch_hit_28    }} & patch_data_28    ) |
                                 ({32{patch_hit_29    }} & patch_data_29    ) |
                                 ({32{patch_hit_30    }} & patch_data_30    ) |
                                 ({32{patch_hit_31    }} & patch_data_31    ) |
                                 ({32{~set_r_patch_hit}} & patch_rdata      ) ;

assign o_hrdatas         = (r_patch_hit & i_hreadyoutm) ? patch_rdata : i_hrdatam ;
assign o_hreadyouts      = i_hreadyoutm                                           ;
assign o_hresps          = i_hrespm                                               ;
assign o_hselm           = ~set_r_patch_hit & i_hsels                             ;
assign o_haddrm          = i_haddrs                                               ;
assign o_htransm         = i_htranss                                              ;
assign o_hwritem         = i_hwrites                                              ;
assign o_hsizem          = i_hsizes                                               ;
assign o_hburstm         = i_hbursts                                              ;
assign o_hprotm          = i_hprots                                               ;
assign o_hwdatam         = i_hwdatas                                              ;
assign o_hmastlockm      = i_hmastlocks                                           ;
assign o_hreadym         = i_hreadys                                              ;

always @(posedge i_hclk or negedge i_hresetn) begin
    if(!i_hresetn) begin
        patch_cfg_0   <= 32'h0              ; 
        patch_cfg_1   <= 32'h0              ; 
        patch_addr_0  <= 16'h0              ; 
        patch_addr_1  <= 16'h0              ; 
        patch_addr_2  <= 16'h0              ; 
        patch_addr_3  <= 16'h0              ; 
        patch_addr_4  <= 16'h0              ; 
        patch_addr_5  <= 16'h0              ; 
        patch_addr_6  <= 16'h0              ; 
        patch_addr_7  <= 16'h0              ; 
        patch_addr_8  <= 16'h0              ; 
        patch_addr_9  <= 16'h0              ; 
        patch_addr_10 <= 16'h0              ; 
        patch_addr_11 <= 16'h0              ; 
        patch_addr_12 <= 16'h0              ; 
        patch_addr_13 <= 16'h0              ; 
        patch_addr_14 <= 16'h0              ; 
        patch_addr_15 <= 16'h0              ; 
        patch_addr_16 <= 16'h0              ; 
        patch_addr_17 <= 16'h0              ; 
        patch_addr_18 <= 16'h0              ; 
        patch_addr_19 <= 16'h0              ; 
        patch_addr_20 <= 16'h0              ; 
        patch_addr_21 <= 16'h0              ; 
        patch_addr_22 <= 16'h0              ; 
        patch_addr_23 <= 16'h0              ; 
        patch_addr_24 <= 16'h0              ; 
        patch_addr_25 <= 16'h0              ; 
        patch_addr_26 <= 16'h0              ; 
        patch_addr_27 <= 16'h0              ; 
        patch_addr_28 <= 16'h0              ; 
        patch_addr_29 <= 16'h0              ; 
        patch_addr_30 <= 16'h0              ; 
        patch_addr_31 <= 16'h0              ; 
        patch_data_0  <= 32'h0              ; 
        patch_data_1  <= 32'h0              ; 
        patch_data_2  <= 32'h0              ; 
        patch_data_3  <= 32'h0              ; 
        patch_data_4  <= 32'h0              ; 
        patch_data_5  <= 32'h0              ; 
        patch_data_6  <= 32'h0              ; 
        patch_data_7  <= 32'h0              ; 
        patch_data_8  <= 32'h0              ; 
        patch_data_9  <= 32'h0              ; 
        patch_data_10 <= 32'h0              ; 
        patch_data_11 <= 32'h0              ; 
        patch_data_12 <= 32'h0              ; 
        patch_data_13 <= 32'h0              ; 
        patch_data_14 <= 32'h0              ; 
        patch_data_15 <= 32'h0              ; 
        patch_data_16 <= 32'h0              ; 
        patch_data_17 <= 32'h0              ; 
        patch_data_18 <= 32'h0              ; 
        patch_data_19 <= 32'h0              ; 
        patch_data_20 <= 32'h0              ; 
        patch_data_21 <= 32'h0              ; 
        patch_data_22 <= 32'h0              ; 
        patch_data_23 <= 32'h0              ; 
        patch_data_24 <= 32'h0              ; 
        patch_data_25 <= 32'h0              ; 
        patch_data_26 <= 32'h0              ; 
        patch_data_27 <= 32'h0              ; 
        patch_data_28 <= 32'h0              ; 
        patch_data_29 <= 32'h0              ; 
        patch_data_30 <= 32'h0              ; 
        patch_data_31 <= 32'h0              ; 
        r_patch_hit   <= 1'b0               ; 
        patch_rdata   <= 32'h0              ; 
    end else begin
        patch_cfg_0   <= patch_cfg_0_next   ; 
        patch_cfg_1   <= patch_cfg_1_next   ; 
        patch_addr_0  <= patch_addr_0_next  ; 
        patch_addr_1  <= patch_addr_1_next  ; 
        patch_addr_2  <= patch_addr_2_next  ; 
        patch_addr_3  <= patch_addr_3_next  ; 
        patch_addr_4  <= patch_addr_4_next  ; 
        patch_addr_5  <= patch_addr_5_next  ; 
        patch_addr_6  <= patch_addr_6_next  ; 
        patch_addr_7  <= patch_addr_7_next  ; 
        patch_addr_8  <= patch_addr_8_next  ; 
        patch_addr_9  <= patch_addr_9_next  ; 
        patch_addr_10 <= patch_addr_10_next ; 
        patch_addr_11 <= patch_addr_11_next ; 
        patch_addr_12 <= patch_addr_12_next ; 
        patch_addr_13 <= patch_addr_13_next ; 
        patch_addr_14 <= patch_addr_14_next ; 
        patch_addr_15 <= patch_addr_15_next ; 
        patch_addr_16 <= patch_addr_16_next ; 
        patch_addr_17 <= patch_addr_17_next ; 
        patch_addr_18 <= patch_addr_18_next ; 
        patch_addr_19 <= patch_addr_19_next ; 
        patch_addr_20 <= patch_addr_20_next ; 
        patch_addr_21 <= patch_addr_21_next ; 
        patch_addr_22 <= patch_addr_22_next ; 
        patch_addr_23 <= patch_addr_23_next ; 
        patch_addr_24 <= patch_addr_24_next ; 
        patch_addr_25 <= patch_addr_25_next ; 
        patch_addr_26 <= patch_addr_26_next ; 
        patch_addr_27 <= patch_addr_27_next ; 
        patch_addr_28 <= patch_addr_28_next ; 
        patch_addr_29 <= patch_addr_29_next ; 
        patch_addr_30 <= patch_addr_30_next ; 
        patch_addr_31 <= patch_addr_31_next ; 
        patch_data_0  <= patch_data_0_next  ; 
        patch_data_1  <= patch_data_1_next  ; 
        patch_data_2  <= patch_data_2_next  ; 
        patch_data_3  <= patch_data_3_next  ; 
        patch_data_4  <= patch_data_4_next  ; 
        patch_data_5  <= patch_data_5_next  ; 
        patch_data_6  <= patch_data_6_next  ; 
        patch_data_7  <= patch_data_7_next  ; 
        patch_data_8  <= patch_data_8_next  ; 
        patch_data_9  <= patch_data_9_next  ; 
        patch_data_10 <= patch_data_10_next ; 
        patch_data_11 <= patch_data_11_next ; 
        patch_data_12 <= patch_data_12_next ; 
        patch_data_13 <= patch_data_13_next ; 
        patch_data_14 <= patch_data_14_next ; 
        patch_data_15 <= patch_data_15_next ; 
        patch_data_16 <= patch_data_16_next ; 
        patch_data_17 <= patch_data_17_next ; 
        patch_data_18 <= patch_data_18_next ; 
        patch_data_19 <= patch_data_19_next ; 
        patch_data_20 <= patch_data_20_next ; 
        patch_data_21 <= patch_data_21_next ; 
        patch_data_22 <= patch_data_22_next ; 
        patch_data_23 <= patch_data_23_next ; 
        patch_data_24 <= patch_data_24_next ; 
        patch_data_25 <= patch_data_25_next ; 
        patch_data_26 <= patch_data_26_next ; 
        patch_data_27 <= patch_data_27_next ; 
        patch_data_28 <= patch_data_28_next ; 
        patch_data_29 <= patch_data_29_next ; 
        patch_data_30 <= patch_data_30_next ; 
        patch_data_31 <= patch_data_31_next ; 
        r_patch_hit   <= r_patch_hit_next   ; 
        patch_rdata   <= patch_rdata_next   ; 
    end
end

endmodule 
