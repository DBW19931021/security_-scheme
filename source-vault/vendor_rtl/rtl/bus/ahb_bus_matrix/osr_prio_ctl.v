//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_prio_ctl (
    input  wire                   hclk         ,
    input  wire                   hresetn      ,
    input  wire                   i_reqport0   ,
    input  wire                   i_reqport1   ,
    input  wire                   i_reqport2   ,
    input  wire                   i_reqport3   ,
    input  wire                   i_reqport4   ,
    input  wire                   i_hreadym    ,
    input  wire                   i_hselm      ,
    input  wire [1:0]             i_htransm    ,
    input  wire [2:0]             i_hburstm    ,
    input  wire                   i_hmastlockm ,
    output wire [3:0]             o_addrinport ,
    output wire                   o_noport
);

localparam TRN_IDLE   = 2'b00  ;
localparam TRN_BUSY   = 2'b01  ;
localparam TRN_NONSEQ = 2'b10  ;
localparam TRN_SEQ    = 2'b11  ;

localparam BUR_SINGLE = 3'b000 ;
localparam BUR_INCR   = 3'b001 ;
localparam BUR_WRAP4  = 3'b010 ;
localparam BUR_INCR4  = 3'b011 ;
localparam BUR_WRAP8  = 3'b100 ;
localparam BUR_INCR8  = 3'b101 ;
localparam BUR_WRAP16 = 3'b110 ;
localparam BUR_INCR16 = 3'b111 ;

wire [3:0] addrinport_in;
reg  [3:0] iaddrinport;
wire       noport_in;
reg        inoport;
wire [3:0] burstcount_in;
reg  [3:0] burstcount;
wire       bursthold_in;
reg        bursthold;

wire       burst_16 = (i_hburstm == BUR_INCR16)|(i_hburstm == BUR_WRAP16);
wire       burst_8  = (i_hburstm == BUR_INCR8 )|(i_hburstm == BUR_WRAP8 );
wire       burst_4  = (i_hburstm == BUR_INCR4 )|(i_hburstm == BUR_WRAP4 );

wire       trn_nons = i_htransm == TRN_NONSEQ;
wire       trn_seq  = i_htransm == TRN_SEQ;
wire       trn_busy = i_htransm == TRN_BUSY;

wire [3:0] burstcount_m1 = burstcount - 1;
wire       burstcount_1  = burstcount == 4'b0001;
assign     burstcount_in = ~i_hreadym ? burstcount :
                           ~i_hselm   ? 4'b0000    : (
                           ({4{(trn_nons & burst_16)}} & 4'b1111) |
                           ({4{(trn_nons & burst_8)}}  & 4'b0111) |
                           ({4{(trn_nons & burst_4)}}  & 4'b0011) |
                           ({4{ trn_seq            }}  & burstcount_m1) |
                           ({4{ trn_busy           }}  & burstcount) );
assign     bursthold_in  = ~i_hreadym ? bursthold   :
                           ~i_hselm   ? 1'b0        : (
                           (trn_nons & (burst_16 | burst_8 | burst_4)) |
                           (trn_seq  & ~burstcount_1 & bursthold) |
                           (trn_busy & bursthold) );

wire       bus_vld           = i_hselm & (|i_htransm);
wire       addrinport_0_vld  = i_reqport0 | ((iaddrinport == 4'b0000) & bus_vld);
wire       addrinport_1_vld  = i_reqport1 | ((iaddrinport == 4'b0001) & bus_vld);
wire       addrinport_2_vld  = i_reqport2 | ((iaddrinport == 4'b0010) & bus_vld);
wire       addrinport_3_vld  = i_reqport3 | ((iaddrinport == 4'b0100) & bus_vld);
wire       addrinport_4_vld  = i_reqport4 | ((iaddrinport == 4'b1000) & bus_vld);
wire       bus_hold          = (i_hmastlockm | (bursthold_in == 1'b1));

assign     addrinport_in  = bus_hold         ? iaddrinport :
                            addrinport_0_vld ? 4'b0000     :
                            addrinport_1_vld ? 4'b0001     :
                            addrinport_2_vld ? 4'b0010     :
                            addrinport_3_vld ? 4'b0100     :
                            addrinport_4_vld ? 4'b1000     : iaddrinport;
assign     noport_in    = ~(bus_hold         |
                            addrinport_0_vld |
                            addrinport_1_vld |
                            addrinport_2_vld |
                            addrinport_3_vld |
                            addrinport_4_vld | i_hselm);

wire       inoport_in     = i_hreadym ? noport_in     : inoport;
wire [3:0] iaddrinport_in = i_hreadym ? addrinport_in : iaddrinport;

always @ (negedge hresetn or posedge hclk)
    if(~hresetn)
        begin
            burstcount  <= 4'b0000;
            bursthold   <= 1'b0;
            inoport     <= 1'b1 ;
            iaddrinport <= 4'b0000;
        end
    else
        begin
            burstcount  <= burstcount_in;
            bursthold   <= bursthold_in;
            inoport     <= inoport_in;
            iaddrinport <= iaddrinport_in;
        end

assign  o_addrinport = iaddrinport ;
assign  o_noport     = inoport ;

endmodule
