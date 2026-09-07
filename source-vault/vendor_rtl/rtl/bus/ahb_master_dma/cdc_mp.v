//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_cdc_mp #(
    parameter p_DATA_WIDTH         =32,
    parameter p_ADDR_WIDTH         =32,
    parameter p_CH_FIFO_EXIST      = 1,
    parameter p_LINK_LIST_EN       = 1,
    parameter p_AHB_SIMPLE         = 0,
    parameter p_CDC_MS_EN          = 1,
    parameter p_CDC_MP_EN          = 1,
    parameter p_R_FIFO_DEPTH_WIDTH = 3,
    parameter p_W_FIFO_DEPTH_WIDTH = 3,
    parameter p_CH_FIFO_DEPTH_WIDTH= 3
)(

    input   wire              i_m_clk  ,
    input   wire              i_m_rst_n ,

    input   wire              i_p_clk  ,
    input   wire              i_p_rst_n ,

    input   wire [1:0]        i_p2m_req,
    output  wire [1:0]        o_p2m_req,

    input   wire [1:0]        i_m2p_finish,
    output  wire [1:0]        o_m2p_finish,
    output  wire [1:0]        o_p2m_finish_back,

    input   wire [1:0]        i_m2p_ack,
    output  wire [1:0]        o_m2p_ack,

    output  wire              o_w_resp ,
    output  wire              o_r_resp  
);
    reg  r_ack_d    ;
    wire r_ack_d_in ;
    wire r_resp     ;
    reg  w_ack_d    ;
    wire w_ack_d_in ;
    wire w_resp     ;

    wire [1:0] p_finish;
    wire [1:0] m_finish;

generate
if(p_CDC_MP_EN) begin:mp_async
    osr_sync_pp #(.p_DWIDTH(2)) u_p2m_dff0(.i_clk  (i_m_clk),          .i_in  (i_p2m_req   ),                                        .o_out  (o_p2m_req   ));

    osr_sync_pp #(.p_DWIDTH(2)) u_m2p_dff0(.i_clk  (i_p_clk),          .i_in  (i_m2p_finish),                                        .o_out  (p_finish    ));
    osr_sync_pp #(.p_DWIDTH(2)) u_p2m_dff1(.i_clk  (i_m_clk),          .i_in  (p_finish    ),                                        .o_out  (m_finish    ));

    osr_async_pulse u_m2p_p2p0(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2p_ack[0]),.i_b_clk(i_p_clk),.i_b_rst_n(i_p_rst_n),.o_b_out(o_m2p_ack[0]));
    osr_async_pulse u_m2p_p2p1(.i_a_clk(i_m_clk),.i_a_rst_n(i_m_rst_n),.i_a_in(i_m2p_ack[1]),.i_b_clk(i_p_clk),.i_b_rst_n(i_p_rst_n),.o_b_out(o_m2p_ack[1]));

    osr_async_pulse u_p2m_p2p0(.i_a_clk(i_p_clk),.i_a_rst_n(i_p_rst_n),.i_a_in(r_resp      ),.i_b_clk(i_m_clk),.i_b_rst_n(i_m_rst_n),.o_b_out(o_r_resp    ));
    osr_async_pulse u_p2m_p2p1(.i_a_clk(i_p_clk),.i_a_rst_n(i_p_rst_n),.i_a_in(w_resp      ),.i_b_clk(i_m_clk),.i_b_rst_n(i_m_rst_n),.o_b_out(o_w_resp    ));

    assign o_m2p_finish      = p_finish&o_m2p_ack     ;
    assign o_p2m_finish_back = m_finish               ;
end
else begin:mp_sync
    assign o_p2m_req         = i_p2m_req              ;

    assign o_m2p_finish      = i_m2p_finish&i_m2p_ack ;
    assign o_p2m_finish_back = i_m2p_finish           ;

    assign o_m2p_ack         = i_m2p_ack              ;
    assign o_r_resp          = r_resp                 ;
    assign o_w_resp          = w_resp                 ;
end
endgenerate

    assign r_ack_d_in = o_m2p_ack[0];    
    assign r_resp     = r_ack_d&i_p2m_req[0];

    assign w_ack_d_in = o_m2p_ack[1];
    assign w_resp     = w_ack_d&i_p2m_req[1];

    always@(posedge i_p_clk or negedge i_p_rst_n)
    begin
        if(~i_p_rst_n) begin
            r_ack_d <= 1'b0;
            w_ack_d <= 1'b0;

        end
        else begin
            r_ack_d <=   r_ack_d_in ;
            w_ack_d <=   w_ack_d_in ;
        end
    end

endmodule
