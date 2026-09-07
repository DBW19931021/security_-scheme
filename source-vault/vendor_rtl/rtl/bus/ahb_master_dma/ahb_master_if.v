//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_master_if #(
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

    input   wire                       i_m_clk                ,
    input   wire                       i_m_rst_n              ,

    output  wire [p_ADDR_WIDTH-1:0]    o_w_base_addr_ns       ,
    output  wire [p_ADDR_WIDTH-1:0]    o_w_trans_len_ns       ,
    output  wire [ 7:0]                o_w_strans_len_ns      ,
    input   wire [p_ADDR_WIDTH-1:0]    i_w_base_addr          ,
    input   wire [p_ADDR_WIDTH-1:0]    i_w_trans_len          ,
    input   wire [ 7:0]                i_w_strans_len         ,

    output  wire                       o_w_rd_en              ,
    input   wire [p_DATA_WIDTH-1:0]    i_w_data               ,
    output  wire                       o_w_strans_done_addr   ,
    output  wire                       o_w_trans_done_addr    ,
    output  wire                       o_w_strans_done_data   ,

    output  wire [p_ADDR_WIDTH-1:0]    o_r_base_addr_ns       ,
    output  wire [p_ADDR_WIDTH-1:0]    o_r_trans_len_ns       ,
    output  wire [7-p_DATA_WIDTH/32:0] o_r_strans_len_burst_ns,
    input   wire [p_ADDR_WIDTH-1:0]    i_r_base_addr          ,
    input   wire [p_ADDR_WIDTH-1:0]    i_r_trans_len          ,
    input   wire [ 7:0]                i_r_strans_len         ,
    input   wire [7-p_DATA_WIDTH/32:0] i_r_strans_len_burst   ,
    input   wire [p_ADDR_WIDTH-1:0]    i_r_ch_saddr_max       ,

    output  wire [p_DATA_WIDTH-1:0]    o_r_data               ,
    output  wire                       o_r_wr_en              ,
    output  wire                       o_r_strans_done_addr   ,
    output  wire                       o_r_trans_done_addr    ,
    output  wire                       o_r_strans_done_data   ,
    output  wire                       o_r_to                 ,
    output  wire                       o_w_to                 ,      

    input   wire [ 7:0]                i_ch_to                ,
    input   wire [ 7:0]                i_ch_bus               ,
    input   wire                       i_r_revr_en            ,
    input   wire                       i_w_revr_en            ,
    input   wire                       i_ch_r_atyp            ,
    input   wire                       i_ch_w_atyp            ,
    input   wire                       i_suspend              ,
    output  wire                       o_suspend_done         ,

    output  wire [p_ADDR_WIDTH-1:0]    o_w_haddr              ,
    output  wire [ 2:0]                o_w_hburst             ,
    output  wire [ 1:0]                o_w_hsize              ,
    output  wire [ 1:0]                o_w_htrans             ,
    output  wire [p_DATA_WIDTH-1:0]    o_w_hwdata             ,
    output  wire [ 3:0]                o_w_hprot              ,
    output  wire                       o_w_hwrite             ,
    input   wire [p_DATA_WIDTH-1:0]    i_w_hrdata             ,
    input   wire                       i_w_hready             ,

    output  wire [p_ADDR_WIDTH-1:0]    o_r_haddr              ,
    output  wire [ 2:0]                o_r_hburst             ,
    output  wire [ 1:0]                o_r_hsize              ,
    output  wire [ 1:0]                o_r_htrans             ,
    output  wire [p_DATA_WIDTH-1:0]    o_r_hwdata             ,
    output  wire [3:0]                 o_r_hprot              ,
    output  wire                       o_r_hwrite             ,
    input   wire [p_DATA_WIDTH-1:0]    i_r_hrdata             ,
    input   wire                       i_r_hready             
);

    localparam IDLE        = 3'h0;
    localparam SINGLE      = 3'h1;
    localparam BURST_START = 3'h2;
    localparam BURST_CONT  = 3'h3;
    localparam DONE        = 3'h4;

    localparam [2:0]ADDR_OFFSET_WIDTH= (p_DATA_WIDTH/8<=4  ) ? 3'd2 :    
                                       (p_DATA_WIDTH/8<=8  ) ? 3'd3 :    
                                       (p_DATA_WIDTH/8<=16 ) ? 3'd4 :    
                                       (p_DATA_WIDTH/8<=32 ) ? 3'd5 :    
                                       (p_DATA_WIDTH/8<=64 ) ? 3'd6 : 3'd0 ;
    genvar  i;

    reg  [2:0]  w_cs ;
    reg  [2:0]  w_ns ;

    reg  [2:0]  r_cs ;
    reg  [2:0]  r_ns ;
    reg  [2:0]  r_ls ;
    wire [2:0]  r_ls_in;
    reg         s_suspend_1d     ;
    wire        s_suspend_flag_nxt ;
    reg         s_suspend_flag     ;

    wire [p_ADDR_WIDTH-1:0] r_base_addr_unalign_compare; 

    reg  [p_ADDR_WIDTH-1:0]    w_haddr         ;
    reg  [p_DATA_WIDTH-1:0]    w_hwdata        ;
    reg  [ 2:0]                w_hburst        ;
    reg  [ 1:0]                w_hsize         ;
    reg  [ 1:0]                w_htrans        ;

    wire [p_ADDR_WIDTH-1:0]    w_haddr_in      ;
    wire [p_DATA_WIDTH-1:0]    w_hwdata_in     ;
    wire [ 2:0]                w_hburst_in     ;
    wire [ 1:0]                w_hsize_in      ;
    wire [ 1:0]                w_htrans_in     ;

    wire [p_DATA_WIDTH-1:0]    w_revr_in       ;
    wire [p_DATA_WIDTH-1:0]    w_revr_out      ;

    wire [p_DATA_WIDTH-1:0]    w_align_in      ;
    reg  [p_DATA_WIDTH-1:0]    w_align_buf     ;
    wire [p_DATA_WIDTH-1:0]    w_align_buf_in  ;
    reg  [p_DATA_WIDTH-1:0]    w_align_out     ;
    reg  [p_DATA_WIDTH/32:0]   w_align_sel     ;
    wire [p_DATA_WIDTH/32:0]   w_align_sel_in  ;
    wire                       w_align_buf_upd_fifo;
    wire                       w_align_buf_upd ;
    wire                       w_align_init    ;

    wire                       w_burst_en      ;

    wire [p_DATA_WIDTH/32+1:0] w_trans_num     ;
    wire [9-p_DATA_WIDTH/32:0] w_burst_dist    ;
    wire [6-p_DATA_WIDTH/32:0] w_burst_dist_small;

    reg  [4:0]                 w_burst_cnt     ;
    wire [4:0]                 w_burst_cnt_in  ;
    wire [4:0]                 w_burst_single  ;

    reg  [2:0]                 w_hburst_sav    ;
    wire [2:0]                 w_hburst_dec    ;
    wire [2:0]                 w_hburst_dec_org;
    wire [1:0]                 w_hsize_dec     ;

    reg  [ 7:0]                w_to_cnt        ;
    wire [ 7:0]                w_to_cnt_in     ;

    reg  [p_ADDR_WIDTH-1:0]    r_haddr             ;
    reg  [ 2:0]                r_hburst            ;
    reg  [ 1:0]                r_htrans            ;

    reg  [ 1:0]                r_htrans_sav        ;

    wire [p_ADDR_WIDTH-1:0]    r_haddr_in          ;
    wire [ 2:0]                r_hburst_in         ;
    wire [ 1:0]                r_htrans_in         ;

    reg                        r_trans_en_ls       ;
    reg                        r_strans_addr_end        ;

    wire [p_DATA_WIDTH-1:0]    r_revr_in           ;
    wire [p_DATA_WIDTH-1:0]    r_revr_out          ;

    wire [p_DATA_WIDTH-1:0]    r_align_in          ;
    reg  [p_DATA_WIDTH-1:0]    r_align_buf         ;
    wire [p_DATA_WIDTH-1:0]    r_align_buf_in      ;
    reg  [p_DATA_WIDTH-1:0]    r_align_out         ;
    wire                       r_align_out_vld     ;
    reg  [p_DATA_WIDTH/32:0]   r_align_sel         ;
    wire [p_DATA_WIDTH/32:0]   r_align_sel_in      ;
    wire                       r_align_buf_upd     ;

    reg                        r_align_rdy         ;
    wire                       r_align_rdy_in      ;
    wire                       r_align_init        ;

    wire                       r_burst_en          ;



    wire [8:0]                 r_burst_dist        ;
    wire [6:0]                 r_burst_dist_small  ;
    wire [4:0]                 r_burst_single      ;

    wire [p_DATA_WIDTH/32+1:0] r_trans_num         ;


    reg  [4:0]                 r_burst_cnt         ;
    wire [4:0]                 r_burst_cnt_in      ;

    reg  [2:0]                 r_hburst_sav        ;
    wire [2:0]                 r_hburst_dec        ;
    wire [2:0]                 r_hburst_dec_org    ;


    reg  [p_ADDR_WIDTH+4:0]    r_ach_fifo_data     ;

    reg  [ 7:0]                r_to_cnt            ;
    wire [ 7:0]                r_to_cnt_in         ;

    reg                        r_htrans_ls         ;

    reg  [p_ADDR_WIDTH-1:0]    r_base_addr_unalign ;
    wire [p_ADDR_WIDTH-1:0]    r_base_addr_unalign_in ;

    reg  [p_DATA_WIDTH/8-1:0]  r_strans_tail_mask     ; 
    wire [p_DATA_WIDTH/8-1:0]  r_strans_tail_mask_in  ;
    wire [p_DATA_WIDTH/8-1:0]  r_strans_tail_mask_post;
    wire [p_DATA_WIDTH/8-1:0]  r_align_out_mask_post  ;    
    wire [p_DATA_WIDTH-1:0]    r_align_out_mask       ;

assign  s_suspend_flag_nxt = i_suspend ? 1'b1 :o_suspend_done ? 1'b0: s_suspend_flag;
    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            s_suspend_1d   <= 1'b0; 
            s_suspend_flag <= 1'b0; 
        end
        else begin
            s_suspend_1d   <= i_suspend; 
            s_suspend_flag <= s_suspend_flag_nxt ; 
        end
    end
assign o_suspend_done = (w_cs == 3'd0) & (r_cs == 3'd0) ? s_suspend_1d : ((r_cs!= 3'd0) & (w_cs == 3'd0))? ( s_suspend_flag & o_r_strans_done_data) :
                       ((w_cs != 3'd0) & (r_cs == 3'd0))? ( s_suspend_flag & o_w_strans_done_data):1'b0 ;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            w_cs <= IDLE ;
        end
        else begin
            w_cs <=   w_ns ;
        end
    end

    always@(*)
    begin
        w_ns=w_cs;
        case(w_cs)
            IDLE : begin
                w_ns= (i_w_strans_len==8'b0)        ? IDLE        :
                      (w_burst_en)                  ? BURST_START : SINGLE ;
            end
            SINGLE : begin
                w_ns= (~i_w_hready)                 ? SINGLE      :
                      (w_burst_en)                  ? BURST_START :
                      (i_w_strans_len==8'b0)        ? DONE        : SINGLE ;
            end
            BURST_START : begin
                w_ns= (~i_w_hready)                 ? BURST_START : BURST_CONT ;
            end
            BURST_CONT : begin
                w_ns= (~i_w_hready)                 ? BURST_CONT  :
                      (w_burst_cnt!=5'b1)           ? BURST_CONT  :
                      (w_burst_en)                  ? BURST_START :                      
                      (i_w_strans_len==8'b0)        ? DONE        : SINGLE ;
            end
            DONE : begin
                w_ns= (~i_w_hready)                 ? DONE        : IDLE   ;
            end
            default:w_ns=IDLE;
        endcase
    end

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            w_haddr  <=  {(p_ADDR_WIDTH){1'b0}};
            w_hwdata <=  {(p_DATA_WIDTH){1'b0}};
            w_hburst <=  3'b0                  ;
            w_hsize  <=  2'b0                  ;
            w_htrans <=  2'b0                  ;
        end
        else begin
            w_haddr  <=    w_haddr_in ;
            w_hwdata <=    w_hwdata_in;
            w_hburst <=    w_hburst_in;
            w_hsize  <=    w_hsize_in ;
            w_htrans <=    w_htrans_in;
        end
    end

    assign w_haddr_in     = ((w_cs==IDLE)&(i_w_strans_len!=8'b0))|
                            ((w_cs!=IDLE)&(i_w_hready))            ? i_w_base_addr : w_haddr ;

    assign w_hwdata_in    = ((w_cs!=IDLE)&(i_w_hready))            ? w_revr_out : w_hwdata ;

    assign w_hsize_in     = ((w_cs==IDLE)&(i_w_strans_len!=8'b0))| 
                            ((w_cs!=IDLE)&(i_w_hready))             ? w_hsize_dec : w_hsize ;

    assign w_htrans_in    = ((w_cs==IDLE)&(i_w_strans_len!=8'b0))|
                            ((w_cs==BURST_CONT)&(i_w_hready)&
                             (w_burst_cnt==5'h1)&(i_w_strans_len!=8'b0)) ? 2'b10 :
                            ((w_cs==SINGLE)|(w_cs==BURST_CONT))&
                            (i_w_strans_len==8'b0)&(i_w_hready)          ? 2'b00 :
                            ((w_cs==BURST_START)&(i_w_hready))           ? 2'b11 : w_htrans ;

    assign w_align_init   = (w_cs==IDLE)&(i_w_strans_len!=8'b0);        
generate
if(p_AHB_SIMPLE) begin:w_logic_smp

        assign w_burst_dist       = 0;
        assign w_burst_dist_small = 0;
        assign w_burst_single     = 0;
        assign w_burst_cnt_in     = 0;
        assign w_hburst_dec_org   = 0;

        assign w_hburst_dec       = 0;
        assign w_hburst_in        = 0;       
        assign o_w_hburst         = 0;         
end
else begin:w_logic_normal

        assign w_burst_dist       = ({1'b1,{{9-p_DATA_WIDTH/32}{1'b0}}}-{1'b0,i_w_base_addr[9:(p_DATA_WIDTH/32+1)]}) ;

        assign w_burst_dist_small = ({3'b0,i_w_strans_len[7:(p_DATA_WIDTH/32+1)]}>w_burst_dist) ? w_burst_dist[(6-p_DATA_WIDTH/32):0] : i_w_strans_len[7:(p_DATA_WIDTH/32+1)] ;

        assign w_burst_single     = (w_burst_dist_small>15) ? 5'h10 :
                                    (w_burst_dist_small>7 ) ? 5'h08 :
                                    (w_burst_dist_small>3 ) ? 5'h04 :
                                    (w_burst_dist_small>1 ) ? {3'b0,w_burst_dist_small[1:0]} : 5'b0 ;

        assign w_burst_cnt_in     = (w_cs==DONE)&(i_w_hready)                                          ? 5'h0             :
                                    ((w_cs==IDLE)&(i_w_strans_len!=8'b0)&(w_burst_en)               )|
                                    ((w_cs==SINGLE)&(i_w_hready)&(w_burst_en)                       )|
                                    ((w_cs==BURST_CONT)&(i_w_hready)&(w_burst_cnt==5'b1)&(w_burst_en)) ? w_burst_single   :
                                    ((w_cs==BURST_CONT)|(w_cs==BURST_START))&(i_w_hready)              ? w_burst_cnt-5'h1 : w_burst_cnt ;

        assign w_hburst_dec_org   = (w_burst_dist_small>15) ? 3'b111 :
                                    (w_burst_dist_small>7 ) ? 3'b101 :
                                    (w_burst_dist_small>3 ) ? 3'b011 :
                                    (w_burst_dist_small>1 ) ? 3'b001 : 3'b000 ;

        assign w_hburst_dec       = w_hburst_dec_org;

        assign w_hburst_in    = (i_ch_w_atyp)                                                         ? 3'b0             :
                                (w_cs==IDLE)&(i_w_strans_len!=8'b0)&(~w_burst_en)                     ? 3'b0             :
                                ((w_cs==IDLE)&(i_w_strans_len!=8'b0)&(w_burst_en)               )|
                                ((w_cs==SINGLE)&(i_w_hready)&(w_burst_en)                       )|
                                ((w_cs==BURST_CONT)&(i_w_hready)&(w_burst_cnt==5'b1)&(w_burst_en))    ? w_hburst_dec_org :
                                (w_cs==BURST_CONT)&(i_w_hready)&(w_burst_cnt==5'b1)&(~w_burst_en)     ? 3'b0             : w_hburst ;

        assign o_w_hburst         =  w_hburst  ; 
end
endgenerate

generate
    if(p_DATA_WIDTH==32) begin:ahb_wlogic_32b
        assign w_burst_en     = ((i_w_base_addr[1:0]==2'b00)&(w_burst_single>5'h1)&(~i_ch_w_atyp));

        assign w_trans_num    = ((i_w_base_addr[1:0]==2'b00)&(i_w_strans_len>8'h3)) ? 3'h4 : 
                                ((i_w_base_addr[0]==1'b0   )&(i_w_strans_len>8'h1)) ? 3'h2 : 
                                (i_w_strans_len>8'h0)                               ? 3'h1 : 3'h0 ;

        assign w_hsize_dec    = (w_trans_num==3'h4) ? 2'b10 : 
                                (w_trans_num==3'h2) ? 2'b01 : 
                                (w_trans_num==3'h1) ? 2'b00 : 2'b00 ;
    end
    else begin:ahb_wlogic_64b
        assign w_burst_en   = ((i_w_base_addr[2:0]==3'b0)&(w_burst_single>5'h1)&(~i_ch_w_atyp));

        assign w_trans_num  = ((i_w_base_addr[2:0]==3'b0)&(i_w_strans_len>8'h7)) ? 4'h8 : 
                              ((i_w_base_addr[1:0]==2'b0)&(i_w_strans_len>8'h3)) ? 4'h4 : 
                              ((i_w_base_addr[0]==1'b0  )&(i_w_strans_len>8'h1)) ? 4'h2 : 
                              (i_w_strans_len>8'h0)                              ? 4'h1 : 4'h0 ;

        assign w_hsize_dec  = (w_trans_num==4'h8) ? 2'b11 :
                              (w_trans_num==4'h4) ? 2'b10 : 
                              (w_trans_num==4'h2) ? 2'b01 : 
                              (w_trans_num==4'h1) ? 2'b00 : 2'b00 ;

    end
endgenerate

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            w_burst_cnt <= 5'b0 ;

        end
        else begin
            w_burst_cnt <=   w_burst_cnt_in ;
        end
    end

    assign o_w_haddr  =   w_haddr       ;

    assign o_w_htrans =   w_htrans      ;
    assign o_w_hsize  =   w_hsize       ;
    assign o_w_hwdata =   w_hwdata      ;
    assign o_w_hprot  =   i_ch_bus[7:4] ;
    assign o_w_hwrite =   1'b1          ; 

    assign w_revr_in  = w_align_out ;
    assign w_align_in = i_w_data ;

    assign w_align_buf_upd_fifo =  w_align_init|
                                   ((w_cs==SINGLE)&(w_burst_en)&(i_w_hready))|
                                   (((w_cs==BURST_START)|(w_cs==BURST_CONT))&(w_burst_en)&(i_w_hready))|
                                   (((w_haddr_in[(p_DATA_WIDTH/32):0]==0)&(i_w_strans_len>{{{7-p_DATA_WIDTH/32}{1'b0}},w_align_sel}))&(i_w_hready)&(w_cs!=DONE)&(w_cs!=IDLE));

    assign w_align_buf_upd      =  w_align_init|
                                   ((w_cs==SINGLE)&(w_burst_en)&(i_w_hready))|
                                   (((w_cs==BURST_START)|(w_cs==BURST_CONT))&(w_burst_en)&(i_w_hready))|
                                   ((w_haddr_in[(p_DATA_WIDTH/32):0]==0)&(i_w_hready)&(w_cs!=DONE)&(w_cs!=IDLE));

    assign w_align_buf_in       = (w_align_buf_upd) ? w_align_in : w_align_buf ;

    assign w_align_sel_in       =  w_align_init ? i_w_base_addr[(p_DATA_WIDTH/32):0] : w_align_sel ;
    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            w_align_buf <= {(p_DATA_WIDTH){1'b0}} ;
            w_align_sel <= {(p_DATA_WIDTH/32+1){1'b0}};
        end
        else begin
            w_align_buf <=   w_align_buf_in ;
            w_align_sel <=   w_align_sel_in ;
        end
    end

generate
    if(p_DATA_WIDTH==32) begin:ahb_walign_32b
        assign w_revr_out = (i_w_revr_en) ? {w_revr_in[7:0],w_revr_in[15:8],w_revr_in[23:16],w_revr_in[31:24]} : w_revr_in ;

        always@(*)
        begin
            w_align_out = w_align_buf ;
            case(w_align_sel)
                2'b00 : begin
                    w_align_out = {w_align_in} ;
                end
                2'b01 : begin
                    w_align_out = {w_align_in[23: 0],w_align_buf[31:24]} ;
                end
                2'b10 : begin
                    w_align_out = {w_align_in[15: 0],w_align_buf[31:16]} ;
                end
                2'b11: begin
                    w_align_out = {w_align_in[ 7: 0],w_align_buf[31: 8]} ;
                end
                default:begin
                    w_align_out = w_align_in ;
                end
            endcase
        end

    end
    else begin:ahb_walign_64b
        assign w_revr_out = (i_w_revr_en) ? {w_revr_in[ 7: 0],w_revr_in[15: 8],w_revr_in[23:16],w_revr_in[31:24],
                                             w_revr_in[39:32],w_revr_in[47:40],w_revr_in[55:48],w_revr_in[63:56]} : w_revr_in ;

        always@(*)
        begin
            w_align_out = w_align_buf ;
            case(w_align_sel)
                3'h0 : begin
                    w_align_out = {w_align_in} ;
                end
                3'h1 : begin
                    w_align_out = {w_align_in[55: 0],w_align_buf[63:56]} ;
                end
                3'h2 : begin
                    w_align_out = {w_align_in[47: 0],w_align_buf[63:48]} ;
                end
                3'h3 : begin
                    w_align_out = {w_align_in[39: 0],w_align_buf[63:40]} ;
                end
                3'h4 : begin
                    w_align_out = {w_align_in[31: 0],w_align_buf[63:32]} ;
                end
                3'h5 : begin
                    w_align_out = {w_align_in[23: 0],w_align_buf[63:24]} ;
                end
                3'h6 : begin
                    w_align_out = {w_align_in[15: 0],w_align_buf[63:16]} ;
                end
                3'h7 : begin
                    w_align_out = {w_align_in[ 7: 0],w_align_buf[63: 8]} ;
                end
                default:begin
                    w_align_out = w_align_in ;
                end
            endcase
        end
    end
endgenerate

        assign w_to_cnt_in   =(~i_w_hready)&o_w_htrans[1] ? w_to_cnt+1'b1 : 8'b0 ;

        always@(posedge i_m_clk or negedge i_m_rst_n)
        begin
            if(~i_m_rst_n) begin
                w_to_cnt    <=  8'b0 ;
            end
            else begin
                w_to_cnt    <=   w_to_cnt_in ;
            end
        end
        assign o_w_to           = (w_to_cnt==i_ch_to)&(w_to_cnt!=0);

        assign o_w_base_addr_ns =(i_ch_w_atyp)                             ? i_w_base_addr              :
                                 ((w_cs==IDLE)&(i_w_strans_len!=8'b0))|
                                 ((w_cs!=IDLE)&(w_cs!=DONE)&(i_w_hready))  ? i_w_base_addr +w_trans_num : i_w_base_addr ;

        assign o_w_trans_len_ns =((w_cs==IDLE)|((w_cs!=IDLE)&(w_cs!=DONE)&(i_w_hready)))&
                                 (i_w_strans_len!=8'b0) ? i_w_trans_len -w_trans_num : i_w_trans_len ;

        assign o_w_strans_len_ns=((w_cs==IDLE)|((w_cs!=IDLE)&(w_cs!=DONE)&(i_w_hready)))&
                                 (i_w_strans_len!=8'b0) ? i_w_strans_len-w_trans_num : i_w_strans_len ;

        assign o_w_rd_en            =  w_align_buf_upd_fifo;
        assign o_w_strans_done_addr = ((w_cs==BURST_CONT)|(w_cs==SINGLE))&(~(|i_w_strans_len))&i_w_hready;
        assign o_w_trans_done_addr  = (~(|i_w_trans_len))&o_w_strans_done_addr;
        assign o_w_strans_done_data = (w_cs==DONE)&i_w_hready;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            r_ls <= IDLE ;
        end
        else begin
            r_ls <=   r_ls_in ;
        end
    end

    assign r_ls_in = (r_cs!=IDLE)&(i_r_hready) ? r_cs :
                     (r_cs==IDLE)              ? IDLE : r_ls ;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            r_cs <= IDLE ;
        end
        else begin
            r_cs <=   r_ns ;
        end
    end

    always@(*)
    begin
        r_ns=r_cs;
        case(r_cs)
            IDLE : begin
                r_ns= (i_r_strans_len_burst==0)     ? IDLE        :
                      (r_burst_en)                  ? BURST_START : SINGLE ;
            end
            SINGLE : begin
                r_ns= (~i_r_hready)                 ? SINGLE      :
                      (r_burst_en)                  ? BURST_START :
                      (i_r_strans_len_burst==0)     ? DONE        : SINGLE ;
            end
            BURST_START : begin
                r_ns= (~i_r_hready)                 ? BURST_START : BURST_CONT ;
            end
            BURST_CONT : begin
                r_ns= (~i_r_hready)                 ? BURST_CONT  :
                      (r_burst_cnt!=5'b1)           ? BURST_CONT  :
                      (r_burst_en)                  ? BURST_START :                      
                      (i_r_strans_len_burst==0)     ? DONE        : SINGLE ;
            end
            DONE : begin
                r_ns= (~i_r_hready)                 ? DONE        : IDLE   ;
            end
            default:r_ns=IDLE;            
        endcase
    end

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            r_haddr  <=  {(p_ADDR_WIDTH){1'b0}};
            r_hburst <=  3'b0                  ;
            r_htrans <=  2'b0                  ;
        end
        else begin
            r_haddr  <=    r_haddr_in ;
            r_hburst <=    r_hburst_in;
            r_htrans <=    r_htrans_in;
        end
    end

    assign r_haddr_in     = ((r_cs==IDLE)&(i_r_strans_len_burst!=0))|
                            ((r_cs==DONE)&(i_r_hready))|(i_ch_r_atyp)    ? i_r_base_addr       :

                            ((r_cs!=IDLE)&(i_r_hready))                  ? r_base_addr_unalign_compare : r_haddr ;

    assign r_htrans_in    = ((r_cs==IDLE)&(i_r_strans_len_burst!=0))|
                            ((r_cs==BURST_CONT)&(i_r_hready)&
                             (r_burst_cnt==5'h1)&(i_r_strans_len_burst!=0)) ? 2'b10 :
                            ((r_cs==BURST_CONT)|(r_cs==SINGLE))&
                            (i_r_strans_len_burst==0)&(i_r_hready)          ? 2'b00 :
                            ((r_cs==BURST_START)&(i_r_hready))              ? 2'b11 : r_htrans ;

    assign r_align_init   = (r_cs==IDLE)&(i_r_strans_len_burst!=0);        

    assign r_burst_en   = (r_burst_single>5'h1)&(~i_ch_r_atyp);

    assign r_trans_num  = {1'b1,{{p_DATA_WIDTH/32+1}{1'b0}}};   

generate
if(p_AHB_SIMPLE) begin:r_logic_smp

        assign r_burst_dist       = 0;
        assign r_burst_dist_small = 0;
        assign r_burst_single     = 0;
        assign r_burst_cnt_in     = 0;
        assign r_hburst_dec_org   = 0;

        assign r_hburst_dec       = 0;
        assign r_hburst_in        = 0;
        assign o_r_hburst         = 0;        
end
else begin:r_logic_normal

        assign r_burst_dist       = (r_cs==IDLE) ? ({{{(p_DATA_WIDTH/32)-1}{1'b0}},1'b1,{{9-p_DATA_WIDTH/32}{1'b0}}}-{{(p_DATA_WIDTH/32){1'b0}},i_r_base_addr[9:(p_DATA_WIDTH/32+1)]}) :
                                                   ({{{(p_DATA_WIDTH/32)-1}{1'b0}},1'b1,{{9-p_DATA_WIDTH/32}{1'b0}}}-{{(p_DATA_WIDTH/32){1'b0}},r_base_addr_unalign[9:(p_DATA_WIDTH/32+1)]}) ;

        assign r_burst_dist_small = ({{{8-(7-p_DATA_WIDTH/32)}{1'b0}},i_r_strans_len_burst}>r_burst_dist) ? {{{(p_DATA_WIDTH/32)-1}{1'b0}},r_burst_dist[(7-p_DATA_WIDTH/32):0]} :
                                                                                                            {{{(p_DATA_WIDTH/32)-1}{1'b0}},i_r_strans_len_burst} ;

        assign r_burst_single     = (r_burst_dist_small>15) ? 5'h10 :
                                    (r_burst_dist_small>7 ) ? 5'h08 :
                                    (r_burst_dist_small>3 ) ? 5'h04 :
                                    (r_burst_dist_small>1 ) ? {3'b0,r_burst_dist_small[1:0]} : 5'b0 ;

        assign r_burst_cnt_in     = (r_cs==DONE)&(i_r_hready)                                          ? 5'h0             :
                                    ((r_cs==IDLE)&(i_r_strans_len_burst!=0)&(r_burst_en)             )|
                                    ((r_cs==SINGLE)&(i_r_hready)&(r_burst_en)                        )|
                                    ((r_cs==BURST_CONT)&(i_r_hready)&(r_burst_cnt==5'b1)&(r_burst_en)) ? r_burst_single   :
                                    ((r_cs==BURST_CONT)|(r_cs==BURST_START))&(i_r_hready)              ? r_burst_cnt-5'h1 : r_burst_cnt ;

        assign r_hburst_dec_org   = (r_burst_dist_small>15) ? 3'b111 :
                                    (r_burst_dist_small>7 ) ? 3'b101 :
                                    (r_burst_dist_small>3 ) ? 3'b011 :
                                    (r_burst_dist_small>1 ) ? 3'b001 : 3'b000 ;

        assign r_hburst_dec       = r_hburst_dec_org ;

        assign r_hburst_in    = (i_ch_r_atyp)                                                      ? 3'b0             :
                                (r_cs==IDLE)&(i_r_strans_len_burst!=0)&(~r_burst_en)               ? 3'b0             :
                                ((r_cs==IDLE)&(i_r_strans_len_burst!=0)&(r_burst_en)         )|
                                ((r_cs==SINGLE)&(i_r_hready)&(r_burst_en)                       )|
                                ((r_cs==BURST_CONT)&(i_r_hready)&(r_burst_cnt==5'b1)&(r_burst_en)) ? r_hburst_dec_org :
                                (r_cs==BURST_CONT)&(i_r_hready)&(r_burst_cnt==5'b1)&(~r_burst_en)  ? 3'b0             : r_hburst ;

        assign o_r_hburst     = r_hburst ;         
end
endgenerate

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            r_burst_cnt <= 5'b0 ;
        end
        else begin
            r_burst_cnt <=   r_burst_cnt_in ;
        end
    end

    assign o_r_haddr  = {r_haddr[p_ADDR_WIDTH-1:p_DATA_WIDTH/32+1],{(p_DATA_WIDTH/32+1){1'b0}}} ;
    assign o_r_htrans = r_htrans;
    assign o_r_hwdata = {p_DATA_WIDTH{1'b0}}; 
    assign o_r_hprot  = i_ch_bus[3:0] ;
    assign o_r_hwrite = 1'b0          ;

generate
    if(p_DATA_WIDTH==32) begin:ahb_rsize_32b
        assign o_r_hsize  =   2'b10 ;
    end
    else begin:ahb_rsize_64b
        assign o_r_hsize  =   2'b11 ;
    end
endgenerate

    assign r_revr_in  = i_r_hrdata ;

        assign r_align_in      = r_revr_out ;    

        assign r_align_buf_upd = i_r_hready&(r_cs!=IDLE);
        assign r_align_buf_in  = (r_align_buf_upd) ? r_align_in : r_align_buf ;
        assign r_align_rdy_in  = (r_cs==IDLE)                                                 ? 1'b0 :
                                 (r_cs!=IDLE)&(i_r_hready)&(r_align_sel_in==0)                ? 1'b1 :
                                 (r_ls!=IDLE)&(r_cs!=IDLE)&(r_align_sel_in!=0)&i_r_hready     ? 1'b1 : r_align_rdy ;

        assign r_align_sel_in  = r_align_init ? i_r_base_addr[(p_DATA_WIDTH/32):0] : r_align_sel ;

        assign r_align_out_vld = (i_r_hready&r_align_rdy)&(r_cs!=IDLE);

        always@(posedge i_m_clk or negedge i_m_rst_n)
        begin
            if(~i_m_rst_n) begin
                r_align_rdy          <=  1'b0 ;
                r_align_buf          <=  {(p_DATA_WIDTH){1'b0}};            
                r_align_sel          <=  {(p_DATA_WIDTH/32+1){1'b0}} ;
            end
            else begin
                r_align_rdy          <=   r_align_rdy_in ;
                r_align_buf          <=   r_align_buf_in ;
                r_align_sel          <=   r_align_sel_in ;
            end
        end

generate
    if(p_DATA_WIDTH==32) begin:ahb_ralign_32b
        assign r_revr_out = (i_r_revr_en) ? {r_revr_in[7:0],r_revr_in[15:8],r_revr_in[23:16],r_revr_in[31:24]} : r_revr_in ;

        always@(*)
        begin
            r_align_out = r_align_buf ;
            case(r_align_sel)
                2'h0 : begin
                    r_align_out = {r_align_in} ;
                end                                
                2'h1 : begin                       
                    r_align_out = {r_align_in[ 7: 0],r_align_buf[31: 8]} ;
                end                             
                2'h2 : begin                    
                    r_align_out = {r_align_in[15: 0],r_align_buf[31:16]} ;
                end                             
                2'h3 : begin                    
                    r_align_out = {r_align_in[23: 0],r_align_buf[31:24]} ;
                end
                default:begin
                    r_align_out = r_align_buf ;
                end
            endcase
        end

    end
    else begin:ahb_ralign_64b

        assign r_revr_out = (i_r_revr_en) ? {r_revr_in[ 7: 0],r_revr_in[15: 8],r_revr_in[23:16],r_revr_in[31:24],
                                             r_revr_in[39:32],r_revr_in[47:40],r_revr_in[55:48],r_revr_in[63:56]} : r_revr_in ;

        always@(*)
        begin
            r_align_out = r_align_buf ;
            case(r_align_sel)
                3'h0 : begin
                    r_align_out = {r_align_in} ;
                end
                3'h1 : begin
                    r_align_out = {r_align_in[ 7: 0],r_align_buf[63: 8]} ;
                end
                3'h2 : begin
                    r_align_out = {r_align_in[15: 0],r_align_buf[63:16]} ;
                end
                3'h3 : begin
                    r_align_out = {r_align_in[23: 0],r_align_buf[63:24]} ;
                end
                3'h4 : begin
                    r_align_out = {r_align_in[31: 0],r_align_buf[63:32]} ;
                end
                3'h5 : begin
                    r_align_out = {r_align_in[39: 0],r_align_buf[63:40]} ;
                end
                3'h6 : begin
                    r_align_out = {r_align_in[47: 0],r_align_buf[63:48]} ;
                end
                3'h7 : begin
                    r_align_out = {r_align_in[55: 0],r_align_buf[63:56]} ;
                end
                default:begin
                    r_align_out = r_align_buf ;
                end
            endcase
        end
    end
endgenerate

    assign r_to_cnt_in   = (~i_r_hready)&o_r_htrans[1] ? r_to_cnt+8'h1 : 8'b0 ;
    assign r_strans_tail_mask_in = ((i_r_strans_len_burst!=0) && (r_cs==IDLE)) ? {{(p_DATA_WIDTH/8-ADDR_OFFSET_WIDTH){1'h0}},i_r_strans_len[ADDR_OFFSET_WIDTH-1:0]} : r_strans_tail_mask;

    always@(posedge i_m_clk or negedge i_m_rst_n)
    begin
        if(~i_m_rst_n) begin
            r_to_cnt            <=  8'b0 ;
            r_base_addr_unalign <= {p_ADDR_WIDTH{1'b0}};
            r_strans_tail_mask  <= {p_DATA_WIDTH/8{1'b1}};
        end
        else begin
            r_to_cnt            <=   r_to_cnt_in ;
            r_base_addr_unalign <=   r_base_addr_unalign_in ;
            r_strans_tail_mask  <=   r_strans_tail_mask_in;
        end
    end
    assign r_align_out_mask_post = ((r_cs==DONE)&&i_r_hready) ? r_strans_tail_mask_post : {(p_DATA_WIDTH/8){1'b1}} ;
generate    
if(1) begin:o_r_data_mask_gen
    for(i=0;i<p_DATA_WIDTH/8;i=i+1) begin:o_r_data_mask_logic
        assign r_strans_tail_mask_post[i]= (r_strans_tail_mask>i)|(r_strans_tail_mask==0);
        assign r_align_out_mask[i*8+:8]= {(8){r_align_out_mask_post[i]}};
    end
end
endgenerate

    assign r_base_addr_unalign_in = ((r_cs==IDLE)&(i_r_strans_len_burst!=0)) ? i_r_base_addr +r_trans_num :
                                    ((r_cs!=IDLE)&(i_r_hready))              ? r_base_addr_unalign +r_trans_num : r_base_addr_unalign ;

    assign r_base_addr_unalign_compare = (r_base_addr_unalign <= i_r_ch_saddr_max) ? r_base_addr_unalign : i_r_ch_saddr_max ;

    assign o_r_to=(r_to_cnt==i_ch_to)&(r_to_cnt!=8'b0);

    assign o_r_base_addr_ns =(i_ch_r_atyp)                          ? i_r_base_addr                :
                             (r_cs==IDLE)&(i_r_strans_len_burst!=0) ? i_r_base_addr+i_r_strans_len : i_r_base_addr ;

    assign o_r_trans_len_ns =(r_cs==IDLE)&(i_r_strans_len_burst!=0) ? i_r_trans_len-i_r_strans_len : i_r_trans_len ;

    assign o_r_strans_len_burst_ns = ((r_cs==IDLE)|((r_cs!=IDLE)&(r_cs!=DONE)&(i_r_hready)))&
                                     (i_r_strans_len_burst!=0) ? i_r_strans_len_burst-1 : i_r_strans_len_burst ;

    assign o_r_wr_en            = r_align_out_vld;
    assign o_r_data             = r_align_out&r_align_out_mask;
    assign o_r_strans_done_addr = ((r_cs==BURST_CONT)|(r_cs==SINGLE))&(i_r_strans_len_burst==0)&i_r_hready;
    assign o_r_trans_done_addr  = (i_r_trans_len==0)&o_r_strans_done_addr;
    assign o_r_strans_done_data = (r_cs==DONE)&i_r_hready;

endmodule
