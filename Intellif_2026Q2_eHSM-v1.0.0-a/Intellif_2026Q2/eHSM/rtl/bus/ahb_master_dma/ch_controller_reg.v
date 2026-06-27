//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ch_controller_reg #(
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

    input   wire                                         i_m_clk                 ,
    input   wire                                         i_m_rst_n               ,

    input   wire                                         i_reg_wr                ,
    input   wire [4:0]                                   i_reg_addr              ,
    input   wire [31:0]                                  i_reg_wdata             ,
    output  reg  [63:0]                                  o_reg_rdata             ,

    input   wire                                         i_dma_start             ,
    input   wire                                         i_suspend               ,
    input   wire                                         i_lli_en                ,
    input   wire [7:0]                                   i_ch_to                 ,
    input   wire                                         i_rd_endian             ,
    input   wire [7:0]                                   i_rd_max_byte           ,
    input   wire                                         i_wr_endian             ,
    input   wire [7:0]                                   i_wr_max_byte           ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_saddr                 ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_daddr                 ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_rlen                  ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_wlen                  ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_llp                   ,
    input   wire                                         i_w_to_int_en           ,
    input   wire                                         i_r_to_int_en           ,
    input   wire                                         i_w_dma_int_en          ,
    input   wire                                         i_r_dma_int_en          ,
    output  wire                                         o_w_dma_done            ,
    output  wire                                         o_r_dma_done            ,
    input   wire                                         i_w_to_clr              ,
    input   wire                                         i_r_to_clr              ,
    input   wire                                         i_w_dma_done_clr        ,
    input   wire                                         i_r_dma_done_clr        ,
    output  wire                                         o_to                    ,

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
    output  wire [p_ADDR_WIDTH-1:0]                      o_r_ch_saddr_max        ,

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
    output  wire                                         o_w_revr_en             ,

    output  wire                                         o_ch_int 
);

    localparam [4:0] REG_CH_STAR         = 5'h0;
    localparam [4:0] REG_CH_CTL          = 5'h1;
    localparam [4:0] REG_CH_RCTL         = 5'h2;
    localparam [4:0] REG_CH_WCTL         = 5'h3;
    localparam [4:0] REG_CH_SADDR        = 5'h4;
    localparam [4:0] REG_CH_DADDR        = 5'h5;   
    localparam [4:0] REG_CH_RLEN         = 5'h6;
    localparam [4:0] REG_CH_WLEN         = 5'h7;
    localparam [4:0] REG_CH_LLP          = 5'h8;
    localparam [4:0] REG_CH_BLK_CNT      = 5'h9;
    localparam [4:0] REG_CH_INTEN        = 5'hA;
    localparam [4:0] REG_CH_INTSTA       = 5'hB;
    localparam [4:0] REG_CH_INTCLR       = 5'hC;
    localparam [4:0] REG_CH_BUS          = 5'hD;

    wire [63:0]                 reg_rdata             ;

    wire                        start_redge           ;
    wire                        start_pulse           ;

    wire                        lli_en                ;

    reg  [2:0]                  lli_reg_sel           ;
    reg                         lli_reg_upd_en        ;
    reg                         lli_reg_rupd_en       ;
    reg                         lli_reg_wupd_en       ;
    reg                         lli_reg_rupd_done     ;
    reg                         lli_reg_wupd_done     ;

    wire [2:0]                  lli_reg_sel_in        ;
    wire                        lli_reg_upd_en_in     ;
    wire                        lli_reg_rupd_en_in    ;
    wire                        lli_reg_wupd_en_in    ;
    wire                        lli_reg_rupd_done_in  ;
    wire                        lli_reg_wupd_done_in  ;

    wire                        lli_reg_upd_done      ;

    wire                        lli_reg_rupd_en_pulse ;
    wire                        lli_reg_wupd_en_pulse ;

    reg  [2:0]                  lli_ch_wlen           ;
    reg  [2:0]                  lli_w_strans_len      ;
    reg  [5:0]                  lli_ch_rlen           ;
    reg  [4-p_DATA_WIDTH/32:0]  lli_r_strans_len_burst;

    wire [2:0]                  lli_ch_wlen_in           ;
    wire [2:0]                  lli_w_strans_len_in      ;
    wire [5:0]                  lli_ch_rlen_in           ;
    wire [4-p_DATA_WIDTH/32:0]  lli_r_strans_len_burst_in;

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

    reg         r_strans_busy       ;
    wire        r_strans_busy_in    ;
    reg         w_strans_busy       ;
    wire        w_strans_busy_in    ;

    reg  [7-p_DATA_WIDTH/32:0] r_strans_len_burst    ;
    wire [7-p_DATA_WIDTH/32:0] r_strans_len_burst_in ;

    wire [ 7:0]                r_strans_len_small    ;

    wire [p_DATA_WIDTH-1:0]             w_fifo_sel_data           ; 
    wire                                w_fifo_sel_rd_enable      ; 
    wire                                w_fifo_sel_empty          ; 
    wire [8:0]                          w_fifo_sel_num_count      ; 
    wire                                w_fifo_sel_clear          ; 

    wire [p_DATA_WIDTH-1:0]             r_fifo_sel_data           ; 
    wire                                r_fifo_sel_wr_enable      ; 
    wire                                r_fifo_sel_full           ; 
    wire [8:0]                          r_fifo_sel_num_count      ; 
    wire                                r_fifo_sel_clear          ;

    wire [p_DATA_WIDTH-1:0]             w_ch_fifo_data            ; 
    wire                                w_ch_fifo_rd_enable       ; 
    wire                                w_ch_fifo_empty           ; 

    wire [p_DATA_WIDTH-1:0]             r_ch_fifo_data            ; 
    wire                                r_ch_fifo_wr_enable       ; 
    wire                                r_ch_fifo_full            ; 

    wire                                wr_ch_fifo_clear          ;
    wire [p_CH_FIFO_DEPTH_WIDTH:0]      wr_ch_fifo_num_count      ; 

    reg  w_strans_done              ;
    reg  r_strans_done              ;

    reg  w_trans_done               ;
    reg  r_trans_done               ;
    reg  w_to                       ;
    reg  r_to                       ;

    wire r_strans_done_in           ;
    wire w_strans_done_in           ;
    wire w_trans_done_in            ;
    wire r_trans_done_in            ;
    wire w_to_in                    ;
    wire r_to_in                    ;

    reg  r_strans_done_addr         ;
    wire r_strans_done_addr_in      ;

    reg  w_strans_done_addr         ;
    wire w_strans_done_addr_in      ;

    reg  r_trans_done_addr          ;
    wire r_trans_done_addr_in       ;

    reg  w_trans_done_addr          ;
    wire w_trans_done_addr_in       ;

    reg         ch_star             ;
    reg         ch_en               ;

    reg  [ 7:0] ch_to               ;
    wire        ch_r_fifo_clr       ;
    wire        ch_w_fifo_clr       ;
    reg         ch_lli_en           ;
    reg         ch_fifo_sel         ;    

    reg  [ 7:0] ch_r_max_burst      ;
    reg         ch_r_en             ;
    reg         ch_r_ed_revr        ;
    reg         ch_r_trig_sel       ;
    reg         ch_r_atyp           ;

    reg  [ 7:0] ch_w_max_burst      ;
    reg         ch_w_en             ;
    reg         ch_w_ed_revr        ;
    reg         ch_w_trig_sel       ;
    reg         ch_w_atyp           ;
    reg         ch_w_lli_wb         ;

    reg  [p_ADDR_WIDTH-1:0] ch_saddr_max;

    reg  [p_ADDR_WIDTH-1:0] ch_saddr;

    reg  [p_ADDR_WIDTH-1:0] ch_daddr;

    reg  [p_ADDR_WIDTH-1:0] ch_rlen ;

    reg                     ch_last_lli;
    reg  [p_ADDR_WIDTH-1:0] ch_wlen ;

    reg  [p_ADDR_WIDTH-1:0] ch_llp  ;

    reg  [15:0] ch_w_blk_cnt        ;
    reg  [15:0] ch_r_blk_cnt        ;

    reg  [ 7:0] ch_int_en           ;

    wire [ 7:0] ch_int_sta          ;

    wire [ 7:0] ch_int_clr          ;

    reg  [ 7:0] ch_bus              ;

    wire        ch_star_in          ;
    wire        ch_en_in            ;

    wire [ 7:0] ch_to_in            ;
    wire        ch_lli_en_in        ;
    wire        ch_fifo_sel_in      ;

    wire [ 7:0] ch_r_max_burst_in   ;
    wire        ch_r_en_in          ;
    wire        ch_r_ed_revr_in     ;
    wire        ch_r_trig_sel_in    ;
    wire        ch_r_atyp_in        ;

    wire        ch_saddr_hit_lli    ;
    wire        ch_daddr_hit_lli    ;
    wire        ch_rlen_hit_lli     ;
    wire        ch_wlen_hit_lli     ;
    wire        ch_llp_hit_lli      ;

    wire [ 7:0] ch_w_max_burst_in   ;
    wire        ch_w_en_in          ;
    wire        ch_w_ed_revr_in     ;
    wire        ch_w_trig_sel_in    ;
    wire        ch_w_atyp_in        ;
    wire        ch_w_lli_wb_in      ;

    wire [p_ADDR_WIDTH-1:0] ch_saddr_max_in ;

    wire [p_ADDR_WIDTH-1:0] ch_saddr_in     ;

    wire [p_ADDR_WIDTH-1:0] ch_daddr_in     ;

    wire [p_ADDR_WIDTH-1:0] ch_rlen_in      ;

    wire                    ch_last_lli_in  ;
    wire [p_ADDR_WIDTH-1:0] ch_wlen_in      ;

    wire [p_ADDR_WIDTH-1:0] ch_llp_in       ;

    wire [15:0]             ch_w_blk_cnt_in ;

    wire [15:0]             ch_r_blk_cnt_in ;

    wire [ 7:0]             ch_int_en_in    ;

    wire [ 7:0]             ch_bus_in       ;

    wire ch_star_hit       ;
    wire ch_ctl_hit        ;
    wire ch_rctl_hit       ;
    wire ch_wctl_hit       ;
    wire ch_saddr_hit      ;
    wire ch_daddr_hit      ;
    wire ch_rlen_hit       ;
    wire ch_wlen_hit       ;
    wire ch_llp_hit        ;
    wire ch_blk_cnt_hit    ;
    wire ch_int_en_hit     ;
    wire ch_int_sta_hit    ;
    wire ch_int_clr_hit    ;
    wire ch_bus_hit        ;

    wire        r_trans_done_clr    ;
    wire        w_trans_done_clr    ;
    wire        r_strans_done_clr   ;
    wire        w_strans_done_clr   ;
    wire        w_to_clr            ;
    wire        r_to_clr            ;

    wire        r_blk_done          ;
    wire        w_blk_done          ;
    wire        r_blk_done_pulse    ;
    wire        w_blk_done_pulse    ;

    reg         reg_ready           ;
    wire        reg_ready_in        ;

    reg         reg_ready_d         ;
    wire        reg_ready_d_in      ;

    wire        trans_done_redge    ;
    wire        trans_done_pulse    ;
    reg         r_trans_busy;
    wire        r_trans_busy_in;
    reg         w_trans_busy;
    wire        w_trans_busy_in;

    wire [23:0] ahb_dmac_version        ;
    wire        ahb_dmac_p_ch_fifo_exist;
    wire        ahb_dmac_p_link_list_en ;
    wire        ahb_dmac_p_ahb_simple   ;
    wire        ahb_dmac_p_cdc_ms_en    ;
    wire        ahb_dmac_p_cdc_mp_en    ;

    reg  reg_w_strans_done          ;
    reg  reg_r_strans_done          ;
    reg  reg_w_trans_done           ;
    reg  reg_r_trans_done           ;
    reg  reg_w_trans_all_done       ;
    reg  reg_r_trans_all_done       ;

    wire reg_w_strans_done_in       ;
    wire reg_r_strans_done_in       ;
    wire reg_w_trans_done_in        ;
    wire reg_r_trans_done_in        ;
    wire reg_w_trans_all_done_in    ;
    wire reg_r_trans_all_done_in    ;

    wire reg_w_strans_done_clr      ;
    wire reg_r_strans_done_clr      ;
    wire reg_w_trans_done_clr       ;
    wire reg_r_trans_done_clr       ;
    wire reg_w_trans_all_done_clr   ;
    wire reg_r_trans_all_done_clr   ;

generate
    if(p_CH_FIFO_EXIST) begin:version_fifo_exist
         assign ahb_dmac_p_ch_fifo_exist=1;
    end
    else begin:version_fifo_none
         assign ahb_dmac_p_ch_fifo_exist=0;
    end
endgenerate

generate
    if(p_LINK_LIST_EN) begin:version_link_list_en
         assign ahb_dmac_p_link_list_en=1;
    end
    else begin:version_link_list_disable
         assign ahb_dmac_p_link_list_en=0;
    end
endgenerate

generate
    if(p_AHB_SIMPLE) begin:version_ahb_simple
         assign ahb_dmac_p_ahb_simple=1;
    end
    else begin:version_ahb_normal
         assign ahb_dmac_p_ahb_simple=0;
    end
endgenerate

generate
    if(p_CDC_MS_EN) begin:version_cdc_ms_en
         assign ahb_dmac_p_cdc_ms_en=1;
    end
    else begin:version_cdc_ms_none
         assign ahb_dmac_p_cdc_ms_en=0;
    end
endgenerate

generate
    if(p_CDC_MP_EN) begin:version_cdc_mp_en
         assign ahb_dmac_p_cdc_mp_en=1;
    end
    else begin:version_cdc_mp_none
         assign ahb_dmac_p_cdc_mp_en=0;
    end
endgenerate

    assign ahb_dmac_version=24'h010000|
                            {19'b0,ahb_dmac_p_ch_fifo_exist,
                             ahb_dmac_p_link_list_en,
                             ahb_dmac_p_ahb_simple,
                             ahb_dmac_p_cdc_ms_en,
                             ahb_dmac_p_cdc_mp_en};

    assign ch_star_hit    =(i_reg_addr==REG_CH_STAR   );
    assign ch_ctl_hit     =(i_reg_addr==REG_CH_CTL    );
    assign ch_rctl_hit    =(i_reg_addr==REG_CH_RCTL   );
    assign ch_wctl_hit    =(i_reg_addr==REG_CH_WCTL   );
    assign ch_saddr_hit   =(i_reg_addr==REG_CH_SADDR  );
    assign ch_daddr_hit   =(i_reg_addr==REG_CH_DADDR  );
    assign ch_rlen_hit    =(i_reg_addr==REG_CH_RLEN   );
    assign ch_wlen_hit    =(i_reg_addr==REG_CH_WLEN   );
    assign ch_llp_hit     =(i_reg_addr==REG_CH_LLP    );
    assign ch_blk_cnt_hit =(i_reg_addr==REG_CH_BLK_CNT);
    assign ch_int_en_hit  =(i_reg_addr==REG_CH_INTEN  );
    assign ch_int_sta_hit =(i_reg_addr==REG_CH_INTSTA );
    assign ch_int_clr_hit =(i_reg_addr==REG_CH_INTCLR );
    assign ch_bus_hit     =(i_reg_addr==REG_CH_BUS    );

    osr_redge_to_pulse #(
        .p_NUM(4)
    ) u_e2p1(
        .clk      (i_m_clk),
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
    assign o_w_finish = w_trans_done_addr&ch_w_en&((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en))) ;

    assign o_r_ack    = r_ack_pulse      ;
    assign o_r_finish = r_trans_done_addr&ch_r_en&((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en))) ;

    assign trans_done_redge= (~r_trans_busy)&(~w_trans_busy) ;

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

    assign r_trans_busy_in = (start_pulse&ch_r_en&(i_rlen != {p_ADDR_WIDTH{1'b0}}))                               ? 1'b1 :
                              i_r_dma_done_clr                                   ? 1'b0 :
                             (~ch_star)|
                             (r_trans_done_addr&i_r_strans_done_data&ch_r_en&
                             ((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en)))) ? 1'b0 : r_trans_busy ;

    assign w_trans_busy_in = (start_pulse&ch_w_en&(i_wlen != {p_ADDR_WIDTH{1'b0}}))                               ? 1'b1 :
                              i_w_dma_done_clr                                   ? 1'b0 :    
                             (~ch_star)|
                             (w_trans_done_addr&i_w_strans_done_data&ch_w_en&
                             ((~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en)))) ? 1'b0 : w_trans_busy ;

    assign reg_ready_in=lli_reg_rupd_en_pulse|(~ch_en)             ? 1'b0 :
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

    assign start_redge = ch_star;

    assign r_trig_req  = i_r_req&ch_r_trig_sel&(~r_ack)&(~r_strans_done_addr);
    assign r_fifo_rdy  = r_fifo_sel_num_count>=(r_strans_len_small[7:p_DATA_WIDTH/32+1]+{|r_strans_len_small[p_DATA_WIDTH/32:0]}+9'd0);

    assign r_trig_star = reg_ready_d& 
                         (r_trig_req|(~ch_r_trig_sel))&
                         r_fifo_rdy&(~i_suspend)&
                         ch_star&ch_r_en&
                         (~r_strans_busy);

    assign w_trig_req  = i_w_req&ch_w_trig_sel&(~w_ack)&(~w_strans_done_addr);
    assign w_fifo_rdy  = w_fifo_sel_num_count>=(w_strans_len_small[7:p_DATA_WIDTH/32+1]+{|w_strans_len_small[p_DATA_WIDTH/32:0]}+9'd0);

    assign w_trig_star = reg_ready_d&
                         (w_trig_req|(~ch_w_trig_sel))&
                         w_fifo_rdy&(~i_suspend)&
                         ch_star&ch_w_en&
                         (~w_strans_busy);

    osr_redge_to_pulse #(
        .p_NUM(4)
    ) u_e2p0(
        .clk      (i_m_clk),
        .rst_n    (i_m_rst_n),
        .in_edge  ({lli_reg_wupd_en,lli_reg_rupd_en,start_redge,trans_done_redge}),
        .out_pulse({lli_reg_wupd_en_pulse,lli_reg_rupd_en_pulse,start_pulse,trans_done_pulse})
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

    assign ch_star_in       =i_dma_start ? 1'b1 :
                             (trans_done_pulse)              ? 1'b0             : ch_star ;

    assign ch_en_in         =1'b1;

    assign ch_to_in         =i_ch_to;

    assign ch_r_fifo_clr    =(ch_ctl_hit   &i_reg_wr)&i_reg_wdata[8];
    assign ch_w_fifo_clr    =(ch_ctl_hit   &i_reg_wr)&i_reg_wdata[9];

    assign ch_lli_en_in     =i_lli_en;
    assign ch_fifo_sel_in   =(ch_ctl_hit   &i_reg_wr&~ch_en) ? i_reg_wdata[11]:ch_fifo_sel ;

    assign ch_w_blk_cnt_in  =(start_pulse)      ? 16'b0              :
                             (w_blk_done_pulse) ? ch_w_blk_cnt+16'b1 : ch_w_blk_cnt ;

    assign ch_r_blk_cnt_in  =(start_pulse)      ? 16'b0              :
                             (r_blk_done_pulse) ? ch_r_blk_cnt+16'b1 : ch_r_blk_cnt ;

    assign ch_int_en_in     ={i_w_to_int_en,i_r_to_int_en,4'b0,
                              i_w_dma_int_en,i_r_dma_int_en};

    assign o_w_dma_done     = w_trans_done_addr&i_w_strans_done_data ;
    assign o_r_dma_done     = r_trans_done_addr&i_r_strans_done_data ;
    assign o_to             = w_to || r_to;

    assign ch_int_sta       ={w_to,r_to,reg_w_strans_done,reg_r_strans_done,reg_w_trans_done,reg_r_trans_done,reg_w_trans_all_done,reg_r_trans_all_done};

    assign ch_int_clr       ={i_w_to_clr,i_r_to_clr,4'b0,
                              i_w_dma_done_clr,i_r_dma_done_clr};

    assign ch_bus_in        =(ch_bus_hit     &i_reg_wr&~ch_en) ? i_reg_wdata[7:0] :ch_bus          ;

    assign ch_r_max_burst_in=i_rd_max_byte;

    assign ch_r_en_in       =1'b1;

    assign ch_r_ed_revr_in  =i_rd_endian;

    assign ch_r_trig_sel_in =(ch_rctl_hit    &i_reg_wr&~ch_en)           ? i_reg_wdata[10]   : ch_r_trig_sel  ;

    assign ch_r_atyp_in     =1'b0;

    assign ch_w_max_burst_in=i_wr_max_byte;

    assign ch_w_en_in       =1'b1;

    assign ch_w_ed_revr_in  =i_wr_endian;

    assign ch_w_trig_sel_in =(ch_wctl_hit    &i_reg_wr&~ch_en)           ? i_reg_wdata[10]   : ch_w_trig_sel  ;

    assign ch_w_atyp_in     =1'b0;

generate
    if(p_DATA_WIDTH==32) begin:reg_bank_32b
      if(p_LINK_LIST_EN) begin:reg_bank_lli_32b
        assign ch_saddr_hit_lli= (lli_reg_sel==1);
        assign ch_daddr_hit_lli= (lli_reg_sel==2);
        assign ch_rlen_hit_lli = (lli_reg_sel==3);
        assign ch_wlen_hit_lli = (lli_reg_sel==4);
        assign ch_llp_hit_lli  = (lli_reg_sel==5);

        assign ch_last_lli_in =(start_pulse)                               ? 1'b0                            :
                                 (lli_reg_upd_en&ch_wlen_hit_lli &i_r_wr_en) ? i_r_data[31]                  : ch_last_lli ;

        assign ch_w_lli_wb_in   =(ch_wctl_hit    &i_reg_wr&~ch_en)           ? i_reg_wdata[12]               : ch_w_lli_wb ;

        assign ch_saddr_max_in  =i_dma_start ? i_saddr + i_rlen - 1 : ch_saddr_max;

        assign ch_saddr_in      =i_dma_start           ? i_saddr :
                                 (lli_reg_upd_en&ch_saddr_hit_lli&i_r_wr_en) ? i_r_data[p_ADDR_WIDTH-1:0]    :
                                 (ch_r_atyp)&(~lli_reg_upd_en)               ? ch_saddr                      : 
                                 (r_strans_busy)                             ? i_r_base_addr_ns              : ch_saddr ;

        assign ch_daddr_in      =i_dma_start           ? i_daddr :
                                 (lli_reg_wupd_en_pulse)                              ? ch_llp                        :
                                 (lli_reg_upd_en&ch_daddr_hit_lli&i_r_wr_en)          ? i_r_data[p_ADDR_WIDTH-1:0]    :
                                 (ch_w_atyp)&(~lli_reg_upd_en)                        ? ch_daddr                      : 
                                 (w_strans_busy|lli_reg_wupd_en)&(~lli_reg_wupd_done) ? i_w_base_addr_ns              : ch_daddr ; 

        assign ch_rlen_in       =i_dma_start           ? i_rlen : 
                                 (lli_reg_upd_en&ch_rlen_hit_lli &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH-1:0]    :
                                 (r_strans_busy)                             ? i_r_trans_len_ns              : ch_rlen ; 

        assign ch_wlen_in       =i_dma_start           ? i_wlen : 
                                 (lli_reg_upd_en&ch_wlen_hit_lli &i_r_wr_en)&
                                 (p_ADDR_WIDTH<32)                           ? i_r_data[p_ADDR_WIDTH-1:0]        :
                                 (lli_reg_upd_en&ch_wlen_hit_lli &i_r_wr_en)&
                                 (p_ADDR_WIDTH==32)                          ? {1'b0,i_r_data[p_ADDR_WIDTH-2:0]} :
                                 (w_strans_busy)                             ? i_w_trans_len_ns                  : ch_wlen        ; 

        assign ch_llp_in        =i_dma_start           ? i_llp : 
                                 (lli_reg_rupd_en&ch_llp_hit_lli &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH-1:0]        : 
                                 (lli_reg_rupd_en)                           ? i_r_base_addr_ns                  : ch_llp          ;
      end
      else begin:reg_bank_no_lli_32b
        assign ch_saddr_hit_lli= 0;
        assign ch_daddr_hit_lli= 0;
        assign ch_rlen_hit_lli = 0;
        assign ch_wlen_hit_lli = 0;
        assign ch_llp_hit_lli  = 0;           

        assign ch_last_lli_in =1'b0;

        assign ch_w_lli_wb_in   =1'b0;

        assign ch_saddr_max_in  =i_dma_start ? i_saddr + i_rlen - 1 : ch_saddr_max;

        assign ch_saddr_in      =i_dma_start                                 ? i_saddr[p_ADDR_WIDTH-1:0] : 
                                 (ch_r_atyp)                                 ? ch_saddr                      : 
                                 (r_strans_busy)                             ? i_r_base_addr_ns              : ch_saddr       ;

        assign ch_daddr_in      =i_dma_start                                 ? i_daddr[p_ADDR_WIDTH-1:0] :
                                 (ch_w_atyp)                                 ? ch_daddr                      : 
                                 (w_strans_busy)                             ? i_w_base_addr_ns              : ch_daddr       ; 

        assign ch_rlen_in       =i_dma_start                                 ? i_rlen[p_ADDR_WIDTH-1:0] :
                                 i_suspend                                   ? {p_ADDR_WIDTH{1'b0}}          :
                                 (r_strans_busy)                             ? i_r_trans_len_ns              : ch_rlen        ; 

        assign ch_wlen_in       =i_dma_start                                 ? i_wlen[p_ADDR_WIDTH-1:0] :
                                 i_suspend                                   ? {p_ADDR_WIDTH{1'b0}}          :
                                 (w_strans_busy)                             ? i_w_trans_len_ns              : ch_wlen        ; 

        assign ch_llp_in        ={p_ADDR_WIDTH{1'b0}}; 
      end
    end
    else begin:reg_bank_64b
      if(p_LINK_LIST_EN) begin:reg_bank_lli_64b
        assign ch_saddr_hit_lli= (lli_reg_sel==0);
        assign ch_daddr_hit_lli= (lli_reg_sel==1);
        assign ch_rlen_hit_lli = (lli_reg_sel==1);
        assign ch_wlen_hit_lli = (lli_reg_sel==2);
        assign ch_llp_hit_lli  = (lli_reg_sel==2);

        assign ch_last_lli_in =(start_pulse)                               ? 1'b0                           :
                               (lli_reg_upd_en&ch_wlen_hit_lli &i_r_wr_en) ? i_r_data[31]                   : ch_last_lli ;

        assign ch_w_lli_wb_in =(ch_wctl_hit    &i_reg_wr&~ch_en)           ? i_reg_wdata[12]                : ch_w_lli_wb ;

        assign ch_saddr_max_in  =i_dma_start ? i_saddr + i_rlen - 1 : ch_saddr_max;

        assign ch_saddr_in    =i_dma_start                                 ? i_saddr[p_ADDR_WIDTH-1:0]  :
                               (lli_reg_upd_en&ch_saddr_hit_lli&i_r_wr_en) ? i_r_data[p_ADDR_WIDTH+31:32]   :
                               (ch_r_atyp)&(~lli_reg_upd_en)               ? ch_saddr                       : 
                               (r_strans_busy)                             ? i_r_base_addr_ns               : ch_saddr ;       

        assign ch_daddr_in    =i_dma_start                                          ? i_daddr[p_ADDR_WIDTH-1:0]      :
                               (lli_reg_wupd_en_pulse)                              ? ch_llp                         :
                               (lli_reg_upd_en&ch_daddr_hit_lli&i_r_wr_en)          ? i_r_data[p_ADDR_WIDTH-1:0]     :
                               (ch_w_atyp)&(~lli_reg_upd_en)                        ? ch_daddr                       : 
                               (w_strans_busy|lli_reg_wupd_en)&(~lli_reg_wupd_done) ? i_w_base_addr_ns               : ch_daddr ;

        assign ch_rlen_in     =i_dma_start                                 ? i_rlen[p_ADDR_WIDTH-1:0]   :
                               (lli_reg_upd_en&ch_rlen_hit_lli &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH+31:32]   :
                               (r_strans_busy)                             ? i_r_trans_len_ns               : ch_rlen ; 

        assign ch_wlen_in     =i_dma_start                                 ? i_wlen[p_ADDR_WIDTH-1:0] : 
                               (lli_reg_upd_en&ch_wlen_hit_lli &i_r_wr_en)&
                               (p_ADDR_WIDTH<32)                           ? i_r_data[p_ADDR_WIDTH-1:0]     :
                               (lli_reg_upd_en&ch_wlen_hit_lli &i_r_wr_en)&
                               (p_ADDR_WIDTH==32)                          ? {1'b0,i_r_data[p_ADDR_WIDTH-2:0]} :
                               (w_strans_busy)                             ? i_w_trans_len_ns                  : ch_wlen ; 

        assign ch_llp_in      =i_dma_start                                 ? i_llp[p_ADDR_WIDTH-1:0] :
                               (lli_reg_rupd_en&ch_llp_hit_lli &i_r_wr_en) ? i_r_data[p_ADDR_WIDTH+31:32]      : 
                               (lli_reg_rupd_en)                           ? i_r_base_addr_ns                  : ch_llp ;
      end
      else begin:reg_bank_no_lli_64b
        assign ch_saddr_hit_lli= 0;
        assign ch_daddr_hit_lli= 0;
        assign ch_rlen_hit_lli = 0;
        assign ch_wlen_hit_lli = 0;
        assign ch_llp_hit_lli  = 0;          

        assign ch_last_lli_in =1'b0;

        assign ch_w_lli_wb_in   =1'b0;

        assign ch_saddr_max_in  =i_dma_start ? i_saddr + i_rlen - 1 : ch_saddr_max;

        assign ch_saddr_in      =i_dma_start                                 ? i_saddr[p_ADDR_WIDTH-1:0]     :
                                 (ch_r_atyp)                                 ? ch_saddr                      : 
                                 (r_strans_busy)                             ? i_r_base_addr_ns              : ch_saddr       ;       

        assign ch_daddr_in      =i_dma_start                                 ? i_daddr[p_ADDR_WIDTH-1:0]     :
                                 (ch_w_atyp)                                 ? ch_daddr                      : 
                                 (w_strans_busy)                             ? i_w_base_addr_ns              : ch_daddr       ; 

        assign ch_rlen_in       =i_dma_start                                 ? i_rlen[p_ADDR_WIDTH-1:0]      :
                                 i_suspend                                   ? {p_ADDR_WIDTH{1'b0}}          :
                                 (r_strans_busy)                             ? i_r_trans_len_ns              : ch_rlen        ; 

        assign ch_wlen_in       =i_dma_start                                 ? i_wlen[p_ADDR_WIDTH-1:0]      :
                                 i_suspend                                   ? {p_ADDR_WIDTH{1'b0}}          :
                                 (w_strans_busy)                             ? i_w_trans_len_ns              : ch_wlen        ;
        assign ch_llp_in        ={p_ADDR_WIDTH{1'b0}}; 
      end
    end
endgenerate

    assign w_strans_len_in       = (w_trig_star)&(w_strans_len==8'b0)&(ch_wlen!=0)&(~i_suspend) ? w_strans_len_small  : i_w_strans_len_ns ;

    assign w_strans_len_small    = (ch_wlen<{{(p_ADDR_WIDTH-8){1'b0}},ch_w_max_burst}) ? ch_wlen[7:0] : ch_w_max_burst ;

    assign r_strans_len_burst_in = (r_trig_star)&(r_strans_len_burst==0)&(ch_rlen!=0)&(~i_suspend) ? {1'b0,r_strans_len_small[7:p_DATA_WIDTH/32+1]}+
                                                                                        (|ch_saddr[p_DATA_WIDTH/32:0])+
                                                                                        (|r_strans_len_small[p_DATA_WIDTH/32:0]) :
                                                                                        i_r_strans_len_burst_ns ;

    assign r_strans_len_small    = (ch_rlen<{{(p_ADDR_WIDTH-8){1'b0}},ch_r_max_burst}) ? ch_rlen[7:0] : ch_r_max_burst ;

    assign o_r_strans_len        = r_strans_len_small ;
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

        assign w_fifo_sel_data          =(~ch_fifo_sel) ? i_w_fifo_data                     : w_ch_fifo_data            ;

        assign w_fifo_sel_rd_enable     =(lli_reg_upd_en)? 1'b0                             : i_w_rd_en;                                           
        assign o_w_fifo_rd_enable       =(~ch_fifo_sel) ? w_fifo_sel_rd_enable              : 1'b0                      ;
        assign w_ch_fifo_rd_enable      =(~ch_fifo_sel) ? 1'b0                              : w_fifo_sel_rd_enable      ;

        assign w_fifo_sel_empty         =(~ch_fifo_sel) ? i_w_fifo_empty                    : w_ch_fifo_empty           ; 
        assign w_fifo_sel_num_count     =(~ch_fifo_sel) ? {{(8-p_W_FIFO_DEPTH_WIDTH){1'b0}},i_w_fifo_num_count} :
                                                          {{(8-p_CH_FIFO_DEPTH_WIDTH){1'b0}},wr_ch_fifo_num_count};

        assign w_fifo_sel_clear         =ch_w_fifo_clr;                                                
        assign o_w_fifo_clear           =(~ch_fifo_sel) ? w_fifo_sel_clear                  : 1'b0                      ;
        assign wr_ch_fifo_clear         =(~ch_fifo_sel) ? 1'b0                              : w_fifo_sel_clear|r_fifo_sel_clear;

        assign r_fifo_sel_data          =i_r_data ;                                            
        assign o_r_fifo_data            =(~ch_fifo_sel) ? r_fifo_sel_data                   : 1'b0                      ;
        assign r_ch_fifo_data           =(~ch_fifo_sel) ? 1'b0                              : r_fifo_sel_data           ;

        assign r_fifo_sel_wr_enable     =(lli_reg_upd_en)? 1'b0                             : i_r_wr_en;                                           
        assign o_r_fifo_wr_enable       =(~ch_fifo_sel)  ? r_fifo_sel_wr_enable              : 1'b0                      ;
        assign r_ch_fifo_wr_enable      =(~ch_fifo_sel)  ? 1'b0                              : r_fifo_sel_wr_enable      ;

        assign r_fifo_sel_full          =(~ch_fifo_sel)  ? i_r_fifo_full                     : r_ch_fifo_full            ; 
        assign r_fifo_sel_num_count     =(~ch_fifo_sel)  ? {{(8-p_R_FIFO_DEPTH_WIDTH){1'b0}},i_r_fifo_num_count} :
                                                           {1'b1,{(p_CH_FIFO_DEPTH_WIDTH){1'b0}}}-{{(8-p_CH_FIFO_DEPTH_WIDTH){1'b0}},wr_ch_fifo_num_count}; 

        assign r_fifo_sel_clear         =ch_r_fifo_clr;                                                
        assign o_r_fifo_clear           =(~ch_fifo_sel) ? r_fifo_sel_clear                  : 1'b0                      ;
    end
    else begin:ch_fifo_none

        assign w_fifo_sel_data          = i_w_fifo_data ;

        assign w_fifo_sel_rd_enable     =(lli_reg_upd_en)? 1'b0 : i_w_rd_en;                                           
        assign o_w_fifo_rd_enable       = w_fifo_sel_rd_enable;

        assign w_fifo_sel_empty         = i_w_fifo_empty ; 
        assign w_fifo_sel_num_count     ={{(8-p_W_FIFO_DEPTH_WIDTH){1'b0}} ,i_w_fifo_num_count};

        assign w_fifo_sel_clear         = ch_w_fifo_clr;                                                
        assign o_w_fifo_clear           = w_fifo_sel_clear ;

        assign r_fifo_sel_data          = i_r_data;                                            
        assign o_r_fifo_data            = r_fifo_sel_data ;

        assign r_fifo_sel_wr_enable     =(lli_reg_upd_en) ? 1'b0 : i_r_wr_en;                                           
        assign o_r_fifo_wr_enable       = r_fifo_sel_wr_enable;

        assign r_fifo_sel_full          = i_r_fifo_full; 
        assign r_fifo_sel_num_count     = {{(8-p_R_FIFO_DEPTH_WIDTH){1'b0}} ,i_r_fifo_num_count}; 

        assign r_fifo_sel_clear         =ch_r_fifo_clr;                                                
        assign o_r_fifo_clear           =r_fifo_sel_clear;

    end
endgenerate

    assign o_w_base_addr      = ch_daddr;
    assign o_w_trans_len      = (lli_reg_wupd_en) ? {{(p_ADDR_WIDTH-3){1'b0}},lli_ch_wlen}               : ch_wlen;
    assign o_w_strans_len     = (lli_reg_wupd_en) ? {5'b0,lli_w_strans_len}                              : w_strans_len;
    assign o_w_data           = (lli_reg_wupd_en) ? {{(p_DATA_WIDTH-32){1'b0}},16'h3,lli_trans_done_cnt} : w_fifo_sel_data ;

    assign o_r_base_addr       = (lli_reg_rupd_en) ? ch_llp                                 : ch_saddr ;
    assign o_r_trans_len       = (lli_reg_rupd_en) ? {{(p_ADDR_WIDTH-6){1'b0}},lli_ch_rlen} : ch_rlen  ;
    assign o_r_strans_len_burst= (lli_reg_rupd_en) ? {3'b0,lli_r_strans_len_burst}          : r_strans_len_burst ;
    assign o_r_ch_saddr_max    = ch_saddr_max;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            ch_star         <=1'b0    ;
            ch_en           <=1'b0    ;

            ch_to           <= 8'b0   ;
            ch_lli_en       <= 1'b0   ;
            ch_fifo_sel     <= 1'b0   ;           

            ch_r_max_burst  <=8'b0    ;
            ch_r_en         <=1'b0    ;
            ch_r_ed_revr    <=1'b0    ;
            ch_r_trig_sel   <=1'b0    ;
            ch_r_atyp       <=1'b0    ;
            ch_last_lli     <=1'b0    ;

            ch_w_max_burst  <=8'b0    ;
            ch_w_en         <=1'b0    ;
            ch_w_ed_revr    <=1'b0    ;
            ch_w_trig_sel   <=1'b0    ;
            ch_w_atyp       <=1'b0    ;
            ch_w_lli_wb     <=1'b0    ;

            ch_saddr_max    <={p_ADDR_WIDTH{1'b0}}   ;

            ch_saddr        <={p_ADDR_WIDTH{1'b0}}   ;

            ch_daddr        <={p_ADDR_WIDTH{1'b0}}   ;

            ch_rlen         <={p_ADDR_WIDTH{1'b0}}   ;

            ch_wlen         <={p_ADDR_WIDTH{1'b0}}   ;

            ch_llp          <={p_ADDR_WIDTH{1'b0}}   ;

            ch_w_blk_cnt    <=16'b0   ;

            ch_r_blk_cnt    <=16'b0   ;

            ch_int_en       <= 8'b0   ;

            ch_bus          <= 8'b0   ;
        end
        else begin
            ch_star         <=  ch_star_in          ;

            ch_en           <=  ch_en_in            ;

            ch_r_max_burst  <=  ch_r_max_burst_in   ;
            ch_r_en         <=  ch_r_en_in          ;
            ch_r_ed_revr    <=  ch_r_ed_revr_in     ;
            ch_r_trig_sel   <=  ch_r_trig_sel_in    ;
            ch_r_atyp       <=  ch_r_atyp_in        ;
            ch_last_lli   <=  ch_last_lli_in    ;

            ch_w_max_burst  <=  ch_w_max_burst_in   ;
            ch_w_en         <=  ch_w_en_in          ;
            ch_w_ed_revr    <=  ch_w_ed_revr_in     ;
            ch_w_trig_sel   <=  ch_w_trig_sel_in    ;
            ch_w_atyp       <=  ch_w_atyp_in        ;
            ch_w_lli_wb     <=  ch_w_lli_wb_in      ;

            ch_saddr_max    <=  ch_saddr_max_in     ;

            ch_saddr        <=  ch_saddr_in         ;

            ch_daddr        <=  ch_daddr_in         ;

            ch_rlen         <=  ch_rlen_in          ;

            ch_wlen         <=  ch_wlen_in          ;

            ch_llp          <=  ch_llp_in           ;

            ch_w_blk_cnt    <=  ch_w_blk_cnt_in     ;            

            ch_r_blk_cnt    <=  ch_r_blk_cnt_in     ;

            ch_to           <=  ch_to_in            ;
            ch_lli_en       <=  ch_lli_en_in        ;
            ch_fifo_sel     <=  ch_fifo_sel_in      ;

            ch_int_en       <=  ch_int_en_in        ;

            ch_bus          <=  ch_bus_in           ;
        end
    end

    osr_cmux_16 #(64) rd_reg_mux0(
        .o_d(reg_rdata),
        .i_d0 ({32'b0,ahb_dmac_version,6'b0,ch_en,ch_star}                  ),.i_s0 (ch_star_hit    ),
        .i_d1 ({52'b0,ch_fifo_sel,ch_lli_en,2'b0,ch_to}                     ),.i_s1 (ch_ctl_hit     ),
        .i_d2 ({48'b0,
                4'b0,ch_r_atyp,ch_r_trig_sel,ch_r_ed_revr,ch_r_en,
                ch_r_max_burst}                                             ),.i_s2 (ch_rctl_hit    ),
        .i_d3 ({48'b0,
                3'b0,ch_w_lli_wb,ch_w_atyp,ch_w_trig_sel,ch_w_ed_revr,ch_w_en,
                ch_w_max_burst}                                             ),.i_s3 (ch_wctl_hit    ),
        .i_d4 ({{(64-p_ADDR_WIDTH){1'b0}},ch_saddr}                         ),.i_s4 (ch_saddr_hit   ),
        .i_d5 ({{(64-p_ADDR_WIDTH){1'b0}},ch_daddr}                         ),.i_s5 (ch_daddr_hit   ),
        .i_d6 ({{(64-p_ADDR_WIDTH){1'b0}},ch_rlen }                         ),.i_s6 (ch_rlen_hit    ),
        .i_d7 ({{(64-p_ADDR_WIDTH){1'b0}},ch_wlen }                         ),.i_s7 (ch_wlen_hit    ),
        .i_d8 ({{(64-p_ADDR_WIDTH){1'b0}},ch_llp  }                         ),.i_s8 (ch_llp_hit     ),
        .i_d9 ({32'b0,ch_w_blk_cnt,ch_r_blk_cnt}                            ),.i_s9 (ch_blk_cnt_hit ),
        .i_d10({56'b0,ch_int_en }                                           ),.i_s10(ch_int_en_hit  ),
        .i_d11({56'b0,ch_int_sta}                                           ),.i_s11(ch_int_sta_hit ),
        .i_d12(64'b0                                                        ),.i_s12(ch_int_clr_hit ),
        .i_d13({56'b0,ch_bus}                                               ),.i_s13(ch_bus_hit     ),
        .i_d14(64'b0                                                        ),.i_s14(1'b0           ),
        .i_d15(64'b0                                                        )

    ); 

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            o_reg_rdata <= 64'b0;
        end
        else begin
            o_reg_rdata <=  reg_rdata;
        end
    end

generate
if(p_LINK_LIST_EN) begin:ch_lli_logic_exist

    assign lli_en= (ch_lli_en);
end
else begin:ch_lli_logic_none
    assign lli_en= 1'b0;
end
endgenerate

    assign lli_reg_sel_in     = (lli_reg_rupd_en_pulse)                                  ? 3'h0             :
                                (lli_reg_rupd_en&i_r_wr_en)                              ? lli_reg_sel+3'b1 : lli_reg_sel ;

    assign lli_reg_rupd_en_in        = ((start_pulse&(~ch_w_lli_wb))|
                                        (lli_reg_wupd_en&i_w_strans_done_data)|
                                        (((ch_rlen==0)&(ch_wlen==0)&(~r_strans_busy)&(~w_strans_busy))&((~ch_w_lli_wb)|(lli_reg_wupd_done))))&
                                       lli_en&(~ch_last_lli)&ch_star                     ? 1'b1 :
                                       (lli_reg_upd_done)|(~lli_en)                      ? 1'b0 : lli_reg_rupd_en ;

    assign lli_reg_wupd_en_in        = (start_pulse|
                                        ((ch_rlen==0)&(ch_wlen==0)&(~w_strans_busy)&(~r_strans_busy)))&
                                       lli_en&(~ch_last_lli)&ch_w_lli_wb&ch_star         ? 1'b1 :
                                       (lli_reg_upd_done)|(~lli_en)                      ? 1'b0 : lli_reg_wupd_en ;

    assign lli_reg_upd_en_in         = (lli_reg_rupd_en_in|lli_reg_wupd_en_in)&lli_en    ? 1'b1 :
                                        lli_reg_upd_done                                 ? 1'b0 : lli_reg_upd_en ;

    assign lli_reg_rupd_done_in      =  lli_reg_rupd_en&ch_llp_hit_lli&i_r_wr_en         ? 1'b1 :
                                        lli_reg_upd_done                                 ? 1'b0 : lli_reg_rupd_done ;

    assign lli_reg_wupd_done_in      =  lli_reg_wupd_en&i_w_trans_done_addr              ? 1'b1 :
                                        (lli_reg_upd_done)                               ? 1'b0 : lli_reg_wupd_done ;

    assign lli_reg_upd_done          =  lli_reg_rupd_done ;

    assign lli_ch_wlen_in            =  (lli_reg_wupd_en_pulse)                          ? 3'h4                   :
                                        (lli_reg_wupd_en)                                ? i_w_trans_len_ns[2:0]  : lli_ch_wlen ;

    assign lli_w_strans_len_in       = (lli_reg_wupd_en_pulse)                           ? 3'h4                   :
                                       (lli_reg_wupd_en)                                 ? i_w_strans_len_ns[2:0] : lli_w_strans_len ;

    assign lli_ch_rlen_in            = (lli_reg_rupd_en_pulse)                           ? 6'h18                  :
                                       (lli_reg_rupd_en)                                 ? i_r_trans_len_ns[5:0]  : lli_ch_rlen ;

    assign lli_r_strans_len_burst_in = (lli_reg_rupd_en_pulse)                           ? lli_ch_rlen_in[5:p_DATA_WIDTH/32+1]+(|ch_llp[p_DATA_WIDTH/32:0])+(|lli_ch_rlen_in[p_DATA_WIDTH/32:0]) :
                                       (lli_reg_rupd_en)                                 ? i_r_strans_len_burst_ns[4-p_DATA_WIDTH/32:0]      : lli_r_strans_len_burst ;

    assign lli_trans_done_cnt_in     = (start_pulse)      ? 16'b0                :
                                       (lli_reg_upd_done) ? lli_trans_done_cnt+1'b1 : lli_trans_done_cnt ;

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
            w_strans_len       <=8'b0   ;
            r_strans_len_burst <={(8-p_DATA_WIDTH/32){1'b0}};
        end
        else begin
            w_strans_len       <=  w_strans_len_in       ;
            r_strans_len_burst <=  r_strans_len_burst_in ;
        end
    end

    assign r_trans_done_clr  =((~r_strans_busy)&r_strans_busy_in)|((i_r_resp|r_req_pulse)&ch_r_trig_sel)|(start_pulse);
    assign w_trans_done_clr  =((~w_strans_busy)&w_strans_busy_in)|((i_w_resp|w_req_pulse)&ch_w_trig_sel)|(start_pulse);
    assign r_strans_done_clr =((~r_strans_busy)&r_strans_busy_in)|((i_r_resp|r_req_pulse)&ch_r_trig_sel)|(start_pulse);
    assign w_strans_done_clr =((~w_strans_busy)&w_strans_busy_in)|((i_w_resp|w_req_pulse)&ch_w_trig_sel)|(start_pulse);
    assign r_to_clr          =ch_int_clr[6];
    assign w_to_clr          =ch_int_clr[7];

    assign w_strans_done_addr_in = (lli_reg_upd_done|w_strans_done_clr)     ? 1'b0 :
                                   (i_w_strans_done_addr)&(~lli_reg_upd_en) ? 1'b1 : w_strans_done_addr ;

    assign w_strans_done_in     = (lli_reg_upd_done|w_strans_done_clr)      ? 1'b0  :
                                  (w_strans_done_addr&i_w_strans_done_data) ? 1'b1  : w_strans_done ;

    assign r_strans_done_addr_in= (lli_reg_upd_done|r_strans_done_clr)      ? 1'b0  :
                                  (i_r_strans_done_addr)&(~lli_reg_upd_en)  ? 1'b1  : r_strans_done_addr ;

    assign r_strans_done_in     = (lli_reg_upd_done|r_strans_done_clr)      ? 1'b0  :
                                  (r_strans_done_addr&i_r_strans_done_data) ? 1'b1  : r_strans_done ;

    assign w_trans_done_addr_in = (lli_reg_upd_done|w_trans_done_clr)       ? 1'b0 :
                                  (i_w_trans_done_addr&(~lli_reg_upd_en))   ? 1'b1 : w_trans_done_addr ;

    assign w_trans_done_in      = (lli_reg_upd_done|w_trans_done_clr)       ? 1'b0  :
                                  (w_trans_done_addr&i_w_strans_done_data)  ? 1'b1  : w_trans_done ;

    assign r_trans_done_addr_in =(lli_reg_upd_done|r_trans_done_clr)        ? 1'b0 :
                                 (i_r_trans_done_addr&(~lli_reg_upd_en))    ? 1'b1 : r_trans_done_addr ;

    assign r_trans_done_in      =(lli_reg_upd_done|r_trans_done_clr)        ? 1'b0 :
                                 (r_trans_done_addr&i_r_strans_done_data)   ? 1'b1 : r_trans_done ;

    assign w_to_in = (i_w_to)   ? 1'b1 :
                     (w_to_clr) ? 1'b0 : w_to ;

    assign r_to_in = (i_r_to)   ? 1'b1 :
                     (r_to_clr) ? 1'b0 : r_to ;
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

            w_to              <= 1'b0;
            r_to              <= 1'b0;
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

            w_to              <=   w_to_in;
            r_to              <=   r_to_in;
        end
    end

    assign reg_r_trans_all_done_clr =ch_int_clr[0];
    assign reg_w_trans_all_done_clr =ch_int_clr[1];
    assign reg_r_trans_done_clr     =ch_int_clr[2];
    assign reg_w_trans_done_clr     =ch_int_clr[3];
    assign reg_r_strans_done_clr    =ch_int_clr[4];
    assign reg_w_strans_done_clr    =ch_int_clr[5];

    assign reg_w_strans_done_in    = (reg_w_strans_done_clr)                  ? 1'b0 :
                                     (w_strans_done_addr&i_w_strans_done_data)? 1'b1 : reg_w_strans_done ;

    assign reg_r_strans_done_in    = (reg_r_strans_done_clr)                  ? 1'b0 :
                                     (r_strans_done_addr&i_r_strans_done_data)? 1'b1 : reg_r_strans_done ;

    assign reg_w_trans_done_in     = (reg_w_trans_done_clr)                   ? 1'b0 :
                                     (w_trans_done_addr&i_w_strans_done_data) ? 1'b1 : reg_w_trans_done ;

    assign reg_r_trans_done_in     = (reg_r_trans_done_clr)                   ? 1'b0 :
                                     (r_trans_done_addr&i_r_strans_done_data) ? 1'b1 : reg_r_trans_done ;

    assign reg_w_trans_all_done_in = (reg_w_trans_all_done_clr)                               ? 1'b0 :
                                     w_trans_done_addr&i_w_strans_done_data&ch_w_en&
                                     (i_suspend|(~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en)))  ? 1'b1 : reg_w_trans_all_done ;

    assign reg_r_trans_all_done_in = (reg_r_trans_all_done_clr)                               ? 1'b0 :
                                     r_trans_done_addr&i_r_strans_done_data&ch_r_en&
                                     (i_suspend|(~lli_en)|(lli_en&ch_last_lli&(~lli_reg_upd_en)))  ? 1'b1 : reg_r_trans_all_done ;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            reg_w_strans_done    <= 1'b0;
            reg_r_strans_done    <= 1'b0;
            reg_w_trans_done     <= 1'b0; 
            reg_r_trans_done     <= 1'b0;
            reg_w_trans_all_done <= 1'b0; 
            reg_r_trans_all_done <= 1'b0;
        end
        else begin
            reg_w_strans_done    <=   reg_w_strans_done_in ; 
            reg_r_strans_done    <=   reg_r_strans_done_in ;  
            reg_w_trans_done     <=   reg_w_trans_done_in  ;  
            reg_r_trans_done     <=   reg_r_trans_done_in  ;  
            reg_w_trans_all_done <=   reg_w_trans_all_done_in  ;  
            reg_r_trans_all_done <=   reg_r_trans_all_done_in  ;
        end
    end

    assign r_blk_done=(lli_en) ? r_trans_done_addr : r_strans_done_addr ;
    assign w_blk_done=(lli_en) ? w_trans_done_addr : w_strans_done_addr ;

    osr_redge_to_pulse #(
        .p_NUM(2)
    ) u_e2p2(
        .clk      (i_m_clk),
        .rst_n    (i_m_rst_n),
        .in_edge  ({w_blk_done,r_blk_done}),
        .out_pulse({w_blk_done_pulse,r_blk_done_pulse})
    );

    assign o_r_revr_en =ch_r_ed_revr;
    assign o_w_revr_en =ch_w_ed_revr;

    assign o_ch_r_atyp =(lli_reg_upd_en) ? 1'b0 : ch_r_atyp ;            
    assign o_ch_w_atyp =(lli_reg_upd_en) ? 1'b0 : ch_w_atyp ; 

    assign o_ch_to     = ch_to ;
    assign o_ch_bus    = ch_bus;

    assign o_ch_int    =|(ch_int_en&ch_int_sta);    
endmodule

module osr_cmux_16 # (
    parameter p_DWIDTH = 8
)(
output  wire [p_DWIDTH-1 : 0]     o_d,
input   wire [p_DWIDTH-1 : 0]     i_d0,
input   wire                      i_s0,
input   wire [p_DWIDTH-1 : 0]     i_d1,
input   wire                      i_s1,
input   wire [p_DWIDTH-1 : 0]     i_d2,
input   wire                      i_s2,
input   wire [p_DWIDTH-1 : 0]     i_d3,
input   wire                      i_s3,
input   wire [p_DWIDTH-1 : 0]     i_d4,
input   wire                      i_s4,
input   wire [p_DWIDTH-1 : 0]     i_d5,
input   wire                      i_s5,
input   wire [p_DWIDTH-1 : 0]     i_d6,
input   wire                      i_s6,
input   wire [p_DWIDTH-1 : 0]     i_d7,
input   wire                      i_s7,
input   wire [p_DWIDTH-1 : 0]     i_d8,
input   wire                      i_s8,
input   wire [p_DWIDTH-1 : 0]     i_d9,
input   wire                      i_s9,
input   wire [p_DWIDTH-1 : 0]     i_d10,
input   wire                      i_s10,
input   wire [p_DWIDTH-1 : 0]     i_d11,
input   wire                      i_s11,
input   wire [p_DWIDTH-1 : 0]     i_d12,
input   wire                      i_s12,
input   wire [p_DWIDTH-1 : 0]     i_d13,
input   wire                      i_s13,
input   wire [p_DWIDTH-1 : 0]     i_d14,
input   wire                      i_s14,
input   wire [p_DWIDTH-1 : 0]     i_d15
);
reg  [p_DWIDTH-1 : 0]   d_mux;
always @(*) begin
    d_mux = i_d15;
    case(1'b1) //synopsys parallel_case
        i_s0 : d_mux = i_d0;
        i_s1 : d_mux = i_d1;
        i_s2 : d_mux = i_d2;
        i_s3 : d_mux = i_d3;
        i_s4 : d_mux = i_d4;
        i_s5 : d_mux = i_d5;
        i_s6 : d_mux = i_d6;
        i_s7 : d_mux = i_d7;
        i_s8 : d_mux = i_d8;
        i_s9 : d_mux = i_d9;
        i_s10: d_mux = i_d10;
        i_s11: d_mux = i_d11;
        i_s12: d_mux = i_d12;
        i_s13: d_mux = i_d13;
        i_s14: d_mux = i_d14;
    endcase
end

assign o_d = d_mux;

endmodule
