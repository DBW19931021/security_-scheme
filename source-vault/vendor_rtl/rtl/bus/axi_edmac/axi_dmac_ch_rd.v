//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_ch_rd #(
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
    parameter [31:0] p_R_ALIGN_STEP           =  8
)(

    input   wire                        i_clk                   ,
    input   wire                        i_rst_n                 ,
    input   wire [ 7:0]                 i_ch_id                 ,

    output  wire                        o_ch_r_busy             ,
    output  wire                        o_ch_r_done             ,
    output  wire [5:0]                  o_ch_r_qid              ,

    input   wire                        i_ch_r_start            ,
    input   wire                        i_ch_r_suspend          ,

    input  wire [p_BUS_AWIDTH-1:0]      i_ch_raddr              ,
    input  wire [31:0]                  i_ch_rlen               ,
    input  wire [15:0]                  i_ch_r_strans_max       ,
    input  wire [31:0]                  i_ch_r_arbus            ,
    input  wire [5:0]                   i_ch_r_qid              ,

    output wire [p_BUS_DWIDTH-1:0]      o_r_fifo_wdata          ,
    output wire                         o_r_fifo_wr             ,
    input  wire [p_R_FIFO_AWIDTH:0]     i_r_fifo_num            ,

    output  wire [2*p_BUS_AWIDTH+61:0]    o_r_abus_data           ,
    output  wire                        o_r_abus_vld            ,
    input   wire                        i_r_abus_rdy            ,

    input   wire                        i_r_dbus_vld            ,
    input   wire [p_BUS_DWIDTH-1:0]     i_r_dbus_data           ,
    input   wire                        i_r_dbus_last           
);
    localparam [2:0]ADDR_OFFSET_WIDTH= (p_BUS_DWIDTH/8<=4  ) ? 3'd2 :    
                                       (p_BUS_DWIDTH/8<=8  ) ? 3'd3 :    
                                       (p_BUS_DWIDTH/8<=16 ) ? 3'd4 :    
                                       (p_BUS_DWIDTH/8<=32 ) ? 3'd5 :    
                                       (p_BUS_DWIDTH/8<=64 ) ? 3'd6 : 3'd0 ;

    localparam [31:0] R_ALIGN_MUX_NUM   = p_BUS_DWIDTH/p_R_ALIGN_STEP;
    localparam [31:0] R_ALIGN_MUX_WIDTH = (R_ALIGN_MUX_NUM<= 2) ? 32'd1 :
                                          (R_ALIGN_MUX_NUM<= 4) ? 32'd2 :
                                          (R_ALIGN_MUX_NUM<= 8) ? 32'd3 :
                                          (R_ALIGN_MUX_NUM<=16) ? 32'd4 :
                                          (R_ALIGN_MUX_NUM<=32) ? 32'd5 :
                                          (R_ALIGN_MUX_NUM<=64) ? 32'd6 : 32'd0 ;

    localparam [31:0] ABUS_FIFO_WIDTH   = ADDR_OFFSET_WIDTH+R_ALIGN_MUX_WIDTH+p_R_FIFO_AWIDTH+1;

    localparam [ 1:0] RA_IDLE           = 2'b00  ;
    localparam [ 1:0] RA_TRANS          = 2'b01  ;
    localparam [ 1:0] RA_DONE           = 2'b10  ;

    localparam [ 1:0] RD_IDLE           = 2'b00  ;
    localparam [ 1:0] RD_TRANS          = 2'b01  ;
    localparam [ 1:0] RD_DONE           = 2'b10  ;

    genvar  i;
    integer ii;

    reg [p_BUS_AWIDTH-1:0]              ch_raddr                ;
    reg [31:0]                          ch_rlen                 ;
    reg [31:0]                          ch_r_arbus              ;
    reg [5:0]                           ch_r_qid                ;

    wire[p_BUS_AWIDTH-1:0]              ch_raddr_in             ; 
    wire[31:0]                          ch_rlen_in              ;
    wire[31:0]                          ch_r_arbus_in           ;
    wire[5:0]                           ch_r_qid_in             ;

    wire                                r_trig_start            ;
    wire                                r_fifo_rdy              ;
    reg                                 ch_r_suspend            ;  
    reg                                 r_busy                  ;
    wire                                r_busy_in               ;

    reg  [1:0]                          ra_cs                   ;
    reg  [1:0]                          ra_ns                   ;

    wire [15:0]                         r_strans_len_small      ;

    wire                                r_fifo_wr               ;
    wire [p_BUS_DWIDTH-1:0]             r_fifo_wdata            ;

    reg  [p_R_FIFO_AWIDTH:0]            r_fifo_wr_cnt           ;
    wire [p_R_FIFO_AWIDTH:0]            r_fifo_wr_cnt_in        ;

    reg  [p_R_FIFO_AWIDTH:0]            r_fifo_cnt              ;
    wire [p_R_FIFO_AWIDTH:0]            r_fifo_cnt_in           ;

    wire [p_R_FIFO_AWIDTH:0]            r_fifo_num              ;

    wire                                r_trans_data_done       ;
    wire                                r_trans_addr_done       ;

    wire [31:0]                         r_trans_len_ns          ;
    wire [p_BUS_AWIDTH-1:0]             r_base_addr_ns          ;

    wire                                r_strans_fifo_inc       ;
    wire [p_R_FIFO_AWIDTH:0]            r_strans_fifo_num       ;

    wire [1:0]                          r_strans_burst_inc      ;
    wire [15:0]                         r_strans_burst_num      ;

    wire [p_R_FIFO_AWIDTH:0]            r_fifo_cnt_inc          ;
    wire [p_R_FIFO_AWIDTH:0]            r_fifo_cnt_dec          ;
    wire [p_R_FIFO_AWIDTH:0]            r_fifo_cnt_add          ;

    wire                                r_abus_fifo_wr          ; 
    wire [ABUS_FIFO_WIDTH-1:0]          r_abus_fifo_wdata       ;
    wire                                r_abus_fifo_full_n      ; 

    wire                                r_abus_fifo_rd          ; 
    wire [ABUS_FIFO_WIDTH-1:0]          r_abus_fifo_rdata       ;
    wire                                r_abus_fifo_empty       ;
    wire                                r_abus_fifo_empty_n     ; 

    reg                                 abus_fifo_vld           ;
    wire                                abus_fifo_vld_in        ;
    wire                                abus_fifo_rdy           ;

    wire [ADDR_OFFSET_WIDTH-1:0]        r_strans_tail_mask_in   ;
    wire [ADDR_OFFSET_WIDTH-1:0]        r_strans_tail_mask_out  ;
    wire [p_BUS_DWIDTH/8-1:0]           r_strans_tail_mask_post ;

    wire [R_ALIGN_MUX_WIDTH-1:0]        r_rd_align_sel_out      ;
    wire [R_ALIGN_MUX_WIDTH-1:0]        r_rd_align_sel          ;
    wire [15:0]                         r_strans_fifo_num_out   ;

    reg                                 r_rd_align_rdy          ;
    wire                                r_rd_align_rdy_in       ;
    wire [p_BUS_DWIDTH-1:0]             r_rd_align_in           ;
    wire [p_BUS_DWIDTH-1:0]             r_rd_align_out          ;
    reg  [p_BUS_DWIDTH-1:0]             r_rd_align_buf          ;
    wire [p_BUS_DWIDTH-1:0]             r_rd_align_buf_in       ;
    wire                                r_rd_align_buf_upd      ;
    wire [p_BUS_DWIDTH-1:0]             r_rd_align_mux [R_ALIGN_MUX_NUM-1:0];    

    wire [p_BUS_DWIDTH/8-1:0]           r_fifo_mask             ;
    wire [p_BUS_DWIDTH-1:0]             r_fifo_wdata_mask       ;

    wire [p_BUS_DWIDTH-1:0]             dma_max_addr_in         ;
    reg  [p_BUS_DWIDTH-1:0]             dma_max_addr_out        ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            ch_raddr         <=   {(p_BUS_AWIDTH){1'b0}};
            ch_rlen          <=   32'b0;
            ch_r_arbus       <=   32'b0;
            ch_r_qid         <=    6'b0;
            ch_r_suspend     <=    1'b0;
            dma_max_addr_out <=   {(p_BUS_DWIDTH){1'b0}};
        end
        else begin
            ch_raddr         <=  ch_raddr_in   ; 
            ch_rlen          <=  ch_rlen_in    ;
            ch_r_arbus       <=  ch_r_arbus_in ;
            ch_r_qid         <=  ch_r_qid_in   ;
            ch_r_suspend     <=  i_ch_r_suspend;
            dma_max_addr_out <=  dma_max_addr_in;
        end
    end

    assign dma_max_addr_in = (i_ch_r_start      ) ? i_ch_raddr + i_ch_rlen - 1: dma_max_addr_out;

    assign ch_raddr_in     = (i_ch_r_start      ) ? i_ch_raddr                :
                             r_abus_fifo_wr       ? r_base_addr_ns            : ch_raddr  ;

    assign ch_rlen_in      = (i_ch_r_start      ) ? i_ch_rlen                 :
                             (i_ch_r_suspend    ) ? 32'b0                     :
                             r_abus_fifo_wr       ? r_trans_len_ns            : ch_rlen   ;

    assign ch_r_arbus_in   = (i_ch_r_start      ) ? i_ch_r_arbus              : ch_r_arbus;

    assign ch_r_qid_in     = (i_ch_r_start      ) ? i_ch_r_qid                : ch_r_qid  ;

    assign r_trans_len_ns  = ch_rlen-{16'b0,r_strans_len_small} ;

    assign r_base_addr_ns  = ch_raddr+{{(p_BUS_AWIDTH-16){1'b0}},r_strans_len_small};

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            r_busy      <= 1'b0 ;
        end
        else begin
            r_busy      <=  r_busy_in ;
        end
    end

    assign r_busy_in    = (i_ch_r_start      )    ? 1'b1 :
                           r_trans_data_done      ? 1'b0 : r_busy ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            ra_cs <= RA_IDLE ;
        end
        else begin
            ra_cs <=  ra_ns ;
        end
    end

    always@(*)
    begin
        ra_ns=ra_cs;
        case(ra_cs)
            RA_IDLE : begin
                ra_ns = r_trig_start ? RA_TRANS : RA_IDLE ;
            end
            RA_TRANS : begin
                ra_ns = ((r_trig_start|ch_r_suspend)&(r_abus_fifo_full_n)&r_trans_addr_done&(i_r_abus_rdy)||((~i_r_abus_rdy)&ch_r_suspend)) ? RA_DONE : RA_TRANS ;
            end
            RA_DONE : begin
                ra_ns = (r_trans_data_done) ? RA_IDLE : RA_DONE ;
            end
        default:ra_ns=RA_IDLE;
        endcase
    end

    assign r_trans_addr_done    = (ch_rlen<={16'b0,i_ch_r_strans_max}) ;

    assign r_strans_len_small   = (r_trans_addr_done) ? ch_rlen[15:0] : i_ch_r_strans_max ;

    assign r_strans_fifo_inc    = (|r_strans_len_small[ADDR_OFFSET_WIDTH-1:0]);
    assign r_strans_fifo_num    = r_strans_len_small[ADDR_OFFSET_WIDTH+:(p_R_FIFO_AWIDTH+1)]+{{{p_R_FIFO_AWIDTH}{1'b0}},r_strans_fifo_inc};

    assign r_strans_burst_inc   = {1'b0,(|r_strans_len_small[ADDR_OFFSET_WIDTH-1:0])}+{1'b0,(|ch_raddr[ADDR_OFFSET_WIDTH-1:0])};
    assign r_strans_burst_num   = {{(ADDR_OFFSET_WIDTH){1'b0}},r_strans_len_small[15:ADDR_OFFSET_WIDTH]}+{14'b0,r_strans_burst_inc};

    assign r_fifo_rdy   = (r_fifo_num>=r_strans_fifo_num);

    assign r_trig_start = r_fifo_rdy&r_busy&(ch_rlen!=0);

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            r_fifo_cnt <= {(p_R_FIFO_AWIDTH+1){1'b0}};
        end
        else begin
            r_fifo_cnt <=  r_fifo_cnt_in ;
        end
    end

    assign r_fifo_cnt_in = ((ra_cs==RA_IDLE)&r_trig_start)              ? {(p_R_FIFO_AWIDTH+1){1'b0}} :
                           ((ra_cs==RA_TRANS)&r_abus_fifo_wr&r_fifo_wr) ? r_fifo_cnt_inc              :
                           ((ra_cs==RA_TRANS)&r_abus_fifo_wr)           ? r_fifo_cnt_add              :
                           (r_fifo_wr)                                  ? r_fifo_cnt_dec              : r_fifo_cnt ;

    assign r_fifo_cnt_inc= r_fifo_cnt_dec+r_strans_fifo_num;

    assign r_fifo_cnt_add= (r_fifo_cnt+r_strans_fifo_num);

    assign r_fifo_cnt_dec= (r_fifo_cnt-{{{p_R_FIFO_AWIDTH}{1'b0}},1'b1});

    assign r_fifo_num    = i_r_fifo_num-r_fifo_cnt ;

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (p_R_SMEM_AWIDTH                     ),
        .p_DWIDTH       (ABUS_FIFO_WIDTH                     )
    ) u_r_abus_fifo(
        .i_clk          (i_clk                  ),
        .i_rst_n        (i_rst_n                ),    

        .i_wr           (r_abus_fifo_wr         ),
        .i_wr_data      (r_abus_fifo_wdata      ),
        .o_wr_num_count (                       ),
        .o_wr_full      (                       ),
        .o_wr_full_n    (r_abus_fifo_full_n     ),
        .o_wr_al_full   (                       ),
        .o_wr_al_full_n (                       ),

        .i_rd           (r_abus_fifo_rd         ),
        .o_rd_data      (r_abus_fifo_rdata      ),
        .o_rd_num_count (                       ),
        .o_rd_empty     (r_abus_fifo_empty      ),
        .o_rd_empty_n   (r_abus_fifo_empty_n    ),
        .o_rd_al_empty  (                       ),
        .o_rd_al_empty_n(                       ),

        .i_clear        (1'b0                   )
    ); 

    assign r_abus_fifo_wr        = (ra_cs==RA_TRANS)&r_trig_start&(r_abus_fifo_full_n)&(i_r_abus_rdy) ;

    assign r_strans_tail_mask_in = r_strans_len_small[ADDR_OFFSET_WIDTH-1:0] ;

    assign r_abus_fifo_wdata     = {r_strans_fifo_num,ch_raddr[(ADDR_OFFSET_WIDTH-R_ALIGN_MUX_WIDTH)+:R_ALIGN_MUX_WIDTH],r_strans_tail_mask_in} ;

    assign r_abus_fifo_rd        = (r_abus_fifo_empty_n)&((~abus_fifo_vld)|(abus_fifo_vld&abus_fifo_rdy));

    assign {r_strans_fifo_num_out,r_rd_align_sel_out,r_strans_tail_mask_out} = {{(16+ADDR_OFFSET_WIDTH+R_ALIGN_MUX_WIDTH-ABUS_FIFO_WIDTH){1'b0}},r_abus_fifo_rdata} ;

    assign r_rd_align_sel        = (p_R_ALIGN) ? {(R_ALIGN_MUX_WIDTH){1'b0}} : r_rd_align_sel_out;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            abus_fifo_vld <= 1'b0 ;
        end
        else begin
            abus_fifo_vld <=  abus_fifo_vld_in ;
        end
    end

    assign abus_fifo_vld_in = (r_abus_fifo_rd) ? 1'b1 :
                               abus_fifo_rdy   ? 1'b0 : abus_fifo_vld ;

    assign abus_fifo_rdy    = abus_fifo_vld&((r_fifo_wr_cnt+{{{p_R_FIFO_AWIDTH}{1'b0}},1'b1})==r_strans_fifo_num_out)&r_fifo_wr ;

    assign r_trans_data_done = ((ra_cs==RA_DONE)&(r_abus_fifo_empty)&(r_fifo_cnt==0))|((ra_cs==RA_IDLE)&(ch_rlen==0)&r_busy);

    always@(posedge i_clk or negedge i_rst_n) begin
        if(!i_rst_n) begin
            r_fifo_wr_cnt <= {(p_R_FIFO_AWIDTH+1){1'b0}};
        end
        else begin
            r_fifo_wr_cnt <=  r_fifo_wr_cnt_in ;
        end
    end

    assign r_fifo_wr_cnt_in = r_abus_fifo_rd ? {(p_R_FIFO_AWIDTH+1){1'b0}} :
                              r_fifo_wr      ? r_fifo_wr_cnt+1'b1             : r_fifo_wr_cnt ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(~i_rst_n) begin
            r_rd_align_buf     <= {(p_BUS_DWIDTH){1'b0}};
            r_rd_align_rdy     <= 1'b0;
        end
        else begin
            r_rd_align_buf     <=  r_rd_align_buf_in;
            r_rd_align_rdy     <=  r_rd_align_rdy_in;
        end
    end

    assign r_rd_align_buf_upd    = (i_r_dbus_vld) ;

    assign r_rd_align_buf_in     = r_rd_align_buf_upd ? i_r_dbus_data : r_rd_align_buf ;    

    assign r_rd_align_rdy_in     = r_abus_fifo_rd                                     ? 1'b0 :
                                   (abus_fifo_vld&i_r_dbus_vld)                       ? 1'b1 : r_rd_align_rdy ;

    assign r_rd_align_in         = i_r_dbus_data ;

    assign r_rd_align_mux[0]={r_rd_align_in};
    generate
    if(p_R_ALIGN) begin:rdata_align_mux_logic_gen
        for(i=1;i<R_ALIGN_MUX_NUM;i=i+1) begin:rdata_align_mux_bypass
            assign r_rd_align_mux[i]={r_rd_align_in};
        end
    end
    else begin:rdata_align_mux_bypass_gen

        for(i=1;i<R_ALIGN_MUX_NUM;i=i+1) begin:rdata_align_mux_logic
            assign r_rd_align_mux[i]={ r_rd_align_in[i*p_R_ALIGN_STEP-1:0],r_rd_align_buf[p_BUS_DWIDTH-1:i*p_R_ALIGN_STEP]};
        end
    end
    endgenerate

    assign r_rd_align_out = r_rd_align_mux[r_rd_align_sel];

    assign r_fifo_mask    = abus_fifo_rdy&
                            (((r_rd_align_sel==0)&i_r_dbus_vld&i_r_dbus_last)|
                             (r_rd_align_rdy&i_r_dbus_vld&i_r_dbus_last))      ? r_strans_tail_mask_post : {(p_BUS_DWIDTH/8){1'b1}} ;

generate    
if(1) begin:r_fifo_wdata_mask_gen
    for(i=0;i<p_BUS_DWIDTH/8;i=i+1) begin:r_fifo_wdata_mask_logic
        assign r_strans_tail_mask_post[i]= (r_strans_tail_mask_out>i)|(r_strans_tail_mask_out==0);
        assign r_fifo_wdata_mask[i*8+:8]= {(8){r_fifo_mask[i]}};
    end
end
endgenerate

    assign r_fifo_wdata = r_rd_align_out&r_fifo_wdata_mask ;

    assign r_fifo_wr    = ((r_rd_align_sel==0)&i_r_dbus_vld)|
                          (r_rd_align_rdy&i_r_dbus_vld);

    assign o_r_fifo_wr    = r_fifo_wr ; 
    assign o_r_fifo_wdata = r_fifo_wdata ; 

    assign o_r_abus_vld   = (ra_cs==RA_TRANS)&r_trig_start&(r_abus_fifo_full_n);

    assign o_r_abus_data  = {dma_max_addr_out,ch_r_qid,ch_r_arbus,i_ch_id,r_strans_burst_num,ch_raddr};

    assign o_ch_r_busy    = r_busy            ; 
    assign o_ch_r_done    = r_trans_data_done ; 
    assign o_ch_r_qid     = ch_r_qid          ;
endmodule
