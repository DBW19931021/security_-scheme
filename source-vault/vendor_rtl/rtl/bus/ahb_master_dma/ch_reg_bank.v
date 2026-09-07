//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ch_reg_bank #(
    parameter p_DATA_WIDTH         =32,
    parameter p_ADDR_WIDTH         =32,
    parameter p_CH_FIFO_EXIST      =0,
    parameter p_LINK_LIST_EN       =1,
    parameter p_AHB_SIMPLE         =0,
    parameter p_CDC_MS_EN          =1,
    parameter p_CDC_MP_EN          =1,
    parameter p_R_FIFO_DEPTH_WIDTH =3,
    parameter p_W_FIFO_DEPTH_WIDTH =3,
    parameter p_CH_FIFO_DEPTH_WIDTH=3
)
(

    input   wire                                         i_s_clk               ,
    input   wire                                         i_s_rst_n             ,

    input   wire                                         i_reg_wr              ,
    input   wire [4:0]                                   i_reg_addr            ,
    input   wire [31:0]                                  i_reg_wdata           ,
    output  reg  [31:0]                                  o_reg_rdata           ,

    input   wire                                         i_dma_start           ,
    input   wire                                         i_lli_en              ,
    input   wire [7:0]                                   i_ch_to               ,
    input   wire                                         i_rd_endian           ,
    input   wire [7:0]                                   i_rd_max_byte         ,
    input   wire                                         i_wr_endian           ,
    input   wire [7:0]                                   i_wr_max_byte         ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_saddr               ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_daddr               ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_rlen                ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_wlen                ,
    input   wire [p_ADDR_WIDTH-1:0]                      i_llp                 ,
    input   wire                                         i_w_to_int_en         ,
    input   wire                                         i_r_to_int_en         ,
    input   wire                                         i_w_dma_int_en        ,
    input   wire                                         i_r_dma_int_en        ,
    output  wire                                         o_w_dma_done          ,
    output  wire                                         o_r_dma_done          ,
    input   wire                                         i_w_to_clr            ,
    input   wire                                         i_r_to_clr            ,
    input   wire                                         i_w_dma_done_clr      ,
    input   wire                                         i_r_dma_done_clr      ,
    output  wire                                         o_to                  ,

    input   wire [ 7:0]                                  i_ch_int_sta_pulse    ,

    input   wire                                         i_r_blk_done          ,
    input   wire                                         i_w_blk_done          ,

    output  wire                                         o_ch_star             ,
    output  wire                                         o_ch_en               ,

    output  wire                                         o_ch_fifo_sel         ,
    output  wire                                         o_ch_lli_en           ,
    output  wire                                         o_ch_r_fifo_clr       ,
    output  wire                                         o_ch_w_fifo_clr       ,
    output  wire [ 7:0]                                  o_ch_to               ,

    output  wire [ 7:0]                                  o_ch_r_max_burst      ,
    output  wire                                         o_ch_r_en             ,
    output  wire                                         o_ch_r_ed_revr        ,
    output  wire                                         o_ch_r_trig_sel       ,
    output  wire                                         o_ch_r_atyp           ,
    output  wire [ 7:0]                                  o_ch_w_max_burst      ,
    output  wire                                         o_ch_w_en             ,
    output  wire                                         o_ch_w_ed_revr        ,
    output  wire                                         o_ch_w_trig_sel       ,
    output  wire                                         o_ch_w_atyp           ,
    output  wire                                         o_ch_w_lli_wb         ,
    output  wire [31:0]                                  o_ch_saddr            ,
    output  wire [31:0]                                  o_ch_daddr            ,
    output  wire [31:0]                                  o_ch_rlen             ,
    output  wire [31:0]                                  o_ch_wlen             ,
    output  wire [31:0]                                  o_ch_llp              ,

    output  wire [ 7:0]                                  o_ch_bus              ,

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

    wire [31:0] reg_rdata           ;

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

    reg  [p_ADDR_WIDTH-1:0] ch_saddr            ;

    reg  [p_ADDR_WIDTH-1:0] ch_daddr            ;

    reg  [p_ADDR_WIDTH-1:0] ch_rlen             ;

    reg  [p_ADDR_WIDTH-1:0] ch_wlen             ;

    reg  [p_ADDR_WIDTH-1:0] ch_llp              ;

    reg  [15:0] ch_w_blk_cnt        ;
    reg  [15:0] ch_r_blk_cnt        ;

    reg  [ 7:0] ch_int_en           ;

    wire [ 7:0] ch_int_sta          ;

    wire [ 7:0] ch_int_clr          ;

    reg  [7:0] ch_bus              ;

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

    wire [ 7:0] ch_w_max_burst_in   ;
    wire        ch_w_en_in          ;
    wire        ch_w_ed_revr_in     ;
    wire        ch_w_trig_sel_in    ;
    wire        ch_w_atyp_in        ;
    wire        ch_w_lli_wb_in      ;

    wire [p_ADDR_WIDTH-1:0] ch_saddr_in         ;

    wire [p_ADDR_WIDTH-1:0] ch_daddr_in         ;

    wire [p_ADDR_WIDTH-1:0] ch_rlen_in          ;

    wire [p_ADDR_WIDTH-1:0] ch_wlen_in          ;

    wire [p_ADDR_WIDTH-1:0] ch_llp_in           ;

    wire [15:0] ch_w_blk_cnt_in     ;

    wire [15:0] ch_r_blk_cnt_in     ;

    wire [ 7:0] ch_int_en_in        ;

    wire [7:0] ch_bus_in           ;

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

    wire        ch_star_redge;
    wire        ch_w_fifo_clr_redge;
    wire        ch_r_fifo_clr_redge;

    wire        ch_star_pulse;
    wire        ch_w_fifo_clr_pulse;
    wire        ch_r_fifo_clr_pulse;

    wire        trans_done_redge;
    wire        trans_done_pulse;
    reg         r_trans_busy;
    wire        r_trans_busy_in;
    reg         w_trans_busy;
    wire        w_trans_busy_in;

    wire [23:0] ahb_dmac_version;
    wire        ahb_dmac_p_ch_fifo_exist;
    wire        ahb_dmac_p_link_list_en;
    wire        ahb_dmac_p_ahb_simple;
    wire        ahb_dmac_p_cdc_ms_en;
    wire        ahb_dmac_p_cdc_mp_en;

    reg  w_to                       ;
    reg  r_to                       ;
    reg  reg_w_strans_done          ;
    reg  reg_r_strans_done          ;
    reg  reg_w_trans_done           ;
    reg  reg_r_trans_done           ;
    reg  reg_w_trans_all_done       ;
    reg  reg_r_trans_all_done       ;

    wire w_to_in                    ;
    wire r_to_in                    ;
    wire reg_w_strans_done_in       ;
    wire reg_r_strans_done_in       ;
    wire reg_w_trans_done_in        ;
    wire reg_r_trans_done_in        ;
    wire reg_w_trans_all_done_in    ;
    wire reg_r_trans_all_done_in    ;

    wire w_to_clr                   ;
    wire r_to_clr                   ;
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

    assign trans_done_redge= (~r_trans_busy)&(~w_trans_busy) ;

    always@(posedge i_s_clk or negedge i_s_rst_n)
    begin
        if(~i_s_rst_n) begin
            r_trans_busy <= 1'b0;
            w_trans_busy <= 1'b0;
        end
        else begin
            r_trans_busy <=  r_trans_busy_in;
            w_trans_busy <=  w_trans_busy_in;
        end
    end

    assign r_trans_busy_in = (ch_star_pulse&ch_r_en)           ? 1'b1 :
                             (reg_r_trans_all_done|(~ch_star)) ? 1'b0 : r_trans_busy ;

    assign w_trans_busy_in = (ch_star_pulse&ch_w_en)           ? 1'b1 :
                             (reg_w_trans_all_done|(~ch_star)) ? 1'b0 : w_trans_busy ;

    assign reg_r_trans_all_done_clr =ch_int_clr[0];
    assign reg_w_trans_all_done_clr =ch_int_clr[1];
    assign reg_r_trans_done_clr     =ch_int_clr[2];
    assign reg_w_trans_done_clr     =ch_int_clr[3];
    assign reg_r_strans_done_clr    =ch_int_clr[4];
    assign reg_w_strans_done_clr    =ch_int_clr[5];
    assign r_to_clr                 =ch_int_clr[6];
    assign w_to_clr                 =ch_int_clr[7];

    assign w_to_in                 = (w_to_clr)                 ? 1'b0 :
                                     (i_ch_int_sta_pulse[7])    ? 1'b1 : w_to ;

    assign r_to_in                 = (r_to_clr)                 ? 1'b0 :
                                     (i_ch_int_sta_pulse[6])    ? 1'b1 : r_to ;

    assign reg_w_strans_done_in    = (reg_w_strans_done_clr) ? 1'b0 :
                                     (i_ch_int_sta_pulse[5]) ? 1'b1 : reg_w_strans_done ;

    assign reg_r_strans_done_in    = (reg_r_strans_done_clr) ? 1'b0 :
                                     (i_ch_int_sta_pulse[4]) ? 1'b1 : reg_r_strans_done ;

    assign reg_w_trans_done_in     = (reg_w_trans_done_clr)  ? 1'b0 :
                                     (i_ch_int_sta_pulse[3]) ? 1'b1 : reg_w_trans_done ;

    assign reg_r_trans_done_in     = (reg_r_trans_done_clr)  ? 1'b0 :
                                     (i_ch_int_sta_pulse[2]) ? 1'b1 : reg_r_trans_done ;

    assign reg_w_trans_all_done_in = (reg_w_trans_all_done_clr) ? 1'b0 :
                                     (i_ch_int_sta_pulse[1])    ? 1'b1 : reg_w_trans_all_done ;

    assign reg_r_trans_all_done_in = (reg_r_trans_all_done_clr) ? 1'b0 :
                                     (i_ch_int_sta_pulse[0])    ? 1'b1 : reg_r_trans_all_done ;

    always@(posedge i_s_clk or negedge i_s_rst_n)
    begin
        if(~i_s_rst_n) begin
            w_to                 <= 1'b0;
            r_to                 <= 1'b0;
            reg_w_strans_done    <= 1'b0;
            reg_r_strans_done    <= 1'b0;
            reg_w_trans_done     <= 1'b0; 
            reg_r_trans_done     <= 1'b0;
            reg_w_trans_all_done <= 1'b0; 
            reg_r_trans_all_done <= 1'b0;
        end
        else begin
            w_to                 <=   w_to_in              ;
            r_to                 <=   r_to_in              ;
            reg_w_strans_done    <=   reg_w_strans_done_in ; 
            reg_r_strans_done    <=   reg_r_strans_done_in ;  
            reg_w_trans_done     <=   reg_w_trans_done_in  ;  
            reg_r_trans_done     <=   reg_r_trans_done_in  ;  
            reg_w_trans_all_done <=   reg_w_trans_all_done_in  ;  
            reg_r_trans_all_done <=   reg_r_trans_all_done_in  ;
        end
    end

    assign ch_star_redge=ch_star;
    assign ch_w_fifo_clr_redge=ch_w_fifo_clr;
    assign ch_r_fifo_clr_redge=ch_r_fifo_clr;

    osr_redge_to_pulse #(
        .p_NUM(4)
    ) u_e2p(
        .clk(i_s_clk),
        .rst_n(i_s_rst_n),
        .in_edge({ch_star_redge,ch_w_fifo_clr_redge,ch_r_fifo_clr_redge,trans_done_redge}),
        .out_pulse({ch_star_pulse,ch_w_fifo_clr_pulse,ch_r_fifo_clr_pulse,trans_done_pulse})
    );

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

    assign ch_star_in       = i_dma_start ? 1'b1 : 
                              (trans_done_pulse)          ? 1'b0              : ch_star    ;

    assign ch_en_in         = 1'b1;

    assign ch_to_in         = i_ch_to;

    assign ch_r_fifo_clr    =(ch_ctl_hit      &i_reg_wr)&i_reg_wdata[8];
    assign ch_w_fifo_clr    =(ch_ctl_hit      &i_reg_wr)&i_reg_wdata[9];

    assign ch_lli_en_in     = i_lli_en;
    assign ch_fifo_sel_in   =(ch_ctl_hit      &i_reg_wr&~ch_en) ? i_reg_wdata[11]:ch_fifo_sel       ;

    assign ch_r_max_burst_in=i_rd_max_byte;
    assign ch_r_en_in       =1'b1;
    assign ch_r_ed_revr_in  =i_rd_endian;
    assign ch_r_trig_sel_in =(ch_rctl_hit    &i_reg_wr&~ch_en) ? i_reg_wdata[10]   :ch_r_trig_sel   ;
    assign ch_r_atyp_in     =1'b0;

    assign ch_w_max_burst_in=i_wr_max_byte;
    assign ch_w_en_in       =1'b1;
    assign ch_w_ed_revr_in  =i_wr_endian;
    assign ch_w_trig_sel_in =(ch_wctl_hit    &i_reg_wr&~ch_en) ? i_reg_wdata[10]   :ch_w_trig_sel   ;
    assign ch_w_atyp_in     =1'b0;
    assign ch_w_lli_wb_in   =(ch_wctl_hit    &i_reg_wr&~ch_en) ? i_reg_wdata[12]   :ch_w_lli_wb     ;

    assign ch_saddr_in      =i_saddr;

    assign ch_daddr_in      =i_daddr;

    assign ch_rlen_in       =i_rlen;

    assign ch_wlen_in       =i_wlen;

    assign ch_llp_in        =i_llp;

    assign ch_w_blk_cnt_in  =(ch_star_pulse)  ? 16'b0              :
                             (i_w_blk_done)   ? ch_w_blk_cnt+16'b1 : ch_w_blk_cnt ;

    assign ch_r_blk_cnt_in  =(ch_star_pulse)  ? 16'b0              :
                             (i_r_blk_done)   ? ch_r_blk_cnt+16'b1 : ch_r_blk_cnt ;

    assign ch_int_en_in     ={i_w_to_int_en,i_r_to_int_en,4'b0,
                              i_w_dma_int_en,i_r_dma_int_en};

    assign o_w_dma_done     = reg_w_trans_all_done;
    assign o_r_dma_done     = reg_r_trans_all_done;

    assign ch_int_sta={w_to,r_to,reg_w_strans_done,reg_r_strans_done,reg_w_trans_done,reg_r_trans_done,reg_w_trans_all_done,reg_r_trans_all_done};

    assign ch_int_clr       ={i_w_to_clr,i_r_to_clr,4'b0,
                              i_w_dma_done_clr,i_r_dma_done_clr};
    assign o_to             = w_to || r_to;

    assign ch_bus_in        =(ch_bus_hit     &i_reg_wr&~ch_en) ? i_reg_wdata[7:0] : ch_bus          ;

    osr_mux16 #(32) rd_reg_mux0(
        .o_d(reg_rdata),
        .i_d0 ({ahb_dmac_version,6'b0,ch_en,ch_star}                        ),.i_s0 (ch_star_hit    ),
        .i_d1 ({20'b0,ch_fifo_sel,ch_lli_en,2'b0,
                ch_to}                                                      ),.i_s1 (ch_ctl_hit     ),
        .i_d2 ({16'b0,
                4'b0,ch_r_atyp,ch_r_trig_sel,ch_r_ed_revr,ch_r_en,
                ch_r_max_burst}                                             ),.i_s2 (ch_rctl_hit    ),
        .i_d3 ({16'b0,
                3'b0,ch_w_lli_wb,ch_w_atyp,ch_w_trig_sel,ch_w_ed_revr,ch_w_en,
                ch_w_max_burst}                                             ),.i_s3 (ch_wctl_hit    ),
        .i_d4 ({{(32-p_ADDR_WIDTH){1'b0}},ch_saddr}                         ),.i_s4 (ch_saddr_hit   ),
        .i_d5 ({{(32-p_ADDR_WIDTH){1'b0}},ch_daddr}                         ),.i_s5 (ch_daddr_hit   ),
        .i_d6 ({{(32-p_ADDR_WIDTH){1'b0}},ch_rlen }                         ),.i_s6 (ch_rlen_hit    ),
        .i_d7 ({{(32-p_ADDR_WIDTH){1'b0}},ch_wlen }                         ),.i_s7 (ch_wlen_hit    ),
        .i_d8 ({{(32-p_ADDR_WIDTH){1'b0}},ch_llp  }                         ),.i_s8 (ch_llp_hit     ),
        .i_d9 ({ch_w_blk_cnt,ch_r_blk_cnt}                                  ),.i_s9 (ch_blk_cnt_hit ),
        .i_d10({24'b0,ch_int_en }                                           ),.i_s10(ch_int_en_hit  ),
        .i_d11({24'b0,ch_int_sta}                                           ),.i_s11(ch_int_sta_hit ),
        .i_d12(32'b0                                                        ),.i_s12(ch_int_clr_hit ),
        .i_d13({24'b0,ch_bus}                                               ),.i_s13(ch_bus_hit     ),
        .i_d14(32'b0                                                        ),.i_s14(1'b0           ),
        .i_d15(32'b0                                                        )

    ); 

    always@(posedge i_s_clk or negedge i_s_rst_n)
    begin
        if(~i_s_rst_n) begin
            o_reg_rdata <= 32'b0;
        end
        else begin
            o_reg_rdata <=  reg_rdata;
        end
    end

    always@(posedge i_s_clk or negedge i_s_rst_n)
    begin
        if(~i_s_rst_n) begin

            ch_star         <=1'b0;
            ch_en           <=1'b0;

            ch_to           <=8'b0;
            ch_lli_en       <=1'b0;
            ch_fifo_sel     <=1'b0;

            ch_r_max_burst  <=8'b0;
            ch_r_en         <=1'b0;
            ch_r_ed_revr    <=1'b0;
            ch_r_trig_sel   <=1'b0;
            ch_r_atyp       <=1'b0;

            ch_w_max_burst  <=8'b0;
            ch_w_en         <=1'b0;
            ch_w_ed_revr    <=1'b0;
            ch_w_trig_sel   <=1'b0;
            ch_w_atyp       <=1'b0;
            ch_w_lli_wb     <=1'b0;

            ch_saddr        <={p_ADDR_WIDTH{1'b0}};

            ch_daddr        <={p_ADDR_WIDTH{1'b0}};

            ch_rlen         <={p_ADDR_WIDTH{1'b0}};

            ch_wlen         <={p_ADDR_WIDTH{1'b0}};

            ch_llp          <={p_ADDR_WIDTH{1'b0}};

            ch_w_blk_cnt    <=16'b0   ;

            ch_r_blk_cnt    <=16'b0   ;

            ch_int_en       <= 8'b0   ;

            ch_bus          <= 8'b0   ;

        end
        else begin

            ch_star          <=  ch_star_in         ;

            ch_en           <=  ch_en_in            ;

            ch_r_max_burst  <=  ch_r_max_burst_in   ;
            ch_r_en         <=  ch_r_en_in          ;
            ch_r_ed_revr    <=  ch_r_ed_revr_in     ;
            ch_r_trig_sel   <=  ch_r_trig_sel_in    ;
            ch_r_atyp       <=  ch_r_atyp_in        ;

            ch_w_max_burst  <=  ch_w_max_burst_in   ;
            ch_w_en         <=  ch_w_en_in          ;
            ch_w_ed_revr    <=  ch_w_ed_revr_in     ;
            ch_w_trig_sel   <=  ch_w_trig_sel_in    ;
            ch_w_atyp       <=  ch_w_atyp_in        ;
            ch_w_lli_wb     <=  ch_w_lli_wb_in      ;

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

    assign o_ch_star       = ch_star&ch_en  ;
    assign o_ch_en         = ch_en          ;

    assign o_ch_to         = ch_to          ;
    assign o_ch_r_fifo_clr = ch_r_fifo_clr_pulse;
    assign o_ch_w_fifo_clr = ch_w_fifo_clr_pulse;
    assign o_ch_lli_en     = ch_lli_en      ;
    assign o_ch_fifo_sel   = ch_fifo_sel    ;

    assign o_ch_r_max_burst= ch_r_max_burst ; 
    assign o_ch_r_en       = ch_r_en        ; 
    assign o_ch_r_ed_revr  = ch_r_ed_revr   ; 
    assign o_ch_r_trig_sel = ch_r_trig_sel  ; 
    assign o_ch_r_atyp     = ch_r_atyp      ; 
    assign o_ch_w_max_burst= ch_w_max_burst ; 
    assign o_ch_w_en       = ch_w_en        ;  
    assign o_ch_w_ed_revr  = ch_w_ed_revr   ;  
    assign o_ch_w_trig_sel = ch_w_trig_sel  ;  
    assign o_ch_w_atyp     = ch_w_atyp      ;  
    assign o_ch_w_lli_wb   = ch_w_lli_wb    ;
    assign o_ch_saddr      = ch_saddr       ;  
    assign o_ch_daddr      = ch_daddr       ;  
    assign o_ch_rlen       = ch_rlen        ;  
    assign o_ch_wlen       = ch_wlen        ;  
    assign o_ch_llp        = ch_llp         ;  

    assign o_ch_bus        = ch_bus         ;  

    assign o_ch_int        =|(ch_int_en&ch_int_sta);

endmodule
