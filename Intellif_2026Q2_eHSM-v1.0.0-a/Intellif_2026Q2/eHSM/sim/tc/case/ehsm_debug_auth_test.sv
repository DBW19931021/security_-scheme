//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

//debug auth 0x00FBFF04
reg   [31:0]                              ehsm_debug_auth_cmd_id     ;
reg   [63:0]                              ehsm_debug_auth_reserved0  ;
reg   [7:0]                               ehsm_debug_auth_type       ;
reg   [7:0]                               ehsm_debug_auth_alg        ;
reg   [79:0]                              ehsm_debug_auth_reserved1  ;
reg   [64-1:0]                            ehsm_debug_auth_sign_addr  ;
reg   [31:0]                              ehsm_debug_auth_sign_size  ;
reg   [64-1:0]                            ehsm_debug_auth_pub_addr   ;
reg   [31:0]                              ehsm_debug_auth_pub_size   ;
reg   [63:0]                              soc_dbg_bitmap_addr        ;
reg   [31:0]                              soc_dbg_bitmap_size        ;
reg   [95:0]                              ehsm_debug_auth_reserved2  ;

wire  [511+2*(64-32):0]  ehsm_debug_auth_cmd = {
 ehsm_debug_auth_reserved2
,soc_dbg_bitmap_size
,soc_dbg_bitmap_addr
,ehsm_debug_auth_pub_size   
,ehsm_debug_auth_pub_addr   
,ehsm_debug_auth_sign_size  
,ehsm_debug_auth_sign_addr  
,ehsm_debug_auth_reserved1  
,ehsm_debug_auth_alg        
,ehsm_debug_auth_type       
,ehsm_debug_auth_reserved0  
,ehsm_debug_auth_cmd_id     
};

reg [31:0]   ehsm_debug_challenge[$]        ;
reg [31:0]   ehsm_debug_uid[$]              ;
reg [31:0]   ehsm_debug_prikey[$]           ;
reg [31:0]   ehsm_debug_pubkey[$]           ;
reg [31:0]   ehsm_debug_sign[$]             ;

task ehsm_debug_auth_test();
       ehsm_debug_auth_cmd_id              =  32'h00FBFF04;
       ehsm_debug_auth_reserved0           =  64'h0;
       ehsm_debug_auth_type                =  8'h1;
       ehsm_debug_auth_alg                 =  8'h1;
       ehsm_debug_auth_reserved1           =  80'h0;
       ehsm_debug_auth_sign_addr           =  {{64-32{1'b0}},32'h6000_2000};
       ehsm_debug_auth_sign_size           =  32'd64;
       ehsm_debug_auth_pub_addr            =  {{64-32{1'b0}},32'h6000_3000};
       ehsm_debug_auth_pub_size            =  32'd65;
       soc_dbg_bitmap_addr                 =  64'h0;
       soc_dbg_bitmap_size                 =  32'h0;
       ehsm_debug_auth_reserved2           =  'h0;

    ehsm_get_challenge();

    @(posedge osc_clk);
    //debug_auth cmd test
    for(i=0;i<DEBUG_AUTH_CMD_NUM;i=i+1) begin
        $display("++++++++++++++++++++++++++DEBUG_AUTH+++++++++++++++++++++++++++");

        $display("------------------------ehsm_debug_challenge------------------------");
        ehsm_debug_challenge = {>>int {ehsm_challenge}};
        sprint_u32_q(ehsm_debug_challenge,"ehsm_debug_challenge");

        $display("------------------------ehsm_debug_uid------------------------------");
        ehsm_debug_uid = {>>int {128'hefcdab89131211101312111013121110}};
        sprint_u32_q(ehsm_debug_uid,"ehsm_debug_uid");

        $display("------------------------ehsm_debug_prikey---------------------------");
        ehsm_debug_prikey = {>>int {256'hC428BB4722FA2F5DB3BD274ACD80966C9083F1A45E8CDF3B0AA645FE883F3C7B}};
        sprint_u32_q(ehsm_debug_prikey,"ehsm_debug_prikey");

        $display("------------------------debug_pubkey---------------------------");
        ehsm_debug_pubkey = {>>int {520'h04D14AAD32D7F4930631A5B1F5EBFEFA9CC3B3D5E5A74855D16387F183592612AEA5EE015068BB1080D9A50CCA25A56AE4323FE05E62806AB974C0714091B335CE}};
        cmd_state = DEBUG_AUTH_SET_PUPKEY;
        set_data(ehsm_debug_auth_pub_addr,ehsm_debug_auth_pub_size,ehsm_debug_pubkey);

        $display("------------------------ehsm_debug_sign-----------------------------");
        //rank k in signing:CCA32757089C04E0BB5B447994D02F0A72A8A2734B141E43E5AD63A9E6E6E6B428EEEBEC29FB72F6D0DC4816CB808558EDADA45E7E0B5BD833E87262B9742931
        ehsm_debug_sign   = {>>int{512'hCCA32757089C04E0BB5B447994D02F0A72A8A2734B141E43E5AD63A9E6E6E6B428EEEBEC29FB72F6D0DC4816CB808558EDADA45E7E0B5BD833E87262B9742931}};// SM2 2M3
        cmd_state = DEBUG_AUTH_SET_SIGN;
        set_data(ehsm_debug_auth_sign_addr,ehsm_debug_sign.size(),ehsm_debug_sign);

        $display("------------------------debug_auth cmd-------------------------");
        cmd_state = DEBUG_AUTH_SET_CMD;
        set_ehsm_cmd(ehsm_debug_auth_cmd);
        cmd_buf_q = {<<byte {ehsm_debug_auth_cmd}};
        sprint_u8_q(cmd_buf_q,"debug_auth_cmd_buf");
        cmd_buf_q = {};
        upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h100,32'h1);
        debug_auth_start=clk_cnt ;
        $display(" debug_auth_start = %0h",debug_auth_start);

        //wait done
        cmd_state = DEBUG_AUTH_WAIT_DONE;
        upper_ahb_rd(MAILBOX_DEBUG_BASE+64'h104,reg_rdata);
        while(reg_rdata[0]!=1'b1) begin
            upper_ahb_rd(MAILBOX_DEBUG_BASE+64'h104,reg_rdata);
        end
        upper_ahb_wr(MAILBOX_DEBUG_BASE+64'h104,32'h1);

        //get rsp
        $display("-----------------------debug_auth rsp--------------------------");
        cmd_state = DEBUG_AUTH_GET_RSP;
            upper_ahb_rd({MBOX_RSP_BUFFER_H,MBOX_RSP_BUFFER_L},reg_rdata);
        if(reg_rdata !== 32'h0000A55A) begin
            $display("resp = %0h",reg_rdata);
            s_result_x.cmd_rsp = CMD_RSP_ERROR;
            $finish;
        end else begin
            s_result_x.cmd_rsp = CMD_RSP_OK;
        end
        $display("cmd_rsp = %0h",reg_rdata);
        $display("cmd_rsp = %s",s_result_x.cmd_rsp);

        //check result
        s_result_x.cmd_id = cmd_id_e'(ehsm_debug_auth_cmd_id);
        s_result_x.result_s = result;
        s_result_x.rm_result_s = {};
        if(reg_rdata !== 32'h0000A55A) begin
            s_result_x.flag_s = 0;
        end else begin
            s_result_x.flag_s = 1;
        end
        s_result_q.push_back(s_result_x);

        //clean up
        result = {};
        rm_result = {};
        reg_rdata = 32'h0;

        $display(" debug_auth_end = %0h",clk_cnt);
        $display(" debug_auth_cycle = %0h",(clk_cnt-debug_auth_start));
        debug_auth_bw=((`OSR_SIM_CLK_FREQ/(clk_cnt-debug_auth_start))*(DEBUG_AUTH_CMD_NUM*ehsm_debug_auth_sign_size))/1048576.000000;//MBs
        $display("Average DEBUG_AUTH BandWidth is %f MB/s at %0d MHz",debug_auth_bw,`OSR_SIM_CLK_FREQ/1000000.000000);
    end

    wait(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.pack_sys_hsm_sta1[16] == 1'b1);
    $display("INFO: sys_sta0[16] = %h",tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_sys.pack_sys_hsm_sta1[16]);
    $display("=============================================================");
    $display("                     DEBUG AUTH SUCCESS                      ");
    $display("=============================================================");

endtask
