//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_source
#(
    parameter CLK_INV_N = 97 ,
`ifdef OSR_TRNG_FPGA 
    parameter p_CONFIG_FPGA=0,
`endif    
    parameter p_IDX = 0,
    parameter delay=25
)(
    input   wire            clk     ,
`ifdef OSR_TRNG_FPGA 
    input   wire   [ 1: 0]  osc_en_clk,
`endif        
    input   wire            resetn  ,
    output  wire            ro_rst  ,
    input   wire            i_scan  ,

    input   wire            i_rosen ,
    input   wire    [ 2: 0] i_conf  ,
    input   wire    [15: 0] i_roen  ,

    output  wire    [31: 0] o_rand  ,
    input   wire            i_flag  ,
    output  wire            o_flag  ,

`ifdef OSR_TRNG_FPGA 
    output  wire            o_en_roclk,
`endif        
    output  wire            o_rornd ,       
    output  wire            o_roclk       
);

    wire    [15: 0] w_rbito        ;
    wire            w_clk          ;
`ifdef OSR_TRNG_FPGA 
    wire            w_en_clk       ;
    wire            w_en_sync_clk  ;
    wire    [15: 0] w_roen_sync1   ;    
`endif        
    wire            w_sync_clk     ;
    wire            ro_rstn        ;
    wire            w_rst_n        ;
    wire            w_rstn_sync1   ;

`ifdef OSR_TRNG_FPGA  
`ifdef OSR_TRNG_SIM
    ro_wrap  #(.p_IDX(16*p_IDX+ 0),.INV_N(17),.delay(810),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_01(.en(i_roen[ 0]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 0]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 0]));        
    ro_wrap  #(.p_IDX(16*p_IDX+ 1),.INV_N(23),.delay(830),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_02(.en(i_roen[ 1]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 1]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 1]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 2),.INV_N(19),.delay(570),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_03(.en(i_roen[ 2]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 2]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 2]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 3),.INV_N(25),.delay(870),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_04(.en(i_roen[ 3]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 3]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 3]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 4),.INV_N(21),.delay(890),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_05(.en(i_roen[ 4]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 4]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 4]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 5),.INV_N(17),.delay(710),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_06(.en(i_roen[ 5]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 5]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 5]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 6),.INV_N(23),.delay(730),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_07(.en(i_roen[ 6]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 6]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 6]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 7),.INV_N(19),.delay(590),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_08(.en(i_roen[ 7]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 7]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 7]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 8),.INV_N(25),.delay(770),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_09(.en(i_roen[ 8]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 8]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 8]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 9),.INV_N(21),.delay(790),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_10(.en(i_roen[ 9]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 9]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 9]));
    ro_wrap  #(.p_IDX(16*p_IDX+10),.INV_N(17),.delay(610),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_11(.en(i_roen[10]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[10]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[10]));
    ro_wrap  #(.p_IDX(16*p_IDX+11),.INV_N(23),.delay(630),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_12(.en(i_roen[11]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[11]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[11]));
    ro_wrap  #(.p_IDX(16*p_IDX+12),.INV_N(19),.delay(670),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_13(.en(i_roen[12]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[12]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[12]));
    ro_wrap  #(.p_IDX(16*p_IDX+13),.INV_N(25),.delay(690),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_14(.en(i_roen[13]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[13]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[13]));
    ro_wrap  #(.p_IDX(16*p_IDX+14),.INV_N(21),.delay(510),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_15(.en(i_roen[14]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[14]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[14]));
    ro_wrap  #(.p_IDX(16*p_IDX+15),.INV_N(17),.delay(530),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_16(.en(i_roen[15]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[15]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[15]));        
`else
    ro_wrap  #(.p_IDX(16*p_IDX+ 0),.INV_N(17),.delay(11),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_01(.en(i_roen[ 0]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 0]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 0]));        
    ro_wrap  #(.p_IDX(16*p_IDX+ 1),.INV_N(23),.delay(12),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_02(.en(i_roen[ 1]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 1]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 1]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 2),.INV_N(19),.delay(13),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_03(.en(i_roen[ 2]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 2]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 2]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 3),.INV_N(25),.delay(14),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_04(.en(i_roen[ 3]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 3]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 3]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 4),.INV_N(21),.delay(15),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_05(.en(i_roen[ 4]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 4]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 4]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 5),.INV_N(17),.delay(16),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_06(.en(i_roen[ 5]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 5]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 5]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 6),.INV_N(23),.delay(14),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_07(.en(i_roen[ 6]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 6]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 6]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 7),.INV_N(19),.delay(15),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_08(.en(i_roen[ 7]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 7]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 7]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 8),.INV_N(25),.delay(16),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_09(.en(i_roen[ 8]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 8]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 8]));
    ro_wrap  #(.p_IDX(16*p_IDX+ 9),.INV_N(21),.delay(17),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_10(.en(i_roen[ 9]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[ 9]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 9]));
    ro_wrap  #(.p_IDX(16*p_IDX+10),.INV_N(17),.delay(18),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_11(.en(i_roen[10]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[10]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[10]));
    ro_wrap  #(.p_IDX(16*p_IDX+11),.INV_N(23),.delay(19),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_12(.en(i_roen[11]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[11]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[11]));
    ro_wrap  #(.p_IDX(16*p_IDX+12),.INV_N(19),.delay(21),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_13(.en(i_roen[12]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[12]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[12]));
    ro_wrap  #(.p_IDX(16*p_IDX+13),.INV_N(25),.delay(22),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_14(.en(i_roen[13]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[13]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[13]));
    ro_wrap  #(.p_IDX(16*p_IDX+14),.INV_N(21),.delay(23),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_15(.en(i_roen[14]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[14]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[14]));
    ro_wrap  #(.p_IDX(16*p_IDX+15),.INV_N(17),.delay(27),.p_CONFIG_FPGA(p_CONFIG_FPGA))ro_16(.en(i_roen[15]),.clk(w_clk),.i_en_clk(w_en_clk),.i_dft_clk_en(w_roen_sync1[15]),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[15]));        
`endif 
`else
`ifdef OSR_TRNG_SIM
    ro_circuit  #(.p_IDX(16*p_IDX+ 0),.INV_N(17),.delay(810))ro_01(.en(i_roen[ 0]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 0]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 1),.INV_N(23),.delay(830))ro_02(.en(i_roen[ 1]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 1]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 2),.INV_N(19),.delay(570))ro_03(.en(i_roen[ 2]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 2]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 3),.INV_N(25),.delay(870))ro_04(.en(i_roen[ 3]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 3]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 4),.INV_N(21),.delay(890))ro_05(.en(i_roen[ 4]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 4]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 5),.INV_N(17),.delay(710))ro_06(.en(i_roen[ 5]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 5]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 6),.INV_N(23),.delay(730))ro_07(.en(i_roen[ 6]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 6]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 7),.INV_N(19),.delay(590))ro_08(.en(i_roen[ 7]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 7]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 8),.INV_N(25),.delay(770))ro_09(.en(i_roen[ 8]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 8]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 9),.INV_N(21),.delay(790))ro_10(.en(i_roen[ 9]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 9]));
    ro_circuit  #(.p_IDX(16*p_IDX+10),.INV_N(17),.delay(610))ro_11(.en(i_roen[10]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[10]));
    ro_circuit  #(.p_IDX(16*p_IDX+11),.INV_N(23),.delay(630))ro_12(.en(i_roen[11]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[11]));
    ro_circuit  #(.p_IDX(16*p_IDX+12),.INV_N(19),.delay(670))ro_13(.en(i_roen[12]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[12]));
    ro_circuit  #(.p_IDX(16*p_IDX+13),.INV_N(25),.delay(690))ro_14(.en(i_roen[13]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[13]));
    ro_circuit  #(.p_IDX(16*p_IDX+14),.INV_N(21),.delay(510))ro_15(.en(i_roen[14]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[14]));
    ro_circuit  #(.p_IDX(16*p_IDX+15),.INV_N(17),.delay(530))ro_16(.en(i_roen[15]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[15]));
`else
    ro_circuit  #(.p_IDX(16*p_IDX+ 0),.INV_N(17),.delay(11))ro_01(.en(i_roen[ 0]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 0]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 1),.INV_N(23),.delay(12))ro_02(.en(i_roen[ 1]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 1]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 2),.INV_N(19),.delay(13))ro_03(.en(i_roen[ 2]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 2]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 3),.INV_N(25),.delay(14))ro_04(.en(i_roen[ 3]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 3]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 4),.INV_N(21),.delay(15))ro_05(.en(i_roen[ 4]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 4]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 5),.INV_N(17),.delay(16))ro_06(.en(i_roen[ 5]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 5]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 6),.INV_N(23),.delay(14))ro_07(.en(i_roen[ 6]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 6]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 7),.INV_N(19),.delay(15))ro_08(.en(i_roen[ 7]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 7]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 8),.INV_N(25),.delay(16))ro_09(.en(i_roen[ 8]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 8]));
    ro_circuit  #(.p_IDX(16*p_IDX+ 9),.INV_N(21),.delay(17))ro_10(.en(i_roen[ 9]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[ 9]));
    ro_circuit  #(.p_IDX(16*p_IDX+10),.INV_N(17),.delay(18))ro_11(.en(i_roen[10]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[10]));
    ro_circuit  #(.p_IDX(16*p_IDX+11),.INV_N(23),.delay(19))ro_12(.en(i_roen[11]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[11]));
    ro_circuit  #(.p_IDX(16*p_IDX+12),.INV_N(19),.delay(21))ro_13(.en(i_roen[12]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[12]));
    ro_circuit  #(.p_IDX(16*p_IDX+13),.INV_N(25),.delay(22))ro_14(.en(i_roen[13]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[13]));
    ro_circuit  #(.p_IDX(16*p_IDX+14),.INV_N(21),.delay(23))ro_15(.en(i_roen[14]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[14]));
    ro_circuit  #(.p_IDX(16*p_IDX+15),.INV_N(17),.delay(27))ro_16(.en(i_roen[15]),.clk(w_clk),.rstn(w_rst_n),.scan(i_scan),.o_rout(w_rbito[15]));
`endif 
`endif

    ro_clk  
`ifdef OSR_TRNG_FPGA  
    #(.p_IDX(p_IDX),.INV_N(CLK_INV_N),.delay(delay),.p_CONFIG_FPGA(p_CONFIG_FPGA))
`else    
    #(.p_IDX(p_IDX),.INV_N(CLK_INV_N),.delay(delay))
`endif    
        roclk(
        .clk        (clk        ),
    `ifdef OSR_TRNG_FPGA  
        .osc_en_clk(osc_en_clk),
    `endif        
        .resetn     (w_rst_n    ),
        .i_enable   (i_rosen    ),
        .scan       (i_scan     ),
        .i_config   (i_conf     ),
    `ifdef OSR_TRNG_FPGA  
        .o_en_clk    (w_en_sync_clk ),
        .o_en_clk_div(w_en_clk      ),
    `endif            
        .o_clk      (w_sync_clk ),
        .o_clk_div  (w_clk      )
    );

    ro_process
`ifdef OSR_TRNG_FPGA 
    #(.p_RO_DELAY(16),.p_CONFIG_FPGA(p_CONFIG_FPGA))  
`else
    #(.p_RO_DELAY(16))  
`endif    
    ro_process(
        .clk     (w_clk     ),
    `ifdef OSR_TRNG_FPGA  
        .i_en_clk(w_en_clk  ),
    `endif            
        .resetn  (w_rst_n   ),
        .i_rbit  (w_rbito   ),
        .i_flag  (i_flag    ),
        .o_flag  (o_flag    ),
        .o_rand  (o_rand    ),
        .o_rornd (o_rornd   )
    );

    wire sel_rst_01 =  resetn & i_rosen ;
    osr_mux2 rst_mux_01(.A(sel_rst_01), .B(resetn), .S(i_scan), .Z(ro_rstn));

    osr_sync #(.p_DWIDTH(1))
    rstn_sync(
        .i_clk  (w_sync_clk     ), 
        .i_rst_n(ro_rstn        ), 
        .i_in   (1'b1           ), 
        .o_out  (w_rstn_sync1   ) 
    );

`ifdef OSR_TRNG_FPGA 
generate
    if (!p_CONFIG_FPGA) begin : sync

        assign  o_en_roclk = 1'b0;    
        assign w_roen_sync1 = 16'h0;                

    end else begin : fpga_sync

    osr_sync #(.p_DWIDTH(16))
    roen_sync(
        .i_clk  (w_sync_clk     ), 
        .i_rst_n(ro_rstn        ), 
        .i_in   (i_roen         ), 
        .o_out  (w_roen_sync1   ) 
    );

        assign  o_en_roclk = w_en_clk;            

    end
endgenerate   

`endif        

    wire sel_rst_02 = w_rstn_sync1 & resetn ;
    osr_mux2 rst_mux_02(.A(sel_rst_02), .B(resetn), .S(i_scan), .Z(w_rst_n));

    assign  ro_rst  = w_rst_n   ;

    assign  o_roclk =  w_clk    ;

endmodule
