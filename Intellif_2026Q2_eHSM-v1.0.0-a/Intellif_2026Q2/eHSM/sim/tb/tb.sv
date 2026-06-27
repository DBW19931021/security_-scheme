//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module blank_m(); 
endmodule // blank_m

module tb (

);

`include "../tc/tc.sv";
begin: This_is_4019 end
blank_m U_4019();

int Version = 16'h100a ;

// PART 1 (MANUAL): declare

//PARAMETER

//WIRE

//REG

reg             clk = 1'b1;
reg             rstn = 1'b0;
int             PC;
reg [1023:0]    HSM_BOOTLOADER;
reg [1023:0]    HSM_FIRMWARE;
reg [2047:0]    tc_name;
reg [1023:0]    fsdbfile;
reg             fsdb_auto_switch_en;
reg [1023:0]    fsdb_auto_switch_size;
reg [1023:0]    fsdb_auto_switch_num;

reg [1023:0]    wave;
int             wavetime;

// PART 2  : wire for sub module outputs

// PART 3 : main/glue logic

wire          OSCCLK         = clk      ;
wire          OSCCLK_N       = ~clk     ;
wire          RST_N          = rstn     ;

wire          i_hsm_trst_n   ;
wire          i_hsm_tck      ;
wire          i_hsm_tms      ;
wire          i_hsm_tdi      ;
wire          o_hsm_tdo      ;

wire          o_upper_uart_txd;
wire          o_hsm_uart_txd;

//wire          i_hsm_uart_rxd = 1'h0;
wire          i_hsm_uart_rxd ;
pullup(i_hsm_uart_rxd);

// PART 4 (MANUAL) : wire for sub module inputs

integer       j ;

wire [3:0]  hsm2soc_sta;
wire jtag_done = hsm2soc_sta[3] | hsm2soc_sta[2] ;

reg rst_jtagn;

initial begin

    $value$plusargs("tc_name=%s", tc_name);
    $display("tc_name = %0s",tc_name);
    $value$plusargs("fsdbfile=%s", fsdbfile);
    $value$plusargs("wave=%0s", wave);
    $value$plusargs("wavetime=%0d", wavetime);
    $display("wave = %0s",wave);
    $value$plusargs("fsdb_auto_switch_en=%d", fsdb_auto_switch_en);
    $value$plusargs("fsdb_auto_switch_size=%d", fsdb_auto_switch_size);
    $value$plusargs("fsdb_auto_switch_num=%d", fsdb_auto_switch_num);

    if(wave == "on") begin
        $display("wavetime = %0d ms",wavetime);
        repeat(wavetime*(10**5)) @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.i_clk);
        $display("INFO: wave dump start @ %8d",$time);
        if(fsdb_auto_switch_en==0) begin
        $fsdbDumpfile(fsdbfile);
        end
        else begin
        $fsdbAutoSwitchDumpfile(fsdb_auto_switch_size,fsdbfile,fsdb_auto_switch_num);
        end
        $fsdbDumpvars(15, tb);
        $fsdbDumpMDA(0, tb);
        $fsdbDumpflush;
    end

end

initial begin
    j = 0;
    forever begin
        $display(" --- %0d ms --- ", j);
        #1ms;
        j = j + 1;
        if(j > `SIM_MS) begin
            $display("time out!");
            $finish;
        end
    end
end

always #1ns clk = ~clk;
always #1ns rstn = 1'h1;
reg tbclk ;
initial begin
    tbclk = 0;
end

always #1 tbclk = ~tbclk;

    reg [38:0] hsm_firmware[131071:0];

reg [38:0] hsm_bootloader[65535:0];

reg [15:0] iram_a;
reg [38:0] iram_d;
reg [31:0] key_import_return_handle;

initial begin:demo_test

`ifdef OSR_SIM_INSTALL_ON
    $display("ehsm_bootcode_install : %0s","ON");
`else
    $display("ehsm_bootcode_install : %0s","Off");
`endif

    iram_a <= 0;
    iram_d <= hsm_firmware[0];

    //wait for dut_reset release
    repeat(10) @(posedge osc_clk);
    wait(dut_rst_n==1'b1);
    repeat(10) @(posedge osc_clk);

    //wait seip ready
    //$display("init seip ");

    if(tc_name == "ram_if_test")begin

    end
    else if(tc_name != "test")begin
        init_seip;
    end

     if (tc_name == "ehsm_debug_auth_test")begin
        $display("INFO: Debug_Auth_Test@ %8d",$time);
        //change lifecycle
        $display("INFO: eHSM Lifecycle = MANUFACTURE_MODE");
            upper_ahb_wr(OTP_BASE+64'h000,32'h46C1A416   );
        //set alg mode
                upper_ahb_wr(OTP_BASE+64'h070 + 4*10* 3 ,~32'hf0000bac);//key Attribute

        $display("INFO: Soft Rst Dut @ %8d",$time);
        //DUT Reset
        upper_ahb_wr(SOFT_RSTN,32'hFFFF_FFFE);//set bit0 to 0
        //DUT Release
        upper_ahb_wr(SOFT_RSTN,32'hFFFF_FFFF);//set bit0 to 1

        @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[0]);
        #1;
        if(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[1] == 1'h0) begin
            $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[0]); @ %0d",$time);
            $display("INFO: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[1] == 1'h0");
            $display("INFO: eHSM hardware boot done(OK)!\n");
        end else begin
            $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[0]); @ %0d",$time);
            $display("Error: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[1] == 1'h1");
            $display("INFO: eHSM hardware boot done with error!\n");
        end

         @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hready);
             #1;
             $display("\nINFO: @ %0d CPU read instruct 0 : 0x%9h",$time,tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hrdata);
             @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hready);
             #1;
             $display("\nINFO: @ %0d CPU read instruct 1 : 0x%9h\n",$time,tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hrdata);

        @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[2]); //wait bootloader done
        #1
        if(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[3] == 1'h0) begin
            $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[2]); @ %0d",$time);
            $display("INFO: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[3] == 1'h0");
            $display("INFO: eHSM Bootloader done(OK)!\n");
            ehsm_bl_verify_image();  //verify firmware from soc_mem
        end else begin
            $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[2]); @ %0d",$time);
            $display("Error: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[3]) == 1");
            $display("INFO: eHSM Bootloader running done with Error!\n");
        end

        repeat(10) @(posedge osc_clk);
        //ehsm_debug_auth_test;
    end

    else begin
        $display("INFO: Soft Rst Dut @ %8d",$time);
        //DUT Reset
        upper_ahb_wr(SOFT_RSTN,32'hFFFF_FFFE);//set bit0 to 0
        //DUT Release
        upper_ahb_wr(SOFT_RSTN,32'hFFFF_FFFF);//set bit0 to 1

        repeat(10) @(posedge osc_clk);
        //wait for fw_done
        cmd_state = WAIT_SEIP_RDY;

        if(tc_name != "ram_if_test")begin
            `ifdef OSR_SIM_INSTALL_ON
                wait(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[4:0] == 'h11);
            `else
                @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[0]);
                #1;
                if(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[1] == 1'h0) begin
                    $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[0]); @ %0d",$time);
                    $display("INFO: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[1] == 1'h0");
                    $display("INFO: eHSM hardware boot done(OK)!\n");
                end else begin
                    $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[0]); @ %0d",$time);
                    $display("Error: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[1] == 1'h1");
                    $display("INFO: eHSM hardware boot done with error!\n");
                end

                @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hready);
                    #1;
                    $display("\nINFO: @ %0d CPU read instruct 0 : 0x%9h",$time,tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hrdata);
                    @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hready);
                    #1;
                    $display("\nINFO: @ %0d CPU read instruct 1 : 0x%9h\n",$time,tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_cpu_wrapper.i_ilm_hrdata);

                @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[2]); //wait bootloader done
                #1
                if(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[3] == 1'h0) begin
                    $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[2]); @ %0d",$time);
                    $display("INFO: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[3] == 1'h0");
                    $display("INFO: eHSM Bootloader done(OK)!\n");
                    ehsm_bl_verify_image();  //verify firmware from soc_mem
                end else begin
                    $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[2]); @ %0d",$time);
                    $display("Error: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[3]) == 1");
                    $display("INFO: eHSM Bootloader running done with Error!\n");
                end

                @(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[4]);
                #1;
                if(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[5] == 1'h0) begin
                    $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[4]); @ %0d",$time);
                    $display("INFO: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[5]) == 0");
                    $display("INFO: eHSM Firmware ready!\n");
                end else begin
                    $display("\n@(posedge tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[4]); @ %0d",$time);
                    $display("Error: tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.o_hsm_status[5]) == 1");
                    $display("INFO: eHSM Firmware running done with Error!\n");
                end

            `endif
        end

        `ifdef OSR_SIM_INSTALL_ON
            //install_code;
            ehsm_soc_upgrade;
        `endif
       end

    //run test
    $display("INFO: Send Cmd Mbx @ %8d",$time);
    case(tc_name)
        "general_task"            :general_task(64'h6000_0000,"/home/xiaofa/project/OSR_eHSM_HP/i_sim/tc/case/hex/import_key_sm4.hex","import_key_sm4",key_import_return_handle);
        "ehsm_ccm_cipher_test"    :ehsm_ccm_cipher_test()       ;
        "ehsm_ecdsa_verify_test"  :ehsm_ecdsa_verify_test()     ;
        "ehsm_gcm_cipher_test"    :ehsm_gcm_cipher_test()       ;
        "ehsm_gmac_gen_test"      :ehsm_gmac_gen_test()         ;
        "ehsm_hash_sha256_test"   :ehsm_hash_sha256_test()      ; 
        "ehsm_hmac_sm3_test"      :ehsm_hmac_sm3_test()         ;
        "ehsm_mac_gen_test"       :ehsm_mac_gen_test()          ;
        "ehsm_pke_rsa_test"       :ehsm_pke_rsa_test()          ;
        "ehsm_pke_sm2_test"       :ehsm_pke_sm2_test()          ;
        "ehsm_rsa_sign_test"      :ehsm_rsa_sign_test()         ;
        "ehsm_ske_sm4_test"       :ehsm_ske_sm4_test()          ;
        "ehsm_sm2_verify"         :ehsm_sm2_verify()            ;
        "ehsm_trng_test"          :ehsm_trng_test()             ;
        "ehsm_get_challenge"      :ehsm_get_challenge()         ;
        "ehsm_debug_auth_test"    :ehsm_debug_auth_test()       ;
        default:#10s;
    endcase

    if(tc_name == "ram_if_test") begin
        //check pass flag
        check_pass_flag();
    end else begin
        //check result
        check_result();
    end
end

initial begin
    force tb.u_osr_fpga_top.upper_i_hsel_s_ext4      = upper_hsel      ;
    force tb.u_osr_fpga_top.upper_i_haddr_s_ext4     = upper_haddr     ;
    force tb.u_osr_fpga_top.upper_i_htrans_s_ext4    = upper_htrans    ;
    force tb.u_osr_fpga_top.upper_i_hwrite_s_ext4    = upper_hwrite    ;
    force tb.u_osr_fpga_top.upper_i_hsize_s_ext4     = upper_hsize     ;
    force tb.u_osr_fpga_top.upper_i_hburst_s_ext4    = upper_hburst    ;
    force tb.u_osr_fpga_top.upper_i_hprot_s_ext4     = upper_hprot     ;
    force tb.u_osr_fpga_top.upper_i_hmaster_s_ext4   = upper_hmaster   ;
    force tb.u_osr_fpga_top.upper_i_hwdata_s_ext4    = upper_hwdata    ;
    force tb.u_osr_fpga_top.upper_i_hmastlock_s_ext4 = upper_hmastlock ;
    force tb.u_osr_fpga_top.upper_i_hready_s_ext4    = hreadyout_s_ext4;
    force tb.u_osr_fpga_top.upper_i_hauser_s_ext4    = upper_hauser    ;
    force tb.u_osr_fpga_top.upper_i_hwuser_s_ext4    = upper_hwuser    ;

                force tb.u_osr_fpga_top.u_upper.Jqcba4.o_cpu_srst_n=1'h0;
end

always@(posedge dut_clk or negedge dut_rst_n) begin
    if(!dut_rst_n) begin
        clk_cnt <= 64'h0 ;
    end else begin
        clk_cnt <= clk_cnt + 64'h1 ;
    end
end

always @ (*) begin
if(tc_name == "max_power_test") begin
   case (tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step)
       1   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START UTC TIMER             ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       2   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START GENERAL TIMER         ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       3   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START WATCHDOGE TIMER       ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       4   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START CRC LFSR              ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       5   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START MONOTONIC COUNTER     ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       6   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START PKE ALG               ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       7   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START SKE ALG               ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       8   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START HASH ALG              ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
       9   : begin $display("eHSM POWER_TEST INFO: [%2d] @ %8d  START TRNG                  ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
   endcase
end
end

always @ (*) begin
if(tc_name != "max_power_test") begin
//  $display("---------------------------Sim time---Event-------------------");
    case (tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step)
        1   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Start dec key               ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        2   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Start read self-flag        ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        3   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Boot load start             ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        4   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Read head                   ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        5   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Analysis head info          ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        6   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Move code                   ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        7   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Check sign                  ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        8   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Calc hash                   ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        9   : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Verify sign:start           ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
        255 : begin $display("eHSM Bootloader INFO: [%2d] @ %8d  Verify sign:end             ",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.sys_run_step,$time); end
    endcase
end
end

// PART 5 : assign for output ports

// PART 6 : instances of sub modules

osr_fpga_top u_osr_fpga_top (
    .OSCCLK             (OSCCLK             ), // input  [        ]
    .OSCCLK_N           (OSCCLK_N           ), // input  [        ]
    .RST_N              (RST_N              ), // input  [        ]
    .i_upper_CS_TCK     (1'h1               ), // JTAG TCK
    .i_upper_CS_TMS     (1'h1               ), // JTAG TMS
    .i_upper_tdi        (1'h1               ), // JTAG TDI
    .o_upper_tdo        (                   ), // JTAG TDO
    .i_upper_uart_rxd   (1'h1               ),
    .o_upper_uart_txd   (o_upper_uart_txd   ),
    .i_upper_uart1_rxd  (1'h1               ),
    .o_upper_uart1_txd  (                   ),

    .o_hsm_led          (hsm2soc_sta        ),
    .i_hsm_tck          (i_hsm_tck          ), // input  [        ]
    .i_hsm_trst_n       (i_hsm_trst_n       ), // input  [        ]
    .i_hsm_tms          (i_hsm_tms          ), // input  [        ]
    .i_hsm_tdi          (i_hsm_tdi          ), // input  [        ]
    .o_hsm_tdo          (o_hsm_tdo          ), // output [        ]
    .i_hsm_uart_rxd     (i_hsm_uart_rxd     ), // input  [        ]
    .o_hsm_uart_txd     (o_hsm_uart_txd     )  // output [        ]
);

reg          uart_capture_clk ;
//gen uart clk
initial
    begin:gen_uart_baudrate_clk    //115200
        uart_capture_clk = 1'b0 ;
        forever #(4340.27ns) uart_capture_clk = ~uart_capture_clk;
    end
uart_capture #(.PREFIX(8'h30))u_soc_cap(
  .RESETn             (RST_N           ),  // Power on reset
  .CLK                (uart_capture_clk),  // Clock (baud rate)
  .RXD                (o_upper_uart_txd),  // Received data
  .SIMULATIONEND      (),  // Simulation end indicator
  .DEBUG_TESTER_ENABLE(),  // Enable debug tester
  .AUXCTRL            (),  // Auxiliary control
  .SPI0               (),  // Shield0 SPI enable
  .SPI1               (),  // Shield1 SPI enable
  .I2C0               (),  // Shield0 I2C enable
  .I2C1               (),  // Shield1 I2C enable
  .UART0              (),  // Shield0 UART enable
  .UART1              () );// Shield1 UART enable

uart_capture #(.PREFIX(8'h31))u_ehsm_cap(
  .RESETn             (RST_N         ),  // Power on reset
  .CLK                (uart_capture_clk),  // Clock (baud rate)
  .RXD                (o_hsm_uart_txd  ),  // Received data
  .SIMULATIONEND      (),  // Simulation end indicator
  .DEBUG_TESTER_ENABLE(),  // Enable debug tester
  .AUXCTRL            (),  // Auxiliary control
  .SPI0               (),  // Shield0 SPI enable
  .SPI1               (),  // Shield1 SPI enable
  .I2C0               (),  // Shield0 I2C enable
  .I2C1               (),  // Shield1 I2C enable
  .UART0              (),  // Shield0 UART enable
  .UART1              () );// Shield1 UART enable

// PART 7 : always blocks

//time_out error
initial
begin:time_out
    //repeat(10000000) @(posedge OSCCLK);
    #10s;
    $display("Time out fail\n");
    $display("\n");
    $display("\n");
        $display(" #########          #          #########   #              ");
        $display(" #                 # #             #       #              ");
        $display(" #                #   #            #       #              ");
        $display(" #               #     #           #       #              ");
        $display(" #######        #########          #       #              ");
        $display(" #             #         #         #       #              ");
        $display(" #            #           #        #       #              ");
        $display(" #           #             #       #       #              ");
        $display(" #          #               #  #########   ##########     ");
    $display("\n");
    $display("\n");
    $display("\n");
    $finish();
end

endmodule // tb
