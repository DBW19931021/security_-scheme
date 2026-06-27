//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_dmac_top #(
parameter p_DATA_WIDTH         =32,
parameter p_ADDR_WIDTH         =32,
parameter p_CH_FIFO_EXIST      =0,
parameter p_LINK_LIST_EN       =1,
parameter p_AHB_SIMPLE         =0,
parameter p_CDC_MS_EN          =1,
parameter p_CDC_MP_EN          =1,
parameter p_R_FIFO_DEPTH_WIDTH =7,
parameter p_W_FIFO_DEPTH_WIDTH =3,
parameter p_CH_FIFO_DEPTH_WIDTH=7
)
(

    input   wire                          i_s_clk           ,
    input   wire                          i_s_rst_n         ,

    input   wire                          i_m_clk           ,
    input   wire                          i_m_rst_n         ,

    input   wire                          i_p_clk           ,
    input   wire                          i_p_rst_n         ,

    input   wire [p_DATA_WIDTH-1:0]       i_w_fifo_data     ,
    output  wire                          o_w_fifo_rd_enable,

    input   wire                          i_w_fifo_empty    ,
    input   wire [p_W_FIFO_DEPTH_WIDTH:0] i_w_fifo_num_count,

    output  wire                          o_w_fifo_clear    ,

    output  wire [p_DATA_WIDTH-1:0]       o_r_fifo_data     ,
    output  wire                          o_r_fifo_wr_enable,

    input   wire                          i_r_fifo_full     ,
    input   wire [p_R_FIFO_DEPTH_WIDTH:0] i_r_fifo_num_count,

    output  wire                          o_r_fifo_clear    ,

    input   wire                          i_w_req           ,
    output  wire                          o_w_ack           ,
    output  wire                          o_w_finish        ,

    input   wire                          i_r_req           ,
    output  wire                          o_r_ack           ,
    output  wire                          o_r_finish        ,

    input   wire                          i_reg_wr          ,
    input   wire [4:0]                    i_reg_addr        ,
    input   wire [31:0]                   i_reg_wdata       ,
    output  wire [63:0]                   o_reg_rdata       ,

    input   wire                          i_dma_start       ,
    input   wire                          i_suspend         ,
    input   wire                          i_lli_en          ,
    input   wire [7:0]                    i_ch_to           ,
    input   wire                          i_rd_endian       ,
    input   wire [7:0]                    i_rd_max_byte     ,
    input   wire                          i_wr_endian       ,
    input   wire [7:0]                    i_wr_max_byte     ,
    input   wire [p_ADDR_WIDTH-1:0]       i_saddr           ,
    input   wire [p_ADDR_WIDTH-1:0]       i_daddr           ,
    input   wire [p_ADDR_WIDTH-1:0]       i_rlen            ,
    input   wire [p_ADDR_WIDTH-1:0]       i_wlen            ,
    input   wire [p_ADDR_WIDTH-1:0]       i_llp             ,
    input   wire                          i_w_to_int_en     ,
    input   wire                          i_r_to_int_en     ,
    input   wire                          i_w_dma_int_en    ,
    input   wire                          i_r_dma_int_en    ,
    output  wire                          o_w_dma_done      ,
    output  wire                          o_w_dma_done_lvl  ,
    output  wire                          o_r_dma_done      ,
    output  wire                          o_r_dma_done_lvl  ,
    output  wire                          o_suspend_done    ,
    input   wire                          i_w_to_clr        ,
    input   wire                          i_r_to_clr        ,
    output  wire [ 1:0]                   o_to              ,
    input   wire                          i_w_dma_done_clr  ,
    input   wire                          i_r_dma_done_clr  ,

    output  wire [p_ADDR_WIDTH-1:0]       o_w_haddr         ,
    output  wire [ 2:0]                   o_w_hburst        ,
    output  wire [ 1:0]                   o_w_hsize         ,
    output  wire [ 1:0]                   o_w_htrans        ,
    output  wire [p_DATA_WIDTH-1:0]       o_w_hwdata        ,
    output  wire [ 3:0]                   o_w_hprot         ,
    output  wire                          o_w_hwrite        ,
    input   wire [p_DATA_WIDTH-1:0]       i_w_hrdata        ,
    input   wire                          i_w_hready        ,

    output  wire [p_ADDR_WIDTH-1:0]       o_r_haddr         ,
    output  wire [ 2:0]                   o_r_hburst        ,
    output  wire [ 1:0]                   o_r_hsize         ,
    output  wire [ 1:0]                   o_r_htrans        ,
    output  wire [p_DATA_WIDTH-1:0]       o_r_hwdata        ,
    output  wire [3:0]                    o_r_hprot         ,
    output  wire                          o_r_hwrite        ,
    input   wire [p_DATA_WIDTH-1:0]       i_r_hrdata        ,
    input   wire                          i_r_hready        ,

    output  wire                          o_int
);

    wire [ 7:0]                           s_ch_int_sta_pulse      ; 

    wire                                  s_r_blk_done            ; 
    wire                                  s_w_blk_done            ; 

    wire                                  s_ch_star               ;
    wire                                  s_ch_en                 ;

    wire [ 7:0]                           s_ch_to                 ;
    wire                                  s_ch_r_fifo_clr         ;
    wire                                  s_ch_w_fifo_clr         ;
    wire                                  s_ch_lli_en             ;
    wire                                  s_ch_fifo_sel           ;

    wire [ 7:0]                           s_ch_r_max_burst        ; 
    wire                                  s_ch_r_en               ; 
    wire                                  s_ch_r_ed_revr          ; 
    wire                                  s_ch_r_trig_sel         ; 
    wire                                  s_ch_r_atyp             ; 
    wire [ 7:0]                           s_ch_w_max_burst        ; 
    wire                                  s_ch_w_en               ; 
    wire                                  s_ch_w_ed_revr          ; 
    wire                                  s_ch_w_trig_sel         ; 
    wire                                  s_ch_w_atyp             ; 
    wire                                  s_ch_w_lli_wb           ; 
    wire [p_ADDR_WIDTH-1:0]               s_ch_saddr              ; 
    wire [p_ADDR_WIDTH-1:0]               s_ch_daddr              ; 
    wire [p_ADDR_WIDTH-1:0]               s_ch_rlen               ; 
    wire [p_ADDR_WIDTH-1:0]               s_ch_wlen               ; 
    wire [p_ADDR_WIDTH-1:0]               s_ch_llp                ;  
    wire [ 7:0]                           s_ch_bus                ; 

    wire [ 7:0]                           m_ch_int_sta_pulse      ; 

    wire                                  m_r_blk_done            ; 
    wire                                  m_w_blk_done            ; 

    wire                                  m_ch_star               ;
    wire                                  m_ch_en                 ; 

    wire [ 7:0]                           m_ch_to                 ;
    wire                                  m_ch_r_fifo_clr         ;
    wire                                  m_ch_w_fifo_clr         ;
    wire                                  m_ch_lli_en             ;
    wire                                  m_ch_fifo_sel           ;

    wire [ 7:0]                           m_ch_r_max_burst        ; 
    wire                                  m_ch_r_en               ; 
    wire                                  m_ch_r_ed_revr          ; 
    wire                                  m_ch_r_trig_sel         ; 
    wire                                  m_ch_r_atyp             ; 
    wire [ 7:0]                           m_ch_w_max_burst        ; 
    wire                                  m_ch_w_en               ; 
    wire                                  m_ch_w_ed_revr          ; 
    wire                                  m_ch_w_trig_sel         ; 
    wire                                  m_ch_w_atyp             ; 
    wire                                  m_ch_w_lli_wb           ; 
    wire [p_ADDR_WIDTH-1:0]               m_ch_saddr              ; 
    wire [p_ADDR_WIDTH-1:0]               m_ch_daddr              ; 
    wire [p_ADDR_WIDTH-1:0]               m_ch_rlen               ; 
    wire [p_ADDR_WIDTH-1:0]               m_ch_wlen               ; 
    wire [p_ADDR_WIDTH-1:0]               m_ch_llp                ; 

    wire [ 7:0]                           m_ch_bus                ; 

    wire                                  m_w_req                ; 
    wire                                  m_w_resp               ; 
    wire                                  m_w_ack                ;  
    wire                                  m_w_finish             ;
    wire                                  m_w_finish_back        ; 

    wire                                  m_r_req                ;
    wire                                  m_r_resp               ; 
    wire                                  m_r_ack                ;  
    wire                                  m_r_finish             ;
    wire                                  m_r_finish_back        ;  

    wire [p_ADDR_WIDTH-1:0]               w_base_addr_ns         ;  
    wire [p_ADDR_WIDTH-1:0]               w_trans_len_ns         ;  
    wire [ 7:0]                           w_strans_len_ns        ;  
    wire [p_ADDR_WIDTH-1:0]               w_base_addr            ;  
    wire [p_ADDR_WIDTH-1:0]               w_trans_len            ;  
    wire [ 7:0]                           w_strans_len           ;  

    wire                                  w_rd_en                ;  
    wire [p_DATA_WIDTH-1:0]               w_data                 ;  
    wire                                  w_strans_done_addr     ;  
    wire                                  w_trans_done_addr      ;
    wire                                  w_strans_done_data     ;

    wire [p_ADDR_WIDTH-1:0]               r_base_addr_ns         ;  
    wire [p_ADDR_WIDTH-1:0]               r_trans_len_ns         ;  
    wire [7-p_DATA_WIDTH/32:0]            r_strans_len_burst_ns  ;  
    wire [p_ADDR_WIDTH-1:0]               r_base_addr            ;  
    wire [p_ADDR_WIDTH-1:0]               r_trans_len            ; 
    wire [ 7:0]                           r_strans_len           ;
    wire [7-p_DATA_WIDTH/32:0]            r_strans_len_burst     ;
    wire [p_ADDR_WIDTH-1:0]               r_ch_saddr_max         ;

    wire [p_DATA_WIDTH-1:0]               r_data                 ;  
    wire                                  r_wr_en                ;  
    wire                                  r_strans_done_addr     ;  
    wire                                  r_trans_done_addr      ;  
    wire                                  r_strans_done_data     ;  

    wire [7:0]                            ch_to                  ;
    wire [7:0]                            ch_bus                 ;

    wire                                  r_revr_en              ;
    wire                                  w_revr_en              ;
    wire                                  ch_r_atyp              ;
    wire                                  ch_w_atyp              ;
    wire                                  r_to                   ;
    wire                                  w_to                   ;

    wire                                  w_dma_done             ;
    wire                                  r_dma_done             ;

    assign o_to[1:0] = {r_to, w_to };             
generate
if(p_CDC_MS_EN) begin:ch_controller_normal

    osr_ch_reg_bank u_ch_reg_bank(
        .i_s_clk           (i_s_clk           ),
        .i_s_rst_n         (i_s_rst_n         ),

        .i_reg_wr          (i_reg_wr          ),
        .i_reg_addr        (i_reg_addr        ),
        .i_reg_wdata       (i_reg_wdata       ),
        .o_reg_rdata       (o_reg_rdata       ),

        .i_dma_start       (i_dma_start       ), 
        .i_lli_en          (i_lli_en          ), 
        .i_ch_to           (i_ch_to           ), 
        .i_rd_endian       (i_rd_endian       ), 
        .i_rd_max_byte     (i_rd_max_byte     ), 
        .i_wr_endian       (i_wr_endian       ), 
        .i_wr_max_byte     (i_wr_max_byte     ), 
        .i_saddr           (i_saddr           ), 
        .i_daddr           (i_daddr           ), 
        .i_rlen            (i_rlen            ), 
        .i_wlen            (i_wlen            ), 
        .i_llp             (i_llp             ), 
        .i_w_to_int_en     (i_w_to_int_en     ), 
        .i_r_to_int_en     (i_r_to_int_en     ), 
        .i_w_dma_int_en    (i_w_dma_int_en    ), 
        .i_r_dma_int_en    (i_r_dma_int_en    ), 
        .o_w_dma_done      (w_dma_done        ), 
        .o_r_dma_done      (r_dma_done       ), 
        .i_w_to_clr        (i_w_to_clr        ), 
        .i_r_to_clr        (i_r_to_clr        ), 
        .o_to              (                  ),
        .i_w_dma_done_clr  (i_w_dma_done_clr  ), 
        .i_r_dma_done_clr  (i_r_dma_done_clr  ), 

        .i_ch_int_sta_pulse(s_ch_int_sta_pulse),

        .i_r_blk_done      (s_r_blk_done      ),
        .i_w_blk_done      (s_w_blk_done      ),

        .o_ch_star         (s_ch_star         ),
        .o_ch_en           (s_ch_en           ),

        .o_ch_to           (s_ch_to           ),
        .o_ch_r_fifo_clr   (s_ch_r_fifo_clr   ),
        .o_ch_w_fifo_clr   (s_ch_w_fifo_clr   ),
        .o_ch_lli_en       (s_ch_lli_en       ),
        .o_ch_fifo_sel     (s_ch_fifo_sel     ),

        .o_ch_r_max_burst  (s_ch_r_max_burst  ),
        .o_ch_r_en         (s_ch_r_en         ),
        .o_ch_r_ed_revr    (s_ch_r_ed_revr    ),
        .o_ch_r_trig_sel   (s_ch_r_trig_sel   ),
        .o_ch_r_atyp       (s_ch_r_atyp       ),
        .o_ch_w_max_burst  (s_ch_w_max_burst  ),
        .o_ch_w_en         (s_ch_w_en         ),
        .o_ch_w_ed_revr    (s_ch_w_ed_revr    ),
        .o_ch_w_trig_sel   (s_ch_w_trig_sel   ),
        .o_ch_w_atyp       (s_ch_w_atyp       ),
        .o_ch_w_lli_wb     (s_ch_w_lli_wb     ),
        .o_ch_saddr        (s_ch_saddr        ),
        .o_ch_daddr        (s_ch_daddr        ),
        .o_ch_rlen         (s_ch_rlen         ),
        .o_ch_wlen         (s_ch_wlen         ),
        .o_ch_llp          (s_ch_llp          ),

        .o_ch_bus          (s_ch_bus          ),

        .o_ch_int          (o_int             )
    );

    osr_ch_controller  #(
        .p_DATA_WIDTH         (p_DATA_WIDTH         ),
        .p_ADDR_WIDTH         (p_ADDR_WIDTH         ),         
        .p_CH_FIFO_EXIST      (p_CH_FIFO_EXIST      ),
        .p_LINK_LIST_EN       (p_LINK_LIST_EN       ),
        .p_AHB_SIMPLE         (p_AHB_SIMPLE         ),
        .p_CDC_MS_EN          (p_CDC_MS_EN          ),
        .p_CDC_MP_EN          (p_CDC_MP_EN          ),
        .p_R_FIFO_DEPTH_WIDTH (p_R_FIFO_DEPTH_WIDTH ),
        .p_W_FIFO_DEPTH_WIDTH (p_W_FIFO_DEPTH_WIDTH ),
        .p_CH_FIFO_DEPTH_WIDTH(p_CH_FIFO_DEPTH_WIDTH)
    ) u_ch_0(

        .i_m_clk                 (i_m_clk           ),
        .i_m_rst_n               (i_m_rst_n         ),

        .o_ch_int_sta_pulse      (m_ch_int_sta_pulse),

        .o_r_blk_done            (m_r_blk_done      ),
        .o_w_blk_done            (m_w_blk_done      ),

        .i_ch_star               (m_ch_star         ),
        .i_ch_en                 (m_ch_en           ),

        .i_ch_to                 (m_ch_to           ),
        .i_ch_r_fifo_clr         (m_ch_r_fifo_clr   ),
        .i_ch_w_fifo_clr         (m_ch_w_fifo_clr   ),
        .i_ch_lli_en             (m_ch_lli_en       ),
        .i_ch_fifo_sel           (m_ch_fifo_sel     ),

        .i_ch_r_max_burst        (m_ch_r_max_burst  ),
        .i_ch_r_en               (m_ch_r_en         ),
        .i_ch_r_ed_revr          (m_ch_r_ed_revr    ),
        .i_ch_r_trig_sel         (m_ch_r_trig_sel   ),
        .i_ch_r_atyp             (m_ch_r_atyp       ),
        .i_ch_w_max_burst        (m_ch_w_max_burst  ),
        .i_ch_w_en               (m_ch_w_en         ),
        .i_ch_w_ed_revr          (m_ch_w_ed_revr    ),
        .i_ch_w_trig_sel         (m_ch_w_trig_sel   ),
        .i_ch_w_atyp             (m_ch_w_atyp       ),
        .i_ch_w_lli_wb           (m_ch_w_lli_wb     ),
        .i_ch_saddr              (m_ch_saddr        ),
        .i_ch_daddr              (m_ch_daddr        ),
        .i_ch_rlen               (m_ch_rlen         ),
        .i_ch_wlen               (m_ch_wlen         ),
        .i_ch_llp                (m_ch_llp          ),
        .i_ch_bus                (m_ch_bus          ),

        .i_w_fifo_data           (i_w_fifo_data        ),
        .o_w_fifo_rd_enable      (o_w_fifo_rd_enable   ),
        .i_w_fifo_empty          (i_w_fifo_empty       ),
        .i_w_fifo_num_count      (i_w_fifo_num_count   ),
        .o_w_fifo_clear          (o_w_fifo_clear       ),

        .o_r_fifo_data           (o_r_fifo_data        ),
        .o_r_fifo_wr_enable      (o_r_fifo_wr_enable   ),
        .i_r_fifo_full           (i_r_fifo_full        ),
        .i_r_fifo_num_count      (i_r_fifo_num_count   ),
        .o_r_fifo_clear          (o_r_fifo_clear       ),

        .i_w_req                 (m_w_req              ),
        .i_w_resp                (m_w_resp             ),
        .o_w_ack                 (m_w_ack              ),
        .o_w_finish              (m_w_finish           ),
        .i_w_finish_back         (m_w_finish_back      ),

        .i_r_req                 (m_r_req              ),
        .i_r_resp                (m_r_resp             ),
        .o_r_ack                 (m_r_ack              ),
        .o_r_finish              (m_r_finish           ),
        .i_r_finish_back         (m_r_finish_back      ),

        .i_w_base_addr_ns        (w_base_addr_ns       ),
        .i_w_trans_len_ns        (w_trans_len_ns       ),
        .i_w_strans_len_ns       (w_strans_len_ns      ),
        .o_w_base_addr           (w_base_addr          ),
        .o_w_trans_len           (w_trans_len          ),
        .o_w_strans_len          (w_strans_len         ),

        .i_w_rd_en               (w_rd_en              ),
        .o_w_data                (w_data               ),
        .i_w_strans_done_addr    (w_strans_done_addr   ),
        .i_w_trans_done_addr     (w_trans_done_addr    ),
        .i_w_strans_done_data    (w_strans_done_data   ),

        .i_r_base_addr_ns        (r_base_addr_ns       ),
        .i_r_trans_len_ns        (r_trans_len_ns       ),
        .i_r_strans_len_burst_ns (r_strans_len_burst_ns),
        .o_r_base_addr           (r_base_addr          ),
        .o_r_trans_len           (r_trans_len          ),
        .o_r_strans_len          (r_strans_len         ),
        .o_r_strans_len_burst    (r_strans_len_burst   ),
        .i_r_data                (r_data               ),
        .i_r_wr_en               (r_wr_en              ),
        .i_r_strans_done_addr    (r_strans_done_addr   ),
        .i_r_trans_done_addr     (r_trans_done_addr    ),
        .i_r_strans_done_data    (r_strans_done_data   ),

        .i_w_to                  (w_to                 ),
        .i_r_to                  (r_to                 ),

        .o_ch_to                 (ch_to                ),
        .o_ch_bus                (ch_bus               ),

        .o_ch_r_atyp             (ch_r_atyp            ),
        .o_ch_w_atyp             (ch_w_atyp            ),

        .o_r_revr_en             (r_revr_en            ),
        .o_w_revr_en             (w_revr_en            )
    );

end
else begin:ch_controller_simple

    osr_ch_controller_reg  #(
        .p_DATA_WIDTH         (p_DATA_WIDTH            ),
        .p_ADDR_WIDTH         (p_ADDR_WIDTH            ),
        .p_CH_FIFO_EXIST      (p_CH_FIFO_EXIST         ),
        .p_LINK_LIST_EN       (p_LINK_LIST_EN          ),
        .p_AHB_SIMPLE         (p_AHB_SIMPLE            ),
        .p_CDC_MS_EN          (p_CDC_MS_EN             ),
        .p_CDC_MP_EN          (p_CDC_MP_EN             ),        
        .p_R_FIFO_DEPTH_WIDTH (p_R_FIFO_DEPTH_WIDTH    ),
        .p_W_FIFO_DEPTH_WIDTH (p_W_FIFO_DEPTH_WIDTH    ),
        .p_CH_FIFO_DEPTH_WIDTH(p_CH_FIFO_DEPTH_WIDTH   )
    ) u_ch_0(

        .i_m_clk                 (i_m_clk              ),
        .i_m_rst_n               (i_m_rst_n            ),

        .i_reg_wr                (i_reg_wr             ),
        .i_reg_addr              (i_reg_addr           ),
        .i_reg_wdata             (i_reg_wdata          ),
        .o_reg_rdata             (o_reg_rdata          ),

        .i_dma_start             (i_dma_start          ), 
        .i_suspend               (i_suspend            ),
        .i_lli_en                (i_lli_en             ), 
        .i_ch_to                 (i_ch_to              ), 
        .i_rd_endian             (i_rd_endian          ), 
        .i_rd_max_byte           (i_rd_max_byte        ), 
        .i_wr_endian             (i_wr_endian          ), 
        .i_wr_max_byte           (i_wr_max_byte        ), 
        .i_saddr                 (i_saddr              ), 
        .i_daddr                 (i_daddr              ), 
        .i_rlen                  (i_rlen               ), 
        .i_wlen                  (i_wlen               ), 
        .i_llp                   (i_llp                ), 
        .i_w_to_int_en           (i_w_to_int_en        ), 
        .i_r_to_int_en           (i_r_to_int_en        ), 
        .i_w_dma_int_en          (i_w_dma_int_en       ), 
        .i_r_dma_int_en          (i_r_dma_int_en       ), 
        .o_w_dma_done            (w_dma_done           ), 
        .o_r_dma_done            (r_dma_done           ), 
        .i_w_to_clr              (i_w_to_clr           ), 
        .i_r_to_clr              (i_r_to_clr           ), 
        .i_w_dma_done_clr        (i_w_dma_done_clr     ), 
        .i_r_dma_done_clr        (i_r_dma_done_clr     ), 
        .o_to                    (                     ),

        .i_w_fifo_data           (i_w_fifo_data        ),
        .o_w_fifo_rd_enable      (o_w_fifo_rd_enable   ),
        .i_w_fifo_empty          (i_w_fifo_empty       ),
        .i_w_fifo_num_count      (i_w_fifo_num_count   ),
        .o_w_fifo_clear          (o_w_fifo_clear       ),

        .o_r_fifo_data           (o_r_fifo_data        ),
        .o_r_fifo_wr_enable      (o_r_fifo_wr_enable   ),
        .i_r_fifo_full           (i_r_fifo_full        ),
        .i_r_fifo_num_count      (i_r_fifo_num_count   ),
        .o_r_fifo_clear          (o_r_fifo_clear       ),

        .i_w_req                 (m_w_req              ),
        .i_w_resp                (m_w_resp             ),
        .o_w_ack                 (m_w_ack              ),
        .o_w_finish              (m_w_finish           ),
        .i_w_finish_back         (m_w_finish_back      ),

        .i_r_req                 (m_r_req              ),
        .i_r_resp                (m_r_resp             ),
        .o_r_ack                 (m_r_ack              ),
        .o_r_finish              (m_r_finish           ),
        .i_r_finish_back         (m_r_finish_back      ),        

        .i_w_base_addr_ns        (w_base_addr_ns       ),
        .i_w_trans_len_ns        (w_trans_len_ns       ),
        .i_w_strans_len_ns       (w_strans_len_ns      ),
        .o_w_base_addr           (w_base_addr          ),
        .o_w_trans_len           (w_trans_len          ),
        .o_w_strans_len          (w_strans_len         ),

        .i_w_rd_en               (w_rd_en              ),
        .o_w_data                (w_data               ),
        .i_w_strans_done_addr    (w_strans_done_addr   ),
        .i_w_trans_done_addr     (w_trans_done_addr    ),
        .i_w_strans_done_data    (w_strans_done_data   ),

        .i_r_base_addr_ns        (r_base_addr_ns       ),
        .i_r_trans_len_ns        (r_trans_len_ns       ),
        .i_r_strans_len_burst_ns (r_strans_len_burst_ns),
        .o_r_base_addr           (r_base_addr          ),
        .o_r_trans_len           (r_trans_len          ),
        .o_r_strans_len          (r_strans_len         ),
        .o_r_strans_len_burst    (r_strans_len_burst   ),
        .i_r_data                (r_data               ),
        .i_r_wr_en               (r_wr_en              ),
        .i_r_strans_done_addr    (r_strans_done_addr   ),
        .i_r_trans_done_addr     (r_trans_done_addr    ),
        .i_r_strans_done_data    (r_strans_done_data   ),
        .o_r_ch_saddr_max        (r_ch_saddr_max       ),

        .i_w_to                  (w_to                 ),
        .i_r_to                  (r_to                 ),

        .o_ch_to                 (ch_to                ),
        .o_ch_bus                (ch_bus               ),

        .o_ch_r_atyp             (ch_r_atyp            ),
        .o_ch_w_atyp             (ch_w_atyp            ),

        .o_r_revr_en             (r_revr_en            ),
        .o_w_revr_en             (w_revr_en            ),

        .o_ch_int                (o_int                )
    );
end
endgenerate

    osr_ahb_master_if   #(
        .p_DATA_WIDTH         (p_DATA_WIDTH             ),
        .p_ADDR_WIDTH         (p_ADDR_WIDTH             ), 
        .p_CH_FIFO_EXIST      (p_CH_FIFO_EXIST          ),
        .p_LINK_LIST_EN       (p_LINK_LIST_EN           ),
        .p_AHB_SIMPLE         (p_AHB_SIMPLE             ),
        .p_CDC_MS_EN          (p_CDC_MS_EN              ),
        .p_CDC_MP_EN          (p_CDC_MP_EN              ),
        .p_R_FIFO_DEPTH_WIDTH (p_R_FIFO_DEPTH_WIDTH     ),
        .p_W_FIFO_DEPTH_WIDTH (p_W_FIFO_DEPTH_WIDTH     ),
        .p_CH_FIFO_DEPTH_WIDTH(p_CH_FIFO_DEPTH_WIDTH    )
    ) u_m_if(
        .i_m_clk                 (i_m_clk               ),
        .i_m_rst_n               (i_m_rst_n             ),

        .o_w_base_addr_ns        (w_base_addr_ns        ),
        .o_w_trans_len_ns        (w_trans_len_ns        ),
        .o_w_strans_len_ns       (w_strans_len_ns       ),
        .i_w_base_addr           (w_base_addr           ),
        .i_w_trans_len           (w_trans_len           ),
        .i_w_strans_len          (w_strans_len          ),

        .o_w_rd_en               (w_rd_en               ),
        .i_w_data                (w_data                ),
        .o_w_strans_done_addr    (w_strans_done_addr    ),
        .o_w_trans_done_addr     (w_trans_done_addr     ),
        .o_w_strans_done_data    (w_strans_done_data    ),

        .o_r_base_addr_ns        (r_base_addr_ns        ),
        .o_r_trans_len_ns        (r_trans_len_ns        ),
        .o_r_strans_len_burst_ns (r_strans_len_burst_ns ),
        .i_r_base_addr           (r_base_addr           ),
        .i_r_trans_len           (r_trans_len           ),
        .i_r_strans_len          (r_strans_len          ),
        .i_r_strans_len_burst    (r_strans_len_burst    ),
        .i_r_ch_saddr_max        (r_ch_saddr_max        ),

        .o_r_data                (r_data                ),
        .o_r_wr_en               (r_wr_en               ),
        .o_r_strans_done_addr    (r_strans_done_addr    ),
        .o_r_trans_done_addr     (r_trans_done_addr     ),
        .o_r_strans_done_data    (r_strans_done_data    ),
        .o_w_to                  (w_to                  ),
        .o_r_to                  (r_to                  ),

        .i_ch_to                 (ch_to                 ),
        .i_ch_bus                (ch_bus                ),
        .i_r_revr_en             (r_revr_en             ),
        .i_w_revr_en             (w_revr_en             ),
        .i_ch_r_atyp             (ch_r_atyp             ),
        .i_ch_w_atyp             (ch_w_atyp             ),
        .i_suspend               (i_suspend            ),
        .o_suspend_done          (o_suspend_done       ),

        .o_w_haddr               (o_w_haddr             ),
        .o_w_hburst              (o_w_hburst            ),
        .o_w_hsize               (o_w_hsize             ),
        .o_w_htrans              (o_w_htrans            ),
        .o_w_hwdata              (o_w_hwdata            ),
        .o_w_hprot               (o_w_hprot             ),
        .o_w_hwrite              (o_w_hwrite            ),
        .i_w_hrdata              (i_w_hrdata            ),
        .i_w_hready              (i_w_hready            ),

        .o_r_haddr               (o_r_haddr             ),
        .o_r_hburst              (o_r_hburst            ),
        .o_r_hsize               (o_r_hsize             ),
        .o_r_htrans              (o_r_htrans            ),
        .o_r_hwdata              (o_r_hwdata            ),
        .o_r_hprot               (o_r_hprot             ),
        .o_r_hwrite              (o_r_hwrite            ),
        .i_r_hrdata              (i_r_hrdata            ),
        .i_r_hready              (i_r_hready            )
    );

generate
if(p_CDC_MS_EN) begin:cdc_ms_en
    osr_cdc_ms u_cdc_ms(
        .i_s_clk  (i_s_clk                    ),
        .i_s_rst_n(i_s_rst_n                  ),

        .i_m_clk  (i_m_clk                    ),
        .i_m_rst_n(i_m_rst_n                  ),

        .i_s2m_p2p({s_ch_r_fifo_clr,s_ch_w_fifo_clr}),
        .o_s2m_p2p({m_ch_r_fifo_clr,m_ch_w_fifo_clr}),

        .i_s2m_dff({s_ch_en,s_ch_star}        ),
        .o_s2m_dff({m_ch_en,m_ch_star}        ),

        .i_m2s_p2p({m_w_blk_done,m_r_blk_done,m_ch_int_sta_pulse}),
        .o_m2s_p2p({s_w_blk_done,s_r_blk_done,s_ch_int_sta_pulse})

    );

    assign m_ch_to          =s_ch_to           ;

    assign m_ch_lli_en      =s_ch_lli_en       ;
    assign m_ch_fifo_sel    =s_ch_fifo_sel     ;  

    assign m_ch_r_max_burst =s_ch_r_max_burst  ; 
    assign m_ch_r_en        =s_ch_r_en         ; 
    assign m_ch_r_ed_revr   =s_ch_r_ed_revr    ; 
    assign m_ch_r_trig_sel  =s_ch_r_trig_sel   ; 
    assign m_ch_r_atyp      =s_ch_r_atyp       ; 
    assign m_ch_w_max_burst =s_ch_w_max_burst  ; 
    assign m_ch_w_en        =s_ch_w_en         ; 
    assign m_ch_w_ed_revr   =s_ch_w_ed_revr    ; 
    assign m_ch_w_trig_sel  =s_ch_w_trig_sel   ; 
    assign m_ch_w_atyp      =s_ch_w_atyp       ; 
    assign m_ch_w_lli_wb    =s_ch_w_lli_wb     ; 
    assign m_ch_saddr       =s_ch_saddr        ; 
    assign m_ch_daddr       =s_ch_daddr        ; 
    assign m_ch_rlen        =s_ch_rlen         ; 
    assign m_ch_wlen        =s_ch_wlen         ; 
    assign m_ch_llp         =s_ch_llp          ; 

    assign m_ch_bus         =s_ch_bus          ; 
end
endgenerate

    osr_cdc_mp #(
        .p_DATA_WIDTH         (p_DATA_WIDTH             ),
        .p_ADDR_WIDTH         (p_ADDR_WIDTH             ), 
        .p_CH_FIFO_EXIST      (p_CH_FIFO_EXIST          ),
        .p_LINK_LIST_EN       (p_LINK_LIST_EN           ),
        .p_AHB_SIMPLE         (p_AHB_SIMPLE             ),
        .p_CDC_MS_EN          (p_CDC_MS_EN              ),
        .p_CDC_MP_EN          (p_CDC_MP_EN              ),
        .p_R_FIFO_DEPTH_WIDTH (p_R_FIFO_DEPTH_WIDTH     ),
        .p_W_FIFO_DEPTH_WIDTH (p_W_FIFO_DEPTH_WIDTH     ),
        .p_CH_FIFO_DEPTH_WIDTH(p_CH_FIFO_DEPTH_WIDTH    )
    ) u_cdc_mp(
        .i_m_clk          (i_m_clk                 ),
        .i_m_rst_n        (i_m_rst_n               ),

        .i_p_clk          (i_p_clk                 ),
        .i_p_rst_n        (i_p_rst_n               ),

        .i_p2m_req        ({i_w_req,i_r_req}       ),
        .o_p2m_req        ({m_w_req,m_r_req}       ),

        .i_m2p_finish     ({m_w_finish,m_r_finish} ),
        .o_m2p_finish     ({o_w_finish,o_r_finish} ),
        .o_p2m_finish_back({m_w_finish_back,m_r_finish_back} ),

        .i_m2p_ack        ({m_w_ack,m_r_ack}       ),
        .o_m2p_ack        ({o_w_ack,o_r_ack}       ),

        .o_w_resp         (m_w_resp                ), 
        .o_r_resp         (m_r_resp                )
    );

reg                  r_r_done;
reg                  r_w_done;

always @(posedge i_s_clk or negedge i_s_rst_n) begin
    if(!i_s_rst_n) begin
        r_r_done    <= 1'h0        ;  
        r_w_done    <= 1'h0        ; 
    end else begin
        r_r_done    <= r_dma_done | (r_r_done & (~i_r_dma_done_clr));
        r_w_done    <= w_dma_done | (r_w_done & (~i_w_dma_done_clr));
    end
end 
assign o_r_dma_done_lvl  =  r_dma_done | r_r_done ;
assign o_w_dma_done_lvl  =  w_dma_done | r_w_done ;

assign o_r_dma_done  = r_dma_done ;
assign o_w_dma_done  = w_dma_done ;

endmodule
