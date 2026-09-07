//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_axi_dmac_mif_rd #(
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
    input   wire [7:0]                    i_dma_rost_max    ,
    input   wire [7:0]                    i_dma_rburst_max  ,
    output  wire                          o_r_rato          ,
    output  wire                          o_r_rdto          ,
    output  wire                          o_r_bus_err       ,

    input   wire [2*p_BUS_AWIDTH+61:0]    i_r_abus_data     ,
    input   wire                          i_r_abus_vld      ,
    output  wire                          o_r_abus_rdy      ,

    output  wire                          o_r_dbus_vld      ,
    output  wire [7:0]                    o_r_dbus_rid      ,
    output  wire [p_BUS_DWIDTH-1:0]       o_r_dbus_data     ,
    output  wire                          o_r_dbus_last     ,

    output  wire [ 1:0]                   o_r_arbar         ,
    output  wire [ 3:0]                   o_r_arsnoop       ,
    output  wire [ 1:0]                   o_r_ardomain      ,

    output  wire [13:0]                   o_r_arid          ,
    output  wire [p_BUS_AWIDTH-1:0]       o_r_araddr        ,
    output  wire [ 7:0]                   o_r_arlen         ,
    output  wire [ 2:0]                   o_r_arsize        ,
    output  wire [ 1:0]                   o_r_arburst       ,
    output  wire [ 1:0]                   o_r_arlock        ,
    output  wire [ 3:0]                   o_r_arcache       ,
    output  wire [ 2:0]                   o_r_arprot        ,
    output  wire [ 3:0]                   o_r_arqos         ,
    output  wire [ 3:0]                   o_r_arregion      ,
    output  wire                          o_r_arvalid       ,
    input   wire                          i_r_arready       ,

    input   wire [ 7:0]                   i_r_rid           ,
    input   wire [p_BUS_DWIDTH-1:0]       i_r_rdata         ,
    input   wire [ 1:0]                   i_r_rresp         ,
    input   wire                          i_r_rlast         ,
    input   wire                          i_r_rvalid        ,
    output  wire                          o_r_rready        
);
    localparam [2:0]ADDR_OFFSET_WIDTH = (p_BUS_DWIDTH/8<=4  ) ? 3'd2 :    
                                        (p_BUS_DWIDTH/8<=8  ) ? 3'd3 :    
                                        (p_BUS_DWIDTH/8<=16 ) ? 3'd4 :    
                                        (p_BUS_DWIDTH/8<=32 ) ? 3'd5 :    
                                        (p_BUS_DWIDTH/8<=64 ) ? 3'd6 : 3'd0 ;

    localparam [ 1:0] RA_IDLE                = 2'd0;
    localparam [ 1:0] RA_PRE                 = 2'd1;
    localparam [ 1:0] RA_VALID               = 2'd2;
    localparam [ 1:0] RA_SUSPEND             = 2'd3;

    localparam [ 0:0] RD_IDLE                = 1'b0;
    localparam [ 0:0] RD_READY               = 1'b1;

    wire                         fifo_wr                   ; 
    wire [2*p_BUS_AWIDTH+61:0]   fifo_wdata                ;
    wire                         fifo_full_n               ; 

    wire                         fifo_rd                   ; 
    wire [2*p_BUS_AWIDTH+61:0]   fifo_rdata                ;
    wire                         fifo_empty_n              ;

    reg                          fifo_dst_vld              ;
    wire                         fifo_dst_vld_in           ;
    wire                         fifo_dst_rdy              ;

    wire [p_BUS_AWIDTH-1:0]      ch_raddr_max              ;
    wire [p_BUS_AWIDTH-1:0]      ch_raddr                  ;
    wire [15:0]                  ch_sburst_num             ;
    wire [7:0]                   ch_rid                    ;
    wire [31:0]                  ch_arbus                  ;
    wire [5:0]                   ch_r_qid                  ;

    reg  [15:0]                  r_sburst_num              ;
    wire [15:0]                  r_sburst_num_in           ;
    wire [15:0]                  r_sburst_num_ns           ;

    reg                          r_dbus_vld                ; 
    reg  [7:0]                   r_dbus_rid                ; 
    reg  [p_BUS_DWIDTH-1:0]      r_dbus_data               ; 
    reg                          r_dbus_last               ;
    reg  [1:0]                   r_rresp                   ;

    wire                         r_dbus_vld_in             ; 
    wire [7:0]                   r_dbus_rid_in             ; 
    wire [p_BUS_DWIDTH-1:0]      r_dbus_data_in            ; 
    wire                         r_dbus_last_in            ; 
    wire [1:0]                   r_rresp_in                ;

    genvar                       i                         ;
    genvar                       j                         ;
    integer                      k                         ;
    reg  [1:0]                   ra_cs                     ;
    reg  [1:0]                   ra_ns                     ;

    reg                          rd_cs                     ;
    reg                          rd_ns                     ;

    reg  [p_R_BMEM_AWIDTH:0]     r_ost_cnt                 ;
    wire [p_R_BMEM_AWIDTH:0]     r_ost_cnt_in              ;
    wire                         r_ost_cnt_inc             ;
    wire                         r_ost_cnt_dec             ;

    wire                         r_ost_full                ;
    wire                         r_ost_al_full             ;
    wire                         r_ost_fifo_vld            ;

    wire                         r_ost_al_empty            ;

    wire [12:0]                  r_burst_bond_dist_pre     ;
    wire [12-ADDR_OFFSET_WIDTH:0]r_burst_bond_dist         ;
    wire [8:0]                   r_burst_max               ;
    wire [8:0]                   r_burst_num_small         ;


    wire                         r_burst_cover             ;

    wire [8:0]                   r_burst_num               ;

    wire [p_BUS_AWIDTH-1:0]      r_araddr_compare          ;
    reg  [p_BUS_AWIDTH-1:0]      r_araddr_pre              ;
    wire [p_BUS_AWIDTH-1:0]      r_araddr_pre_in           ;
    wire [p_BUS_AWIDTH-1:0]      r_araddr_pre_ns           ;

    reg  [ 1:0]                  r_arbar                   ; 
    reg  [ 3:0]                  r_arsnoop                 ; 
    reg  [ 1:0]                  r_ardomain                ; 
    reg  [13:0]                  r_arid                    ; 
    reg  [p_BUS_AWIDTH-1:0]      r_araddr                  ; 
    reg  [ 7:0]                  r_arlen                   ; 
    wire [ 2:0]                  r_arsize                  ; 
    wire [ 1:0]                  r_arburst                 ; 
    reg  [ 1:0]                  r_arlock                  ; 
    reg  [ 3:0]                  r_arcache                 ; 
    reg  [ 2:0]                  r_arprot                  ; 
    reg  [ 3:0]                  r_arqos                   ; 
    reg  [ 3:0]                  r_arregion                ; 
    reg                          r_arvalid                 ;
    wire [ 1:0]                  r_arbar_in                ; 
    wire [ 3:0]                  r_arsnoop_in              ; 
    wire [ 1:0]                  r_ardomain_in             ; 
    wire [13:0]                  r_arid_in                 ;
    wire [p_BUS_AWIDTH-1:0]      r_araddr_in               ;

    wire [ 7:0]                  r_arlen_in                ;
    wire [ 8:0]                  r_arlen_post              ; 

    wire [ 1:0]                  r_arlock_in               ; 
    wire [ 3:0]                  r_arcache_in              ; 
    wire [ 2:0]                  r_arprot_in               ; 
    wire [ 3:0]                  r_arqos_in                ; 
    wire [ 3:0]                  r_arregion_in             ; 
    wire                         r_arvalid_in              ; 

    reg                          r_rready                  ;
    wire                         r_rready_in               ;

    wire                         ra_to                     ;
    reg [15:0]                   ra_to_cnt                 ;
    wire[15:0]                   ra_to_cnt_in              ;
    wire                         rd_to                     ;
    reg [15:0]                   rd_to_cnt                 ;
    wire[15:0]                   rd_to_cnt_in              ;

    wire [ 7:0]                  r_dma_ost_max_small       ;
    wire [ 7:0]                  r_dma_ost_max_small_inc   ;

    assign fifo_wr = i_r_abus_vld&(fifo_full_n);

    assign fifo_wdata = i_r_abus_data ;

    assign o_r_abus_rdy = (fifo_full_n) ;

    osr_axi_dmac_sync_fifo #(
        .p_AWIDTH       (1),
        .p_DWIDTH       (2*p_BUS_AWIDTH+62 )
    ) u_r_abus_fifo(
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

    assign fifo_dst_rdy  = (ra_cs==RA_VALID)&(r_sburst_num==0)&i_r_arready ;

    assign ch_raddr      = fifo_rdata[p_BUS_AWIDTH-1:0];
    assign ch_sburst_num = fifo_rdata[p_BUS_AWIDTH+:16];
    assign ch_rid        = fifo_rdata[(p_BUS_AWIDTH+16)+:8];
    assign ch_arbus      = fifo_rdata[(p_BUS_AWIDTH+24)+:32];
    assign ch_r_qid      = fifo_rdata[(p_BUS_AWIDTH+56)+:6 ];
    assign ch_raddr_max  = fifo_rdata[(p_BUS_AWIDTH+62)+:p_BUS_AWIDTH];

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            ra_to_cnt <= 16'b0 ;
            rd_to_cnt <= 16'b0 ;
        end
        else begin
            ra_to_cnt <=  ra_to_cnt_in ;
            rd_to_cnt <=  rd_to_cnt_in ;
        end
    end

    assign ra_to_cnt_in = (ra_cs==RA_VALID)&i_r_arready    ? 16'b0          : 
                          (ra_cs==RA_VALID)&(!i_r_arready) ? ra_to_cnt+1'b1 : ra_to_cnt ;

    assign ra_to        = (ra_to_cnt==i_dma_to)&(i_dma_to!=0);

    assign rd_to_cnt_in = (rd_cs==RD_READY)&i_r_rvalid     ? 16'b0          : 
                          (rd_cs==RD_READY)&(!i_r_rvalid)  ? rd_to_cnt+1'b1 : rd_to_cnt ;

    assign rd_to        = (rd_to_cnt==i_dma_to)&(i_dma_to!=0);

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            ra_cs <= RA_IDLE;
        end
        else begin
            ra_cs <=  ra_ns ;
        end
    end

    always@(*)
    begin
        ra_ns=ra_cs;
        case(ra_cs)
            RA_IDLE    : begin
                ra_ns = (fifo_dst_vld) ? RA_PRE : RA_IDLE ;
            end
            RA_PRE     : begin
                ra_ns = (r_ost_fifo_vld) ? RA_VALID : RA_PRE ;
            end
            RA_VALID   : begin
                ra_ns = (r_sburst_num==0)&i_r_arready                ? RA_IDLE    : 
                        (r_ost_al_full)&(~r_ost_cnt_dec)&i_r_arready ? RA_SUSPEND : RA_VALID ;
            end
            RA_SUSPEND : begin
                ra_ns = (r_ost_cnt_dec) ? RA_VALID : RA_SUSPEND ;
            end
            default:ra_ns=RA_IDLE;
        endcase
    end

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            r_arbar      <=  2'b0;
            r_arsnoop    <=  4'b0;
            r_ardomain   <=  2'b0;
            r_arid       <= 14'b0;
            r_araddr     <=  {(p_BUS_AWIDTH){1'b0}};
            r_arlen      <=  8'b0; 

            r_arlock     <=  2'b0; 
            r_arcache    <=  4'b0; 
            r_arprot     <=  3'b0; 
            r_arqos      <=  4'b0; 
            r_arregion   <=  4'b0; 
            r_arvalid    <=  1'b0;

            r_araddr_pre <= {(p_BUS_AWIDTH){1'b0}};
            r_sburst_num <= 16'b0; 
        end
        else begin
            r_arbar      <=  r_arbar_in   ;
            r_arsnoop    <=  r_arsnoop_in ;
            r_ardomain   <=  r_ardomain_in;
            r_arid       <=  r_arid_in    ;
            r_araddr     <=  r_araddr_in  ;
            r_arlen      <=  r_arlen_in   ; 

            r_arlock     <=  r_arlock_in  ; 
            r_arcache    <=  r_arcache_in ; 
            r_arprot     <=  r_arprot_in  ; 
            r_arqos      <=  r_arqos_in   ; 
            r_arregion   <=  r_arregion_in; 
            r_arvalid    <=  r_arvalid_in ;

            r_araddr_pre <=  r_araddr_pre_in ;
            r_sburst_num <=  r_sburst_num_in ; 
        end
    end

    assign r_arid_in     =   {ch_r_qid,ch_rid} ;

    assign r_araddr_in   =  ((ra_cs==RA_PRE)&r_ost_fifo_vld)|

                            ((ra_cs==RA_VALID)&i_r_arready)   ? r_araddr_compare : r_araddr ;

    assign r_araddr_compare= ({r_araddr_pre[p_BUS_AWIDTH-1:ADDR_OFFSET_WIDTH],{(ADDR_OFFSET_WIDTH){1'b0}}} <= ch_raddr_max ) ? r_araddr_pre : r_araddr_pre - {{(p_BUS_AWIDTH-ADDR_OFFSET_WIDTH-9){1'b0}},r_burst_num,{(ADDR_OFFSET_WIDTH){1'b0}}};

    assign r_arlen_post  =  r_burst_num-9'b1 ;

    assign r_arlen_in    =  (ra_cs==RA_VALID)&(r_sburst_num==0)&i_r_arready ? 8'b0              :     
                            ((ra_cs==RA_PRE)&r_ost_fifo_vld)|
                            ((ra_cs==RA_VALID)&i_r_arready)                 ? r_arlen_post[7:0] : r_arlen ; 

    assign r_arsize      =  ADDR_OFFSET_WIDTH;
    assign r_arburst     =  2'b01;

    assign r_arsnoop_in  =  ch_arbus[31:28];

    assign r_arbar_in    =  ch_arbus[27:26]; 
    assign r_ardomain_in =  ch_arbus[25:24];

    assign r_arlock_in   =  ch_arbus[17:16]; 

    assign r_arcache_in  =  ch_arbus[15:12]; 
    assign r_arprot_in   =  ch_arbus[10: 8];

    assign r_arqos_in    =  ch_arbus[ 7: 4];
    assign r_arregion_in =  ch_arbus[ 3: 0];

    assign r_arvalid_in  =  ((ra_cs==RA_PRE)&r_ost_fifo_vld)|
                            ((ra_cs==RA_SUSPEND)&r_ost_cnt_dec)    ? 1'b1 :
                            ((ra_cs==RA_VALID)&i_r_arready&
                             ((r_sburst_num==0)|
                              ((r_ost_al_full)&(!r_ost_cnt_dec)))) ? 1'b0 : r_arvalid ;

    assign r_sburst_num_in = ((ra_cs==RA_IDLE)&fifo_dst_vld)                     ? ch_sburst_num   :
                             (((ra_cs==RA_PRE)&r_ost_fifo_vld)|
                              ((ra_cs==RA_VALID)&i_r_arready))&(r_sburst_num!=0) ? r_sburst_num_ns : r_sburst_num ;

    assign r_sburst_num_ns = r_sburst_num-{7'b0,r_burst_num} ;

    assign r_araddr_pre_in = ((ra_cs==RA_IDLE)&fifo_dst_vld)                   ? ch_raddr :
                             ((ra_cs==RA_PRE)&r_ost_fifo_vld)|
                             ((ra_cs==RA_VALID)&i_r_arready)&(r_sburst_num!=0) ? r_araddr_pre_ns : r_araddr_pre ;

    assign r_araddr_pre_ns = r_araddr_pre+{{(p_BUS_AWIDTH-ADDR_OFFSET_WIDTH-9){1'b0}},r_burst_num,{(ADDR_OFFSET_WIDTH){1'b0}}};

    assign r_burst_bond_dist           = ({1'b1,{(12-ADDR_OFFSET_WIDTH){1'b0}}}-{1'b0,r_araddr_pre [11:ADDR_OFFSET_WIDTH]}) ;
    assign r_burst_bond_dist_pre       = {{(ADDR_OFFSET_WIDTH){1'b0}},r_burst_bond_dist};

    assign r_burst_max                 = i_dma_rburst_max+9'b1;
    assign r_burst_num_small           = ({4'b0,r_burst_max}>r_burst_bond_dist_pre) ? r_burst_bond_dist_pre[8:0] : r_burst_max ; 

    assign r_burst_cover               = (r_sburst_num<={7'b0,r_burst_num_small}) ;
    assign r_burst_num                 = (r_burst_cover) ? r_sburst_num[8:0] : r_burst_num_small ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            r_ost_cnt <= {(p_R_BMEM_AWIDTH+1){1'b0}};
        end
        else begin
            r_ost_cnt <=  r_ost_cnt_in ;
        end
    end

    assign r_ost_cnt_in     = r_ost_cnt_inc&r_ost_cnt_dec ? r_ost_cnt      :
                              r_ost_cnt_inc               ? r_ost_cnt+1'b1 :
                              r_ost_cnt_dec               ? r_ost_cnt-1'b1 : r_ost_cnt ;

    assign r_ost_cnt_inc    = ((ra_cs==RA_VALID)&i_r_arready) ;

    assign r_ost_cnt_dec    = ((rd_cs==RD_READY)&i_r_rvalid&i_r_rlast) ;

    assign r_ost_full       = ({r_ost_cnt}>=(r_dma_ost_max_small_inc[p_R_BMEM_AWIDTH:0])) ;

    assign r_ost_al_full    = ({r_ost_cnt}>=r_dma_ost_max_small[p_R_BMEM_AWIDTH:0]);

    assign r_ost_fifo_vld   = ~r_ost_full ;

    assign r_ost_al_empty   = (r_ost_cnt<=1) ;

    assign r_dma_ost_max_small     = (i_dma_rost_max>{{(8-p_R_BMEM_AWIDTH){1'b0}},{(p_R_BMEM_AWIDTH){1'b1}}}) 
                                     ? {{(8-p_R_BMEM_AWIDTH){1'b0}},{(p_R_BMEM_AWIDTH){1'b1}}} : i_dma_rost_max ;

    assign r_dma_ost_max_small_inc = r_dma_ost_max_small+8'b1 ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            rd_cs <= RD_IDLE;
        end
        else begin
            rd_cs <=  rd_ns ;
        end
    end

    always@(*)
    begin
        rd_ns=rd_cs;
        case(rd_cs)
            RD_IDLE : begin
                rd_ns = (r_ost_cnt!=0) ? RD_READY : RD_IDLE ;
            end
            RD_READY : begin
                rd_ns = (r_ost_al_empty)&(!r_ost_cnt_inc)&(i_r_rvalid)&(i_r_rlast) ? RD_IDLE : RD_READY ;
            end
            default:rd_ns=RD_IDLE;
        endcase
    end

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            r_rready <= 1'b0 ; 
        end
        else begin
            r_rready <=  r_rready_in;
        end
    end

    assign r_rready_in = ((rd_cs==RD_IDLE)&(r_ost_cnt!=0))                        ? 1'b1 :
                         ((rd_cs==RD_READY)&
                          (r_ost_al_empty&(!r_ost_cnt_inc)&i_r_rvalid&i_r_rlast)) ? 1'b0 : r_rready ;

    always@(posedge i_clk or negedge i_rst_n)
    begin
        if(!i_rst_n) begin
            r_dbus_vld  <= 1'b0 ; 
            r_dbus_rid  <= 8'b0 ; 
            r_dbus_data <= {(p_BUS_DWIDTH){1'b0}} ; 
            r_dbus_last <= 1'b0 ;
            r_rresp     <= 2'b0 ; 
        end
        else begin
            r_dbus_vld  <=  r_dbus_vld_in ; 
            r_dbus_rid  <=  r_dbus_rid_in ; 
            r_dbus_data <=  r_dbus_data_in; 
            r_dbus_last <=  r_dbus_last_in; 
            r_rresp     <=  r_rresp_in    ;
        end
    end

    assign r_dbus_vld_in  = (rd_cs==RD_READY)&i_r_rvalid  ; 
    assign r_dbus_rid_in  = i_r_rid    ;
    assign r_dbus_data_in = i_r_rdata  ; 
    assign r_dbus_last_in = i_r_rlast  ; 
    assign r_rresp_in     = i_r_rresp  ; 

    assign o_r_arbar             = r_arbar    ; 
    assign o_r_arsnoop           = r_arsnoop  ; 
    assign o_r_ardomain          = r_ardomain ; 

    assign o_r_arid              = r_arid     ;  
    assign o_r_araddr            = {r_araddr[p_BUS_AWIDTH-1:ADDR_OFFSET_WIDTH],{(ADDR_OFFSET_WIDTH){1'b0}}};  
    assign o_r_arlen             = r_arlen    ;  
    assign o_r_arsize            = r_arsize   ;  
    assign o_r_arburst           = r_arburst  ;  
    assign o_r_arlock            = r_arlock   ;  
    assign o_r_arcache           = r_arcache  ;  
    assign o_r_arprot            = r_arprot   ;  
    assign o_r_arqos             = r_arqos    ;  
    assign o_r_arregion          = r_arregion ;  
    assign o_r_arvalid           = r_arvalid  ;  

    assign o_r_rready            = r_rready   ;  

    assign o_r_rato              = ra_to      ;
    assign o_r_rdto              = rd_to      ;
    assign o_r_bus_err           = (r_rresp[1])&(r_dbus_vld);

    assign o_r_dbus_vld          = r_dbus_vld  ; 
    assign o_r_dbus_rid          = r_dbus_rid  ; 
    assign o_r_dbus_data         = r_dbus_data ; 
    assign o_r_dbus_last         = r_dbus_last ; 

endmodule
