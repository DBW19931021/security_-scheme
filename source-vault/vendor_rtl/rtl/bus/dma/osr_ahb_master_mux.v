//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_ahb_master_mux #(
    parameter P_PORT0_ENABLE=1,
    parameter P_PORT1_ENABLE=1,
    parameter P_PORT2_ENABLE=1,
    parameter P_AWIDTH=32,
    parameter P_DWIDTH=32
)
(
    input  wire                i_hclk,
    input  wire                i_hreset_n,

    input  wire                i_hsels0,
    input  wire [P_AWIDTH-1:0] i_haddrs0,
    input  wire [1:0]          i_htranss0,
    input  wire [2:0]          i_hsizes0,
    input  wire                i_hwrites0,
    input  wire                i_hreadys0,
    input  wire [3:0]          i_hprots0,
    input  wire [2:0]          i_hbursts0,
    input  wire                i_hmastlocks0,
    input  wire [P_DWIDTH-1:0] i_hwdatas0,
    output wire                o_hreadyouts0,
    output wire                o_hresps0,
    output wire [P_DWIDTH-1:0] o_hrdatas0,

    input  wire                i_hsels1,
    input  wire [P_AWIDTH-1:0] i_haddrs1,
    input  wire [1:0]          i_htranss1,
    input  wire [2:0]          i_hsizes1,
    input  wire                i_hwrites1,
    input  wire                i_hreadys1,
    input  wire [3:0]          i_hprots1,
    input  wire [2:0]          i_hbursts1,
    input  wire                i_hmastlocks1,
    input  wire [P_DWIDTH-1:0] i_hwdatas1,
    output wire                o_hreadyouts1,
    output wire                o_hresps1,
    output wire [P_DWIDTH-1:0] o_hrdatas1,

    input  wire                i_hsels2,
    input  wire [P_AWIDTH-1:0] i_haddrs2,
    input  wire [1:0]          i_htranss2,
    input  wire [2:0]          i_hsizes2,
    input  wire                i_hwrites2,
    input  wire                i_hreadys2,
    input  wire [3:0]          i_hprots2,
    input  wire [2:0]          i_hbursts2,
    input  wire                i_hmastlocks2,
    input  wire [P_DWIDTH-1:0] i_hwdatas2,
    output wire                o_hreadyouts2,
    output wire                o_hresps2,
    output wire [P_DWIDTH-1:0] o_hrdatas2,

    output wire                o_hselm,
    output wire [P_AWIDTH-1:0] o_haddrm,
    output wire [1:0]          o_htransm,
    output wire [2:0]          o_hsizem,
    output wire                o_hwritem,
    output wire                o_hreadym,
    output wire [3:0]          o_hprotm,
    output wire [2:0]          o_hburstm,
    output wire                o_hmastlockm,
    output wire [P_DWIDTH-1:0] o_hwdatam,
    input  wire                i_hreadyoutm,
    input  wire                i_hrespm,
    input  wire [P_DWIDTH-1:0] i_hrdatam,

    output wire  [1:0]         o_hmasterm
);

  localparam       HSIZE_W   = (P_DWIDTH > 64) ? 3 : 2;
  localparam       HSIZE_MAX = HSIZE_W - 1;

  reg  [2:0]          mux_sel_addr_phase;
  reg  [2:0]          mux_sel_data_phase;
  reg  [2:0]          mux_sel_addr_phase_reg;
  wire [2:0]          bus_request;
  wire                port_s0_trans_valid;
  wire                port_s0_input_hold;
  reg                 port_s0_hold_active;
  wire                nxt_port_s0_hold_active;

  reg  [P_AWIDTH-1:0] haddr_s0_reg;
  reg                 htrans_s0_reg;
  reg                 hwrite_s0_reg;
  reg  [HSIZE_MAX:0]  hsize_s0_reg;
  reg  [3:0]          hprot_s0_reg;
  reg  [2:0]          hburst_s0_reg;
  reg                 hmastlock_s0_reg;

  wire [P_AWIDTH-1:0] haddr_s0_ips;
  wire [1:0]          htrans_s0_ips;
  wire                hwrite_s0_ips;
  wire [HSIZE_MAX:0]  hsize_s0_ips;
  wire [3:0]          hprot_s0_ips;
  wire [2:0]          hburst_s0_ips;
  wire                hmastlock_s0_ips;
  wire                hready_s0_ips;

  wire                port_s1_trans_valid;
  wire                port_s1_input_hold;
  reg                 port_s1_hold_active;
  wire                nxt_port_s1_hold_active;

  reg  [P_AWIDTH-1:0] haddr_s1_reg;
  reg                 htrans_s1_reg;
  reg                 hwrite_s1_reg;
  reg  [HSIZE_MAX:0]  hsize_s1_reg;
  reg  [3:0]          hprot_s1_reg;
  reg  [2:0]          hburst_s1_reg;
  reg                 hmastlock_s1_reg;

  wire [P_AWIDTH-1:0]haddr_s1_ips;
  wire [1:0]         htrans_s1_ips;
  wire               hwrite_s1_ips;
  wire [HSIZE_MAX:0] hsize_s1_ips;
  wire [3:0]         hprot_s1_ips;
  wire [2:0]         hburst_s1_ips;
  wire               hmastlock_s1_ips;
  wire               hready_s1_ips;

  wire               port_s2_trans_valid;
  wire               port_s2_input_hold;
  reg                port_s2_hold_active;
  wire               nxt_port_s2_hold_active;

  reg  [P_AWIDTH-1:0]haddr_s2_reg;
  reg                htrans_s2_reg;
  reg                hwrite_s2_reg;
  reg  [HSIZE_MAX:0] hsize_s2_reg;
  reg  [3:0]         hprot_s2_reg;
  reg  [2:0]         hburst_s2_reg;
  reg                hmastlock_s2_reg;

  wire [P_AWIDTH-1:0]haddr_s2_ips;
  wire [1:0]         htrans_s2_ips;
  wire               hwrite_s2_ips;
  wire [HSIZE_MAX:0] hsize_s2_ips;
  wire [3:0]         hprot_s2_ips;
  wire [2:0]         hburst_s2_ips;
  wire               hmastlock_s2_ips;
  wire               hready_s2_ips;

  wire               hsel_mux;
  wire [P_AWIDTH-1:0]haddr_mux;
  wire [1:0]         htrans_mux;
  wire               hwrite_mux;
  wire [HSIZE_MAX:0] hsize_mux;
  wire [2:0]         hburst_mux;
  wire [3:0]         hprot_mux;
  wire               hmastlock_mux;
  wire               hready_mux;
  wire [P_DWIDTH-1:0]hwdata_mux;

  wire [2:0]         port_enable_mask;
  reg                active_transfer_reg;
  wire               nxt_active_transfer;

  assign port_enable_mask[0] = (P_PORT0_ENABLE!=1'b0) ? 1'b1 : 1'b0;
  assign port_enable_mask[1] = (P_PORT1_ENABLE!=1'b0) ? 1'b1 : 1'b0;
  assign port_enable_mask[2] = (P_PORT2_ENABLE!=1'b0) ? 1'b1 : 1'b0;

  assign port_s0_trans_valid = i_hsels0 & i_htranss0[1] & i_hreadys0;
  assign port_s0_input_hold  = port_s0_trans_valid & (~(mux_sel_addr_phase[0] & i_hreadyoutm));
  assign nxt_port_s0_hold_active = port_s0_input_hold | (port_s0_hold_active & (~(mux_sel_addr_phase[0] & i_hreadyoutm)));

  always @(posedge i_hclk or negedge i_hreset_n)
  begin
    if (~i_hreset_n)
      port_s0_hold_active <= 1'b0;
    else if (port_enable_mask[0])
      port_s0_hold_active <= nxt_port_s0_hold_active;
  end

  always @(posedge i_hclk or negedge i_hreset_n)
  begin
    if (~i_hreset_n)
      begin
      haddr_s0_reg     <= {P_AWIDTH{1'b0}};
      htrans_s0_reg    <= 1'b0;
      hwrite_s0_reg    <= 1'b0;
      hsize_s0_reg     <= {HSIZE_W{1'b0}};
      hprot_s0_reg     <= {4{1'b0}};
      hburst_s0_reg    <= {3{1'b0}};
      hmastlock_s0_reg <= 1'b0;
      end
    else if (port_s0_input_hold & port_enable_mask[0])
      begin
      haddr_s0_reg     <= i_haddrs0;
      htrans_s0_reg    <= i_htranss0[0];
      hwrite_s0_reg    <= i_hwrites0;
      hsize_s0_reg     <= i_hsizes0[HSIZE_MAX:0];
      hprot_s0_reg     <= i_hprots0;
      hburst_s0_reg    <= i_hbursts0;
      hmastlock_s0_reg <= i_hmastlocks0;
      end
  end

  assign haddr_s0_ips     = (port_s0_hold_active) ? haddr_s0_reg    : i_haddrs0;
  assign htrans_s0_ips    = (port_s0_hold_active) ? {1'b1, htrans_s0_reg}: i_htranss0;
  assign hwrite_s0_ips    = (port_s0_hold_active) ? hwrite_s0_reg   : i_hwrites0;
  assign hsize_s0_ips     = (port_s0_hold_active) ? hsize_s0_reg    : i_hsizes0[HSIZE_MAX:0];
  assign hprot_s0_ips     = (port_s0_hold_active) ? hprot_s0_reg    : i_hprots0;
  assign hburst_s0_ips    = (port_s0_hold_active) ? hburst_s0_reg   : i_hbursts0;
  assign hmastlock_s0_ips = (port_s0_hold_active) ? hmastlock_s0_reg: i_hmastlocks0;
  assign hready_s0_ips    = (port_s0_hold_active) ? 1'b1            : i_hreadys0;

  assign o_hreadyouts0 = (~port_s0_hold_active) &
                       ((mux_sel_data_phase[0]) ? i_hreadyoutm : 1'b1);
  assign o_hresps0     = ((mux_sel_data_phase[0]) ? i_hrespm     : 1'b0);
  assign o_hrdatas0    =                            i_hrdatam;

  assign port_s1_trans_valid = i_hsels1 & i_htranss1[1] & i_hreadys1;
  assign port_s1_input_hold  = port_s1_trans_valid & (~(mux_sel_addr_phase[1] & i_hreadyoutm));
  assign nxt_port_s1_hold_active = port_s1_input_hold | (port_s1_hold_active & (~(mux_sel_addr_phase[1] & i_hreadyoutm)));

  always @(posedge i_hclk or negedge i_hreset_n)
  begin
    if (~i_hreset_n)
      port_s1_hold_active <= 1'b0;
    else if (port_enable_mask[1])
      port_s1_hold_active <= nxt_port_s1_hold_active;
  end
  always @(posedge i_hclk or negedge i_hreset_n)
  begin
    if (~i_hreset_n)
      begin
      haddr_s1_reg     <= {P_AWIDTH{1'b0}};
      htrans_s1_reg    <= 1'b0;
      hwrite_s1_reg    <= 1'b0;
      hsize_s1_reg     <= {HSIZE_W{1'b0}};
      hprot_s1_reg     <= {4{1'b0}};
      hburst_s1_reg    <= {3{1'b0}};
      hmastlock_s1_reg <= 1'b0;
      end
    else if (port_s1_input_hold & port_enable_mask[1])
      begin
      haddr_s1_reg     <= i_haddrs1;
      htrans_s1_reg    <= i_htranss1[0];
      hwrite_s1_reg    <= i_hwrites1;
      hsize_s1_reg     <= i_hsizes1[HSIZE_MAX:0];
      hprot_s1_reg     <= i_hprots1;
      hburst_s1_reg    <= i_hbursts1;
      hmastlock_s1_reg <= i_hmastlocks1;
      end
  end

  assign haddr_s1_ips     = (port_s1_hold_active) ? haddr_s1_reg    : i_haddrs1;
  assign htrans_s1_ips    = (port_s1_hold_active) ? {1'b1, htrans_s1_reg}: i_htranss1;
  assign hwrite_s1_ips    = (port_s1_hold_active) ? hwrite_s1_reg   : i_hwrites1;
  assign hsize_s1_ips     = (port_s1_hold_active) ? hsize_s1_reg    : i_hsizes1[HSIZE_MAX:0];
  assign hprot_s1_ips     = (port_s1_hold_active) ? hprot_s1_reg    : i_hprots1;
  assign hburst_s1_ips    = (port_s1_hold_active) ? hburst_s1_reg   : i_hbursts1;
  assign hmastlock_s1_ips = (port_s1_hold_active) ? hmastlock_s1_reg: i_hmastlocks1;
  assign hready_s1_ips    = (port_s1_hold_active) ? 1'b1            : i_hreadys1;

  assign o_hreadyouts1 = (~port_s1_hold_active) &
                       ((mux_sel_data_phase[1]) ? i_hreadyoutm : 1'b1);
  assign o_hresps1     = ((mux_sel_data_phase[1]) ? i_hrespm     : 1'b0);
  assign o_hrdatas1    =                            i_hrdatam;

  assign port_s2_trans_valid = i_hsels2 & i_htranss2[1] & i_hreadys2;

  assign port_s2_input_hold  = port_s2_trans_valid & (~(mux_sel_addr_phase[2] & i_hreadyoutm));

  assign nxt_port_s2_hold_active = port_s2_input_hold | (port_s2_hold_active & (~(mux_sel_addr_phase[2] & i_hreadyoutm)));

  always @(posedge i_hclk or negedge i_hreset_n)
  begin
    if (~i_hreset_n)
      port_s2_hold_active <= 1'b0;
    else if (port_enable_mask[2])
      port_s2_hold_active <= nxt_port_s2_hold_active;
  end

  always @(posedge i_hclk or negedge i_hreset_n)
  begin
    if (~i_hreset_n)
      begin
      haddr_s2_reg     <= {P_AWIDTH{1'b0}};
      htrans_s2_reg    <= 1'b0;
      hwrite_s2_reg    <= 1'b0;
      hsize_s2_reg     <= {HSIZE_W{1'b0}};
      hprot_s2_reg     <= {4{1'b0}};
      hburst_s2_reg    <= {3{1'b0}};
      hmastlock_s2_reg <= 1'b0;
      end
    else if (port_s2_input_hold & port_enable_mask[2])
      begin
      haddr_s2_reg     <= i_haddrs2;
      htrans_s2_reg    <= i_htranss2[0];
      hwrite_s2_reg    <= i_hwrites2;
      hsize_s2_reg     <= i_hsizes2[HSIZE_MAX:0];
      hprot_s2_reg     <= i_hprots2;
      hburst_s2_reg    <= i_hbursts2;
      hmastlock_s2_reg <= i_hmastlocks2;
      end
  end

  assign haddr_s2_ips     = (port_s2_hold_active) ? haddr_s2_reg    : i_haddrs2;
  assign htrans_s2_ips    = (port_s2_hold_active) ? {1'b1, htrans_s2_reg}: i_htranss2;
  assign hwrite_s2_ips    = (port_s2_hold_active) ? hwrite_s2_reg   : i_hwrites2;
  assign hsize_s2_ips     = (port_s2_hold_active) ? hsize_s2_reg    : i_hsizes2[HSIZE_MAX:0];
  assign hprot_s2_ips     = (port_s2_hold_active) ? hprot_s2_reg    : i_hprots2;
  assign hburst_s2_ips    = (port_s2_hold_active) ? hburst_s2_reg   : i_hbursts2;
  assign hmastlock_s2_ips = (port_s2_hold_active) ? hmastlock_s2_reg: i_hmastlocks2;
  assign hready_s2_ips    = (port_s2_hold_active) ? 1'b1            : i_hreadys2;

  assign o_hreadyouts2 = (~port_s2_hold_active) &
                       ((mux_sel_data_phase[2]) ? i_hreadyoutm : 1'b1);
  assign o_hresps2     = ((mux_sel_data_phase[2]) ? i_hrespm     : 1'b0);
  assign o_hrdatas2    =                            i_hrdatam;

        wire   trans_announced_stalled = hsel_mux & htrans_mux[1] & (~hready_mux);
        reg    trans_announced_stalled_reg;

        always @(posedge i_hclk or negedge i_hreset_n)
          begin
          if (~i_hreset_n)
            trans_announced_stalled_reg <= 1'b0;
          else
            trans_announced_stalled_reg <= trans_announced_stalled;
          end

        reg    hsel_lock;
        wire   next_hsel_lock;
        assign next_hsel_lock = (hsel_mux & hmastlock_mux) ? 1'b1 :
                                (hmastlock_mux == 1'b0) ? 1'b0 :
                                hsel_lock;

        always @ (negedge i_hreset_n or posedge i_hclk)
          begin : p_hsel_lock
            if (~i_hreset_n)
              hsel_lock <= 1'b0;
            else
              if (hready_mux)
                hsel_lock <= next_hsel_lock;
          end

               wire   locked_active_trans;
        reg    locked_active_trans_reg;
        assign locked_active_trans = hmastlock_mux & (hsel_lock | hsel_mux);

        always @(posedge i_hclk or negedge i_hreset_n)
          begin
          if (~i_hreset_n)
            locked_active_trans_reg <= 1'b0;
          else if (hready_mux)
            locked_active_trans_reg <= locked_active_trans;
          end

        wire   fixed_length_burst0 = i_hsels0 & i_htranss0[0] & (i_hbursts0[2:1] != 2'b00) & mux_sel_addr_phase_reg[0];
        wire   fixed_length_burst1 = i_hsels1 & i_htranss1[0] & (i_hbursts1[2:1] != 2'b00) & mux_sel_addr_phase_reg[1];
        wire   fixed_length_burst2 = i_hsels2 & i_htranss2[0] & (i_hbursts2[2:1] != 2'b00) & mux_sel_addr_phase_reg[2];
        wire   fixed_length_burst = fixed_length_burst0 | fixed_length_burst1 | fixed_length_burst2;

        reg   reg_fixed_length_burst;
        wire  nxt_fixed_length_burst;
        reg   reg_fixed_length_burst_err;
        wire  nxt_fixed_length_burst_err;

        assign nxt_fixed_length_burst = (hburst_mux[2:1] != 2'b00) & (|htrans_mux);

        always @(posedge i_hclk or negedge i_hreset_n)
          begin
          if (~i_hreset_n)
            reg_fixed_length_burst <= 1'b0;
          else if (hready_mux)
            reg_fixed_length_burst <= nxt_fixed_length_burst;
          end

        assign nxt_fixed_length_burst_err = active_transfer_reg & reg_fixed_length_burst &
                                            (~i_hreadyoutm) & i_hrespm;

        always @(posedge i_hclk or negedge i_hreset_n)
          begin
          if (~i_hreset_n)
            reg_fixed_length_burst_err <= 1'b0;
          else if (nxt_fixed_length_burst_err|reg_fixed_length_burst_err)
            reg_fixed_length_burst_err <= nxt_fixed_length_burst_err;
          end

    always @(posedge i_hclk or negedge i_hreset_n)
    begin
      if (~i_hreset_n)
        mux_sel_addr_phase_reg <= 3'b000;
      else
        mux_sel_addr_phase_reg <= mux_sel_addr_phase;
    end

  reg  reg_round_robin_state;
  wire nxt_round_robin_state;
  assign nxt_round_robin_state = (mux_sel_addr_phase[1] & port_enable_mask[1]) |
                                 (reg_round_robin_state & ((~mux_sel_addr_phase[0]) & port_enable_mask[0]));

  always @(posedge i_hclk or negedge i_hreset_n)
    begin
      if (~i_hreset_n)
        reg_round_robin_state <= 1'b0;
      else if (hready_mux)
        reg_round_robin_state <= nxt_round_robin_state;
    end

  assign bus_request[0] = (port_s0_hold_active | port_s0_trans_valid) & port_enable_mask[0];
  assign bus_request[1] = (port_s1_hold_active | port_s1_trans_valid) & port_enable_mask[1];
  assign bus_request[2] = (port_s2_hold_active | port_s2_trans_valid) & port_enable_mask[2];

   always @(mux_sel_addr_phase_reg or trans_announced_stalled_reg or
           locked_active_trans_reg or fixed_length_burst or reg_fixed_length_burst_err or
           bus_request or reg_round_robin_state or port_enable_mask)
    begin
      if (trans_announced_stalled_reg | locked_active_trans_reg | fixed_length_burst |
          reg_fixed_length_burst_err)
                mux_sel_addr_phase = mux_sel_addr_phase_reg & port_enable_mask;
      else if (bus_request[2])
        mux_sel_addr_phase = 3'b100 & port_enable_mask;
      else if (bus_request[1:0]==2'b11)
        mux_sel_addr_phase = ({1'b0, ~reg_round_robin_state, reg_round_robin_state}) & port_enable_mask;
      else
        mux_sel_addr_phase = bus_request & port_enable_mask;
    end

  wire [2:0] nxt_mux_sel_data_phase;
  assign     nxt_mux_sel_data_phase = mux_sel_addr_phase & port_enable_mask;

  always @(posedge i_hclk or negedge i_hreset_n)
    begin
      if (~i_hreset_n)
        mux_sel_data_phase <= 3'b000;
      else if (hready_mux)
        mux_sel_data_phase <= nxt_mux_sel_data_phase;
    end

  assign nxt_active_transfer = hsel_mux & htrans_mux[1];

  always @(posedge i_hclk or negedge i_hreset_n)
    begin
      if (~i_hreset_n)
        active_transfer_reg <= 1'b0;
      else if (hready_mux)
        active_transfer_reg <= nxt_active_transfer;
    end

    wire[2:0] ch_rdy;
    assign ch_rdy[0] = (~port_s0_hold_active)&(mux_sel_data_phase[0]);
    assign ch_rdy[1] = (~port_s1_hold_active)&(mux_sel_data_phase[1]);
    assign ch_rdy[2] = (~port_s2_hold_active)&(mux_sel_data_phase[2]);

  assign   hsel_mux   =  ((   mux_sel_addr_phase[0]) & (port_s0_hold_active|i_hsels0)) |
                         ((   mux_sel_addr_phase[1]) & (port_s1_hold_active|i_hsels1)) |
                         ((   mux_sel_addr_phase[2]) & (port_s2_hold_active|i_hsels2)) ;
  assign   haddr_mux  =  ({P_AWIDTH{mux_sel_addr_phase[0]}} & haddr_s0_ips) |
                         ({P_AWIDTH{mux_sel_addr_phase[1]}} & haddr_s1_ips) |
                         ({P_AWIDTH{mux_sel_addr_phase[2]}} & haddr_s2_ips) ;
  assign   htrans_mux =  ({ 2{mux_sel_addr_phase[0]}} & htrans_s0_ips) |
                         ({ 2{mux_sel_addr_phase[1]}} & htrans_s1_ips) |
                         ({ 2{mux_sel_addr_phase[2]}} & htrans_s2_ips) ;
  assign   hwrite_mux =  (    mux_sel_addr_phase[0]   & hwrite_s0_ips) |
                         (    mux_sel_addr_phase[1]   & hwrite_s1_ips) |
                         (    mux_sel_addr_phase[2]   & hwrite_s2_ips) ;
  assign   hsize_mux  =  ({ HSIZE_W{mux_sel_addr_phase[0]}} & hsize_s0_ips) |
                         ({ HSIZE_W{mux_sel_addr_phase[1]}} & hsize_s1_ips) |
                         ({ HSIZE_W{mux_sel_addr_phase[2]}} & hsize_s2_ips) ;
  assign   hprot_mux  =  ({ 4{mux_sel_addr_phase[0]}} & hprot_s0_ips) |
                         ({ 4{mux_sel_addr_phase[1]}} & hprot_s1_ips) |
                         ({ 4{mux_sel_addr_phase[2]}} & hprot_s2_ips) ;
  assign   hburst_mux =  ({ 3{mux_sel_addr_phase[0]}} & hburst_s0_ips) |
                         ({ 3{mux_sel_addr_phase[1]}} & hburst_s1_ips) |
                         ({ 3{mux_sel_addr_phase[2]}} & hburst_s2_ips) ;
  assign   hmastlock_mux=(    mux_sel_addr_phase[0]   & hmastlock_s0_ips) |
                         (    mux_sel_addr_phase[1]   & hmastlock_s1_ips) |
                         (    mux_sel_addr_phase[2]   & hmastlock_s2_ips) ;
  assign   hready_mux =  (active_transfer_reg) ?
                         i_hreadyoutm :
                         (
                          (    mux_sel_addr_phase[0]   & hready_s0_ips) |
                          (    mux_sel_addr_phase[1]   & hready_s1_ips) |
                          (    mux_sel_addr_phase[2]   & hready_s2_ips) |
                          (    mux_sel_addr_phase == 3'b000));

  assign   hwdata_mux =  ({P_DWIDTH{(mux_sel_data_phase[0] & port_enable_mask[0])}} & i_hwdatas0) |
                         ({P_DWIDTH{(mux_sel_data_phase[1] & port_enable_mask[1])}} & i_hwdatas1) |
                         ({P_DWIDTH{(mux_sel_data_phase[2] & port_enable_mask[2])}} & i_hwdatas2) ;

  reg      reg_idle_flag;
  wire     nxt_idle_flag = ((o_htransm==2'b00) | reg_idle_flag) & (~i_hreadyoutm);

  always @(posedge i_hclk or negedge i_hreset_n)
    begin
      if (~i_hreset_n)
        reg_idle_flag <= 1'b0;
      else
        reg_idle_flag <= nxt_idle_flag;
    end

  wire    [1:0]  seq_tran_suppress = ((mux_sel_addr_phase != mux_sel_data_phase)|reg_idle_flag) ? 2'b10 : 2'b11;

  assign   o_hselm      = hsel_mux;
  assign   o_haddrm     = haddr_mux;
  assign   o_htransm    = htrans_mux & seq_tran_suppress;
  assign   o_hwritem    = hwrite_mux;
  assign   o_hreadym    = hready_mux;
  assign   o_hsizem[1:0] = hsize_mux;
  assign   o_hsizem[2]   = (P_DWIDTH > 64) ? hsize_mux[HSIZE_MAX] : 1'b0;
  assign   o_hprotm     = hprot_mux;
  assign   o_hburstm    = hburst_mux;
  assign   o_hmastlockm = hmastlock_mux;
  assign   o_hwdatam    = hwdata_mux;

  assign   o_hmasterm[1]  = mux_sel_addr_phase[2] | (mux_sel_addr_phase==3'b000);
  assign   o_hmasterm[0]  = mux_sel_addr_phase[1] | (mux_sel_addr_phase==3'b000);

endmodule
