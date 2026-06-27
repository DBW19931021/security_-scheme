//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_wdt(
   input  wire                 pclk,          
   input  wire                 presetn,       
   input  wire                 i_psel,          
   input  wire                 i_pwrite,        
   input  wire                 i_penable,       
   input  wire [9:0]           i_paddr,         
   input  wire [31:0]          i_pwdata,        
   output wire [31:0]          o_prdata,        

   output wire                 o_wdtintr,       
   output wire                 o_nresetout      
   );

   reg  [31:0]             wdtpwreg;
   reg  [5:0]              wdtcreg;
   reg  [15:0]             wdtldreg;
   reg  [15:0]             wdtpsreg;
   reg                     wdtisreg;
   reg  [15:0]             wdtrcreg;
   wire [15:0]             pscount_in;  
   reg  [15:0]             pscount;     
   reg                     psclken;     
   wire [6:0]              divcount_in;
   reg  [6:0]              divcount;
   wire                    divclken_t;
   wire                    divclken_in; 
   reg                     divclken;
   reg  [15:0]             countreg;
   wire [15:0]             countreg_in; 
   reg  [15:0]             resetcount;   
   wire [15:0]             resetcount_in;
   reg                     rstout;      
   wire [31:0]             prdata_t;
   wire [31:0]             prdata_in;
   reg [31:0]              prdata;

   wire                    wdtcregwren;
   wire                    wdtpsregwren;
   wire                    wdtldregwren;
   wire                    wdtisregwren;
   wire                    wdtrcregwren;
   wire                    wdtpwregwren;
   wire [5:0]              wdtcreg_in;
   wire [15:0]             wdtldreg_in;
   wire [15:0]             wdtpsreg_in;
   wire                    wdtisreg_in;
   wire [15:0]             wdtrcreg_in;
   wire [31:0]             wdtpwreg_in;

   wire [1:0]              div;

   wire                    clksel;
   wire                    inten;
   wire                    rsten;
   wire                    wdten;

   wire                    psclken_in;
   wire                    div16clken;
   wire                    div32clken;
   wire                    div64clken;
   wire                    div128clken;
   wire                    psdivclken;
   wire                    wdtclken;
   wire                    reload;
   wire                    resetcnten;
   wire                    rstout_in;

   wire   wdtreg_wren   = wdtpwreg == 32'h7b3e_8c5d;
   assign wdtcregwren   = ((i_paddr[7:2] == 6'b0000_00 ) & i_psel & i_pwrite & i_penable & wdtreg_wren);
   assign wdtpsregwren  = ((i_paddr[7:2] == 6'b0000_01 ) & i_psel & i_pwrite & i_penable & wdtreg_wren);
   assign wdtldregwren  = ((i_paddr[7:2] == 6'b0000_10 ) & i_psel & i_pwrite & i_penable & wdtreg_wren);
   assign wdtisregwren  = ((i_paddr[7:2] == 6'b0001_00 ) & i_psel & i_pwrite & i_penable & wdtreg_wren);
   assign wdtrcregwren  = ((i_paddr[7:2] == 6'b0001_01 ) & i_psel & i_pwrite & i_penable & wdtreg_wren);
   assign wdtpwregwren  = ((i_paddr[7:2] == 6'b0001_10 ) & i_psel & i_pwrite & i_penable );

   assign wdtcreg_in  = wdtcregwren  ? i_pwdata[5:0]  : wdtcreg;
   assign wdtldreg_in = wdtldregwren ? i_pwdata[15:0] : wdtldreg;
   assign wdtpsreg_in = wdtpsregwren ? i_pwdata[15:0] : wdtpsreg;
   assign wdtrcreg_in = wdtrcregwren ? i_pwdata[15:0] : wdtrcreg;
   assign wdtisreg_in = reload | (wdtisreg & ~(wdtisregwren & ~i_pwdata[0]));
   assign wdtpwreg_in = wdtpwregwren ? i_pwdata[31:0] : wdtpwreg;

   assign div    = wdtcreg[5:4];
   assign clksel = wdtcreg[3];
   assign inten  = wdtcreg[2];
   assign rsten  = wdtcreg[1];
   assign wdten  = wdtcreg[0];

   assign pscount_in = wdtpsregwren ? i_pwdata[15:0] :
                       wdten        ? ((pscount == 16'h0) ? wdtpsreg : pscount - 1) : pscount;

   assign psclken_in  = ((pscount == 16'b0000_0000_0000_0001) || 
                        (wdtpsreg == 16'b0000_0000_0000_0000) && wdten) ? 1'b1 : 1'b0;

   assign divcount_in = wdtcregwren ? 7'h0 :
                        wdten       ? (psclken ? divcount + 1'b1 : divcount) : divcount;

   assign div16clken  = (divcount[3:0] == 4'b1110    ) ? 1'b1 : 1'b0;
   assign div32clken  = (divcount[4:0] == 5'b1_1110  ) ? 1'b1 : 1'b0;
   assign div64clken  = (divcount[5:0] == 6'b11_1110 ) ? 1'b1 : 1'b0;
   assign div128clken = (divcount      == 7'b111_1110) ? 1'b1 : 1'b0;

   osr_cmux_4 #(1) div_mux(
   divclken_t,
   div16clken,    (div == 2'b00),
   div32clken,    (div == 2'b01),
   div64clken,    (div == 2'b10),
   div128clken
   );

   assign divclken_in = psclken ? divclken_t : divclken;

   assign psdivclken = psclken & divclken;

   assign wdtclken = clksel ? psclken : psdivclken;

   assign reload  = ((countreg == 16'h0000) && wdtclken) ? 1'b1 : 1'b0;

   assign countreg_in = wdtldregwren     ? i_pwdata[15:0] :
                        wdten & reload   ? wdtldreg     : 
                        wdten & wdtclken ? countreg - 1 : countreg;

   assign resetcnten    = (resetcount != 16'h0000) ? 1'b1 :1'b0;

   assign resetcount_in = reload ? wdtrcreg : resetcnten ? resetcount - 1 : resetcount;

   assign rstout_in     = rsten & (reload | ( rstout & resetcnten));

   assign o_wdtintr   = inten & wdtisreg;
   assign o_nresetout = ~rstout;

   wire   prdataen  = i_psel & ~i_pwrite  & ~i_penable;

   osr_cmux_8 #(32) prdata_mux(
   prdata_t,
   {26'h0, wdtcreg},    (i_paddr[7:2] == 6'b0000_00),
   {16'h0, wdtpsreg},   (i_paddr[7:2] == 6'b0000_01),
   {16'h0, wdtldreg},   (i_paddr[7:2] == 6'b0000_10),
   {16'h0, countreg},   (i_paddr[7:2] == 6'b0000_11),
   {31'h0, wdtisreg},   (i_paddr[7:2] == 6'b0001_00),
   {16'h0, wdtrcreg},   (i_paddr[7:2] == 6'b0001_01),
   32'h0,               (i_paddr[7:2] == 6'b1000_01),
   32'h0
   );
   assign prdata_in = prdataen ? prdata_t : prdata;
   assign o_prdata  = prdata;

   always @(posedge pclk or negedge presetn)
       if(!presetn) begin
           wdtcreg  <= 6'h0;
           wdtldreg <= 16'h0;
           wdtpsreg <= 16'h0;
           wdtisreg <= 1'b0;
           wdtrcreg <= 16'h0;
           wdtpwreg <= 32'h0;
       end
       else begin
           wdtcreg <= wdtcreg_in;
           wdtldreg <= wdtldreg_in;
           wdtpsreg <= wdtpsreg_in;
           wdtisreg <= wdtisreg_in;
           wdtrcreg <= wdtrcreg_in;
           wdtpwreg <= wdtpwreg_in;
       end

   always @(posedge pclk or negedge presetn)
       if(!presetn) begin
           pscount <= 16'h0;
           psclken <= 1'b0;
           divcount <= 7'h0;
           divclken <= 1'b0;
           countreg <= 16'h0;
           resetcount <= 16'h0;
           rstout <= 1'b0;
           prdata <= 32'h0;
       end
       else begin
           pscount <= pscount_in;
           psclken <= psclken_in;
           divcount <= divcount_in;
           divclken <= divclken_in;
           countreg <= countreg_in;
           resetcount <= resetcount_in;
           rstout <= rstout_in;
           prdata <= prdata_in;
       end
endmodule

module osr_cmux_4 # (
    parameter p_DWIDTH = 8
)(
output  wire [p_DWIDTH-1 : 0]     o_d,
input   wire [p_DWIDTH-1 : 0]     i_d0,
input   wire                      i_s0,
input   wire [p_DWIDTH-1 : 0]     i_d1,
input   wire                      i_s1,
input   wire [p_DWIDTH-1 : 0]     i_d2,
input   wire                      i_s2,
input   wire [p_DWIDTH-1 : 0]     i_d3
);
reg  [p_DWIDTH-1 : 0]   d_mux;
always @(*) begin
    d_mux = i_d3;
    case(1'b1) //synopsys parallel_case
        i_s0 : d_mux = i_d0;
        i_s1 : d_mux = i_d1;
        i_s2 : d_mux = i_d2;
    endcase
end

assign o_d = d_mux;

endmodule

module osr_cmux_8 # (
    parameter p_DWIDTH = 8
)(
output  wire [p_DWIDTH-1 : 0]     o_d,
input   wire [p_DWIDTH-1 : 0]     i_d0,
input   wire                      i_s0,
input   wire [p_DWIDTH-1 : 0]     i_d1,
input   wire                      i_s1,
input   wire [p_DWIDTH-1 : 0]     i_d2,
input   wire                      i_s2,
input   wire [p_DWIDTH-1 : 0]     i_d3,
input   wire                      i_s3,
input   wire [p_DWIDTH-1 : 0]     i_d4,
input   wire                      i_s4,
input   wire [p_DWIDTH-1 : 0]     i_d5,
input   wire                      i_s5,
input   wire [p_DWIDTH-1 : 0]     i_d6,
input   wire                      i_s6,
input   wire [p_DWIDTH-1 : 0]     i_d7
);
reg  [p_DWIDTH-1 : 0]   d_mux;
always @(*) begin
    d_mux = i_d7;
    case(1'b1) //synopsys parallel_case
        i_s0 : d_mux = i_d0;
        i_s1 : d_mux = i_d1;
        i_s2 : d_mux = i_d2;
        i_s3 : d_mux = i_d3;
        i_s4 : d_mux = i_d4;
        i_s5 : d_mux = i_d5;
        i_s6 : d_mux = i_d6;
    endcase
end

assign o_d = d_mux;

endmodule
