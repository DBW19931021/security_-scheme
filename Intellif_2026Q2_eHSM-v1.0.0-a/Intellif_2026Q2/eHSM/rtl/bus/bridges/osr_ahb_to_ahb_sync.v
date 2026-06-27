//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_ahb_to_ahb_sync #(

  parameter       P_AW    = 32,  
  parameter       P_DW    = 32,  
  parameter       P_MW    = 4,   
  parameter       P_BURST = 0)   

 (

  input  wire            i_hclk,       
  input  wire            i_hresetn,    

  input  wire            i_hsels,
  input  wire [P_AW-1:0] i_haddrs,
  input  wire [1:0]      i_htranss,
  input  wire [2:0]      i_hsizes,
  input  wire            i_hwrites,
  input  wire            i_hreadys,
  input  wire [3:0]      i_hprots,
  input  wire [P_MW-1:0] i_hmasters,
  input  wire            i_hmastlocks,
  input  wire [P_DW-1:0] i_hwdatas,
  input  wire [2:0]      i_hbursts,

  output wire            o_hreadyouts,
  output wire            o_hresps,
  output wire [P_DW-1:0] o_hrdatas,

  output wire [P_AW-1:0] o_haddrm,
  output wire [1:0]      o_htransm,
  output wire [2:0]      o_hsizem,
  output wire            o_hwritem,
  output wire [3:0]      o_hprotm,
  output wire [P_MW-1:0] o_hmasterm,
  output wire            o_hmastlockm,
  output wire [P_DW-1:0] o_hwdatam,
  output wire [2:0]      o_hburstm,

  input  wire            i_hreadyoutm,
  input  wire            i_hresps,
  input  wire [P_DW-1:0] i_hrdatam);

  localparam      HSIZE_W   = (P_DW > 64) ? 3 : 2; 
  localparam      HSIZE_MAX = HSIZE_W - 1;       

  localparam TRN_IDLE   = 2'b00;             
  localparam TRN_BUSY   = 2'b01;             
  localparam TRN_NONSEQ = 2'b10;             
  localparam TRN_SEQ    = 2'b11;             

  localparam RESP_OKAY  = 1'b0;              
  localparam RESP_ERR   = 1'b1;              

  localparam BUR_SINGLE = 3'b000;            
  localparam BUR_INCR   = 3'b001;            
  localparam BUR_WRAP4  = 3'b010;            
  localparam BUR_INCR4  = 3'b011;            
  localparam BUR_WRAP8  = 3'b100;            
  localparam BUR_INCR8  = 3'b101;            
  localparam BUR_WRAP16 = 3'b110;            
  localparam BUR_INCR16 = 3'b111;            

  localparam FSM_IDLE  = 4'b0100;            
  localparam FSM_ADDR  = 4'b0001;            
  localparam FSM_UNLOCK= 4'b0000;            
  localparam FSM_WAIT  = 4'b0010;            
  localparam FSM_DONE  = 4'b0110;            
  localparam FSM_ERR1  = 4'b1000;            
  localparam FSM_ERR2  = 4'b1100;            

  reg [P_AW-1:0]  haddr_reg;
  reg           hwrite_reg;
  reg [HSIZE_MAX:0]  hsize_reg;
  reg    [3:0]  hprot_reg;
  reg [P_MW-1:0]  hmaster_reg;
  reg           hmastlock_reg;
  reg    [2:0]  hburst_reg;
  reg    [1:0]  hold_htrans_reg;            

  reg    [3:0]  reg_fsm_state;
  reg    [3:0]  nxt_fsm_state;

  wire          trans_finish_addr;           

  reg  [P_DW-1:0] hdata_reg;                   
  wire [P_DW-1:0] nxt_hdata_reg;               

  wire          data_update_read;            
  wire          data_update_write;           
  wire          data_update_both;            

  wire [1:0]    next_htransm;                
  reg [1:0]     htransm_reg;                 
  wire          trans_update;                

  wire  [1:0]   htransm_canc;                
  wire          hreadyoutm_canc;             
  wire          hrespm_canc;                 

  reg      lock_trans_valid;      
  wire     unlock_idle_valid;     

  always @(posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn) begin
       lock_trans_valid   <= 1'b0;
    end
    else if (trans_finish_addr & i_hmastlocks)
           begin

           lock_trans_valid   <= 1'b1;
           end
         else if (i_hreadys &((i_hsels & (~i_hmastlocks)) | (~i_hsels)))
           begin

           lock_trans_valid   <= 1'b0;
           end
    end

  assign   unlock_idle_valid =  i_hreadys & lock_trans_valid & (((~i_hmastlocks) & i_hsels)
                              | (~i_hsels));

  assign trans_finish_addr = i_hsels & i_htranss[1] & i_hreadys;

  always @(*) begin
    case (reg_fsm_state)
      FSM_IDLE: begin 
                   nxt_fsm_state = ~trans_finish_addr ? FSM_IDLE   :
                                   unlock_idle_valid  ? FSM_UNLOCK :
                                                        FSM_ADDR;
                end
      FSM_UNLOCK: begin 
                   nxt_fsm_state = FSM_ADDR;
                end
      FSM_ADDR: begin 
                   nxt_fsm_state = FSM_WAIT;
                end
      FSM_WAIT: begin 
                   nxt_fsm_state = hrespm_canc     ? FSM_ERR1 :
                                   hreadyoutm_canc ? FSM_DONE :
                                                     FSM_WAIT;
                end
      FSM_DONE: begin 
                   nxt_fsm_state = ~trans_finish_addr ? FSM_IDLE   :
                                   unlock_idle_valid  ? FSM_UNLOCK :
                                                        FSM_ADDR;
                end
      FSM_ERR1: begin 
                   nxt_fsm_state = FSM_ERR2;
                end
      FSM_ERR2: begin  
                   nxt_fsm_state = ~trans_finish_addr ? FSM_IDLE   :
                                   unlock_idle_valid  ? FSM_UNLOCK :
                                                        FSM_ADDR;
                end
      default : begin
                   nxt_fsm_state = 4'bxxxx; 
                end
    endcase
  end

  always @(posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn)
      reg_fsm_state <= FSM_IDLE;
    else
      reg_fsm_state <= nxt_fsm_state;
  end

  always @(posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn) begin
      hsize_reg       <= { HSIZE_W{1'b0}};
      hwrite_reg      <=     1'b0;
      hprot_reg       <= { 4{1'b0}};
      hmaster_reg     <= { P_MW{1'b0}};
      hold_htrans_reg <= TRN_IDLE;
    end
    else if (trans_finish_addr ) begin
      hsize_reg       <= i_hsizes[HSIZE_MAX:0];
      hwrite_reg      <= i_hwrites;
      hprot_reg       <= i_hprots;
      hmaster_reg     <= i_hmasters;
      hold_htrans_reg <= i_htranss;
    end
  end

  assign data_update_read  = (reg_fsm_state == FSM_WAIT) & (hwrite_reg==1'b0) & (hreadyoutm_canc==1'b1);
  assign data_update_write = (reg_fsm_state == FSM_ADDR) & (hwrite_reg==1'b1);
  assign data_update_both  = data_update_read | data_update_write;
  assign nxt_hdata_reg     = (data_update_write) ? i_hwdatas : i_hrdatam ;

  always @(posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn)
      hdata_reg <= {P_DW{1'b0}};
    else if (data_update_both)
      hdata_reg <= nxt_hdata_reg;
  end

  always @(posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn)
      hmastlock_reg <=  1'b0;
    else if (trans_finish_addr|unlock_idle_valid)
      hmastlock_reg <= i_hmastlocks;
  end

  assign   trans_update = (nxt_fsm_state == FSM_ADDR);

  generate

  if (P_BURST == 1)
    begin: gen_burst_support

    osr_ahb_to_ahb_sync_error_canc
     u_ahb_error_canc
     (

      .i_hclk      (i_hclk),
      .i_hclkEN    (1'b1),
      .i_hresetn   (i_hresetn),

      .i_htranss   (htransm_reg),

      .o_hreadyouts(hreadyoutm_canc),
      .o_hresps    (hrespm_canc),

      .o_htransm   (htransm_canc),

      .i_hreadyoutm   (i_hreadyoutm),
      .i_hresps    (i_hresps)
     );

  wire     busy_override;              
  wire     transm_addr_update;         

  assign   busy_override = i_htranss[0] & i_hsels; 

  assign   next_htransm  = unlock_idle_valid  ? TRN_IDLE : 
                           trans_update       ? ((reg_fsm_state == FSM_UNLOCK)? hold_htrans_reg :i_htranss):

                           busy_override      ? TRN_BUSY:
                                                TRN_IDLE;

  always @ (posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn)
        htransm_reg  <= TRN_IDLE;
    else if (hreadyoutm_canc)
        htransm_reg  <= next_htransm;
  end

   assign o_htransm        = htransm_canc;

   assign transm_addr_update = (trans_finish_addr | 
                                ((reg_fsm_state == FSM_ADDR) & busy_override)

                               );

   always @(posedge i_hclk or negedge i_hresetn) begin
     if (~i_hresetn) begin
       haddr_reg     <= {P_AW{1'b0}};
       end
     else if (transm_addr_update) begin
       haddr_reg     <= i_haddrs;
       end
   end

   always @(posedge i_hclk or negedge i_hresetn) begin
     if (~i_hresetn) begin
        hburst_reg    <= BUR_SINGLE;
        end
     else if (trans_finish_addr) begin
        hburst_reg    <= i_hbursts;
        end
   end

  assign o_hburstm = hburst_reg;

  end

  else begin: gen_no_burst_support

  assign  next_htransm  =  unlock_idle_valid  ? TRN_IDLE : 
                           trans_update       ? ((reg_fsm_state == FSM_UNLOCK)? {hold_htrans_reg[1],1'b0} :{i_htranss[1],1'b0}):

                                                TRN_IDLE;

  always @ (posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn)
        htransm_reg  <= TRN_IDLE;
    else if (i_hreadyoutm)
        htransm_reg  <= next_htransm;
  end

  assign o_htransm         = {htransm_reg[1], 1'b0}; 
  assign htransm_canc    = TRN_IDLE; 

  assign hrespm_canc     = i_hresps;
  assign hreadyoutm_canc = i_hreadyoutm;

  always @(posedge i_hclk or negedge i_hresetn) begin
    if (~i_hresetn)
      begin
       haddr_reg     <= {P_AW{1'b0}};
      end
    else if (trans_finish_addr) 
      begin
       haddr_reg     <= i_haddrs;
      end
  end

   assign  o_hburstm    = BUR_SINGLE; 

  end 
  endgenerate

  assign o_hreadyouts   = reg_fsm_state[2];
  assign o_hresps       = reg_fsm_state[3];
  assign o_hrdatas      = hdata_reg;

  assign o_haddrm       = haddr_reg;
  assign o_hsizem[2]    = (P_DW > 64) ? hsize_reg[HSIZE_MAX] : 1'b0;
  assign o_hsizem[1:0]  = hsize_reg[1:0];
  assign o_hwritem      = hwrite_reg;
  assign o_hprotm       = hprot_reg;
  assign o_hmasterm     = hmaster_reg;
  assign o_hmastlockm   = hmastlock_reg;
  assign o_hwdatam      = hdata_reg;

endmodule
