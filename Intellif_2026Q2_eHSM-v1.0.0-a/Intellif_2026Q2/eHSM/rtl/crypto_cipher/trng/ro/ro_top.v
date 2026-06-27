//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_top 
#(
    parameter                   p_CONFIG_FPGA  = 0
)
(
    input   wire            clk     ,
`ifdef OSR_TRNG_FPGA      
    input   wire    [ 1: 0] osc_en_clk,
`endif            
    input   wire            resetn  ,
    input   wire            i_scan  ,
    input   wire            i_sclk_sel,
    input   wire            i_rngen ,
    input   wire            i_alarm ,
    input   wire    [ 1: 0] i_conf  ,
    input   wire    [63: 0] i_roen  ,
    input   wire    [ 3: 0] i_rosen ,
    input   wire            i_full  ,
    output  wire            o_we    ,
    output  wire    [31: 0] o_wdata ,

    output  wire    [3:0]   o_rornd ,
    output  wire    [3:0]   o_roclk 
);

wire    [31: 0] w_rand1,w_rand2,w_rand3,w_rand4;
wire    [63: 0] w_roen    ;
wire    [ 3: 0] w_rosen   ;
wire    [ 2: 0] w_conf    ;
wire            w_iflag1,w_iflag2,w_iflag3,w_iflag4;
wire            w_oflag1,w_oflag2,w_oflag3,w_oflag4;
wire            w_ro_rst1,w_ro_rst2,w_ro_rst3,w_ro_rst4;
wire            w_isflag1,w_isflag2,w_isflag3,w_isflag4;
wire            w_osflag1,w_osflag2,w_osflag3,w_osflag4;
wire            w_rornd1,w_rornd2,w_rornd3,w_rornd4;
wire            w_roclk1,w_roclk2,w_roclk3,w_roclk4;
wire            w_sclk_sel;

`ifdef OSR_TRNG_FPGA  
wire            w_en_roclk1,w_en_roclk2,w_en_roclk3,w_en_roclk4;

generate
  if (p_CONFIG_FPGA) begin : fpga_sync
  reg             w_roclk1_reg,w_roclk2_reg,w_roclk3_reg,w_roclk4_reg;

  always @(posedge w_roclk1 or negedge resetn)
    if(!resetn)              w_roclk1_reg <= 1'b0;
    else if (w_en_roclk1)    w_roclk1_reg <= 1'b1;
    else                     w_roclk1_reg <= 1'b0;
  always @(posedge w_roclk2 or negedge resetn)
    if(!resetn)              w_roclk2_reg <= 1'b0;
    else if (w_en_roclk2)    w_roclk2_reg <= 1'b1;
    else                     w_roclk2_reg <= 1'b0;
  always @(posedge w_roclk3 or negedge resetn)
    if(!resetn)              w_roclk3_reg <= 1'b0;
    else if (w_en_roclk3)    w_roclk3_reg <= 1'b1;
    else                     w_roclk3_reg <= 1'b0;
  always @(posedge w_roclk4 or negedge resetn)
    if(!resetn)              w_roclk4_reg <= 1'b0;
    else if (w_en_roclk4)    w_roclk4_reg <= 1'b1;
    else                     w_roclk4_reg <= 1'b0;
    assign  o_roclk = {w_roclk4_reg, w_roclk3_reg, w_roclk2_reg, w_roclk1_reg};
  end else begin : no_sync
    assign  o_roclk = {w_roclk4, w_roclk3, w_roclk2, w_roclk1};

  end
endgenerate
`else
assign  o_roclk = {w_roclk4, w_roclk3, w_roclk2, w_roclk1};
`endif
assign  o_rornd = {w_rornd4, w_rornd3, w_rornd2, w_rornd1};
assign  w_conf  = {w_sclk_sel,i_conf};

ro_ctrl    ro_ctrl(
.clk        (clk            ),
.resetn     (resetn         ),
.i_sclk_sel (i_sclk_sel     ),
.i_rngen    (i_rngen        ),
.i_alarm    (i_alarm        ),
.i_roen     (i_roen         ),
.i_rosen    (i_rosen        ),
.o_sclk_sel (w_sclk_sel     ),
.o_roen     (w_roen         ),
.o_rosen    (w_rosen        )
);

`ifdef OSR_TRNG_FPGA  
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(0),.CLK_INV_N(97),.delay(5010),.p_CONFIG_FPGA(p_CONFIG_FPGA))   
`else
ro_source #(.p_IDX(0),.CLK_INV_N(97),.delay(160),.p_CONFIG_FPGA(p_CONFIG_FPGA))  
`endif
`else
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(0),.CLK_INV_N(97),.delay(5010))  
`else
ro_source #(.p_IDX(0),.CLK_INV_N(97),.delay(160))  
`endif
`endif        
ros1(
.clk        (clk            ),
`ifdef OSR_TRNG_FPGA  
.osc_en_clk (osc_en_clk     ),
`endif        
.resetn     (resetn         ),
.ro_rst     (w_ro_rst1      ),
.i_scan     (i_scan         ),
.i_rosen    (w_rosen[3]     ),
.i_conf     (w_conf         ),
.i_roen     (w_roen[63:48]  ),
.o_rand     (w_rand1        ),
.i_flag     (w_iflag1       ),
.o_flag     (w_oflag1       ),
`ifdef OSR_TRNG_FPGA  
.o_en_roclk (w_en_roclk1    ),
`endif        
.o_rornd    (w_rornd1       ),
.o_roclk    (w_roclk1       )
);

`ifdef OSR_TRNG_FPGA  
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(1),.CLK_INV_N(95),.delay(4770),.p_CONFIG_FPGA(p_CONFIG_FPGA))    
`else
ro_source #(.p_IDX(1),.CLK_INV_N(95),.delay(170),.p_CONFIG_FPGA(p_CONFIG_FPGA))  
`endif
`else        
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(1),.CLK_INV_N(95),.delay(4770))  
`else
ro_source #(.p_IDX(1),.CLK_INV_N(95),.delay(170))  
`endif
`endif

ros2(
.clk        (clk            ),
`ifdef OSR_TRNG_FPGA 
.osc_en_clk (osc_en_clk     ),
`endif        
.resetn     (resetn         ),
.ro_rst     (w_ro_rst2      ),
.i_scan     (i_scan         ),
.i_rosen    (w_rosen[2]     ),
.i_conf     (w_conf         ),
.i_roen     (w_roen[47:32]  ),
.o_rand     (w_rand2        ),
.i_flag     (w_iflag2       ),
.o_flag     (w_oflag2       ),
`ifdef OSR_TRNG_FPGA  
.o_en_roclk (w_en_roclk2    ),
`endif       
.o_rornd    (w_rornd2       ),
.o_roclk    (w_roclk2       )
);

`ifdef OSR_TRNG_FPGA  
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(2),.CLK_INV_N(93),.delay(4510),.p_CONFIG_FPGA(p_CONFIG_FPGA))    
`else
ro_source #(.p_IDX(2),.CLK_INV_N(93),.delay(180),.p_CONFIG_FPGA(p_CONFIG_FPGA))  
`endif
`else 
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(2),.CLK_INV_N(93),.delay(4510))  
`else
ro_source #(.p_IDX(2),.CLK_INV_N(93),.delay(180))  
`endif
`endif
ros3(
.clk        (clk            ),
`ifdef OSR_TRNG_FPGA  
.osc_en_clk (osc_en_clk     ),
`endif       
.resetn     (resetn         ),
.ro_rst     (w_ro_rst3      ),
.i_scan     (i_scan         ),
.i_rosen    (w_rosen[1]     ),
.i_conf     (w_conf         ),
.i_roen     (w_roen[31:16]  ),
.o_rand     (w_rand3        ),
.i_flag     (w_iflag3       ),
.o_flag     (w_oflag3       ),
`ifdef OSR_TRNG_FPGA  
.o_en_roclk (w_en_roclk3    ),
`endif       
.o_rornd    (w_rornd3       ),
.o_roclk    (w_roclk3       )
);

`ifdef OSR_TRNG_FPGA  
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(3),.CLK_INV_N(91),.delay(4170),.p_CONFIG_FPGA(p_CONFIG_FPGA))    
`else
ro_source #(.p_IDX(3),.CLK_INV_N(91),.delay(190),.p_CONFIG_FPGA(p_CONFIG_FPGA))  
`endif
`else 
`ifdef OSR_TRNG_SIM
ro_source #(.p_IDX(3),.CLK_INV_N(91),.delay(4170))  
`else
ro_source #(.p_IDX(3),.CLK_INV_N(91),.delay(190))  
`endif
`endif
ros4(
.clk        (clk            ),
`ifdef OSR_TRNG_FPGA  
.osc_en_clk (osc_en_clk     ),
`endif        
.resetn     (resetn         ),
.ro_rst     (w_ro_rst4      ),
.i_scan     (i_scan         ),
.i_rosen    (w_rosen[0]     ),
.i_conf     (w_conf         ),
.i_roen     (w_roen[15: 0]  ),
.o_rand     (w_rand4        ),
.i_flag     (w_iflag4       ),
.o_flag     (w_oflag4       ),
`ifdef OSR_TRNG_FPGA  
.o_en_roclk (w_en_roclk4    ),
`endif        
.o_rornd    (w_rornd4       ),
.o_roclk    (w_roclk4       )
);

ro_sync    
#(.p_CONFIG_FPGA(p_CONFIG_FPGA))
ro_sync1(
.clk        (clk            ),
.resetn     (resetn         ),
.ro_rst     (w_ro_rst1      ),
.i_sflag    (w_osflag1      ),
.o_sflag    (w_isflag1      ),
.ro_clk     (w_roclk1       ),
`ifdef OSR_TRNG_FPGA  
.en_ro_clk  (w_en_roclk1    ),
`endif     
.i_flag     (w_oflag1       ),
.o_flag     (w_iflag1       )
);

ro_sync 
#(.p_CONFIG_FPGA(p_CONFIG_FPGA))
ro_sync2(
.clk        (clk            ),
.resetn     (resetn         ),
.ro_rst     (w_ro_rst2      ),
.i_sflag    (w_osflag2      ),
.o_sflag    (w_isflag2      ),
.ro_clk     (w_roclk2       ),
`ifdef OSR_TRNG_FPGA  
.en_ro_clk  (w_en_roclk2    ),
`endif     
.i_flag     (w_oflag2       ),
.o_flag     (w_iflag2       )
);

ro_sync
#(.p_CONFIG_FPGA(p_CONFIG_FPGA))
ro_sync3(
.clk        (clk            ),
.resetn     (resetn         ),
.ro_rst     (w_ro_rst3      ),
.i_sflag    (w_osflag3      ),
.o_sflag    (w_isflag3      ),
.ro_clk     (w_roclk3       ),
`ifdef OSR_TRNG_FPGA  
.en_ro_clk  (w_en_roclk3    ),
`endif     
.i_flag     (w_oflag3       ),
.o_flag     (w_iflag3       )
);

ro_sync
#(.p_CONFIG_FPGA(p_CONFIG_FPGA))
ro_sync4(
.clk        (clk            ),
.resetn     (resetn         ),
.ro_rst     (w_ro_rst4      ),
.i_sflag    (w_osflag4      ),
.o_sflag    (w_isflag4      ),
.ro_clk     (w_roclk4       ),
`ifdef OSR_TRNG_FPGA 
.en_ro_clk  (w_en_roclk4    ),
`endif     
.i_flag     (w_oflag4       ),
.o_flag     (w_iflag4       )
);

ro_comb    ro_comb(
.clk        (clk            ),
.resetn     (resetn         ),
.i_rngen    (i_rngen        ),
.i_alarm    (i_alarm        ),
.i_rosen    (i_rosen        ),
.i_sflag1   (w_isflag1      ),
.o_sflag1   (w_osflag1      ),
.i_rand1    (w_rand1        ),
.i_sflag2   (w_isflag2      ),
.o_sflag2   (w_osflag2      ),
.i_rand2    (w_rand2        ),
.i_sflag3   (w_isflag3      ),
.o_sflag3   (w_osflag3      ),
.i_rand3    (w_rand3        ),
.i_sflag4   (w_isflag4      ),
.o_sflag4   (w_osflag4      ),
.i_rand4    (w_rand4        ),
.i_full     (i_full         ),
.o_we       (o_we           ),
.o_wdata    (o_wdata        ) 
);

endmodule
