//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_ahb2apb(
  input  wire        hclk       ,   
  input  wire        hresetn    ,   

  input  wire [31:0] i_haddr    ,   
  input  wire [1:0]  i_htrans   ,   
  input  wire        i_hwrite   ,   
  input  wire [31:0] i_hwdata   ,   
  input  wire        i_hsel     ,   
  input  wire        i_hready   ,   

  input  wire [31:0] i_prdatas0 ,   
  input  wire [31:0] i_prdatas1 ,   
  input  wire [31:0] i_prdatas2 ,   
  input  wire [31:0] i_prdatas3 ,   
  input  wire [31:0] i_prdatas4 ,   
  input  wire [31:0] i_prdatas5 ,   
  input  wire [31:0] i_prdatas6 ,   
  input  wire [31:0] i_prdatas7 ,   
  input  wire [31:0] i_prdatas8 ,   
  input  wire [31:0] i_prdatas9 ,   
  input  wire [31:0] i_prdatas10,   
  input  wire [31:0] i_prdatas11,   
  input  wire [31:0] i_prdatas12,   
  input  wire [31:0] i_prdatas13,   
  input  wire [31:0] i_prdatas14,   
  input  wire [31:0] i_prdatas15,   

  input  wire        i_pready0  ,   
  input  wire        i_pready1  ,   
  input  wire        i_pready2  ,   
  input  wire        i_pready3  ,   
  input  wire        i_pready4  ,   
  input  wire        i_pready5  ,   
  input  wire        i_pready6  ,   
  input  wire        i_pready7  ,   
  input  wire        i_pready8  ,   
  input  wire        i_pready9  ,   
  input  wire        i_pready10 ,   
  input  wire        i_pready11 ,   
  input  wire        i_pready12 ,   
  input  wire        i_pready13 ,   
  input  wire        i_pready14 ,   
  input  wire        i_pready15 ,   

  input  wire        i_pclk_phase,

  output wire [31:0] o_hrdata   ,   
  output wire        o_hreadyout,   
  output wire [1:0]  o_hresp    ,   

  output wire [31:0] o_pwdata   ,     
  output wire        o_penable  ,     
  output wire        o_psels0   ,     
  output wire        o_psels1   ,     
  output wire        o_psels2   ,     
  output wire        o_psels3   ,     
  output wire        o_psels4   ,     
  output wire        o_psels5   ,     
  output wire        o_psels6   ,     
  output wire        o_psels7   ,     
  output wire        o_psels8   ,     
  output wire        o_psels9   ,     
  output wire        o_psels10  ,     
  output wire        o_psels11  ,     
  output wire        o_psels12  ,     
  output wire        o_psels13  ,     
  output wire        o_psels14  ,     
  output wire        o_psels15  ,     
  output reg  [26:0] o_paddr    ,     
  output reg         o_pwrite         

); 

  localparam ST_IDLE    = 3'b000,
             ST_RWAIT   = 3'b001,
             ST_READ    = 3'b010,
             ST_RENABLE = 3'b011,
             ST_WWAIT   = 3'b100,
             ST_WRITE   = 3'b101,
             ST_WENABLE = 3'b110;

  localparam S0BASE  = 4'b0000,
             S1BASE  = 4'b0001,
             S2BASE  = 4'b0010,
             S3BASE  = 4'b0011,
             S4BASE  = 4'b0100,
             S5BASE  = 4'b0101,
             S6BASE  = 4'b0110,
             S7BASE  = 4'b0111,
             S8BASE  = 4'b1000,
             S9BASE  = 4'b1001,
             S10BASE = 4'b1010,
             S11BASE = 4'b1011,
             S12BASE = 4'b1100,
             S13BASE = 4'b1101,
             S14BASE = 4'b1110,
             S15BASE = 4'b1111;

  localparam PSEL_S0  = 4'b0000,
             PSEL_S1  = 4'b0001,
             PSEL_S2  = 4'b0010,
             PSEL_S3  = 4'b0011,
             PSEL_S4  = 4'b0100,
             PSEL_S5  = 4'b0101,
             PSEL_S6  = 4'b0110,
             PSEL_S7  = 4'b0111,
             PSEL_S8  = 4'b1000,
             PSEL_S9  = 4'b1001,
             PSEL_S10 = 4'b1010,
             PSEL_S11 = 4'b1011,
             PSEL_S12 = 4'b1100,
             PSEL_S13 = 4'b1101,
             PSEL_S14 = 4'b1110,
             PSEL_S15 = 4'b1111;

  localparam APB0_PADDR0_START  = 4'h0,
             APB0_PADDR0_END    = 4'h0;
  localparam APB0_PADDR1_START  = 4'h1,
             APB0_PADDR1_END    = 4'h1;
  localparam APB0_PADDR2_START  = 4'h2,
             APB0_PADDR2_END    = 4'h2;
  localparam APB0_PADDR3_START  = 4'h3,
             APB0_PADDR3_END    = 4'h3;
  localparam APB0_PADDR4_START  = 4'h4,
             APB0_PADDR4_END    = 4'h4;
  localparam APB0_PADDR5_START  = 4'h5,
             APB0_PADDR5_END    = 4'h5;
  localparam APB0_PADDR6_START  = 4'h6,
             APB0_PADDR6_END    = 4'h6;
  localparam APB0_PADDR7_START  = 4'h7,
             APB0_PADDR7_END    = 4'h7;
  localparam APB0_PADDR8_START  = 4'h8,
             APB0_PADDR8_END    = 4'h8;
  localparam APB0_PADDR9_START  = 4'h9,
             APB0_PADDR9_END    = 4'h9;
  localparam APB0_PADDR10_START = 4'hA,
             APB0_PADDR10_END   = 4'hA;
  localparam APB0_PADDR11_START = 4'hB,
             APB0_PADDR11_END   = 4'hB;
  localparam APB0_PADDR12_START = 4'hC,
             APB0_PADDR12_END   = 4'hC;
  localparam APB0_PADDR13_START = 4'hD,
             APB0_PADDR13_END   = 4'hD;
  localparam APB0_PADDR14_START = 4'hE,
             APB0_PADDR14_END   = 4'hE;
  localparam APB0_PADDR15_START = 4'hF,
             APB0_PADDR15_END   = 4'hF;

  reg  [31:0] IntPWDATA;        
  wire        IntPENABLE;

  wire        Valid;         

  reg   [2:0] NextState;     
  reg   [2:0] CurrentState;

  reg         iHREADYOUT;    

  reg         piped_hwrite;
  reg         pipeline;
  reg  [31:0] saved_hwdata;
  reg         use_saved;
  reg         use_saved_data;
  reg  [31:0] piped_haddr;
  reg  [31:0] saved_haddr;

  reg  [31:0] PRDATA;

  wire [3:0] PselBus;  

  reg         PselS0Int_haddr;
  reg         PselS1Int_haddr;
  reg         PselS2Int_haddr;
  reg         PselS3Int_haddr;
  reg         PselS4Int_haddr;
  reg         PselS5Int_haddr;
  reg         PselS6Int_haddr;
  reg         PselS7Int_haddr;
  reg         PselS8Int_haddr;
  reg         PselS9Int_haddr;
  reg         PselS10Int_haddr;
  reg         PselS11Int_haddr;
  reg         PselS12Int_haddr;
  reg         PselS13Int_haddr;
  reg         PselS14Int_haddr;
  reg         PselS15Int_haddr;

  reg         PselS0Int_piped_haddr;
  reg         PselS1Int_piped_haddr;
  reg         PselS2Int_piped_haddr;
  reg         PselS3Int_piped_haddr;
  reg         PselS4Int_piped_haddr;
  reg         PselS5Int_piped_haddr;
  reg         PselS6Int_piped_haddr;
  reg         PselS7Int_piped_haddr;
  reg         PselS8Int_piped_haddr;
  reg         PselS9Int_piped_haddr;
  reg         PselS10Int_piped_haddr;
  reg         PselS11Int_piped_haddr;
  reg         PselS12Int_piped_haddr;
  reg         PselS13Int_piped_haddr;
  reg         PselS14Int_piped_haddr;
  reg         PselS15Int_piped_haddr;

  reg         PselS0Int_saved_haddr;
  reg         PselS1Int_saved_haddr;
  reg         PselS2Int_saved_haddr;
  reg         PselS3Int_saved_haddr;
  reg         PselS4Int_saved_haddr;
  reg         PselS5Int_saved_haddr;
  reg         PselS6Int_saved_haddr;
  reg         PselS7Int_saved_haddr;
  reg         PselS8Int_saved_haddr;
  reg         PselS9Int_saved_haddr;
  reg         PselS10Int_saved_haddr;
  reg         PselS11Int_saved_haddr;
  reg         PselS12Int_saved_haddr;
  reg         PselS13Int_saved_haddr;
  reg         PselS14Int_saved_haddr;
  reg         PselS15Int_saved_haddr;

  reg [3:0]  haddr_sel;
  reg [3:0]  piped_haddr_sel;
  reg [3:0]  saved_haddr_sel;

  reg         iPSELS0;       
  reg         iPSELS1;
  reg         iPSELS2;
  reg         iPSELS3;
  reg         iPSELS4;
  reg         iPSELS5;
  reg         iPSELS6;
  reg         iPSELS7;
  reg         iPSELS8;
  reg         iPSELS9;
  reg         iPSELS10;
  reg         iPSELS11;
  reg         iPSELS12;
  reg         iPSELS13;
  reg         iPSELS14;
  reg         iPSELS15;

  wire pready  =  ( iPSELS0   & i_pready0  ) |
                  ( iPSELS1   & i_pready1  ) |
                  ( iPSELS2   & i_pready2  ) |
                  ( iPSELS3   & i_pready3  ) |
                  ( iPSELS4   & i_pready4  ) |
                  ( iPSELS5   & i_pready5  ) |
                  ( iPSELS6   & i_pready6  ) |
                  ( iPSELS7   & i_pready7  ) |
                  ( iPSELS8   & i_pready8  ) |
                  ( iPSELS9   & i_pready9  ) |
                  ( iPSELS10  & i_pready10 ) |
                  ( iPSELS11  & i_pready11 ) |
                  ( iPSELS12  & i_pready12 ) |
                  ( iPSELS13  & i_pready13 ) |
                  ( iPSELS14  & i_pready14 ) |
                  ( iPSELS15  & i_pready15 );

  assign Valid = ((i_hsel == 1'b1 && i_hready == 1'b1 && 
                  i_htrans[1]) ? 1'b1 : 1'b0);

  always @ (CurrentState or Valid or i_hwrite or  
            i_pclk_phase or pipeline or piped_hwrite or pready
           )
    begin
      case (CurrentState)

          ST_IDLE:
            begin            
              if (Valid == 1'b1)
                begin
                  if (i_hwrite == 1'b0)
                    begin
                      if (i_pclk_phase == 1'b1)
                        NextState = ST_READ;
                      else
                        NextState = ST_RWAIT;
                    end
                  else
                    begin
                      NextState = ST_WWAIT;
                    end
                end
              else 
                 begin
                   NextState = ST_IDLE;
                 end                 
            end

          ST_WWAIT:
            begin
              if (i_pclk_phase == 1'b1)
                NextState = ST_WRITE;
              else
                NextState = ST_WWAIT;
            end

          ST_WRITE : 
            begin
              if (i_pclk_phase == 1'b1)
                NextState = ST_WENABLE;
              else
                NextState = ST_WRITE;
            end

          ST_RENABLE, 
          ST_WENABLE:
            begin
              if (pipeline == 1'b1)
                begin
                  if (piped_hwrite == 1'b1)
                    begin
                      if ((i_pclk_phase == 1'b1)&(pready == 1'b1))
                        NextState = ST_WRITE;
                     else
                       begin
                         if(CurrentState==ST_RENABLE)
                           NextState = ST_RENABLE;
                         else
                           NextState = ST_WENABLE;
                       end
                    end
                  else 
                    begin
                      if ((i_pclk_phase == 1'b1)&(pready == 1'b1))
                        NextState = ST_READ;
                      else
                        begin
                          if(CurrentState==ST_RENABLE)
                            NextState = ST_RENABLE;
                          else
                            NextState = ST_WENABLE;
                        end
                    end                  
                end
              else
                begin
                  if (Valid == 1'b1)
                    begin
                      if (i_hwrite == 1'b0)
                        begin
                          if ((i_pclk_phase == 1'b1)&(pready == 1'b1))
                            NextState = ST_READ;
                          else
                            begin
                              if(CurrentState==ST_RENABLE)
                                NextState = ST_RENABLE;
                              else
                                NextState = ST_WENABLE;
                            end
                        end
                      else

                        begin
                          if ((i_pclk_phase == 1'b1)&(pready == 1'b1))
                            NextState = ST_WWAIT;
                          else
                            begin
                              if(CurrentState==ST_RENABLE)
                                NextState = ST_RENABLE;
                              else
                                NextState = ST_WENABLE;
                            end                            
                        end
                    end
                  else
                    begin

                      if ((i_pclk_phase == 1'b1)&(pready == 1'b1))
                        NextState = ST_IDLE;
                      else
                        begin
                          if(CurrentState==ST_RENABLE)
                            NextState = ST_RENABLE;
                          else
                            NextState = ST_WENABLE;
                        end                      
                    end
                end
            end

          ST_RWAIT:
            begin
              if (i_pclk_phase == 1'b1)
                NextState = ST_READ;
              else
                NextState = ST_RWAIT;
            end

          ST_READ:
            begin
              if (i_pclk_phase == 1'b1)
                NextState = ST_RENABLE;
              else
                NextState = ST_READ;
            end

          default: 
            begin
              NextState = ST_IDLE;
            end
      endcase
    end 

  always @ (negedge hresetn or posedge hclk)
    begin : p_CurrentStateSeq
      if ((!hresetn))
        CurrentState <= ST_IDLE;
      else
        CurrentState <= NextState;
    end 

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        pipeline <= 1'b0;
      else
        begin
          if (i_pclk_phase == 1'b0)
            begin
              case (CurrentState)
                ST_WWAIT, 
                ST_WRITE:
                  begin
                    if (Valid == 1'b1)
                      begin
                        pipeline <= 1'b1;
                      end                    
                  end

                ST_WENABLE:
                   begin
                     if (Valid == 1'b1)
                        pipeline <= 1'b1;
                   end

                ST_RENABLE:
                   begin
                     if (Valid == 1'b1)
                       pipeline <= 1'b1;
                   end            

                ST_IDLE: 
                  begin
                    pipeline <= 1'b0;
                  end
                default:
                  begin
                    pipeline <= pipeline;
                  end
              endcase
            end 
          else
            begin
              case (CurrentState)
                ST_WWAIT, 
                ST_WRITE: 
                  begin
                    if (Valid == 1'b1)
                      pipeline <= 1'b1;
                  end
                ST_WENABLE:
                  begin
                    if (Valid == 1'b0)
                      begin
                        pipeline <= 1'b0;
                      end
                    else
                      begin
                        if ((piped_hwrite == 1'b0))
                          pipeline <= 1'b0;
                      end
                  end

                ST_RENABLE:
                   begin
                     pipeline <= 1'b0;                     
                   end

                ST_RWAIT,
                ST_IDLE: 
                  begin
                    pipeline <= 1'b0;
                  end
                default:
                  begin
                    pipeline <= pipeline;
                  end
              endcase
          end          
        end
    end

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        begin
          piped_haddr <= {32{1'b0}};
        end
      else
        begin
          case (CurrentState)
            ST_WWAIT,
            ST_WRITE, 
            ST_WENABLE: 
              begin
                if (Valid == 1'b1)
                  piped_haddr <= i_haddr;
              end
            default:
              begin
                piped_haddr <= piped_haddr;
              end
          endcase
        end      
    end

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        begin
          saved_haddr <= {32{1'b0}};
        end
      else
        begin
          case (NextState)
            ST_WWAIT: 
              begin
                if ((CurrentState == ST_IDLE) || (CurrentState == ST_RENABLE))
                   saved_haddr <= i_haddr;
                else
                  begin
                   if (CurrentState == ST_WENABLE)

                     if (pipeline == 1'b0)
                       saved_haddr <= i_haddr;
                  end
              end
            ST_RWAIT:
              begin

                if (CurrentState == ST_IDLE)
                  saved_haddr <= i_haddr;                
              end

            ST_RENABLE:
               begin
                 if (CurrentState == ST_RENABLE && Valid == 1'b1)
                   saved_haddr <= i_haddr;
               end

            default : saved_haddr <= saved_haddr;
          endcase          
        end
    end

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        begin
          piped_hwrite <= 1'b0;
        end
      else
        begin
          case (CurrentState)
            ST_WWAIT, 
            ST_WRITE, 
            ST_WENABLE: 
              begin
                if (Valid == 1'b1)
                  piped_hwrite <= i_hwrite;
              end

            ST_RENABLE:
               begin
                 if (Valid == 1'b1)
                   piped_hwrite <= i_hwrite;
               end

            default:
              begin
                piped_hwrite <= piped_hwrite;
              end
          endcase
        end
    end

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        begin
          saved_hwdata <= {32{1'b0}};
        end
      else
        begin        
          if (use_saved_data == 1'b1) 
            begin
              saved_hwdata <= saved_hwdata;
            end
          else
            begin
              case (NextState)
                ST_WWAIT:
                  begin

                    if (CurrentState == ST_WWAIT)
                      saved_hwdata <= i_hwdata;
                  end
                ST_WENABLE:
                  begin
                    if ((CurrentState == ST_WRITE) && (pipeline == 1'b1))
                      saved_hwdata <= i_hwdata;
                  end      
                default:
                  begin
                    saved_hwdata <= saved_hwdata;
                  end
              endcase
            end
        end
    end

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        begin
          use_saved <= 1'b0;  
        end
      else
        begin
          case (NextState)
            ST_WWAIT: 
              begin
                if ((CurrentState == ST_IDLE) || (CurrentState == ST_WENABLE) || (CurrentState == ST_RENABLE))
                  use_saved <= 1'b1;
              end       
            ST_WRITE: 
              begin
                if (CurrentState == ST_WWAIT)
                  use_saved <= 1'b0;

                else if (CurrentState == ST_RENABLE)
                  use_saved <= 1'b0;

              end

            ST_RENABLE:
               begin
                 if (CurrentState == ST_RENABLE && Valid == 1'b1)
                   use_saved <= 1'b1;
               end

            ST_READ:
               begin
                 if (CurrentState == ST_RENABLE)
                   use_saved <= 1'b0;
               end

            default:
              begin
                use_saved <= use_saved;
              end
          endcase          
        end
    end

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        begin
          use_saved_data <= 1'b0;
        end
      else
        begin
          if ((use_saved == 1'b1) && (i_pclk_phase == 0))
            use_saved_data <= 1'b1;
          else
            use_saved_data <= 1'b0;          
        end
    end   

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        o_paddr <= {27{1'b0}};
      else
        begin
          if (i_pclk_phase)
            begin
              case (NextState)
                ST_WRITE:
                  begin
                    if (use_saved == 1'b1)
                      o_paddr    <=  saved_haddr[26:0];
                    else
                      o_paddr    <=  piped_haddr[26:0];
                  end
                ST_READ:
                  begin
                    case(CurrentState)
                      ST_RWAIT:
                        begin
                          if (pipeline == 1'b0)
                            o_paddr <=  saved_haddr[26:0];
                        end
                      ST_WENABLE:
                        begin
                          if (Valid == 1'b1)
                            o_paddr <=  i_haddr[26:0];
                          else
                            o_paddr <=  piped_haddr[26:0];                          
                        end

                      ST_RENABLE:
                         begin
                           if (use_saved == 1'b1)
                             o_paddr <=  saved_haddr[26:0];

                           else
                             o_paddr <=  i_haddr[26:0];

                         end

                      default:
                        o_paddr <=  i_haddr[26:0];
                    endcase
                  end
                default:
                  begin
                    o_paddr    <=  o_paddr;
                  end                
              endcase
            end
        end 
    end

    assign IntPENABLE = (CurrentState == ST_RENABLE) || (CurrentState == ST_WENABLE);

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        o_pwrite <= 1'b0;
      else
        begin
          if (i_pclk_phase)
            begin
              case (NextState)
                ST_WRITE: o_pwrite <=  1'b1;
                ST_READ : o_pwrite <=  1'b0;
                default : o_pwrite <=  o_pwrite;
              endcase
            end
        end 
    end 

wire [31:0] IntPWDATA_next;
  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        IntPWDATA <= {32{1'b0}};
      else
        begin

            IntPWDATA <= IntPWDATA_next;
        end
    end 
assign IntPWDATA_next = (CurrentState == ST_WRITE) ? i_hwdata : IntPWDATA;

  always @ (*)
    begin
      if (i_haddr[23:20] >= (APB0_PADDR0_START) && i_haddr[23:20] <= (APB0_PADDR0_END))
        haddr_sel = S0BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR1_START) && i_haddr[23:20] <= (APB0_PADDR1_END))
        haddr_sel = S1BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR2_START) && i_haddr[23:20] <= (APB0_PADDR2_END))
        haddr_sel = S2BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR3_START) && i_haddr[23:20] <= (APB0_PADDR3_END))
        haddr_sel = S3BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR4_START) && i_haddr[23:20] <= (APB0_PADDR4_END))
        haddr_sel = S4BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR5_START) && i_haddr[23:20] <= (APB0_PADDR5_END))
        haddr_sel = S5BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR6_START) && i_haddr[23:20] <= (APB0_PADDR6_END))
        haddr_sel = S6BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR7_START) && i_haddr[23:20] <= (APB0_PADDR7_END))
        haddr_sel = S7BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR8_START) && i_haddr[23:20] <= (APB0_PADDR8_END))
        haddr_sel = S8BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR9_START) && i_haddr[23:20] <= (APB0_PADDR9_END))
        haddr_sel = S9BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR10_START) && i_haddr[23:20] <= (APB0_PADDR10_END))
        haddr_sel = S10BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR11_START) && i_haddr[23:20] <= (APB0_PADDR11_END))
        haddr_sel = S11BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR12_START) && i_haddr[23:20] <= (APB0_PADDR12_END))
        haddr_sel = S12BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR13_START) && i_haddr[23:20] <= (APB0_PADDR13_END))
        haddr_sel = S13BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR14_START) && i_haddr[23:20] <= (APB0_PADDR14_END))
        haddr_sel = S14BASE;
      else if (i_haddr[23:20] >= (APB0_PADDR15_START) && i_haddr[23:20] <= (APB0_PADDR15_END))
        haddr_sel = S15BASE;
      else
        haddr_sel = S0BASE;
    end
  always @ (haddr_sel)
    begin

      PselS0Int_haddr = 1'b0;
      PselS1Int_haddr = 1'b0;
      PselS2Int_haddr = 1'b0; 
      PselS3Int_haddr = 1'b0; 
      PselS4Int_haddr = 1'b0;
      PselS5Int_haddr = 1'b0;
      PselS6Int_haddr = 1'b0;
      PselS7Int_haddr = 1'b0;
      PselS8Int_haddr = 1'b0;
      PselS9Int_haddr = 1'b0;
      PselS10Int_haddr = 1'b0;
      PselS11Int_haddr = 1'b0;
      PselS12Int_haddr = 1'b0;
      PselS13Int_haddr = 1'b0;
      PselS14Int_haddr = 1'b0;
      PselS15Int_haddr = 1'b0;
      case (haddr_sel)
        S0BASE :
          PselS0Int_haddr = 1'b1;

        S1BASE :
           PselS1Int_haddr = 1'b1;

        S2BASE :
          PselS2Int_haddr = 1'b1;

        S3BASE :
          PselS3Int_haddr = 1'b1;

        S4BASE :
          PselS4Int_haddr = 1'b1;

        S5BASE :
          PselS5Int_haddr = 1'b1;

        S6BASE :
          PselS6Int_haddr = 1'b1;

        S7BASE :
          PselS7Int_haddr = 1'b1;

        S8BASE :
          PselS8Int_haddr = 1'b1;

        S9BASE :
          PselS9Int_haddr = 1'b1;

        S10BASE :
          PselS10Int_haddr = 1'b1;

        S11BASE :
          PselS11Int_haddr = 1'b1;

        S12BASE :
          PselS12Int_haddr = 1'b1;

        S13BASE :
          PselS13Int_haddr = 1'b1;

        S14BASE :
          PselS14Int_haddr = 1'b1;

        S15BASE :
          PselS15Int_haddr = 1'b1;

      endcase
    end

  always @ (*)
    begin
      if (piped_haddr[23:20] >= (APB0_PADDR0_START) && piped_haddr[23:20] <= (APB0_PADDR0_END))
        piped_haddr_sel = S0BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR1_START) && piped_haddr[23:20] <= (APB0_PADDR1_END))
        piped_haddr_sel = S1BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR2_START) && piped_haddr[23:20] <= (APB0_PADDR2_END))
        piped_haddr_sel = S2BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR3_START) && piped_haddr[23:20] <= (APB0_PADDR3_END))
        piped_haddr_sel = S3BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR4_START) && piped_haddr[23:20] <= (APB0_PADDR4_END))
        piped_haddr_sel = S4BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR5_START) && piped_haddr[23:20] <= (APB0_PADDR5_END))
        piped_haddr_sel = S5BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR6_START) && piped_haddr[23:20] <= (APB0_PADDR6_END))
        piped_haddr_sel = S6BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR7_START) && piped_haddr[23:20] <= (APB0_PADDR7_END))
        piped_haddr_sel = S7BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR8_START) && piped_haddr[23:20] <= (APB0_PADDR8_END))
        piped_haddr_sel = S8BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR9_START) && piped_haddr[23:20] <= (APB0_PADDR9_END))
        piped_haddr_sel = S9BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR10_START) && piped_haddr[23:20] <= (APB0_PADDR10_END))
        piped_haddr_sel = S10BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR11_START) && piped_haddr[23:20] <= (APB0_PADDR11_END))
        piped_haddr_sel = S11BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR12_START) && piped_haddr[23:20] <= (APB0_PADDR12_END))
        piped_haddr_sel = S12BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR13_START) && piped_haddr[23:20] <= (APB0_PADDR13_END))
        piped_haddr_sel = S13BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR14_START) && piped_haddr[23:20] <= (APB0_PADDR14_END))
        piped_haddr_sel = S14BASE;
      else if (piped_haddr[23:20] >= (APB0_PADDR15_START) && piped_haddr[23:20] <= (APB0_PADDR15_END))
        piped_haddr_sel = S15BASE;
      else
        piped_haddr_sel = S0BASE;
    end
  always @ (piped_haddr_sel)
    begin

      PselS0Int_piped_haddr = 1'b0;
      PselS1Int_piped_haddr = 1'b0;
      PselS2Int_piped_haddr = 1'b0;
      PselS3Int_piped_haddr = 1'b0;
      PselS4Int_piped_haddr = 1'b0;
      PselS5Int_piped_haddr = 1'b0;
      PselS6Int_piped_haddr = 1'b0;
      PselS7Int_piped_haddr = 1'b0;
      PselS8Int_piped_haddr = 1'b0;
      PselS9Int_piped_haddr = 1'b0;
      PselS10Int_piped_haddr = 1'b0;
      PselS11Int_piped_haddr = 1'b0;
      PselS12Int_piped_haddr = 1'b0;
      PselS13Int_piped_haddr = 1'b0;
      PselS14Int_piped_haddr = 1'b0;
      PselS15Int_piped_haddr = 1'b0;

      case (piped_haddr_sel)
        S0BASE :
          PselS0Int_piped_haddr = 1'b1;

        S1BASE :
           PselS1Int_piped_haddr = 1'b1;

        S2BASE :
          PselS2Int_piped_haddr = 1'b1;

        S3BASE :
          PselS3Int_piped_haddr = 1'b1;

        S4BASE :
          PselS4Int_piped_haddr = 1'b1;

        S5BASE :
          PselS5Int_piped_haddr = 1'b1;

        S6BASE :
          PselS6Int_piped_haddr = 1'b1;

        S7BASE :
          PselS7Int_piped_haddr = 1'b1;

        S8BASE :
          PselS8Int_piped_haddr = 1'b1;

        S9BASE :
          PselS9Int_piped_haddr = 1'b1;

        S10BASE :
          PselS10Int_piped_haddr = 1'b1;

        S11BASE :
          PselS11Int_piped_haddr = 1'b1;

        S12BASE :
          PselS12Int_piped_haddr = 1'b1;

        S13BASE :
          PselS13Int_piped_haddr = 1'b1;

        S14BASE :
          PselS14Int_piped_haddr = 1'b1;

        S15BASE :
          PselS15Int_piped_haddr = 1'b1;

      endcase
    end

  always @ (*)
    begin
      if (saved_haddr[23:20] >= (APB0_PADDR0_START) && saved_haddr[23:20] <= (APB0_PADDR0_END))
        saved_haddr_sel = S0BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR1_START) && saved_haddr[23:20] <= (APB0_PADDR1_END))
        saved_haddr_sel = S1BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR2_START) && saved_haddr[23:20] <= (APB0_PADDR2_END))
        saved_haddr_sel = S2BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR3_START) && saved_haddr[23:20] <= (APB0_PADDR3_END))
        saved_haddr_sel = S3BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR4_START) && saved_haddr[23:20] <= (APB0_PADDR4_END))
        saved_haddr_sel = S4BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR5_START) && saved_haddr[23:20] <= (APB0_PADDR5_END))
        saved_haddr_sel = S5BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR6_START) && saved_haddr[23:20] <= (APB0_PADDR6_END))
        saved_haddr_sel = S6BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR7_START) && saved_haddr[23:20] <= (APB0_PADDR7_END))
        saved_haddr_sel = S7BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR8_START) && saved_haddr[23:20] <= (APB0_PADDR8_END))
        saved_haddr_sel = S8BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR9_START) && saved_haddr[23:20] <= (APB0_PADDR9_END))
        saved_haddr_sel = S9BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR10_START) && saved_haddr[23:20] <= (APB0_PADDR10_END))
        saved_haddr_sel = S10BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR11_START) && saved_haddr[23:20] <= (APB0_PADDR11_END))
        saved_haddr_sel = S11BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR12_START) && saved_haddr[23:20] <= (APB0_PADDR12_END))
        saved_haddr_sel = S12BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR13_START) && saved_haddr[23:20] <= (APB0_PADDR13_END))
        saved_haddr_sel = S13BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR14_START) && saved_haddr[23:20] <= (APB0_PADDR14_END))
        saved_haddr_sel = S14BASE;
      else if (saved_haddr[23:20] >= (APB0_PADDR15_START) && saved_haddr[23:20] <= (APB0_PADDR15_END))
        saved_haddr_sel = S15BASE;
      else
        saved_haddr_sel = S0BASE;
    end
  always @ (saved_haddr_sel)
    begin

      PselS0Int_saved_haddr = 1'b0;
      PselS1Int_saved_haddr = 1'b0;
      PselS2Int_saved_haddr = 1'b0;
      PselS3Int_saved_haddr = 1'b0;
      PselS4Int_saved_haddr = 1'b0;
      PselS5Int_saved_haddr = 1'b0;
      PselS6Int_saved_haddr = 1'b0;
      PselS7Int_saved_haddr = 1'b0;
      PselS8Int_saved_haddr = 1'b0;
      PselS9Int_saved_haddr = 1'b0;
      PselS10Int_saved_haddr = 1'b0;
      PselS11Int_saved_haddr = 1'b0;
      PselS12Int_saved_haddr = 1'b0;
      PselS13Int_saved_haddr = 1'b0;
      PselS14Int_saved_haddr = 1'b0;
      PselS15Int_saved_haddr = 1'b0;

      case (saved_haddr_sel)
        S0BASE :
          PselS0Int_saved_haddr = 1'b1;

        S1BASE :
           PselS1Int_saved_haddr = 1'b1;

        S2BASE :
          PselS2Int_saved_haddr = 1'b1;

        S3BASE :
          PselS3Int_saved_haddr = 1'b1;

        S4BASE :
          PselS4Int_saved_haddr = 1'b1;

        S5BASE :
          PselS5Int_saved_haddr = 1'b1;

        S6BASE :
          PselS6Int_saved_haddr = 1'b1;

        S7BASE :
          PselS7Int_saved_haddr = 1'b1;

        S8BASE :
          PselS8Int_saved_haddr = 1'b1;

        S9BASE :
          PselS9Int_saved_haddr = 1'b1;

        S10BASE :
          PselS10Int_saved_haddr = 1'b1;

        S11BASE :
          PselS11Int_saved_haddr = 1'b1;

        S12BASE :
          PselS12Int_saved_haddr = 1'b1;

        S13BASE :
          PselS13Int_saved_haddr = 1'b1;

        S14BASE :
          PselS14Int_saved_haddr = 1'b1;

        S15BASE :
          PselS15Int_saved_haddr = 1'b1;

      endcase
    end

  always @ (negedge hresetn or posedge hclk)
    begin
      if ((!hresetn))
        begin
          iPSELS0  <= 1'b0;
          iPSELS1  <= 1'b0;
          iPSELS2  <= 1'b0;
          iPSELS3  <= 1'b0;
          iPSELS4  <= 1'b0;
          iPSELS5  <= 1'b0;
          iPSELS6  <= 1'b0;
          iPSELS7  <= 1'b0;
          iPSELS8  <= 1'b0;
          iPSELS9  <= 1'b0;
          iPSELS10 <= 1'b0;
          iPSELS11 <= 1'b0;
          iPSELS12 <= 1'b0;
          iPSELS13 <= 1'b0;
          iPSELS14 <= 1'b0;
          iPSELS15 <= 1'b0;
        end
      else
        begin
          if (i_pclk_phase)
            begin
              case (NextState)
                ST_WRITE:
                  begin
                    if (use_saved == 1'b1)
                      begin
                      iPSELS0    <= PselS0Int_saved_haddr;
                      iPSELS1    <= PselS1Int_saved_haddr;
                      iPSELS2    <= PselS2Int_saved_haddr;
                      iPSELS3    <= PselS3Int_saved_haddr;
                      iPSELS4    <= PselS4Int_saved_haddr;
                      iPSELS5    <= PselS5Int_saved_haddr;
                      iPSELS6    <= PselS6Int_saved_haddr;
                      iPSELS7    <= PselS7Int_saved_haddr;
                      iPSELS8    <= PselS8Int_saved_haddr;
                      iPSELS9    <= PselS9Int_saved_haddr;
                      iPSELS10   <= PselS10Int_saved_haddr;
                      iPSELS11   <= PselS11Int_saved_haddr;
                      iPSELS12   <= PselS12Int_saved_haddr;
                      iPSELS13   <= PselS13Int_saved_haddr;
                      iPSELS14   <= PselS14Int_saved_haddr;
                      iPSELS15   <= PselS15Int_saved_haddr;
                      end
                    else
                      begin
                      iPSELS0    <= PselS0Int_piped_haddr;
                      iPSELS1    <= PselS1Int_piped_haddr;
                      iPSELS2    <= PselS2Int_piped_haddr;
                      iPSELS3    <= PselS3Int_piped_haddr;
                      iPSELS4    <= PselS4Int_piped_haddr;
                      iPSELS5    <= PselS5Int_piped_haddr;
                      iPSELS6    <= PselS6Int_piped_haddr;
                      iPSELS7    <= PselS7Int_piped_haddr;
                      iPSELS8    <= PselS8Int_piped_haddr;
                      iPSELS9    <= PselS9Int_piped_haddr;
                      iPSELS10   <= PselS10Int_piped_haddr;
                      iPSELS11   <= PselS11Int_piped_haddr;
                      iPSELS12   <= PselS12Int_piped_haddr;
                      iPSELS13   <= PselS13Int_piped_haddr;
                      iPSELS14   <= PselS14Int_piped_haddr;
                      iPSELS15   <= PselS15Int_piped_haddr;
                      end
                  end
                ST_READ:
                  begin
                    case(CurrentState)
                      ST_RWAIT:
                        begin

                          if (pipeline == 1'b0)
                            begin
                              iPSELS0    <= PselS0Int_saved_haddr;
                              iPSELS1    <= PselS1Int_saved_haddr;
                              iPSELS2    <= PselS2Int_saved_haddr;
                              iPSELS3    <= PselS3Int_saved_haddr;
                              iPSELS4    <= PselS4Int_saved_haddr;
                              iPSELS5    <= PselS5Int_saved_haddr;
                              iPSELS6    <= PselS6Int_saved_haddr;
                              iPSELS7    <= PselS7Int_saved_haddr;
                              iPSELS8    <= PselS8Int_saved_haddr;
                              iPSELS9    <= PselS9Int_saved_haddr;
                              iPSELS10   <= PselS10Int_saved_haddr;
                              iPSELS11   <= PselS11Int_saved_haddr;
                              iPSELS12   <= PselS12Int_saved_haddr;
                              iPSELS13   <= PselS13Int_saved_haddr;
                              iPSELS14   <= PselS14Int_saved_haddr;
                              iPSELS15   <= PselS15Int_saved_haddr;
                            end
                        end

                      ST_WENABLE:
                        begin
                          if (Valid == 1'b1)
                            begin
                              iPSELS0    <= PselS0Int_haddr;
                              iPSELS1    <= PselS1Int_haddr;
                              iPSELS2    <= PselS2Int_haddr;
                              iPSELS3    <= PselS3Int_haddr;
                              iPSELS4    <= PselS4Int_haddr;
                              iPSELS5    <= PselS5Int_haddr;
                              iPSELS6    <= PselS6Int_haddr;
                              iPSELS7    <= PselS7Int_haddr;
                              iPSELS8    <= PselS8Int_haddr;
                              iPSELS9    <= PselS9Int_haddr;
                              iPSELS10    <= PselS10Int_haddr;
                              iPSELS11    <= PselS11Int_haddr;
                              iPSELS12    <= PselS12Int_haddr;
                              iPSELS13    <= PselS13Int_haddr;
                              iPSELS14    <= PselS14Int_haddr;
                              iPSELS15    <= PselS15Int_haddr;
                            end
                          else
                            begin
                              iPSELS0    <= PselS0Int_piped_haddr;
                              iPSELS1    <= PselS1Int_piped_haddr;
                              iPSELS2    <= PselS2Int_piped_haddr;
                              iPSELS3    <= PselS3Int_piped_haddr;
                              iPSELS4    <= PselS4Int_piped_haddr;
                              iPSELS5    <= PselS5Int_piped_haddr;
                              iPSELS6    <= PselS6Int_piped_haddr;
                              iPSELS7    <= PselS7Int_piped_haddr;
                              iPSELS8    <= PselS8Int_piped_haddr;
                              iPSELS9    <= PselS9Int_piped_haddr;
                              iPSELS10   <= PselS10Int_piped_haddr;
                              iPSELS11   <= PselS11Int_piped_haddr;
                              iPSELS12   <= PselS12Int_piped_haddr;
                              iPSELS13   <= PselS13Int_piped_haddr;
                              iPSELS14   <= PselS14Int_piped_haddr;
                              iPSELS15   <= PselS15Int_piped_haddr;
                            end
                        end
                      ST_RENABLE:
                         begin
                           if (use_saved == 1'b1)
                             begin
                               iPSELS0    <= PselS0Int_saved_haddr;
                               iPSELS1    <= PselS1Int_saved_haddr;
                               iPSELS2    <= PselS2Int_saved_haddr;
                               iPSELS3    <= PselS3Int_saved_haddr;
                               iPSELS4    <= PselS4Int_saved_haddr;
                               iPSELS5    <= PselS5Int_saved_haddr;
                               iPSELS6    <= PselS6Int_saved_haddr;
                               iPSELS7    <= PselS7Int_saved_haddr;
                               iPSELS8    <= PselS8Int_saved_haddr;
                               iPSELS9    <= PselS9Int_saved_haddr;
                               iPSELS10   <= PselS10Int_saved_haddr;
                               iPSELS11   <= PselS11Int_saved_haddr;
                               iPSELS12   <= PselS12Int_saved_haddr;
                               iPSELS13   <= PselS13Int_saved_haddr;
                               iPSELS14   <= PselS14Int_saved_haddr;
                               iPSELS15   <= PselS15Int_saved_haddr;
                             end

                           else
                             begin
                               iPSELS0    <= PselS0Int_haddr;
                               iPSELS1    <= PselS1Int_haddr;
                               iPSELS2    <= PselS2Int_haddr;
                               iPSELS3    <= PselS3Int_haddr;
                               iPSELS4    <= PselS4Int_haddr;
                               iPSELS5    <= PselS5Int_haddr;
                               iPSELS6    <= PselS6Int_haddr;
                               iPSELS7    <= PselS7Int_haddr;
                               iPSELS8    <= PselS8Int_haddr;
                               iPSELS9    <= PselS9Int_haddr;
                               iPSELS10   <= PselS10Int_haddr;
                               iPSELS11   <= PselS11Int_haddr;
                               iPSELS12   <= PselS12Int_haddr;
                               iPSELS13   <= PselS13Int_haddr;
                               iPSELS14   <= PselS14Int_haddr;
                               iPSELS15   <= PselS15Int_haddr;
                             end

                         end
                      default:                     
                        begin
                          iPSELS0    <= PselS0Int_haddr;
                          iPSELS1    <= PselS1Int_haddr;
                          iPSELS2    <= PselS2Int_haddr;
                          iPSELS3    <= PselS3Int_haddr;
                          iPSELS4    <= PselS4Int_haddr;
                          iPSELS5    <= PselS5Int_haddr;
                          iPSELS6    <= PselS6Int_haddr;
                          iPSELS7    <= PselS7Int_haddr;
                          iPSELS8    <= PselS8Int_haddr;
                          iPSELS9    <= PselS9Int_haddr;
                          iPSELS10   <= PselS10Int_haddr;
                          iPSELS11   <= PselS11Int_haddr;
                          iPSELS12   <= PselS12Int_haddr;
                          iPSELS13   <= PselS13Int_haddr;
                          iPSELS14   <= PselS14Int_haddr;
                          iPSELS15   <= PselS15Int_haddr;
                        end
                    endcase
                  end
                ST_RWAIT,
                ST_WWAIT,
                ST_IDLE:
                  begin
                    iPSELS0  <= 1'b0;
                    iPSELS1  <= 1'b0;
                    iPSELS2  <= 1'b0;
                    iPSELS3  <= 1'b0;
                    iPSELS4  <= 1'b0;
                    iPSELS5  <= 1'b0;
                    iPSELS6  <= 1'b0;
                    iPSELS7  <= 1'b0;
                    iPSELS8  <= 1'b0;
                    iPSELS9  <= 1'b0;
                    iPSELS10 <= 1'b0;
                    iPSELS11 <= 1'b0;
                    iPSELS12 <= 1'b0;
                    iPSELS13 <= 1'b0;
                    iPSELS14 <= 1'b0;
                    iPSELS15 <= 1'b0;
                  end
                default:
                  begin
                    iPSELS0  <= iPSELS0 ;
                    iPSELS1  <= iPSELS1 ;
                    iPSELS2  <= iPSELS2 ;
                    iPSELS3  <= iPSELS3 ;
                    iPSELS4  <= iPSELS4 ;
                    iPSELS5  <= iPSELS5 ;
                    iPSELS6  <= iPSELS6 ;
                    iPSELS7  <= iPSELS7 ;
                    iPSELS8  <= iPSELS8 ;
                    iPSELS9  <= iPSELS9 ;
                    iPSELS10 <= iPSELS10;
                    iPSELS11 <= iPSELS11;
                    iPSELS12 <= iPSELS12;
                    iPSELS13 <= iPSELS13;
                    iPSELS14 <= iPSELS14;
                    iPSELS15 <= iPSELS15;
                  end
              endcase
            end
        end
    end

  assign   o_psels0  = iPSELS0;
  assign   o_psels1  = iPSELS1;
  assign   o_psels2  = iPSELS2;
  assign   o_psels3  = iPSELS3;
  assign   o_psels4  = iPSELS4;
  assign   o_psels5  = iPSELS5;
  assign   o_psels6  = iPSELS6;
  assign   o_psels7  = iPSELS7;
  assign   o_psels8  = iPSELS8;
  assign   o_psels9  = iPSELS9;
  assign   o_psels10 = iPSELS10;
  assign   o_psels11 = iPSELS11;
  assign   o_psels12 = iPSELS12;
  assign   o_psels13 = iPSELS13;
  assign   o_psels14 = iPSELS14;
  assign   o_psels15 = iPSELS15;
  assign   o_pwdata  = IntPWDATA_next;
  assign   o_penable = IntPENABLE;

  assign o_hrdata = PRDATA;

  assign PselBus[3] = o_psels15 | o_psels14 | o_psels13 | o_psels12 |
    o_psels11 | o_psels10 | o_psels9 | o_psels8;
  assign PselBus[2] = o_psels15 | o_psels14 | o_psels13 | o_psels12 |
    o_psels7 | o_psels6 | o_psels5 | o_psels4;
  assign PselBus[1] = o_psels15 | o_psels14 | o_psels11 | o_psels10 |
    o_psels7 | o_psels6 | o_psels3 | o_psels2;
  assign PselBus[0] = o_psels15 | o_psels13 | o_psels11 | o_psels9 |
    o_psels7 | o_psels5 | o_psels3 | o_psels1;

  always @ (PselBus or i_prdatas0 or i_prdatas1 or i_prdatas2 or
            i_prdatas3 or i_prdatas4 or i_prdatas5 or i_prdatas6 or
            i_prdatas7 or i_prdatas8 or i_prdatas9 or i_prdatas10 or
            i_prdatas11 or i_prdatas12 or i_prdatas13 or i_prdatas14 or
            i_prdatas15 or o_psels0)
    begin
      case (PselBus)
        PSEL_S0 : 
        if(o_psels0)
          PRDATA = i_prdatas0;
        else
          PRDATA = {32{1'b0}};

        PSEL_S1  : PRDATA = i_prdatas1;
        PSEL_S2  : PRDATA = i_prdatas2;
        PSEL_S3  : PRDATA = i_prdatas3;
        PSEL_S4  : PRDATA = i_prdatas4;
        PSEL_S5  : PRDATA = i_prdatas5;
        PSEL_S6  : PRDATA = i_prdatas6;
        PSEL_S7  : PRDATA = i_prdatas7;
        PSEL_S8  : PRDATA = i_prdatas8;
        PSEL_S9  : PRDATA = i_prdatas9;
        PSEL_S10 : PRDATA = i_prdatas10;
        PSEL_S11 : PRDATA = i_prdatas11;
        PSEL_S12 : PRDATA = i_prdatas12;
        PSEL_S13 : PRDATA = i_prdatas13;
        PSEL_S14 : PRDATA = i_prdatas14;
        PSEL_S15 : PRDATA = i_prdatas15;
        default  : PRDATA = {32{1'b0}};
      endcase
    end

always@(*)begin
case(CurrentState)
ST_IDLE   :iHREADYOUT = 1'b1;
ST_WWAIT  :iHREADYOUT = 1'b0;
ST_WRITE  :iHREADYOUT = 1'b0;
ST_RENABLE:iHREADYOUT = pready;
ST_WENABLE:iHREADYOUT = pready;
ST_RWAIT  :iHREADYOUT = 1'b0;
ST_READ   :iHREADYOUT = 1'b0;
default   :iHREADYOUT = 1'b0;
endcase
end
  assign o_hreadyout = iHREADYOUT;

  assign o_hresp = 2'b00;

endmodule
