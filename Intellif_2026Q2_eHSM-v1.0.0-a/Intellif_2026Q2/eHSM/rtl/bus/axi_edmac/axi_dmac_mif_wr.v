//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_mif_wr #(
    parameter [31:0] p_BUS_DWIDTH             =512,
    parameter [31:0] p_BUS_AWIDTH             = 64,
    parameter [31:0] p_R_BMEM_AWIDTH          =  8,
    parameter [31:0] p_W_BMEM_AWIDTH          =  8,
    parameter [31:0] p_R_SMEM_AWIDTH          =  3,
    parameter [31:0] p_W_SMEM_AWIDTH          =  3,
    parameter [31:0] p_R_FIFO_AWIDTH          =  3,
    parameter [31:0] p_W_FIFO_AWIDTH          =  3,
    parameter [31:0] p_CH_NUM                 =  8,
    parameter [ 0:0] p_W_ALIGN                =  0,
    parameter [ 0:0] p_R_ALIGN                =  0,
    parameter [31:0] p_W_ALIGN_STEP           =  8,
    parameter [31:0] p_R_ALIGN_STEP           =  8,
    parameter [ 0:0] p_TIMING_ENHANCE         =  0
)(

    input   wire                          i_clk             ,
    input   wire                          i_rst_n           ,
    input   wire [15:0]                   i_dma_to          ,
    input   wire [7:0]                    i_dma_wost_max    ,
    input   wire [7:0]                    i_dma_wburst_max  ,
    output  wire                          o_w_wato          ,
    output  wire                          o_w_wdto          ,
    output  wire                          o_w_bus_err       ,

    input   wire [p_BUS_AWIDTH+61:0]      i_w_abus_data     ,
    input   wire                          i_w_abus_vld      ,
    output  wire                          o_w_abus_rdy      ,

    input   wire [p_BUS_DWIDTH-1:0]       i_w_dbus_data     ,
    input   wire                          i_w_dbus_vld      ,
    output  wire                          o_w_dbus_rdy      ,

    output  wire [7:0]                    o_w_wbus_wid      ,
    output  wire                          o_w_wbus_vld      ,
    output  wire                          o_w_wbus_end      ,

    output  wire [7:0]                    o_w_bbus_bid      ,
    output  wire                          o_w_bbus_vld      ,
    output  wire [1:0]                    o_w_bbus_data     ,

    output  wire [ 1:0]                   o_w_awbar         ,
    output  wire [ 2:0]                   o_w_awsnoop       ,
    output  wire [ 1:0]                   o_w_awdomain      ,

    output  wire [13:0]                   o_w_awid          ,
    output  wire [p_BUS_AWIDTH-1:0]       o_w_awaddr        ,
    output  wire [ 7:0]                   o_w_awlen         ,
    output  wire [ 2:0]                   o_w_awsize        ,
    output  wire [ 1:0]                   o_w_awburst       ,
    output  wire [ 1:0]                   o_w_awlock        ,
    output  wire [ 3:0]                   o_w_awcache       ,
    output  wire [ 2:0]                   o_w_awprot        ,
    output  wire [ 3:0]                   o_w_awqos         ,
    output  wire [ 3:0]                   o_w_awregion      ,
    output  wire                          o_w_awvalid       ,
    input   wire                          i_w_awready       ,

    output  wire [ 7:0]                   o_w_wid           ,
    output  wire [p_BUS_DWIDTH-1:0]       o_w_wdata         ,
    output  wire [p_BUS_DWIDTH/8-1:0]     o_w_wstrb         ,
    output  wire                          o_w_wlast         ,
    output  wire                          o_w_wvalid        ,
    input   wire                          i_w_wready        ,

    input   wire [ 7:0]                   i_w_bid           ,
    input   wire [ 1:0]                   i_w_bresp         ,
    input   wire                          i_w_bvalid        ,
    output  wire                          o_w_bready        

);
    localparam [2:0]ADDR_OFFSET_WIDTH = (p_BUS_DWIDTH/8<=4  ) ? 3'd2 :    
                                        (p_BUS_DWIDTH/8<=8  ) ? 3'd3 :    
                                        (p_BUS_DWIDTH/8<=16 ) ? 3'd4 :    
                                        (p_BUS_DWIDTH/8<=32 ) ? 3'd5 :    
                                        (p_BUS_DWIDTH/8<=64 ) ? 3'd6 : 3'd0 ;

    localparam [31:0] WMEM_DWIDTH       = ADDR_OFFSET_WIDTH*2+32'd3+32'd8+32'd8 ;

    localparam [31:0] W_OST_MEM_NUM     = 2**p_W_BMEM_AWIDTH+32'b0;

    localparam [31:0] W_ALIGN_MUX_NUM   = p_BUS_DWIDTH/p_W_ALIGN_STEP;
    localparam [31:0] W_ALIGN_MUX_WIDTH = (W_ALIGN_MUX_NUM<= 2) ? 32'd1 :
                                          (W_ALIGN_MUX_NUM<= 4) ? 32'd2 :
                                          (W_ALIGN_MUX_NUM<= 8) ? 32'd3 :
                                          (W_ALIGN_MUX_NUM<=16) ? 32'd4 :
                                          (W_ALIGN_MUX_NUM<=32) ? 32'd5 :
                                          (W_ALIGN_MUX_NUM<=64) ? 32'd6 : 32'd0 ;

    localparam [ 1:0] WA_IDLE                = 2'd0;
    localparam [ 1:0] WA_PRE                 = 2'd1;
    localparam [ 1:0] WA_VALID               = 2'd2;
    localparam [ 1:0] WA_SUSPEND             = 2'd3;

    localparam [ 1:0] WD_IDLE                = 2'd0;
    localparam [ 1:0] WD_TRANS               = 2'd1;
    localparam [ 1:0] WD_LAST                = 2'd2;
    localparam [ 1:0] WD_SUSPEND             = 2'd3;

    wire                             fifo_wr                   ; 
    wire [p_BUS_AWIDTH+61:0]         fifo_wdata                ;
    wire                             fifo_full_n               ; 

    wire                             fifo_rd                   ; 
    wire [p_BUS_AWIDTH+61:0]         fifo_rdata                ;
    wire                             fifo_empty_n              ;

    reg                              fifo_dst_vld              ;
    wire                             fifo_dst_vld_in           ;
    wire                             fifo_dst_rdy              ;

    wire [p_BUS_AWIDTH-1:0]          ch_waddr                  ;
    wire [15:0]                      ch_strans_wlen            ;
    wire [7:0]                       ch_wid                    ;
    wire [31:0]                      ch_awbus                  ;
    wire [5:0]                       ch_w_qid                  ;

    reg  [15:0]                      w_strans_len              ;
    wire [15:0]                      w_strans_len_in           ;
    wire [15:0]                      w_strans_len_ns           ;

    reg                              w_strans_first            ;
    wire                             w_strans_first_in         ;
    wire                             w_strans_last             ;
    wire                             w_strans_rerd             ;
    reg  [ADDR_OFFSET_WIDTH-1:0]     w_strans_end_addr         ;
    wire [ADDR_OFFSET_WIDTH-1:0]     w_strans_end_addr_in      ;
    reg  [ADDR_OFFSET_WIDTH-1:0]     w_strans_offset           ;
    wire [ADDR_OFFSET_WIDTH-1:0]     w_strans_offset_in        ;

    wire [7:0]                       w_awlen_out               ;
    wire [7:0]                       w_awid_out                ;
    wire                             w_strans_first_out        ;
    wire                             w_strans_last_out         ;
    wire [ADDR_OFFSET_WIDTH-1:0]     w_strans_end_addr_out     ;
    wire [ADDR_OFFSET_WIDTH-1:0]     w_strans_offset_out       ;
    wire                             w_strans_rerd_out         ;

    wire                             dbus_fifo_wr              ;
    wire [p_BUS_DWIDTH-1:0]          dbus_fifo_wdata           ;
    wire                             dbus_fifo_full_n          ; 

    wire                             dbus_fifo_rd              ;
    wire [p_BUS_DWIDTH-1:0]          dbus_fifo_rdata           ;
    wire                             dbus_fifo_empty_n         ;

    reg                              dbus_dst_vld              ;
    wire                             dbus_dst_vld_in           ;
    wire                             dbus_dst_rdy              ;

    wire                             ost_fifo_wr               ;
    wire [WMEM_DWIDTH-1:0]           ost_fifo_wdata            ;
    reg  [WMEM_DWIDTH-1:0]           ost_fifo_wdata_pre        ;
    wire [WMEM_DWIDTH-1:0]           ost_fifo_wdata_pre_in     ;
    wire                             ost_fifo_wdata_latch      ;

    wire                             ost_fifo_rd               ;
    wire [WMEM_DWIDTH-1:0]           ost_fifo_rdata            ;
    wire                             ost_fifo_empty_n          ;

    reg                              ost_dst_vld               ;
    wire                             ost_dst_vld_in            ;
    wire                             ost_dst_rdy               ;

    reg                              dbus_fifo_cnt             ;
    reg  [7:0]                       w_burst_beat_cnt          ;
    wire [7:0]                       w_burst_beat_cnt_in       ;
    reg  [7:0]                       w_strans_burst_cnt        ;
    wire [7:0]                       w_strans_burst_cnt_in     ;

    reg  [7:0]                       w_bbus_bid                ; 
    reg                              w_bbus_vld                ; 
    reg  [1:0]                       w_bbus_data               ; 

    wire [7:0]                       w_bbus_bid_in             ; 
    wire                             w_bbus_vld_in             ; 
    wire [1:0]                       w_bbus_data_in            ; 

    reg  [7:0]                       w_wbus_wid                ; 
    reg                              w_wbus_vld                ;
    reg                              w_wbus_end                ; 

    wire [7:0]                       w_wbus_wid_in             ; 
    wire                             w_wbus_vld_in             ;
    wire                             w_wbus_end_in             ; 

    genvar                           i;
    genvar                           j;
    integer                          k;

    reg  [1:0]                       wa_cs;
    reg  [1:0]                       wa_ns;

    reg  [1:0]                       wd_cs;
    reg  [1:0]                       wd_ns;

    reg  [p_W_BMEM_AWIDTH:0]         w_ost_cnt                 ;
    wire [p_W_BMEM_AWIDTH:0]         w_ost_cnt_in              ;

    wire                             w_ost_cnt_dec             ;
    wire                             w_ost_cnt_inc             ;

    wire                             w_ost_full                ;
    wire                             w_ost_al_full             ;

    wire                             w_ost_mem_full_n          ;
    wire                             w_ost_mem_al_full         ;
    wire                             w_ost_fifo_vld            ;

    wire [12:0]                      w_burst_bond_dist_pre     ;
    wire [12-ADDR_OFFSET_WIDTH:0]    w_burst_bond_dist         ;
    wire [8:0]                       w_burst_max               ;
    wire [8:0]                       w_burst_len_small         ;

    wire [16:0]                      w_strans_burst_range      ;
    wire [16:0]                      w_strans_burst_range_dec  ;

    wire [16-ADDR_OFFSET_WIDTH:0]    w_strans_burst_len        ;

    wire                             w_burst_cover             ;
    wire [8:0]                       w_burst_len               ;

    wire [15:0]                      w_burst_len_byte_uncover  ;
    wire [15:0]                      w_burst_len_byte          ;

    reg  [p_BUS_DWIDTH-1:0]          w_wr_align_buf            ;
    wire [p_BUS_DWIDTH-1:0]          w_wr_align_buf_in         ;
    wire                             w_wr_align_buf_upd        ;

    wire [p_BUS_DWIDTH-1:0]          w_wr_align_mux [W_ALIGN_MUX_NUM-1:0];
    wire [p_BUS_DWIDTH-1:0]          w_wr_align_in             ;
    wire [p_BUS_DWIDTH-1:0]          w_wr_align_out            ;
    wire [ADDR_OFFSET_WIDTH-1:0]     w_wr_align_sel            ;

    wire [ADDR_OFFSET_WIDTH-1:0]     w_wr_bus_start            ;
    wire [ADDR_OFFSET_WIDTH-1:0]     w_wr_bus_end              ;
    wire [p_BUS_DWIDTH/8-1:0]        w_wr_bus_byte_sel         ;

    reg  [p_BUS_AWIDTH-1:0]          w_awaddr_pre              ; 
    wire [p_BUS_AWIDTH-1:0]          w_awaddr_pre_in           ;
    wire [p_BUS_AWIDTH-1:0]          w_awaddr_pre_ns           ;

    wire [ADDR_OFFSET_WIDTH-1:0]     w_awaddr_offset_dec       ;

    reg  [ 1:0]                      w_awbar                   ; 
    reg  [ 2:0]                      w_awsnoop                 ; 
    reg  [ 1:0]                      w_awdomain                ; 
    reg  [13:0]                      w_awid                    ; 
    reg  [p_BUS_AWIDTH-1:0]          w_awaddr                  ; 
    reg  [ 7:0]                      w_awlen                   ; 
    wire [ 2:0]                      w_awsize                  ; 
    wire [ 1:0]                      w_awburst                 ; 
    reg  [ 1:0]                      w_awlock                  ; 
    reg  [ 3:0]                      w_awcache                 ; 
    reg  [ 2:0]                      w_awprot                  ; 
    reg  [ 3:0]                      w_awqos                   ; 
    reg  [ 3:0]                      w_awregion                ; 
    reg                              w_awvalid                 ; 

    wire [ 1:0]                      w_awbar_in                ; 
    wire [ 2:0]                      w_awsnoop_in              ; 
    wire [ 1:0]                      w_awdomain_in             ; 
    wire [13:0]                      w_awid_in                 ; 
    wire [p_BUS_AWIDTH-1:0]          w_awaddr_in               ;

    wire [ 7:0]                      w_awlen_in                ;
    wire [ 8:0]                      w_awlen_post              ; 

    wire [ 1:0]                      w_awlock_in               ; 
    wire [ 3:0]                      w_awcache_in              ; 
    wire [ 2:0]                      w_awprot_in               ; 
    wire [ 3:0]                      w_awqos_in                ; 
    wire [ 3:0]                      w_awregion_in             ; 
    wire                             w_awvalid_in              ; 

    reg  [ 7:0]                      w_wid                     ;
    reg  [p_BUS_DWIDTH-1:0]          w_wdata                   ; 
    reg  [p_BUS_DWIDTH/8-1:0]        w_wstrb                   ; 
    reg                              w_wlast                   ; 
    reg                              w_wvalid                  ;

    wire [ 7:0]                      w_wid_in                  ;                                                                                                                                                      
    wire [p_BUS_DWIDTH-1:0]          w_wdata_in                ; 
    wire [p_BUS_DWIDTH/8-1:0]        w_wstrb_in                ; 
    wire                             w_wlast_in                ; 
    wire                             w_wvalid_in               ;

    reg                              w_wend                    ;
    wire                             w_wend_in                 ;

    wire                             wa_to                     ;
    reg [15:0]                       wa_to_cnt                 ;
    wire[15:0]                       wa_to_cnt_in              ;
    wire                             wd_to                     ;
    reg [15:0]                       wd_to_cnt                 ;
    wire[15:0]                       wd_to_cnt_in              ;

    wire [ 7:0]                      w_dma_ost_max_small       ;
    wire [ 7:0]                      w_dma_ost_max_small_inc   ;

    assign fifo_wr = i_w_abus_vld&(fifo_full_n);

    assign fifo_wdata = i_w_abus_data ;

    assign o_w_abus_rdy = (fifo_full_n) ;

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (1),
        .p_DWIDTH       (p_BUS_AWIDTH+62   )
    ) u_w_abus_fifo(
        .i_clk          (i_clk             ),
        .i_rst_n        (i_rst_n           ),    

        .i_wr           (fifo_wr           ),
        .i_wr_data      (fifo_wdata        ),
        .o_wr_num_count (                  ),
        .o_wr_full      (                  ),
        .o_wr_full_n    (fifo_full_n       ),
        .o_wr_al_full   (                  ),
        .o_wr_al_full_n (                  ),

        .i_rd           (fifo_rd           ),
        .o_rd_data      (fifo_rdata        ),
        .o_rd_num_count (                  ),
        .o_rd_empty     (                  ),
        .o_rd_empty_n   (fifo_empty_n      ),
        .o_rd_al_empty  (                  ),
        .o_rd_al_empty_n(                  ),

        .i_clear        (1'b0              )
    ); 

    assign fifo_rd = (fifo_empty_n)&((!fifo_dst_vld)|(fifo_dst_vld&fifo_dst_rdy));

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            fifo_dst_vld <=  1'b0 ;
        end
        else begin
            fifo_dst_vld <=  fifo_dst_vld_in ;
        end
    end

    assign fifo_dst_vld_in = (fifo_rd)    ? 1'b1 :
                             fifo_dst_rdy ? 1'b0 : fifo_dst_vld ;

    assign fifo_dst_rdy  = (wa_cs==WA_VALID)&(w_strans_len==0)&i_w_awready ;

    assign ch_waddr      = fifo_rdata[p_BUS_AWIDTH-1:0];
    assign ch_strans_wlen= fifo_rdata[p_BUS_AWIDTH+:16];
    assign ch_wid        = fifo_rdata[(p_BUS_AWIDTH+16)+:8];
    assign ch_awbus      = fifo_rdata[(p_BUS_AWIDTH+24)+:32];
    assign ch_w_qid      = fifo_rdata[(p_BUS_AWIDTH+56)+:6 ];

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            wa_to_cnt <= 16'b0 ;
            wd_to_cnt <= 16'b0 ;
        end
        else begin
            wa_to_cnt <=   wa_to_cnt_in ;
            wd_to_cnt <=   wd_to_cnt_in ;
        end
    end

    assign wa_to_cnt_in = (wa_cs==WA_VALID)&i_w_awready    ? 16'b0          : 
                          (wa_cs==WA_VALID)&(!i_w_awready) ? wa_to_cnt+1'b1 : wa_to_cnt ;

    assign wa_to        = (wa_to_cnt==i_dma_to)&(i_dma_to!=0);

    assign wd_to_cnt_in = (wd_cs!=WD_IDLE)&(wd_cs!=WD_SUSPEND)&i_w_wready    ? 16'b0          : 
                          (wd_cs!=WD_IDLE)&(wd_cs!=WD_SUSPEND)&(!i_w_wready) ? wd_to_cnt+1'b1 : wd_to_cnt ;

    assign wd_to        = (wd_to_cnt==i_dma_to)&(i_dma_to!=0);

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            wa_cs <= WA_IDLE;
        end
        else begin
            wa_cs <=  wa_ns ;
        end
    end

    always@(*)
    begin
        wa_ns=wa_cs;
        case(wa_cs)
            WA_IDLE    : begin
                wa_ns = (fifo_dst_vld) ? WA_PRE : WA_IDLE ;
            end
            WA_PRE     : begin
                wa_ns = (w_ost_fifo_vld) ? WA_VALID : WA_PRE ;
            end
            WA_VALID   : begin
                wa_ns = (w_strans_len==0)&i_w_awready                ? WA_IDLE    : 
                        (w_ost_al_full)&(~w_ost_cnt_dec)&i_w_awready ? WA_SUSPEND : WA_VALID ;
            end
            WA_SUSPEND : begin
                wa_ns = (w_ost_cnt_dec) ? WA_VALID : WA_SUSPEND ;
            end
            default:wa_ns=WA_IDLE;
        endcase
    end

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            w_awbar      <=  2'b0;
            w_awsnoop    <=  3'b0;
            w_awdomain   <=  2'b0;
            w_awid       <= 14'b0;
            w_awaddr     <=  {(p_BUS_AWIDTH){1'b0}};
            w_awlen      <=  8'b0; 

            w_awlock     <=  2'b0; 
            w_awcache    <=  4'b0; 
            w_awprot     <=  3'b0; 
            w_awqos      <=  4'b0; 
            w_awregion   <=  4'b0; 
            w_awvalid    <=  1'b0;

            w_awaddr_pre <= {(p_BUS_AWIDTH){1'b0}};
            w_strans_len <= 16'b0; 
        end
        else begin
            w_awbar      <=  w_awbar_in   ;
            w_awsnoop    <=  w_awsnoop_in ;
            w_awdomain   <=  w_awdomain_in;
            w_awid       <=  w_awid_in    ;
            w_awaddr     <=  w_awaddr_in  ;
            w_awlen      <=  w_awlen_in   ; 

            w_awlock     <=  w_awlock_in  ; 
            w_awcache    <=  w_awcache_in ; 
            w_awprot     <=  w_awprot_in  ; 
            w_awqos      <=  w_awqos_in   ; 
            w_awregion   <=  w_awregion_in; 
            w_awvalid    <=  w_awvalid_in ;

            w_awaddr_pre <=  w_awaddr_pre_in ;
            w_strans_len <=  w_strans_len_in ; 
        end
    end

    assign w_awid_in     =  {ch_w_qid,ch_wid} ;
    assign w_awaddr_in   =  ((wa_cs==WA_PRE)&w_ost_fifo_vld)|                                               
                            ((wa_cs==WA_VALID)&i_w_awready)   ? w_awaddr_pre : w_awaddr ;

    assign w_awlen_post  =  w_burst_len-1'b1 ;

    assign w_awlen_in    =  (wa_cs==WA_VALID)&(w_strans_len==0)&i_w_awready ? 8'b0              :
                            ((wa_cs==WA_PRE)&w_ost_fifo_vld)|                 
                            ((wa_cs==WA_VALID)&i_w_awready)                 ? w_awlen_post[7:0] : w_awlen ; 

    assign w_awsize      =  ADDR_OFFSET_WIDTH;
    assign w_awburst     =  2'b01;

    assign w_awsnoop_in  =  ch_awbus[30:28];

    assign w_awbar_in    =  ch_awbus[27:26]; 
    assign w_awdomain_in =  ch_awbus[25:24];

    assign w_awlock_in   =  ch_awbus[17:16]; 

    assign w_awcache_in  =  ch_awbus[15:12]; 
    assign w_awprot_in   =  ch_awbus[10: 8];

    assign w_awqos_in    =  ch_awbus[ 7: 4];
    assign w_awregion_in =  ch_awbus[ 3: 0];

    assign w_awvalid_in  =  ((wa_cs==WA_PRE)&w_ost_fifo_vld)|                     
                            ((wa_cs==WA_SUSPEND)&w_ost_cnt_dec)    ? 1'b1 :
                            ((wa_cs==WA_VALID)&i_w_awready&
                             ((w_strans_len==0)|
                              ((w_ost_al_full)&(!w_ost_cnt_dec)))) ? 1'b0 : w_awvalid ;

    assign w_strans_len_in = ((wa_cs==WA_IDLE)&fifo_dst_vld)                     ? ch_strans_wlen  :
                             (((wa_cs==WA_PRE)&w_ost_fifo_vld)|
                              ((wa_cs==WA_VALID)&i_w_awready))&(w_strans_len!=0) ? w_strans_len_ns : w_strans_len ;

    assign w_strans_len_ns = w_strans_len-w_burst_len_byte ;

    assign w_awaddr_pre_in = ((wa_cs==WA_IDLE)&fifo_dst_vld)                   ? ch_waddr :
                             ((wa_cs==WA_PRE)&w_ost_fifo_vld)|
                             ((wa_cs==WA_VALID)&i_w_awready)&(w_strans_len!=0) ? w_awaddr_pre_ns : w_awaddr_pre ;

    assign w_awaddr_pre_ns = w_awaddr_pre+{{(p_BUS_AWIDTH-16){1'b0}},w_burst_len_byte};

    assign w_burst_bond_dist           = ({1'b1,{(12-ADDR_OFFSET_WIDTH){1'b0}}}-{1'b0,w_awaddr_pre[11:ADDR_OFFSET_WIDTH]}) ;
    assign w_burst_bond_dist_pre       = {{(ADDR_OFFSET_WIDTH){1'b0}},w_burst_bond_dist};

    assign w_burst_max                 = i_dma_wburst_max+9'b1;
    assign w_burst_len_small           = ({4'b0,w_burst_max}>w_burst_bond_dist_pre) ? w_burst_bond_dist_pre[8:0] : w_burst_max ;

    assign w_strans_burst_range        = ({1'b0,w_strans_len}+{{(17-ADDR_OFFSET_WIDTH){1'b0}},w_awaddr_pre[ADDR_OFFSET_WIDTH-1:0]});

wire                          w_strans_burst_range_inc_tmp = (|w_strans_burst_range[ADDR_OFFSET_WIDTH-1:0]);
wire [16-ADDR_OFFSET_WIDTH:0] w_strans_burst_range_add_tmp = {w_strans_burst_range[16:ADDR_OFFSET_WIDTH]}+{{(16-ADDR_OFFSET_WIDTH){1'b0}},w_strans_burst_range_inc_tmp};
    assign w_strans_burst_len          = (w_strans_len==0) ? {(17-ADDR_OFFSET_WIDTH){1'b0}}  : w_strans_burst_range_add_tmp;

    assign w_burst_cover               = (w_strans_burst_len<={{{(8-ADDR_OFFSET_WIDTH){1'b0}}},w_burst_len_small}) ;
    assign w_burst_len                 = (w_burst_cover) ? w_strans_burst_len[8:0] : w_burst_len_small ;

    assign w_burst_len_byte_uncover    = ({{(7-ADDR_OFFSET_WIDTH){1'b0}},w_burst_len,{(ADDR_OFFSET_WIDTH){1'b0}}}-{{(16-ADDR_OFFSET_WIDTH){1'b0}},w_awaddr_pre[ADDR_OFFSET_WIDTH-1:0]});
    assign w_burst_len_byte            = (w_burst_cover) ? w_strans_len : w_burst_len_byte_uncover ;

    assign w_awaddr_offset_dec         = w_awaddr_pre[ADDR_OFFSET_WIDTH-1:0]-{{(ADDR_OFFSET_WIDTH-1){1'b0}},1'b1};
    assign w_strans_burst_range_dec    = w_strans_len+{{(17-ADDR_OFFSET_WIDTH){1'b0}},w_awaddr_offset_dec};

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            w_strans_end_addr <= {(ADDR_OFFSET_WIDTH){1'b0}};
            w_strans_offset   <= {(ADDR_OFFSET_WIDTH){1'b0}};
        end
        else begin
            w_strans_end_addr <=  w_strans_end_addr_in ;
            w_strans_offset   <=  w_strans_offset_in ;
        end
    end

    assign w_strans_end_addr_in = ((wa_cs==WA_PRE)&w_ost_fifo_vld) ? w_strans_burst_range_dec[ADDR_OFFSET_WIDTH-1:0] : w_strans_end_addr ;
    assign w_strans_offset_in   = ((wa_cs==WA_PRE)&w_ost_fifo_vld) ? w_awaddr_pre[ADDR_OFFSET_WIDTH-1:0] : w_strans_offset ;

    assign w_strans_rerd        = w_strans_burst_range_inc_tmp ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            w_ost_cnt <= {(p_W_BMEM_AWIDTH+1){1'b0}};
        end
        else begin
            w_ost_cnt <=  w_ost_cnt_in ;
        end
    end

    assign w_ost_cnt_in     = w_ost_cnt_inc&w_ost_cnt_dec ? w_ost_cnt      :
                              w_ost_cnt_inc               ? w_ost_cnt+1'b1 :
                              w_ost_cnt_dec               ? w_ost_cnt-1'b1 : w_ost_cnt ;

    assign w_ost_cnt_inc    = ((wa_cs==WA_VALID)&i_w_awready) ;

    assign w_ost_cnt_dec    = i_w_bvalid ;

    assign w_ost_full       = (w_ost_cnt>=w_dma_ost_max_small_inc[p_W_BMEM_AWIDTH:0]) ;

    assign w_ost_al_full    = (w_ost_cnt>=w_dma_ost_max_small[p_W_BMEM_AWIDTH:0])|w_ost_mem_al_full;

    assign w_ost_fifo_vld   = (~w_ost_full)&w_ost_mem_full_n ;

    assign w_dma_ost_max_small     = (i_dma_wost_max>{{(8-p_W_BMEM_AWIDTH){1'b0}},{(p_W_BMEM_AWIDTH){1'b1}}}) 
                                     ? {{(8-p_W_BMEM_AWIDTH){1'b0}},{(p_W_BMEM_AWIDTH){1'b1}}} : i_dma_wost_max ;

    assign w_dma_ost_max_small_inc = w_dma_ost_max_small+8'b1 ;

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (p_W_BMEM_AWIDTH     ),
        .p_DWIDTH       (WMEM_DWIDTH         )
    ) u_w_ost_fifo(
        .i_clk          (i_clk               ),
        .i_rst_n        (i_rst_n             ),    

        .i_wr           (ost_fifo_wr         ),
        .i_wr_data      (ost_fifo_wdata      ),
        .o_wr_num_count (                    ),
        .o_wr_full      (                    ),
        .o_wr_full_n    (w_ost_mem_full_n    ),
        .o_wr_al_full   (w_ost_mem_al_full   ),
        .o_wr_al_full_n (                    ),

        .i_rd           (ost_fifo_rd         ),
        .o_rd_data      (ost_fifo_rdata      ),
        .o_rd_num_count (                    ),
        .o_rd_empty     (                    ),
        .o_rd_empty_n   (ost_fifo_empty_n    ),
        .o_rd_al_empty  (                    ),
        .o_rd_al_empty_n(                    ),

        .i_clear        (1'b0                )
    ); 

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            w_strans_first     <= 1'b0 ;
            ost_fifo_wdata_pre <= {(WMEM_DWIDTH){1'b0}};
        end
        else begin
            w_strans_first     <=  w_strans_first_in ;
            ost_fifo_wdata_pre <=  ost_fifo_wdata_pre_in ;
        end
    end

    assign w_strans_first_in       = (wa_cs==WA_IDLE)&fifo_dst_vld     ? 1'b1 :
                                     ((wa_cs==WA_PRE)&w_ost_fifo_vld)  ? 1'b0 : w_strans_first ;

    assign w_strans_last           = ost_fifo_wdata_latch&(w_burst_cover) ;

    assign ost_fifo_wdata_latch    = ((wa_cs==WA_PRE)&w_ost_fifo_vld)|
                                     ((wa_cs==WA_VALID)&i_w_awready)&(w_strans_len!=0) ;

    assign ost_fifo_wdata_pre_in   = ost_fifo_wdata_latch ? {w_strans_end_addr_in,w_strans_offset_in,w_strans_rerd,w_strans_first,w_strans_last,w_awlen_post[7:0],ch_wid} : ost_fifo_wdata_pre ;

    assign ost_fifo_wr             = w_ost_cnt_inc ;
    assign ost_fifo_wdata          = ost_fifo_wdata_pre;

    assign ost_fifo_rd             = (ost_fifo_empty_n)&((!ost_dst_vld)|(ost_dst_vld&ost_dst_rdy));

    assign {w_strans_end_addr_out,w_strans_offset_out,w_strans_rerd_out,w_strans_first_out,w_strans_last_out,w_awlen_out,w_awid_out} = ost_fifo_rdata ;

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            ost_dst_vld <= 1'b0 ;
        end
        else begin
            ost_dst_vld <=  ost_dst_vld_in ;
        end
    end

    assign ost_dst_vld_in = (ost_fifo_rd) ? 1'b1 :
                             ost_dst_rdy  ? 1'b0 : ost_dst_vld ;

    assign ost_dst_rdy    = ((wd_cs==WD_IDLE)&ost_dst_vld&(w_awlen_out==0)&dbus_dst_vld)|
                            ((wd_cs==WD_TRANS)&(i_w_wready)&(dbus_dst_vld)&(w_burst_beat_cnt==w_awlen_out))|
                            ((wd_cs==WD_LAST)&i_w_wready&ost_dst_vld&(w_awlen_out==0)&dbus_dst_vld)|
                            ((wd_cs==WD_SUSPEND)&(dbus_dst_vld)&(w_burst_beat_cnt==w_awlen_out)) ;

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (1                  ),
        .p_DWIDTH       (p_BUS_DWIDTH       )
    ) u_w_dbus_fifo(
        .i_clk          (i_clk              ),
        .i_rst_n        (i_rst_n            ),    

        .i_wr           (dbus_fifo_wr       ),
        .i_wr_data      (dbus_fifo_wdata    ),
        .o_wr_num_count (                   ),
        .o_wr_full      (                   ),
        .o_wr_full_n    (dbus_fifo_full_n   ),
        .o_wr_al_full   (                   ),
        .o_wr_al_full_n (                   ),

        .i_rd           (dbus_fifo_rd       ),
        .o_rd_data      (dbus_fifo_rdata    ),
        .o_rd_num_count (                   ),
        .o_rd_empty     (                   ),
        .o_rd_empty_n   (dbus_fifo_empty_n  ),
        .o_rd_al_empty  (                   ),
        .o_rd_al_empty_n(                   ),

        .i_clear        (1'b0               )
    ); 

    assign dbus_fifo_wdata = i_w_dbus_data ;

    assign o_w_dbus_rdy    = (dbus_fifo_full_n) ;

    assign dbus_fifo_wr    = (i_w_dbus_vld&dbus_fifo_full_n) ;

    assign dbus_fifo_rd    = (dbus_fifo_empty_n)&((!dbus_dst_vld)|(dbus_dst_vld&dbus_dst_rdy));

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            dbus_dst_vld       <= 1'b0 ;
            w_burst_beat_cnt   <= 8'b0 ;
            w_strans_burst_cnt <= 8'b0 ;
        end
        else begin
            dbus_dst_vld       <=  dbus_dst_vld_in ;
            w_burst_beat_cnt   <=  w_burst_beat_cnt_in ;
            w_strans_burst_cnt <=  w_strans_burst_cnt_in;
        end
    end

    assign dbus_dst_vld_in        = (dbus_fifo_rd) ? 1'b1 :
                                     dbus_dst_rdy  ? 1'b0 : dbus_dst_vld ;

    assign dbus_dst_rdy           =  ((wd_cs==WD_IDLE)&ost_dst_vld&dbus_dst_vld)|
                                     ((wd_cs==WD_TRANS)&(i_w_wready)&(dbus_dst_vld))|
                                     ((wd_cs==WD_LAST)&i_w_wready&ost_dst_vld&dbus_dst_vld)|
                                     ((wd_cs==WD_SUSPEND)&dbus_dst_vld) ;

    assign w_burst_beat_cnt_in    = ((wd_cs==WD_IDLE)&ost_dst_vld&dbus_dst_vld)|
                                    ((wd_cs==WD_LAST)&i_w_wready&ost_dst_vld&dbus_dst_vld)  ? 8'b1               :
                                    ((wd_cs==WD_TRANS)&dbus_dst_vld&i_w_wready)|
                                    ((wd_cs==WD_SUSPEND)&dbus_dst_vld)                      ? w_burst_beat_cnt+1'b1 : w_burst_beat_cnt ;

    assign w_strans_burst_cnt_in  = (wd_cs==WD_IDLE)&dbus_dst_vld&w_strans_first_out ? 8'b0                 :
                                    (wd_cs==WD_TRANS)&i_w_wready                     ? w_strans_burst_cnt+1'b1 : w_strans_burst_cnt ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            wd_cs <= WD_IDLE;
        end
        else begin
            wd_cs <=   wd_ns ;
        end
    end

    always@(*)
    begin
        wd_ns=wd_cs;
        case(wd_cs)
            WD_IDLE : begin
                wd_ns = ost_dst_vld&(w_awlen_out==0)&dbus_dst_vld ? WD_LAST  :
                        ost_dst_vld&dbus_dst_vld                  ? WD_TRANS : WD_IDLE ;
            end
            WD_TRANS : begin
                wd_ns = (i_w_wready)&(!dbus_dst_vld)                 ? WD_SUSPEND  :
                        (i_w_wready)&(w_burst_beat_cnt==w_awlen_out) ? WD_LAST     : WD_TRANS ;
            end
            WD_LAST : begin
                wd_ns = !i_w_wready                               ? WD_LAST  :
                        ost_dst_vld&(w_awlen_out==0)&dbus_dst_vld ? WD_LAST  :          
                        ost_dst_vld&dbus_dst_vld                  ? WD_TRANS : WD_IDLE ;
            end
            WD_SUSPEND : begin
                wd_ns = (dbus_dst_vld)&(w_burst_beat_cnt==w_awlen_out) ? WD_LAST    :
                        (dbus_dst_vld)                                 ? WD_TRANS   : WD_SUSPEND ;
            end
            default:wd_ns=WD_IDLE;
        endcase
    end

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            w_wid    <= 8'b0 ;
            w_wdata  <= {(p_BUS_DWIDTH){1'b0}}  ; 
            w_wstrb  <= {(p_BUS_DWIDTH/8){1'b0}}; 
            w_wlast  <= 1'b0 ; 
            w_wvalid <= 1'b0 ;

            w_wend   <= 1'b0 ; 
        end
        else begin
            w_wid    <=   w_wid_in   ;
            w_wdata  <=   w_wdata_in ;
            w_wstrb  <=   w_wstrb_in ;
            w_wlast  <=   w_wlast_in ;
            w_wvalid <=   w_wvalid_in;

            w_wend   <=   w_wend_in  ;
        end
    end

    assign w_wid_in    = ((wd_cs==WD_IDLE)&ost_dst_vld&dbus_dst_vld)|
                         ((wd_cs==WD_LAST)&i_w_wready&ost_dst_vld&dbus_dst_vld) ? w_awid_out : w_wid ;

    assign w_wdata_in  = dbus_dst_rdy ? w_wr_align_out : w_wdata ;

    assign w_wstrb_in  = dbus_dst_rdy ? w_wr_bus_byte_sel : w_wstrb ;

    assign w_wlast_in  = ost_dst_rdy                                                                  ? 1'b1 :
                         ((wd_cs==WD_LAST)&i_w_wready&(!(ost_dst_vld&(w_awlen_out==0)&dbus_dst_vld))) ? 1'b0 : w_wlast ;

    assign w_wvalid_in = ((wd_cs==WD_IDLE)&ost_dst_vld&dbus_dst_vld)|
                         ((wd_cs==WD_SUSPEND)&dbus_dst_vld)                             ? 1'b1 :
                         ((wd_cs==WD_TRANS)&(i_w_wready)&(!dbus_dst_vld))|
                         ((wd_cs==WD_LAST)&(i_w_wready)&(!(ost_dst_vld&dbus_dst_vld)))  ? 1'b0 : w_wvalid ;  

    assign w_wend_in   = ost_dst_rdy&w_strans_last_out                                                ? 1'b1 :
                         (ost_dst_rdy&(!w_strans_last_out))|
                         ((wd_cs==WD_LAST)&i_w_wready&(!(ost_dst_vld&(w_awlen_out==0)&dbus_dst_vld))) ? 1'b0 : w_wend ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            w_wr_align_buf <= {(p_BUS_DWIDTH){1'b0}};
        end
        else begin
            w_wr_align_buf <=   w_wr_align_buf_in;
        end
    end

    assign w_wr_align_buf_upd = (dbus_dst_vld&dbus_dst_rdy) ;

    assign w_wr_align_buf_in  = w_wr_align_buf_upd ? dbus_fifo_rdata : w_wr_align_buf ;

    assign w_wr_align_sel     = (p_W_ALIGN) ?  {(W_ALIGN_MUX_WIDTH){1'b0}} : w_strans_offset_out ;

    assign w_wr_align_in      = dbus_fifo_rdata ;

    assign w_wr_bus_start     = (((wd_cs==WD_IDLE)&ost_dst_vld&dbus_dst_vld)|
                                 ((wd_cs==WD_LAST)&i_w_wready&ost_dst_vld&dbus_dst_vld))&w_strans_first_out ? (w_strans_offset_out) : {(ADDR_OFFSET_WIDTH){1'b0}} ;

    assign w_wr_bus_end       = ost_dst_rdy&w_strans_last_out ? (w_strans_end_addr_out) : {(ADDR_OFFSET_WIDTH){1'b1}} ;

    generate
    if(1) begin:wdata_byte_sel_logic_gen
        for(i=0;i<p_BUS_DWIDTH/8;i=i+1) begin:wdata_byte_sel_logic
            assign w_wr_bus_byte_sel[i]=(w_wr_bus_start<=i)&(w_wr_bus_end>=i);
        end
    end
    endgenerate

   assign w_wr_align_mux[0]={ w_wr_align_in };

    generate
    if(p_W_ALIGN) begin:wdata_align_sel_logic_gen
        for(i=1;i<W_ALIGN_MUX_NUM;i=i+1) begin:wdata_align_sel_logic
            assign w_wr_align_mux[i]={ w_wr_align_in};
        end
    end
    else begin:rdata_align_mux_bypass_gen

        for(i=1;i<W_ALIGN_MUX_NUM;i=i+1) begin:wdata_align_sel_logic
            assign w_wr_align_mux[i]={ w_wr_align_in[p_BUS_DWIDTH-p_W_ALIGN_STEP*i-1:0],w_wr_align_buf[p_BUS_DWIDTH-1:p_BUS_DWIDTH-i*p_W_ALIGN_STEP]};
        end
    end
    endgenerate

    assign w_wr_align_out=w_wr_align_mux[w_wr_align_sel];

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            w_bbus_vld  <= 1'b0 ; 
            w_bbus_bid  <= 8'b0 ; 
            w_bbus_data <= 2'b0 ;
            w_wbus_vld  <= 1'b0 ; 
            w_wbus_wid  <= 8'b0 ;
            w_wbus_end  <= 1'b0 ; 
        end
        else begin
            w_bbus_vld  <=  w_bbus_vld_in ; 
            w_bbus_bid  <=  w_bbus_bid_in ; 
            w_bbus_data <=  w_bbus_data_in;
            w_wbus_vld  <=  w_wbus_vld_in ; 
            w_wbus_wid  <=  w_wbus_wid_in ;
            w_wbus_end  <=  w_wbus_end_in ;
        end
    end

    assign w_bbus_vld_in  = (i_w_bvalid); 
    assign w_bbus_bid_in  = i_w_bid     ;
    assign w_bbus_data_in = i_w_bresp   ;
    assign w_wbus_vld_in  = w_wlast&i_w_wready ; 
    assign w_wbus_wid_in  = w_wid ;
    assign w_wbus_end_in  = w_wend&i_w_wready  ; 

    assign o_w_awbar             = w_awbar    ; 
    assign o_w_awsnoop           = w_awsnoop  ; 
    assign o_w_awdomain          = w_awdomain ; 

    assign o_w_awid              = w_awid     ;  
    assign o_w_awaddr            = w_awaddr   ;  
    assign o_w_awlen             = w_awlen    ;  
    assign o_w_awsize            = w_awsize   ;  
    assign o_w_awburst           = w_awburst  ;  
    assign o_w_awlock            = w_awlock   ;  
    assign o_w_awcache           = w_awcache  ;  
    assign o_w_awprot            = w_awprot   ;  
    assign o_w_awqos             = w_awqos    ;  
    assign o_w_awregion          = w_awregion ;  
    assign o_w_awvalid           = w_awvalid  ;  

    assign o_w_wid               = w_wid      ; 
    assign o_w_wdata             = w_wdata    ; 
    assign o_w_wstrb             = w_wstrb    ; 
    assign o_w_wlast             = w_wlast    ; 
    assign o_w_wvalid            = w_wvalid   ; 

    assign o_w_bready            = 1'b1       ;

    assign o_w_wato              = wa_to      ;
    assign o_w_wdto              = wd_to      ;
    assign o_w_bus_err           = (w_bbus_data[1])&(w_bbus_vld);

    assign o_w_wbus_wid          = w_wbus_wid   ; 
    assign o_w_wbus_vld          = w_wbus_vld   ;
    assign o_w_wbus_end          = w_wbus_end   ; 

    assign o_w_bbus_bid          = w_bbus_bid   ; 
    assign o_w_bbus_vld          = w_bbus_vld   ; 
    assign o_w_bbus_data         = w_bbus_data  ; 
endmodule
