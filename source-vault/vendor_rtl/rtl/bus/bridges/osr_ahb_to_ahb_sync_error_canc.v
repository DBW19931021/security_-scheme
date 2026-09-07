//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_ahb_to_ahb_sync_error_canc
  (

  input  wire        i_hclk,       
  input  wire        i_hclkEN,     
  input  wire        i_hresetn,    

  input  wire  [1:0] i_htranss,    

  output reg         o_hreadyouts, 
  output reg         o_hresps,     

  output reg   [1:0] o_htransm,    

  input  wire        i_hreadyoutm,    
  input  wire        i_hresps);    

  localparam TRN_IDLE         = 2'b00;   
  localparam TRN_BUSY         = 2'b01;   
  localparam TRN_NONSEQ       = 2'b10;   
  localparam TRN_SEQ          = 2'b11;   

  localparam RSP_OKAY         = 1'b0;    
  localparam RSP_ERROR        = 1'b1;    

  localparam ST_CONTROL_IDLE  = 1'b0;      
  localparam ST_CONTROL_ERROR = 1'b1;     

  localparam ST_GEN_IDLE      = 2'b01;
  localparam ST_GEN_ERROR1    = 2'b10;
  localparam ST_GEN_ERROR2    = 2'b11;

  reg            next_control_state;
  reg            control_state;

  reg    [1:0]   next_gen_state;
  reg    [1:0]   gen_state;

  wire           burst_transfer;

  wire           gen_hready_outs;
  wire           gen_hresps;

  assign burst_transfer = ((i_htranss ==TRN_BUSY) | (i_htranss ==TRN_SEQ));

  always @ (i_hresps or burst_transfer or control_state or gen_hready_outs)
    begin : p_control_state_comb
      case (control_state)
        ST_CONTROL_IDLE : begin

              next_control_state = ((i_hresps == RSP_ERROR) & burst_transfer)?
                                   ST_CONTROL_ERROR: ST_CONTROL_IDLE;
          end
        ST_CONTROL_ERROR : begin

             next_control_state =   ((burst_transfer == 1'b0) & gen_hready_outs) ?
                                   ST_CONTROL_IDLE : ST_CONTROL_ERROR;
          end
        default:
          next_control_state = 1'bx; 

      endcase
    end

  always @ (i_hresps or i_htranss or gen_state or control_state)
    begin : p_gen_state_comb
      case (gen_state)

        ST_GEN_IDLE : begin
              next_gen_state = (i_hresps == RSP_ERROR) ?
                                ST_GEN_ERROR2 :
                                ((i_htranss == TRN_SEQ)  & (control_state == ST_CONTROL_ERROR))?
                                ST_GEN_ERROR1 :
                                ST_GEN_IDLE;
          end
        ST_GEN_ERROR1 :
              next_gen_state = ST_GEN_ERROR2;      

        ST_GEN_ERROR2 : begin
              next_gen_state =  (i_htranss == TRN_SEQ) ?
                                ST_GEN_ERROR1:  
                                ST_GEN_IDLE; 
            end

        default:
          next_gen_state = 2'bxx;        

      endcase
    end

  always @ (posedge i_hclk or negedge i_hresetn)
    begin : p_state_seq
      if  (~i_hresetn )
        begin
          control_state <= ST_CONTROL_IDLE;
          gen_state     <= ST_GEN_IDLE;
        end
      else if (i_hclkEN)
        begin
          control_state <= next_control_state;
          gen_state     <= next_gen_state;
        end
    end

  assign gen_hready_outs = ((gen_state == ST_GEN_IDLE) |
                          (gen_state == ST_GEN_ERROR2));

  assign gen_hresps = ((gen_state == ST_GEN_ERROR1) |
                      (gen_state == ST_GEN_ERROR2)) ? RSP_ERROR :
                      RSP_OKAY;

  always @ (control_state or i_hreadyoutm or i_hresps or gen_hready_outs or gen_hresps)
    begin : p_ahb1_output_comb
      if  (control_state == ST_CONTROL_IDLE) 
        begin
          o_hreadyouts = i_hreadyoutm;
          o_hresps     = i_hresps;
        end
      else                                  
        begin
          o_hreadyouts = gen_hready_outs;
          o_hresps     = gen_hresps;
        end
    end 

  always @ (i_htranss or control_state)
    begin : p_ahb2_output_comb
      if  (control_state ==ST_CONTROL_IDLE)   
        o_htransm = i_htranss;
      else                       
        if (i_htranss == TRN_NONSEQ)
          o_htransm = i_htranss;
        else
          o_htransm = TRN_IDLE;
    end

endmodule 
