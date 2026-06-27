//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_sync
#(
  parameter p_CONFIG_FPGA = 0
)
(
    input   wire            clk     ,
    input   wire            resetn  ,
    input   wire            ro_rst  ,
    input   wire            i_sflag ,
    output  wire            o_sflag ,

    input   wire            ro_clk  ,
`ifdef OSR_TRNG_FPGA  
    input   wire            en_ro_clk ,
`endif            
    input   wire            i_flag  ,
    output  wire            o_flag  
);

reg      r_tflag ,r_tsflag   ;
wire     r_flag  ,r_sflag    ;

osr_sync #(.p_DWIDTH(1))
ro_flag_sync(
    .i_clk  (clk    ), 
    .i_rst_n(resetn ), 
    .i_in   (i_flag ), 
    .o_out  (r_flag ) 
);

always  @(posedge clk or negedge resetn)
    if(!resetn)    r_tflag <= 'b0     ;
    else           r_tflag <= r_flag  ;

`ifdef OSR_TRNG_FPGA 
generate
  if (!p_CONFIG_FPGA) begin : sync

    osr_sync #(.p_DWIDTH(1))
    sys_flag_sync(
        .i_clk  (ro_clk ), 
        .i_rst_n(ro_rst ), 
        .i_in   (i_sflag), 
        .o_out  (r_sflag) 
    );

    always  @(posedge ro_clk or negedge ro_rst)
        if(!ro_rst)    r_tsflag <= 'b0      ;
        else r_tsflag <= r_sflag  ;

      end else begin : fpga_sync
        reg      r_sync0;
        reg      r_sync1;
        always @ (posedge ro_clk or negedge ro_rst)
            if(!ro_rst)
                begin
                    r_sync0 <= 1'b0;
                    r_sync1 <= 1'b0;
                end
            else if (en_ro_clk)
                begin
                    r_sync0 <=  i_sflag;
                    r_sync1 <=  r_sync0;
                end
        assign r_sflag = r_sync1;

    always  @(posedge ro_clk or negedge ro_rst)
        if(!ro_rst)    r_tsflag <= 'b0      ;
        else if (en_ro_clk)  r_tsflag <= r_sflag  ;

      end
    endgenerate

`else
    osr_sync #(.p_DWIDTH(1))
    sys_flag_sync(
        .i_clk  (ro_clk ), 
        .i_rst_n(ro_rst ), 
        .i_in   (i_sflag), 
        .o_out  (r_sflag) 
    );

    always  @(posedge ro_clk or negedge ro_rst)
        if(!ro_rst)    r_tsflag <= 'b0      ;
        else           r_tsflag <= r_sflag  ;
`endif
assign  o_sflag = r_tflag ^ r_flag  ;
assign  o_flag = r_tsflag ^ r_sflag ;

endmodule
