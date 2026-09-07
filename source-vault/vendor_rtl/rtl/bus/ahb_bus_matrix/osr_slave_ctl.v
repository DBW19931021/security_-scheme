//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_slave_ctl #(
    parameter P_BUS_MATRIX_AWIDTH    =  32                 ,
    parameter P_BUS_MATRIX_DELAY_EN  =  1'b0                
)
(
    input  wire                            hclk            ,
    input  wire                            hresetn         ,
    input  wire                            i_hsels         ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]  i_haddrs        ,
    input  wire [1:0]                      i_htranss       ,
    input  wire                            i_hwrites       ,
    input  wire [2:0]                      i_hsizes        ,
    input  wire [2:0]                      i_hbursts       ,
    input  wire [3:0]                      i_hprots        ,
    input  wire [7:0]                      i_hmasters      ,
    input  wire                            i_hmastlocks    ,
    input  wire                            i_hreadys       ,
    input  wire                            i_active        ,
    input  wire                            i_readyout      ,
    input  wire                            i_readyoutnd    ,
    input  wire [1:0]                      i_resp          ,

    output wire                            o_pendtran      ,

    output wire                            o_hreadyouts    ,
    output wire [1:0]                      o_hresps        ,
    output wire                            o_sel           ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]  o_addr          ,
    output wire [1:0]                      o_trans         ,
    output wire                            o_write         ,
    output wire [2:0]                      o_size          ,
    output wire [2:0]                      o_burst         ,
    output wire [3:0]                      o_prot          ,
    output wire [7:0]                      o_master        ,
    output wire                            o_mastlock      ,
    output wire                            o_heldtran
    );

localparam TRN_IDLE   = 2'b00    ;
localparam TRN_BUSY   = 2'b01    ;
localparam TRN_NONSEQ = 2'b10    ;
localparam TRN_SEQ    = 2'b11    ;

localparam SZ_BYTE    = 3'b000   ;
localparam SZ_HALF    = 3'b001   ;
localparam SZ_WORD    = 3'b010   ;

localparam BUR_SINGLE = 3'b000   ;
localparam BUR_INCR   = 3'b001   ;
localparam BUR_WRAP4  = 3'b010   ;
localparam BUR_INCR4  = 3'b011   ;
localparam BUR_WRAP8  = 3'b100   ;
localparam BUR_INCR8  = 3'b101   ;
localparam BUR_WRAP16 = 3'b110   ;
localparam BUR_INCR16 = 3'b111   ;

localparam RSP_OKAY   = 2'b00    ;
localparam RSP_ERROR  = 2'b01    ;
localparam RSP_RETRY  = 2'b10    ;
localparam RSP_SPLIT  = 2'b11    ;

wire                            loadreg;
wire                            pendtran;
reg                             pendtranreg;
wire                            addrvalid;
reg                             datavalid;
reg  [1:0]                      regtrans;
reg  [P_BUS_MATRIX_AWIDTH-1:0]  regaddr;
reg                             regwrite;
reg  [2:0]                      regsize;
reg  [2:0]                      regburst;
reg  [3:0]                      regprot;
reg  [7:0]                      regmaster;
reg                             regmastlock;
wire [1:0]                      transb;
wire [1:0]                      transint;
wire [2:0]                      burstint;
reg  [3:0]                      offsetaddr;
reg  [3:0]                      checkaddr;
reg                             burstoverride;
wire                            burstoverridenext;
reg                             bound;
wire                            boundnext;

wire                            seq_det_in;
reg                             seq_det;

wire [1:0]                      regtrans_in    = loadreg ? i_htranss    : regtrans;
wire [P_BUS_MATRIX_AWIDTH-1:0]  regaddr_in     = loadreg ? i_haddrs     : regaddr;
wire                            regwrite_in    = loadreg ? i_hwrites    : regwrite;
wire [2:0]                      regsize_in     = loadreg ? i_hsizes     : regsize;
wire [2:0]                      regburst_in    = loadreg ? i_hbursts    : regburst;
wire [3:0]                      regprot_in     = loadreg ? i_hprots     : regprot;
wire [7:0]                      regmaster_in   = loadreg ? i_hmasters   : regmaster;
wire                            regmastlock_in = loadreg ? i_hmastlocks : regmastlock;

assign addrvalid = (i_hsels & i_htranss[1]);

assign loadreg   = (addrvalid & i_hreadys); 

wire   datavalid_in = i_hreadys ? addrvalid : datavalid;

generate
    if(P_BUS_MATRIX_DELAY_EN)begin
        assign pendtran   = (loadreg & (!i_active)) ? 1'b1 :
                           ((i_active & i_readyout & ~i_resp[0]) ? 1'b0 : pendtranreg);
    end
    else begin
        assign pendtran   = (loadreg & (!i_active))     ? 1'b1 :
                            (loadreg & (!i_readyoutnd)) ? 1'b1 : 
                            ((i_active & i_readyout)    ? 1'b0 : pendtranreg);
    end
endgenerate

assign seq_det_in = pendtran & addrvalid & (i_htranss == 2'b11);

assign o_heldtran = (loadreg | pendtranreg);

assign o_sel      = pendtranreg ? 1'b1        : i_hsels;
assign o_addr     = pendtranreg ? regaddr     : i_haddrs;
assign o_write    = pendtranreg ? regwrite    : i_hwrites;
assign o_size     = pendtranreg ? regsize     : i_hsizes;
assign o_prot     = pendtranreg ? regprot     : i_hprots;
assign o_master   = pendtranreg ? regmaster   : i_hmasters;
assign o_mastlock = pendtranreg ? regmastlock : i_hmastlocks;
assign transint   = pendtranreg & i_resp[0]   ? 2'b0 : pendtranreg ? TRN_NONSEQ  : i_htranss;
assign burstint   = pendtranreg ? regburst    : i_hbursts;

assign transb     = pendtranreg ? regtrans    : i_htranss;

assign o_trans    = (burstoverride & bound) ? {transint[1], 1'b0} : transint;

assign o_burst    = (burstoverride & (transb != TRN_NONSEQ)) ? BUR_INCR : burstint;

assign o_pendtran = pendtranreg;

assign o_hreadyouts = ~datavalid ? 1'b1     : pendtranreg ? 1'b0     : i_readyout;

assign o_hresps     = (~datavalid | pendtranreg)          ? RSP_OKAY : i_resp;

assign burstoverridenext  = ((i_htranss == TRN_NONSEQ) |
                             (i_htranss == TRN_IDLE)) ? 1'b0 :
                           ((loadreg & (~i_active) &
                              (i_htranss ==TRN_SEQ)) ? 1'b1 :burstoverride);

wire  burstoverride_in = i_hreadys ? burstoverridenext : burstoverride;

always@(*)begin
    case(i_hsizes)
        3'b001 :offsetaddr = i_haddrs[4:1];
        3'b010 :offsetaddr = i_haddrs[5:2];
        3'b011 :offsetaddr = i_haddrs[6:3];
        default:offsetaddr = i_haddrs[3:0];
    endcase
end

always@(*)begin
    case(i_hbursts)
        BUR_WRAP4 :checkaddr = {2'b11,offsetaddr[1:0]};
        BUR_WRAP8 :checkaddr = {1'b1, offsetaddr[2:0]};
        BUR_WRAP16:checkaddr = offsetaddr[3:0];
        default   :checkaddr = 4'b0000;
    endcase
end

assign boundnext = (checkaddr == 4'b1111) ? 1'b1 : 1'b0;

always @(negedge hresetn or posedge hclk)
    if(~hresetn) begin
        regtrans      <= 2'b00;
        regaddr       <= {(P_BUS_MATRIX_AWIDTH){1'b0}};
        regwrite      <= 1'b0;
        regsize       <= 3'b000;
        regburst      <= 3'b000;
        regprot       <= {4{1'b0}};
        regmaster     <= 8'h0;
        regmastlock   <= 1'b0;
        datavalid     <= 1'b0;
        pendtranreg   <= 1'b0;
        burstoverride <= 1'b0;
        bound         <= 1'b0;
        seq_det       <= 1'b0;
    end
    else begin
        regtrans      <= regtrans_in;
        regaddr       <= regaddr_in;
        regwrite      <= regwrite_in;
        regsize       <= regsize_in;
        regburst      <= regburst_in;
        regprot       <= regprot_in;
        regmaster     <= regmaster_in;
        regmastlock   <= regmastlock_in;
        datavalid     <= datavalid_in;
        pendtranreg   <= pendtran;
        burstoverride <= burstoverride_in;
        bound         <= boundnext;
        seq_det       <= seq_det_in;
    end

endmodule
