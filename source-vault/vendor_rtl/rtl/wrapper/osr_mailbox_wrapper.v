//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.1.0-a
// 
// 4019 4_1_0_dev_Intellifusion ac64bbf3e97113d575f8d196a8a63253d369e0a9
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_mailbox_wrapper #(
    parameter           P_S2H_NOTE_N  = 1          ,   
    parameter           P_H2S_NOTE_N  = 1          ,   
    parameter           P_MBOX_NUM    = 16         ,   
    parameter           P_HAVE_FUSA   = 1              

)(

    input  wire         i_hclk          ,
    input  wire         i_hreset_n      ,
    input  wire         i_hsm_clk_en    ,

    input  wire         i_se_hsel       ,
    input  wire [31:0]  i_se_haddr      ,
    input  wire [1:0]   i_se_htrans     ,
    input  wire         i_se_hwrite     ,
    input  wire [2:0]   i_se_hsize      ,
    input  wire [31:0]  i_se_hwdata     ,
    input  wire         i_se_hready     ,
    input  wire [2:0]   i_se_hburst     ,    
    input  wire [3:0]   i_se_hprot      ,
    input  wire         i_se_hmastlock  ,

    output wire [31:0]  o_se_hrdata     ,
    output wire [1:0]   o_se_hresp      ,
    output wire         o_se_hreadyout  ,

    input  wire [63:0]  i_hsm_status    ,

    output wire         o_se_irq        ,

    input  wire         i_soc_hsel      , 
    input  wire [31:0]  i_soc_haddr     ,
    input  wire [1:0]   i_soc_htrans    ,
    input  wire         i_soc_hwrite    ,
    input  wire [2:0]   i_soc_hsize     ,
    input  wire [31:0]  i_soc_hwdata    ,
    input  wire         i_soc_hready    ,
    input  wire [2:0]   i_soc_hburst    ,    
    input  wire [3:0]   i_soc_hprot     ,
    input  wire         i_soc_hmastlock ,

    output wire [31:0]  o_soc_hrdata    ,
    output wire [1:0]   o_soc_hresp     ,
    output wire         o_soc_hreadyout ,

    output wire [15:0]  o_soc_irq       

);

localparam A1P_MBOX_NUM       = (P_MBOX_NUM      > 0) ? P_MBOX_NUM       : 1 ;

localparam     MBX_P_S2H_NOTE_N = P_S2H_NOTE_N ;
localparam     MBX_P_H2S_NOTE_N = P_H2S_NOTE_N ;
localparam     MBX_P_HAVE_FUSA  = 1            ;

wire  [15:0]  se_irq ;
wire  [15:0]  soc_irq ;

wire [31:0]    soc_hrdata[0:15];     
wire           soc_hreadyout[0:15];  
wire [1:0]     soc_hresp[0:15] ; 

wire [31:0]    hsm_hrdata[0:15];     
wire           hsm_hreadyout[0:15];  
wire [1:0]     hsm_hresp[0:15] ;  

wire           sdec_o_hsel0  ;
wire           sdec_o_hsel1  ;
wire           sdec_o_hsel2  ;
wire           sdec_o_hsel3  ;
wire           sdec_o_hsel4  ;
wire           sdec_o_hsel5  ;
wire           sdec_o_hsel6  ;
wire           sdec_o_hsel7  ;
wire           sdec_o_hsel8  ;
wire           sdec_o_hsel9  ;
wire           sdec_o_hsel10 ;
wire           sdec_o_hsel11 ;
wire           sdec_o_hsel12 ;
wire           sdec_o_hsel13 ;
wire           sdec_o_hsel14 ;
wire           sdec_o_hsel15 ;

wire           hdec_o_hsel0  ;
wire           hdec_o_hsel1  ;
wire           hdec_o_hsel2  ;
wire           hdec_o_hsel3  ;
wire           hdec_o_hsel4  ;
wire           hdec_o_hsel5  ;
wire           hdec_o_hsel6  ;
wire           hdec_o_hsel7  ;
wire           hdec_o_hsel8  ;
wire           hdec_o_hsel9  ;
wire           hdec_o_hsel10 ;
wire           hdec_o_hsel11 ;
wire           hdec_o_hsel12 ;
wire           hdec_o_hsel13 ;
wire           hdec_o_hsel14 ;
wire           hdec_o_hsel15 ;

wire [31:0]    soc_o_hrdata      ;
wire           soc_o_hready      ;
wire [1:0]     soc_o_hresp       ;

wire [31:0]    hsm_o_hrdata      ;
wire           hsm_o_hready      ;
wire [1:0]     hsm_o_hresp       ;

wire [A1P_MBOX_NUM*(31+1)-1:0] mbx_o_se_hrdata     ;
wire [A1P_MBOX_NUM*(1+1)-1:0]  mbx_o_se_hresp      ;
wire [A1P_MBOX_NUM-1:0]        mbx_o_se_hreadyout  ;
wire [A1P_MBOX_NUM-1:0]        mbx_o_se_irq        ;
wire [A1P_MBOX_NUM*(31+1)-1:0] mbx_o_soc_hrdata    ;
wire [A1P_MBOX_NUM*(1+1)-1:0]  mbx_o_soc_hresp     ;
wire [A1P_MBOX_NUM-1:0]        mbx_o_soc_hreadyout ;
wire [A1P_MBOX_NUM-1:0]        mbx_o_soc_irq       ;

wire [15:0] mbx_se_hsel  = {hdec_o_hsel15,hdec_o_hsel14,hdec_o_hsel13,hdec_o_hsel12,
                            hdec_o_hsel11,hdec_o_hsel10,hdec_o_hsel9, hdec_o_hsel8 ,
                            hdec_o_hsel7, hdec_o_hsel6, hdec_o_hsel5, hdec_o_hsel4 ,
                            hdec_o_hsel3, hdec_o_hsel2, hdec_o_hsel1, hdec_o_hsel0 };

wire [15:0] mbx_soc_hsel = {sdec_o_hsel15,sdec_o_hsel14,sdec_o_hsel13,sdec_o_hsel12,
                            sdec_o_hsel11,sdec_o_hsel10,sdec_o_hsel9, sdec_o_hsel8 ,
                            sdec_o_hsel7, sdec_o_hsel6, sdec_o_hsel5, sdec_o_hsel4 ,
                            sdec_o_hsel3, sdec_o_hsel2, sdec_o_hsel1, sdec_o_hsel0 };

wire [31:0] se_irq_all  ; 
wire [31:0] soc_irq_all ;

wire           sdec_i_hsel   = i_soc_hsel  ; 
wire [31:0]    sdec_i_haddr  = i_soc_haddr ;

wire           hdec_i_hsel   = i_se_hsel   ;
wire [31:0]    hdec_i_haddr  = i_se_haddr  ;

wire           soc_hclk          = i_hclk    ;
wire           soc_hresetn       = i_hreset_n ;
wire           soc_i_clk_en      = 1'h1          ; 
wire           soc_i_hsel0       = sdec_o_hsel0  ; 
wire           soc_i_hsel1       = sdec_o_hsel1  ; 
wire           soc_i_hsel2       = sdec_o_hsel2  ; 
wire           soc_i_hsel3       = sdec_o_hsel3  ; 
wire           soc_i_hsel4       = sdec_o_hsel4  ; 
wire           soc_i_hsel5       = sdec_o_hsel5  ; 
wire           soc_i_hsel6       = sdec_o_hsel6  ; 
wire           soc_i_hsel7       = sdec_o_hsel7  ; 
wire           soc_i_hsel8       = sdec_o_hsel8  ; 
wire           soc_i_hsel9       = sdec_o_hsel9  ; 
wire           soc_i_hsel10      = sdec_o_hsel10 ; 
wire           soc_i_hsel11      = sdec_o_hsel11 ; 
wire           soc_i_hsel12      = sdec_o_hsel12 ; 
wire           soc_i_hsel13      = sdec_o_hsel13 ; 
wire           soc_i_hsel14      = sdec_o_hsel14 ; 
wire           soc_i_hsel15      = sdec_o_hsel15 ; 
wire [31:0]    soc_i_hrdata0     = soc_hrdata[0]    ;
wire           soc_i_hreadyout0  = soc_hreadyout[0] ;
wire [1:0]     soc_i_hresp0      = soc_hresp[0]     ;
wire [31:0]    soc_i_hrdata1     = soc_hrdata[1]    ;
wire           soc_i_hreadyout1  = soc_hreadyout[1] ;
wire [1:0]     soc_i_hresp1      = soc_hresp[1]     ;
wire [31:0]    soc_i_hrdata2     = soc_hrdata[2]    ;
wire           soc_i_hreadyout2  = soc_hreadyout[2] ;
wire [1:0]     soc_i_hresp2      = soc_hresp[2]     ;
wire [31:0]    soc_i_hrdata3     = soc_hrdata[3]    ;
wire           soc_i_hreadyout3  = soc_hreadyout[3] ;
wire [1:0]     soc_i_hresp3      = soc_hresp[3]     ;
wire [31:0]    soc_i_hrdata4     = soc_hrdata[4]    ;
wire           soc_i_hreadyout4  = soc_hreadyout[4] ;
wire [1:0]     soc_i_hresp4      = soc_hresp[4]     ;
wire [31:0]    soc_i_hrdata5     = soc_hrdata[5]    ;
wire           soc_i_hreadyout5  = soc_hreadyout[5] ;
wire [1:0]     soc_i_hresp5      = soc_hresp[5]     ;
wire [31:0]    soc_i_hrdata6     = soc_hrdata[6]    ;
wire           soc_i_hreadyout6  = soc_hreadyout[6] ;
wire [1:0]     soc_i_hresp6      = soc_hresp[6]     ;
wire [31:0]    soc_i_hrdata7     = soc_hrdata[7]    ;
wire           soc_i_hreadyout7  = soc_hreadyout[7] ;
wire [1:0]     soc_i_hresp7      = soc_hresp[7]     ;
wire [31:0]    soc_i_hrdata8     = soc_hrdata[8]    ;
wire           soc_i_hreadyout8  = soc_hreadyout[8] ;
wire [1:0]     soc_i_hresp8      = soc_hresp[8]     ;
wire [31:0]    soc_i_hrdata9     = soc_hrdata[9]    ;
wire           soc_i_hreadyout9  = soc_hreadyout[9] ;
wire [1:0]     soc_i_hresp9      = soc_hresp[9]     ;
wire [31:0]    soc_i_hrdata10    = soc_hrdata[10]    ;
wire           soc_i_hreadyout10 = soc_hreadyout[10] ;
wire [1:0]     soc_i_hresp10     = soc_hresp[10]     ;
wire [31:0]    soc_i_hrdata11    = soc_hrdata[11]    ;
wire           soc_i_hreadyout11 = soc_hreadyout[11] ;
wire [1:0]     soc_i_hresp11     = soc_hresp[11]     ;
wire [31:0]    soc_i_hrdata12    = soc_hrdata[12]    ;
wire           soc_i_hreadyout12 = soc_hreadyout[12] ;
wire [1:0]     soc_i_hresp12     = soc_hresp[12]     ;
wire [31:0]    soc_i_hrdata13    = soc_hrdata[13]    ;
wire           soc_i_hreadyout13 = soc_hreadyout[13] ;
wire [1:0]     soc_i_hresp13     = soc_hresp[13]     ;
wire [31:0]    soc_i_hrdata14    = soc_hrdata[14]    ;
wire           soc_i_hreadyout14 = soc_hreadyout[14] ;
wire [1:0]     soc_i_hresp14     = soc_hresp[14]     ;
wire [31:0]    soc_i_hrdata15    = soc_hrdata[15]    ;
wire           soc_i_hreadyout15 = soc_hreadyout[15] ;
wire [1:0]     soc_i_hresp15     = soc_hresp[15]     ;

wire           hsm_hclk          = i_hclk    ; 
wire           hsm_hresetn       = i_hreset_n ;
wire           hsm_i_clk_en      = i_hsm_clk_en  ;
wire           hsm_i_hsel0       = hdec_o_hsel0  ; 
wire           hsm_i_hsel1       = hdec_o_hsel1  ; 
wire           hsm_i_hsel2       = hdec_o_hsel2  ; 
wire           hsm_i_hsel3       = hdec_o_hsel3  ; 
wire           hsm_i_hsel4       = hdec_o_hsel4  ; 
wire           hsm_i_hsel5       = hdec_o_hsel5  ; 
wire           hsm_i_hsel6       = hdec_o_hsel6  ; 
wire           hsm_i_hsel7       = hdec_o_hsel7  ; 
wire           hsm_i_hsel8       = hdec_o_hsel8  ; 
wire           hsm_i_hsel9       = hdec_o_hsel9  ; 
wire           hsm_i_hsel10      = hdec_o_hsel10 ; 
wire           hsm_i_hsel11      = hdec_o_hsel11 ; 
wire           hsm_i_hsel12      = hdec_o_hsel12 ; 
wire           hsm_i_hsel13      = hdec_o_hsel13 ; 
wire           hsm_i_hsel14      = hdec_o_hsel14 ; 
wire           hsm_i_hsel15      = hdec_o_hsel15 ; 
wire [31:0]    hsm_i_hrdata0     = hsm_hrdata[0]    ;
wire           hsm_i_hreadyout0  = hsm_hreadyout[0] ;
wire [1:0]     hsm_i_hresp0      = hsm_hresp[0]     ;
wire [31:0]    hsm_i_hrdata1     = hsm_hrdata[1]    ;
wire           hsm_i_hreadyout1  = hsm_hreadyout[1] ;
wire [1:0]     hsm_i_hresp1      = hsm_hresp[1]     ;
wire [31:0]    hsm_i_hrdata2     = hsm_hrdata[2]    ;
wire           hsm_i_hreadyout2  = hsm_hreadyout[2] ;
wire [1:0]     hsm_i_hresp2      = hsm_hresp[2]     ;
wire [31:0]    hsm_i_hrdata3     = hsm_hrdata[3]    ;
wire           hsm_i_hreadyout3  = hsm_hreadyout[3] ;
wire [1:0]     hsm_i_hresp3      = hsm_hresp[3]     ;
wire [31:0]    hsm_i_hrdata4     = hsm_hrdata[4]    ;
wire           hsm_i_hreadyout4  = hsm_hreadyout[4] ;
wire [1:0]     hsm_i_hresp4      = hsm_hresp[4]     ;
wire [31:0]    hsm_i_hrdata5     = hsm_hrdata[5]    ;
wire           hsm_i_hreadyout5  = hsm_hreadyout[5] ;
wire [1:0]     hsm_i_hresp5      = hsm_hresp[5]     ;
wire [31:0]    hsm_i_hrdata6     = hsm_hrdata[6]    ;
wire           hsm_i_hreadyout6  = hsm_hreadyout[6] ;
wire [1:0]     hsm_i_hresp6      = hsm_hresp[6]     ;
wire [31:0]    hsm_i_hrdata7     = hsm_hrdata[7]    ;
wire           hsm_i_hreadyout7  = hsm_hreadyout[7] ;
wire [1:0]     hsm_i_hresp7      = hsm_hresp[7]     ;
wire [31:0]    hsm_i_hrdata8     = hsm_hrdata[8]    ;
wire           hsm_i_hreadyout8  = hsm_hreadyout[8] ;
wire [1:0]     hsm_i_hresp8      = hsm_hresp[8]     ;
wire [31:0]    hsm_i_hrdata9     = hsm_hrdata[9]    ;
wire           hsm_i_hreadyout9  = hsm_hreadyout[9] ;
wire [1:0]     hsm_i_hresp9      = hsm_hresp[9]     ;
wire [31:0]    hsm_i_hrdata10    = hsm_hrdata[10]    ;
wire           hsm_i_hreadyout10 = hsm_hreadyout[10] ;
wire [1:0]     hsm_i_hresp10     = hsm_hresp[10]     ;
wire [31:0]    hsm_i_hrdata11    = hsm_hrdata[11]    ;
wire           hsm_i_hreadyout11 = hsm_hreadyout[11] ;
wire [1:0]     hsm_i_hresp11     = hsm_hresp[11]     ;
wire [31:0]    hsm_i_hrdata12    = hsm_hrdata[12]    ;
wire           hsm_i_hreadyout12 = hsm_hreadyout[12] ;
wire [1:0]     hsm_i_hresp12     = hsm_hresp[12]     ;
wire [31:0]    hsm_i_hrdata13    = hsm_hrdata[13]    ;
wire           hsm_i_hreadyout13 = hsm_hreadyout[13] ;
wire [1:0]     hsm_i_hresp13     = hsm_hresp[13]     ;
wire [31:0]    hsm_i_hrdata14    = hsm_hrdata[14]    ;
wire           hsm_i_hreadyout14 = hsm_hreadyout[14] ;
wire [1:0]     hsm_i_hresp14     = hsm_hresp[14]     ;
wire [31:0]    hsm_i_hrdata15    = hsm_hrdata[15]    ;
wire           hsm_i_hreadyout15 = hsm_hreadyout[15] ;
wire [1:0]     hsm_i_hresp15     = hsm_hresp[15]     ;

wire [A1P_MBOX_NUM-1:0]        mbx_i_hclk          = {A1P_MBOX_NUM{i_hclk}}       ; 
wire [A1P_MBOX_NUM-1:0]        mbx_i_hreset_n      = {A1P_MBOX_NUM{i_hreset_n}}   ; 
wire [A1P_MBOX_NUM-1:0]        mbx_i_hsm_clk_en    = {A1P_MBOX_NUM{i_hsm_clk_en}} ;
wire [A1P_MBOX_NUM-1:0]        mbx_i_se_hsel       = mbx_se_hsel[A1P_MBOX_NUM-1:0];
wire [A1P_MBOX_NUM*(11+1)-1:0] mbx_i_se_haddr      = {A1P_MBOX_NUM{i_se_haddr[11:0]}} ;
wire [A1P_MBOX_NUM*(1+1)-1:0]  mbx_i_se_htrans     = {A1P_MBOX_NUM{i_se_htrans[1:0]}};
wire [A1P_MBOX_NUM-1:0]        mbx_i_se_hwrite     = {A1P_MBOX_NUM{i_se_hwrite}};
wire [A1P_MBOX_NUM*(2+1)-1:0]  mbx_i_se_hsize      = {A1P_MBOX_NUM{i_se_hsize}} ;
wire [A1P_MBOX_NUM*(31+1)-1:0] mbx_i_se_hwdata     = {A1P_MBOX_NUM{i_se_hwdata[31:0]}};
wire [A1P_MBOX_NUM-1:0]        mbx_i_se_hready     = {A1P_MBOX_NUM{i_se_hready}};
wire [A1P_MBOX_NUM*(2+1)-1:0]  mbx_i_se_hburst     = {A1P_MBOX_NUM{i_se_hburst}};
wire [A1P_MBOX_NUM*(3+1)-1:0]  mbx_i_se_hprot      = {A1P_MBOX_NUM{i_se_hprot}} ;
wire [A1P_MBOX_NUM-1:0]        mbx_i_se_hmastlock  = {A1P_MBOX_NUM{i_se_hmastlock}};
wire [A1P_MBOX_NUM*(63+1)-1:0] mbx_i_hsm_status    = {A1P_MBOX_NUM{i_hsm_status}};
wire [A1P_MBOX_NUM*(31+1)-1:0] mbx_i_se_irq_all    = {A1P_MBOX_NUM{se_irq_all}} ;
wire [A1P_MBOX_NUM*(63+1)-1:0] mbx_i_hsm_cfg       = {64'h15,64'h14,64'h13,64'h12,64'h11,64'h10,64'h9,64'h8,64'h7,64'h6,64'h5,64'h4,64'h3,64'h2,64'h1,64'h0} ;
wire [A1P_MBOX_NUM-1:0]        mbx_i_soc_hsel      = mbx_soc_hsel[A1P_MBOX_NUM-1:0];
wire [A1P_MBOX_NUM*(11+1)-1:0] mbx_i_soc_haddr     = {A1P_MBOX_NUM{i_soc_haddr[11:0]}} ;
wire [A1P_MBOX_NUM*(1+1)-1:0]  mbx_i_soc_htrans    = {A1P_MBOX_NUM{i_soc_htrans}};
wire [A1P_MBOX_NUM-1:0]        mbx_i_soc_hwrite    = {A1P_MBOX_NUM{i_soc_hwrite}}; 
wire [A1P_MBOX_NUM*(2+1)-1:0]  mbx_i_soc_hsize     = {A1P_MBOX_NUM{i_soc_hsize}} ;
wire [A1P_MBOX_NUM*(31+1)-1:0] mbx_i_soc_hwdata    = {A1P_MBOX_NUM{i_soc_hwdata[31:0]}};
wire [A1P_MBOX_NUM-1:0]        mbx_i_soc_hready    = {A1P_MBOX_NUM{i_soc_hready}};
wire [A1P_MBOX_NUM*(2+1)-1:0]  mbx_i_soc_hburst    = {A1P_MBOX_NUM{i_soc_hburst}};
wire [A1P_MBOX_NUM*(3+1)-1:0]  mbx_i_soc_hprot     = {A1P_MBOX_NUM{i_soc_hprot}} ;
wire [A1P_MBOX_NUM-1:0]        mbx_i_soc_hmastlock = {A1P_MBOX_NUM{i_soc_hmastlock}};

wire [A1P_MBOX_NUM*(31+1)-1:0] mbx_i_soc_irq_all   = {A1P_MBOX_NUM{soc_irq_all}};

genvar i;
generate
for(i = 0; i < P_MBOX_NUM; i = i + 1) begin: g_se_irq
    assign se_irq[i] = mbx_o_se_irq[i] ; 
end
endgenerate

generate
for(i = 0; i < P_MBOX_NUM; i = i + 1) begin: g_soc_irq
    assign soc_irq[i] = mbx_o_soc_irq[i] ; 
end
endgenerate

generate
for(i = 0; i < P_MBOX_NUM; i = i + 1) begin: g_soc_hslave_out
    assign soc_hrdata[i]    = {mbx_o_soc_hrdata[i*32 +: 32]} ; 
    assign soc_hreadyout[i] = mbx_o_soc_hreadyout[i]              ;
    assign soc_hresp[i]     = mbx_o_soc_hresp[i*2 +: 2]           ;
end
endgenerate

generate
for(i = 0; i < P_MBOX_NUM; i = i + 1) begin: g_hsm_hslave_out
    assign hsm_hrdata[i]    = {mbx_o_se_hrdata[i*32 +: 32]} ; 
    assign hsm_hreadyout[i] = mbx_o_se_hreadyout[i]              ;
    assign hsm_hresp[i]     = mbx_o_se_hresp[i*2 +: 2]           ;
end
endgenerate

generate
for(i = P_MBOX_NUM; i < 16; i = i + 1) begin: g_se_irq_none
    assign se_irq[i] = 1'h0 ;
end
endgenerate

generate
for(i = P_MBOX_NUM; i < 16; i = i + 1) begin: g_soc_irq_none
    assign soc_irq[i] = 1'h0 ;
end
endgenerate

generate
for(i = P_MBOX_NUM; i < 16; i = i + 1) begin: g_soc_hslave_out_none
    assign soc_hrdata[i]    = 32'h0 ;
    assign soc_hreadyout[i] = 1'h1  ;
    assign soc_hresp[i]     = 2'h0  ;
end
endgenerate

generate
for(i = P_MBOX_NUM; i < 16; i = i + 1) begin: g_hsm_hslave_out_none
    assign hsm_hrdata[i]    = 32'h0 ;
    assign hsm_hreadyout[i] = 1'h1  ;
    assign hsm_hresp[i]     = 2'h0  ;
end
endgenerate

assign se_irq_all  = {16'h0,se_irq };
assign soc_irq_all = {16'h0,soc_irq};

assign o_se_hrdata          = hsm_o_hrdata ; 
assign o_se_hresp           = hsm_o_hresp  ;
assign o_se_hreadyout       = hsm_o_hready ;
assign o_se_irq             = |se_irq[15:0];       
assign o_soc_hrdata         = soc_o_hrdata[31:0] ;
assign o_soc_hresp          = soc_o_hresp  ;
assign o_soc_hreadyout      = soc_o_hready ;
assign o_soc_irq            = soc_irq[15:0];

osr_mbox_decoder u_mbox_decoder_soc (
    .i_hsel       ( sdec_i_hsel   ), 
    .i_haddr      ( sdec_i_haddr  ), 
    .o_hsel0      ( sdec_o_hsel0  ), 
    .o_hsel1      ( sdec_o_hsel1  ), 
    .o_hsel2      ( sdec_o_hsel2  ), 
    .o_hsel3      ( sdec_o_hsel3  ), 
    .o_hsel4      ( sdec_o_hsel4  ), 
    .o_hsel5      ( sdec_o_hsel5  ), 
    .o_hsel6      ( sdec_o_hsel6  ), 
    .o_hsel7      ( sdec_o_hsel7  ), 
    .o_hsel8      ( sdec_o_hsel8  ), 
    .o_hsel9      ( sdec_o_hsel9  ), 
    .o_hsel10     ( sdec_o_hsel10 ), 
    .o_hsel11     ( sdec_o_hsel11 ), 
    .o_hsel12     ( sdec_o_hsel12 ), 
    .o_hsel13     ( sdec_o_hsel13 ), 
    .o_hsel14     ( sdec_o_hsel14 ), 
    .o_hsel15     ( sdec_o_hsel15 )  
    ); 

osr_mbox_decoder u_mbox_decoder_hsm (
    .i_hsel       ( hdec_i_hsel   ), 
    .i_haddr      ( hdec_i_haddr  ), 
    .o_hsel0      ( hdec_o_hsel0  ), 
    .o_hsel1      ( hdec_o_hsel1  ), 
    .o_hsel2      ( hdec_o_hsel2  ), 
    .o_hsel3      ( hdec_o_hsel3  ), 
    .o_hsel4      ( hdec_o_hsel4  ), 
    .o_hsel5      ( hdec_o_hsel5  ), 
    .o_hsel6      ( hdec_o_hsel6  ), 
    .o_hsel7      ( hdec_o_hsel7  ), 
    .o_hsel8      ( hdec_o_hsel8  ), 
    .o_hsel9      ( hdec_o_hsel9  ), 
    .o_hsel10     ( hdec_o_hsel10 ), 
    .o_hsel11     ( hdec_o_hsel11 ), 
    .o_hsel12     ( hdec_o_hsel12 ), 
    .o_hsel13     ( hdec_o_hsel13 ), 
    .o_hsel14     ( hdec_o_hsel14 ), 
    .o_hsel15     ( hdec_o_hsel15 )  
    ); 

osr_ahb_bus_muxs2m u_ahb_bus_muxs2m_soc (
    .hclk             ( soc_hclk          ), 
    .hresetn          ( soc_hresetn       ), 
    .i_clk_en         ( soc_i_clk_en      ), 
    .i_hsel0          ( soc_i_hsel0       ), 
    .i_hsel1          ( soc_i_hsel1       ), 
    .i_hsel2          ( soc_i_hsel2       ), 
    .i_hsel3          ( soc_i_hsel3       ), 
    .i_hsel4          ( soc_i_hsel4       ), 
    .i_hsel5          ( soc_i_hsel5       ), 
    .i_hsel6          ( soc_i_hsel6       ), 
    .i_hsel7          ( soc_i_hsel7       ), 
    .i_hsel8          ( soc_i_hsel8       ), 
    .i_hsel9          ( soc_i_hsel9       ), 
    .i_hsel10         ( soc_i_hsel10      ), 
    .i_hsel11         ( soc_i_hsel11      ), 
    .i_hsel12         ( soc_i_hsel12      ), 
    .i_hsel13         ( soc_i_hsel13      ), 
    .i_hsel14         ( soc_i_hsel14      ), 
    .i_hsel15         ( soc_i_hsel15      ), 
    .i_hrdata0        ( soc_i_hrdata0     ), 
    .i_hreadyout0     ( soc_i_hreadyout0  ), 
    .i_hresp0         ( soc_i_hresp0      ), 
    .i_hrdata1        ( soc_i_hrdata1     ), 
    .i_hreadyout1     ( soc_i_hreadyout1  ), 
    .i_hresp1         ( soc_i_hresp1      ), 
    .i_hrdata2        ( soc_i_hrdata2     ), 
    .i_hreadyout2     ( soc_i_hreadyout2  ), 
    .i_hresp2         ( soc_i_hresp2      ), 
    .i_hrdata3        ( soc_i_hrdata3     ), 
    .i_hreadyout3     ( soc_i_hreadyout3  ), 
    .i_hresp3         ( soc_i_hresp3      ), 
    .i_hrdata4        ( soc_i_hrdata4     ), 
    .i_hreadyout4     ( soc_i_hreadyout4  ), 
    .i_hresp4         ( soc_i_hresp4      ), 
    .i_hrdata5        ( soc_i_hrdata5     ), 
    .i_hreadyout5     ( soc_i_hreadyout5  ), 
    .i_hresp5         ( soc_i_hresp5      ), 
    .i_hrdata6        ( soc_i_hrdata6     ), 
    .i_hreadyout6     ( soc_i_hreadyout6  ), 
    .i_hresp6         ( soc_i_hresp6      ), 
    .i_hrdata7        ( soc_i_hrdata7     ), 
    .i_hreadyout7     ( soc_i_hreadyout7  ), 
    .i_hresp7         ( soc_i_hresp7      ), 
    .i_hrdata8        ( soc_i_hrdata8     ), 
    .i_hreadyout8     ( soc_i_hreadyout8  ), 
    .i_hresp8         ( soc_i_hresp8      ), 
    .i_hrdata9        ( soc_i_hrdata9     ), 
    .i_hreadyout9     ( soc_i_hreadyout9  ), 
    .i_hresp9         ( soc_i_hresp9      ), 
    .i_hrdata10       ( soc_i_hrdata10    ), 
    .i_hreadyout10    ( soc_i_hreadyout10 ), 
    .i_hresp10        ( soc_i_hresp10     ), 
    .i_hrdata11       ( soc_i_hrdata11    ), 
    .i_hreadyout11    ( soc_i_hreadyout11 ), 
    .i_hresp11        ( soc_i_hresp11     ), 
    .i_hrdata12       ( soc_i_hrdata12    ), 
    .i_hreadyout12    ( soc_i_hreadyout12 ), 
    .i_hresp12        ( soc_i_hresp12     ), 
    .i_hrdata13       ( soc_i_hrdata13    ), 
    .i_hreadyout13    ( soc_i_hreadyout13 ), 
    .i_hresp13        ( soc_i_hresp13     ), 
    .i_hrdata14       ( soc_i_hrdata14    ), 
    .i_hreadyout14    ( soc_i_hreadyout14 ), 
    .i_hresp14        ( soc_i_hresp14     ), 
    .i_hrdata15       ( soc_i_hrdata15    ), 
    .i_hreadyout15    ( soc_i_hreadyout15 ), 
    .i_hresp15        ( soc_i_hresp15     ), 
    .o_hrdata         ( soc_o_hrdata      ), 
    .o_hready         ( soc_o_hready      ), 
    .o_hresp          ( soc_o_hresp       )  
    ); 

osr_ahb_bus_muxs2m u_ahb_bus_muxs2m_hsm (
    .hclk             ( hsm_hclk          ), 
    .hresetn          ( hsm_hresetn       ), 
    .i_clk_en         ( hsm_i_clk_en      ), 
    .i_hsel0          ( hsm_i_hsel0       ), 
    .i_hsel1          ( hsm_i_hsel1       ), 
    .i_hsel2          ( hsm_i_hsel2       ), 
    .i_hsel3          ( hsm_i_hsel3       ), 
    .i_hsel4          ( hsm_i_hsel4       ), 
    .i_hsel5          ( hsm_i_hsel5       ), 
    .i_hsel6          ( hsm_i_hsel6       ), 
    .i_hsel7          ( hsm_i_hsel7       ), 
    .i_hsel8          ( hsm_i_hsel8       ), 
    .i_hsel9          ( hsm_i_hsel9       ), 
    .i_hsel10         ( hsm_i_hsel10      ), 
    .i_hsel11         ( hsm_i_hsel11      ), 
    .i_hsel12         ( hsm_i_hsel12      ), 
    .i_hsel13         ( hsm_i_hsel13      ), 
    .i_hsel14         ( hsm_i_hsel14      ), 
    .i_hsel15         ( hsm_i_hsel15      ), 
    .i_hrdata0        ( hsm_i_hrdata0     ), 
    .i_hreadyout0     ( hsm_i_hreadyout0  ), 
    .i_hresp0         ( hsm_i_hresp0      ), 
    .i_hrdata1        ( hsm_i_hrdata1     ), 
    .i_hreadyout1     ( hsm_i_hreadyout1  ), 
    .i_hresp1         ( hsm_i_hresp1      ), 
    .i_hrdata2        ( hsm_i_hrdata2     ), 
    .i_hreadyout2     ( hsm_i_hreadyout2  ), 
    .i_hresp2         ( hsm_i_hresp2      ), 
    .i_hrdata3        ( hsm_i_hrdata3     ), 
    .i_hreadyout3     ( hsm_i_hreadyout3  ), 
    .i_hresp3         ( hsm_i_hresp3      ), 
    .i_hrdata4        ( hsm_i_hrdata4     ), 
    .i_hreadyout4     ( hsm_i_hreadyout4  ), 
    .i_hresp4         ( hsm_i_hresp4      ), 
    .i_hrdata5        ( hsm_i_hrdata5     ), 
    .i_hreadyout5     ( hsm_i_hreadyout5  ), 
    .i_hresp5         ( hsm_i_hresp5      ), 
    .i_hrdata6        ( hsm_i_hrdata6     ), 
    .i_hreadyout6     ( hsm_i_hreadyout6  ), 
    .i_hresp6         ( hsm_i_hresp6      ), 
    .i_hrdata7        ( hsm_i_hrdata7     ), 
    .i_hreadyout7     ( hsm_i_hreadyout7  ), 
    .i_hresp7         ( hsm_i_hresp7      ), 
    .i_hrdata8        ( hsm_i_hrdata8     ), 
    .i_hreadyout8     ( hsm_i_hreadyout8  ), 
    .i_hresp8         ( hsm_i_hresp8      ), 
    .i_hrdata9        ( hsm_i_hrdata9     ), 
    .i_hreadyout9     ( hsm_i_hreadyout9  ), 
    .i_hresp9         ( hsm_i_hresp9      ), 
    .i_hrdata10       ( hsm_i_hrdata10    ), 
    .i_hreadyout10    ( hsm_i_hreadyout10 ), 
    .i_hresp10        ( hsm_i_hresp10     ), 
    .i_hrdata11       ( hsm_i_hrdata11    ), 
    .i_hreadyout11    ( hsm_i_hreadyout11 ), 
    .i_hresp11        ( hsm_i_hresp11     ), 
    .i_hrdata12       ( hsm_i_hrdata12    ), 
    .i_hreadyout12    ( hsm_i_hreadyout12 ), 
    .i_hresp12        ( hsm_i_hresp12     ), 
    .i_hrdata13       ( hsm_i_hrdata13    ), 
    .i_hreadyout13    ( hsm_i_hreadyout13 ), 
    .i_hresp13        ( hsm_i_hresp13     ), 
    .i_hrdata14       ( hsm_i_hrdata14    ), 
    .i_hreadyout14    ( hsm_i_hreadyout14 ), 
    .i_hresp14        ( hsm_i_hresp14     ), 
    .i_hrdata15       ( hsm_i_hrdata15    ), 
    .i_hreadyout15    ( hsm_i_hreadyout15 ), 
    .i_hresp15        ( hsm_i_hresp15     ), 
    .o_hrdata         ( hsm_o_hrdata      ), 
    .o_hready         ( hsm_o_hready      ), 
    .o_hresp          ( hsm_o_hresp       )  
    ); 

genvar gi0;
generate
for(gi0 = 0; gi0 < P_MBOX_NUM; gi0 = gi0 + 1) begin : g_mbox
osr_mailbox #(
    .P_S2H_NOTE_N     ( MBX_P_S2H_NOTE_N ), 
    .P_H2S_NOTE_N     ( MBX_P_H2S_NOTE_N ), 
    .P_HAVE_FUSA      ( MBX_P_HAVE_FUSA  )  
    ) u_mbox (
    .i_hclk             ( mbx_i_hclk          [gi0]                  ), 
    .i_hreset_n         ( mbx_i_hreset_n      [gi0]                  ), 
    .i_hsm_clk_en       ( mbx_i_hsm_clk_en    [gi0]                  ), 
    .i_se_hsel          ( mbx_i_se_hsel       [gi0]                  ), 
    .i_se_haddr         ( mbx_i_se_haddr      [gi0*(11+1) +: (11+1)] ), 
    .i_se_htrans        ( mbx_i_se_htrans     [gi0*(1+1) +: (1+1)]   ), 
    .i_se_hwrite        ( mbx_i_se_hwrite     [gi0]                  ), 
    .i_se_hsize         ( mbx_i_se_hsize      [gi0*(2+1) +: (2+1)]   ), 
    .i_se_hwdata        ( mbx_i_se_hwdata     [gi0*(31+1) +: (31+1)] ), 
    .i_se_hready        ( mbx_i_se_hready     [gi0]                  ), 
    .i_se_hburst        ( mbx_i_se_hburst     [gi0*(2+1) +: (2+1)]   ), 
    .i_se_hprot         ( mbx_i_se_hprot      [gi0*(3+1) +: (3+1)]   ), 
    .i_se_hmastlock     ( mbx_i_se_hmastlock  [gi0]                  ), 
    .o_se_hrdata        ( mbx_o_se_hrdata     [gi0*(31+1) +: (31+1)] ), 
    .o_se_hresp         ( mbx_o_se_hresp      [gi0*(1+1) +: (1+1)]   ), 
    .o_se_hreadyout     ( mbx_o_se_hreadyout  [gi0]                  ), 
    .i_hsm_status       ( mbx_i_hsm_status    [gi0*(63+1) +: (63+1)] ), 
    .o_se_irq           ( mbx_o_se_irq        [gi0]                  ), 
    .i_se_irq_all       ( mbx_i_se_irq_all    [gi0*(31+1) +: (31+1)] ), 
    .i_hsm_cfg          ( mbx_i_hsm_cfg       [gi0*(63+1) +: (63+1)] ), 
    .i_soc_hsel         ( mbx_i_soc_hsel      [gi0]                  ), 
    .i_soc_haddr        ( mbx_i_soc_haddr     [gi0*(11+1) +: (11+1)] ), 
    .i_soc_htrans       ( mbx_i_soc_htrans    [gi0*(1+1) +: (1+1)]   ), 
    .i_soc_hwrite       ( mbx_i_soc_hwrite    [gi0]                  ), 
    .i_soc_hsize        ( mbx_i_soc_hsize     [gi0*(2+1) +: (2+1)]   ), 
    .i_soc_hwdata       ( mbx_i_soc_hwdata    [gi0*(31+1) +: (31+1)] ), 
    .i_soc_hready       ( mbx_i_soc_hready    [gi0]                  ), 
    .i_soc_hburst       ( mbx_i_soc_hburst    [gi0*(2+1) +: (2+1)]   ), 
    .i_soc_hprot        ( mbx_i_soc_hprot     [gi0*(3+1) +: (3+1)]   ), 
    .i_soc_hmastlock    ( mbx_i_soc_hmastlock [gi0]                  ), 
    .o_soc_hrdata       ( mbx_o_soc_hrdata    [gi0*(31+1) +: (31+1)] ), 
    .o_soc_hresp        ( mbx_o_soc_hresp     [gi0*(1+1) +: (1+1)]   ), 
    .o_soc_hreadyout    ( mbx_o_soc_hreadyout [gi0]                  ), 
    .o_soc_irq          ( mbx_o_soc_irq       [gi0]                  ), 
    .i_soc_irq_all      ( mbx_i_soc_irq_all   [gi0*(31+1) +: (31+1)] )  
    ); 
end
endgenerate

endmodule 
