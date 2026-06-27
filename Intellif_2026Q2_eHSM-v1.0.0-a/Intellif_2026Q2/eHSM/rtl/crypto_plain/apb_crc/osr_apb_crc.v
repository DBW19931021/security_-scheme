//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_apb_crc(   
  input  wire               pclk              ,
  input  wire               presetn           ,
  input  wire               i_psel            ,
  input  wire               i_penable         ,
  input  wire               i_pwrite          ,
  input  wire [17:0]        i_paddr           ,
  input  wire [31:0]        i_pwdata          ,
  output wire [31:0]        o_prdata           
);

  reg    [6:0]                       crc_con                            ;
  reg    [7:0]                       crc_dat                            ;
  reg                                crc_wr                             ;
  reg    [31:0]                      lfsr_dat                           ;
  reg                                lfsr_wr                            ;

  reg    [31:0]                      READ_DATA                          ;
  reg    [31:0]                      NXT_READ_DATA                      ;

  wire                               write_enable                       ;
  wire                               read_enable                        ;
  wire   [31:0]                      crc_res                            ;
  wire   [31:0]                      crc_res32                          ;
  wire   [31:0]                      crc_res32_rev                      ;
  wire   [15:0]                      crc_res16                          ;
  wire   [15:0]                      crc_res16_rev                      ;
  wire   [31:0]                      lfsr_res                           ;

  wire   [7:0]                       crc32_data                         ;

parameter [7:0] PADDR_CRC_CON        = 8'h00 ;
parameter [7:0] PADDR_CRC_DAT        = 8'h01 ;
parameter [7:0] PADDR_LFSR_DAT       = 8'h02 ;
parameter [7:0] PADDR_LFSR_KS        = 8'h03 ;

  assign      write_enable = i_psel & i_pwrite & i_penable ;
  wire        crc_con_wr   = write_enable & (i_paddr[9:2]==PADDR_CRC_CON);
  wire        crc_dat_wr   = write_enable & (i_paddr[9:2]==PADDR_CRC_DAT);
  wire        lfsr_dat_wr  = write_enable & (i_paddr[9:2]==PADDR_LFSR_DAT);
  wire        lfsr_ks      = write_enable & (i_paddr[9:2]==PADDR_LFSR_KS);
  wire [6:0]  crc_con_in   = crc_con_wr ? i_pwdata[6:0] : crc_con;
  wire [7:0]  crc_dat_in   = crc_dat_wr ? i_pwdata[7:0] : crc_dat;
  wire [31:0] lfsr_dat_in  = lfsr_dat_wr? i_pwdata      : lfsr_dat;

  always @ (posedge pclk or negedge presetn)
      if (~presetn) begin
          crc_con    <= 7'h0;
          crc_dat    <= 8'h0;
          crc_wr     <= 1'h0;
          lfsr_dat   <= 32'h0;
          lfsr_wr    <= 1'h0;
      end
      else begin
          crc_con    <= crc_con_in;
          crc_dat    <= crc_dat_in;
          crc_wr     <= crc_dat_wr;
          lfsr_dat   <= lfsr_dat_in;
          lfsr_wr    <= lfsr_dat_wr;
      end

  assign read_enable  = i_psel & ~i_pwrite & ~i_penable ;

  always @ (posedge pclk or negedge presetn)
      if (~presetn)
          READ_DATA <= 32'b0  ;
      else
          READ_DATA <= NXT_READ_DATA ;

  always @ (*) begin
      NXT_READ_DATA = READ_DATA ;
      if (read_enable)
          case (i_paddr[9:2])
              PADDR_CRC_CON    : NXT_READ_DATA  = {25'h0, crc_con};
              PADDR_CRC_DAT    : NXT_READ_DATA  = crc_res         ;
              PADDR_LFSR_DAT   : NXT_READ_DATA  = lfsr_res        ;
              default          : NXT_READ_DATA  = 32'h0           ;
          endcase
      else
          NXT_READ_DATA = READ_DATA ;
  end

  assign o_prdata                  = READ_DATA           ;

wire          crc_en     = crc_con[0];
wire          crc_init   = crc_con_wr & i_pwdata[1];
wire          crc_mode   = crc_con[2]; 
wire          crc32_en   = crc_en & crc_mode;
wire          crc16_en   = crc_en & ~crc_mode;
wire          crc32_wr   = crc_wr & crc_en & crc_mode;
wire          crc16_wr   = crc_wr & crc_en & ~crc_mode;
wire          crc32_init = crc_init & crc_en & crc_mode;
wire          crc16_init = crc_init & crc_en & ~crc_mode;

assign        crc_res32_rev  = crc_res32;
assign        crc32_data     = {crc_dat[0],crc_dat[1],crc_dat[2],crc_dat[3],crc_dat[4],crc_dat[5],crc_dat[6],crc_dat[7]};

assign        crc_res16_rev  = ~{crc_res16[0], crc_res16[1], crc_res16[2],  crc_res16[3],  crc_res16[4],  crc_res16[5],  crc_res16[6],  crc_res16[7],
                                 crc_res16[8], crc_res16[9], crc_res16[10], crc_res16[11], crc_res16[12], crc_res16[13], crc_res16[14], crc_res16[15]};
assign        crc_res    = crc16_en ? {16'h0, crc_res16_rev} : crc_res32_rev;
osr_crc32 u_crc32 (
    .crc_reg  (crc_res32    ), 
    .crc      (             ),

    .d        (crc32_data   ),
    .calc     (crc32_en     ),
    .init     (crc32_init   ),
    .d_valid  (crc32_wr     ),
    .clk      (pclk         ),
    .rst_n    (presetn      ) 
   );

osr_crc16 u_crc16 (
    .crc_reg  (crc_res16    ), 
    .crc      (             ),
    .d        (crc_dat[7:0] ),
    .calc     (crc16_en     ),
    .init     (crc16_init   ),
    .d_valid  (crc16_wr     ),
    .clk      (pclk         ),
    .rst_n    (presetn      ) 
   );

wire          lfsr_sel = crc_con[4];
wire          lfsr_en  = crc_con[5];
wire          lfsr_mode= crc_con[6];

osr_lfsr u_lfsr(
    .clk             (pclk              ),
    .rst_n           (presetn           ),
    .i_lfsr_mode     (lfsr_mode         ),
    .i_lfsr_ks       (lfsr_ks           ),
    .i_lfsr_en       (lfsr_en           ),
    .i_lfsr_sel      (lfsr_sel          ), 
    .i_seed_vld      (lfsr_wr           ),
    .i_seed_data     (lfsr_dat          ),
    .o_lfsr_data     (lfsr_res          )
);
endmodule
