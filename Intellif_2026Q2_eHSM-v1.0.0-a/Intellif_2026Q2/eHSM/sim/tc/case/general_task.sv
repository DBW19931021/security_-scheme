//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
task general_task(
input   reg   [63:0]   hex_addr ,  //base addr of SOC_MEM
input   string         hex_file ,  //path of hex file
input   string         cmd_name ,  //command name
output  reg   [31:0]   rsp_code    //return code
);
reg [31:0]     hex_data [0:1579]  ;
reg [31:0]     reg_rdata         ;
reg [31:0]     rsp_data0         ;
reg [31:0]     rsp_data1         ;
reg [31:0]     golden_result[$]  ;
reg [31:0]     actual_result[$]  ;

$display("################################ %s start ######################################\n",cmd_name);
//step_1 
$display("------------------------write hex_data---------------------------------------------\n");
$readmemh(hex_file,hex_data);
set_data_n(hex_addr,1580,hex_data); //write hex data to SOC_MEM

//step_2
$display("------------------------write TEST_PACKET_ADDR to s2h_info-------------------------\n");
upper_ahb_wr(MAILBOX_BASE+64'h1000,hex_addr[31:0]+32'h18a0); //send TEST_PACKET_ADDR to Mailbox

//step_3
$display("------------------------set s2h_note-----------------------------------------------\n");
upper_ahb_wr(MAILBOX_BASE+64'h1100,32'h1);  //write s2h_note, trigger Mbox irq

//step_4
$display("------------------------read h2s_note----------------------------------------------\n");
upper_ahb_rd(MAILBOX_BASE+64'h1104,reg_rdata); //read h2s_note
while(reg_rdata[0] != 1'b1) begin
    upper_ahb_rd(MAILBOX_BASE+64'h1104,reg_rdata); 
end
$display("h2s_note = %h",reg_rdata);

//step_5
$display("------------------------write 1 to clear h2s_note----------------------------------\n");
 upper_ahb_wr(MAILBOX_BASE+64'h1104,32'h1);

//step_6
$display("------------------------get result-------------------------------------------------\n");
get_result_n(hex_addr+64'h10a0,8,golden_result,"golden_result");
get_result_n(hex_addr+64'h8a0,8,actual_result,"actual_result");

actual_result = {};
golden_result = {};

//step_7
$display("------------------------get rsp----------------------------------------------------\n");
if(reg_rdata == 32'h1) begin
    upper_ahb_rd(hex_addr+64'h80,rsp_code );
    upper_ahb_rd(hex_addr+64'h84,rsp_data0);
    upper_ahb_rd(hex_addr+64'h88,rsp_data1);
end

$display("rsp_code  = %h",rsp_code );
$display("rsp_data0 = %h",rsp_data0);
$display("rsp_data1 = %h",rsp_data1);

if (rsp_code == 32'ha55a) begin
    s_result_x.cmd_rsp = CMD_RSP_OK;
    s_result_x.flag_s  = 1;
end else begin
    s_result_x.cmd_rsp = CMD_RSP_ERROR;
    s_result_x.flag_s  = 0;
end
s_result_x.cmd_name = cmd_name ;

$display("-----------------------------------------------------------------------------------\n");
$display("################################ %s end ######################################\n\n",cmd_name);

s_result_q.push_back(s_result_x);
key_import_bw=(`OSR_SIM_CLK_FREQ/((clk_cnt-key_import_start)/KEY_IMPORT_CMD_NUM));//times/s
$display("Average Key Import BandWidth is %f times/s at %0d MHz",key_import_bw,`OSR_SIM_CLK_FREQ/1000000.000000);

endtask
