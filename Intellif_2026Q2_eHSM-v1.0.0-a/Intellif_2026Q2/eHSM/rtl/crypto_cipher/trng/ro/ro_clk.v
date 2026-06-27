//========================================================================================
// Copyright (C) 2025 Open Security Research Inc. - All Rights Reserved
//                                 !!!   PlainText   !!!
// Define : OSR_SIM_CELL CBC_SM4_EN 
// hotfix 67145cc10bbe83ff79471d679f76a018a4bdbd10
// 900 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module    ro_clk
#(
    parameter p_IDX = 0,     
`ifdef OSR_TRNG_FPGA
    parameter p_CONFIG_FPGA = 0,
`endif
    parameter INV_N = 97 ,
    parameter delay = 5000
)(
    input   wire            clk         ,
`ifdef OSR_TRNG_FPGA   
    input   wire    [ 1: 0] osc_en_clk  ,
`endif            
    input   wire            resetn      ,
    input   wire            i_enable    ,
    input   wire            scan        ,
    input   wire    [ 2: 0] i_config    ,
`ifdef OSR_TRNG_FPGA
    output  wire            o_en_clk    ,   
    output  wire            o_en_clk_div,    
`endif    
    output  wire            o_clk       ,   
    output  wire            o_clk_div   
);

    wire    dft_clk_div01  ;
    wire    dft_clk_div02  ;
    wire    dft_clk_div04  ;
    wire    dft_clk_div08  ;
    wire    dft_clk_div16  ;

    reg     clk_div02 ;
    reg     clk_div04 ;
    reg     clk_div08 ;
    reg     clk_div16 ;
    reg     clk_div32 ;

    wire    clk_div_sel01;
    wire    clk_div_sel02;

    wire    dft_clk_div01_in ;
    wire    dft_clk_div02_in ;
    wire    dft_clk_div04_in ;
    wire    dft_clk_div08_in ;  
    wire    dft_clk_div16_in ;

    wire  clk_sel   ;
    wire  ro_clk_out;
    wire  clk_out;
    wire  o_clk_div_in ;
`ifdef OSR_TRNG_FPGA
    wire  en_ro_clk_out;
    wire  en_dft_clk_div01;

`endif        

`ifdef OSR_TRNG_FPGA
    generate
      if (!p_CONFIG_FPGA) begin : oscillator

          ro_clk_circuit #(.p_IDX(p_IDX),.INV_N(INV_N),.delay(delay)) ro (.enable(i_enable),.clk(clk),.resetn(resetn),.scan(scan),.clk_out(clk_out));
        `ifdef OSR_TRNG_RAND_SIM
            assign  clk_sel    = clk_out;
        `else
            osr_mux2 clk_mux_01(.A(clk_out), .B(clk), .S(i_config[2]), .Z(clk_sel));
        `endif
          osr_mux2 clk_mux_div01(.A(clk_sel), .B(clk), .S(scan), .Z(dft_clk_div01_in));
          assign en_dft_clk_div01 = 1'b0 ;

           assign dft_clk_div01 = dft_clk_div01_in ;

        always @(posedge dft_clk_div01 or negedge resetn)
          if(!resetn)   clk_div02 <= 1'b0;
          else          clk_div02 <= ~clk_div02;

          osr_mux2 clk_mux_03(.A(clk_div02), .B(clk), .S(scan), .Z(dft_clk_div02_in));

           assign dft_clk_div02 = dft_clk_div02_in ;

        always @(posedge dft_clk_div02 or negedge resetn)
          if(!resetn)   clk_div04 <= 1'b0;
          else          clk_div04 <= ~clk_div04;

          osr_mux2 clk_mux_04(.A(clk_div04), .B(clk), .S(scan), .Z(dft_clk_div04_in));

           assign dft_clk_div04 = dft_clk_div04_in ;

        always @(posedge dft_clk_div04 or negedge resetn)
          if(!resetn)   clk_div08 <= 1'b0;
          else          clk_div08 <= ~clk_div08;

          osr_mux2 clk_mux_05(.A(clk_div08), .B(clk), .S(scan), .Z(dft_clk_div08_in));

           assign dft_clk_div08 = dft_clk_div08_in ;

        always @(posedge dft_clk_div08 or negedge resetn)
          if(!resetn)   clk_div16 <= 1'b0;
          else          clk_div16 <= ~clk_div16;

          osr_mux2 clk_mux_06(.A(clk_div16), .B(clk), .S(scan), .Z(dft_clk_div16_in));

           assign dft_clk_div16 = dft_clk_div16_in ;

        always @(posedge dft_clk_div16 or negedge resetn)
          if(!resetn)   clk_div32 <= 1'b0;
          else          clk_div32 <= ~clk_div32;  

          osr_mux2 clk_div_mux_01(.A(clk_div16),     .B(clk_div32),     .S(i_config[0]), .Z(clk_div_sel01));
          osr_mux2 clk_div_mux_02(.A(clk_div04),     .B(clk_div08),     .S(i_config[0]), .Z(clk_div_sel02));
          osr_mux2 clk_div_mux_03(.A(clk_div_sel02), .B(clk_div_sel01), .S(i_config[1]), .Z(ro_clk_out));

          osr_mux2 clk_mux_07(.A(ro_clk_out), .B(clk), .S(scan), .Z(o_clk_div_in));

           assign o_clk_div = o_clk_div_in ;

          assign  en_ro_clk_out = 1'b0;

          assign  o_clk         = dft_clk_div01 ;  
          assign  o_en_clk      = 1'b0;

          assign  o_en_clk_div  = 1'b0;

        end else begin : fpga_oscillator
          reg   [4:0]  clk_div;

          assign clk_out  = 1'b0; 

          assign clk_sel = 1'b0; 

          wire sel_clk = scan | i_config[2];
          osr_mux2 clk_mux_08(.A(osc_en_clk[0]), .B(clk), .S(sel_clk), .Z(dft_clk_div01));
          assign en_dft_clk_div01 = (scan | i_config[2]) ? 1'b1 : osc_en_clk[1] ;

          always @(posedge dft_clk_div01 or negedge resetn)
            if(!resetn)   clk_div <= 5'b0000;
            else if (en_dft_clk_div01) begin
              clk_div <= clk_div+1;
            end
          always @( * ) begin : clk_division
            clk_div02 = scan | (&clk_div[0:0] & en_dft_clk_div01);
            clk_div04 = scan | (&clk_div[1:0] & en_dft_clk_div01);
            clk_div08 = scan | (&clk_div[2:0] & en_dft_clk_div01);
            clk_div16 = scan | (&clk_div[3:0] & en_dft_clk_div01);
            clk_div32 = scan | (&clk_div[4:0] & en_dft_clk_div01);
          end

          assign  ro_clk_out    = dft_clk_div01;
          assign  en_ro_clk_out = i_config[1] ? (i_config[0] ? clk_div32 : clk_div16):(i_config[0] ? clk_div08 : clk_div04);

          assign  o_clk         = dft_clk_div01 ; 
          assign  o_en_clk      = en_dft_clk_div01;
          assign  o_clk_div     = ro_clk_out ;
          osr_mux2 clk_mux_09(.A(en_ro_clk_out), .B(1'b1), .S(scan), .Z(o_en_clk_div));

        end
    endgenerate

`else

    ro_clk_circuit #(.p_IDX(p_IDX),.INV_N(INV_N),.delay(delay)) ro (.enable(i_enable),.clk(clk),.resetn(resetn),.scan(scan),.clk_out(clk_out));
    `ifdef OSR_TRNG_RAND_SIM
        assign  clk_sel    = clk_out;
    `else
        osr_mux2 clk_mux_10(.A(clk_out), .B(clk), .S(i_config[2]), .Z(clk_sel));

    `endif
        osr_mux2 clk_mux_div01(.A(clk_sel), .B(clk), .S(scan), .Z(dft_clk_div01_in));

     assign dft_clk_div01 = dft_clk_div01_in ;

  always @(posedge dft_clk_div01 or negedge resetn)
    if(!resetn)   clk_div02 <= 1'b0;
    else          clk_div02 <= ~clk_div02;

        osr_mux2 clk_mux_12(.A(clk_div02), .B(clk), .S(scan), .Z(dft_clk_div02_in));

     assign dft_clk_div02 = dft_clk_div02_in ;

  always @(posedge dft_clk_div02 or negedge resetn)
    if(!resetn)   clk_div04 <= 1'b0;
    else          clk_div04 <= ~clk_div04;

        osr_mux2 clk_mux_13(.A(clk_div04), .B(clk), .S(scan), .Z(dft_clk_div04_in));

     assign dft_clk_div04 = dft_clk_div04_in ;

  always @(posedge dft_clk_div04 or negedge resetn)
    if(!resetn)   clk_div08 <= 1'b0;
    else          clk_div08 <= ~clk_div08;

        osr_mux2 clk_mux_14(.A(clk_div08), .B(clk), .S(scan), .Z(dft_clk_div08_in));

     assign dft_clk_div08 = dft_clk_div08_in ;

  always @(posedge dft_clk_div08 or negedge resetn)
    if(!resetn)   clk_div16 <= 1'b0;
    else          clk_div16 <= ~clk_div16;

        osr_mux2 clk_mux_15(.A(clk_div16), .B(clk), .S(scan), .Z(dft_clk_div16_in));

     assign dft_clk_div16 = dft_clk_div16_in ;

  always @(posedge dft_clk_div16 or negedge resetn)
    if(!resetn)   clk_div32 <= 1'b0;
    else          clk_div32 <= ~clk_div32;  

     osr_mux2 clk_div_mux_01(.A(clk_div16),     .B(clk_div32),     .S(i_config[0]), .Z(clk_div_sel01));
     osr_mux2 clk_div_mux_02(.A(clk_div04),     .B(clk_div08),     .S(i_config[0]), .Z(clk_div_sel02));
     osr_mux2 clk_div_mux_03(.A(clk_div_sel02), .B(clk_div_sel01), .S(i_config[1]), .Z(ro_clk_out));  

     osr_mux2 clk_mux_16(.A(ro_clk_out), .B(clk), .S(scan), .Z(o_clk_div_in));

     assign o_clk_div = o_clk_div_in ;

  assign  o_clk = dft_clk_div01 ;
`endif
endmodule
