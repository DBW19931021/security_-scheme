//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

//bl_verify_image 0x00f9ff06
reg [31:0]                      bl_verify_image_cmd_id              ;
reg [79:0]                      bl_verify_image_reserved0           ;
reg [7:0]                       bl_verify_image_check_version       ;
reg [7:0]                       bl_verify_image_boot_after_verify   ;
reg [63:0]                      bl_verify_image_addr                ;
reg [31:0]                      bl_verify_image_size                ;
reg [63:0]                      bl_verify_image_out_addr            ;
reg [63:0]                      bl_verify_image_code_addr           ;
reg [7:0]                       bl_verify_image_only_copy_code      ;

wire[543:0]  bl_verify_image_cmd = { 
 bl_verify_image_only_copy_code
,bl_verify_image_code_addr
,bl_verify_image_out_addr
,bl_verify_image_size
,bl_verify_image_addr
,bl_verify_image_boot_after_verify
,bl_verify_image_check_version
,bl_verify_image_reserved0
,bl_verify_image_cmd_id
};

task ehsm_bl_verify_image();
    bl_verify_image_cmd_id              = 32'h00f9ff06  ;
    bl_verify_image_reserved0           = 80'h0         ;
    bl_verify_image_check_version       = 8'h0          ;
    bl_verify_image_boot_after_verify   = 8'h1          ;
    bl_verify_image_addr                = 32'h6004_0000 ; 
    bl_verify_image_size                = 32'h00024BE0  ; // need to update each time when firmware changes
    bl_verify_image_out_addr            = 32'h0         ;
    bl_verify_image_code_addr           = 64'h0         ;
    bl_verify_image_only_copy_code      = 8'h0          ;

        $display("++++++++++++++++++++++++++BL_VERIFY_IMAGE++++++++++++++++++++++++++++");

        repeat(10)@(posedge osc_clk);

        $display("------------------------ehsm bl verify image cmd--------------------------");
        //set cmd
        set_ehsm_cmd(bl_verify_image_cmd);
        cmd_buf_q = {<<byte {bl_verify_image_cmd}};
        sprint_u8_q(cmd_buf_q,"bl_verify_image_cmd_buf");
        cmd_buf_q = {};
        upper_ahb_wr(MAILBOX_BASE+64'h100,32'h1);

        //wait done
        upper_ahb_rd(MAILBOX_BASE+64'h104,reg_rdata);
        while(reg_rdata[0]!=1'b1) begin
            upper_ahb_rd(MAILBOX_BASE+64'h104,reg_rdata);
        end
        upper_ahb_wr(MAILBOX_BASE+64'h104,32'h1);

        //get rsp
        $display("-----------------------ehsm bl verify image rsp---------------------------");
        upper_ahb_rd({MBOX_RSP_BUFFER_H,MBOX_RSP_BUFFER_L},reg_rdata);
        if(reg_rdata !== 32'ha55a) begin
            $display("EHSM_BL_VERIFY_IMAGE CMD RSP ERROR !!!",reg_rdata);
            $display("resp = %0h",reg_rdata);
            s_result_x.cmd_rsp = CMD_RSP_ERROR;
            $finish;
        end else begin
            $display("EHSM_BL_VERIFY_IMAGE CMD RSP OK !!!",reg_rdata);
            s_result_x.cmd_rsp = CMD_RSP_OK;
            s_result_x.flag_s = 1;
        end
        $display("cmd_rsp = %s",s_result_x.cmd_rsp);
        s_result_x.cmd_id = cmd_id_e'(bl_verify_image_cmd_id);
        $display("cmd_rsp_flag = %h",reg_rdata);
        s_result_q.push_back(s_result_x);
        //clean up
        result = {};
        //rm_result = {};
        reg_rdata = 32'h0;

endtask
