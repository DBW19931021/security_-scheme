//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_process #(
`ifdef OSR_TRNG_FPGA 
    parameter       p_CONFIG_FPGA = 0,
`endif        
    parameter       p_RO_DELAY = 16
    )(
    input   wire            clk     ,
`ifdef OSR_TRNG_FPGA 
    input   wire            i_en_clk ,
`endif            
    input   wire            resetn  ,
    input   wire    [15: 0] i_rbit  ,
    input   wire            i_flag  ,
    output  wire            o_flag  ,
    output  wire    [31: 0] o_rand  ,
    output  wire            o_rornd 
);

    reg     [31: 0] r_rand  ;
    reg     [ 5: 0] r_cnt   ;
    reg             r_flag  ;
    wire            w_rbit  ;
    wire            w_flag  ;
    wire            w_flop  ;
    reg     [4:0]   r_ro_delay_cnt;
    wire            w_ro_delay_flag;

assign  w_rbit = ^i_rbit    ;
assign  w_flag = r_cnt[5]   ;
assign  w_flop = r_cnt==6'd31;
assign  w_ro_delay_flag = r_ro_delay_cnt >= p_RO_DELAY;
assign  o_flag = r_flag     ;
assign  o_rand = r_rand     ;
assign  o_rornd = r_rand[0] ;

`ifdef OSR_TRNG_FPGA
    generate
      if (p_CONFIG_FPGA) begin : fpga_output
          always  @(posedge clk or negedge resetn)
              if(!resetn)         r_flag <= 'b0       ;
          `ifdef OSR_TRNG_FPGA 
              else if(~i_en_clk)   r_flag <= r_flag   ;
          `endif    
              else if(w_flop)     r_flag <= ~r_flag   ;
              else                r_flag <= r_flag    ;

          always  @(posedge clk or negedge resetn)
              if(!resetn)                 r_cnt <= 'd0            ;
          `ifdef OSR_TRNG_FPGA 
              else if(~i_en_clk)   r_cnt <= r_cnt   ;
          `endif    
              else if(i_flag)             r_cnt <= 'd0            ;
              else if(w_flag)             r_cnt <= r_cnt          ;
              else if(w_ro_delay_flag)    r_cnt <= r_cnt + 'd1    ;
              else                        r_cnt <= r_cnt          ;

          always @(posedge clk or negedge resetn)
          begin
              if(!resetn)                 r_ro_delay_cnt  <= 'd0;
          `ifdef OSR_TRNG_FPGA 
              else if(~i_en_clk)   r_ro_delay_cnt <= r_ro_delay_cnt ;
          `endif    
              else if(w_ro_delay_flag)    r_ro_delay_cnt  <= r_ro_delay_cnt;
              else                        r_ro_delay_cnt  <= r_ro_delay_cnt + 'd1;
          end

          always  @(posedge clk or negedge resetn)
              if(!resetn)         r_rand <= 'h0           ;
          `ifdef OSR_TRNG_FPGA 
              else if(~i_en_clk)   r_rand <= r_rand ;
          `endif    
              else if(w_flag)     r_rand <= r_rand        ;
              else                r_rand <= {r_rand[30: 0],w_rbit};

      end else begin : normal_output     
          always  @(posedge clk or negedge resetn)
              if(!resetn)         r_flag <= 'b0       ;
              else if(w_flop)     r_flag <= ~r_flag   ;
              else                r_flag <= r_flag    ;

          always  @(posedge clk or negedge resetn)
              if(!resetn)                 r_cnt <= 'd0            ;   
              else if(i_flag)             r_cnt <= 'd0            ;
              else if(w_flag)             r_cnt <= r_cnt          ;
              else if(w_ro_delay_flag)    r_cnt <= r_cnt + 'd1    ;
              else                        r_cnt <= r_cnt          ;

          always @(posedge clk or negedge resetn)
          begin
              if(!resetn)                 r_ro_delay_cnt  <= 'd0;
              else if(w_ro_delay_flag)    r_ro_delay_cnt  <= r_ro_delay_cnt;
              else                        r_ro_delay_cnt  <= r_ro_delay_cnt + 'd1;
          end

          always  @(posedge clk or negedge resetn)
              if(!resetn)         r_rand <= 'h0           ;
              else if(w_flag)     r_rand <= r_rand        ;
              else                r_rand <= {r_rand[30: 0],w_rbit};
      end          
    endgenerate

`else

    always  @(posedge clk or negedge resetn)
        if(!resetn)         r_flag <= 'b0       ;  
        else if(w_flop)     r_flag <= ~r_flag   ;
        else                r_flag <= r_flag    ;

    always  @(posedge clk or negedge resetn)
        if(!resetn)                 r_cnt <= 'd0            ; 
        else if(i_flag)             r_cnt <= 'd0            ;
        else if(w_flag)             r_cnt <= r_cnt          ;
        else if(w_ro_delay_flag)    r_cnt <= r_cnt + 'd1    ;
        else                        r_cnt <= r_cnt          ;

    always @(posedge clk or negedge resetn)
    begin
        if(!resetn)                 r_ro_delay_cnt  <= 'd0;  
        else if(w_ro_delay_flag)    r_ro_delay_cnt  <= r_ro_delay_cnt;
        else                        r_ro_delay_cnt  <= r_ro_delay_cnt + 'd1;
    end

    always  @(posedge clk or negedge resetn)
        if(!resetn)         r_rand <= 'h0           ;  
        else if(w_flag)     r_rand <= r_rand        ;
        else                r_rand <= {r_rand[30: 0],w_rbit};

`endif
endmodule
