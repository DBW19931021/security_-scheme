//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_ahb_downsizer64to32
  #(
  parameter HMASTER_WIDTH   = 1,                
  parameter ERR_BURST_BLOCK_ALL = 1)
  (
  input  wire                      i_hclk,         
  input  wire                      i_hreset_n,     

  input  wire                      i_hsels,        
  input  wire [31:0]               i_haddrs,       
  input  wire [1:0]                i_htranss,      
  input  wire [2:0]                i_hbursts,      
  input  wire [3:0]                i_hprots,        
  input  wire                      i_hwrites,       
  input  wire [2:0]                i_hsizes,       
  input  wire                      i_hmastlocks,   
  input  wire                      i_hreadys,      
  input  wire [63:0]               i_hwdatas,  

  output wire [63:0]               o_hrdatas,      
  output reg                       o_hresps,       
  output reg                       o_hreadyouts,   

  output wire                      o_hselm,        
  output wire [31:0]               o_haddrm,       
  output reg  [1:0]                o_htransm,      
  output reg  [2:0]                o_hburstm,      
  output reg  [3:0]                o_hprotm,       
  output reg                       o_hwritem,      
  output wire [2:0]                o_hsizem,       
  output reg                       o_hmastlockm,   
  output wire [31:0]               o_hwdatam,      
  output wire                      o_hreadym,      

  input  wire [31:0]               i_hrdatam,      
  input  wire                      i_hrespm,       
  input  wire                      i_hreadyoutm

);    

  wire [HMASTER_WIDTH-1:0]  i_hmasters;
  reg  [HMASTER_WIDTH-1:0]  o_hmasterm;

  assign i_hmasters = 1'b0;

  localparam AHB_TRANS_IDLE        = 2'b00;      
  localparam AHB_TRANS_BUSY        = 2'b01;      
  localparam AHB_TRANS_NONSEQ      = 2'b10;      
  localparam AHB_TRANS_SEQ         = 2'b11;      

  localparam AHB_RESP_OKAY         = 1'b0;       
  localparam AHB_RESP_ERROR        = 1'b1;       

  localparam AHB_BURST_SINGLE      = 3'b000;     
  localparam AHB_BURST_INCR        = 3'b001;     
  localparam AHB_BURST_WRAP4       = 3'b010;     
  localparam AHB_BURST_INCR4       = 3'b011;     
  localparam AHB_BURST_WRAP8       = 3'b100;     
  localparam AHB_BURST_INCR8       = 3'b101;     
  localparam AHB_BURST_WRAP16      = 3'b110;     
  localparam AHB_BURST_INCR16      = 3'b111;     

  localparam AHB_SIZE_BYTE         = 3'b000;     
  localparam AHB_SIZE_HALF         = 3'b001;     
  localparam AHB_SIZE_WORD         = 3'b010;     
  localparam AHB_SIZE_DWORD        = 3'b011;     

  localparam BERR_FSM_NORMAL       = 2'b00;      
  localparam BERR_FSM_ERROR1       = 2'b01;      
  localparam BERR_FSM_ERROR2       = 2'b10;      
  localparam BERR_FSM_BUSY         = 2'b11;     

  localparam DS_FSM_IDLE           = 3'b000;     
  localparam DS_FSM_CYC1           = 3'b001;     
  localparam DS_FSM_CYC2           = 3'b010;     
  localparam DS_FSM_DELAY          = 3'b011;     
  localparam DS_FSM_ABORT          = 3'b101;     

  reg [1:0]     berr_current_state;  
  reg [1:0]     berr_next_state;     
  reg           reg_seq_to_nseq;     
  reg           nxt_seq_to_nseq;     

  reg [2:0]     dsc_current_state;   
  reg [2:0]     ds_next_state;       

  reg           hsel_ds_in;          
  reg [1:0]     htrans_ds_in;        
  reg           hready_ds_out;       

  reg           htrans_ds_active;    

  wire          next_wrapped;        
  reg           wrapped;             

  reg           block_disable;       

  wire          next_block_disable;  

  reg [31:0]    haddr_reg;          
  reg [2:0]     haddr_i16_reg;      

  reg           htrans0_reg;        
  reg [2:0]     hsize_reg;          
  reg [2:0]     hburst_reg;         
  reg [3:0]     hprot_reg;          
  reg           hwrite_reg;         
  reg           hmastlock_reg;      
  reg [HMASTER_WIDTH-1:0] hmaster_reg;  

  wire          haddr_i16_reg_en;   
  wire [2:0]    haddr_i16_bound;    

  reg [2:0]     hburst_mapped;      
  reg [2:0]     hburst_mapped_reg;  

  reg [31:0]    i_haddr_m;          
  wire          i_hready_m;         
  reg           i_hsel_m;           

  reg           fwd_mux_ctrl;       
  reg [31:0]    hrdata_reg;         
  reg           frd_mux_ctrl;       
  reg           next_frd_mux_ctrl;  

  wire          s_not_sel_nxt;      
  reg           s_not_sel;          
  wire          m_not_sel_nxt;      
  reg           m_not_sel;          

  always @ (i_htranss or i_hsels or i_hreadys or i_hrespm or i_hreadyoutm or berr_current_state or
            block_disable)
    begin : p_berrcomb
      if ((~i_hsels) & i_hreadys)
        berr_next_state = BERR_FSM_NORMAL;
      else
        begin
          case (berr_current_state)
            BERR_FSM_NORMAL :
              begin
                if (block_disable) 

                  berr_next_state = BERR_FSM_NORMAL;
                else if ((i_hrespm == AHB_RESP_ERROR) & (~i_hreadyoutm))

                  berr_next_state = BERR_FSM_ERROR2;
                else
                  berr_next_state = BERR_FSM_NORMAL;
              end

            BERR_FSM_ERROR1 :
              begin

                berr_next_state = BERR_FSM_ERROR2;
              end

            BERR_FSM_ERROR2, BERR_FSM_BUSY:
              begin 

                if (i_htranss == AHB_TRANS_SEQ)
                  berr_next_state = BERR_FSM_ERROR1;

                else if (i_htranss == AHB_TRANS_BUSY)
                  berr_next_state = BERR_FSM_BUSY;

                else 
                  berr_next_state = BERR_FSM_NORMAL;
              end
            default :
              berr_next_state = {2{1'bx}};
          endcase
        end
    end

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_berrseq
      if (~i_hreset_n)
        berr_current_state <= BERR_FSM_NORMAL;
      else
        berr_current_state <= berr_next_state;
    end

  always @(i_hrespm or i_hreadyoutm or i_htranss or i_hsels or i_hreadys or reg_seq_to_nseq)
    begin
    if ((i_htranss==AHB_TRANS_IDLE)|(i_htranss==AHB_TRANS_NONSEQ)|((~i_hsels) & i_hreadys))
      nxt_seq_to_nseq = 1'b0; 
    else if ((i_hrespm==AHB_RESP_ERROR) & (~i_hreadyoutm) & (ERR_BURST_BLOCK_ALL==1'b0))
      nxt_seq_to_nseq = 1'b1; 
    else
      nxt_seq_to_nseq = reg_seq_to_nseq;     end

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_burst_err_continue
      if (~i_hreset_n)
        reg_seq_to_nseq <= 1'b0;
      else   
        reg_seq_to_nseq <= nxt_seq_to_nseq;
    end

  always @ (berr_current_state or i_htranss or i_hsels)
    begin : p_hstdsin
      if (((berr_current_state == BERR_FSM_ERROR1) |
          ((berr_current_state !=BERR_FSM_NORMAL) & (i_htranss !=AHB_TRANS_NONSEQ))
          ) & i_hsels)
        begin  
          hsel_ds_in = 1'b0;
          htrans_ds_in = AHB_TRANS_IDLE;
        end
      else
        begin  
          hsel_ds_in = i_hsels;

          htrans_ds_in = i_htranss & {2{i_hsels}};
        end
    end

  always @ (berr_current_state or hready_ds_out or i_hrespm)
    begin : p_berr_resp
      case (berr_current_state)

        BERR_FSM_NORMAL :
          begin
            o_hreadyouts = hready_ds_out;
            o_hresps = i_hrespm;
          end

        BERR_FSM_ERROR1 :
          begin

            o_hreadyouts = 1'b0;
            o_hresps = AHB_RESP_ERROR;
          end

        BERR_FSM_ERROR2 :
          begin

            o_hreadyouts = 1'b1;
            o_hresps = AHB_RESP_ERROR;
          end

        BERR_FSM_BUSY:
          begin

            o_hreadyouts = 1'b1;
            o_hresps = AHB_RESP_OKAY;
          end

        default:
          begin
            o_hreadyouts = 1'bx;
            o_hresps = 1'bx;
          end
      endcase
    end

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_store_ctrls
      if (~i_hreset_n)
        begin
          haddr_reg        <= {32{1'b0}};
          htrans0_reg      <= 1'b0;
          hsize_reg        <= {3{1'b0}};
          hburst_reg       <= {3{1'b0}};
          hburst_mapped_reg<= {3{1'b0}};
          hprot_reg        <= {4{1'b0}};
          hwrite_reg       <= 1'b0;
          hmastlock_reg    <= 1'b0;
          hmaster_reg      <= {HMASTER_WIDTH{1'b0}};
        end
      else
        begin
          if (i_hreadys)
            begin
              haddr_reg        <= i_haddrs;
              htrans0_reg      <= i_htranss[0];
              hsize_reg        <= i_hsizes;
              hburst_reg       <= i_hbursts;
              hburst_mapped_reg<= hburst_mapped;
              hprot_reg        <= i_hprots;
              hwrite_reg       <= i_hwrites;
              hmastlock_reg    <= i_hmastlocks;
              hmaster_reg      <= i_hmasters;
            end
        end
    end

  always @ (htrans_ds_in)
    begin : p_trans_active
      if ((htrans_ds_in == AHB_TRANS_NONSEQ) | (htrans_ds_in == AHB_TRANS_SEQ))
        htrans_ds_active = 1'b1;
      else
        htrans_ds_active = 1'b0;
    end

  assign haddr_i16_reg_en = (i_hreadys & hsel_ds_in & (htrans_ds_in == AHB_TRANS_NONSEQ) &
                            (i_hbursts == AHB_BURST_INCR16));

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_store_i16addr
      if (~i_hreset_n)
        haddr_i16_reg <= {3{1'b0}};
      else
        if (haddr_i16_reg_en)
          haddr_i16_reg <= i_haddrs[5:3];
    end

  always @ (dsc_current_state or hsel_ds_in or i_hsizes or hsize_reg or i_hready_m or
            i_hrespm or i_hreadys or htrans_ds_active)
    begin : p_ds_fsm_comb
      case (dsc_current_state)

        DS_FSM_IDLE :
          begin
            if (i_hrespm == AHB_RESP_ERROR)
              begin
                ds_next_state = DS_FSM_ABORT;
              end
            else if (hsel_ds_in & i_hreadys)
              begin  
                if ((i_hsizes == AHB_SIZE_DWORD) & htrans_ds_active)
                  begin
                    ds_next_state = DS_FSM_CYC1;

                  end
                else
                  begin
                    ds_next_state = DS_FSM_IDLE;

                  end
              end
            else
              begin
                ds_next_state = DS_FSM_IDLE;
              end
          end

        DS_FSM_CYC1 :
          begin

            if (i_hrespm == AHB_RESP_ERROR)
              begin
                ds_next_state = DS_FSM_ABORT;

              end
            else if (i_hready_m)
              begin
                ds_next_state = DS_FSM_CYC2;

              end
            else
              begin
                ds_next_state = DS_FSM_CYC1;

              end
          end

        DS_FSM_CYC2 :
          begin

            if (i_hrespm == AHB_RESP_ERROR)
              begin
                ds_next_state = DS_FSM_ABORT;

              end
            else if (i_hready_m)
              begin

                if (hsel_ds_in)
                  begin

                    if ((i_hsizes == AHB_SIZE_DWORD) & htrans_ds_active)
                      begin
                        ds_next_state = DS_FSM_CYC1;

                      end
                    else
                      begin
                        ds_next_state = DS_FSM_IDLE;
                      end
                  end
                else
                  begin
                    ds_next_state = DS_FSM_IDLE;

                  end
              end
            else
              begin
                ds_next_state = DS_FSM_CYC2;

              end
          end

        DS_FSM_ABORT :
          begin

            if (hsel_ds_in & htrans_ds_active)
              ds_next_state = DS_FSM_DELAY;

            else
              begin
                ds_next_state = DS_FSM_IDLE;

              end
          end

        DS_FSM_DELAY :
          begin

            if (hsize_reg == AHB_SIZE_DWORD)
              begin

                ds_next_state = DS_FSM_CYC1;

              end
            else
              begin
                ds_next_state = DS_FSM_IDLE;

              end
          end

        3'b100, 3'b110, 3'b111:
          begin
            ds_next_state = DS_FSM_IDLE;

          end
        default:
          begin
            ds_next_state = 3'bxxx;

          end
      endcase
    end

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_ds_fsm_seq
      if (~i_hreset_n)
        dsc_current_state <= DS_FSM_IDLE;
      else
        dsc_current_state <= ds_next_state;
    end

  assign  next_block_disable = ((i_htranss!=AHB_TRANS_IDLE) &
          ((i_hsizes == AHB_SIZE_DWORD)|(ERR_BURST_BLOCK_ALL!=1'b0))) ? 1'b0 : 1'b1;

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_disable_seq
    if (~i_hreset_n)
      block_disable <= 1'b1;
    else if (i_hreadys)
      block_disable <= next_block_disable;
    end

  always @ (dsc_current_state or hsel_ds_in)
    begin : p_i_hsel_m
      if ((dsc_current_state == DS_FSM_CYC1) | (dsc_current_state == DS_FSM_DELAY))
        begin
          i_hsel_m = 1'b1;
        end
      else
        begin
          i_hsel_m = hsel_ds_in;
        end
    end

  assign o_hselm = i_hsel_m;

   wire[3:0]     temp_add_result = (haddr_i16_reg + 3'b111)+4'b0;
  assign haddr_i16_bound = temp_add_result[2:0];

  assign next_wrapped = (
                        (

                         (htrans_ds_active &
                          (i_haddrs[6:3]== 4'b1111) & (i_hbursts == AHB_BURST_WRAP16)) |

                         ((htrans_ds_in == AHB_TRANS_SEQ) &
                          (i_haddrs[5:3]== haddr_i16_bound) &
                          (i_hbursts == AHB_BURST_INCR16))
                         ) ? 1'b1 :
                        ((htrans_ds_in == AHB_TRANS_BUSY) ? wrapped : 1'b0)
                        );

  always @ (negedge i_hreset_n or posedge i_hclk)
    begin : p_wrapped
      if (~i_hreset_n)
        wrapped <= 1'b0;
      else
        if (i_hreadys)
          wrapped <= next_wrapped;
    end

  always @ (dsc_current_state or htrans_ds_in or
            wrapped or reg_seq_to_nseq or i_hsizes or i_hsels)
    begin : p_htrans
      if (dsc_current_state == DS_FSM_CYC1)
        begin

          o_htransm = AHB_TRANS_NONSEQ;

        end

      else if (dsc_current_state == DS_FSM_ABORT)
        begin  
          o_htransm = AHB_TRANS_IDLE;
        end

      else if (dsc_current_state == DS_FSM_DELAY)
        begin

          o_htransm = AHB_TRANS_NONSEQ;
        end

      else if ((wrapped & (i_hsizes == AHB_SIZE_DWORD) & i_hsels))
        begin

          o_htransm[1] = htrans_ds_in[1];
          o_htransm[0] = 1'b0;
        end

      else
        begin

          o_htransm[1] = htrans_ds_in[1];
          o_htransm[0] = htrans_ds_in[0] & (~reg_seq_to_nseq);
        end
    end

  always @ (dsc_current_state or i_haddrs or haddr_reg)
    begin : p_haddr
      if (dsc_current_state == DS_FSM_DELAY)
          i_haddr_m = haddr_reg;

      else if (dsc_current_state == DS_FSM_CYC1)
          i_haddr_m = {haddr_reg[31:3],1'b1,haddr_reg[1:0]};

      else
          i_haddr_m = i_haddrs;
    end

  assign o_haddrm = i_haddr_m;

  always @ (i_hbursts)
    begin : p_burstmap
      case (i_hbursts)
        AHB_BURST_WRAP4 : begin
          hburst_mapped = AHB_BURST_WRAP8;
        end
        AHB_BURST_INCR4 : begin
          hburst_mapped = AHB_BURST_INCR8;
        end
        AHB_BURST_WRAP8 : begin
          hburst_mapped = AHB_BURST_WRAP16;
        end
        AHB_BURST_INCR8 : begin
          hburst_mapped = AHB_BURST_INCR16;
        end
        AHB_BURST_WRAP16 : begin
          hburst_mapped = AHB_BURST_INCR;
        end
        AHB_BURST_INCR16 : begin
          hburst_mapped = AHB_BURST_INCR16;
        end
        AHB_BURST_INCR : begin
          hburst_mapped = AHB_BURST_INCR;
        end
        default: begin
          hburst_mapped = AHB_BURST_INCR;

        end
      endcase
    end

  always @ (dsc_current_state or hburst_mapped or hburst_reg or i_hbursts or htrans0_reg or
            hburst_mapped_reg or i_hsizes or hsize_reg or i_hsels or reg_seq_to_nseq or htrans_ds_in)
    begin : p_hburstm
      if (reg_seq_to_nseq & (ERR_BURST_BLOCK_ALL==1'b0) & (htrans_ds_in[0]|
          ((dsc_current_state == DS_FSM_DELAY) & htrans0_reg))) 

        begin
        o_hburstm = AHB_BURST_INCR;
        end
      else if (dsc_current_state == DS_FSM_DELAY)
        begin
          if (hsize_reg == AHB_SIZE_DWORD)
            begin
              o_hburstm = hburst_mapped_reg;

            end
          else
            begin
              o_hburstm = hburst_reg;

            end
        end
      else if (dsc_current_state == DS_FSM_CYC1)
        begin

          o_hburstm = hburst_mapped_reg;
        end
      else
        begin
          if ((i_hsizes == AHB_SIZE_DWORD) & i_hsels)
            begin
              o_hburstm = hburst_mapped;

            end
          else
            begin
              o_hburstm = i_hbursts;

            end
        end
    end

  always @ (dsc_current_state or hprot_reg or i_hprots)
    begin : p_hprotm
      if ((dsc_current_state == DS_FSM_CYC1) | (dsc_current_state == DS_FSM_DELAY))
        begin
          o_hprotm = hprot_reg;
        end
      else
        begin
          o_hprotm = i_hprots;
        end
    end

  always @ (dsc_current_state or hwrite_reg or i_hwrites)
    begin : p_hwritem
      if ((dsc_current_state == DS_FSM_CYC1) | (dsc_current_state == DS_FSM_DELAY))
        begin
          o_hwritem = hwrite_reg;
        end
      else
        begin
          o_hwritem = i_hwrites;
        end
    end

  always @ (dsc_current_state or hmastlock_reg or i_hmastlocks)
    begin : p_hmastlockm
      if ((dsc_current_state == DS_FSM_CYC1) | (dsc_current_state == DS_FSM_DELAY))
        begin
          o_hmastlockm = hmastlock_reg;
        end
      else
        begin
          o_hmastlockm = i_hmastlocks;
        end
    end

  reg [2:0] i_hsize_m;
  always @ (dsc_current_state or i_hsizes or hsize_reg)
    begin : p_hsizem
      if ((dsc_current_state == DS_FSM_CYC1) | (dsc_current_state == DS_FSM_DELAY))
        begin
          i_hsize_m = hsize_reg;
        end
      else
        begin
          i_hsize_m = i_hsizes;
        end
    end

  assign o_hsizem = (i_hsize_m == AHB_SIZE_DWORD) ? AHB_SIZE_WORD : i_hsize_m;

  always @ (dsc_current_state or hmaster_reg or i_hmasters)
    begin : p_hmasterm
      if ((dsc_current_state == DS_FSM_CYC1) | (dsc_current_state == DS_FSM_DELAY))
        begin
          o_hmasterm = hmaster_reg;
        end
      else
        begin
          o_hmasterm = i_hmasters;
        end
    end

  assign s_not_sel_nxt = ((i_hsels & i_hreadys) ? 1'b0
                       : (((~i_hsels) & i_hreadys) ? 1'b1
                          : s_not_sel)
                       );

  assign m_not_sel_nxt = ((i_hsel_m & i_hready_m) ? 1'b0
                       : (((~i_hsel_m) & i_hready_m) ? 1'b1
                          : m_not_sel)
                       );

  always@(negedge i_hreset_n or posedge i_hclk)
    begin : p_hreadyseq
      if (~i_hreset_n)
        begin
          s_not_sel <= 1'b1;
          m_not_sel <= 1'b1;
        end
      else
        begin
          s_not_sel <= s_not_sel_nxt;
          m_not_sel <= m_not_sel_nxt;
        end
    end

  always @ (dsc_current_state or i_hreadyoutm or s_not_sel or m_not_sel)
    begin : p_hreadyouts
      if ((dsc_current_state == DS_FSM_DELAY) | (dsc_current_state == DS_FSM_CYC1))
        hready_ds_out = 1'b0;
      else if (s_not_sel | m_not_sel)
        hready_ds_out = 1'b1;
      else
        hready_ds_out = i_hreadyoutm;
    end

  assign i_hready_m = (m_not_sel ? i_hreadys : i_hreadyoutm);

  assign o_hreadym  = i_hready_m;

  always @ (negedge i_hreset_n or posedge i_hclk)
    begin : p_data_muxctrl
      if (~i_hreset_n)
        begin
          fwd_mux_ctrl <= 1'b0;

        end
      else
        begin
          if (i_hready_m)
            begin
              fwd_mux_ctrl <= i_haddr_m[2];
            end
        end
    end

  assign o_hwdatam = (fwd_mux_ctrl ? i_hwdatas[63:32] : i_hwdatas[31:0]);

  always @ (i_hsizes or htrans_ds_in)
    begin : p_readmuxcomb
      if ((i_hsizes == AHB_SIZE_DWORD) &
          ((htrans_ds_in == AHB_TRANS_NONSEQ) | (htrans_ds_in == AHB_TRANS_SEQ))
          )
        begin
          next_frd_mux_ctrl = 1'b1;
        end
      else
        begin
          next_frd_mux_ctrl = 1'b0;
        end
    end

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_readmuxseq
      if (~i_hreset_n)
        begin
          frd_mux_ctrl <= 1'b0;
        end
      else
        begin
          if (i_hreadys)
            begin
              frd_mux_ctrl <= next_frd_mux_ctrl;
            end
        end
    end

  assign o_hrdatas = (frd_mux_ctrl ? {i_hrdatam,hrdata_reg} : {i_hrdatam,i_hrdatam});

  always @ (posedge i_hclk or negedge i_hreset_n)
    begin : p_hrdataseq
      if (~i_hreset_n)
        hrdata_reg <= {32{1'b0}};
      else
        if (i_hready_m)
          hrdata_reg <= i_hrdatam;
    end

endmodule
