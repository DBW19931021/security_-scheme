//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_ctrl(
    input   wire            clk     ,
    input   wire            resetn  ,
    input   wire            i_sclk_sel,
    input   wire            i_rngen ,
    input   wire            i_alarm ,
    input   wire    [63: 0] i_roen  ,
    input   wire    [ 3: 0] i_rosen ,
    output   wire           o_sclk_sel,
    output  wire    [63: 0] o_roen  ,
    output  wire    [ 3: 0] o_rosen 
);

reg     [15: 0] r_ro4en   ;
reg     [15: 0] r_ro3en   ;
reg     [15: 0] r_ro2en   ;
reg     [15: 0] r_ro1en   ;

reg     [ 7: 0] dly_cnt   ;
reg     [15: 0] roen_ctrl ;

reg     [ 3: 0] r_rosen   ;
reg             r_sel     ;
wire            w_stop    ;
wire            cnt_flag  ;         

wire    [15: 0] w_ro4en =  i_roen[ 0+: 16];    
wire    [15: 0] w_ro3en =  i_roen[16+: 16];    
wire    [15: 0] w_ro2en =  i_roen[32+: 16];    
wire    [15: 0] w_ro1en =  i_roen[48+: 16];    

assign  cnt_flag    = ~w_stop & ~dly_cnt[7] ;

assign  w_stop      = i_alarm | !i_rngen;
assign  o_roen      = {r_ro1en, r_ro2en, r_ro3en, r_ro4en} ;
assign  o_rosen     = r_rosen   ;
assign  o_sclk_sel  = r_sel     ;

always @( * )
  case(dly_cnt[7:3])
    5'h00: roen_ctrl = 16'b0000000000000001;
    5'h01: roen_ctrl = 16'b0000000000000011;
    5'h02: roen_ctrl = 16'b0000000000000111;
    5'h03: roen_ctrl = 16'b0000000000001111;
    5'h04: roen_ctrl = 16'b0000000000011111;
    5'h05: roen_ctrl = 16'b0000000000111111;
    5'h06: roen_ctrl = 16'b0000000001111111;
    5'h07: roen_ctrl = 16'b0000000011111111;
    5'h08: roen_ctrl = 16'b0000000111111111;
    5'h09: roen_ctrl = 16'b0000001111111111;
    5'h0a: roen_ctrl = 16'b0000011111111111;
    5'h0b: roen_ctrl = 16'b0000111111111111;
    5'h0c: roen_ctrl = 16'b0001111111111111;
    5'h0d: roen_ctrl = 16'b0011111111111111;
    5'h0e: roen_ctrl = 16'b0111111111111111;
    5'h0f: roen_ctrl = 16'b1111111111111111;     
  default: roen_ctrl = 16'b1111111111111111;
  endcase         

always  @(posedge clk or negedge resetn)           
    if(!resetn)         dly_cnt <= 'h0        ;         
    else if(~i_rngen)   dly_cnt <= 'h0        ;
    else if(cnt_flag)   dly_cnt <= dly_cnt + 1'b1 ;
    else                dly_cnt <= dly_cnt    ;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_ro1en  <= 'h0        ;
    else if(w_stop)     r_ro1en  <= 'h0        ;
    else                r_ro1en  <= w_ro1en & roen_ctrl ; 

always  @(posedge clk or negedge resetn)        
    if(!resetn)         r_ro2en  <= 'h0        ; 
    else if(w_stop)     r_ro2en  <= 'h0        ; 
    else                r_ro2en  <= w_ro2en & roen_ctrl ; 

always  @(posedge clk or negedge resetn)        
    if(!resetn)         r_ro3en  <= 'h0        ; 
    else if(w_stop)     r_ro3en  <= 'h0        ; 
    else                r_ro3en  <= w_ro3en & roen_ctrl ; 

always  @(posedge clk or negedge resetn)        
    if(!resetn)         r_ro4en  <= 'h0        ; 
    else if(w_stop)     r_ro4en  <= 'h0        ; 
    else                r_ro4en  <= w_ro4en & roen_ctrl ; 

always  @(posedge clk or negedge resetn)      
    if(!resetn)         r_rosen <= 'h0         ;
    else if(w_stop)     r_rosen <= 'h0         ;
    else if(cnt_flag)   r_rosen <= 'h0         ;
    else                r_rosen <= i_rosen     ;

always  @(posedge clk or negedge resetn)
    if(!resetn)         r_sel   <= 'b0         ;
    else if(w_stop)     r_sel   <= 'b0         ;
    else                r_sel   <= i_sclk_sel  ;

endmodule
