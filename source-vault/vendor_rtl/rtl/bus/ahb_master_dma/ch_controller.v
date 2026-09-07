//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ch_controller #(
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

    input   wire                                         i_m_clk               ,
    input   wire                                         i_m_rst_n             ,

    output  wire [ 7:0]                                  o_ch_int_sta_pulse    ,

    output  wire                                         o_r_blk_done          ,
    output  wire                                         o_w_blk_done          ,

    input   wire                                         i_ch_star             ,
    input   wire                                         i_ch_en               ,

    input   wire [ 7:0]                                  i_ch_to               ,
    input   wire                                         i_ch_r_fifo_clr       , 
    input   wire                                         i_ch_w_fifo_clr       , 
    input   wire                                         i_ch_lli_en           , 
    input   wire                                         i_ch_fifo_sel         ,

    input   wire [ 7:0]                                  i_ch_r_max_burst      ,
    input   wire                                         i_ch_r_en             ,
    input   wire                                         i_ch_r_ed_revr        ,
    input   wire                                         i_ch_r_trig_sel       ,
    input   wire                                         i_ch_r_atyp           ,
    input   wire [ 7:0]                                  i_ch_w_max_burst      ,
    input   wire                                         i_ch_w_en             ,
    input   wire                                         i_ch_w_ed_revr        ,
    input   wire                                         i_ch_w_trig_sel       ,
    input   wire                                         i_ch_w_atyp           ,
    input   wire                                         i_ch_w_lli_wb         ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_ch_saddr            ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_ch_daddr            ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_ch_rlen             ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_ch_wlen             ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_ch_llp              ,

    input   wire [ 7:0]                                  i_ch_bus              ,

    input   wire [p_DATA_WIDTH-1:0]                      i_w_fifo_data           ,
    output  wire                                         o_w_fifo_rd_enable      ,
    input   wire                                         i_w_fifo_empty          ,
    input   wire [p_W_FIFO_DEPTH_WIDTH:0]                i_w_fifo_num_count      ,

    output  wire                                         o_w_fifo_clear          ,

    output  wire [p_DATA_WIDTH-1:0]                      o_r_fifo_data           ,
    output  wire                                         o_r_fifo_wr_enable      ,
    input   wire                                         i_r_fifo_full           ,
    input   wire [p_R_FIFO_DEPTH_WIDTH:0]                i_r_fifo_num_count      ,

    output  wire                                         o_r_fifo_clear          ,

    input   wire                                         i_w_req                 ,
    input   wire                                         i_w_resp                ,
    output  wire                                         o_w_ack                 ,
    output  wire                                         o_w_finish              ,
    input   wire                                         i_w_finish_back         ,

    input   wire                                         i_r_req                 ,
    input   wire                                         i_r_resp                ,
    output  wire                                         o_r_ack                 ,
    output  wire                                         o_r_finish              ,
    input   wire                                         i_r_finish_back         ,

    input   wire [p_ADDR_WIDTH-1:0]                      i_w_base_addr_ns        ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_w_trans_len_ns        ,
    input   wire [ 7:0]                                  i_w_strans_len_ns       ,
    output  wire [p_ADDR_WIDTH-1:0]                      o_w_base_addr           ,
    output  wire [p_ADDR_WIDTH-1:0]                      o_w_trans_len           ,
    output  wire [ 7:0]                                  o_w_strans_len          ,

    input   wire                                         i_w_rd_en               ,
    output  wire [p_DATA_WIDTH-1:0]                      o_w_data                ,
    input   wire                                         i_w_strans_done_addr    ,
    input   wire                                         i_w_trans_done_addr     ,
    input   wire                                         i_w_strans_done_data    ,

    input   wire [p_ADDR_WIDTH-1:0]                      i_r_base_addr_ns        ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_r_trans_len_ns        ,
    input   wire [7-p_DATA_WIDTH/32:0]                   i_r_strans_len_burst_ns ,
    output  wire [p_ADDR_WIDTH-1:0]                      o_r_base_addr           ,
    output  wire [p_ADDR_WIDTH-1:0]                      o_r_trans_len           ,
    output  wire [ 7:0]                                  o_r_strans_len          ,
    output  wire [7-p_DATA_WIDTH/32:0]                   o_r_strans_len_burst    , 

    input   wire [p_DATA_WIDTH-1:0]                      i_r_data                ,
    input   wire                                         i_r_wr_en               ,
    input   wire                                         i_r_strans_done_addr    ,
    input   wire                                         i_r_trans_done_addr     ,
    input   wire                                         i_r_strans_done_data    ,
    input   wire                                         i_w_to                  ,
    input   wire                                         i_r_to                  ,

    output  wire [7:0]                                   o_ch_to                 ,
    output  wire [7:0]                                   o_ch_bus                ,

    output  wire                                         o_ch_r_atyp             ,
    output  wire                                         o_ch_w_atyp             ,

    output  wire                                         o_r_revr_en             ,
    output  wire                                         o_w_revr_en             
);

    wire        start_redge           ;
    wire        start_pulse           ;

    wire        lli_en                ;

    reg  [2:0]  lli_reg_sel           ;
    reg         lli_reg_upd_en        ;
    reg         lli_reg_rupd_en       ;
    reg         lli_reg_wupd_en       ;
    reg         lli_reg_rupd_done     ;
    reg         lli_reg_wupd_done     ;

    wire [2:0]  lli_reg_sel_in        ;
    wire        lli_reg_upd_en_in     ;
    wire        lli_reg_rupd_en_in    ;
    wire        lli_reg_wupd_en_in    ;
    wire        lli_reg_rupd_done_in  ;
    wire        lli_reg_wupd_done_in  ;

    wire        lli_reg_upd_done      ;

    wire        lli_reg_rupd_en_pulse ;
    wire        lli_reg_wupd_en_pulse ;

    reg  [2:0]                  lli_ch_wlen               ;
    reg  [2:0]                  lli_w_strans_len          ;
    reg  [5:0]                  lli_ch_rlen               ;
    reg  [4-p_DATA_WIDTH/32:0]  lli_r_strans_len_burst    ;

    wire [2:0]                  lli_ch_wlen_in            ;
    wire [2:0]                  lli_w_strans_len_in       ;
    wire [5:0]                  lli_ch_rlen_in            ;
    wire [4-p_DATA_WIDTH/32:0]  lli_r_strans_len_burst_in ;

    reg  [15:0]                 lli_trans_done_cnt        ;
    wire [15:0]                 lli_trans_done_cnt_in     ;

    wire        w_trig_star         ;

    wire        w_fifo_rdy          ;
    wire        w_trig_req          ;
    wire        w_ack_redge         ;
    reg         w_ack               ;
    wire        w_ack_in            ;
    wire        w_ack_pulse         ;
    wire        w_req_pulse         ;

    reg  [ 7:0] w_strans_len        ;
    wire [ 7:0] w_strans_len_in     ;
    wire [ 7:0] w_strans_len_small  ;

    wire        r_trig_star         ;

    wire        r_fifo_rdy          ;
    wire        r_trig_req          ;
    wire        r_ack_redge         ;
    reg         r_ack               ;
    wire        r_ack_in            ;
    wire        r_ack_pulse         ;
    wire        r_req_pulse         ;

    reg         r_strans_busy        ;
    wire        r_strans_busy_in     ;
    reg         w_strans_busy        ;
    wire        w_strans_busy_in     ;

    reg  [7-p_DATA_WIDTH/32:0] r_strans_len_burst    ;
    wire [7-p_DATA_WIDTH/32:0] r_strans_len_burst_in ;

    wire [ 7:0]                r_strans_len_small    ;

    wire [p_DATA_WIDTH-1:0]        w_fifo_sel_data      ; 
    wire                           w_fifo_sel_rd_enable ; 
    wire                           w_fifo_sel_empty     ; 
    wire [8:0]                     w_fifo_sel_num_count ; 
    wire                           w_fifo_sel_clear     ; 

    wire [p_DATA_WIDTH-1:0]        r_fifo_sel_data      ; 
    wire                           r_fifo_sel_wr_enable ; 
    wire                           r_fifo_sel_full      ; 
    wire [8:0]                     r_fifo_sel_num_count ; 
    wire                           r_fifo_sel_clear     ;

    wire [p_DATA_WIDTH-1:0]        w_ch_fifo_data       ; 
    wire                           w_ch_fifo_rd_enable  ; 
    wire                           w_ch_fifo_empty      ; 

    wire [p_DATA_WIDTH-1:0]        r_ch_fifo_data       ; 
    wire                           r_ch_fifo_wr_enable  ; 
    wire                           r_ch_fifo_full       ; 

    wire                           wr_ch_fifo_clear     ;
    wire [p_CH_FIFO_DEPTH_WIDTH:0] wr_ch_fifo_num_count ; 

    reg  w_strans_done                      ;
    reg  r_strans_done                      ;

    reg  w_trans_done                       ;
    reg  r_trans_done                       ;
    reg  w_trans_all_done                   ;
    reg  r_trans_all_done                   ;

    reg  w_to                               ;
    reg  r_to                               ;

    wire r_strans_done_in                   ;
    wire w_strans_done_in                   ;
    wire w_trans_done_in                    ;
    wire r_trans_done_in                    ;

    reg  r_strans_done_addr                 ;
    wire r_strans_done_addr_in              ;

    reg  w_strans_done_addr                 ;
    wire w_strans_done_addr_in              ;

    reg  r_trans_done_addr                  ;
    wire r_trans_done_addr_in               ;

    reg  w_trans_done_addr                  ;
    wire w_trans_done_addr_in               ;

    wire [ 7:0] ch_r_max_burst              ;
    wire        ch_r_en                     ;
    wire        ch_r_ed_revr                ;
    wire        ch_r_trig_sel               ;
    wire        ch_r_atyp                   ;

    wire [ 7:0] ch_w_max_burst              ;
    wire        ch_w_en                     ;
    wire        ch_w_ed_revr                ;
    wire        ch_w_trig_sel               ;
    wire        ch_w_atyp                   ;
    wire        ch_w_lli_wb                 ;

    reg  [p_ADDR_WIDTH-1:0] ch_saddr        ;

    reg  [p_ADDR_WIDTH-1:0] ch_daddr        ;

    reg  [p_ADDR_WIDTH-1:0] ch_rlen         ;

    reg                     ch_last_lli     ;
    reg  [p_ADDR_WIDTH-1:0] ch_wlen         ;

    reg  [p_ADDR_WIDTH-1:0] ch_llp          ;

    wire                    ch_saddr_hit    ;
    wire                    ch_daddr_hit    ;
    wire                    ch_rlen_hit     ;
    wire                    ch_wlen_hit     ;
    wire                    ch_llp_hit      ;

    wire [p_ADDR_WIDTH-1:0] ch_saddr_in     ;

    wire [p_ADDR_WIDTH-1:0] ch_daddr_in     ;

    wire [p_ADDR_WIDTH-1:0] ch_rlen_in      ;

    wire                    ch_last_lli_in  ;
    wire [p_ADDR_WIDTH-1:0] ch_wlen_in      ;

    wire [p_ADDR_WIDTH-1:0] ch_llp_in       ;

    wire     r_trans_done_clr           ;
    wire     w_trans_done_clr           ;
    wire     r_strans_done_clr          ;
    wire     w_strans_done_clr          ;

    wire     r_blk_done                 ;
    wire     w_blk_done                 ;
    wire     r_blk_done_pulse           ;
    wire     w_blk_done_pulse           ;

    reg      reg_ready                  ;
    wire     reg_ready_in               ;

    reg      reg_ready_d                ;
    wire     reg_ready_d_in             ;

    reg      r_trans_busy               ;
    wire     r_trans_busy_in            ;
    reg      w_trans_busy               ;
    wire     w_trans_busy_in            ;

    wire     ch_r_fifo_clr              ; 
    wire     ch_w_fifo_clr              ; 
    wire     ch_lli_en                  ;
    wire     ch_fifo_sel                ;

    wire     reg_w_strans_done_in       ;
    wire     reg_r_strans_done_in       ;
    wire     reg_w_trans_done_in        ;
    wire     reg_r_trans_done_in        ;
    wire     reg_w_trans_all_done_in    ;
    wire     reg_r_trans_all_done_in    ;

    osr_redge_to_pulse #(
        .p_NUM(4)
    ) u_e2p1(
        .clk      (i_m_clk  ),
        .rst_n    (i_m_rst_n),
        .in_edge  ({i_w_req,i_r_req,w_ack_redge,r_ack_redge}),
        .out_pulse({w_req_pulse,r_req_pulse,w_ack_pulse,r_ack_pulse})
    );
    assign w_ack_redge=w_ack&((~o_w_finish)|i_w_finish_back);
    assign r_ack_redge=r_ack&((~o_r_finish)|i_r_finish_back);

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            w_ack <= 1'b0;
            r_ack <= 1'b0;
        end
        else begin
            w_ack <=    w_ack_in;
            r_ack <=    r_ack_in;
        end
    end

    assign w_ack_in   = (i_w_resp|(~i_w_req))          ? 1'b0 :
                         w_strans_done_in&w_trans_busy ? 1'b1 : w_ack ;

    assign r_ack_in   = (i_r_resp|(~i_r_req))          ? 1'b0 :
                         r_strans_done_in&r_trans_busy ? 1'b1 : r_ack ;

    assign o_w_ack    = w_ack_pulse      ;

    assign o_w_finish = w_trans_done_addr&ch_w_en&((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en)));

    assign o_r_ack    = r_ack_pulse      ;
    assign o_r_finish = r_trans_done_addr&ch_r_en&((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en)));

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            r_trans_busy <= 1'b0;
            w_trans_busy <= 1'b0;
        end
        else begin
            r_trans_busy <=  r_trans_busy_in;
            w_trans_busy <=  w_trans_busy_in;
        end
    end

    assign r_trans_busy_in = (start_pulse&ch_r_en&(i_ch_rlen != {p_ADDR_WIDTH{1'b0}}))                 ? 1'b1 :
                             (reg_r_trans_all_done_in|(~i_ch_star)) ? 1'b0 : r_trans_busy ;

    assign w_trans_busy_in = (start_pulse&ch_w_en&(i_ch_wlen != {p_ADDR_WIDTH{1'b0}}))                 ? 1'b1 :
                             (reg_w_trans_all_done_in|(~i_ch_star)) ? 1'b0 : w_trans_busy ;

    assign reg_ready_in=lli_reg_rupd_en_pulse|(~i_ch_en)           ? 1'b0 :
                        ((start_pulse&(~lli_en))|lli_reg_upd_done) ? 1'b1 : reg_ready ;

    assign reg_ready_d_in=reg_ready;
    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            reg_ready   <= 1'b0;
            reg_ready_d <= 1'b0;
        end
        else begin
            reg_ready   <= reg_ready_in;
            reg_ready_d <= reg_ready_d_in;
        end
    end

    assign start_redge  = i_ch_star;

    assign r_trig_req   = i_r_req&ch_r_trig_sel&(~r_ack)&(~r_strans_done_addr);

    assign r_fifo_rdy   = r_fifo_sel_num_count>=(r_strans_len_small[7:p_DATA_WIDTH/32+1]+{|r_strans_len_small[p_DATA_WIDTH/32:0]});

    assign r_trig_star  = reg_ready_d& 
                          (r_trig_req|(~ch_r_trig_sel))&
                          r_fifo_rdy&
                          i_ch_star&ch_r_en&
                          (~r_strans_busy);

    assign w_trig_req   = i_w_req&ch_w_trig_sel&(~w_ack)&(~w_strans_done_addr);

    assign w_fifo_rdy   = w_fifo_sel_num_count>=(w_strans_len_small[7:p_DATA_WIDTH/32+1]+{|w_strans_len_small[p_DATA_WIDTH/32:0]});

    assign w_trig_star  = reg_ready_d&
                          (w_trig_req|(~ch_w_trig_sel))&
                          w_fifo_rdy&
                          i_ch_star&ch_w_en&
                          (~w_strans_busy);

    osr_redge_to_pulse #(
        .p_NUM(3)
    ) u_e2p0(
        .clk      (i_m_clk),
        .rst_n    (i_m_rst_n),
        .in_edge  ({lli_reg_wupd_en,lli_reg_rupd_en,start_redge}),
        .out_pulse({lli_reg_wupd_en_pulse,lli_reg_rupd_en_pulse,start_pulse})
    );

    assign r_strans_busy_in=(i_r_strans_done_data)                ? 1'b0 :
                            (r_trig_star)&(r_strans_len_small!=0) ? 1'b1 : r_strans_busy ;

    assign w_strans_busy_in=(i_w_strans_done_data)                ? 1'b0 :
                            (w_trig_star)&(w_strans_len_small!=0) ? 1'b1 : w_strans_busy ;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            r_strans_busy <= 1'b0 ;
            w_strans_busy <= 1'b0 ;
        end
        else begin
            r_strans_busy <=   r_strans_busy_in;
            w_strans_busy <=   w_strans_busy_in;

        end
    end

generate
    if(p_DATA_WIDTH==32) begin:reg_bank_32b

        assign ch_saddr_hit= (lli_reg_sel==1);
        assign ch_daddr_hit= (lli_reg_sel==2);
        assign ch_rlen_hit = (lli_reg_sel==3);
        assign ch_wlen_hit = (lli_reg_sel==4);
        assign ch_llp_hit  = (lli_reg_sel==5);

        assign ch_last_lli_in = (start_pulse)                           ? 1'b0                              :
                                (lli_reg_upd_en&ch_wlen_hit &i_r_wr_en) ? i_r_data[31]                      : ch_last_lli ;

        assign ch_saddr_in    = (start_pulse)                           ? i_ch_saddr                        :
                                (lli_reg_upd_en&ch_saddr_hit&i_r_wr_en) ? i_r_data[p_ADDR_WIDTH-1:0]        :
                                (ch_r_atyp)&(~lli_reg_upd_en)           ? ch_saddr                          : 
                                (r_strans_busy)                         ? i_r_base_addr_ns                  : ch_saddr ;

        assign ch_daddr_in    = (start_pulse)                                        ? i_ch_daddr                        :
                                (lli_reg_wupd_en_pulse)                              ? ch_llp                            :
                                (lli_reg_upd_en&ch_daddr_hit&i_r_wr_en)              ? i_r_data[p_ADDR_WIDTH-1:0]        :
                                (ch_w_atyp)&(~lli_reg_upd_en)                        ? ch_daddr                          : 
                                (w_strans_busy|lli_reg_wupd_en)&(~lli_reg_wupd_done) ? i_w_base_addr_ns                  : ch_daddr ; 

        assign ch_rlen_in     = (start_pulse)                           ? i_ch_rlen                         :
                                (lli_reg_upd_en&ch_rlen_hit &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH-1:0]        :
                                (r_strans_busy)                         ? i_r_trans_len_ns                  : ch_rlen ; 

        assign ch_wlen_in     = (start_pulse)                           ? i_ch_wlen                         :
                                (lli_reg_upd_en&ch_wlen_hit &i_r_wr_en)&
                                (p_ADDR_WIDTH<32)                       ? i_r_data[p_ADDR_WIDTH-1:0]        :
                                (lli_reg_upd_en&ch_wlen_hit &i_r_wr_en)&
                                (p_ADDR_WIDTH==32)                      ? {1'b0,i_r_data[p_ADDR_WIDTH-2:0]} :
                                (w_strans_busy)                         ? i_w_trans_len_ns                  : ch_wlen ;

        if(p_LINK_LIST_EN) begin:reg_bank_lli_32b
        assign ch_llp_in      = (start_pulse)                           ? i_ch_llp                          :
                                (lli_reg_rupd_en&ch_llp_hit &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH-1:0]        : 
                                (lli_reg_rupd_en)                       ? i_r_base_addr_ns                  : ch_llp         ;
        end
        else begin:reg_bank_lli_no_32b
        assign ch_llp_in      = {p_ADDR_WIDTH{1'b0}};           
        end
    end
    else begin:reg_bank_64b
        assign ch_saddr_hit= (lli_reg_sel==0);
        assign ch_daddr_hit= (lli_reg_sel==1);
        assign ch_rlen_hit = (lli_reg_sel==1);
        assign ch_wlen_hit = (lli_reg_sel==2);
        assign ch_llp_hit  = (lli_reg_sel==2);

        assign ch_last_lli_in = (start_pulse)                           ? 1'b0                              :
                                (lli_reg_upd_en&ch_wlen_hit &i_r_wr_en) ? i_r_data[31]                      : ch_last_lli ;

        assign ch_saddr_in    = (start_pulse)                           ? i_ch_saddr                        :
                                (lli_reg_upd_en&ch_saddr_hit&i_r_wr_en) ? i_r_data[p_ADDR_WIDTH+31:32]      :
                                (ch_r_atyp)&(~lli_reg_upd_en)           ? ch_saddr                          : 
                                (r_strans_busy)                         ? i_r_base_addr_ns                  : ch_saddr ;     

        assign ch_daddr_in    = (start_pulse)                                        ? i_ch_daddr                        :
                                (lli_reg_wupd_en_pulse)                              ? ch_llp                            :
                                (lli_reg_upd_en&ch_daddr_hit&i_r_wr_en)              ? i_r_data[p_ADDR_WIDTH-1:0]        :
                                (ch_w_atyp)&(~lli_reg_upd_en)                        ? ch_daddr                          : 
                                (w_strans_busy|lli_reg_wupd_en)&(~lli_reg_wupd_done) ? i_w_base_addr_ns                  : ch_daddr ;

        assign ch_rlen_in     = (start_pulse)                           ? i_ch_rlen                         :
                                (lli_reg_upd_en&ch_rlen_hit &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH+31:32]      :
                                (r_strans_busy)                         ? i_r_trans_len_ns                  : ch_rlen ;

        assign ch_wlen_in     = (start_pulse)                           ? i_ch_wlen                         :
                                (lli_reg_upd_en&ch_wlen_hit &i_r_wr_en)&
                                (p_ADDR_WIDTH<32)                       ? i_r_data[p_ADDR_WIDTH-1:0]        :
                                (lli_reg_upd_en&ch_wlen_hit &i_r_wr_en)&
                                (p_ADDR_WIDTH==32)                      ? {1'b0,i_r_data[p_ADDR_WIDTH-2:0]} :
                                (w_strans_busy)                         ? i_w_trans_len_ns                  : ch_wlen ; 

        if(p_LINK_LIST_EN) begin:reg_bank_lli_64b
        assign ch_llp_in      = (start_pulse)                           ? i_ch_llp                          :
                                (lli_reg_rupd_en&ch_llp_hit &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH+31:32]      : 
                                (lli_reg_rupd_en)                       ? i_r_base_addr_ns                  : ch_llp ;
        end
        else begin:reg_bank_lli_no_64b
        assign ch_llp_in      = {p_ADDR_WIDTH{1'b0}};           
        end

    end
endgenerate

    assign w_strans_len_in      = (w_trig_star)&(w_strans_len==8'b0)&(ch_wlen!=0)    ? w_strans_len_small  : i_w_strans_len_ns ;

    assign w_strans_len_small   = (ch_wlen<ch_w_max_burst)                           ? ch_wlen[7:0]        : ch_w_max_burst ;

    assign r_strans_len_burst_in= (r_trig_star)&(r_strans_len_burst==0)&(ch_rlen!=0) ? {1'b0,r_strans_len_small[7:p_DATA_WIDTH/32+1]}+
                                                                                       (|ch_saddr[p_DATA_WIDTH/32:0])+
                                                                                       (|r_strans_len_small[p_DATA_WIDTH/32:0]) :
                                                                                       i_r_strans_len_burst_ns ;

    assign r_strans_len_small   = (ch_rlen<ch_r_max_burst)                           ? ch_rlen[7:0]                          : ch_r_max_burst ;

    assign o_r_strans_len       = r_strans_len_small ;
generate
    if(p_CH_FIFO_EXIST) begin:ch_fifo_exist

        sync_FIFO #(
         .pDEPTH_WIDTH       (p_CH_FIFO_DEPTH_WIDTH         ),
         .pDATA_WIDTH        (p_DATA_WIDTH                  )
        )u_ch_fifo(
         .O_data             (w_ch_fifo_data                ),
         .O_empty            (w_ch_fifo_empty               ),
         .O_al_empty         (                              ),
         .I_al_empty_thres   ({p_CH_FIFO_DEPTH_WIDTH{1'b0}} ),
         .I_rd_enable        (w_ch_fifo_rd_enable           ),

         .I_data             (r_ch_fifo_data                ),
         .O_full             (r_ch_fifo_full                ),
         .O_al_full          (                              ),
         .I_al_full_thres    ({p_CH_FIFO_DEPTH_WIDTH{1'b0}} ),
         .I_wr_enable        (r_ch_fifo_wr_enable           ),

         .O_num_count        (wr_ch_fifo_num_count          ),
         .I_clear            (wr_ch_fifo_clear              ),
         .clk                (i_m_clk                       ),
         .resetn             (i_m_rst_n                     )
        );

        assign w_fifo_sel_data      =(~ch_fifo_sel) ? i_w_fifo_data        : w_ch_fifo_data       ;

        assign w_fifo_sel_rd_enable =(lli_reg_upd_en)? 1'b0                : i_w_rd_en            ;                          
        assign o_w_fifo_rd_enable   =(~ch_fifo_sel) ? w_fifo_sel_rd_enable : 1'b0                 ;
        assign w_ch_fifo_rd_enable  =(~ch_fifo_sel) ? 1'b0                 : w_fifo_sel_rd_enable ;

        assign w_fifo_sel_empty     =(~ch_fifo_sel) ? i_w_fifo_empty       : w_ch_fifo_empty      ; 
        assign w_fifo_sel_num_count =(~ch_fifo_sel) ? {{(8-p_W_FIFO_DEPTH_WIDTH){1'b0}},i_w_fifo_num_count} :
                                                      {{(8-p_CH_FIFO_DEPTH_WIDTH){1'b0}},wr_ch_fifo_num_count};

        assign w_fifo_sel_clear     =ch_w_fifo_clr;                                   
        assign o_w_fifo_clear       =(~ch_fifo_sel) ? w_fifo_sel_clear     : 1'b0                              ;
        assign wr_ch_fifo_clear     =(~ch_fifo_sel) ? 1'b0                 : w_fifo_sel_clear|r_fifo_sel_clear ;

        assign r_fifo_sel_data      =i_r_data ;                               
        assign o_r_fifo_data        =(~ch_fifo_sel) ? r_fifo_sel_data      : 1'b0                 ;
        assign r_ch_fifo_data       =(~ch_fifo_sel) ? 1'b0                 : r_fifo_sel_data      ;

        assign r_fifo_sel_wr_enable =(lli_reg_upd_en)? 1'b0                 : i_r_wr_en;                                      
        assign o_r_fifo_wr_enable   =(~ch_fifo_sel)  ? r_fifo_sel_wr_enable : 1'b0                 ;
        assign r_ch_fifo_wr_enable  =(~ch_fifo_sel)  ? 1'b0                 : r_fifo_sel_wr_enable ;

        assign r_fifo_sel_full      =(~ch_fifo_sel)  ? i_r_fifo_full        : r_ch_fifo_full       ; 
        assign r_fifo_sel_num_count =(~ch_fifo_sel)  ? {{(8-p_R_FIFO_DEPTH_WIDTH){1'b0}},i_r_fifo_num_count} :
                                                       {1'b1,{(p_CH_FIFO_DEPTH_WIDTH){1'b0}}}-{{(8-p_CH_FIFO_DEPTH_WIDTH){1'b0}},wr_ch_fifo_num_count}; 

        assign r_fifo_sel_clear     =ch_r_fifo_clr;                                   
        assign o_r_fifo_clear       =(~ch_fifo_sel)  ? r_fifo_sel_clear     : 1'b0                 ;
    end
    else begin:ch_fifo_none

        assign w_fifo_sel_data      = i_w_fifo_data ;

        assign w_fifo_sel_rd_enable =(lli_reg_upd_en)? 1'b0 : i_w_rd_en;                                           
        assign o_w_fifo_rd_enable   = w_fifo_sel_rd_enable             ;

        assign w_fifo_sel_empty     = i_w_fifo_empty     ; 
        assign w_fifo_sel_num_count = {{(8-p_W_FIFO_DEPTH_WIDTH){1'b0}},i_w_fifo_num_count};

        assign w_fifo_sel_clear     = ch_w_fifo_clr    ;                                                
        assign o_w_fifo_clear       = w_fifo_sel_clear ;

        assign r_fifo_sel_data      = i_r_data        ;                                            
        assign o_r_fifo_data        = r_fifo_sel_data ;

        assign r_fifo_sel_wr_enable =(lli_reg_upd_en) ? 1'b0 : i_r_wr_en;                                           
        assign o_r_fifo_wr_enable   = r_fifo_sel_wr_enable;

        assign r_fifo_sel_full      = i_r_fifo_full     ; 
        assign r_fifo_sel_num_count = {{(8-p_R_FIFO_DEPTH_WIDTH){1'b0}},i_r_fifo_num_count}; 

        assign r_fifo_sel_clear     =ch_r_fifo_clr   ;                                                
        assign o_r_fifo_clear       =r_fifo_sel_clear;

    end
endgenerate

    assign o_w_base_addr      = ch_daddr;
    assign o_w_trans_len      = (lli_reg_wupd_en) ? {{(p_ADDR_WIDTH-3){1'b0}},lli_ch_wlen}               : ch_wlen;
    assign o_w_strans_len     = (lli_reg_wupd_en) ? {5'b0,lli_w_strans_len}                              : w_strans_len;
    assign o_w_data           = (lli_reg_wupd_en) ? {{(p_DATA_WIDTH-32){1'b0}},16'h3,lli_trans_done_cnt} : w_fifo_sel_data ;

    assign o_r_base_addr       = (lli_reg_rupd_en) ? ch_llp                                 : ch_saddr ;
    assign o_r_trans_len       = (lli_reg_rupd_en) ? {{(p_ADDR_WIDTH-6){1'b0}},lli_ch_rlen} : ch_rlen  ;
    assign o_r_strans_len_burst= (lli_reg_rupd_en) ? {3'b0,lli_r_strans_len_burst}          : r_strans_len_burst ;

generate
if(p_LINK_LIST_EN) begin:ch_lli_logic_exist

    assign lli_en= (ch_lli_en);
end
else begin:ch_lli_logic_none

    assign lli_en= 1'b0;
end
endgenerate

    assign lli_reg_sel_in            = (lli_reg_rupd_en_pulse)                           ? 3'h0             :
                                       (lli_reg_rupd_en&i_r_wr_en)                       ? lli_reg_sel+3'b1 : lli_reg_sel ;

    assign lli_reg_rupd_en_in        = ((start_pulse&(~ch_w_lli_wb))|
                                        (lli_reg_wupd_en&i_w_strans_done_data)|
                                        (((ch_rlen==0)&(ch_wlen==0)&(~r_strans_busy)&(~w_strans_busy))&((~ch_w_lli_wb)|(lli_reg_wupd_done))))&
                                       lli_en&(~ch_last_lli)&i_ch_star                   ? 1'b1 :
                                       (lli_reg_upd_done)|(~lli_en)                      ? 1'b0 : lli_reg_rupd_en ;

    assign lli_reg_wupd_en_in        = (start_pulse|
                                        ((ch_rlen==0)&(ch_wlen==0)&(~w_strans_busy)&(~r_strans_busy)))&
                                       lli_en&(~ch_last_lli)&ch_w_lli_wb&i_ch_star       ? 1'b1 :
                                       (lli_reg_upd_done)|(~lli_en)                      ? 1'b0 : lli_reg_wupd_en ;

    assign lli_reg_upd_en_in         = (lli_reg_rupd_en_in|lli_reg_wupd_en_in)&lli_en    ? 1'b1 :
                                        lli_reg_upd_done                                 ? 1'b0 : lli_reg_upd_en ;

    assign lli_reg_rupd_done_in      =  lli_reg_rupd_en&ch_llp_hit&i_r_wr_en             ? 1'b1 :
                                        lli_reg_upd_done                                 ? 1'b0 : lli_reg_rupd_done ;

    assign lli_reg_wupd_done_in      =  lli_reg_wupd_en&i_w_trans_done_addr              ? 1'b1 :
                                        (lli_reg_upd_done)                               ? 1'b0 : lli_reg_wupd_done ;

    assign lli_reg_upd_done          =  lli_reg_rupd_done ;

    assign lli_ch_wlen_in            =  (lli_reg_wupd_en_pulse) ? 3'h4                   :
                                        (lli_reg_wupd_en)       ? i_w_trans_len_ns[2:0]  : lli_ch_wlen ;

    assign lli_w_strans_len_in       = (lli_reg_wupd_en_pulse)  ? 3'h4                   :
                                       (lli_reg_wupd_en)        ? i_w_strans_len_ns[2:0] : lli_w_strans_len ;

    assign lli_ch_rlen_in            = (lli_reg_rupd_en_pulse)  ? 6'h18                  :
                                       (lli_reg_rupd_en)        ? i_r_trans_len_ns[5:0]  : lli_ch_rlen ;

    assign lli_r_strans_len_burst_in = (lli_reg_rupd_en_pulse)  ? lli_ch_rlen_in[5:p_DATA_WIDTH/32+1]+
                                                                  (|ch_llp[p_DATA_WIDTH/32:0])+(|lli_ch_rlen_in[p_DATA_WIDTH/32:0]) :
                                       (lli_reg_rupd_en)        ? i_r_strans_len_burst_ns[4-p_DATA_WIDTH/32:0]                      : lli_r_strans_len_burst ;

    assign lli_trans_done_cnt_in     = (start_pulse)      ? 16'b0                :
                                       (lli_reg_upd_done) ? lli_trans_done_cnt+1 : lli_trans_done_cnt ;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            lli_reg_sel           <= 3'b0;
            lli_reg_rupd_en       <= 1'b0;
            lli_reg_wupd_en       <= 1'b0;
            lli_reg_upd_en        <= 1'b0;
            lli_reg_rupd_done     <= 1'b0;
            lli_reg_wupd_done     <= 1'b0;

            lli_ch_wlen           <= 3'b0; 
            lli_w_strans_len      <= 3'b0; 
            lli_ch_rlen           <= 6'b0; 
            lli_r_strans_len_burst<= {(5-p_DATA_WIDTH/32){1'b0}};
            lli_trans_done_cnt    <= 16'b0; 
        end
        else begin
            lli_reg_sel           <=   lli_reg_sel_in            ;
            lli_reg_rupd_en       <=   lli_reg_rupd_en_in        ;
            lli_reg_wupd_en       <=   lli_reg_wupd_en_in        ;
            lli_reg_upd_en        <=   lli_reg_upd_en_in         ;
            lli_reg_rupd_done     <=   lli_reg_rupd_done_in      ;
            lli_reg_wupd_done     <=   lli_reg_wupd_done_in      ;

            lli_ch_wlen           <=   lli_ch_wlen_in            ; 
            lli_w_strans_len      <=   lli_w_strans_len_in       ; 
            lli_ch_rlen           <=   lli_ch_rlen_in            ; 
            lli_r_strans_len_burst<=   lli_r_strans_len_burst_in ;
            lli_trans_done_cnt    <=   lli_trans_done_cnt_in     ; 
        end
    end

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            ch_last_lli        <=1'b0 ;

            ch_saddr           <={p_ADDR_WIDTH{1'b0}};

            ch_daddr           <={p_ADDR_WIDTH{1'b0}};

            ch_rlen            <={p_ADDR_WIDTH{1'b0}};

            ch_wlen            <={p_ADDR_WIDTH{1'b0}};

            ch_llp             <={p_ADDR_WIDTH{1'b0}};

            w_strans_len       <=8'b0 ;

            r_strans_len_burst <={(8-p_DATA_WIDTH/32){1'b0}};

        end
        else begin
            ch_last_lli   <=  ch_last_lli_in ;

            ch_saddr        <=  ch_saddr_in  ;

            ch_daddr        <=  ch_daddr_in  ;

            ch_rlen         <=  ch_rlen_in   ;

            ch_wlen         <=  ch_wlen_in   ;

            ch_llp          <=  ch_llp_in    ;

            w_strans_len    <=  w_strans_len_in;

            r_strans_len_burst <=  r_strans_len_burst_in ;

        end
    end

    assign r_trans_done_clr  =((~r_strans_busy)&r_strans_busy_in)|((i_r_resp|r_req_pulse)&ch_r_trig_sel)|(start_pulse);
    assign w_trans_done_clr  =((~w_strans_busy)&w_strans_busy_in)|((i_w_resp|w_req_pulse)&ch_w_trig_sel)|(start_pulse);
    assign r_strans_done_clr =((~r_strans_busy)&r_strans_busy_in)|((i_r_resp|r_req_pulse)&ch_r_trig_sel)|(start_pulse);
    assign w_strans_done_clr =((~w_strans_busy)&w_strans_busy_in)|((i_w_resp|w_req_pulse)&ch_w_trig_sel)|(start_pulse);

    assign w_strans_done_addr_in= (lli_reg_upd_done|w_strans_done_clr)     ? 1'b0  :
                                  (i_w_strans_done_addr)&(~lli_reg_upd_en) ? 1'b1  : w_strans_done_addr ;

    assign w_strans_done_in     = (lli_reg_upd_done|w_strans_done_clr)     ? 1'b0  :
                                  (w_strans_done_addr&i_w_strans_done_data)? 1'b1  : w_strans_done ;

    assign r_strans_done_addr_in= (lli_reg_upd_done|r_strans_done_clr)     ? 1'b0  :
                                  (i_r_strans_done_addr)&(~lli_reg_upd_en) ? 1'b1  : r_strans_done_addr ;

    assign r_strans_done_in     = (lli_reg_upd_done|r_strans_done_clr)     ? 1'b0  :
                                  (r_strans_done_addr&i_r_strans_done_data)? 1'b1  : r_strans_done ;

    assign w_trans_done_addr_in = (lli_reg_upd_done|w_trans_done_clr)      ? 1'b0 :
                                  (i_w_trans_done_addr&(~lli_reg_upd_en))  ? 1'b1 : w_trans_done_addr ;

    assign w_trans_done_in      = (lli_reg_upd_done|w_trans_done_clr)      ? 1'b0  :
                                  (w_trans_done_addr&i_w_strans_done_data) ? 1'b1  : w_trans_done ;

    assign r_trans_done_addr_in = (lli_reg_upd_done|r_trans_done_clr)      ? 1'b0 :
                                  (i_r_trans_done_addr&(~lli_reg_upd_en))  ? 1'b1 : r_trans_done_addr ;

    assign r_trans_done_in      = (lli_reg_upd_done|r_trans_done_clr)      ? 1'b0 :
                                  (r_trans_done_addr&i_r_strans_done_data) ? 1'b1 : r_trans_done ;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            w_strans_done     <= 1'b0;
            w_strans_done_addr<= 1'b0;

            r_strans_done     <= 1'b0;
            r_strans_done_addr<= 1'b0;

            w_trans_done      <= 1'b0; 
            w_trans_done_addr <= 1'b0;

            r_trans_done      <= 1'b0;
            r_trans_done_addr <= 1'b0;

        end
        else begin
            w_strans_done     <=   w_strans_done_in ; 
            w_strans_done_addr<=   w_strans_done_addr_in  ;

            r_strans_done     <=   r_strans_done_in ;  
            r_strans_done_addr<=   r_strans_done_addr_in  ;

            w_trans_done_addr <=   w_trans_done_addr_in  ;
            w_trans_done      <=   w_trans_done_in  ;  

            r_trans_done_addr <=   r_trans_done_addr_in  ;
            r_trans_done      <=   r_trans_done_in  ;  

        end
    end

    assign reg_w_strans_done_in    = (w_strans_done_addr&i_w_strans_done_data) ;

    assign reg_r_strans_done_in    = (r_strans_done_addr&i_r_strans_done_data) ;

    assign reg_w_trans_done_in     = (w_trans_done_addr&i_w_strans_done_data) ;

    assign reg_r_trans_done_in     = (r_trans_done_addr&i_r_strans_done_data) ;

    assign reg_w_trans_all_done_in = w_trans_done_addr&i_w_strans_done_data&ch_w_en&
                                     ((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en))) ;

    assign reg_r_trans_all_done_in = r_trans_done_addr&i_r_strans_done_data&ch_r_en&
                                     ((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en))) ;

    assign r_blk_done=(lli_en) ? r_trans_done_addr : r_strans_done_addr ;
    assign w_blk_done=(lli_en) ? w_trans_done_addr : w_strans_done_addr ;

    osr_redge_to_pulse #(
        .p_NUM(10)
    ) u_e2p2(
        .clk(i_m_clk),
        .rst_n(i_m_rst_n),
        .in_edge(  {w_blk_done,r_blk_done,
                    i_w_to,i_r_to,reg_w_strans_done_in,reg_r_strans_done_in,
                    reg_w_trans_done_in,reg_r_trans_done_in,reg_w_trans_all_done_in,reg_r_trans_all_done_in}),
        .out_pulse({w_blk_done_pulse,r_blk_done_pulse,
                    o_ch_int_sta_pulse[7],o_ch_int_sta_pulse[6],o_ch_int_sta_pulse[5],o_ch_int_sta_pulse[4],
                    o_ch_int_sta_pulse[3],o_ch_int_sta_pulse[2],o_ch_int_sta_pulse[1],o_ch_int_sta_pulse[0]})
    );

    assign o_r_blk_done=r_blk_done_pulse;
    assign o_w_blk_done=w_blk_done_pulse;

    assign o_r_revr_en =  ch_r_ed_revr ;
    assign o_w_revr_en =  ch_w_ed_revr ;

    assign o_ch_r_atyp = (lli_reg_upd_en) ? 1'b0 : ch_r_atyp ;            
    assign o_ch_w_atyp = (lli_reg_upd_en) ? 1'b0 : ch_w_atyp ;                

    assign o_ch_to       = (i_ch_en) ? i_ch_to         : 8'b0;
    assign o_ch_bus      = (i_ch_en) ? i_ch_bus        : 8'b0;

    assign ch_r_fifo_clr = (i_ch_en) ? i_ch_r_fifo_clr : 1'b0;
    assign ch_w_fifo_clr = (i_ch_en) ? i_ch_w_fifo_clr : 1'b0;

    assign ch_lli_en     = (i_ch_en) ? i_ch_lli_en     : 1'b0;
    assign ch_fifo_sel   = (i_ch_en) ? i_ch_fifo_sel   : 1'b0;

    assign ch_r_max_burst =(i_ch_en) ? i_ch_r_max_burst : 8'b0 ;
    assign ch_r_en        =(i_ch_en) ? i_ch_r_en        : 1'b0 ;
    assign ch_r_ed_revr   =(i_ch_en) ? i_ch_r_ed_revr   : 1'b0 ;
    assign ch_r_trig_sel  =(i_ch_en) ? i_ch_r_trig_sel  : 1'b0 ;
    assign ch_r_atyp      =(i_ch_en) ? i_ch_r_atyp      : 1'b0 ;

    assign ch_w_max_burst =(i_ch_en) ? i_ch_w_max_burst : 8'b0 ;
    assign ch_w_en        =(i_ch_en) ? i_ch_w_en        : 1'b0 ;
    assign ch_w_ed_revr   =(i_ch_en) ? i_ch_w_ed_revr   : 1'b0 ;
    assign ch_w_trig_sel  =(i_ch_en) ? i_ch_w_trig_sel  : 1'b0 ;
    assign ch_w_atyp      =(i_ch_en) ? i_ch_w_atyp      : 1'b0 ;
    assign ch_w_lli_wb    =(i_ch_en) ? i_ch_w_lli_wb    : 1'b0 ;

endmodule
