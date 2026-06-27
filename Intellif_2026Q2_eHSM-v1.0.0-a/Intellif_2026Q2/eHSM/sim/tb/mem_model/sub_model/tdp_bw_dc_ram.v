//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module tdp_bw_dc_ram 
#(
    parameter   DW = 32       ,   // Data Width in bits
    parameter   AW = 10       ,   // addr width
    parameter   CW = 8        ,   // col width

    parameter   DP = 2**AW    ,   // depth 

    parameter   BW = DW/CW    ,   // byte-write enable width

    parameter   I0 = 0,
    parameter   I1 = 0,
    parameter   ID0 = "init.data",
    parameter   SA0 = 0          ,   
    parameter   ID1 = "init.data",
    parameter   SA1 = 0          ,
    parameter   HD = 0          // hold ouput when writing

    //----------------------------------------------------------------------
) (
    input wire                  CKA     ,
    input wire                  CSAN    ,
    input wire      [BW-1:0]    WEAN    ,
    input wire      [AW-1:0]    A       ,
    input wire      [DW-1:0]    DIA     ,
    output wire     [DW-1:0]    DOA     ,
    input wire                  CKB     ,
    input wire                  CSBN    ,
    input wire      [BW-1:0]    WEBN    ,
    input wire      [AW-1:0]    B       ,
    input wire      [DW-1:0]    DIB     ,
    output wire     [DW-1:0]    DOB        
);
//------------------------------------------------------------------------------
//reg declare
// Core Memory
reg [DW-1:0] ram [DP-1:0];

//==============================================================================
//1. All addressable words are initialized to the same value.
//initial begin for (i=0; i<DP; i=i+1) ram[i] = 0;
//end

//2. Use the file read function in the HDL source code to load 
//   the RAM initial contents from an external data file.
//initial begin
//    $readmemh("bit_init.data", ram, [start_address], [end_address]);
//end

integer i;

generate 

if(I0 == 1) begin : init_0
    initial begin for (i=0; i<DP; i=i+1) ram[i] = 0; end
end
else if(I1 == 1) begin : init_1
    wire [DW-1:0] idata = 0;
    wire [DW-1:0] iidata = ~idata;
    initial begin for (i=0; i<DP; i=i+1) ram[i] = iidata; end
end
else begin 
    if(ID0 != "init.data") begin : init0_data
        initial begin $readmemh(ID0, ram, SA0);end
    end

    if(ID1 != "init.data") begin : init1_data
        initial begin $readmemh(ID1, ram, SA1); end
    end
end

endgenerate 

//==============================================================================

reg  [DW-1:0]  DOAi;      
reg  [DW-1:0]  DOBi;      
//==============================================================================
//main logic
//------------------------------------------------------------------------------
// Port-A Operation
wire rena = (HD == 1'h1) ? (&WEAN) : 1'h1; 
always @ (posedge CKA) begin
    if(~CSAN) begin
        for(i=0;i<BW;i=i+1) begin
            if(~WEAN[i]) begin
                ram[A][i*CW +: CW] <= DIA[i*CW +: CW];
            end
        end
        DOAi <= rena ? ram[A] : DOAi;
    end
end

// Port-B Operation:
wire renb = (HD == 1'h1) ? (&WEBN) : 1'h1; 
always @ (posedge CKB) begin
    if(!CSBN) begin
        for(i=0;i<BW;i=i+1) begin
            if(~WEBN[i]) begin
                ram[B][i*CW +: CW] <= DIB[i*CW +: CW];
            end
        end
        DOBi <= renb ? ram[B] : DOBi;
    end
end

reg do_valid;
always @(posedge CKA) begin
    do_valid <= ~CSAN ;
end
assign DOA = do_valid ? DOAi : 'hx;

reg do_validb;
always @(posedge CKB) begin
    do_validb <= ~CSBN ;
end
assign DOB = do_validb ? DOBi : 'hx;

endmodule
//==============================================================================
