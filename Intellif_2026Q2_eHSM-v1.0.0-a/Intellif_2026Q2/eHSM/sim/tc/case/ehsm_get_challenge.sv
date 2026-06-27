//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

reg [31:0]  ehsm_challenge[$]                ;

//get_challenge 0x00FCFF03
reg [31:0]    ehsm_get_challenge_cmd_id      ;
reg [63:0]    ehsm_get_challenge_reserved0   ;
reg [7:0]     ehsm_get_challenge_type        ;
reg [279:0]   ehsm_get_challenge_reserved1   ;
reg [64-1:0]  ehsm_get_challenge_addr        ;
reg [95:0]    ehsm_get_challenge_reserved2   ;

wire[511+1*(64-32):0] ehsm_get_challenge_cmd = {
 ehsm_get_challenge_reserved2       
,ehsm_get_challenge_addr   
,ehsm_get_challenge_reserved1  
,ehsm_get_challenge_type    
,ehsm_get_challenge_reserved0     
,ehsm_get_challenge_cmd_id  
};

task ehsm_get_challenge();

    ehsm_get_challenge_cmd_id      = 32'h00FCFF03    ;
    ehsm_get_challenge_reserved0   = 64'h0           ;
    ehsm_get_challenge_type        = 1'h1            ;
    ehsm_get_challenge_reserved1   = 280'h0          ;
    ehsm_get_challenge_addr        = {{64-32{1'b0}},32'h6000_1000 };
    ehsm_get_challenge_reserved2   = 96'h0           ;

    @(posedge osc_clk);
    //get_challenge cmd test
    for(i=0;i<GET_CHALLENGE_CMD_NUM;i=i+1) begin
        $display("++++++++++++++++++++++++++GET_CHALLENGE+++++++++++++++++++++++++++");
        $display("------------------------get_challenge cmd-------------------------");
        cmd_state = GET_CHALLENGE_SET_CMD;
        set_ehsm_cmd(ehsm_get_challenge_cmd);
        cmd_buf_q = {<<byte {ehsm_get_challenge_cmd}};
        sprint_u8_q(cmd_buf_q,"ehsm_get_challenge_cmd_buf");
        cmd_buf_q = {};
        upper_ahb_wr(MAILBOX_BASE+64'h100,32'h1);

        get_challenge_start=clk_cnt ;
        wait(tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_trng.o_trng_rdy == 1);
        $display("=========================TRNG_READY==========================");

        force tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_trng.u_trng.Y821Zc.Y538Zc.i_rng_dr[31:0] = 32'haaaa_aaaa;
        $display("=========================TRNG_FORCE==========================");

        //wait done
        cmd_state = GET_CHALLENGE_WAIT_DONE;
        upper_ahb_rd(MAILBOX_BASE+64'h104,reg_rdata);
        while(reg_rdata[0]!=1'b1) begin
            upper_ahb_rd(MAILBOX_BASE+64'h104,reg_rdata);
        end
        upper_ahb_wr(MAILBOX_BASE+64'h104,32'h1);

        //get rsp
        $display("-----------------------get_challenge rsp--------------------------");
        cmd_state = GET_CHALLENGE_GET_RSP;
            upper_ahb_rd({MBOX_RSP_BUFFER_H,MBOX_RSP_BUFFER_L},reg_rdata);
        if(reg_rdata !== 32'h0000A55A) begin
            s_result_x.cmd_rsp = CMD_RSP_ERROR;
            s_result_x.flag_s  = 0;
            $display("resp = %0h",reg_rdata);
            $display("cmd_rsp = %0s","CMD_RSP_ERROR");
            $display("=============================================================");
            $display("                   GET CHALLENGE FAILED                      ");
            $display("=============================================================");
            #10ns;
            $finish;
        end else begin
            s_result_x.cmd_rsp = CMD_RSP_OK;
            s_result_x.flag_s  = 1;
            $display("resp = %0h",reg_rdata);
            $display("cmd_rsp = %0s","CMD_RSP_OK");
        end

        //get result
        $display("-----------------------get_challenge result-----------------------");
        cmd_state = GET_CHALLENGE_GET_RESULT;
        get_result(ehsm_get_challenge_addr,8,result);
        ehsm_challenge = {>>int {result}};
        s_result_q.push_back(s_result_x);

        //clean up
        result = {};
        reg_rdata = 32'h0;

        get_challenge_bw=((`OSR_SIM_CLK_FREQ/(clk_cnt-get_challenge_start))*(GET_CHALLENGE_CMD_NUM*32))/1048576.000000;//MBs
        $display("Average GET_CHALLENGE BandWidth is %f MB/s at %0d MHz",get_challenge_bw,`OSR_SIM_CLK_FREQ/1000000);
    end

      release tb.u_osr_fpga_top.u_osr_ehsm_top_wrapper.u_osr_ehsm_top.u_ahb_wrapper.u_trng.u_trng.Y821Zc.Y538Zc.i_rng_dr[31:0];
endtask
