//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_uart (

  input  wire        pclk,     
  input  wire        presetn,  

  input  wire        psel,     
  input  wire [11:2] paddr,    
  input  wire        penable,  
  input  wire        pwrite,   
  input  wire [31:0] pwdata,   

  output wire [31:0] prdata,   
  output wire        pready,   
  output wire        pslverr,  

  input  wire        rxd,      
  output wire        txd,      
  output wire        txen,     
  output wire        baudtick, 

  output wire        txint,    
  output wire        rxint,    
  output wire        txovrint, 
  output wire        rxovrint, 
  output wire        uartint); 

  `ifndef OSR_FPGA
  localparam [31:0] UART_CLK  = `OSR_FREQ_I_CLK;
  localparam [31:0] BAUD_RATE = `OSR_BUAD_RATE;
  `else
  localparam [31:0] UART_CLK  = 10_000_000;
  localparam [31:0] BAUD_RATE = 115200;
  `endif
  localparam [19:0] BAUD_DIV  = ((UART_CLK+(BAUD_RATE>>1))/BAUD_RATE) + 20'b0;

wire          read_enable;
wire          write_enable;
wire          write_enable00; 
wire          write_enable04; 
wire          write_enable08; 
wire          write_enable0c; 
wire          write_enable10; 
reg     [7:0] read_mux_byte0; 
reg     [7:0] read_mux_byte0_reg; 
wire   [31:0] read_mux_word;  

reg     [6:0] reg_ctrl;       
reg     [7:0] reg_tx_buf;     
reg     [7:0] reg_rx_buf;     
reg    [19:0] reg_baud_div;   

reg    [15:0] reg_baud_cntr_i; 
wire   [15:0] nxt_baud_cntr_i;
reg     [3:0] reg_baud_cntr_f; 
wire    [3:0] nxt_baud_cntr_f;
wire    [3:0] mapped_cntr_f;   
reg           reg_baud_tick;   
reg           baud_updated;    
wire          reload_i;        
wire          reload_f;        
wire          baud_div_en;     

wire    [3:0] uart_status;     
reg           reg_rx_overrun;  
wire          rx_overrun;      
reg           reg_tx_overrun;  
wire          tx_overrun;      
wire          nxt_rx_overrun;  
wire          nxt_tx_overrun;  

reg           reg_txintr;      
reg           reg_rxintr;      
wire          tx_overflow_intr;
wire          rx_overflow_intr;
wire    [3:0] intr_state;      
wire    [1:0] intr_stat_set;   
wire    [1:0] intr_stat_clear; 

reg     [3:0] tx_state;    
reg     [4:0] nxt_tx_state;
wire          tx_state_update;
wire          tx_state_inc; 
reg     [3:0] tx_tick_cnt;  
wire    [4:0] nxt_tx_tick_cnt;
reg     [7:0] tx_shift_buf;      
wire    [7:0] nxt_tx_shift_buf;  
wire          tx_buf_ctrl_shift; 
wire          tx_buf_ctrl_load;  
reg           tx_buf_full;  
reg           reg_txd;      
wire          nxt_txd;      
wire          update_reg_txd; 
wire          tx_buf_clear; 

reg     [2:0] rxd_lpf;     
wire    [2:0] nxt_rxd_lpf;
wire          rx_shift_in; 

reg     [3:0] rx_state;   
reg     [4:0] nxt_rx_state;
wire          rx_state_update;
reg     [3:0] rx_tick_cnt; 
wire    [4:0] nxt_rx_tick_cnt;
wire          update_rx_tick_cnt;
wire          rx_state_inc;
reg     [6:0] rx_shift_buf;
wire    [6:0] nxt_rx_shift_buf;
reg           rx_buf_full;  
wire          nxt_rx_buf_full;
wire          rxbuf_sample; 
wire          rx_data_read; 
wire    [7:0] nxt_rx_buf;

assign  read_enable  = psel & (~pwrite); 
assign  write_enable = psel & (~penable) & pwrite; 
assign  write_enable00 = write_enable & (paddr[11:2] == 10'h000);
assign  write_enable04 = write_enable & (paddr[11:2] == 10'h001);
assign  write_enable08 = write_enable & (paddr[11:2] == 10'h002);
assign  write_enable0c = write_enable & (paddr[11:2] == 10'h003);
assign  write_enable10 = write_enable & (paddr[11:2] == 10'h004);

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_tx_buf <= {8{1'b0}};
    else if (write_enable00)
      reg_tx_buf <= pwdata[7:0];
  end

  assign nxt_rx_overrun = (reg_rx_overrun & (~((write_enable04|write_enable0c) & pwdata[3]))) | rx_overrun;
  assign nxt_tx_overrun = (reg_tx_overrun & (~((write_enable04|write_enable0c) & pwdata[2]))) | tx_overrun;

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_rx_overrun <= 1'b0;
    else if (rx_overrun | write_enable04 | write_enable0c)
      reg_rx_overrun <= nxt_rx_overrun;
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_tx_overrun <= 1'b0;
    else if (tx_overrun | write_enable04 | write_enable0c)
      reg_tx_overrun <= nxt_tx_overrun;
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_ctrl <= {7{1'b0}};
    else if (write_enable08)
      reg_ctrl <= pwdata[6:0];
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_baud_div <= BAUD_DIV;
    else if (write_enable10)
      reg_baud_div <= pwdata[19:0];
  end

  assign uart_status = {reg_rx_overrun, reg_tx_overrun, rx_buf_full, tx_buf_full};

 always @(*)
  begin
     case (paddr[11:2])
     10'h0: read_mux_byte0 =  reg_rx_buf;
     10'h1: read_mux_byte0 =  {{4{1'b0}},uart_status};
     10'h2: read_mux_byte0 =  {{1{1'b0}},reg_ctrl};
     10'h3: read_mux_byte0 =  {{4{1'b0}},intr_state};
     10'h4: read_mux_byte0 =  reg_baud_div[7:0];
     default:  read_mux_byte0 =   {8{1'b0}};
     endcase
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      read_mux_byte0_reg      <= {8{1'b0}};
    else if (read_enable)
      read_mux_byte0_reg      <= read_mux_byte0;
  end

  assign read_mux_word[ 7: 0] = read_mux_byte0_reg;
  assign read_mux_word[19: 8] = (paddr[11:2]==10'h004) ? reg_baud_div[19:8] : {12{1'b0}};
  assign read_mux_word[31:20] = {12{1'b0}};

  assign prdata[31: 0] = (read_enable) ? read_mux_word : {32{1'b0}};
  assign pready  = 1'b1; 
  assign pslverr = 1'b0; 

  assign baud_div_en    = (reg_ctrl[1:0] != 2'b00);
  assign mapped_cntr_f  = {reg_baud_cntr_f[0],reg_baud_cntr_f[1],
                           reg_baud_cntr_f[2],reg_baud_cntr_f[3]};

  assign reload_i      = (baud_div_en &
         (((mapped_cntr_f >= reg_baud_div[3:0]) &
         (reg_baud_cntr_i[15:1] == {15{1'b0}})) |
         (reg_baud_cntr_i[15:0] == {16{1'b0}})));

  assign nxt_baud_cntr_i = (baud_updated | reload_i) ? reg_baud_div[19:4] :
                           (reg_baud_cntr_i - 16'h0001);

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_baud_cntr_i   <= {16{1'b0}};
    else if (baud_updated | baud_div_en)
      reg_baud_cntr_i   <= nxt_baud_cntr_i;
  end

  assign reload_f      = baud_div_en & (reg_baud_cntr_f==4'h0) &
                        reload_i;

  assign nxt_baud_cntr_f =
                        (reload_f|baud_updated) ? 4'hf :
                        (reg_baud_cntr_f - 4'h1);

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_baud_cntr_f   <= {4{1'b0}};
    else if (baud_updated | reload_f | reload_i)
      reg_baud_cntr_f   <= nxt_baud_cntr_f;
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      baud_updated    <= 1'b0;
    else if (write_enable10 | baud_updated)

      baud_updated    <= write_enable10;
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_baud_tick    <= 1'b0;
    else if (reload_i | reg_baud_tick)
      reg_baud_tick    <= reload_i;
  end

  assign baudtick = reg_baud_tick;

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      tx_buf_full     <= 1'b0;
    else if (write_enable00 | tx_buf_clear)
      tx_buf_full     <= write_enable00;
  end

  assign nxt_tx_tick_cnt = ((tx_state==4'h1) & reg_baud_tick) ? {5{1'b0}} :
                        tx_tick_cnt + {{4{1'b0}},reg_baud_tick};

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      tx_tick_cnt     <= {4{1'b0}};
    else if (reg_baud_tick)
      tx_tick_cnt     <= nxt_tx_tick_cnt[3:0];
  end

  assign tx_state_inc   = (((&tx_tick_cnt)|(tx_state==4'h1)) & reg_baud_tick)|reg_ctrl[6];

  assign tx_buf_clear   = ((tx_state==4'h0) & tx_buf_full) |
                        ((tx_state==4'hb) & tx_buf_full & tx_state_inc);

  always @(tx_state or tx_buf_full or tx_state_inc or reg_ctrl)
  begin
  case (tx_state)
    0: begin
       nxt_tx_state = (tx_buf_full & reg_ctrl[0]) ? 5'h01 : 5'h00;  
       end
    1,                         
    2,3,4,5,6,7,8,9,10: begin  
       nxt_tx_state = tx_state + {3'b000,tx_state_inc} + 5'b0;
       end
    11: begin 
       nxt_tx_state = (tx_state_inc) ? ( tx_buf_full ? 5'h02:5'h00) : {1'b0, tx_state};
       end
    default:
       nxt_tx_state = {5{1'b0}};
  endcase
  end

  assign tx_state_update = tx_state_inc | ((tx_state==4'h0) & tx_buf_full & reg_ctrl[0]) | (tx_state>4'd11);

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      tx_state        <= {4{1'b0}};
    else if (tx_state_update)
      tx_state        <= nxt_tx_state[3:0];
  end

  assign tx_buf_ctrl_load  = (((tx_state==4'h0) & tx_buf_full) |
                              ((tx_state==4'hb) & tx_buf_full & tx_state_inc));
  assign tx_buf_ctrl_shift =  ((tx_state>4'h2) & tx_state_inc);

  assign nxt_tx_shift_buf = tx_buf_ctrl_load ? reg_tx_buf : {1'b1,tx_shift_buf[7:1]};

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      tx_shift_buf    <= {8{1'b0}};
    else if (tx_buf_ctrl_shift | tx_buf_ctrl_load)
      tx_shift_buf    <= nxt_tx_shift_buf;
  end

  assign nxt_txd = (tx_state==4'h2) ? 1'b0 :
                   (tx_state>4'h2) ? tx_shift_buf[0] : 1'b1;

  assign update_reg_txd = (nxt_txd != reg_txd);

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_txd         <= 1'b1;
    else if (update_reg_txd)
      reg_txd         <= nxt_txd;
  end

  assign tx_overrun = tx_buf_full & (~tx_buf_clear) & write_enable00;

  assign txd  = reg_txd;
  assign txen = reg_ctrl[0];

  wire rxd_sync   ;
  osr_sync_level_3 u_rx_sync_level_3(.i_clk (pclk), .i_rst_n (presetn), .i_async (rxd), .o_sync (rxd_sync));

  wire rxd_sync_3 = reg_ctrl[1] ? rxd_sync : 1'h1;

  assign nxt_rxd_lpf = {rxd_lpf[1:0], rxd_sync_3};

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      rxd_lpf <= 3'b111;
    else if (reg_baud_tick)
      rxd_lpf <= nxt_rxd_lpf;
  end

  assign rx_shift_in = (rxd_lpf[1] & rxd_lpf[0]) |
                       (rxd_lpf[1] & rxd_lpf[2]) |
                       (rxd_lpf[0] & rxd_lpf[2]);

  assign nxt_rx_tick_cnt = ((rx_state==4'h0) & (~rx_shift_in)) ? 5'h08 :
                        rx_tick_cnt + {{4{1'b0}},reg_baud_tick};

  assign update_rx_tick_cnt = ((rx_state==4'h0) & (~rx_shift_in)) | reg_baud_tick;

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      rx_tick_cnt    <= {4{1'b0}};
    else if (update_rx_tick_cnt)
      rx_tick_cnt    <= nxt_rx_tick_cnt[3:0];
  end

  assign rx_state_inc   = ((&rx_tick_cnt) & reg_baud_tick);

  assign nxt_rx_buf_full = rxbuf_sample | (rx_buf_full & (~rx_data_read));

  assign rxbuf_sample  = ((rx_state==4'h9) & rx_state_inc);

  assign rx_data_read   = (psel & (~penable) & (paddr[11:2]==10'h000) & (~pwrite));

  assign rx_overrun = rx_buf_full & rxbuf_sample & (~rx_data_read);

  always @(rx_state or rx_shift_in or rx_state_inc or reg_ctrl)
  begin
  case (rx_state)
    0: begin
       nxt_rx_state = ((~rx_shift_in) & reg_ctrl[1]) ? 5'h01 : 5'h00;  
       end
    1,                      
    2,3,4,5,6,7,8,9: begin  
       nxt_rx_state = rx_state + {3'b000,rx_state_inc} + 5'b0;
       end
    10: begin 
       nxt_rx_state = (rx_state_inc) ? 5'h00 : 5'h0a;
       end
    default:
       nxt_rx_state = {5{1'b0}};
  endcase
  end

  assign rx_state_update = rx_state_inc |  ((~rx_shift_in) & reg_ctrl[1]);

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      rx_state       <= {4{1'b0}};
    else if (rx_state_update)
      rx_state       <= nxt_rx_state[3:0];
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      rx_buf_full     <= 1'b0;
    else if (rxbuf_sample | rx_data_read)
      rx_buf_full     <= nxt_rx_buf_full;
  end

  assign nxt_rx_buf     = {rx_shift_in, rx_shift_buf};

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_rx_buf      <= {8{1'b0}};
    else if  (rxbuf_sample)
      reg_rx_buf      <= nxt_rx_buf;
  end

  assign nxt_rx_shift_buf= {rx_shift_in, rx_shift_buf[6:1]};

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      rx_shift_buf    <= {7{1'b0}};
    else if (rx_state_inc)
      rx_shift_buf    <= nxt_rx_shift_buf;
  end

  assign intr_stat_set[1] = reg_ctrl[3] & rxbuf_sample; 
  assign intr_stat_set[0] = reg_ctrl[2] & reg_ctrl[0] & tx_buf_full & tx_buf_clear;

  assign intr_stat_clear[1:0] = {2{write_enable0c}} & pwdata[1:0];

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_txintr    <= 1'b0;
    else if (intr_stat_set[0] | intr_stat_clear[0])
      reg_txintr    <= intr_stat_set[0];
  end

  always @(posedge pclk or negedge presetn)
  begin
    if (~presetn)
      reg_rxintr    <= 1'b0;
    else if (intr_stat_set[1] | intr_stat_clear[1])
      reg_rxintr    <= intr_stat_set[1];
  end

  assign rx_overflow_intr = reg_rx_overrun & reg_ctrl[5];
  assign tx_overflow_intr = reg_tx_overrun & reg_ctrl[4];

  assign intr_state = {rx_overflow_intr, tx_overflow_intr, reg_rxintr, reg_txintr};

  assign txint    = reg_txintr;
  assign rxint    = reg_rxintr;
  assign txovrint = tx_overflow_intr;
  assign rxovrint = rx_overflow_intr;
  assign uartint  = reg_txintr | reg_rxintr | tx_overflow_intr | rx_overflow_intr;

  endmodule
