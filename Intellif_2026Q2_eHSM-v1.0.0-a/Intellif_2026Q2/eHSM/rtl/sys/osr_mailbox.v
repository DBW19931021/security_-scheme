//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_mailbox #(
    parameter           P_S2H_NOTE_N  = 1          ,   
    parameter           P_H2S_NOTE_N  = 1          ,   
    parameter           P_HAVE_FUSA   = 1              
)(

    input  wire         i_hclk          ,
    input  wire         i_hreset_n      ,
    input  wire         i_hsm_clk_en    ,

    input  wire         i_se_hsel       ,
    input  wire [11:0]  i_se_haddr      ,
    input  wire [1:0]   i_se_htrans     ,
    input  wire         i_se_hwrite     ,
    input  wire [2:0]   i_se_hsize      ,
    input  wire [31:0]  i_se_hwdata     ,
    input  wire         i_se_hready     ,
    input  wire [2:0]   i_se_hburst     ,    
    input  wire [3:0]   i_se_hprot      ,
    input  wire         i_se_hmastlock  ,

    output wire [31:0]  o_se_hrdata     ,
    output wire [1:0]   o_se_hresp      ,
    output wire         o_se_hreadyout  ,

    input  wire [63:0]  i_hsm_status    ,

    output wire         o_se_irq        ,
    input  wire [31:0]  i_se_irq_all    ,

    input  wire [63:0]   i_hsm_cfg       ,       

    input  wire         i_soc_hsel      ,
    input  wire [11:0]  i_soc_haddr     ,
    input  wire [1:0]   i_soc_htrans    ,
    input  wire         i_soc_hwrite    ,
    input  wire [2:0]   i_soc_hsize     ,
    input  wire [31:0]  i_soc_hwdata    ,
    input  wire         i_soc_hready    ,
    input  wire [2:0]   i_soc_hburst    ,    
    input  wire [3:0]   i_soc_hprot     ,
    input  wire         i_soc_hmastlock ,

    output wire [31:0]  o_soc_hrdata    ,
    output wire [1:0]   o_soc_hresp     ,
    output wire         o_soc_hreadyout ,

    output wire         o_soc_irq       , 
    input  wire [31:0]  i_soc_irq_all     
);

localparam  ADDR_S2H_INFO_0     = 12'h000;    
localparam  ADDR_S2H_INFO_1     = 12'h004;    

localparam  ADDR_H2S_INFO_0     = 12'h080;    
localparam  ADDR_H2S_INFO_1     = 12'h084;    

localparam  ADDR_S2H_NOTE       = 12'h100;    
localparam  ADDR_H2S_NOTE       = 12'h104;    

localparam  ADDR_S2H_S_INT      = 12'h110;    
localparam  ADDR_S2H_S_INTEN    = 12'h114;    
localparam  ADDR_S2H_H_INT      = 12'h118;    
localparam  ADDR_S2H_H_INTEN    = 12'h11C;    

localparam  ADDR_H2S_S_INT      = 12'h120;    
localparam  ADDR_H2S_S_INTEN    = 12'h124;    
localparam  ADDR_H2S_H_INT      = 12'h128;    
localparam  ADDR_H2S_H_INTEN    = 12'h12C;    

localparam  ADDR_HSM_STA_0      = 12'h400;    
localparam  ADDR_HSM_STA_1      = 12'h404;    

localparam  ADDR_INT_ALL        = 12'h480;    

localparam  ADDR_HSM_CFG0       = 12'h4F8;    
localparam  ADDR_HSM_CFG1       = 12'h4FC;    

localparam     SMCNT_P_WIDTH = 29   ;

localparam     PT_P_REG_GROUP_NUM     = 6 ; 
localparam     PT_P_REG_DATA_WIDTH    = 32;
localparam     PT_LP_REG_PARITY_WIDTH = (PT_P_REG_DATA_WIDTH > (PT_P_REG_DATA_WIDTH/8)*8) ? ((PT_P_REG_DATA_WIDTH/8) + 1) : (PT_P_REG_DATA_WIDTH/8) ; 

wire            reg_s_req;
wire            reg_s_rd;
wire    [11:0]  reg_s_raddr;
wire            reg_s_wr;
wire    [31:0]  reg_s_wdata;

wire            reg_h_req;
wire            reg_h_rd;
wire    [11:0]  reg_h_raddr;
wire            reg_h_wr;
wire    [31:0]  reg_h_wdata;

wire    [63:0]  hsm_cfg = i_hsm_cfg ;

reg     [31:0]  h2s_info_0 ;
reg     [31:0]  h2s_info_1 ;
reg     [31:0]  s2h_info_0 ;
reg     [31:0]  s2h_info_1 ;

reg                    hsm_clk_en_dly  ;
reg                    reg_s_wr_dly    ;
reg [11:0]             reg_s_waddr     ;
reg [31:0]             reg_s_rdata     ;
reg                    reg_h_wr_dly    ;
reg [11:0]             reg_h_waddr     ;
reg [31:0]             reg_h_rdata     ;
reg [P_S2H_NOTE_N-1:0] s2h_note        ;
reg [P_H2S_NOTE_N-1:0] h2s_note        ;
reg [P_S2H_NOTE_N-1:0] s2h_s_int       ;
reg [P_S2H_NOTE_N-1:0] s2h_s_inten     ;
reg [P_S2H_NOTE_N-1:0] s2h_h_int       ;
reg [P_S2H_NOTE_N-1:0] s2h_h_inten     ;
reg                    s2h_s_col_int   ;
reg                    s2h_s_col_inten ;
reg                    s2h_h_col_int   ;
reg                    s2h_h_col_inten ;
reg [P_H2S_NOTE_N-1:0] h2s_s_int       ;
reg [P_H2S_NOTE_N-1:0] h2s_s_inten     ;
reg [P_H2S_NOTE_N-1:0] h2s_h_int       ;
reg [P_H2S_NOTE_N-1:0] h2s_h_inten     ;
reg                    h2s_s_col_int   ;
reg                    h2s_s_col_inten ;
reg                    h2s_h_col_int   ;
reg                    h2s_h_col_inten ;
reg [31:0]             hsm_status_0    ;
reg [31:0]             hsm_status_1    ;






wire        wr_s2h_h_inten = reg_h_wr && (reg_h_waddr == ADDR_S2H_H_INTEN);

wire        wr_h2s_h_inten = reg_h_wr && (reg_h_waddr == ADDR_H2S_H_INTEN);

wire            hsm_clk_en_dly_next = i_hsm_clk_en; 
wire            reg_s_wr_dly_next = i_soc_hready ? (reg_s_req & i_soc_hwrite) : reg_s_wr_dly;
wire    [11:0]  reg_s_waddr_next = i_soc_hready ? reg_s_raddr : reg_s_waddr;
reg     [31:0]  reg_s_rdata_next ;

wire            reg_h_wr_dly_next = (i_se_hready && i_hsm_clk_en) ? (reg_h_req & i_se_hwrite) : reg_h_wr_dly;
wire    [11:0]  reg_h_waddr_next = (i_se_hready && i_hsm_clk_en) ? reg_h_raddr : reg_h_waddr;
reg     [31:0]  reg_h_rdata_next ;

wire    [31:0]  s2h_info_0_nxt = (reg_s_wr && (reg_s_waddr == ADDR_S2H_INFO_0)) ? reg_s_wdata : s2h_info_0;
wire    [31:0]  s2h_info_1_nxt = (reg_s_wr && (reg_s_waddr == ADDR_S2H_INFO_1)) ? reg_s_wdata : s2h_info_1;

wire    [31:0]  h2s_info_0_nxt  = (reg_h_wr && (reg_h_waddr == ADDR_H2S_INFO_0)) ? reg_h_wdata : h2s_info_0;
wire    [31:0]  h2s_info_1_nxt  = (reg_h_wr && (reg_h_waddr == ADDR_H2S_INFO_1)) ? reg_h_wdata : h2s_info_1;

wire    [P_S2H_NOTE_N-1:0]    s2h_note_next = ((reg_s_wr && (reg_s_waddr == ADDR_S2H_NOTE)) ? reg_s_wdata[P_S2H_NOTE_N-1:0] : {P_S2H_NOTE_N{1'b0}}) | 
                                            (((reg_h_wr && (reg_h_waddr == ADDR_S2H_NOTE)) ? (~reg_h_wdata[P_S2H_NOTE_N-1:0]) : {P_S2H_NOTE_N{1'b1}}) & s2h_note);

wire    [P_H2S_NOTE_N-1:0]    h2s_note_next = ((reg_h_wr && (reg_h_waddr == ADDR_H2S_NOTE)) ? reg_h_wdata[P_H2S_NOTE_N-1:0] : {P_H2S_NOTE_N{1'b0}}) | 
                                            (((reg_s_wr && (reg_s_waddr == ADDR_H2S_NOTE)) ? (~reg_s_wdata[P_H2S_NOTE_N-1:0]) : {P_H2S_NOTE_N{1'b1}}) & h2s_note);

wire    [P_S2H_NOTE_N-1:0]    s2h_s_int_next   = (s2h_note & (~s2h_note_next)) | 
                               (((reg_s_wr && (reg_s_waddr == ADDR_S2H_S_INT  )) ? (~reg_s_wdata[P_S2H_NOTE_N-1:0]) : {P_S2H_NOTE_N{1'b1}}) & s2h_s_int);
wire    [P_S2H_NOTE_N-1:0]    s2h_s_inten_next = (reg_s_wr && (reg_s_waddr == ADDR_S2H_S_INTEN)) ? reg_s_wdata[P_S2H_NOTE_N-1:0] : s2h_s_inten ;
wire    [P_S2H_NOTE_N-1:0]    s2h_h_int_next   = ((~s2h_note) & s2h_note_next) | 
                               (((reg_h_wr && (reg_h_waddr == ADDR_S2H_H_INT  )) ? (~reg_h_wdata[P_S2H_NOTE_N-1:0]) : {P_S2H_NOTE_N{1'b1}}) & s2h_h_int);

wire    [P_S2H_NOTE_N-1:0]    s2h_h_inten_nxt = wr_s2h_h_inten ? reg_h_wdata[P_S2H_NOTE_N-1:0] : s2h_h_inten ;

wire    [P_S2H_NOTE_N-1:0]    s2h_h_inten_next = s2h_h_inten_nxt ; 

wire            s2h_s_col_int_next = (|(((reg_s_wr && (reg_s_waddr == ADDR_S2H_NOTE)) ? reg_s_wdata[P_S2H_NOTE_N-1:0] : {P_S2H_NOTE_N{1'b0}}) & 
                                        ((reg_h_wr && (reg_h_waddr == ADDR_S2H_NOTE)) ? reg_h_wdata[P_S2H_NOTE_N-1:0] : {P_S2H_NOTE_N{1'b0}}))) | 
                                     (((reg_s_wr && (reg_s_waddr == ADDR_S2H_S_INT  )) ? (~reg_s_wdata[31]) : 1'b1) & s2h_s_col_int);
wire            s2h_s_col_inten_next = (reg_s_wr && (reg_s_waddr == ADDR_S2H_S_INTEN)) ? reg_s_wdata[31] : s2h_s_col_inten;
wire            s2h_h_col_int_next = (|(((reg_s_wr && (reg_s_waddr == ADDR_S2H_NOTE)) ? reg_s_wdata[P_S2H_NOTE_N-1:0] : {P_S2H_NOTE_N{1'b0}}) & 
                                        ((reg_h_wr && (reg_h_waddr == ADDR_S2H_NOTE)) ? reg_h_wdata[P_S2H_NOTE_N-1:0] : {P_S2H_NOTE_N{1'b0}}))) | 
                                     (((reg_h_wr && (reg_h_waddr == ADDR_S2H_H_INT  )) ? (~reg_h_wdata[31]) : 1'b1) & s2h_h_col_int);
wire            s2h_h_col_inten_nxt = wr_s2h_h_inten  ? reg_h_wdata[31] : s2h_h_col_inten;
wire            s2h_h_col_inten_next = s2h_h_col_inten_nxt ; 

wire    [P_H2S_NOTE_N-1:0]    h2s_s_int_next   = ((~h2s_note) & h2s_note_next) | 
                               (((reg_s_wr && (reg_s_waddr == ADDR_H2S_S_INT  )) ? (~reg_s_wdata[P_H2S_NOTE_N-1:0]) : {P_H2S_NOTE_N{1'b1}}) & h2s_s_int);
wire    [P_H2S_NOTE_N-1:0]    h2s_s_inten_next = (reg_s_wr && (reg_s_waddr == ADDR_H2S_S_INTEN)) ? reg_s_wdata[P_H2S_NOTE_N-1:0] : h2s_s_inten ;
wire    [P_H2S_NOTE_N-1:0]    h2s_h_int_next   = (h2s_note & (~h2s_note_next)) | 
                               (((reg_h_wr && (reg_h_waddr == ADDR_H2S_H_INT  )) ? (~reg_h_wdata[P_H2S_NOTE_N-1:0]) : {P_H2S_NOTE_N{1'b1}}) & h2s_h_int);

wire    [P_H2S_NOTE_N-1:0]    h2s_h_inten_nxt = wr_h2s_h_inten ? reg_h_wdata[P_H2S_NOTE_N-1:0] : h2s_h_inten ;

wire    [P_H2S_NOTE_N-1:0]    h2s_h_inten_next = h2s_h_inten_nxt ;

wire            h2s_s_col_int_next = (|(((reg_s_wr && (reg_s_waddr == ADDR_H2S_NOTE)) ? reg_s_wdata[P_H2S_NOTE_N-1:0] : {P_H2S_NOTE_N{1'b0}}) & 
                                        ((reg_h_wr && (reg_h_waddr == ADDR_H2S_NOTE)) ? reg_h_wdata[P_H2S_NOTE_N-1:0] : {P_H2S_NOTE_N{1'b0}}))) | 
                                     (((reg_s_wr && (reg_s_waddr == ADDR_H2S_S_INT  )) ? (~reg_s_wdata[31]) : 1'b1) & h2s_s_col_int);
wire            h2s_s_col_inten_next = (reg_s_wr && (reg_s_waddr == ADDR_H2S_S_INTEN)) ? reg_s_wdata[31] : h2s_s_col_inten;
wire            h2s_h_col_int_next = (|(((reg_s_wr && (reg_s_waddr == ADDR_H2S_NOTE)) ? reg_s_wdata[P_H2S_NOTE_N-1:0] : {P_H2S_NOTE_N{1'b0}}) & 
                                        ((reg_h_wr && (reg_h_waddr == ADDR_H2S_NOTE)) ? reg_h_wdata[P_H2S_NOTE_N-1:0] : {P_H2S_NOTE_N{1'b0}}))) | 
                                     (((reg_h_wr && (reg_h_waddr == ADDR_H2S_H_INT  )) ? (~reg_h_wdata[31]) : 1'b1) & h2s_h_col_int);
wire            h2s_h_col_inten_nxt = wr_h2s_h_inten ? reg_h_wdata[31] : h2s_h_col_inten;
wire            h2s_h_col_inten_next = h2s_h_col_inten_nxt ; 

wire    [31:0]  hsm_status_0_next = i_hsm_status[31: 0]; 
wire    [31:0]  hsm_status_1_next = i_hsm_status[63:32]; 

assign reg_s_req   = i_soc_hsel & i_soc_htrans[1] & (i_soc_hsize==3'h2) & i_soc_hready;
assign reg_s_rd    = reg_s_req & (~i_soc_hwrite);
assign reg_s_raddr = i_soc_haddr[11:0];
assign reg_s_wr    = i_soc_hready & reg_s_wr_dly;
assign reg_s_wdata = i_soc_hwdata;

assign reg_h_req   = i_se_hsel & i_se_htrans[1] & (i_se_hsize==3'h2) & i_se_hready;
assign reg_h_rd    = reg_h_req & (~i_se_hwrite);
assign reg_h_raddr = i_se_haddr[11:0];
assign reg_h_wr    = i_se_hready & reg_h_wr_dly & hsm_clk_en_dly;
assign reg_h_wdata = i_se_hwdata;

assign o_se_hrdata     = reg_h_rdata; 
assign o_se_hresp      = 2'h0 ;
assign o_se_hreadyout  = 1'h1 ;
assign o_se_irq        = (|(s2h_h_int & s2h_h_inten)) | (|(s2h_h_col_int & s2h_h_col_inten)) | 
                         (|(h2s_h_int & h2s_h_inten)) | (|(h2s_h_col_int & h2s_h_col_inten)) ;
assign o_soc_hrdata    = reg_s_rdata; 
assign o_soc_hresp     = 2'h0 ;
assign o_soc_hreadyout = 1'h1 ;

assign o_soc_irq       = (|(s2h_s_int & s2h_s_inten)) | (|(s2h_s_col_int & s2h_s_col_inten)) |
                         (|(h2s_s_int & h2s_s_inten)) | (|(h2s_s_col_int & h2s_s_col_inten)) ;

always @(posedge i_hclk or negedge i_hreset_n) begin
    if(!i_hreset_n) begin
        hsm_clk_en_dly  <= 1'b0                 ; 
        reg_s_wr_dly    <= 1'b0                 ; 
        reg_s_waddr     <= 12'h0                ; 
        reg_s_rdata     <= 32'h0                ; 
        reg_h_wr_dly    <= 1'b0                 ; 
        reg_h_waddr     <= 12'h0                ; 
        reg_h_rdata     <= 32'h0                ; 
    end else begin
        hsm_clk_en_dly  <= hsm_clk_en_dly_next  ; 
        reg_s_wr_dly    <= reg_s_wr_dly_next    ; 
        reg_s_waddr     <= reg_s_waddr_next     ; 
        reg_s_rdata     <= reg_s_rdata_next     ; 
        reg_h_wr_dly    <= reg_h_wr_dly_next    ; 
        reg_h_waddr     <= reg_h_waddr_next     ; 
        reg_h_rdata     <= reg_h_rdata_next     ; 
    end
end

always @(posedge i_hclk or negedge i_hreset_n) begin
    if(!i_hreset_n) begin
        s2h_note        <= {P_S2H_NOTE_N{1'b0}} ; 
        h2s_note        <= {P_H2S_NOTE_N{1'b0}} ; 
        s2h_s_int       <= {P_S2H_NOTE_N{1'b0}} ; 
        s2h_s_inten     <= {P_S2H_NOTE_N{1'b0}} ; 
        s2h_h_int       <= {P_S2H_NOTE_N{1'b0}} ; 
    end else begin
        s2h_note        <= s2h_note_next        ; 
        h2s_note        <= h2s_note_next        ; 
        s2h_s_int       <= s2h_s_int_next       ; 
        s2h_s_inten     <= s2h_s_inten_next     ; 
        s2h_h_int       <= s2h_h_int_next       ; 
    end
end

always @(posedge i_hclk or negedge i_hreset_n) begin
    if(!i_hreset_n) begin
        s2h_h_inten     <= {P_S2H_NOTE_N{1'b0}} ; 
        s2h_s_col_int   <= 1'b0                 ; 
        s2h_s_col_inten <= 1'b0                 ; 
        s2h_h_col_int   <= 1'b0                 ; 
        s2h_h_col_inten <= 1'b0                 ; 
        h2s_s_int       <= {P_H2S_NOTE_N{1'b0}} ; 
        h2s_s_inten     <= {P_H2S_NOTE_N{1'b0}} ; 
        h2s_h_int       <= {P_H2S_NOTE_N{1'b0}} ; 
    end else begin
        s2h_h_inten     <= s2h_h_inten_next     ; 
        s2h_s_col_int   <= s2h_s_col_int_next   ; 
        s2h_s_col_inten <= s2h_s_col_inten_next ; 
        s2h_h_col_int   <= s2h_h_col_int_next   ; 
        s2h_h_col_inten <= s2h_h_col_inten_next ; 
        h2s_s_int       <= h2s_s_int_next       ; 
        h2s_s_inten     <= h2s_s_inten_next     ; 
        h2s_h_int       <= h2s_h_int_next       ; 
    end
end

always @(posedge i_hclk or negedge i_hreset_n) begin
    if(!i_hreset_n) begin
        h2s_h_inten     <= {P_H2S_NOTE_N{1'b0}} ; 
        h2s_s_col_int   <= 1'b0                 ; 
        h2s_s_col_inten <= 1'b0                 ; 
        h2s_h_col_int   <= 1'b0                 ; 
        h2s_h_col_inten <= 1'b0                 ; 
        hsm_status_0    <= 32'b0                ; 
        hsm_status_1    <= 32'b0                ; 
    end else begin
        h2s_h_inten     <= h2s_h_inten_next     ; 
        h2s_s_col_int   <= h2s_s_col_int_next   ; 
        h2s_s_col_inten <= h2s_s_col_inten_next ; 
        h2s_h_col_int   <= h2s_h_col_int_next   ; 
        h2s_h_col_inten <= h2s_h_col_inten_next ; 
        hsm_status_0    <= hsm_status_0_next    ; 
        hsm_status_1    <= hsm_status_1_next    ; 
    end
end

always @(posedge i_hclk or negedge i_hreset_n) begin
    if(!i_hreset_n) begin
        s2h_info_0      <= 32'h0               ; 
        s2h_info_1      <= 32'h0               ; 
        h2s_info_0      <= 32'h0               ; 
        h2s_info_1      <= 32'h0               ; 
    end else begin
        s2h_info_0      <= s2h_info_0_nxt      ; 
        s2h_info_1      <= s2h_info_1_nxt      ; 
        h2s_info_0      <= h2s_info_0_nxt      ; 
        h2s_info_1      <= h2s_info_1_nxt      ; 
    end
end

always @ (*)
begin
    reg_s_rdata_next = reg_s_rdata;
    if (i_soc_hready)
    begin
        reg_s_rdata_next = 32'h0;

        if (reg_s_rd && i_soc_hready)
        begin
            case (reg_s_raddr)
                ADDR_S2H_INFO_0 : reg_s_rdata_next  = s2h_info_0[31:0];
                ADDR_S2H_INFO_1 : reg_s_rdata_next  = s2h_info_1[31:0];

                ADDR_H2S_INFO_0  : reg_s_rdata_next = h2s_info_0[31:0];
                ADDR_H2S_INFO_1  : reg_s_rdata_next = h2s_info_1[31:0];

                ADDR_S2H_NOTE    : begin                                          reg_s_rdata_next[P_S2H_NOTE_N-1:0] = s2h_note    ; end
                ADDR_H2S_NOTE    : begin                                          reg_s_rdata_next[P_H2S_NOTE_N-1:0] = h2s_note    ; end
                ADDR_S2H_S_INT   : begin reg_s_rdata_next[31] = s2h_s_col_int   ; reg_s_rdata_next[P_S2H_NOTE_N-1:0] = s2h_s_int   ; end
                ADDR_S2H_S_INTEN : begin reg_s_rdata_next[31] = s2h_s_col_inten ; reg_s_rdata_next[P_S2H_NOTE_N-1:0] = s2h_s_inten ; end
                ADDR_S2H_H_INT   : begin reg_s_rdata_next[31] = s2h_h_col_int   ; reg_s_rdata_next[P_S2H_NOTE_N-1:0] = s2h_h_int   ; end
                ADDR_S2H_H_INTEN : begin reg_s_rdata_next[31] = s2h_h_col_inten ; reg_s_rdata_next[P_S2H_NOTE_N-1:0] = s2h_h_inten ; end
                ADDR_H2S_S_INT   : begin reg_s_rdata_next[31] = h2s_s_col_int   ; reg_s_rdata_next[P_H2S_NOTE_N-1:0] = h2s_s_int   ; end
                ADDR_H2S_S_INTEN : begin reg_s_rdata_next[31] = h2s_s_col_inten ; reg_s_rdata_next[P_H2S_NOTE_N-1:0] = h2s_s_inten ; end
                ADDR_H2S_H_INT   : begin reg_s_rdata_next[31] = h2s_h_col_int   ; reg_s_rdata_next[P_H2S_NOTE_N-1:0] = h2s_h_int   ; end
                ADDR_H2S_H_INTEN : begin reg_s_rdata_next[31] = h2s_h_col_inten ; reg_s_rdata_next[P_H2S_NOTE_N-1:0] = h2s_h_inten ; end
                ADDR_HSM_STA_0   : reg_s_rdata_next = hsm_status_0;
                ADDR_HSM_STA_1   : reg_s_rdata_next = hsm_status_1;

                ADDR_INT_ALL     : reg_s_rdata_next = i_soc_irq_all;

                default          : reg_s_rdata_next = 32'h0;
            endcase
        end
    end
end

always @ (*)
begin
    reg_h_rdata_next = reg_h_rdata;
    if (i_se_hready && i_hsm_clk_en)
    begin
        reg_h_rdata_next = 32'h0;

        if (reg_h_rd && i_se_hready)
        begin
            case (reg_h_raddr)
                ADDR_S2H_INFO_0 : reg_h_rdata_next  = s2h_info_0[31:0];
                ADDR_S2H_INFO_1 : reg_h_rdata_next  = s2h_info_1[31:0];

                ADDR_H2S_INFO_0  : reg_h_rdata_next = h2s_info_0[31:0];
                ADDR_H2S_INFO_1  : reg_h_rdata_next = h2s_info_1[31:0];

                ADDR_S2H_NOTE    : begin                                          reg_h_rdata_next[P_S2H_NOTE_N-1:0] = s2h_note    ; end
                ADDR_H2S_NOTE    : begin                                          reg_h_rdata_next[P_H2S_NOTE_N-1:0] = h2s_note    ; end
                ADDR_S2H_S_INT   : begin reg_h_rdata_next[31] = s2h_s_col_int   ; reg_h_rdata_next[P_S2H_NOTE_N-1:0] = s2h_s_int   ; end
                ADDR_S2H_S_INTEN : begin reg_h_rdata_next[31] = s2h_s_col_inten ; reg_h_rdata_next[P_S2H_NOTE_N-1:0] = s2h_s_inten ; end
                ADDR_S2H_H_INT   : begin reg_h_rdata_next[31] = s2h_h_col_int   ; reg_h_rdata_next[P_S2H_NOTE_N-1:0] = s2h_h_int   ; end
                ADDR_S2H_H_INTEN : begin reg_h_rdata_next[31] = s2h_h_col_inten ; reg_h_rdata_next[P_S2H_NOTE_N-1:0] = s2h_h_inten ; end
                ADDR_H2S_S_INT   : begin reg_h_rdata_next[31] = h2s_s_col_int   ; reg_h_rdata_next[P_H2S_NOTE_N-1:0] = h2s_s_int   ; end
                ADDR_H2S_S_INTEN : begin reg_h_rdata_next[31] = h2s_s_col_inten ; reg_h_rdata_next[P_H2S_NOTE_N-1:0] = h2s_s_inten ; end
                ADDR_H2S_H_INT   : begin reg_h_rdata_next[31] = h2s_h_col_int   ; reg_h_rdata_next[P_H2S_NOTE_N-1:0] = h2s_h_int   ; end
                ADDR_H2S_H_INTEN : begin reg_h_rdata_next[31] = h2s_h_col_inten ; reg_h_rdata_next[P_H2S_NOTE_N-1:0] = h2s_h_inten ; end
                ADDR_HSM_STA_0   : reg_h_rdata_next = hsm_status_0;
                ADDR_HSM_STA_1   : reg_h_rdata_next = hsm_status_1;

                ADDR_INT_ALL     : reg_h_rdata_next = i_se_irq_all   ;

                ADDR_HSM_CFG0    : reg_h_rdata_next = hsm_cfg[31: 0] ; 
                ADDR_HSM_CFG1    : reg_h_rdata_next = hsm_cfg[63:32] ; 

                default          : reg_h_rdata_next = 32'h0;
            endcase
        end
    end
end

endmodule 
