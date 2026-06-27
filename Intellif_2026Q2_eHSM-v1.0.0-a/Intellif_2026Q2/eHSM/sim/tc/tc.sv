//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
localparam [63:0] SOFT_RSTN      = 64'h4001_0008;   

localparam [63:0] UART_DATA      = 64'h5000_4000;   
localparam [63:0] UART_STATE     = 64'h5000_4004;   
localparam [63:0] UART_CTRL      = 64'h5000_4008;   
localparam [63:0] UART_INTSTATUS = 64'h5000_400C;   
localparam [63:0] UART_BAUDDIV   = 64'h5000_4010;   

localparam [63:0] MAILBOX_BASE      = 64'h8000_0000;
localparam [63:0] MAILBOX_DEBUG_BASE= 64'h8000_0000;

localparam [63:0] IRAM_CODE_BASE = 64'h6000_2000;

localparam [63:0] OTP_BASE = 64'h6007_C000;

localparam [31:0] MBOX_CMD_BUFFER_L = 32'h6003_1000  ;
localparam [31:0] MBOX_CMD_BUFFER_H = 32'h0          ;
localparam [31:0] MBOX_RSP_BUFFER_L = 32'h6003_2000  ;
localparam [31:0] MBOX_RSP_BUFFER_H = 32'h0          ;
localparam [11:0] KEY_BASE = 12'h118      ;

reg [7:0]    soc_hexfile      [262143 : 0];//256k soc_rom

    reg [38:0]   seip_rom_hexfile [131071  : 0];//512k rom
reg [31:0]   hsm_dfu2 [65535:0]; //64KB 16383

reg [38:0]   seip_ram_hexfile [65535:0]   ;//256k seip_ram
reg [31:0]   soc_coe          [65535:0]   ;
reg [31:0]   seip_coe         [32767:0]   ;

reg [31:0]   nvm_hexfile      [65535:0]   ;

reg [31:0]   otp_hexfile  [1023 : 0];//256k seip_rom

//reuse ext4 as tb AHB master

wire dut_clk      = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.i_clk ;
wire dut_rst_n    = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.i_rst_n ;
wire osc_clk      = tb.u_osr_fpga_top.pll_clk;

//wire pke_sram0_clk = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_pke_sram0_CK;
//wire pke_sram1_clk = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_pke_sram1_CK;
//wire pke_sram2_clk = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_pke_sram2_CK;
//wire pke_sram3_clk = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_pke_sram3_CK;

//upper AHB interface
reg           upper_hsel      =  1'h0 ;
reg  [63:0]   upper_haddr     = 64'h0 ;
reg  [1:0]    upper_htrans    =  2'h0 ;
reg           upper_hwrite    =  1'h0 ;
reg  [2:0]    upper_hsize     =  3'h0 ;
reg  [2:0]    upper_hburst    =  3'h0 ;
reg  [3:0]    upper_hprot     =  4'h0 ;
reg  [3:0]    upper_hmaster   =  4'h0 ;
reg  [31:0]   upper_hwdata    = 32'h0 ;
reg           upper_hmastlock =  1'h0 ;
reg  [2:0]    upper_hauser    =  3'h0 ;
reg  [2:0]    upper_hwuser    =  3'h0 ;
//reg  [31:0]   upper_hrdata    ;
//reg           upper_hreadyout ;
//reg  [1:0]    upper_hresp     ;
//reg  [2:0]    upper_hruser    ;

wire hreadyout_s_ext4   = tb.u_osr_fpga_top.upper_o_hreadyout_s_ext4 ;
wire [31:0] hrdata_s_ext4 = tb.u_osr_fpga_top.upper_o_hrdata_s_ext4 ;
wire o_dram_CK;

reg [31:0]   upper_reg_rdata = 32'h0;
reg [31:0]   upper_reg_wdata = 32'h0;

reg [31:0]   reg_rdata = 32'h0;
reg [31:0]   reg_wdata = 32'h0;

//hsm AHB interface
reg           hsm_hsel      =  1'h0 ;
reg  [31:0]   hsm_haddr     = 32'h0 ;
reg  [1:0]    hsm_htrans    =  2'h0 ;
reg           hsm_hwrite    =  1'h0 ;
reg  [2:0]    hsm_hsize     =  3'h0 ;
reg  [2:0]    hsm_hburst    =  3'h0 ;
reg  [3:0]    hsm_hprot     =  4'h0 ;
reg  [3:0]    hsm_hmaster   =  4'h0 ;
reg  [31:0]   hsm_hwdata    = 32'h0 ;
reg           hsm_hmastlock =  1'h0 ;
reg  [2:0]    hsm_hauser    =  3'h0 ;
reg  [2:0]    hsm_hwuser    =  3'h0 ;
//reg  [31:0]   hsm_hrdata    ;
//reg           hsm_hreadyout ;
//reg  [1:0]    hsm_hresp     ;
//reg  [2:0]    hsm_hruser    ;
wire hsm_hreadyout = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_matrix.o_hreadyouts0 ;
wire [31:0] hsm_hrdata = tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_matrix.o_hrdatas0[31:0] ;

reg [31:0]   hsm_reg_rdata = 32'h0;
reg [31:0]   hsm_reg_wdata = 32'h0;

reg [7:0]    cmd_buf_q[$];
reg [31:0]   result[$]   ;
reg [31:0]   result_add[$];
reg [31:0]   rm_result[$];
string       str = "";

reg          pass_flag =  1'b1;

`include "./case/global_var.sv";

integer     cnt;
integer      i ;

task init_seip();
    //init Boot Information Data
    init_boot_data();

endtask

task init_boot_data();
    repeat(100) @(posedge osc_clk);

    cmd_state = SET_OTP;

    `ifdef OSR_SIM_LC_TEST
    $display("INFO: eHSM Lifecycle = TEST_MODE");
    upper_ahb_wr(OTP_BASE+64'h000,32'h00000000           );
    `elsif OSR_SIM_LC_DEV
    $display("INFO: eHSM Lifecycle = DEVELOP_MODE");
    upper_ahb_wr(OTP_BASE+64'h000,32'h42818414        );
    `elsif OSR_SIM_LC_MANU
    $display("INFO: eHSM Lifecycle = MANUFACTURE_MODE");
    upper_ahb_wr(OTP_BASE+64'h000,32'h46C1A416    );
    `elsif OSR_SIM_LC_USER
    $display("INFO: eHSM Lifecycle = USER_MODE");
    upper_ahb_wr(OTP_BASE+64'h000,32'h57C1EC96           );
    `elsif OSR_SIM_LC_DEBUG
    $display("INFO: eHSM Lifecycle = DEBUG_MODE");
    upper_ahb_wr(OTP_BASE+64'h000,32'hD7C5FCDE          );
    `elsif OSR_SIM_LC_DESTORY
    $display("INFO: eHSM Lifecycle = DESTORY_MODE");
    upper_ahb_wr(OTP_BASE+64'h000,32'hFFFFFFFF        );
    `else
    $display("INFO: eHSM Lifecycle = DEVELOP_MODE");
    upper_ahb_wr(OTP_BASE+64'h000,32'h42818414        );// default = DEVELOP_MODE
    `endif

    upper_ahb_wr(OTP_BASE+64'h004,32'h89abcdef);//UID
    upper_ahb_wr(OTP_BASE+64'h008,32'h10111213);
    upper_ahb_wr(OTP_BASE+64'h00c,32'h10111213);
    upper_ahb_wr(OTP_BASE+64'h010,32'h10111213);
    upper_ahb_wr(OTP_BASE+64'h014,32'h5b7dedf2);//h5b7dedf2

    upper_ahb_wr(OTP_BASE+64'h018,32'hf0fe53aa);//control_fileds
    upper_ahb_wr(OTP_BASE+64'h01c,32'h00000F00);//longer time for trng

    `ifdef OSR_SIM_BYPASS_SELF_TEST                  //FW control field 1: 32'h00000041,c-No patch
    upper_ahb_wr(OTP_BASE+64'h020,32'h00000000);
    $display("INFO: BYPASS SELF_TEST");    
    `elsif OSR_SIM_ENABLE_SELF_TEST
    upper_ahb_wr(OTP_BASE+64'h020,32'h00000041);
    $display("INFO: ENABLE SELF_TEST");    
    `else
    upper_ahb_wr(OTP_BASE+64'h020,32'h00000000);
    $display("INFO: BYPASS SELF_TEST");    
    `endif

    `ifdef OSR_SIM_BOOT_RSA                          //FW control field 2: 32'h00000000
    upper_ahb_wr(OTP_BASE+64'h024,32'h00000000);
    `elsif OSR_SIM_BOOT_SM2
    upper_ahb_wr(OTP_BASE+64'h024,32'h00000011);
    `elsif OSR_SIM_BOOT_AES
            upper_ahb_wr(OTP_BASE+64'h024,32'h00000042);//cmac
    `elsif OSR_SIM_BOOT_SM4
            upper_ahb_wr(OTP_BASE+64'h024,32'h00000053);//cmac
    `else
    upper_ahb_wr(OTP_BASE+64'h024,32'h00000000);
    `endif

    `ifdef OSR_SIM_BOOT_RSA                          //FW control field 2: 32'h00000000
    upper_ahb_wr(OTP_BASE+64'h028,32'h00000100);
    `elsif OSR_SIM_BOOT_SM2
    upper_ahb_wr(OTP_BASE+64'h028,32'h00000111);
    `elsif OSR_SIM_BOOT_AES
      //  `ifdef BOOT_CMAC
            upper_ahb_wr(OTP_BASE+64'h028,32'h00000142);//bit9-8:1parallel;bit7-4:SocUpAlg;bit3-0:SocBootAlg
      //  `else
      //      upper_ahb_wr(OTP_BASE+64'h028,32'h00000022);//gcm
      //  `endif
    `elsif OSR_SIM_BOOT_SM4
      //  `ifdef BOOT_CMAC
            upper_ahb_wr(OTP_BASE+64'h028,32'h00000153);//cmac
      //  `else
      //      upper_ahb_wr(OTP_BASE+64'h028,32'h00000133);//gcm
      //  `endif
    `else
    upper_ahb_wr(OTP_BASE+64'h028,32'h00000100); //bit9-8 parallel
    `endif
    //upper_ahb_wr(OTP_BASE+64'h028,32'h00000142);//bit9-8:01parallel;bit7-4:SocUpAlg;bit3-0:SocBootAlg
    upper_ahb_wr(OTP_BASE+64'h02C,32'h00000000);

    upper_ahb_wr(OTP_BASE+64'h030,32'h0);//Version Counter
    upper_ahb_wr(OTP_BASE+64'h034,32'h0);//Version Counter
    upper_ahb_wr(OTP_BASE+64'h038,32'h0);//Version Counter
    upper_ahb_wr(OTP_BASE+64'h03c,32'h0);//Version Counter

    upper_ahb_wr(OTP_BASE+64'h040,32'h0);//Version Counter
    upper_ahb_wr(OTP_BASE+64'h044,32'h0);//Version Counter
    upper_ahb_wr(OTP_BASE+64'h048,32'h0);//Version Counter
    upper_ahb_wr(OTP_BASE+64'h04c,32'h0);//Version Counter

    upper_ahb_wr(OTP_BASE+64'h050,32'hffff_ffff); //patch_en_0
    upper_ahb_wr(OTP_BASE+64'h054,32'hffff_ffff); //patch_en_1

    upper_ahb_wr(OTP_BASE+64'h058,32'hffff_ffff); //patch_addr_0
    upper_ahb_wr(OTP_BASE+64'h05C,32'hffff_ffff); //patch_addr_1
    upper_ahb_wr(OTP_BASE+64'h060,32'hffff_ffff); //patch_addr_2
    upper_ahb_wr(OTP_BASE+64'h064,32'hffff_ffff); //patch_addr_3
    upper_ahb_wr(OTP_BASE+64'h068,32'hffff_ffff); //patch_addr_4
    upper_ahb_wr(OTP_BASE+64'h06C,32'hffff_ffff); //patch_addr_5
    upper_ahb_wr(OTP_BASE+64'h070,32'hffff_ffff); //patch_addr_6
    upper_ahb_wr(OTP_BASE+64'h074,32'hffff_ffff); //patch_addr_7
    upper_ahb_wr(OTP_BASE+64'h078,32'hffff_ffff); //patch_addr_8    
    upper_ahb_wr(OTP_BASE+64'h07C,32'hffff_ffff); //patch_addr_9    
    upper_ahb_wr(OTP_BASE+64'h080,32'hffff_ffff); //patch_addr_10   
    upper_ahb_wr(OTP_BASE+64'h084,32'hffff_ffff); //patch_addr_11
    upper_ahb_wr(OTP_BASE+64'h088,32'hffff_ffff); //patch_addr_12
    upper_ahb_wr(OTP_BASE+64'h08C,32'hffff_ffff); //patch_addr_13
    upper_ahb_wr(OTP_BASE+64'h090,32'hffff_ffff); //patch_addr_14
    upper_ahb_wr(OTP_BASE+64'h094,32'hffff_ffff); //patch_addr_15

    upper_ahb_wr(OTP_BASE+64'h098,32'hffff_ffff); //patch_data_0
    upper_ahb_wr(OTP_BASE+64'h09C,32'hffff_ffff); //patch_data_1
    upper_ahb_wr(OTP_BASE+64'h0A0,32'hffff_ffff); //patch_data_2
    upper_ahb_wr(OTP_BASE+64'h0A4,32'hffff_ffff); //patch_data_3
    upper_ahb_wr(OTP_BASE+64'h0A8,32'hffff_ffff); //patch_data_4
    upper_ahb_wr(OTP_BASE+64'h0AC,32'hffff_ffff); //patch_data_5
    upper_ahb_wr(OTP_BASE+64'h0B0,32'hffff_ffff); //patch_data_6
    upper_ahb_wr(OTP_BASE+64'h0B4,32'hffff_ffff); //patch_data_7
    upper_ahb_wr(OTP_BASE+64'h0B8,32'hffff_ffff); //patch_data_8
    upper_ahb_wr(OTP_BASE+64'h0BC,32'hffff_ffff); //patch_data_9
    upper_ahb_wr(OTP_BASE+64'h0C0,32'hffff_ffff); //patch_data_10
    upper_ahb_wr(OTP_BASE+64'h0C4,32'hffff_ffff); //patch_data_11
    upper_ahb_wr(OTP_BASE+64'h0C8,32'hffff_ffff); //patch_data_12
    upper_ahb_wr(OTP_BASE+64'h0CC,32'hffff_ffff); //patch_data_13
    upper_ahb_wr(OTP_BASE+64'h0D0,32'hffff_ffff); //patch_data_14
    upper_ahb_wr(OTP_BASE+64'h0D4,32'hffff_ffff); //patch_data_15
    upper_ahb_wr(OTP_BASE+64'h0D8,32'hffff_ffff); //patch_data_16
    upper_ahb_wr(OTP_BASE+64'h0DC,32'hffff_ffff); //patch_data_17
    upper_ahb_wr(OTP_BASE+64'h0E0,32'hffff_ffff); //patch_data_18
    upper_ahb_wr(OTP_BASE+64'h0E4,32'hffff_ffff); //patch_data_19
    upper_ahb_wr(OTP_BASE+64'h0E8,32'hffff_ffff); //patch_data_20
    upper_ahb_wr(OTP_BASE+64'h0EC,32'hffff_ffff); //patch_data_21
    upper_ahb_wr(OTP_BASE+64'h0F0,32'hffff_ffff); //patch_data_22
    upper_ahb_wr(OTP_BASE+64'h0F4,32'hffff_ffff); //patch_data_23
    upper_ahb_wr(OTP_BASE+64'h0F8,32'hffff_ffff); //patch_data_24
    upper_ahb_wr(OTP_BASE+64'h0FC,32'hffff_ffff); //patch_data_25
    upper_ahb_wr(OTP_BASE+64'h100,32'hffff_ffff); //patch_data_26
    upper_ahb_wr(OTP_BASE+64'h104,32'hffff_ffff); //patch_data_27
    upper_ahb_wr(OTP_BASE+64'h108,32'hffff_ffff); //patch_data_28
    upper_ahb_wr(OTP_BASE+64'h10C,32'hffff_ffff); //patch_data_29
    upper_ahb_wr(OTP_BASE+64'h110,32'hffff_ffff); //patch_data_30
    upper_ahb_wr(OTP_BASE+64'h114,32'hffff_ffff); //patch_data_31

    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 0 ,~32'hf0000eac);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 1 ,~32'hf0000eac);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 2 ,~32'hf0000eac);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 3 ,~32'hf0000bac);//key Attribute; eac->bac, for ehsm dbg key

    `ifdef OSR_SIM_BOOT_AES
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 4 ,~32'hf0000eac);//key Attribute//symmetric
    `elsif OSR_SIM_BOOT_SM4
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 4 ,~32'hf0000eac);//key Attribute//symmetric
    `elsif OSR_SIM_BOOT_RSA
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 4 ,~32'hf0000bac);//key Attribute//asymmetric
    `elsif OSR_SIM_BOOT_SM2
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 4 ,~32'hf0000bac);//key Attribute//asymmetric
    `else
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 4 ,~32'hf0000bac);//key Attribute//default asymmetric
    `endif

    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 5 ,~32'hf0000eac);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 6 ,~32'hf0000eac);//key Attribute

    `ifdef OSR_SIM_BOOT_AES
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 7 ,~32'hf0000eac);//key Attribute
    `elsif OSR_SIM_BOOT_SM4
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 7 ,~32'hf0000eac);//key Attribute
    `elsif OSR_SIM_BOOT_RSA
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 7 ,~32'hf0000bac);//key Attribute
    `elsif OSR_SIM_BOOT_SM2
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 7 ,~32'hf0000bac);//key Attribute
    `else
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 7 ,~32'hf0000bac);//key Attribute
    `endif

    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 8 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10* 9 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*10 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*11 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*12 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*13 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*14 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*15 ,~32'h00001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*16 ,~32'hf0001e5c);//key Attribute
    upper_ahb_wr(OTP_BASE+KEY_BASE + 4*10*17 ,~32'h00001e5c);//key Attribute

    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h01),32'h2ADEB001);//chip root key      //01b0de2a2a92da7c399c0d03c4b886b23f8954c4790d3e029b77f5af5312c947
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h02),32'h7CDA922A);                     //2adeb001 7cda922a 030d9c39 b286b8c4 c454893f 023e0d79 aff5779b 47c91253
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h03),32'h030D9C39);                     //2C22C1EA76260EF3F0B5F5C44284943600000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h04),32'hB286B8C4);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h05),32'hC454893F);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h06),32'h023E0D79);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h07),32'hAFF5779B);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h08),32'h47C91253);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  0 + 8'h09),32'habd8e12a);//chip root key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h01),32'h12e0fc02);//device root key    //02fce012d20d5e9c3e12db0a1bd74dea3512a2460e0e9b98969ba522177bc996
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h02),32'h9c5e0dd2);                     //12e0fc02 9c5e0dd2 0adb123e ea4dd71b 46a21235 989b0e0e 22a59b96 96c97b17
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h03),32'h0adb123e);                     //74707FEC47C6AC3CE3CFF91C1BEC6D5D00000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h04),32'hea4dd71b);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h05),32'h46a21235);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h06),32'h989b0e0e);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h07),32'h22a59b96);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h08),32'h96c97b17);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  1 + 8'h09),32'h73347f41);//device root key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h01),32'hf9465e25);//user root key      //255e46f95e7b0ebf20ff93e287a39304883f71e84f9d80f1c867788fca588c0d
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h02),32'hbf0e7b5e);                     //f9465e25 bf0e7b5e e293ff20 0493a387 e8713f88 f1809d4f 8f7867c8 0d8c58ca
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h03),32'he293ff20);                     //1120892B0A8B0E50CBC0068E1F3D552A00000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h04),32'h0493a387);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h05),32'he8713f88);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h06),32'hf1809d4f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h07),32'h8f7867c8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h08),32'h0d8c58ca);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  2 + 8'h09),32'h8c20ef97);//user root key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h01),32'h684008E3);//ehsm debug key     //E3084068CA5337D8DAA536096CFB7F42346AD2496DE6C7EBE6FD5DA36C61EA7B
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h02),32'hD83753CA);                     //684008E3 D83753CA 0936A5DA 427FFB6C 49D26A34 EBC7E66D A35DFDE6 7BEA616C
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h03),32'h0936A5DA);                     //9CDD7BF62BFC1BAB1C31A0C4FC78A1B006D5F1795147A956B577B9845286C5B1
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h04),32'h427FFB6C);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h05),32'h49D26A34);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h06),32'hEBC7E66D);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h07),32'hA35DFDE6);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h08),32'h7BEA616C);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  3 + 8'h09),32'hDA1EE4DD);//ehsm debug key crc
    `ifdef OSR_SIM_BOOT_SM2                                               
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h01),32'h9339ea7a);//ehsm fw verify key //7aea3993535747dd536b6c9132fa639834b045688fafc2118dcfb7d52ade4054
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h02),32'hdd475753);                     //9339ea7a dd475753 916c6b53 9863fa32 6845b034 11c2af8f d5b7cf8d 5440de2a
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h03),32'h916c6b53);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h04),32'h9863fa32);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h05),32'h6845b034);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h06),32'h11c2af8f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h07),32'hd5b7cf8d);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h08),32'h5440de2a);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h09),32'h2537c227);//ehsm fw verify key crc
   `else                                                                  
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h01),32'h5fa93fb9);//ehsm fw verify key //b93fa95fa029673a6c45ea3b6d7ba332cc80bb6c25d40ee9872a8d6aae2a5cc6
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h02),32'h3a6729a0);                     //5fa93fb9 3a6729a0 3bea456c 32a37b6d 6cbb80cc e90ed425 6a8d2a87 c65c2aae
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h03),32'h3bea456c);                     //32BB3737FB468C4B6AC9D84C1661737D7E0848F182FD82171EA13DF74D263AC7
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h04),32'h32a37b6d);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h05),32'h6cbb80cc);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h06),32'he90ed425);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h07),32'h6a8d2a87);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h08),32'hc65c2aae);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  4 + 8'h09),32'h5e952944);//ehsm fw verify key crc
    `endif                                                                
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h01),32'h8586641f);//ehsm encrypt key   //1f648685bf5cea0dc79aeccdc296eb723512a2460e0e9b98969ba522177bc996
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h02),32'h0dea5cbf);                     //8586641f 0dea5cbf cdec9ac7 72eb96c2 46a21235 989b0e0e 22a59b96 96c97b17
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h03),32'hcdec9ac7);                     //9FEBA879C596E5022C22C1EA76260EF300000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h04),32'h72eb96c2);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h05),32'h46a21235);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h06),32'h989b0e0e);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h07),32'h22a59b96);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h08),32'h96c97b17);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  5 + 8'h09),32'h99062504);//ehsm encrypt key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h01),32'h8586641f);//ehsm upgrate encrypt key //1f648685bf5cea0dc79aeccdc296eb723512a2460e0e9b98969ba522177bc996
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h02),32'h0dea5cbf);                           //8586641f 0dea5cbf cdec9ac7 72eb96c2 46a21235 989b0e0e 22a59b96 96c97b17
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h03),32'hcdec9ac7);                           //9FEBA879C596E5022C22C1EA76260EF300000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h04),32'h72eb96c2);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h05),32'h46a21235);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h06),32'h989b0e0e);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h07),32'h22a59b96);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h08),32'h96c97b17);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  6 + 8'h09),32'h99062504);//ehsm upgrate encrypt key crc
`ifdef OSR_SIM_BOOT_SM2                                                   
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h01),32'h9339ea7a);//ehsm upgrate verify key  //7aea3993535747dd536b6c9132fa639834b045688fafc2118dcfb7d52ade4054
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h02),32'hdd475753);                           //9339ea7a dd475753 916c6b53 9863fa32 6845b034 11c2af8f d5b7cf8d 5440de2a
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h03),32'h916c6b53);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h04),32'h9863fa32);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h05),32'h6845b034);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h06),32'h11c2af8f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h07),32'hd5b7cf8d);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h08),32'h5440de2a);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h09),32'h2537c227);//ehsm upgrate verify key crc
`else                                                                     
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h01),32'h5fa93fb9);//ehsm upgrate verify key  //b93fa95fa029673a6c45ea3b6d7ba332cc80bb6c25d40ee9872a8d6aae2a5cc6
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h02),32'h3a6729a0);                           //5fa93fb9 3a6729a0 3bea456c 32a37b6d 6cbb80cc e90ed425 6a8d2a87 c65c2aae
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h03),32'h3bea456c);                           //31DCE8521C49B51C793CE7EB38AB4B1000000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h04),32'h32a37b6d);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h05),32'h6cbb80cc);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h06),32'he90ed425);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h07),32'h6a8d2a87);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h08),32'hc65c2aae);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  7 + 8'h09),32'h5e952944);//ehsm upgrate verify key crc
`endif                                                                    
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h01),32'h8c75514c);//ehsm private key         //4c51758c4930cb3bb388aa28849b864049193a30ac218270d342beb072669d45
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h02),32'h3bcb3049);                           //8c75514c 3bcb3049 28aa88b3 40869b84 303a1949 708221ac b0be42d3 459d6672
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h03),32'h28aa88b3);                           //32BB3737FB468C4B6AC9D84C1661737D7E0848F182FD82171EA13DF74D263AC7
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h04),32'h40869b84);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h05),32'h303a1949);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h06),32'h708221ac);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h07),32'hb0be42d3);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h08),32'h459d6672);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  8 + 8'h09),32'h0e1df36f);//ehsm private key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h01),32'h826f69fd);//soc debug key            //fd696f8240e2f21119b608388ef59dbe051db24f2f9de837047e7707b840e396
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h02),32'h11f2e240);                           //826f69fd 11f2e240 3808b619 be9df58e 4fb21d05 37e89d2f 07777e04 96e340b8
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h03),32'h3808b619);                           //E3CFF91C1BEC6D5D128E69C14338D69500000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h04),32'hbe9df58e);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h05),32'h4fb21d05);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h06),32'h37e89d2f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h07),32'h07777e04);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h08),32'h96e340b8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a *  9 + 8'h09),32'h12533b6c);//soc debug key crc
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h01),32'hb5ce5b47);//soc  fw verify key       //475bceb56672e1ed056f0e79a8c775033be3b82ca684632aa506c4f1e310230a
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h02),32'hede17266);                           //b5ce5b47 ede17266 790e6f05 0375c7a8 2cb8e33b 2a6384a6 f1c406a5 0a2310e3
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h03),32'h790e6f05);                           //6427B1552F7F2DFA74707FEC47C6AC3C1619715546912D92122A547EF2C3C651
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h04),32'h0375c7a8);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h05),32'h2cb8e33b);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h06),32'h2a6384a6);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h07),32'hf1c406a5);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h08),32'h0a2310e3);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h09),32'he7079a57);//soc  fw verify key crc
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h01),32'h9aa76d5e);//soc  encrypt key         //5e6da79a5249a56134fe58965664ffc92f8063fc137085331dff28ae081c081e
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h02),32'h61a54952);                           //9aa76d5e 61a54952 9658fe34 c9ff6456 fc63802f 33857013 ae28ff1d 1e081c08
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h03),32'h9658fe34);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h04),32'hc9ff6456);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h05),32'hfc63802f);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h06),32'h33857013);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h07),32'hae28ff1d);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h08),32'h1e081c08);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h09),32'ha089ed17);//soc  encrypt key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h01),32'h980c56ba);//soc  fw verify key       //ba560c98 4a106cbf_15514fce 9fb45e78_ac616cfb 2f463921_c618cd05 51c2e4cb
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h02),32'hbf6c104a);                           //980c56ba bf6c104a ce4f5115 785eb49f fb6c61ac 2139462f 05cd18c6 cbe4c251 
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h03),32'hce4f5115);                           //32bb3737fb468c4b6ac9d84c1661737d7e0848f182fd82171ea13df74d263ac7
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h04),32'h785eb49f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h05),32'hfb6c61ac);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h06),32'h2139462f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h07),32'h05cd18c6);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h08),32'hcbe4c251);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 10 + 8'h09),32'ha3465b35);//soc  fw verify key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h01),32'hbbb6117b);//soc  encrypt key         //7b11b6bb 49b0de4f 485a8531 550220e5 883f71e8 4f9d80f1 c867788f ca588c0d
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h02),32'h4fdeb049);                           //bbb6117b 4fdeb049 31855a48 e5200255 e8713f88 f1809d4f 8f7867c8 0d8c58ca
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h03),32'h31855a48);                           //9feba879c596e5022c22c1ea76260ef300000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h04),32'he5200255);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h05),32'he8713f88);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h06),32'hf1809d4f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h07),32'h8f7867c8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h08),32'h0d8c58ca);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 11 + 8'h09),32'hdbc20f02);//soc  encrypt key crc
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h01),32'h0cfac2af);//soc  upgrade encrypt key //afc2fa0c3ab2b1c88bcb4bb78f57daa1883f71e84f9d80f1c867788fca588c0d
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h02),32'hc8b1b23a);                           //0cfac2af c8b1b23a b74bcb8b a1da578f e8713f88 f1809d4f 8f7867c8 0d8c58ca
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h03),32'hb74bcb8b);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h04),32'ha1da578f);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h05),32'he8713f88);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h06),32'hf1809d4f);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h07),32'h8f7867c8);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h08),32'h0d8c58ca);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h09),32'h588b5a5d);//soc  upgrade encrypt key crc
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h01),32'hbbb6117b);//soc  upgrade verify key  //7b11b6bb49b0de4f485a8531550220e5883f71e84f9d80f1c867788fca588c0d
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h02),32'h4fdeb049);                           //bbb6117b 4fdeb049 31855a48 e5200255 e8713f88 f1809d4f 8f7867c8 0d8c58ca
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h03),32'h31855a48);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h04),32'he5200255);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h05),32'he8713f88);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h06),32'hf1809d4f);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h07),32'h8f7867c8);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h08),32'h0d8c58ca);
   // upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h09),32'hdbc20f02);//soc  upgrade verify key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h01),32'hbbb6117b);//soc  upgrade encrypt key //7b11b6bb 49b0de4f 485a8531 550220e5 883f71e8 4f9d80f1 c867788f ca588c0d
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h02),32'h4fdeb049);                           //bbb6117b 4fdeb049 31855a48 e5200255 e8713f88 f1809d4f 8f7867c8 0d8c58ca
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h03),32'h31855a48);                           //9feba879c596e5022c22c1ea76260ef300000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h04),32'he5200255);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h05),32'he8713f88);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h06),32'hf1809d4f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h07),32'h8f7867c8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h08),32'h0d8c58ca);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 12 + 8'h09),32'hdbc20f02);//soc  upgrade encrypt key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h01),32'h980c56ba);//soc  upgrade verify key  //ba560c98 4a106cbf_15514fce 9fb45e78_ac616cfb 2f463921_c618cd05 51c2e4cb
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h02),32'hbf6c104a);                           //980c56ba bf6c104a ce4f5115 785eb49f fb6c61ac 2139462f 05cd18c6 cbe4c251
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h03),32'hce4f5115);                           //32bb3737fb468c4b6ac9d84c1661737d7e0848f182fd82171ea13df74d263ac7
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h04),32'h785eb49f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h05),32'hfb6c61ac);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h06),32'h2139462f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h07),32'h05cd18c6);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h08),32'hcbe4c251);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 13 + 8'h09),32'ha3465b35);//soc  upgrade verify key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h01),32'hb29f6344);//soc  private key         //44639fb2bb4e61ff92a2ee7ef196eb7f883f71e84f9d80f1c867788fca588c0d
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h02),32'hff614ebb);                           //b29f6344 ff614ebb 7eeea292 7feb96f1 e8713f88 f1809d4f 8f7867c8 0d8c58ca
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h03),32'h7eeea292);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h04),32'h7feb96f1);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h05),32'he8713f88);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h06),32'hf1809d4f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h07),32'h8f7867c8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h08),32'h0d8c58ca);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 14 + 8'h09),32'h19a7ad2d);//soc  private key crc
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h01),32'h2ab99fba);//reserve  key             //ba9fb92a49e05d529d5ae74ebe890c16883f71e84f9d80f1c867788fca588c0d
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h02),32'h525de049);                           //2ab99fba 525de049 4ee75a9d 160c89be e8713f88 f1809d4f 8f7867c8 0d8c58ca
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h03),32'h4ee75a9d);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h04),32'h160c89be);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h05),32'he8713f88);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h06),32'hf1809d4f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h07),32'h8f7867c8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h08),32'h0d8c58ca);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 15 + 8'h09),32'h0705c492);//reserve  key crc

    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h01),32'hf9465e25);//user root key            //255e46f95e7b0ebf20ff93e287a39304883f71e84f9d80f1c867788fca588c0d
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h02),32'hbf0e7b5e);                           //f9465e25 bf0e7b5e e293ff20 0493a387 e8713f88 f1809d4f 8f7867c8 0d8c58ca
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h03),32'he293ff20);                           //1120892B0A8B0E50CBC0068E1F3D552A00000000000000000000000000000000
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h04),32'h0493a387);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h05),32'he8713f88);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h06),32'hf1809d4f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h07),32'h8f7867c8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h08),32'h0d8c58ca);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 16 + 8'h09),32'h8c20ef97);//user root key crc

    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h01),32'h2ab99fba);//                         //ba9fb92a49e05d529d5ae74ebe890c16883f71e84f9d80f1c867788fca588c0d
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h02),32'h525de049);                           //2ab99fba 525de049 4ee75a9d 160c89be e8713f88 f1809d4f 8f7867c8 0d8c58ca
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h03),32'h4ee75a9d);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h04),32'h160c89be);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h05),32'he8713f88);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h06),32'hf1809d4f);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h07),32'h8f7867c8);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h08),32'h0d8c58ca);
    upper_ahb_wr(OTP_BASE+KEY_BASE+4*(8'h0a * 17 + 8'h09),32'h0705c492);//                

endtask

task upper_ahb_rd(
    input  reg [63:0] raddr,
    output reg [31:0] upper_reg_rdata
);
begin
    while(hreadyout_s_ext4!=1'b1)@(posedge osc_clk);
    upper_hsel      <=  1'b1 ;
    upper_haddr     <=  raddr;
    upper_htrans    <=  2'h2 ;
    upper_hwrite    <=  1'b0 ;
    upper_hsize     <=  3'h2 ;
    upper_hburst    <=  3'b0 ;
    upper_hprot     <=  4'b0 ;
    upper_hmaster   <=  4'h4 ;
    upper_hwdata    <= 32'b0 ;
    upper_hmastlock <=  1'b0 ;
    upper_hauser    <=  3'b0 ;
    upper_hwuser    <=  3'b0 ;
    @(posedge osc_clk);
    upper_hsel      <=  1'b1 ;
    upper_haddr     <= 64'b0 ;
    upper_htrans    <=  2'b0 ;
    upper_hwrite    <=  1'b0 ;
    upper_hsize     <=  3'b0 ;
    upper_hburst    <=  3'b0 ;
    upper_hprot     <=  4'b0 ;
    upper_hmaster   <=  4'b0 ;
    upper_hwdata    <= 32'b0 ;
    upper_hmastlock <=  1'b0 ;
    upper_hauser    <=  3'b0 ;
    upper_hwuser    <=  3'b0 ;
    @(posedge osc_clk);
    while(hreadyout_s_ext4!=1'b1)@(posedge osc_clk);
    upper_reg_rdata    <=  hrdata_s_ext4 ;
    upper_hsel      <=  1'b0 ;
    upper_haddr     <= 64'b0 ;
    upper_htrans    <=  2'b0 ;
    upper_hwrite    <=  1'b0 ;
    upper_hsize     <=  3'b0 ;
    upper_hburst    <=  3'b0 ;
    upper_hprot     <=  4'b0 ;
    upper_hmaster   <=  4'b0 ;
    upper_hwdata    <= 32'b0 ;
    upper_hmastlock <=  1'b0 ;
    upper_hauser    <=  3'b0 ;
    upper_hwuser    <=  3'b0 ;
    @(posedge osc_clk);

end
endtask

task upper_ahb_wr(
    input  reg [63:0] waddr,
    input  reg [31:0] wdata
);
begin
    while(hreadyout_s_ext4!=1'b1)@(posedge osc_clk);
    upper_hsel      <=  1'b1 ;
    upper_haddr     <=  waddr;
    upper_htrans    <=  2'h2 ;
    upper_hwrite    <=  1'b1 ;
    upper_hsize     <=  3'h2 ;
    upper_hburst    <=  3'b0 ;
    upper_hprot     <=  4'b0 ;
    upper_hmaster   <=  4'h0 ;
    upper_hwdata    <= 32'b0 ;
    upper_hmastlock <=  1'b0 ;
    upper_hauser    <=  3'b0 ;
    upper_hwuser    <=  3'b0 ;
    @(posedge osc_clk);
    upper_hsel      <=  1'b1 ;
    upper_haddr     <= 64'b0 ;
    upper_htrans    <=  2'b0 ;
    upper_hwrite    <=  1'b0 ;
    upper_hsize     <=  3'b0 ;
    upper_hburst    <=  3'b0 ;
    upper_hprot     <=  4'b0 ;
    upper_hmaster   <=  4'b0 ;
    upper_hwdata    <=  wdata;
    upper_hmastlock <=  1'b0 ;
    upper_hauser    <=  3'b0 ;
    upper_hwuser    <=  3'b0 ;
    @(posedge osc_clk);
    while(hreadyout_s_ext4!=1'b1)@(posedge osc_clk);
    upper_hsel      <=  1'b0 ;
    upper_haddr     <= 64'b0 ;
    upper_htrans    <=  2'b0 ;
    upper_hwrite    <=  1'b0 ;
    upper_hsize     <=  3'b0 ;
    upper_hburst    <=  3'b0 ;
    upper_hprot     <=  4'b0 ;
    upper_hmaster   <=  4'b0 ;
    upper_hwdata    <= 32'b0 ;
    upper_hmastlock <=  1'b0 ;
    upper_hauser    <=  3'b0 ;
    upper_hwuser    <=  3'b0 ;
    @(posedge osc_clk);
//    `ifdef TBL
    //$display("Write OTP addr = 0x%h  data = 0x%h, 0x%h, 0x%h, 0x%h, ",waddr,wdata[7:0],wdata[15:8],wdata[23:16],wdata[31:24]);
//    `endif
end
endtask

task hsm_ahb_rd(
    input  reg [63:0] raddr,
    output reg [31:0] hsm_reg_rdata
);
begin
    while(hsm_hreadyout!=1'b1)@(posedge dut_clk);
    hsm_hsel      <=  1'b1 ;
    hsm_haddr     <=  raddr;
    hsm_htrans    <=  2'h2 ;
    hsm_hwrite    <=  1'b0 ;
    hsm_hsize     <=  3'h2 ;
    hsm_hburst    <=  3'b0 ;
    hsm_hprot     <=  4'b0 ;
    hsm_hmaster   <=  4'h4 ;
    hsm_hwdata    <= 32'b0 ;
    hsm_hmastlock <=  1'b0 ;
    hsm_hauser    <=  3'b0 ;
    hsm_hwuser    <=  3'b0 ;
    @(posedge dut_clk);
    hsm_hsel      <=  1'b1 ;
    hsm_haddr     <= 64'b0 ;
    hsm_htrans    <=  2'b0 ;
    hsm_hwrite    <=  1'b0 ;
    hsm_hsize     <=  3'b0 ;
    hsm_hburst    <=  3'b0 ;
    hsm_hprot     <=  4'b0 ;
    hsm_hmaster   <=  4'b0 ;
    hsm_hwdata    <= 32'b0 ;
    hsm_hmastlock <=  1'b0 ;
    hsm_hauser    <=  3'b0 ;
    hsm_hwuser    <=  3'b0 ;
    @(posedge dut_clk);
    while(hsm_hreadyout!=1'b1)@(posedge dut_clk);
    hsm_reg_rdata    <=  hsm_hrdata ;
    hsm_hsel      <=  1'b0 ;
    hsm_haddr     <= 64'b0 ;
    hsm_htrans    <=  2'b0 ;
    hsm_hwrite    <=  1'b0 ;
    hsm_hsize     <=  3'b0 ;
    hsm_hburst    <=  3'b0 ;
    hsm_hprot     <=  4'b0 ;
    hsm_hmaster   <=  4'b0 ;
    hsm_hwdata    <= 32'b0 ;
    hsm_hmastlock <=  1'b0 ;
    hsm_hauser    <=  3'b0 ;
    hsm_hwuser    <=  3'b0 ;
    @(posedge dut_clk);

end
endtask

task hsm_ahb_wr(
    input  reg [63:0] waddr,
    input  reg [31:0] wdata
);
begin
    while(hsm_hreadyout!=1'b1)@(posedge dut_clk);
    hsm_hsel      <=  1'b1 ;
    hsm_haddr     <=  waddr;
    hsm_htrans    <=  2'h2 ;
    hsm_hwrite    <=  1'b1 ;
    hsm_hsize     <=  3'h2 ;
    hsm_hburst    <=  3'b0 ;
    hsm_hprot     <=  4'b0 ;
    hsm_hmaster   <=  4'h0 ;
    hsm_hwdata    <= 32'b0 ;
    hsm_hmastlock <=  1'b0 ;
    hsm_hauser    <=  3'b0 ;
    hsm_hwuser    <=  3'b0 ;
    @(posedge dut_clk);
    hsm_hsel      <=  1'b1 ;
    hsm_haddr     <= 64'b0 ;
    hsm_htrans    <=  2'b0 ;
    hsm_hwrite    <=  1'b0 ;
    hsm_hsize     <=  3'b0 ;
    hsm_hburst    <=  3'b0 ;
    hsm_hprot     <=  4'b0 ;
    hsm_hmaster   <=  4'b0 ;
    hsm_hwdata    <=  wdata;
    hsm_hmastlock <=  1'b0 ;
    hsm_hauser    <=  3'b0 ;
    hsm_hwuser    <=  3'b0 ;
    @(posedge dut_clk);
    while(hsm_hreadyout!=1'b1)@(posedge dut_clk);
    hsm_hsel      <=  1'b0 ;
    hsm_haddr     <= 64'b0 ;
    hsm_htrans    <=  2'b0 ;
    hsm_hwrite    <=  1'b0 ;
    hsm_hsize     <=  3'b0 ;
    hsm_hburst    <=  3'b0 ;
    hsm_hprot     <=  4'b0 ;
    hsm_hmaster   <=  4'b0 ;
    hsm_hwdata    <= 32'b0 ;
    hsm_hmastlock <=  1'b0 ;
    hsm_hauser    <=  3'b0 ;
    hsm_hwuser    <=  3'b0 ;
    @(posedge dut_clk);
end
endtask

task sprint_u32_q(
    input  reg [31:0] i_input[$],
    string name
);
begin
    str = "";
    foreach(i_input[i]) begin
    str = $sformatf("%s%h",str,i_input[i]);
    end
    $display("%s = %s",name,str);
    str = "";
end
endtask

task sprint_u8_q(
    input  reg [7:0] i_input[$],
    string name
);
begin
    str = "";
    foreach(i_input[i]) begin
    str = $sformatf("%s%h",str,i_input[i]);
    end
    $display("%s = %s",name,str);
    str = "";
end
endtask

task set_cmd(
    input reg[511:0] i_cmd
);
begin
    upper_ahb_wr(MAILBOX_BASE+64'h00,i_cmd[ 31:  0]);
    upper_ahb_wr(MAILBOX_BASE+64'h04,i_cmd[ 63: 32]);
    upper_ahb_wr(MAILBOX_BASE+64'h08,i_cmd[ 95: 64]);
    upper_ahb_wr(MAILBOX_BASE+64'h0C,i_cmd[127: 96]);
    upper_ahb_wr(MAILBOX_BASE+64'h10,i_cmd[159:128]);
    upper_ahb_wr(MAILBOX_BASE+64'h14,i_cmd[191:160]);
    upper_ahb_wr(MAILBOX_BASE+64'h18,i_cmd[223:192]);
    upper_ahb_wr(MAILBOX_BASE+64'h1C,i_cmd[255:224]);
    upper_ahb_wr(MAILBOX_BASE+64'h20,i_cmd[287:256]);
    upper_ahb_wr(MAILBOX_BASE+64'h24,i_cmd[319:288]);
    upper_ahb_wr(MAILBOX_BASE+64'h28,i_cmd[351:320]);
    upper_ahb_wr(MAILBOX_BASE+64'h2C,i_cmd[383:352]);
    upper_ahb_wr(MAILBOX_BASE+64'h30,i_cmd[415:384]);
    upper_ahb_wr(MAILBOX_BASE+64'h34,i_cmd[447:416]);
    upper_ahb_wr(MAILBOX_BASE+64'h38,i_cmd[479:448]);
    upper_ahb_wr(MAILBOX_BASE+64'h3C,i_cmd[511:480]);
end
endtask

task set_ehsm_cmd(
    input reg[799:0] i_cmd
);
begin
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h00,i_cmd[ 31:  0]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h04,i_cmd[ 63: 32]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h08,i_cmd[ 95: 64]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h0C,i_cmd[127: 96]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h10,i_cmd[159:128]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h14,i_cmd[191:160]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h18,i_cmd[223:192]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h1C,i_cmd[255:224]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h20,i_cmd[287:256]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h24,i_cmd[319:288]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h28,i_cmd[351:320]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h2C,i_cmd[383:352]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h30,i_cmd[415:384]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h34,i_cmd[447:416]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h38,i_cmd[479:448]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h3C,i_cmd[511:480]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h40,i_cmd[543:512]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h44,i_cmd[575:544]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h48,i_cmd[607:576]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h4c,i_cmd[639:608]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h50,i_cmd[671:640]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h54,i_cmd[703:672]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h58,i_cmd[735:704]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h5c,i_cmd[767:736]);
    upper_ahb_wr(MBOX_CMD_BUFFER_L+64'h60,i_cmd[799:768]);

    upper_ahb_wr(32'h6003_0000,MBOX_CMD_BUFFER_L);  //CMD ADDR
    upper_ahb_wr(32'h6003_0004,MBOX_CMD_BUFFER_H);

    upper_ahb_wr(32'h6003_0008,MBOX_RSP_BUFFER_L); //RSP ADDR
    upper_ahb_wr(32'h6003_000C,MBOX_RSP_BUFFER_H); 

    upper_ahb_wr(MAILBOX_BASE+64'h0,32'h6003_0000); //send TEST_PACKET_ADDR to Mailbox

end
endtask

task set_ehsm_debug_cmd(
    input reg[799:0] i_cmd
);
begin
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h00,i_cmd[ 31:  0]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h04,i_cmd[ 63: 32]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h08,i_cmd[ 95: 64]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h0C,i_cmd[127: 96]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h10,i_cmd[159:128]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h14,i_cmd[191:160]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h18,i_cmd[223:192]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h1C,i_cmd[255:224]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h20,i_cmd[287:256]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h24,i_cmd[319:288]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h28,i_cmd[351:320]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h2C,i_cmd[383:352]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h30,i_cmd[415:384]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h34,i_cmd[447:416]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h38,i_cmd[479:448]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h3C,i_cmd[511:480]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h40,i_cmd[543:512]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h44,i_cmd[575:544]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h48,i_cmd[607:576]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h4c,i_cmd[639:608]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h50,i_cmd[671:640]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h54,i_cmd[703:672]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h58,i_cmd[735:704]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h5c,i_cmd[767:736]);
    upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h60,i_cmd[799:768]);
end
endtask

task set_data_n(
    input reg [63:0] i_base_radix,
    input reg [63:0] i_world_len,
    input reg [31:0] i_data[$]
);
begin
    integer i;
    string str = "";
    foreach(i_data[i]) begin
        str = $sformatf("%s%h",str,i_data[i]);
    end
    $display("i_data   = %s",str);
    str = "";
    for(i=0;i<i_world_len;i++) begin
        i_data[i] = i_data[i];
        upper_ahb_wr(i_base_radix+i*4,i_data[i]);
    end
end
endtask

task set_data(
    input reg [63:0] i_base_radix,
    input reg [63:0] i_world_len,
    input reg [31:0] i_data[$]
);
begin
    integer i;
    string str = "";
    foreach(i_data[i]) begin
        str = $sformatf("%s%h",str,i_data[i]);
    end
    $display("i_data   = %s",str);
    str = "";
    for(i=0;i<i_world_len;i++) begin
        i_data[i] = {<<byte {i_data[i]}};
        upper_ahb_wr(i_base_radix+i*4,i_data[i]);
    end
end
endtask

task get_result_n(
    input  reg [63:0] i_base_radix,
    input  reg [63:0] i_world_len,
    output reg [31:0] o_result[$],
    input  string            name
);
begin
    integer i;
    string str = "";
    o_result = {};
    for(i=0;i<i_world_len;i++) begin
        upper_ahb_rd(i_base_radix+i*4,o_result[i]);
        o_result[i] = {<<byte {o_result[i]}};
    end
    foreach(o_result[i]) begin
        str = $sformatf("%s%h",str,o_result[i]);
    end
    $display("%s = %s",name,str);
    str = "";
end
endtask

task get_result(
    input  reg [63:0] i_base_radix,
    input  reg [63:0] i_world_len,
    output reg [31:0] o_result[$]
);
begin
    integer i;
    string str = "";
    o_result = {};
    for(i=0;i<i_world_len;i++) begin
        upper_ahb_rd(i_base_radix+i*4,o_result[i]);
        o_result[i] = {<<byte {o_result[i]}};
    end
    foreach(o_result[i]) begin
        str = $sformatf("%s%h",str,o_result[i]);
    end
    $display("o_result = %s",str);
    str = "";
end
endtask

task onebit2dworld(
    input reg [63:0] saddr,
    input reg [31:0] data,
    input reg [5:0]  bitnum
);
begin : one_bit_to_doubleworld
    integer i;
    reg [63:0] addr;
    addr = 0;
    addr = saddr;
    //$display("o_result = %h",addr);
    for(i=0;i<bitnum;i++)begin
        if(data[i]==1)begin
            upper_ahb_wr(addr+64'd0,32'hFFFF_FFFF);
            upper_ahb_wr(addr+64'd4,32'hFFFF_FFFF);
        end
        else begin
            upper_ahb_wr(addr+64'd0,32'hF000_0000);
            upper_ahb_wr(addr+64'd4,32'hF000_0000);
        end
        addr = addr + 8;
    end
    addr = 0;
end
endtask

task onebit_2_4world(
    input reg [63:0] saddr,
    input reg [31:0] data,
    input reg [5:0]  bitnum
);
begin : one_bit_to_4world
    integer i;
    reg [63:0] addr;
    addr = 0;
    addr = saddr;
    //$display("o_result = %h",addr);
    for(i=0;i<bitnum;i++)begin
        if(data[i]==1)begin
            upper_ahb_wr(addr+64'd0,32'hFFFF_FFFF);
            upper_ahb_wr(addr+64'd4,32'hFFFF_FFFF);
            upper_ahb_wr(addr+64'd8,32'hFFFF_FFFF);
            upper_ahb_wr(addr+64'd12,32'hFFFF_FFFF);
        end
        else begin
            upper_ahb_wr(addr+64'd0,32'hF000_0000);
            upper_ahb_wr(addr+64'd4,32'hF000_0000);
            upper_ahb_wr(addr+64'd8,32'hF000_0000);
            upper_ahb_wr(addr+64'd12,32'hF000_0000);
        end
        addr = addr + 16;
    end
    addr = 0;
end
endtask

task check_result();
    //check pass flag
    foreach(s_result_q[i])begin
        integer      j;
        if(s_result_q[i].cmd_rsp == CMD_RSP_OK)
        begin
        if(s_result_q[i].flag_s==1)
        begin
        $display("Run_Pass\n");
        $display("------------------------ cmd_id = %s ------------------------",s_result_q[i].cmd_id);
        $display("\n");
        $display("\n");
                $display(" PPPPPPPP           A          SSSSSSSSS    SSSSSSSSS     ");
                $display(" P       P         A A        S            S              ");
                $display(" P        P       A   A       S            S              ");
                $display(" P       P       AA    A      S            S              ");
                $display(" PPPPPPPP       AAAAAAAAA      SSSSSSSSS    SSSSSSSSS     ");
                $display(" P             A         A              S            S    ");
                $display(" P            A           A             S            S    ");
                $display(" P           A             A            S            S    ");
                $display(" P          A               A  SSSSSSSSS    SSSSSSSSS     ");
        $display("\n");
        $display("\n");
        $display("\n");
        end
        else begin
        $display("Run_Fail\n");
        $display("---------------------- cmd_id = %s -----------------------",s_result_q[i].cmd_id);
        $display("\n");
        $display("\n");
            $display(" FFFFFFFFF          A          IIIIIIIII   L              ");
            $display(" F                 A A             I       L              ");
            $display(" F                A   A            I       L              ");
            $display(" F               AA    A           I       L              ");
            $display(" FFFFFFF        AAAAAAAAA          I       L              ");
            $display(" F             A         A         I       L              ");
            $display(" F            A           A        I       L              ");
            $display(" F           A             A       I       L              ");
            $display(" F          A               A  IIIIIIIII   LLLLLLLLLLL    ");
        $display("\n");
        $display("\n");
        $display("\n");
        $display("------------------------- DUT: ---------------------------\n");
        sprint_u32_q(s_result_q[i].result_s,"dut_result");
        $display("------------------------- RM:  ---------------------------\n");
        sprint_u32_q(s_result_q[i].rm_result_s,"rm_result");
        end
        end else begin
        $display("Rsp_Error\n");
        $display("---------------------- cmd_id = %s -----------------------",s_result_q[i].cmd_id);
        $display("\n");
        $display("\n");
            $display(" EEEEEEEEE      R    R    R    R       OOOO       R    R   ");
            $display(" E              R   R     R   R       O    O      R   R    ");
            $display(" E              R  R      R  R       O      O     R  R     ");
            $display(" E              R R       R R       O        O    R R      ");
            $display(" EEEEEEEEF      RR        RR        O        O    RR       ");
            $display(" E              R         R         O        O    R        ");
            $display(" E              R         R          O      O     R        ");
            $display(" E              R         R           O    O      R        ");
            $display(" EEEEEEEEE      R         R            OOOO       R        ");
        $display("\n");
        $display("\n");
        $display("\n");
        end
    end
    $finish();

endtask

task check_pass_flag();

    if(pass_flag==1)
    begin
    $display("Run_Pass\n");
    $display("\n");
    $display("\n");
            $display(" PPPPPPPP           A          SSSSSSSSS    SSSSSSSSS     ");
            $display(" P       P         A A        S            S              ");
            $display(" P        P       A   A       S            S              ");
            $display(" P       P       AA    A      S            S              ");
            $display(" PPPPPPPP       AAAAAAAAA      SSSSSSSSS    SSSSSSSSS     ");
            $display(" P             A         A              S            S    ");
            $display(" P            A           A             S            S    ");
            $display(" P           A             A            S            S    ");
            $display(" P          A               A  SSSSSSSSS    SSSSSSSSS     ");
    $display("\n");
    $display("\n");
    $display("\n");
    end
    else begin
    $display("Run_Fail\n");
    $display("\n");
    $display("\n");
        $display(" FFFFFFFFF          A          IIIIIIIII   L              ");
        $display(" F                 A A             I       L              ");
        $display(" F                A   A            I       L              ");
        $display(" F               AA    A           I       L              ");
        $display(" FFFFFFF        AAAAAAAAA          I       L              ");
        $display(" F             A         A         I       L              ");
        $display(" F            A           A        I       L              ");
        $display(" F           A             A       I       L              ");
        $display(" F          A               A  IIIIIIIII   LLLLLLLLLLL    ");
    $display("\n");
    $display("\n");
    $display("\n");
    end
    $finish();

endtask

`include "./case/ehsm_hash_sha256_test.sv";
`include "./case/ehsm_hmac_sm3_test.sv";
`include "./case/ehsm_ske_sm4_test.sv";
`include "./case/ehsm_trng_test.sv";
`include "./case/ehsm_rsa_sign_test.sv";
`include "./case/ehsm_ecdsa_verify_test.sv";
`include "./case/ehsm_gcm_cipher_test.sv";
`include "./case/ehsm_ccm_cipher_test.sv";
`include "./case/ehsm_gmac_gen_test.sv";
`include "./case/ehsm_mac_gen_test.sv";
`include "./case/ehsm_pke_sm2_test.sv";
`include "./case/ehsm_pke_rsa_test.sv";
`include "./case/ehsm_sm2_verify.sv";
`include "./case/general_task.sv";
`include "./case/ehsm_get_challenge.sv";
`include "./case/ehsm_debug_auth_test.sv";
`include "./case/ehsm_bl_verify_image.sv";
