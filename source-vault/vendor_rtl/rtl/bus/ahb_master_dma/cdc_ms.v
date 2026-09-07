//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_cdc_ms
(

    input   wire              i_s_clk  ,
    input   wire              i_s_rst_n ,

    input   wire              i_m_clk  ,
    input   wire              i_m_rst_n ,

    input   wire [1:0]        i_s2m_p2p,
    output  wire [1:0]        o_s2m_p2p,

    input   wire [1:0]        i_s2m_dff,
    output  wire [1:0]        o_s2m_dff,

    input   wire [9:0]        i_m2s_p2p,
    output  wire [9:0]        o_m2s_p2p
);
    osr_async_pulse u_s2m_p2p0(.i_a_clk(i_s_clk),.i_a_rst_n(i_s_rst_n),.i_a_in(i_s2m_p2p[0]),.i_b_clk(i_m_clk),.i_b_rst_n(i_m_rst_n),.o_b_out(o_s2m_p2p[0]));
    osr_async_pulse u_s2m_p2p1(.i_a_clk(i_s_clk),.i_a_rst_n(i_s_rst_n),.i_a_in(i_s2m_p2p[1]),.i_b_clk(i_m_clk),.i_b_rst_n(i_m_rst_n),.o_b_out(o_s2m_p2p[1]));

    osr_sync_pp #(.p_DWIDTH(2)) u_s2m_dff0(.i_clk  (i_m_clk),         .i_in   (i_s2m_dff),                                           .o_out  (o_s2m_dff));

    osr_async_pulse u_m2s_p2p0(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[0]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[0]));
    osr_async_pulse u_m2s_p2p1(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[1]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[1]));
    osr_async_pulse u_m2s_p2p2(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[2]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[2]));
    osr_async_pulse u_m2s_p2p3(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[3]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[3]));
    osr_async_pulse u_m2s_p2p4(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[4]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[4]));
    osr_async_pulse u_m2s_p2p5(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[5]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[5]));
    osr_async_pulse u_m2s_p2p6(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[6]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[6]));
    osr_async_pulse u_m2s_p2p7(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[7]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[7]));
    osr_async_pulse u_m2s_p2p8(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[8]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[8]));
    osr_async_pulse u_m2s_p2p9(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2s_p2p[9]),.i_b_clk(i_s_clk),.i_b_rst_n(i_s_rst_n),.o_b_out(o_m2s_p2p[9]));                                                                               
endmodule
