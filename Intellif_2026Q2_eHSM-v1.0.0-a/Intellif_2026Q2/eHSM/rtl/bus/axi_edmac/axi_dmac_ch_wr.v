//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_ch_wr #(
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

    input   wire                        i_clk                   ,
    input   wire                        i_rst_n                 ,
    input   wire [ 7:0]                 i_ch_id                 ,    

    output  wire                        o_ch_w_busy             ,
    output  wire                        o_ch_w_done             ,
    output  wire [5:0]                  o_ch_w_qid              ,

    input   wire                        i_ch_w_start            ,
    input   wire                        i_ch_w_suspend          ,

    input  wire [p_BUS_AWIDTH-1:0]      i_ch_waddr              ,
    input  wire [31:0]                  i_ch_wlen               ,
    input  wire [15:0]                  i_ch_w_strans_max       ,
    input  wire [31:0]                  i_ch_w_awbus            ,
    input  wire [5:0]                   i_ch_w_qid              ,

    input  wire [p_BUS_DWIDTH-1:0]      i_w_fifo_rdata          ,
    output wire                         o_w_fifo_rd             ,
    input  wire                         i_w_fifo_ack            ,
    input  wire [p_R_FIFO_AWIDTH:0]     i_w_fifo_num            ,

    output  wire [p_BUS_AWIDTH+61:0]    o_w_abus_data           ,
    output  wire                        o_w_abus_vld            ,
    input   wire                        i_w_abus_rdy            ,

    output  wire [p_BUS_DWIDTH-1:0]     o_w_dbus_data           ,
    output  wire                        o_w_dbus_vld            ,
    input   wire                        i_w_dbus_rdy            ,

    input   wire                        i_w_wbus_vld            ,
    input   wire                        i_w_wbus_end            ,

    input   wire                        i_w_bbus_vld            ,
    input   wire [1:0]                  i_w_bbus_data           
);
    localparam [2:0]ADDR_OFFSET_WIDTH= (p_BUS_DWIDTH/8<=4  ) ? 3'd2 :    
                                       (p_BUS_DWIDTH/8<=8  ) ? 3'd3 :    
                                       (p_BUS_DWIDTH/8<=16 ) ? 3'd4 :    
                                       (p_BUS_DWIDTH/8<=32 ) ? 3'd5 :    
                                       (p_BUS_DWIDTH/8<=64 ) ? 3'd6 : 3'd0 ;

    localparam [ 1:0] WA_IDLE           = 2'b00  ;
    localparam [ 1:0] WA_TRANS          = 2'b01  ;
    localparam [ 1:0] WA_DONE           = 2'b10  ;

    localparam [31:0]MUX_FIFO_WIDTH   = p_W_FIFO_AWIDTH+1+p_W_FIFO_AWIDTH+1 ;

    genvar  i;
    integer ii;

    reg [p_BUS_AWIDTH-1:0]              ch_waddr                ;
    reg [31:0]                          ch_wlen                 ;
    reg [31:0]                          ch_w_awbus              ;
    reg [5:0]                           ch_w_qid                ;

    wire[p_BUS_AWIDTH-1:0]              ch_waddr_in             ; 
    wire[31:0]                          ch_wlen_in              ;
    wire[31:0]                          ch_w_awbus_in           ;
    wire[5:0]                           ch_w_qid_in             ;

    wire                                w_trig_start            ;
    wire                                w_fifo_rdy              ;
    reg                                 ch_w_suspend            ; 

    reg                                 w_busy                  ;
    wire                                w_busy_in               ;

    reg  [1:0]                          wa_cs                   ;
    reg  [1:0]                          wa_ns                   ;

    wire [15:0]                         w_strans_len_small      ;

    reg  [p_W_FIFO_AWIDTH:0]            w_fifo_cnt              ;
    wire [p_W_FIFO_AWIDTH:0]            w_fifo_cnt_in           ;

    wire [p_W_FIFO_AWIDTH:0]            w_fifo_num              ;

    wire                                w_trans_data_done       ;
    wire                                w_trans_addr_done       ;

    wire [31:0]                         w_trans_len_ns          ;
    wire [p_BUS_AWIDTH-1:0]             w_base_addr_ns          ;

    wire                                w_strans_fifo_inc       ;
    wire [p_W_FIFO_AWIDTH:0]            w_strans_fifo_num       ;

    wire [p_W_FIFO_AWIDTH:0]            w_fifo_cnt_inc          ;
    wire [p_W_FIFO_AWIDTH:0]            w_fifo_cnt_dec          ;
    wire [p_W_FIFO_AWIDTH:0]            w_fifo_cnt_add          ;

    wire                                w_abus_fifo_wr          ;

    wire                                w_fifo_rd               ;

    reg  [p_W_BMEM_AWIDTH:0]            w_trans_sub_cnt         ;
    wire [p_W_BMEM_AWIDTH:0]            w_trans_sub_cnt_in      ;

    reg  [p_W_BMEM_AWIDTH:0]            w_trans_bbus_cnt        ;
    wire [p_W_BMEM_AWIDTH:0]            w_trans_bbus_cnt_in     ;


    wire [p_BUS_DWIDTH-1:0]             w_dbus_data             ;
    reg                                 w_dbus_vld              ;

    wire                                w_dbus_vld_in           ;

    wire                                     mux_fifo_wr            ;
    wire [MUX_FIFO_WIDTH-1:0]                mux_fifo_wdata         ;
    wire                                     mux_fifo_full_n        ; 

    wire                                     mux_fifo_rd            ;
    wire [MUX_FIFO_WIDTH-1:0]                mux_fifo_rdata         ;
    wire                                     mux_fifo_empty_n       ;
    reg                                      mux_fifo_vld           ;
    wire                                     mux_fifo_vld_in        ;
    wire                                     mux_fifo_rdy           ;

    wire [16:0]                              w_strans_burst_range   ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_burst_len     ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_burst_len_out ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_fifo_num_out  ;

    reg  [p_W_FIFO_AWIDTH:0]                 w_strans_beat_cnt      ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_beat_cnt_in   ;

    reg  [p_W_FIFO_AWIDTH:0]                 w_strans_fifo_cnt      ;
    wire [p_W_FIFO_AWIDTH:0]                 w_strans_fifo_cnt_in   ;

    wire                                     w_fifo_rd_post         ;
    wire                                     w_fifo_ack_post        ;

    reg  [p_BUS_DWIDTH-1:0]                  w_fifo_rdata_latch     ;
    wire [p_BUS_DWIDTH-1:0]                  w_fifo_rdata_latch_in  ;
    reg                                      w_fifo_rd_d            ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            ch_waddr         <=   {(p_BUS_AWIDTH){1'b0}};
            ch_wlen          <=   32'b0;
            ch_w_awbus       <=   32'b0;
            ch_w_qid         <=    6'b0;
            ch_w_suspend     <=    1'b0;
        end
        else begin
            ch_waddr         <=  ch_waddr_in   ;
            ch_wlen          <=  ch_wlen_in    ;
            ch_w_awbus       <=  ch_w_awbus_in ;
            ch_w_qid         <=  ch_w_qid_in   ;
            ch_w_suspend     <=  i_ch_w_suspend;
        end
    end

    assign ch_waddr_in     = (i_ch_w_start      ) ? i_ch_waddr                :
                             w_abus_fifo_wr       ? w_base_addr_ns            : ch_waddr ;

    assign ch_wlen_in      = (i_ch_w_start      ) ? i_ch_wlen                 :
                             (i_ch_w_suspend    ) ? 32'b0                     :
                             w_abus_fifo_wr       ? w_trans_len_ns            : ch_wlen ;

    assign ch_w_awbus_in   = (i_ch_w_start      ) ? i_ch_w_awbus              : ch_w_awbus;

    assign ch_w_qid_in     = (i_ch_w_start      ) ? i_ch_w_qid                : ch_w_qid  ;

    assign w_trans_len_ns  = ch_wlen-{16'b0,w_strans_len_small} ;

    assign w_base_addr_ns  = ch_waddr+{{(p_BUS_AWIDTH-16){1'b0}},w_strans_len_small};

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            w_busy      <= 1'b0 ;
        end
        else begin
            w_busy      <=  w_busy_in ;
        end
    end

    assign w_busy_in    = (i_ch_w_start      )    ? 1'b1 :
                           w_trans_data_done      ? 1'b0 : w_busy ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            wa_cs <= WA_IDLE ;
        end
        else begin
            wa_cs <=  wa_ns ;
        end
    end

    always@(*)
    begin
        wa_ns=wa_cs;
        case(wa_cs)
            WA_IDLE : begin
                wa_ns = w_trig_start ? WA_TRANS : WA_IDLE ;
            end
            WA_TRANS : begin
                wa_ns = ((w_trig_start|ch_w_suspend)&(mux_fifo_full_n)&w_trans_addr_done&(i_w_abus_rdy)||((~i_w_abus_rdy)&ch_w_suspend)) ? WA_DONE : WA_TRANS ;
            end
            WA_DONE : begin
                wa_ns = (w_trans_data_done) ? WA_IDLE : WA_DONE ;
            end
        default:wa_ns=WA_IDLE;
        endcase
    end

    assign w_trans_addr_done    = (ch_wlen<={16'b0,i_ch_w_strans_max}) ;

    assign w_strans_len_small   = (w_trans_addr_done) ? ch_wlen[15:0] : i_ch_w_strans_max ;

    assign w_strans_fifo_inc = (|w_strans_len_small[ADDR_OFFSET_WIDTH-1:0]);
    assign w_strans_fifo_num = w_strans_len_small[ADDR_OFFSET_WIDTH+:(p_W_FIFO_AWIDTH+1)]+{{{p_W_FIFO_AWIDTH}{1'b0}},w_strans_fifo_inc};

    assign w_fifo_rdy   = (w_fifo_num>=w_strans_fifo_num);

    assign w_trig_start = w_fifo_rdy&w_busy&(ch_wlen!=0);

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            w_fifo_cnt <= {(p_W_FIFO_AWIDTH+1){1'b0}};
        end
        else begin
            w_fifo_cnt <=  w_fifo_cnt_in ;
        end
    end
    assign w_abus_fifo_wr= (wa_cs==WA_TRANS)&w_trig_start&(mux_fifo_full_n)&i_w_abus_rdy ;

    assign w_fifo_cnt_in = ((wa_cs==WA_IDLE)&w_trig_start)                           ? {(p_R_FIFO_AWIDTH+1){1'b0}} :
                           ((wa_cs==WA_TRANS)&w_abus_fifo_wr&w_fifo_rd&i_w_fifo_ack) ? w_fifo_cnt_inc              :
                           ((wa_cs==WA_TRANS)&w_abus_fifo_wr)                        ? w_fifo_cnt_add              :
                           (w_fifo_rd&i_w_fifo_ack)                                  ? w_fifo_cnt_dec              : w_fifo_cnt ;

    assign w_fifo_cnt_inc= (w_fifo_cnt_dec)+w_strans_fifo_num;

    assign w_fifo_cnt_add= (w_fifo_cnt+w_strans_fifo_num);

    assign w_fifo_cnt_dec= (w_fifo_cnt-{{{p_W_FIFO_AWIDTH}{1'b0}},1'b1});

    assign w_fifo_num    = i_w_fifo_num-w_fifo_cnt ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            w_dbus_vld    <= 1'b0;
        end
        else begin
            w_dbus_vld    <=  w_dbus_vld_in    ;
        end
    end

    assign w_dbus_vld_in   = (w_fifo_rd_post&w_fifo_ack_post)  ? 1'b1 :
                             i_w_dbus_rdy                      ? 1'b0 : w_dbus_vld ;

    assign w_fifo_rd_post  = mux_fifo_vld&((!w_dbus_vld)|(w_dbus_vld&i_w_dbus_rdy))&(w_strans_beat_cnt<=w_strans_burst_len_out);

    assign w_fifo_rd       = mux_fifo_vld&((!w_dbus_vld)|(w_dbus_vld&i_w_dbus_rdy))&(w_fifo_cnt!=0)&(w_strans_fifo_cnt<w_strans_fifo_num_out);

    assign w_fifo_ack_post = (w_fifo_rd&i_w_fifo_ack)|(!w_fifo_rd);

    assign w_strans_burst_range        = (w_strans_len_small+{{(17-ADDR_OFFSET_WIDTH){1'b0}},ch_waddr[ADDR_OFFSET_WIDTH-1:0]});

wire                          w_strans_burst_range_inc_tmp = (|w_strans_burst_range[ADDR_OFFSET_WIDTH-1:0]);
wire [16-ADDR_OFFSET_WIDTH:0] w_strans_burst_range_add_tmp = w_strans_burst_range[16:ADDR_OFFSET_WIDTH]+{{(16-ADDR_OFFSET_WIDTH){1'b0}},w_strans_burst_range_inc_tmp};
    assign w_strans_burst_len          = (w_strans_len_small==0) ? {(p_W_FIFO_AWIDTH+1){1'b0}}  : w_strans_burst_range_add_tmp[p_W_FIFO_AWIDTH:0];

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (p_W_SMEM_AWIDTH   ),
        .p_DWIDTH       (MUX_FIFO_WIDTH    )
    ) u_mux_fifo(
        .i_clk          (i_clk             ),
        .i_rst_n        (i_rst_n           ),    

        .i_wr           (mux_fifo_wr       ),
        .i_wr_data      (mux_fifo_wdata    ),
        .o_wr_num_count (                  ),
        .o_wr_full      (                  ),
        .o_wr_full_n    (mux_fifo_full_n   ),
        .o_wr_al_full   (                  ),
        .o_wr_al_full_n (                  ),

        .i_rd           (mux_fifo_rd       ),
        .o_rd_data      (mux_fifo_rdata    ),
        .o_rd_num_count (                  ),
        .o_rd_empty     (                  ),
        .o_rd_empty_n   (mux_fifo_empty_n  ),
        .o_rd_al_empty  (                  ),
        .o_rd_al_empty_n(                  ),

        .i_clear        (1'b0              )
    ); 

    assign mux_fifo_wr           = w_abus_fifo_wr ;
    assign mux_fifo_wdata        = {w_strans_fifo_num,w_strans_burst_len};

    assign mux_fifo_rd           = (mux_fifo_empty_n)&((!mux_fifo_vld)|(mux_fifo_vld&mux_fifo_rdy));

    assign {w_strans_fifo_num_out,w_strans_burst_len_out} = mux_fifo_rdata;

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            mux_fifo_vld      <= 1'b0 ;
            w_strans_beat_cnt <= {(p_W_FIFO_AWIDTH+1){1'b0}};
            w_strans_fifo_cnt <= {(p_W_FIFO_AWIDTH+1){1'b0}};
        end
        else begin
            mux_fifo_vld      <=  mux_fifo_vld_in ;
            w_strans_beat_cnt <=  w_strans_beat_cnt_in ;
            w_strans_fifo_cnt <=  w_strans_fifo_cnt_in ;                        
        end
    end

    assign mux_fifo_vld_in = (mux_fifo_rd) ? 1'b1 :
                             mux_fifo_rdy  ? 1'b0 : mux_fifo_vld ;

    assign mux_fifo_rdy         = (w_strans_beat_cnt==w_strans_burst_len_out)&w_fifo_rd_post&w_fifo_ack_post;

    assign w_strans_beat_cnt_in = mux_fifo_rd                    ? {{(p_W_FIFO_AWIDTH){1'b0}},1'b1} :
                                  w_fifo_rd_post&w_fifo_ack_post ? w_strans_beat_cnt+1'b1              : w_strans_beat_cnt ;

    assign w_strans_fifo_cnt_in = mux_fifo_rd                    ? {{(p_W_FIFO_AWIDTH+1){1'b0}}}    :
                                  w_fifo_rd_post&w_fifo_ack_post ? w_strans_fifo_cnt+1'b1              : w_strans_fifo_cnt ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            w_trans_sub_cnt  <= {(p_W_BMEM_AWIDTH+1){1'b0}};
            w_trans_bbus_cnt <= {(p_W_BMEM_AWIDTH+1){1'b0}};
        end
        else begin
            w_trans_sub_cnt  <=  w_trans_sub_cnt_in  ;
            w_trans_bbus_cnt <=  w_trans_bbus_cnt_in ;
        end
    end

    assign w_trans_sub_cnt_in  = ((wa_cs==WA_IDLE)&w_trig_start)              ? {(p_W_BMEM_AWIDTH+1){1'b0}} :
                                 (w_abus_fifo_wr&i_w_wbus_vld&i_w_wbus_end)   ?  w_trans_sub_cnt            :
                                 (w_abus_fifo_wr)                             ?  w_trans_sub_cnt+1'b1          :
                                 (i_w_wbus_vld&i_w_wbus_end)                  ?  w_trans_sub_cnt-1'b1          : w_trans_sub_cnt ;

    assign w_trans_bbus_cnt_in = ((wa_cs==WA_IDLE)&w_trig_start) ? {(p_W_BMEM_AWIDTH+1){1'b0}} :
                                 (i_w_wbus_vld&i_w_bbus_vld)     ?  w_trans_bbus_cnt           :
                                 (i_w_wbus_vld)                  ?  w_trans_bbus_cnt+1'b1         :
                                 (i_w_bbus_vld)                  ?  w_trans_bbus_cnt-1'b1         : w_trans_bbus_cnt ;

    assign w_trans_data_done = ((wa_cs==WA_DONE)&(w_trans_sub_cnt==0)&(w_trans_bbus_cnt==0))|((wa_cs==WA_IDLE)&(ch_wlen==0)&w_busy);

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            w_fifo_rdata_latch <= {(p_BUS_DWIDTH){1'b0}};
            w_fifo_rd_d        <= 1'b0 ;
        end
        else begin
            w_fifo_rdata_latch <=  w_fifo_rdata_latch_in ;
            w_fifo_rd_d        <=  w_fifo_rd ;
        end
    end

    assign w_fifo_rdata_latch_in = (w_fifo_rd_d) ? i_w_fifo_rdata : w_fifo_rdata_latch ;

    assign w_dbus_data           = w_fifo_rdata_latch_in ;

    assign o_w_abus_vld   = (wa_cs==WA_TRANS)&w_trig_start&(mux_fifo_full_n);

    assign o_w_abus_data  = {ch_w_qid,ch_w_awbus,i_ch_id,w_strans_len_small,ch_waddr};

    assign o_w_fifo_rd    = w_fifo_rd ; 
    assign o_w_dbus_vld   = w_dbus_vld;
    assign o_w_dbus_data  = w_dbus_data;

    assign o_ch_w_busy    = w_busy            ;
    assign o_ch_w_done    = w_trans_data_done ;
    assign o_ch_w_qid     = ch_w_qid          ;
endmodule
