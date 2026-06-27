//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_timer (
input  wire           pclk           ,
input  wire           presetn        ,
input  wire           i_psel         ,
input  wire           i_penable      ,
input  wire           i_pwrite       ,
input  wire [7:0]     i_paddr        ,
input  wire [31:0]    i_pwdata       ,
output wire [31:0]    o_prdata       ,

output wire           o_int_tim0     ,
output wire           o_int_tim1      
  );

  reg  [3:0]  TCTL0   ; 
  reg  [3:0]  TCTL1   ; 

  reg  [7:0]  TPRS0   ; 
  reg  [7:0]  TPRS1   ; 

  reg  [31:0] TMOD0   ; 
  reg  [31:0] TMOD1   ; 

  wire [31:0] TCNT0   ; 
  wire [31:0] TCNT1   ; 

  reg  [1:0]  TSTA0   ; 
  reg  [1:0]  TSTA1   ; 

  wire    Triger_int0  ;
  wire    Triger_int1  ;

  wire cr_wen;

  assign cr_wen = i_psel & i_penable & i_pwrite;
  wire [3:0]  TCTL0_in = (cr_wen & i_paddr==8'h00) ? i_pwdata[3:0]  : TCTL0;
  wire [3:0]  TCTL1_in = (cr_wen & i_paddr==8'h04) ? i_pwdata[3:0]  : TCTL1;
  wire [7:0]  TPRS0_in = (cr_wen & i_paddr==8'h10) ? i_pwdata[7:0]  : TPRS0;
  wire [7:0]  TPRS1_in = (cr_wen & i_paddr==8'h14) ? i_pwdata[7:0]  : TPRS1;
  wire [31:0] TMOD0_in = (cr_wen & i_paddr==8'h20) ? i_pwdata[31:0] : TMOD0;
  wire [31:0] TMOD1_in = (cr_wen & i_paddr==8'h24) ? i_pwdata[31:0] : TMOD1;

  wire [1:0]  TSTA0_in;
  wire [1:0]  TSTA1_in;
  assign      TSTA0_in[0] = (Triger_int0 | TSTA0[0]) & ~(cr_wen & (i_paddr==8'h40) & ~i_pwdata[0]);
  assign      TSTA0_in[1] = (cr_wen & (i_paddr==8'h40)) ? i_pwdata[1] : TSTA0[1];
  assign      TSTA1_in[0] = (Triger_int1 | TSTA1[0]) & ~(cr_wen & (i_paddr==8'h44) & ~i_pwdata[0]);
  assign      TSTA1_in[1] = (cr_wen & (i_paddr==8'h44)) ? i_pwdata[1] : TSTA1[1];

  wire [31:0] iPRDATA_TIM;
  reg [31:0] PRDATA_TIM;

   osr_cmux_13 #(32) div_mux(
   iPRDATA_TIM,
   {28'h0,TCTL0},    (i_paddr == 8'h00),
   {28'h0,TCTL1},    (i_paddr == 8'h04),
   {24'h0,TPRS0},    (i_paddr == 8'h10),
   {24'h0,TPRS1},    (i_paddr == 8'h14),
   TMOD0,            (i_paddr == 8'h20),
   TMOD1,            (i_paddr == 8'h24),
   TCNT0,            (i_paddr == 8'h30),
   TCNT1,            (i_paddr == 8'h34),
   {30'h0,TSTA0},    (i_paddr == 8'h40),
   {30'h0,TSTA1},    (i_paddr == 8'h44),
   32'h0,            1'b0,
   32'h0,            1'b0,
   32'h0
   );

  wire [31:0] PRDATA_TIM_in = i_psel ? iPRDATA_TIM : PRDATA_TIM;

  always @(negedge presetn or posedge pclk)
    if(!presetn) PRDATA_TIM <= 32'h0;
    else         PRDATA_TIM <= PRDATA_TIM_in;

  assign      o_prdata      = PRDATA_TIM;

  wire    CNT_CLK_EN0 ;
  wire    CNT_CLK_EN1 ;

  osr_timer_prs u_timer_prs0(
    .pclk        (pclk         ),
    .presetn     (presetn      ),
    .i_tprs      (TPRS0        ),
    .o_cnt_clk_en(CNT_CLK_EN0  ));

  osr_timer_prs u_timer_prs1(
    .pclk        (pclk         ),
    .presetn     (presetn      ),
    .i_tprs      (TPRS1        ),
    .o_cnt_clk_en(CNT_CLK_EN1  ));

  reg   TIM0_start;
  reg   TIM1_start;

  wire  TIM0_start_in = (cr_wen & (i_pwdata[0]==1'b1) & (i_paddr==8'h00));
  wire  TIM1_start_in = (cr_wen & (i_pwdata[0]==1'b1) & (i_paddr==8'h04));

  osr_timer_cnt u_timer_cnt0(
    .presetn        (presetn        ),
    .pclk           (pclk           ),
    .i_cnt_clk_en   (CNT_CLK_EN0    ),
    .i_tctl         (TCTL0[2:0]     ),
    .i_tmod         (TMOD0          ),
    .i_tim_start    (TIM0_start     ),
    .o_tcnt         (TCNT0          ),
    .o_triger_int   (Triger_int0    ));

  osr_timer_cnt u_timer_cnt1(
    .presetn        (presetn        ),
    .pclk           (pclk           ),
    .i_cnt_clk_en   (CNT_CLK_EN1    ),
    .i_tctl         (TCTL1[2:0]     ),
    .i_tmod         (TMOD1          ),
    .i_tim_start    (TIM1_start     ),
    .o_tcnt         (TCNT1          ),
    .o_triger_int   (Triger_int1    ));

  assign o_int_tim0 = TSTA0[1] ? TSTA0[0] : Triger_int0;
  assign o_int_tim1 = TSTA1[1] ? TSTA1[0] : Triger_int1;

  always @(posedge pclk or negedge presetn )
      if(!presetn) begin
          TCTL0 <= 4'h0;
          TCTL1 <= 4'h0;
          TPRS0 <= 8'h0;
          TPRS1 <= 8'h0;
          TMOD0 <= 32'hFFFFFFFF;
          TMOD1 <= 32'hFFFFFFFF;
          TSTA0 <= 2'h0;
          TSTA1 <= 2'h0;
      end
      else begin
          TCTL0 <= TCTL0_in;
          TCTL1 <= TCTL1_in;
          TPRS0 <= TPRS0_in;
          TPRS1 <= TPRS1_in;
          TMOD0 <= TMOD0_in;
          TMOD1 <= TMOD1_in;
          TSTA0 <= TSTA0_in;
          TSTA1 <= TSTA1_in;
      end

  always @(posedge pclk or negedge presetn )
      if(!presetn) begin
          TIM0_start <= 1'b0;
          TIM1_start <= 1'b0;
      end
      else begin
          TIM0_start <= TIM0_start_in;
          TIM1_start <= TIM1_start_in;
      end
endmodule

module osr_cmux_13 # (
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
input   wire [p_DWIDTH-1 : 0]     i_d7,
input   wire                      i_s7,
input   wire [p_DWIDTH-1 : 0]     i_d8,
input   wire                      i_s8,
input   wire [p_DWIDTH-1 : 0]     i_d9,
input   wire                      i_s9,
input   wire [p_DWIDTH-1 : 0]     i_d10,
input   wire                      i_s10,
input   wire [p_DWIDTH-1 : 0]     i_d11,
input   wire                      i_s11,
input   wire [p_DWIDTH-1 : 0]     i_d12 
);
reg  [p_DWIDTH-1 : 0]   d_mux;
always @(*) begin
    d_mux = i_d12;
    case(1'b1) //synopsys parallel_case
        i_s0 : d_mux = i_d0;
        i_s1 : d_mux = i_d1;
        i_s2 : d_mux = i_d2;
        i_s3 : d_mux = i_d3;
        i_s4 : d_mux = i_d4;
        i_s5 : d_mux = i_d5;
        i_s6 : d_mux = i_d6;
        i_s7 : d_mux = i_d7;
        i_s8 : d_mux = i_d8;
        i_s9 : d_mux = i_d9;
        i_s10: d_mux = i_d10;
        i_s11: d_mux = i_d11;
    endcase
end

assign o_d = d_mux;

endmodule
