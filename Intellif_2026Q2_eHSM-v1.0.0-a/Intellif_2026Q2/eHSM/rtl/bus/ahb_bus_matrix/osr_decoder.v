//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_decoder #(
    parameter P_BUS_MATRIX_AWIDTH    =  32                  ,
    parameter P_BUS_MATRIX_DWIDTH    =  32                  ,
    parameter P_BUS_MATRIX_DELAY_EN  =  1'b0                ,
    parameter P_SLAVE0_ADDR_MSB      =  12'h100             ,
    parameter P_SLAVE1_ADDR_MSB      =  12'h108             ,
    parameter P_SLAVE1_RMP8_EN       =  0                   ,
    parameter P_SLAVE1_ADDR_RMP8     =  8'h78               ,
    parameter P_SLAVE2_ADDR_MSB      =  8'h20               ,
    parameter P_SLAVE3_ADDR_MSB      =  4'h3                ,
    parameter P_SLAVE4_ADDR_MSB      =  4'h7                ,
    parameter P_SLAVE4_RMP8_EN       =  0                   ,
    parameter P_SLAVE4_ADDR_RMP8     =  8'h70               ,
    parameter P_SLAVE4_RMP10_EN      =  0                   ,
    parameter P_SLAVE4_ADDR_RMP10    =  10'h0               ,
    parameter P_SLAVE4_RMP12_EN      =  0                   ,
    parameter P_SLAVE4_ADDR_RMP12    =  12'h104             ,
    parameter P_SLAVE5_ADDR_MSB      =  1'h1                 
)
(
    input  wire                              hclk             ,
    input  wire                              hresetn          ,
    input  wire                              i_hreadys        ,
    input  wire                              i_sel            ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]    i_addr           ,
    input  wire [1:0]                        i_trans          ,
    input  wire                              i_pendtran       ,

    input  wire                              i_active0        ,
    input  wire                              i_readyout0      ,
    input  wire [1:0]                        i_resp0          ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]    i_rdata0         ,
    input  wire                              i_active1        ,
    input  wire                              i_readyout1      ,
    input  wire [1:0]                        i_resp1          ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]    i_rdata1         ,
    input  wire                              i_active2        ,
    input  wire                              i_readyout2      ,
    input  wire [1:0]                        i_resp2          ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]    i_rdata2         ,
    input  wire                              i_active3        ,
    input  wire                              i_readyout3      ,
    input  wire [1:0]                        i_resp3          ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]    i_rdata3         ,
    input  wire                              i_active4        ,
    input  wire                              i_readyout4      ,
    input  wire [1:0]                        i_resp4          ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]    i_rdata4         ,
    input  wire                              i_active5        ,
    input  wire                              i_readyout5      ,
    input  wire [1:0]                        i_resp5          ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]    i_rdata5         ,

    output wire                              o_sel0           ,
    output wire                              o_sel1           ,
    output wire                              o_sel2           ,
    output wire                              o_sel3           ,
    output wire                              o_sel4           ,
    output wire                              o_sel5           ,

    output wire                              o_data_dft       ,
    output wire                              o_active         ,
    output wire                              o_hreadyouts     ,
    output wire                              o_hreadyoutnd    ,
    output wire                              o_hreadyoutarb   ,
    output wire  [1:0]                       o_hresps         ,
    output wire  [P_BUS_MATRIX_DWIDTH-1:0]   o_hrdatas
);

localparam RSP_OKAY  =  2'b00  ;
localparam RSP_ERROR =  2'b01  ;

localparam DATAOUTPORT_DEFAULT = 4'b1111 ;

wire          w_dft_valid;
reg           r_dft_readyout;
wire          w_dft_readyout;
reg     [1:0] r_dft_resp;
wire    [1:0] w_dft_resp;

reg     [3:0] dataoutport;

wire          slavehit0;
wire          slavehit1;
wire          slavehit2;
wire          slavehit3;
wire          slavehit4;
wire          slavehit5;

wire trans_addr_phase = (i_sel & i_trans[1]);

assign w_dft_valid    = (i_hreadys & i_sel & i_trans[1]);
assign w_dft_readyout = r_dft_readyout ? ~w_dft_valid : 1'b1;
assign w_dft_resp     = RSP_OKAY;

always @ (negedge hresetn or posedge hclk)
    if(~hresetn)begin
        r_dft_readyout  <= 1'b1;
        r_dft_resp       <= RSP_OKAY;
    end
    else begin 
        r_dft_readyout  <= w_dft_readyout;
        if(r_dft_readyout)
        r_dft_resp      <= w_dft_resp;
    end

assign slavehit0   = trans_addr_phase & (i_addr[31:20] == P_SLAVE0_ADDR_MSB); 
generate
    if(P_SLAVE1_RMP8_EN == 1)begin
    assign slavehit1   = trans_addr_phase & ((i_addr[31:24] == P_SLAVE1_ADDR_RMP8)||(i_addr[31:20] == P_SLAVE1_ADDR_MSB)); 
    end
    else begin
    assign slavehit1   = trans_addr_phase & (i_addr[31:20] == P_SLAVE1_ADDR_MSB); 
    end
endgenerate
assign slavehit2   = trans_addr_phase & (i_addr[31:24] == P_SLAVE2_ADDR_MSB); 
assign slavehit3   = trans_addr_phase & (i_addr[31:28] == P_SLAVE3_ADDR_MSB); 
generate
    if(P_SLAVE4_RMP8_EN == 1)begin
        assign slavehit4   = trans_addr_phase & ((i_addr[31:24] == P_SLAVE4_ADDR_RMP8[7:0])||(i_addr[31:22] == P_SLAVE4_ADDR_RMP12[11:2])); 
    end 
    else if(P_SLAVE4_RMP12_EN == 1)begin
        assign slavehit4   = trans_addr_phase & (i_addr[31:22] == P_SLAVE4_ADDR_RMP12[11:2]); 
    end
    else if(P_SLAVE4_RMP10_EN == 1)begin
        assign slavehit4   = trans_addr_phase & (i_addr[31:22] == P_SLAVE4_ADDR_RMP10); 
    end        
    else begin
        assign slavehit4   = trans_addr_phase & (i_addr[31:28] == P_SLAVE4_ADDR_MSB);
    end
endgenerate
assign slavehit5   = trans_addr_phase & (i_addr[31   ] == P_SLAVE5_ADDR_MSB); 

wire noslave = ~( slavehit0 |
                  slavehit1 |
                  slavehit2 |
                  slavehit3 |
                  slavehit4 | 
                  slavehit5 );

wire [3:0] addroutport = ( 4'b0000 & {4{slavehit0}} )  | 
                         ( 4'b0001 & {4{slavehit1}} )  | 
                         ( 4'b0010 & {4{slavehit2}} )  | 
                         ( 4'b0011 & {4{slavehit3}} )  | 
                         ( 4'b0100 & {4{slavehit4}} )  | 
                         ( 4'b0101 & {4{slavehit5}} )  | 
                         ( DATAOUTPORT_DEFAULT & {4{noslave}}     )  ;

wire   addroutport_0   = (addroutport == 4'b0000);
wire   addroutport_1   = (addroutport == 4'b0001);
wire   addroutport_2   = (addroutport == 4'b0010);
wire   addroutport_3   = (addroutport == 4'b0011);
wire   addroutport_4   = (addroutport == 4'b0100);
wire   addroutport_5   = (addroutport == 4'b0101);
wire   addroutport_dft = (addroutport == DATAOUTPORT_DEFAULT);

assign o_sel0 = i_sel & addroutport_0;
assign o_sel1 = i_sel & addroutport_1;
assign o_sel2 = i_sel & addroutport_2;
assign o_sel3 = i_sel & addroutport_3;
assign o_sel4 = i_sel & addroutport_4;
assign o_sel5 = i_sel & addroutport_5;

assign o_active = addroutport_0 & i_active0 |
                  addroutport_1 & i_active1 |
                  addroutport_2 & i_active2 |
                  addroutport_3 & i_active3 |
                  addroutport_4 & i_active4 |
                  addroutport_5 & i_active5 |
                  addroutport_dft & w_dft_valid;

wire [3:0] dataoutport_in = i_hreadys ? addroutport : dataoutport;
always @ (posedge hclk or negedge hresetn)
    if(~hresetn)
        dataoutport  <= 4'b0000;
    else
        dataoutport  <= dataoutport_in;

wire   dataoutport_0   = (dataoutport == 4'b0000);
wire   dataoutport_1   = (dataoutport == 4'b0001);
wire   dataoutport_2   = (dataoutport == 4'b0010);
wire   dataoutport_3   = (dataoutport == 4'b0011);
wire   dataoutport_4   = (dataoutport == 4'b0100);
wire   dataoutport_5   = (dataoutport == 4'b0101);
wire   dataoutport_dft = (dataoutport == DATAOUTPORT_DEFAULT);

assign o_data_dft      = dataoutport_dft;

generate
    if(P_BUS_MATRIX_DELAY_EN)begin
        assign o_hreadyoutnd  = 1'b0;
        assign o_hreadyoutarb = 1'b0;

    end
    else begin
        assign o_hreadyoutnd  = (dataoutport_in == 4'b0000) & i_readyout0 |
                                (dataoutport_in == 4'b0001) & i_readyout1 |
                                (dataoutport_in == 4'b0010) & i_readyout2 |
                                (dataoutport_in == 4'b0011) & i_readyout3 |
                                (dataoutport_in == 4'b0100) & i_readyout4 |
                                (dataoutport_in == 4'b0101) & i_readyout5 |
                                dataoutport_dft;

        assign o_hreadyoutarb = (addroutport == 4'b0000) & i_readyout0 ? 1'b1 :
                                (addroutport == 4'b0001) & i_readyout1 ? 1'b1 :
                                (addroutport == 4'b0010) & i_readyout2 ? 1'b1 :
                                (addroutport == 4'b0011) & i_readyout3 ? 1'b1 :
                                (addroutport == 4'b0100) & i_readyout4 ? 1'b1 :
                                (addroutport == 4'b0101) & i_readyout5 ? 1'b1 :
                                (addroutport == DATAOUTPORT_DEFAULT)   ? 1'b1 : 1'b0 ;      
    end
endgenerate

assign o_hreadyouts  = dataoutport_0   & i_readyout0 |
                       dataoutport_1   & i_readyout1 |
                       dataoutport_2   & i_readyout2 |
                       dataoutport_3   & i_readyout3 |
                       dataoutport_4   & i_readyout4 |
                       dataoutport_5   & i_readyout5 |
                       dataoutport_dft & r_dft_readyout;

assign o_hresps   = {2{dataoutport_0}}   & i_resp0 |
                    {2{dataoutport_1}}   & i_resp1 |
                    {2{dataoutport_2}}   & i_resp2 |
                    {2{dataoutport_3}}   & i_resp3 |
                    {2{dataoutport_4}}   & i_resp4 |
                    {2{dataoutport_5}}   & i_resp5 |
                    {2{dataoutport_dft}} & r_dft_resp;

assign o_hrdatas = ( i_rdata0 & {P_BUS_MATRIX_DWIDTH{dataoutport_0}} )  |
                   ( i_rdata1 & {P_BUS_MATRIX_DWIDTH{dataoutport_1}} )  |
                   ( i_rdata2 & {P_BUS_MATRIX_DWIDTH{dataoutport_2}} )  |
                   ( i_rdata3 & {P_BUS_MATRIX_DWIDTH{dataoutport_3}} )  |
                   ( i_rdata4 & {P_BUS_MATRIX_DWIDTH{dataoutport_4}} )  |
                   ( i_rdata5 & {P_BUS_MATRIX_DWIDTH{dataoutport_5}} )  ;

endmodule
