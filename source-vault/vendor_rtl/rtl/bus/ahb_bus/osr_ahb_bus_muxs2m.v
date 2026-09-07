//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_bus_muxs2m (
    input  wire          hclk                     ,
    input  wire          hresetn                  ,
    input  wire          i_clk_en                 ,    
    input  wire          i_hsel0                  ,
    input  wire          i_hsel1                  ,
    input  wire          i_hsel2                  ,
    input  wire          i_hsel3                  ,
    input  wire          i_hsel4                  ,
    input  wire          i_hsel5                  ,
    input  wire          i_hsel6                  ,
    input  wire          i_hsel7                  ,
    input  wire          i_hsel8                  ,
    input  wire          i_hsel9                  ,
    input  wire          i_hsel10                 ,
    input  wire          i_hsel11                 ,
    input  wire          i_hsel12                 ,
    input  wire          i_hsel13                 ,
    input  wire          i_hsel14                 ,
    input  wire          i_hsel15                 ,
    input  wire [31:0]   i_hrdata0                ,
    input  wire          i_hreadyout0             ,
    input  wire [1:0]    i_hresp0                 ,
    input  wire [31:0]   i_hrdata1                ,
    input  wire          i_hreadyout1             ,
    input  wire [1:0]    i_hresp1                 ,
    input  wire [31:0]   i_hrdata2                ,
    input  wire          i_hreadyout2             ,
    input  wire [1:0]    i_hresp2                 ,
    input  wire [31:0]   i_hrdata3                ,
    input  wire          i_hreadyout3             ,
    input  wire [1:0]    i_hresp3                 ,
    input  wire [31:0]   i_hrdata4                ,
    input  wire          i_hreadyout4             ,
    input  wire [1:0]    i_hresp4                 ,
    input  wire [31:0]   i_hrdata5                ,
    input  wire          i_hreadyout5             ,
    input  wire [1:0]    i_hresp5                 ,
    input  wire [31:0]   i_hrdata6                ,
    input  wire          i_hreadyout6             ,
    input  wire [1:0]    i_hresp6                 ,
    input  wire [31:0]   i_hrdata7                ,
    input  wire          i_hreadyout7             ,
    input  wire [1:0]    i_hresp7                 ,
    input  wire [31:0]   i_hrdata8                ,
    input  wire          i_hreadyout8             ,
    input  wire [1:0]    i_hresp8                 ,
    input  wire [31:0]   i_hrdata9                ,
    input  wire          i_hreadyout9             ,
    input  wire [1:0]    i_hresp9                 ,
    input  wire [31:0]   i_hrdata10               ,
    input  wire          i_hreadyout10            ,
    input  wire [1:0]    i_hresp10                ,
    input  wire [31:0]   i_hrdata11               ,
    input  wire          i_hreadyout11            ,
    input  wire [1:0]    i_hresp11                ,
    input  wire [31:0]   i_hrdata12               ,
    input  wire          i_hreadyout12            ,
    input  wire [1:0]    i_hresp12                ,
    input  wire [31:0]   i_hrdata13               ,
    input  wire          i_hreadyout13            ,
    input  wire [1:0]    i_hresp13                ,
    input  wire [31:0]   i_hrdata14               ,
    input  wire          i_hreadyout14            ,
    input  wire [1:0]    i_hresp14                ,
    input  wire [31:0]   i_hrdata15               ,
    input  wire          i_hreadyout15            ,
    input  wire [1:0]    i_hresp15                ,
    output reg  [31:0]   o_hrdata                 ,
    output reg           o_hready                 ,
    output reg  [1:0]    o_hresp                   
);

parameter P_HSEL15= 16'b1000_0000_0000_0000;
parameter P_HSEL14= 16'b0100_0000_0000_0000;
parameter P_HSEL13= 16'b0010_0000_0000_0000;
parameter P_HSEL12= 16'b0001_0000_0000_0000;
parameter P_HSEL11= 16'b0000_1000_0000_0000;
parameter P_HSEL10= 16'b0000_0100_0000_0000;
parameter P_HSEL9 = 16'b0000_0010_0000_0000;
parameter P_HSEL8 = 16'b0000_0001_0000_0000;
parameter P_HSEL7 = 16'b0000_0000_1000_0000;
parameter P_HSEL6 = 16'b0000_0000_0100_0000;
parameter P_HSEL5 = 16'b0000_0000_0010_0000;
parameter P_HSEL4 = 16'b0000_0000_0001_0000;
parameter P_HSEL3 = 16'b0000_0000_0000_1000;
parameter P_HSEL2 = 16'b0000_0000_0000_0100;
parameter P_HSEL1 = 16'b0000_0000_0000_0010;
parameter P_HSEL0 = 16'b0000_0000_0000_0001;

wire      [15:0]   HselReg_in               ;
wire      [15:0]   HselReg_t                ;
reg       [15:0]   HselReg                  ;

assign HselReg_t = {
                     i_hsel15     ,
                     i_hsel14     ,
                     i_hsel13     ,
                     i_hsel12     ,
                     i_hsel11     ,
                     i_hsel10     ,
                     i_hsel9      ,
                     i_hsel8      ,
                     i_hsel7      ,
                     i_hsel6      ,
                     i_hsel5      ,
                     i_hsel4      ,
                     i_hsel3      ,
                     i_hsel2      ,
                     i_hsel1      ,
                     i_hsel0      };
assign HselReg_in = o_hready & i_clk_en ? HselReg_t : HselReg;

always @( negedge hresetn or posedge hclk)
    if (!hresetn)
        HselReg  <= 16'b0;
    else
        HselReg <= HselReg_in;

always @(*) begin
    case(HselReg)
        P_HSEL0 : begin
            o_hresp     = i_hresp0      ;
            o_hready    = i_hreadyout0  ;
            o_hrdata    = i_hrdata0     ;
        end
        P_HSEL1 : begin
            o_hresp     = i_hresp1      ;
            o_hready    = i_hreadyout1  ;
            o_hrdata    = i_hrdata1     ;
        end
        P_HSEL2 : begin
            o_hresp     = i_hresp2      ;
            o_hready    = i_hreadyout2  ;
            o_hrdata    = i_hrdata2     ;
        end
        P_HSEL3 : begin
            o_hresp     = i_hresp3      ;
            o_hready    = i_hreadyout3  ;
            o_hrdata    = i_hrdata3     ;
        end
        P_HSEL4 : begin
            o_hresp     = i_hresp4      ;
            o_hready    = i_hreadyout4  ;
            o_hrdata    = i_hrdata4     ;
        end
        P_HSEL5 : begin
            o_hresp     = i_hresp5      ;
            o_hready    = i_hreadyout5  ;
            o_hrdata    = i_hrdata5     ;
        end
        P_HSEL6 : begin
            o_hresp     = i_hresp6      ;
            o_hready    = i_hreadyout6  ;
            o_hrdata    = i_hrdata6     ;
        end
        P_HSEL7 : begin
            o_hresp     = i_hresp7      ;
            o_hready    = i_hreadyout7  ;
            o_hrdata    = i_hrdata7     ;
        end
        P_HSEL8 : begin
            o_hresp     = i_hresp8      ;
            o_hready    = i_hreadyout8  ;
            o_hrdata    = i_hrdata8     ;
        end
        P_HSEL9 : begin
            o_hresp     = i_hresp9      ;
            o_hready    = i_hreadyout9  ;
            o_hrdata    = i_hrdata9     ;
        end
        P_HSEL10 : begin
            o_hresp     = i_hresp10     ;
            o_hready    = i_hreadyout10 ;
            o_hrdata    = i_hrdata10    ;
        end
        P_HSEL11 : begin
            o_hresp     = i_hresp11     ;
            o_hready    = i_hreadyout11 ;
            o_hrdata    = i_hrdata11    ;
        end
        P_HSEL12 : begin
            o_hresp     = i_hresp12     ;
            o_hready    = i_hreadyout12 ;
            o_hrdata    = i_hrdata12    ;
        end
        P_HSEL13 : begin
            o_hresp     = i_hresp13     ;
            o_hready    = i_hreadyout13 ;
            o_hrdata    = i_hrdata13    ;
        end
        P_HSEL14 : begin
            o_hresp     = i_hresp14     ;
            o_hready    = i_hreadyout14 ;
            o_hrdata    = i_hrdata14    ;
        end
        P_HSEL15 : begin
            o_hresp     = i_hresp15     ;
            o_hready    = i_hreadyout15 ;
            o_hrdata    = i_hrdata15    ;
        end
        default : begin
            o_hresp     = 2'h0          ;
            o_hready    = 1'h1          ;
            o_hrdata    = 32'h0         ;
        end
    endcase
end 

endmodule 
