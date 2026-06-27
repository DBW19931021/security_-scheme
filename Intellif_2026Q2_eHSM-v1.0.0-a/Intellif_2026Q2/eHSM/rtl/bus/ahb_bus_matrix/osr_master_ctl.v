//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_master_ctl #(
    parameter P_BUS_MATRIX_AWIDTH    =  32              ,
    parameter P_BUS_MATRIX_DWIDTH    =  32              ,
    parameter P_BUS_MATRIX_DELAY_EN  =  1'b1             
)
(
    input  wire                             hclk         ,
    input  wire                             hresetn      ,

    input  wire                             i_sel0       ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_addr0      ,
    input  wire [1:0]                       i_trans0     ,
    input  wire                             i_write0     ,
    input  wire [2:0]                       i_size0      ,
    input  wire [2:0]                       i_burst0     ,
    input  wire [3:0]                       i_prot0      ,
    input  wire [7:0]                       i_master0    ,
    input  wire                             i_mastlock0  ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_wdata0     ,
    input  wire                             i_heldtran0  ,

    input  wire                             i_sel1       ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_addr1      ,
    input  wire [1:0]                       i_trans1     ,
    input  wire                             i_write1     ,
    input  wire [2:0]                       i_size1      ,
    input  wire [2:0]                       i_burst1     ,
    input  wire [3:0]                       i_prot1      ,
    input  wire [7:0]                       i_master1    ,
    input  wire                             i_mastlock1  ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_wdata1     ,
    input  wire                             i_heldtran1  ,

    input  wire                             i_sel2       ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_addr2      ,
    input  wire [1:0]                       i_trans2     ,
    input  wire                             i_write2     ,
    input  wire [2:0]                       i_size2      ,
    input  wire [2:0]                       i_burst2     ,
    input  wire [3:0]                       i_prot2      ,
    input  wire [7:0]                       i_master2    ,
    input  wire                             i_mastlock2  ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_wdata2     ,
    input  wire                             i_heldtran2  ,

    input  wire                             i_sel3       ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_addr3      ,
    input  wire [1:0]                       i_trans3     ,
    input  wire                             i_write3     ,
    input  wire [2:0]                       i_size3      ,
    input  wire [2:0]                       i_burst3     ,
    input  wire [3:0]                       i_prot3      ,
    input  wire [7:0]                       i_master3    ,
    input  wire                             i_mastlock3  ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_wdata3     ,
    input  wire                             i_heldtran3  ,

    input  wire                             i_sel4       ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_addr4      ,
    input  wire [1:0]                       i_trans4     ,
    input  wire                             i_write4     ,
    input  wire [2:0]                       i_size4      ,
    input  wire [2:0]                       i_burst4     ,
    input  wire [3:0]                       i_prot4      ,
    input  wire [7:0]                       i_master4    ,
    input  wire                             i_mastlock4  ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_wdata4     ,
    input  wire                             i_heldtran4  ,

    input  wire                             i_hreadyoutm ,

    input  wire                             i_readyoutarb0,
    input  wire                             i_readyoutarb1,
    input  wire                             i_readyoutarb2,
    input  wire                             i_readyoutarb3,
    input  wire                             i_readyoutarb4,

    output wire                             o_active0    ,
    output wire                             o_active1    ,
    output wire                             o_active2    ,
    output wire                             o_active3    ,
    output wire                             o_active4    ,

    output wire                             o_hselm      ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]   o_haddrm     ,
    output wire [1:0]                       o_htransm    ,
    output wire                             o_hwritem    ,
    output wire [2:0]                       o_hsizem     ,
    output wire [2:0]                       o_hburstm    ,
    output wire [3:0]                       o_hprotm     ,
    output wire [7:0]                       o_hmasterm   ,
    output wire                             o_hmastlockm ,
    output wire                             o_hreadymuxm ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hwdatam
);

wire        reqport0;
wire        reqport1;
wire        reqport2;
wire        reqport3;
wire        reqport4;

wire  [3:0] addrinport;
wire  [3:0] datainport_in;
reg   [3:0] datainport;
wire        noport;
reg         slavesel;

reg         hsellock;
wire        hsellock_in;
wire        hlockarb;

wire        ihselm;
wire  [1:0] ihtransm;
wire  [2:0] ihburstm;
wire        ihreadymuxm;
wire        ihmastlockm;

assign reqport0  = (i_heldtran0 & i_sel0);
assign reqport1  = (i_heldtran1 & i_sel1);
assign reqport2  = (i_heldtran2 & i_sel2);
assign reqport3  = (i_heldtran3 & i_sel3);
assign reqport4  = (i_heldtran4 & i_sel4);

generate 
    if(P_BUS_MATRIX_DELAY_EN)begin:busmatrix_addr_delay_one_beat
        osr_prio_ctl u_prio_ctl(
            .hclk                  (hclk),
            .hresetn               (hresetn),
            .i_reqport0            (reqport0),
            .i_reqport1            (reqport1),
            .i_reqport2            (reqport2),
            .i_reqport3            (reqport3),
            .i_reqport4            (reqport4),
            .i_hreadym             (ihreadymuxm),
            .i_hselm               (ihselm),
            .i_htransm             (ihtransm),
            .i_hburstm             (ihburstm),
            .i_hmastlockm          (hlockarb),
            .o_addrinport          (addrinport),
            .o_noport              (noport)
            );
    end
    else begin:busmatrix_addr_no_delay
        assign   noport       = ~ (reqport0 | reqport1 | reqport2 | reqport3 | reqport4);
        assign   addrinport   = (reqport0 & i_sel0 & (|i_trans0)) ? 4'b0000 : 
                                (reqport1 & i_sel1 & (|i_trans1)) ? 4'b0001 : 
                                (reqport2 & i_sel2 & (|i_trans2)) ? 4'b0010 : 
                                (reqport3 & i_sel3 & (|i_trans3)) ? 4'b0100 :
                                (reqport4 & i_sel4 & (|i_trans4)) ? 4'b1000 : 4'b0000; 
    end
endgenerate

wire   addrinport_0 = addrinport == 4'b0000;
wire   addrinport_1 = addrinport == 4'b0001;
wire   addrinport_2 = addrinport == 4'b0010;
wire   addrinport_3 = addrinport == 4'b0100;
wire   addrinport_4 = addrinport == 4'b1000;

generate 
    if(P_BUS_MATRIX_DELAY_EN)begin
        assign o_active0 = ~noport & addrinport_0;
        assign o_active1 = ~noport & addrinport_1;
        assign o_active2 = ~noport & addrinport_2;
        assign o_active3 = ~noport & addrinport_3;
        assign o_active4 = ~noport & addrinport_4;
    end
    else begin
        assign o_active0 = ~noport & addrinport_0 & i_readyoutarb0;
        assign o_active1 = ~noport & addrinport_1 & i_readyoutarb1;
        assign o_active2 = ~noport & addrinport_2 & i_readyoutarb2;
        assign o_active3 = ~noport & addrinport_3 & i_readyoutarb3;
        assign o_active4 = ~noport & addrinport_4 & i_readyoutarb4;
    end
endgenerate

assign ihselm    = o_active0 & i_sel0 |
                   o_active1 & i_sel1 |
                   o_active2 & i_sel2 |
                   o_active3 & i_sel3 |
                   o_active4 & i_sel4 ;

assign o_haddrm  = {{P_BUS_MATRIX_AWIDTH}{o_active0 & ihselm}} & i_addr0 |
                   {{P_BUS_MATRIX_AWIDTH}{o_active1 & ihselm}} & i_addr1 |
                   {{P_BUS_MATRIX_AWIDTH}{o_active2 & ihselm}} & i_addr2 |
                   {{P_BUS_MATRIX_AWIDTH}{o_active3 & ihselm}} & i_addr3 |
                   {{P_BUS_MATRIX_AWIDTH}{o_active4 & ihselm}} & i_addr4 ;

assign ihtransm  = {2{o_active0 & ihselm & i_hreadyoutm }} & i_trans0 |
                   {2{o_active1 & ihselm & i_hreadyoutm }} & i_trans1 |
                   {2{o_active2 & ihselm & i_hreadyoutm }} & i_trans2 |
                   {2{o_active3 & ihselm & i_hreadyoutm }} & i_trans3 |
                   {2{o_active4 & ihselm & i_hreadyoutm }} & i_trans4 ;

assign o_hwritem = o_active0 & i_write0 & ihselm |
                   o_active1 & i_write1 & ihselm |
                   o_active2 & i_write2 & ihselm |
                   o_active3 & i_write3 & ihselm |
                   o_active4 & i_write4 & ihselm ;

assign o_hsizem = {3{o_active0 & ihselm}} & i_size0 |
                  {3{o_active1 & ihselm}} & i_size1 |
                  {3{o_active2 & ihselm}} & i_size2 |
                  {3{o_active3 & ihselm}} & i_size3 |
                  {3{o_active4 & ihselm}} & i_size4 ;

assign ihburstm = {3{o_active0 & ihselm}} & i_burst0 |
                  {3{o_active1 & ihselm}} & i_burst1 |
                  {3{o_active2 & ihselm}} & i_burst2 |
                  {3{o_active3 & ihselm}} & i_burst3 |
                  {3{o_active4 & ihselm}} & i_burst4 ;

assign o_hprotm = {4{o_active0 & ihselm}} & i_prot0 |
                  {4{o_active1 & ihselm}} & i_prot1 |
                  {4{o_active2 & ihselm}} & i_prot2 |
                  {4{o_active3 & ihselm}} & i_prot3 |
                  {4{o_active4 & ihselm}} & i_prot4 ;

assign o_hmasterm = {8{o_active0 & ihselm}} & i_master0 |
                    {8{o_active1 & ihselm}} & i_master1 |
                    {8{o_active2 & ihselm}} & i_master2 |
                    {8{o_active3 & ihselm}} & i_master3 |
                    {8{o_active4 & ihselm}} & i_master4 ;

assign ihmastlockm = o_active0 & i_mastlock0 & ihselm |
                     o_active1 & i_mastlock1 & ihselm |
                     o_active2 & i_mastlock2 & ihselm |
                     o_active3 & i_mastlock3 & ihselm |
                     o_active4 & i_mastlock4 & ihselm ;

wire   hsellock_t   = (ihselm & ihtransm[1] & ihmastlockm) ? 1'b1 :
                       ihmastlockm ? hsellock : 1'b0;
assign hsellock_in  = ihreadymuxm ? hsellock_t : hsellock;

assign hlockarb     = ihmastlockm & (hsellock | ihselm);

assign o_htransm    = ihtransm ;
assign o_hburstm    = ihburstm ;
assign o_hselm      = ihselm ;
assign o_hmastlockm = ihmastlockm ;

generate 
     if(P_BUS_MATRIX_DELAY_EN)begin:busmatrix_data_delay_one_beat
        assign datainport_in = ihreadymuxm ? addrinport : datainport;
     end
     else begin:busmatrix_data_no_delay
        assign datainport_in = ihselm ? addrinport : datainport;
     end
endgenerate

wire   datainport_0 = datainport == 4'b0000;
wire   datainport_1 = datainport == 4'b0001;
wire   datainport_2 = datainport == 4'b0010;
wire   datainport_3 = datainport == 4'b0100;
wire   datainport_4 = datainport == 4'b1000;

assign o_hwdatam = {{P_BUS_MATRIX_DWIDTH}{datainport_0}} & i_wdata0 |
                   {{P_BUS_MATRIX_DWIDTH}{datainport_1}} & i_wdata1 |
                   {{P_BUS_MATRIX_DWIDTH}{datainport_2}} & i_wdata2 |
                   {{P_BUS_MATRIX_DWIDTH}{datainport_3}} & i_wdata3 |
                   {{P_BUS_MATRIX_DWIDTH}{datainport_4}} & i_wdata4 ;

wire   slavesel_in = ihreadymuxm ? ihselm : slavesel;

assign ihreadymuxm = slavesel ? i_hreadyoutm : 1'b1;
assign o_hreadymuxm  = ihreadymuxm ;

always @ (negedge hresetn or posedge hclk)
    if(~hresetn) begin
        hsellock    <= 1'b0;
        datainport  <= 4'b0000;
        slavesel    <= 1'b0 ;
    end
    else begin
        hsellock    <= hsellock_in;
        datainport  <= datainport_in ;
        slavesel    <= slavesel_in ;
    end

endmodule
