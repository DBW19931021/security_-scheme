//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_mtx_wrapper (
    input  wire                  i_hclk                    ,
    input  wire                  i_hresetn                 ,
    input  wire                  i_cipher_en               ,
    input  wire [31:0]           i_key                     ,
    input  wire [31:0]           i_dat_nounce              ,
    input  wire [31:0]           i_adr_nounce              ,

    input  wire                  i_nosec_m0                ,
    input  wire [4:0]            i_masterid_m0             ,
    input  wire [7:0]            i_chid_m0                 ,
    input  wire                  i_nosec_m1                ,
    input  wire [4:0]            i_masterid_m1             ,
    input  wire [7:0]            i_chid_m1                 ,
    input  wire                  i_nosec_m2                ,
    input  wire [4:0]            i_masterid_m2             ,
    input  wire [7:0]            i_chid_m2                 ,
    input  wire                  i_nosec_m3                ,
    input  wire [4:0]            i_masterid_m3             ,
    input  wire [7:0]            i_chid_m3                 ,

    input  wire                  i_hsels0                  ,
    input  wire [31:0]           i_haddrs0                 ,
    input  wire  [1:0]           i_htranss0                ,
    input  wire                  i_hwrites0                ,
    input  wire  [2:0]           i_hsizes0                 ,
    input  wire  [2:0]           i_hbursts0                ,
    input  wire  [3:0]           i_hprots0                 ,
    input  wire  [7:0]           i_hmasters0               ,
    input  wire [35:0]           i_hwdatas0                ,
    input  wire                  i_hmastlocks0             ,
    input  wire                  i_hreadys0                ,

    output wire [35:0]           o_hrdatas0                ,
    output wire                  o_hreadyouts0             ,
    output wire  [1:0]           o_hresps0                 ,

    input  wire                  i_hsels1                  ,
    input  wire [31:0]           i_haddrs1                 ,
    input  wire  [1:0]           i_htranss1                ,
    input  wire                  i_hwrites1                ,
    input  wire  [2:0]           i_hsizes1                 ,
    input  wire  [2:0]           i_hbursts1                ,
    input  wire  [3:0]           i_hprots1                 ,
    input  wire  [7:0]           i_hmasters1               ,
    input  wire [35:0]           i_hwdatas1                ,
    input  wire                  i_hmastlocks1             ,
    input  wire                  i_hreadys1                ,

    output wire [35:0]           o_hrdatas1                ,
    output wire                  o_hreadyouts1             ,
    output wire  [1:0]           o_hresps1                 ,

    input  wire                  i_hsels2                  ,
    input  wire [31:0]           i_haddrs2                 ,
    input  wire  [1:0]           i_htranss2                ,
    input  wire                  i_hwrites2                ,
    input  wire  [2:0]           i_hsizes2                 ,
    input  wire  [2:0]           i_hbursts2                ,
    input  wire  [3:0]           i_hprots2                 ,
    input  wire  [7:0]           i_hmasters2               ,
    input  wire [35:0]           i_hwdatas2                ,
    input  wire                  i_hmastlocks2             ,
    input  wire                  i_hreadys2                ,

    output wire [35:0]           o_hrdatas2                ,
    output wire                  o_hreadyouts2             ,
    output wire  [1:0]           o_hresps2                 ,

    input  wire                  i_hsels3                  ,
    input  wire [31:0]           i_haddrs3                 ,
    input  wire  [1:0]           i_htranss3                ,
    input  wire                  i_hwrites3                ,
    input  wire  [2:0]           i_hsizes3                 ,
    input  wire  [2:0]           i_hbursts3                ,
    input  wire  [3:0]           i_hprots3                 ,
    input  wire  [7:0]           i_hmasters3               ,
    input  wire [35:0]           i_hwdatas3                ,
    input  wire                  i_hmastlocks3             ,
    input  wire                  i_hreadys3                ,

    output wire [35:0]           o_hrdatas3                ,
    output wire                  o_hreadyouts3             ,
    output wire  [1:0]           o_hresps3                 ,

    input  wire                  i_hsels4                  ,
    input  wire [31:0]           i_haddrs4                 ,
    input  wire  [1:0]           i_htranss4                ,
    input  wire                  i_hwrites4                ,
    input  wire  [2:0]           i_hsizes4                 ,
    input  wire  [2:0]           i_hbursts4                ,
    input  wire  [3:0]           i_hprots4                 ,
    input  wire  [7:0]           i_hmasters4               ,
    input  wire [35:0]           i_hwdatas4                ,
    input  wire                  i_hmastlocks4             ,
    input  wire                  i_hreadys4                ,

    output wire [35:0]           o_hrdatas4                ,
    output wire                  o_hreadyouts4             ,
    output wire  [1:0]           o_hresps4                 ,

    output wire                  o_hselm0                  ,
    output wire [31:0]           o_haddrm0                 ,
    output wire  [1:0]           o_htransm0                ,
    output wire                  o_hwritem0                ,
    output wire  [2:0]           o_hsizem0                 ,
    output wire  [2:0]           o_hburstm0                ,
    output wire  [3:0]           o_hprotm0                 ,
    output wire  [7:0]           o_hmasterm0               ,
    output wire [35:0]           o_hwdatam0                ,
    output wire                  o_hmastlockm0             ,
    output wire                  o_hreadymuxm0             ,

    input  wire [35:0]           i_hrdatam0                ,
    input  wire                  i_hreadyoutm0             ,
    input  wire  [1:0]           i_hrespm0                 ,

    output wire                  o_hselm1                  ,
    output wire [31:0]           o_haddrm1                 ,
    output wire  [1:0]           o_htransm1                ,
    output wire                  o_hwritem1                ,
    output wire  [2:0]           o_hsizem1                 ,
    output wire  [2:0]           o_hburstm1                ,
    output wire  [3:0]           o_hprotm1                 ,
    output wire  [7:0]           o_hmasterm1               ,
    output wire [35:0]           o_hwdatam1                ,
    output wire                  o_hmastlockm1             ,
    output wire                  o_hreadymuxm1             ,

    input  wire [35:0]           i_hrdatam1                ,
    input  wire                  i_hreadyoutm1             ,
    input  wire  [1:0]           i_hrespm1                 ,

    output wire                  o_hselm2                  ,
    output wire [31:0]           o_haddrm2                 ,
    output wire  [1:0]           o_htransm2                ,
    output wire                  o_hwritem2                ,
    output wire  [2:0]           o_hsizem2                 ,
    output wire  [2:0]           o_hburstm2                ,
    output wire  [3:0]           o_hprotm2                 ,
    output wire  [7:0]           o_hmasterm2               ,
    output wire [35:0]           o_hwdatam2                ,
    output wire                  o_hmastlockm2             ,
    output wire                  o_hreadymuxm2             ,

    input  wire [35:0]           i_hrdatam2                ,
    input  wire                  i_hreadyoutm2             ,
    input  wire  [1:0]           i_hrespm2                 ,

    output wire                  o_hselm3                  ,
    output wire [31:0]           o_haddrm3                 ,
    output wire  [1:0]           o_htransm3                ,
    output wire                  o_hwritem3                ,
    output wire  [2:0]           o_hsizem3                 ,
    output wire  [2:0]           o_hburstm3                ,
    output wire  [3:0]           o_hprotm3                 ,
    output wire  [7:0]           o_hmasterm3               ,
    output wire [35:0]           o_hwdatam3                ,
    output wire                  o_hmastlockm3             ,
    output wire                  o_hreadymuxm3             ,

    input  wire [35:0]           i_hrdatam3                ,
    input  wire                  i_hreadyoutm3             ,
    input  wire  [1:0]           i_hrespm3                 ,

    output wire                  o_hselm4                  ,
    output wire [31:0]           o_haddrm4                 ,
    output wire  [1:0]           o_htransm4                ,
    output wire                  o_hwritem4                ,
    output wire  [2:0]           o_hsizem4                 ,
    output wire  [2:0]           o_hburstm4                ,
    output wire  [3:0]           o_hprotm4                 ,
    output wire  [7:0]           o_hmasterm4               ,
    output wire [35:0]           o_hwdatam4                ,
    output wire                  o_hmastlockm4             ,
    output wire                  o_hreadymuxm4             ,

    input  wire [35:0]           i_hrdatam4                ,
    input  wire                  i_hreadyoutm4             ,
    input  wire  [1:0]           i_hrespm4                 ,

    output wire                  o_hselm5                  ,
    output wire [31:0]           o_haddrm5                 ,
    output wire  [1:0]           o_htransm5                ,
    output wire                  o_hwritem5                ,
    output wire  [2:0]           o_hsizem5                 ,
    output wire  [2:0]           o_hburstm5                ,
    output wire  [3:0]           o_hprotm5                 ,
    output wire  [7:0]           o_hmasterm5               ,
    output wire [35:0]           o_hwdatam5                ,
    output wire                  o_hmastlockm5             ,
    output wire                  o_hreadymuxm5             ,

    input  wire [35:0]           i_hrdatam5                ,
    input  wire                  i_hreadyoutm5             ,
    input  wire  [1:0]           i_hrespm5
);

localparam P_BUS_MATRIX_DELAY_EN = 1;

localparam     MTX_P_BUS_MATRIX_AWIDTH   = 32                         ;
localparam     MTX_P_BUS_MATRIX_DWIDTH   = 36                         ;
localparam     MTX_P_BUS_MATRIX_DELAY_EN = P_BUS_MATRIX_DELAY_EN      ;

localparam     CS0_p_ADDR_WIDTH       = 12;
localparam     CS0_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CS0_p_DATA_WIDTH       = 32;
localparam     CS0_p_ROUND_NUM        = 2 ;

localparam     CS1_p_ADDR_WIDTH       = 12;
localparam     CS1_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CS1_p_DATA_WIDTH       = 32;
localparam     CS1_p_ROUND_NUM        = 2 ;

localparam     CS2_p_ADDR_WIDTH       = 12;
localparam     CS2_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CS2_p_DATA_WIDTH       = 32;
localparam     CS2_p_ROUND_NUM        = 2 ;

localparam     CS3_p_ADDR_WIDTH       = 12;
localparam     CS3_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CS3_p_DATA_WIDTH       = 32;
localparam     CS3_p_ROUND_NUM        = 2 ;

localparam     CS4_p_ADDR_WIDTH       = 12;
localparam     CS4_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CS4_p_DATA_WIDTH       = 32;
localparam     CS4_p_ROUND_NUM        = 2 ;

localparam     CM0_p_ADDR_WIDTH       = 12;
localparam     CM0_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CM0_p_DATA_WIDTH       = 32;
localparam     CM0_p_ROUND_NUM        = 2 ;

localparam     CM1_p_ADDR_WIDTH       = 12;
localparam     CM1_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CM1_p_DATA_WIDTH       = 32;
localparam     CM1_p_ROUND_NUM        = 2 ;

localparam     CM2_p_ADDR_WIDTH       = 12;
localparam     CM2_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CM2_p_DATA_WIDTH       = 32;
localparam     CM2_p_ROUND_NUM        = 2 ;

localparam     CM3_p_ADDR_WIDTH       = 12;
localparam     CM3_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CM3_p_DATA_WIDTH       = 32;
localparam     CM3_p_ROUND_NUM        = 2 ;

localparam     CM4_p_ADDR_WIDTH       = 12;
localparam     CM4_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CM4_p_DATA_WIDTH       = 32;
localparam     CM4_p_ROUND_NUM        = 2 ;

localparam     CM5_p_ADDR_WIDTH       = 12;
localparam     CM5_p_ADDR_BLOCK_WIDTH = 0 ;
localparam     CM5_p_DATA_WIDTH       = 32;
localparam     CM5_p_ROUND_NUM        = 2 ;

wire                                  mtx_o_data_dft0   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hrdatas0    ;
wire                                  mtx_o_hreadyouts0 ;
wire [1:0]                            mtx_o_hresps0     ;
wire                                  mtx_o_data_dft1   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hrdatas1    ;
wire                                  mtx_o_hreadyouts1 ;
wire [1:0]                            mtx_o_hresps1     ;
wire                                  mtx_o_data_dft2   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hrdatas2    ;
wire                                  mtx_o_hreadyouts2 ;
wire [1:0]                            mtx_o_hresps2     ;
wire                                  mtx_o_data_dft3   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hrdatas3    ;
wire                                  mtx_o_hreadyouts3 ;
wire [1:0]                            mtx_o_hresps3     ;
wire                                  mtx_o_data_dft4   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hrdatas4    ;
wire                                  mtx_o_hreadyouts4 ;
wire [1:0]                            mtx_o_hresps4     ;
wire                                  mtx_o_hselm0      ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_o_haddrm0     ;
wire [1:0]                            mtx_o_htransm0    ;
wire                                  mtx_o_hwritem0    ;
wire [2:0]                            mtx_o_hsizem0     ;
wire [2:0]                            mtx_o_hburstm0    ;
wire [3:0]                            mtx_o_hprotm0     ;
wire [7:0]                            mtx_o_hmasterm0   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hwdatam0    ;
wire                                  mtx_o_hmastlockm0 ;
wire                                  mtx_o_hreadymuxm0 ;
wire                                  mtx_o_hselm1      ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_o_haddrm1     ;
wire [1:0]                            mtx_o_htransm1    ;
wire                                  mtx_o_hwritem1    ;
wire [2:0]                            mtx_o_hsizem1     ;
wire [2:0]                            mtx_o_hburstm1    ;
wire [3:0]                            mtx_o_hprotm1     ;
wire [7:0]                            mtx_o_hmasterm1   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hwdatam1    ;
wire                                  mtx_o_hmastlockm1 ;
wire                                  mtx_o_hreadymuxm1 ;
wire                                  mtx_o_hselm2      ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_o_haddrm2     ;
wire [1:0]                            mtx_o_htransm2    ;
wire                                  mtx_o_hwritem2    ;
wire [2:0]                            mtx_o_hsizem2     ;
wire [2:0]                            mtx_o_hburstm2    ;
wire [3:0]                            mtx_o_hprotm2     ;
wire [7:0]                            mtx_o_hmasterm2   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hwdatam2    ;
wire                                  mtx_o_hmastlockm2 ;
wire                                  mtx_o_hreadymuxm2 ;
wire                                  mtx_o_hselm3      ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_o_haddrm3     ;
wire [1:0]                            mtx_o_htransm3    ;
wire                                  mtx_o_hwritem3    ;
wire [2:0]                            mtx_o_hsizem3     ;
wire [2:0]                            mtx_o_hburstm3    ;
wire [3:0]                            mtx_o_hprotm3     ;
wire [7:0]                            mtx_o_hmasterm3   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hwdatam3    ;
wire                                  mtx_o_hmastlockm3 ;
wire                                  mtx_o_hreadymuxm3 ;
wire                                  mtx_o_hselm4      ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_o_haddrm4     ;
wire [1:0]                            mtx_o_htransm4    ;
wire                                  mtx_o_hwritem4    ;
wire [2:0]                            mtx_o_hsizem4     ;
wire [2:0]                            mtx_o_hburstm4    ;
wire [3:0]                            mtx_o_hprotm4     ;
wire [7:0]                            mtx_o_hmasterm4   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hwdatam4    ;
wire                                  mtx_o_hmastlockm4 ;
wire                                  mtx_o_hreadymuxm4 ;
wire                                  mtx_o_hselm5      ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_o_haddrm5     ;
wire [1:0]                            mtx_o_htransm5    ;
wire                                  mtx_o_hwritem5    ;
wire [2:0]                            mtx_o_hsizem5     ;
wire [2:0]                            mtx_o_hburstm5    ;
wire [3:0]                            mtx_o_hprotm5     ;
wire [7:0]                            mtx_o_hmasterm5   ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_o_hwdatam5    ;
wire                                  mtx_o_hmastlockm5 ;
wire                                  mtx_o_hreadymuxm5 ;

wire [CS0_p_ADDR_WIDTH-1:0]     cs0_o_bus_addr    = i_haddrs0[13:2]   ;
wire [CS0_p_DATA_WIDTH-1:0]     cs0_o_cipher_data = i_hwdatas0[31:0]    ;
wire [CS0_p_DATA_WIDTH-1:0]     cs0_o_plain_data  = mtx_o_hrdatas0[31:0];

wire [CS1_p_ADDR_WIDTH-1:0]     cs1_o_bus_addr    = i_haddrs1[13:2]   ;
wire [CS1_p_DATA_WIDTH-1:0]     cs1_o_cipher_data = i_hwdatas1[31:0]    ;
wire [CS1_p_DATA_WIDTH-1:0]     cs1_o_plain_data  = mtx_o_hrdatas1[31:0];

wire [CS2_p_ADDR_WIDTH-1:0]     cs2_o_bus_addr    = i_haddrs2[13:2]   ;
wire [CS2_p_DATA_WIDTH-1:0]     cs2_o_cipher_data = i_hwdatas2[31:0]    ;
wire [CS2_p_DATA_WIDTH-1:0]     cs2_o_plain_data  = mtx_o_hrdatas2[31:0];

wire [CS3_p_ADDR_WIDTH-1:0]     cs3_o_bus_addr    = i_haddrs3[13:2]   ;
wire [CS3_p_DATA_WIDTH-1:0]     cs3_o_cipher_data = i_hwdatas3[31:0]    ;
wire [CS3_p_DATA_WIDTH-1:0]     cs3_o_plain_data  = mtx_o_hrdatas3[31:0];

wire [CS4_p_ADDR_WIDTH-1:0]     cs4_o_bus_addr    = i_haddrs4[13:2]   ;
wire [CS4_p_DATA_WIDTH-1:0]     cs4_o_cipher_data = i_hwdatas4[31:0]    ;
wire [CS4_p_DATA_WIDTH-1:0]     cs4_o_plain_data  = mtx_o_hrdatas4[31:0];

wire [CM0_p_ADDR_WIDTH-1:0]     cm0_o_bus_addr    = mtx_o_haddrm0[13:2] ;
wire [CM0_p_DATA_WIDTH-1:0]     cm0_o_cipher_data = i_hrdatam0[31:0]    ;
wire [CM0_p_DATA_WIDTH-1:0]     cm0_o_plain_data  = mtx_o_hwdatam0[31:0];

wire [CM1_p_ADDR_WIDTH-1:0]     cm1_o_bus_addr    = mtx_o_haddrm1[13:2] ;
wire [CM1_p_DATA_WIDTH-1:0]     cm1_o_cipher_data = i_hrdatam1[31:0]    ;
wire [CM1_p_DATA_WIDTH-1:0]     cm1_o_plain_data  = mtx_o_hwdatam1[31:0];

wire [CM2_p_ADDR_WIDTH-1:0]     cm2_o_bus_addr    = mtx_o_haddrm2[13:2] ;
wire [CM2_p_DATA_WIDTH-1:0]     cm2_o_cipher_data = i_hrdatam2[31:0]    ;
wire [CM2_p_DATA_WIDTH-1:0]     cm2_o_plain_data  = mtx_o_hwdatam2[31:0];

wire [CM3_p_ADDR_WIDTH-1:0]     cm3_o_bus_addr    = mtx_o_haddrm3[13:2] ;
wire [CM3_p_DATA_WIDTH-1:0]     cm3_o_cipher_data = i_hrdatam3[31:0]    ;
wire [CM3_p_DATA_WIDTH-1:0]     cm3_o_plain_data  = mtx_o_hwdatam3[31:0];

wire [CM4_p_ADDR_WIDTH-1:0]     cm4_o_bus_addr    = mtx_o_haddrm4[13:2] ;
wire [CM4_p_DATA_WIDTH-1:0]     cm4_o_cipher_data = i_hrdatam4[31:0]    ;
wire [CM4_p_DATA_WIDTH-1:0]     cm4_o_plain_data  = mtx_o_hwdatam4[31:0];

wire [CM5_p_ADDR_WIDTH-1:0]     cm5_o_bus_addr    = mtx_o_haddrm5[13:2] ;
wire [CM5_p_DATA_WIDTH-1:0]     cm5_o_cipher_data = i_hrdatam5[31:0]    ;
wire [CM5_p_DATA_WIDTH-1:0]     cm5_o_plain_data  = mtx_o_hwdatam5[31:0];

wire [CS0_p_DATA_WIDTH-1:0] cs0_plain_data = mtx_o_data_dft0 ? 32'h0 : cs0_o_plain_data;
wire [CS1_p_DATA_WIDTH-1:0] cs1_plain_data = mtx_o_data_dft1 ? 32'h0 : cs1_o_plain_data;
wire [CS2_p_DATA_WIDTH-1:0] cs2_plain_data = mtx_o_data_dft2 ? 32'h0 : cs2_o_plain_data;
wire [CS3_p_DATA_WIDTH-1:0] cs3_plain_data = mtx_o_data_dft3 ? 32'h0 : cs3_o_plain_data;
wire [CS4_p_DATA_WIDTH-1:0] cs4_plain_data = mtx_o_data_dft4 ? 32'h0 : cs4_o_plain_data;

wire                                  mtx_hclk          = i_hclk                                           ;
wire                                  mtx_hresetn       = i_hresetn                                        ;
wire                                  mtx_i_hsels0      = i_hsels0                                         ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_i_haddrs0     = {i_haddrs0[31:14],cs0_o_bus_addr,i_haddrs0[1:0]} ;
wire [1:0]                            mtx_i_htranss0    = i_htranss0                                       ;
wire                                  mtx_i_hwrites0    = i_hwrites0                                       ;
wire [2:0]                            mtx_i_hsizes0     = i_hsizes0                                        ;
wire [2:0]                            mtx_i_hbursts0    = i_hbursts0                                       ;
wire [3:0]                            mtx_i_hprots0     = i_hprots0                                        ;
wire [7:0]                            mtx_i_hmasters0   = i_hmasters0                                      ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hwdatas0    = {i_hwdatas0[35:32],cs0_o_cipher_data}             ;
wire                                  mtx_i_hmastlocks0 = i_hmastlocks0                                    ;
wire                                  mtx_i_hreadys0    = mtx_o_hreadyouts0                                ;
wire                                  mtx_i_hsels1      = i_hsels1                                         ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_i_haddrs1     = {i_haddrs1[31:14],cs1_o_bus_addr,i_haddrs1[1:0]} ;
wire [1:0]                            mtx_i_htranss1    = i_htranss1                                       ;
wire                                  mtx_i_hwrites1    = i_hwrites1                                       ;
wire [2:0]                            mtx_i_hsizes1     = i_hsizes1                                        ;
wire [2:0]                            mtx_i_hbursts1    = i_hbursts1                                       ;
wire [3:0]                            mtx_i_hprots1     = i_hprots1                                        ;
wire [7:0]                            mtx_i_hmasters1   = i_hmasters1                                      ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hwdatas1    = {i_hwdatas1[35:32],cs1_o_cipher_data}             ;
wire                                  mtx_i_hmastlocks1 = i_hmastlocks1                                    ;
wire                                  mtx_i_hreadys1    = mtx_o_hreadyouts1                                ;
wire                                  mtx_i_hsels2      = i_hsels2                                         ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_i_haddrs2     = {i_haddrs2[31:14],cs2_o_bus_addr,i_haddrs2[1:0]} ;
wire [1:0]                            mtx_i_htranss2    = i_htranss2                                       ;
wire                                  mtx_i_hwrites2    = i_hwrites2                                       ;
wire [2:0]                            mtx_i_hsizes2     = i_hsizes2                                        ;
wire [2:0]                            mtx_i_hbursts2    = i_hbursts2                                       ;
wire [3:0]                            mtx_i_hprots2     = i_hprots2                                        ;
wire [7:0]                            mtx_i_hmasters2   = i_hmasters2                                      ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hwdatas2    = {i_hwdatas2[35:32],cs2_o_cipher_data}             ;
wire                                  mtx_i_hmastlocks2 = i_hmastlocks2                                    ;
wire                                  mtx_i_hreadys2    = mtx_o_hreadyouts2                                ;
wire                                  mtx_i_hsels3      = i_hsels3                                         ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_i_haddrs3     = {i_haddrs3[31:14],cs3_o_bus_addr,i_haddrs3[1:0]} ;
wire [1:0]                            mtx_i_htranss3    = i_htranss3                                       ;
wire                                  mtx_i_hwrites3    = i_hwrites3                                       ;
wire [2:0]                            mtx_i_hsizes3     = i_hsizes3                                        ;
wire [2:0]                            mtx_i_hbursts3    = i_hbursts3                                       ;
wire [3:0]                            mtx_i_hprots3     = i_hprots3                                        ;
wire [7:0]                            mtx_i_hmasters3   = i_hmasters3                                      ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hwdatas3    = {i_hwdatas3[35:32],cs3_o_cipher_data}             ;
wire                                  mtx_i_hmastlocks3 = i_hmastlocks3                                    ;
wire                                  mtx_i_hreadys3    = mtx_o_hreadyouts3                                ;
wire                                  mtx_i_hsels4      = i_hsels4                                         ;
wire [MTX_P_BUS_MATRIX_AWIDTH-1:0]    mtx_i_haddrs4     = {i_haddrs4[31:14],cs4_o_bus_addr,i_haddrs4[1:0]} ;
wire [1:0]                            mtx_i_htranss4    = i_htranss4                                       ;
wire                                  mtx_i_hwrites4    = i_hwrites4                                       ;
wire [2:0]                            mtx_i_hsizes4     = i_hsizes4                                        ;
wire [2:0]                            mtx_i_hbursts4    = i_hbursts4                                       ;
wire [3:0]                            mtx_i_hprots4     = i_hprots4                                        ;
wire [7:0]                            mtx_i_hmasters4   = i_hmasters4                                      ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hwdatas4    = {i_hwdatas4[35:32],cs4_o_cipher_data}             ;
wire                                  mtx_i_hmastlocks4 = i_hmastlocks4                                    ;
wire                                  mtx_i_hreadys4    = mtx_o_hreadyouts4                                ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hrdatam0    = {i_hrdatam0[35:32],cm0_o_cipher_data}             ;
wire                                  mtx_i_hreadyoutm0 = i_hreadyoutm0                                     ;
wire [1:0]                            mtx_i_hrespm0     = i_hrespm0                                         ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hrdatam1    = {i_hrdatam1[35:32],cm1_o_cipher_data}             ;
wire                                  mtx_i_hreadyoutm1 = i_hreadyoutm1                                     ;
wire [1:0]                            mtx_i_hrespm1     = i_hrespm1                                         ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hrdatam2    = {i_hrdatam2[35:32],cm2_o_cipher_data}             ;
wire                                  mtx_i_hreadyoutm2 = i_hreadyoutm2                                     ;
wire [1:0]                            mtx_i_hrespm2     = i_hrespm2                                         ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hrdatam3    = {i_hrdatam3[35:32],cm3_o_cipher_data}             ;
wire                                  mtx_i_hreadyoutm3 = i_hreadyoutm3                                     ;
wire [1:0]                            mtx_i_hrespm3     = i_hrespm3                                         ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hrdatam4    = {i_hrdatam4[35:32],cm4_o_cipher_data}             ;
wire                                  mtx_i_hreadyoutm4 = i_hreadyoutm4                                     ;
wire [1:0]                            mtx_i_hrespm4     = i_hrespm4                                         ;
wire [MTX_P_BUS_MATRIX_DWIDTH-1:0]    mtx_i_hrdatam5    = {i_hrdatam5[35:32],cm5_o_cipher_data}             ;
wire                                  mtx_i_hreadyoutm5 = i_hreadyoutm5                                     ;
wire [1:0]                            mtx_i_hrespm5     = i_hrespm5                                         ;

assign o_hrdatas0    = {mtx_o_hrdatas0[35:32],cs0_plain_data}                     ;
assign o_hreadyouts0 = mtx_o_hreadyouts0                                          ;
assign o_hresps0     = mtx_o_hresps0                                              ;
assign o_hrdatas1    = {mtx_o_hrdatas1[35:32],cs1_plain_data}                     ;
assign o_hreadyouts1 = mtx_o_hreadyouts1                                          ;
assign o_hresps1     = mtx_o_hresps1                                              ;
assign o_hrdatas2    = {mtx_o_hrdatas2[35:32],cs2_plain_data}                     ;
assign o_hreadyouts2 = mtx_o_hreadyouts2                                          ;
assign o_hresps2     = mtx_o_hresps2                                              ;
assign o_hrdatas3    = {mtx_o_hrdatas3[35:32],cs3_plain_data}                     ;
assign o_hreadyouts3 = mtx_o_hreadyouts3                                          ;
assign o_hresps3     = mtx_o_hresps3                                              ;
assign o_hrdatas4    = {mtx_o_hrdatas4[35:32],cs4_plain_data}                     ;
assign o_hreadyouts4 = mtx_o_hreadyouts4                                          ;
assign o_hresps4     = mtx_o_hresps4                                              ;
assign o_hselm0      = mtx_o_hselm0                                               ;
assign o_haddrm0     = {mtx_o_haddrm0[31:14],cm0_o_bus_addr,mtx_o_haddrm0[1:0]}   ;
assign o_htransm0    = mtx_o_htransm0                                             ;
assign o_hwritem0    = mtx_o_hwritem0                                             ;
assign o_hsizem0     = mtx_o_hsizem0                                              ;
assign o_hburstm0    = mtx_o_hburstm0                                             ;
assign o_hprotm0     = mtx_o_hprotm0                                              ;
assign o_hmasterm0   = mtx_o_hmasterm0                                            ;
assign o_hwdatam0    = {mtx_o_hwdatam0[35:32],cm0_o_plain_data}                   ;
assign o_hmastlockm0 = mtx_o_hmastlockm0                                          ;
assign o_hreadymuxm0 = mtx_o_hreadymuxm0                                          ;
assign o_hselm1      = mtx_o_hselm1                                               ;
assign o_haddrm1     = {mtx_o_haddrm1[31:14],cm1_o_bus_addr,mtx_o_haddrm1[1:0]}   ;
assign o_htransm1    = mtx_o_htransm1                                             ;
assign o_hwritem1    = mtx_o_hwritem1                                             ;
assign o_hsizem1     = mtx_o_hsizem1                                              ;
assign o_hburstm1    = mtx_o_hburstm1                                             ;
assign o_hprotm1     = mtx_o_hprotm1                                              ;
assign o_hmasterm1   = mtx_o_hmasterm1                                            ;
assign o_hwdatam1    = {mtx_o_hwdatam1[35:32],cm1_o_plain_data}                   ;
assign o_hmastlockm1 = mtx_o_hmastlockm1                                          ;
assign o_hreadymuxm1 = mtx_o_hreadymuxm1                                          ;
assign o_hselm2      = mtx_o_hselm2                                               ;
assign o_haddrm2     = {mtx_o_haddrm2[31:14],cm2_o_bus_addr,mtx_o_haddrm2[1:0]}   ;
assign o_htransm2    = mtx_o_htransm2                                             ;
assign o_hwritem2    = mtx_o_hwritem2                                             ;
assign o_hsizem2     = mtx_o_hsizem2                                              ;
assign o_hburstm2    = mtx_o_hburstm2                                             ;
assign o_hprotm2     = mtx_o_hprotm2                                              ;
assign o_hmasterm2   = mtx_o_hmasterm2                                            ;
assign o_hwdatam2    = {mtx_o_hwdatam2[35:32],cm2_o_plain_data}                   ;
assign o_hmastlockm2 = mtx_o_hmastlockm2                                          ;
assign o_hreadymuxm2 = mtx_o_hreadymuxm2                                          ;
assign o_hselm3      = mtx_o_hselm3                                               ;
assign o_haddrm3     = {mtx_o_haddrm3[31:14],cm3_o_bus_addr,mtx_o_haddrm3[1:0]}   ;
assign o_htransm3    = mtx_o_htransm3                                             ;
assign o_hwritem3    = mtx_o_hwritem3                                             ;
assign o_hsizem3     = mtx_o_hsizem3                                              ;
assign o_hburstm3    = mtx_o_hburstm3                                             ;
assign o_hprotm3     = mtx_o_hprotm3                                              ;
assign o_hmasterm3   = mtx_o_hmasterm3                                            ;
assign o_hwdatam3    = {mtx_o_hwdatam3[35:32],cm3_o_plain_data}                   ; 
assign o_hmastlockm3 = mtx_o_hmastlockm3                                          ;
assign o_hreadymuxm3 = mtx_o_hreadymuxm3                                          ;
assign o_hselm4      = mtx_o_hselm4                                               ;
assign o_haddrm4     = {mtx_o_haddrm4[31:14],cm4_o_bus_addr,mtx_o_haddrm4[1:0]}   ;
assign o_htransm4    = mtx_o_htransm4                                             ;
assign o_hwritem4    = mtx_o_hwritem4                                             ;
assign o_hsizem4     = mtx_o_hsizem4                                              ;
assign o_hburstm4    = mtx_o_hburstm4                                             ;
assign o_hprotm4     = mtx_o_hprotm4                                              ;
assign o_hmasterm4   = mtx_o_hmasterm4                                            ;
assign o_hwdatam4    = {mtx_o_hwdatam4[35:32],cm4_o_plain_data}                   ;
assign o_hmastlockm4 = mtx_o_hmastlockm4                                          ;
assign o_hreadymuxm4 = mtx_o_hreadymuxm4                                          ;
assign o_hselm5      = mtx_o_hselm5                                               ;
assign o_haddrm5     = {mtx_o_haddrm5[31:14],cm5_o_bus_addr,mtx_o_haddrm5[1:0]}   ;
assign o_htransm5    = mtx_o_htransm5                                             ;
assign o_hwritem5    = mtx_o_hwritem5                                             ;
assign o_hsizem5     = mtx_o_hsizem5                                              ;
assign o_hburstm5    = mtx_o_hburstm5                                             ;
assign o_hprotm5     = mtx_o_hprotm5                                              ;
assign o_hmasterm5   = mtx_o_hmasterm5                                            ;
assign o_hwdatam5    = {mtx_o_hwdatam5[35:32],cm5_o_plain_data}                   ;
assign o_hmastlockm5 = mtx_o_hmastlockm5                                          ;
assign o_hreadymuxm5 = mtx_o_hreadymuxm5                                          ;

osr_ahb_busmatrix_top #(
    .P_BUS_MATRIX_AWIDTH       ( MTX_P_BUS_MATRIX_AWIDTH   ), 
    .P_BUS_MATRIX_DWIDTH       ( MTX_P_BUS_MATRIX_DWIDTH   ), 
    .P_BUS_MATRIX_DELAY_EN     ( MTX_P_BUS_MATRIX_DELAY_EN )  
    ) u_osr_ahb_busmatrix_top (
    .hclk             ( mtx_hclk          ), 
    .hresetn          ( mtx_hresetn       ), 
    .i_hsels0         ( mtx_i_hsels0      ), 
    .i_haddrs0        ( mtx_i_haddrs0     ), 
    .i_htranss0       ( mtx_i_htranss0    ), 
    .i_hwrites0       ( mtx_i_hwrites0    ), 
    .i_hsizes0        ( mtx_i_hsizes0     ), 
    .i_hbursts0       ( mtx_i_hbursts0    ), 
    .i_hprots0        ( mtx_i_hprots0     ), 
    .i_hmasters0      ( mtx_i_hmasters0   ), 
    .i_hwdatas0       ( mtx_i_hwdatas0    ), 
    .i_hmastlocks0    ( mtx_i_hmastlocks0 ), 
    .i_hreadys0       ( mtx_i_hreadys0    ), 
    .o_data_dft0      ( mtx_o_data_dft0   ), 
    .o_hrdatas0       ( mtx_o_hrdatas0    ), 
    .o_hreadyouts0    ( mtx_o_hreadyouts0 ), 
    .o_hresps0        ( mtx_o_hresps0     ), 
    .i_hsels1         ( mtx_i_hsels1      ), 
    .i_haddrs1        ( mtx_i_haddrs1     ), 
    .i_htranss1       ( mtx_i_htranss1    ), 
    .i_hwrites1       ( mtx_i_hwrites1    ), 
    .i_hsizes1        ( mtx_i_hsizes1     ), 
    .i_hbursts1       ( mtx_i_hbursts1    ), 
    .i_hprots1        ( mtx_i_hprots1     ), 
    .i_hmasters1      ( mtx_i_hmasters1   ), 
    .i_hwdatas1       ( mtx_i_hwdatas1    ), 
    .i_hmastlocks1    ( mtx_i_hmastlocks1 ), 
    .i_hreadys1       ( mtx_i_hreadys1    ), 
    .o_data_dft1      ( mtx_o_data_dft1   ), 
    .o_hrdatas1       ( mtx_o_hrdatas1    ), 
    .o_hreadyouts1    ( mtx_o_hreadyouts1 ), 
    .o_hresps1        ( mtx_o_hresps1     ), 
    .i_hsels2         ( mtx_i_hsels2      ), 
    .i_haddrs2        ( mtx_i_haddrs2     ), 
    .i_htranss2       ( mtx_i_htranss2    ), 
    .i_hwrites2       ( mtx_i_hwrites2    ), 
    .i_hsizes2        ( mtx_i_hsizes2     ), 
    .i_hbursts2       ( mtx_i_hbursts2    ), 
    .i_hprots2        ( mtx_i_hprots2     ), 
    .i_hmasters2      ( mtx_i_hmasters2   ), 
    .i_hwdatas2       ( mtx_i_hwdatas2    ), 
    .i_hmastlocks2    ( mtx_i_hmastlocks2 ), 
    .i_hreadys2       ( mtx_i_hreadys2    ), 
    .o_data_dft2      ( mtx_o_data_dft2   ), 
    .o_hrdatas2       ( mtx_o_hrdatas2    ), 
    .o_hreadyouts2    ( mtx_o_hreadyouts2 ), 
    .o_hresps2        ( mtx_o_hresps2     ), 
    .i_hsels3         ( mtx_i_hsels3      ), 
    .i_haddrs3        ( mtx_i_haddrs3     ), 
    .i_htranss3       ( mtx_i_htranss3    ), 
    .i_hwrites3       ( mtx_i_hwrites3    ), 
    .i_hsizes3        ( mtx_i_hsizes3     ), 
    .i_hbursts3       ( mtx_i_hbursts3    ), 
    .i_hprots3        ( mtx_i_hprots3     ), 
    .i_hmasters3      ( mtx_i_hmasters3   ), 
    .i_hwdatas3       ( mtx_i_hwdatas3    ), 
    .i_hmastlocks3    ( mtx_i_hmastlocks3 ), 
    .i_hreadys3       ( mtx_i_hreadys3    ), 
    .o_data_dft3      ( mtx_o_data_dft3   ), 
    .o_hrdatas3       ( mtx_o_hrdatas3    ), 
    .o_hreadyouts3    ( mtx_o_hreadyouts3 ), 
    .o_hresps3        ( mtx_o_hresps3     ), 
    .i_hsels4         ( mtx_i_hsels4      ), 
    .i_haddrs4        ( mtx_i_haddrs4     ), 
    .i_htranss4       ( mtx_i_htranss4    ), 
    .i_hwrites4       ( mtx_i_hwrites4    ), 
    .i_hsizes4        ( mtx_i_hsizes4     ), 
    .i_hbursts4       ( mtx_i_hbursts4    ), 
    .i_hprots4        ( mtx_i_hprots4     ), 
    .i_hmasters4      ( mtx_i_hmasters4   ), 
    .i_hwdatas4       ( mtx_i_hwdatas4    ), 
    .i_hmastlocks4    ( mtx_i_hmastlocks4 ), 
    .i_hreadys4       ( mtx_i_hreadys4    ), 
    .o_data_dft4      ( mtx_o_data_dft4   ), 
    .o_hrdatas4       ( mtx_o_hrdatas4    ), 
    .o_hreadyouts4    ( mtx_o_hreadyouts4 ), 
    .o_hresps4        ( mtx_o_hresps4     ), 
    .o_hselm0         ( mtx_o_hselm0      ), 
    .o_haddrm0        ( mtx_o_haddrm0     ), 
    .o_htransm0       ( mtx_o_htransm0    ), 
    .o_hwritem0       ( mtx_o_hwritem0    ), 
    .o_hsizem0        ( mtx_o_hsizem0     ), 
    .o_hburstm0       ( mtx_o_hburstm0    ), 
    .o_hprotm0        ( mtx_o_hprotm0     ), 
    .o_hmasterm0      ( mtx_o_hmasterm0   ), 
    .o_hwdatam0       ( mtx_o_hwdatam0    ), 
    .o_hmastlockm0    ( mtx_o_hmastlockm0 ), 
    .o_hreadymuxm0    ( mtx_o_hreadymuxm0 ), 
    .i_hrdatam0       ( mtx_i_hrdatam0    ), 
    .i_hreadyoutm0    ( mtx_i_hreadyoutm0 ), 
    .i_hrespm0        ( mtx_i_hrespm0     ), 
    .o_hselm1         ( mtx_o_hselm1      ), 
    .o_haddrm1        ( mtx_o_haddrm1     ), 
    .o_htransm1       ( mtx_o_htransm1    ), 
    .o_hwritem1       ( mtx_o_hwritem1    ), 
    .o_hsizem1        ( mtx_o_hsizem1     ), 
    .o_hburstm1       ( mtx_o_hburstm1    ), 
    .o_hprotm1        ( mtx_o_hprotm1     ), 
    .o_hmasterm1      ( mtx_o_hmasterm1   ), 
    .o_hwdatam1       ( mtx_o_hwdatam1    ), 
    .o_hmastlockm1    ( mtx_o_hmastlockm1 ), 
    .o_hreadymuxm1    ( mtx_o_hreadymuxm1 ), 
    .i_hrdatam1       ( mtx_i_hrdatam1    ), 
    .i_hreadyoutm1    ( mtx_i_hreadyoutm1 ), 
    .i_hrespm1        ( mtx_i_hrespm1     ), 
    .o_hselm2         ( mtx_o_hselm2      ), 
    .o_haddrm2        ( mtx_o_haddrm2     ), 
    .o_htransm2       ( mtx_o_htransm2    ), 
    .o_hwritem2       ( mtx_o_hwritem2    ), 
    .o_hsizem2        ( mtx_o_hsizem2     ), 
    .o_hburstm2       ( mtx_o_hburstm2    ), 
    .o_hprotm2        ( mtx_o_hprotm2     ), 
    .o_hmasterm2      ( mtx_o_hmasterm2   ), 
    .o_hwdatam2       ( mtx_o_hwdatam2    ), 
    .o_hmastlockm2    ( mtx_o_hmastlockm2 ), 
    .o_hreadymuxm2    ( mtx_o_hreadymuxm2 ), 
    .i_hrdatam2       ( mtx_i_hrdatam2    ), 
    .i_hreadyoutm2    ( mtx_i_hreadyoutm2 ), 
    .i_hrespm2        ( mtx_i_hrespm2     ), 
    .o_hselm3         ( mtx_o_hselm3      ), 
    .o_haddrm3        ( mtx_o_haddrm3     ), 
    .o_htransm3       ( mtx_o_htransm3    ), 
    .o_hwritem3       ( mtx_o_hwritem3    ), 
    .o_hsizem3        ( mtx_o_hsizem3     ), 
    .o_hburstm3       ( mtx_o_hburstm3    ), 
    .o_hprotm3        ( mtx_o_hprotm3     ), 
    .o_hmasterm3      ( mtx_o_hmasterm3   ), 
    .o_hwdatam3       ( mtx_o_hwdatam3    ), 
    .o_hmastlockm3    ( mtx_o_hmastlockm3 ), 
    .o_hreadymuxm3    ( mtx_o_hreadymuxm3 ), 
    .i_hrdatam3       ( mtx_i_hrdatam3    ), 
    .i_hreadyoutm3    ( mtx_i_hreadyoutm3 ), 
    .i_hrespm3        ( mtx_i_hrespm3     ), 
    .o_hselm4         ( mtx_o_hselm4      ), 
    .o_haddrm4        ( mtx_o_haddrm4     ), 
    .o_htransm4       ( mtx_o_htransm4    ), 
    .o_hwritem4       ( mtx_o_hwritem4    ), 
    .o_hsizem4        ( mtx_o_hsizem4     ), 
    .o_hburstm4       ( mtx_o_hburstm4    ), 
    .o_hprotm4        ( mtx_o_hprotm4     ), 
    .o_hmasterm4      ( mtx_o_hmasterm4   ), 
    .o_hwdatam4       ( mtx_o_hwdatam4    ), 
    .o_hmastlockm4    ( mtx_o_hmastlockm4 ), 
    .o_hreadymuxm4    ( mtx_o_hreadymuxm4 ), 
    .i_hrdatam4       ( mtx_i_hrdatam4    ), 
    .i_hreadyoutm4    ( mtx_i_hreadyoutm4 ), 
    .i_hrespm4        ( mtx_i_hrespm4     ), 
    .o_hselm5         ( mtx_o_hselm5      ), 
    .o_haddrm5        ( mtx_o_haddrm5     ), 
    .o_htransm5       ( mtx_o_htransm5    ), 
    .o_hwritem5       ( mtx_o_hwritem5    ), 
    .o_hsizem5        ( mtx_o_hsizem5     ), 
    .o_hburstm5       ( mtx_o_hburstm5    ), 
    .o_hprotm5        ( mtx_o_hprotm5     ), 
    .o_hmasterm5      ( mtx_o_hmasterm5   ), 
    .o_hwdatam5       ( mtx_o_hwdatam5    ), 
    .o_hmastlockm5    ( mtx_o_hmastlockm5 ), 
    .o_hreadymuxm5    ( mtx_o_hreadymuxm5 ), 
    .i_hrdatam5       ( mtx_i_hrdatam5    ), 
    .i_hreadyoutm5    ( mtx_i_hreadyoutm5 ), 
    .i_hrespm5        ( mtx_i_hrespm5     )  
    ); 

endmodule
