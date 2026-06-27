//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_ahb_busmatrix_top #(
    parameter P_BUS_MATRIX_AWIDTH    =  32                  ,
    parameter P_BUS_MATRIX_DWIDTH    =  32                  ,
    parameter P_BUS_MATRIX_DELAY_EN  =  1'b0                
)
(
    input  wire                             hclk              ,
    input  wire                             hresetn           ,

    input  wire                             i_hsels0          ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_haddrs0         ,
    input  wire [1:0]                       i_htranss0        ,
    input  wire                             i_hwrites0        ,
    input  wire [2:0]                       i_hsizes0         ,
    input  wire [2:0]                       i_hbursts0        ,
    input  wire [3:0]                       i_hprots0         ,
    input  wire [7:0]                       i_hmasters0       ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hwdatas0        ,
    input  wire                             i_hmastlocks0     ,
    input  wire                             i_hreadys0        ,

    output wire                             o_data_dft0       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hrdatas0        ,
    output wire                             o_hreadyouts0     ,
    output wire [1:0]                       o_hresps0         ,

    input  wire                             i_hsels1          ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_haddrs1         ,
    input  wire [1:0]                       i_htranss1        ,
    input  wire                             i_hwrites1        ,
    input  wire [2:0]                       i_hsizes1         ,
    input  wire [2:0]                       i_hbursts1        ,
    input  wire [3:0]                       i_hprots1         ,
    input  wire [7:0]                       i_hmasters1       ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hwdatas1        ,
    input  wire                             i_hmastlocks1     ,
    input  wire                             i_hreadys1        ,

    output wire                             o_data_dft1       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hrdatas1        ,
    output wire                             o_hreadyouts1     ,
    output wire [1:0]                       o_hresps1         ,

    input  wire                             i_hsels2          ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_haddrs2         ,
    input  wire [1:0]                       i_htranss2        ,
    input  wire                             i_hwrites2        ,
    input  wire [2:0]                       i_hsizes2         ,
    input  wire [2:0]                       i_hbursts2        ,
    input  wire [3:0]                       i_hprots2         ,
    input  wire [7:0]                       i_hmasters2       ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hwdatas2        ,
    input  wire                             i_hmastlocks2     ,
    input  wire                             i_hreadys2        ,

    output wire                             o_data_dft2       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hrdatas2        ,
    output wire                             o_hreadyouts2     ,
    output wire [1:0]                       o_hresps2         ,

    input  wire                             i_hsels3          ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_haddrs3         ,
    input  wire [1:0]                       i_htranss3        ,
    input  wire                             i_hwrites3        ,
    input  wire [2:0]                       i_hsizes3         ,
    input  wire [2:0]                       i_hbursts3        ,
    input  wire [3:0]                       i_hprots3         ,
    input  wire [7:0]                       i_hmasters3       ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hwdatas3        ,
    input  wire                             i_hmastlocks3     ,
    input  wire                             i_hreadys3        ,

    output wire                             o_data_dft3       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hrdatas3        ,
    output wire                             o_hreadyouts3     ,
    output wire [1:0]                       o_hresps3         ,

    input  wire                             i_hsels4          ,
    input  wire [P_BUS_MATRIX_AWIDTH-1:0]   i_haddrs4         ,
    input  wire [1:0]                       i_htranss4        ,
    input  wire                             i_hwrites4        ,
    input  wire [2:0]                       i_hsizes4         ,
    input  wire [2:0]                       i_hbursts4        ,
    input  wire [3:0]                       i_hprots4         ,
    input  wire [7:0]                       i_hmasters4       ,
    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hwdatas4        ,
    input  wire                             i_hmastlocks4     ,
    input  wire                             i_hreadys4        ,

    output wire                             o_data_dft4       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hrdatas4        ,
    output wire                             o_hreadyouts4     ,
    output wire [1:0]                       o_hresps4         ,

    output wire                             o_hselm0          ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]   o_haddrm0         ,
    output wire [1:0]                       o_htransm0        ,
    output wire                             o_hwritem0        ,
    output wire [2:0]                       o_hsizem0         ,
    output wire [2:0]                       o_hburstm0        ,
    output wire [3:0]                       o_hprotm0         ,
    output wire [7:0]                       o_hmasterm0       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hwdatam0        ,
    output wire                             o_hmastlockm0     ,
    output wire                             o_hreadymuxm0     ,

    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hrdatam0        ,
    input  wire                             i_hreadyoutm0     ,
    input  wire [1:0]                       i_hrespm0         ,

    output wire                             o_hselm1          ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]   o_haddrm1         ,
    output wire [1:0]                       o_htransm1        ,
    output wire                             o_hwritem1        ,
    output wire [2:0]                       o_hsizem1         ,
    output wire [2:0]                       o_hburstm1        ,
    output wire [3:0]                       o_hprotm1         ,
    output wire [7:0]                       o_hmasterm1       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hwdatam1        ,
    output wire                             o_hmastlockm1     ,
    output wire                             o_hreadymuxm1     ,

    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hrdatam1        ,
    input  wire                             i_hreadyoutm1     ,
    input  wire [1:0]                       i_hrespm1         ,

    output wire                             o_hselm2          ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]   o_haddrm2         ,
    output wire [1:0]                       o_htransm2        ,
    output wire                             o_hwritem2        ,
    output wire [2:0]                       o_hsizem2         ,
    output wire [2:0]                       o_hburstm2        ,
    output wire [3:0]                       o_hprotm2         ,
    output wire [7:0]                       o_hmasterm2       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hwdatam2        ,
    output wire                             o_hmastlockm2     ,
    output wire                             o_hreadymuxm2     ,

    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hrdatam2        ,
    input  wire                             i_hreadyoutm2     ,
    input  wire [1:0]                       i_hrespm2         ,

    output wire                             o_hselm3          ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]   o_haddrm3         ,
    output wire [1:0]                       o_htransm3        ,
    output wire                             o_hwritem3        ,
    output wire [2:0]                       o_hsizem3         ,
    output wire [2:0]                       o_hburstm3        ,
    output wire [3:0]                       o_hprotm3         ,
    output wire [7:0]                       o_hmasterm3       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hwdatam3        ,
    output wire                             o_hmastlockm3     ,
    output wire                             o_hreadymuxm3     ,

    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hrdatam3        ,
    input  wire                             i_hreadyoutm3     ,
    input  wire [1:0]                       i_hrespm3         ,

    output wire                             o_hselm4          ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]   o_haddrm4         ,
    output wire [1:0]                       o_htransm4        ,
    output wire                             o_hwritem4        ,
    output wire [2:0]                       o_hsizem4         ,
    output wire [2:0]                       o_hburstm4        ,
    output wire [3:0]                       o_hprotm4         ,
    output wire [7:0]                       o_hmasterm4       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hwdatam4        ,
    output wire                             o_hmastlockm4     ,
    output wire                             o_hreadymuxm4     ,

    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hrdatam4        ,
    input  wire                             i_hreadyoutm4     ,
    input  wire [1:0]                       i_hrespm4         ,

    output wire                             o_hselm5          ,
    output wire [P_BUS_MATRIX_AWIDTH-1:0]   o_haddrm5         ,
    output wire [1:0]                       o_htransm5        ,
    output wire                             o_hwritem5        ,
    output wire [2:0]                       o_hsizem5         ,
    output wire [2:0]                       o_hburstm5        ,
    output wire [3:0]                       o_hprotm5         ,
    output wire [7:0]                       o_hmasterm5       ,
    output wire [P_BUS_MATRIX_DWIDTH-1:0]   o_hwdatam5        ,
    output wire                             o_hmastlockm5     ,
    output wire                             o_hreadymuxm5     ,

    input  wire [P_BUS_MATRIX_DWIDTH-1:0]   i_hrdatam5        ,
    input  wire                             i_hreadyoutm5     ,
    input  wire [1:0]                       i_hrespm5          

);

wire                             sel0;
wire [P_BUS_MATRIX_AWIDTH-1:0]   addr0;
wire [1:0]                       trans0;
wire                             write0;
wire [2:0]                       size0;
wire [2:0]                       burst0;
wire [3:0]                       prot0;
wire [7:0]                       master0;
wire                             mastlock0;
wire                             active0;
wire                             heldtran0;
wire                             readyout0;
wire  [1:0]                      resp0;
wire                             pendtran0;

wire                             sel1;
wire [P_BUS_MATRIX_AWIDTH-1:0]   addr1;
wire [1:0]                       trans1;
wire                             write1;
wire [2:0]                       size1;
wire [2:0]                       burst1;
wire [3:0]                       prot1;
wire [7:0]                       master1;
wire                             mastlock1;
wire                             active1;
wire                             heldtran1;
wire                             readyout1;
wire [1:0]                       resp1;
wire                             pendtran1;

wire                             sel2;
wire [P_BUS_MATRIX_AWIDTH-1:0]   addr2;
wire [1:0]                       trans2;
wire                             write2;
wire [2:0]                       size2;
wire [2:0]                       burst2;
wire [3:0]                       prot2;
wire [7:0]                       master2;
wire                             mastlock2;
wire                             active2;
wire                             heldtran2;
wire                             readyout2;
wire [1:0]                       resp2;
wire                             pendtran2;

wire                             sel3;
wire [P_BUS_MATRIX_AWIDTH-1:0]   addr3;
wire [1:0]                       trans3;
wire                             write3;
wire [2:0]                       size3;
wire [2:0]                       burst3;
wire [3:0]                       prot3;
wire [7:0]                       master3;
wire                             mastlock3;
wire                             active3;
wire                             heldtran3;
wire                             readyout3;
wire [1:0]                       resp3;
wire                             pendtran3;

wire                             sel4;
wire [P_BUS_MATRIX_AWIDTH-1:0]   addr4;
wire [1:0]                       trans4;
wire                             write4;
wire [2:0]                       size4;
wire [2:0]                       burst4;
wire [3:0]                       prot4;
wire [7:0]                       master4;
wire                             mastlock4;
wire                             active4;
wire                             heldtran4;
wire                             readyout4;
wire [1:0]                       resp4;
wire                             pendtran4;

wire                           sel0to0;
wire                           active0to0;
wire                           sel1to0;
wire                           active1to0;
wire                           sel2to0;
wire                           active2to0;
wire                           sel3to0;
wire                           active3to0;
wire                           sel4to0;
wire                           active4to0;

wire                           sel0to1;
wire                           active0to1;
wire                           sel1to1;
wire                           active1to1;
wire                           sel2to1;
wire                           active2to1;
wire                           sel3to1;
wire                           active3to1;
wire                           sel4to1;
wire                           active4to1;

wire                           sel0to2;
wire                           active0to2;
wire                           sel1to2;
wire                           active1to2;
wire                           sel2to2;
wire                           active2to2;
wire                           sel3to2;
wire                           active3to2;
wire                           sel4to2;
wire                           active4to2;

wire                           sel0to3;
wire                           active0to3;
wire                           sel1to3;
wire                           active1to3;
wire                           sel2to3;
wire                           active2to3;
wire                           sel3to3;
wire                           active3to3;
wire                           sel4to3;
wire                           active4to3;

wire                           sel0to4;
wire                           active0to4;
wire                           sel1to4;
wire                           active1to4;
wire                           sel2to4;
wire                           active2to4;
wire                           sel3to4;
wire                           active3to4;
wire                           sel4to4;
wire                           active4to4;

wire                           sel0to5;
wire                           active0to5;
wire                           sel1to5;
wire                           active1to5;
wire                           sel2to5;
wire                           active2to5;
wire                           sel3to5;
wire                           active3to5;
wire                           sel4to5;
wire                           active4to5;

wire                           ihreadymuxm0;
wire                           ihreadymuxm1;
wire                           ihreadymuxm2;
wire                           ihreadymuxm3;
wire                           ihreadymuxm4;
wire                           ihreadymuxm5;

wire                           readyoutnd0;
wire                           readyoutnd1;
wire                           readyoutnd2;
wire                           readyoutnd3;
wire                           readyoutnd4;

wire                           readyoutarb0;
wire                           readyoutarb1;
wire                           readyoutarb2;
wire                           readyoutarb3;
wire                           readyoutarb4;

localparam P_SLAVE3_ADDR_MSB =  4'd3  ;

osr_slave_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)    
)
u_slave_ctl_0(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hsels      (i_hsels0           ),
    .i_haddrs     (i_haddrs0          ),
    .i_htranss    (i_htranss0         ),
    .i_hwrites    (i_hwrites0         ),
    .i_hsizes     (i_hsizes0          ),
    .i_hbursts    (i_hbursts0         ),
    .i_hprots     (i_hprots0          ),
    .i_hmasters   (i_hmasters0        ),
    .i_hmastlocks (i_hmastlocks0      ),
    .i_hreadys    (i_hreadys0         ),
    .i_active     (active0            ),
    .i_readyout   (readyout0          ),
    .i_readyoutnd (readyoutnd0        ),
    .i_resp       (resp0              ),

    .o_pendtran   (pendtran0          ),

    .o_hreadyouts (o_hreadyouts0      ),
    .o_hresps     (o_hresps0          ),
    .o_sel        (sel0               ),
    .o_addr       (addr0              ),
    .o_trans      (trans0             ),
    .o_write      (write0             ),
    .o_size       (size0              ),
    .o_burst      (burst0             ),
    .o_prot       (prot0              ),
    .o_master     (master0            ),
    .o_mastlock   (mastlock0          ),
    .o_heldtran   (heldtran0          )
);

osr_slave_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)    
)
u_slave_ctl_1(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hsels      (i_hsels1           ),
    .i_haddrs     (i_haddrs1          ),
    .i_htranss    (i_htranss1         ),
    .i_hwrites    (i_hwrites1         ),
    .i_hsizes     (i_hsizes1          ),
    .i_hbursts    (i_hbursts1         ),
    .i_hprots     (i_hprots1          ),
    .i_hmasters   (i_hmasters1        ),
    .i_hmastlocks (i_hmastlocks1      ),
    .i_hreadys    (i_hreadys1         ),
    .i_active     (active1            ),
    .i_readyout   (readyout1          ),
    .i_readyoutnd (readyoutnd1        ),    
    .i_resp       (resp1              ),

    .o_pendtran   (pendtran1          ),

    .o_hreadyouts (o_hreadyouts1      ),
    .o_hresps     (o_hresps1          ),
    .o_sel        (sel1               ),
    .o_addr       (addr1              ),
    .o_trans      (trans1             ),
    .o_write      (write1             ),
    .o_size       (size1              ),
    .o_burst      (burst1             ),
    .o_prot       (prot1              ),
    .o_master     (master1            ),
    .o_mastlock   (mastlock1          ),
    .o_heldtran   (heldtran1          )
  );

osr_slave_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_slave_ctl_2(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hsels      (i_hsels2           ),
    .i_haddrs     (i_haddrs2          ),
    .i_htranss    (i_htranss2         ),
    .i_hwrites    (i_hwrites2         ),
    .i_hsizes     (i_hsizes2          ),
    .i_hbursts    (i_hbursts2         ),
    .i_hprots     (i_hprots2          ),
    .i_hmasters   (i_hmasters2        ),
    .i_hmastlocks (i_hmastlocks2      ),
    .i_hreadys    (i_hreadys2         ),
    .i_active     (active2            ),
    .i_readyout   (readyout2          ),
    .i_readyoutnd (readyoutnd2        ), 
    .i_resp       (resp2              ),

    .o_pendtran   (pendtran2          ),

    .o_hreadyouts (o_hreadyouts2      ),
    .o_hresps     (o_hresps2          ),
    .o_sel        (sel2               ),
    .o_addr       (addr2              ),
    .o_trans      (trans2             ),
    .o_write      (write2             ),
    .o_size       (size2              ),
    .o_burst      (burst2             ),
    .o_prot       (prot2              ),
    .o_master     (master2            ),
    .o_mastlock   (mastlock2          ),
    .o_heldtran   (heldtran2          )
  );

osr_slave_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_slave_ctl_3(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hsels      (i_hsels3           ),
    .i_haddrs     (i_haddrs3          ),
    .i_htranss    (i_htranss3         ),
    .i_hwrites    (i_hwrites3         ),
    .i_hsizes     (i_hsizes3          ),
    .i_hbursts    (i_hbursts3         ),
    .i_hprots     (i_hprots3          ),
    .i_hmasters   (i_hmasters3        ),
    .i_hmastlocks (i_hmastlocks3      ),
    .i_hreadys    (i_hreadys3         ),
    .i_active     (active3            ),
    .i_readyout   (readyout3          ),
    .i_readyoutnd (readyoutnd3        ), 
    .i_resp       (resp3              ),

    .o_pendtran   (pendtran3          ),

    .o_hreadyouts (o_hreadyouts3      ),
    .o_hresps     (o_hresps3          ),
    .o_sel        (sel3               ),
    .o_addr       (addr3              ),
    .o_trans      (trans3             ),
    .o_write      (write3             ),
    .o_size       (size3              ),
    .o_burst      (burst3             ),
    .o_prot       (prot3              ),
    .o_master     (master3            ),
    .o_mastlock   (mastlock3          ),
    .o_heldtran   (heldtran3          )
  );

osr_slave_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_slave_ctl_4(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hsels      (i_hsels4           ),
    .i_haddrs     (i_haddrs4          ),
    .i_htranss    (i_htranss4         ),
    .i_hwrites    (i_hwrites4         ),
    .i_hsizes     (i_hsizes4          ),
    .i_hbursts    (i_hbursts4         ),
    .i_hprots     (i_hprots4          ),
    .i_hmasters   (i_hmasters4        ),
    .i_hmastlocks (i_hmastlocks4      ),
    .i_hreadys    (i_hreadys4         ),
    .i_active     (active4            ),
    .i_readyout   (readyout4          ),
    .i_readyoutnd (readyoutnd4        ), 
    .i_resp       (resp4              ),

    .o_pendtran   (pendtran4          ),

    .o_hreadyouts (o_hreadyouts4      ),
    .o_hresps     (o_hresps4          ),
    .o_sel        (sel4               ),
    .o_addr       (addr4              ),
    .o_trans      (trans4             ),
    .o_write      (write4             ),
    .o_size       (size4              ),
    .o_burst      (burst4             ),
    .o_prot       (prot4              ),
    .o_master     (master4            ),
    .o_mastlock   (mastlock4          ),
    .o_heldtran   (heldtran4          )
  );

osr_decoder #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN),
    .P_SLAVE3_ADDR_MSB     (P_SLAVE3_ADDR_MSB    ),
    .P_SLAVE1_RMP8_EN      (1'b1                 ),
    .P_SLAVE1_ADDR_RMP8    (8'h78                ),
    .P_SLAVE4_RMP8_EN      (1'b1                 ),
    .P_SLAVE4_ADDR_RMP8    (8'h70                )
)u_decoder_0 (
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hreadys    (i_hreadys0         ),
    .i_sel        (sel0               ),
    .i_addr       (addr0              ),
    .i_trans      (trans0             ),
    .i_pendtran   (pendtran0          ),
    .i_active0    (active0to0         ),
    .i_readyout0  (ihreadymuxm0       ),
    .i_resp0      (i_hrespm0          ),
    .i_rdata0     (i_hrdatam0         ),
    .i_active1    (active0to1         ),
    .i_readyout1  (ihreadymuxm1       ),
    .i_resp1      (i_hrespm1          ),
    .i_rdata1     (i_hrdatam1         ),
    .i_active2    (active0to2         ),
    .i_readyout2  (ihreadymuxm2       ),
    .i_resp2      (i_hrespm2          ),
    .i_rdata2     (i_hrdatam2         ),
    .i_active3    (active0to3         ),
    .i_readyout3  (ihreadymuxm3       ),
    .i_resp3      (i_hrespm3          ),
    .i_rdata3     (i_hrdatam3         ),
    .i_active4    (active0to4         ),
    .i_readyout4  (ihreadymuxm4       ),
    .i_resp4      (i_hrespm4          ),
    .i_rdata4     (i_hrdatam4         ),
    .i_active5    (active0to5         ),
    .i_readyout5  (ihreadymuxm5       ),
    .i_resp5      (i_hrespm5          ),
    .i_rdata5     (i_hrdatam5         ),

    .o_sel0       (sel0to0            ),
    .o_sel1       (sel0to1            ),
    .o_sel2       (sel0to2            ),
    .o_sel3       (sel0to3            ),
    .o_sel4       (sel0to4            ),
    .o_sel5       (sel0to5            ),
    .o_data_dft   (o_data_dft0        ),
    .o_active     (active0            ),
    .o_hreadyouts (readyout0          ),
    .o_hreadyoutnd(readyoutnd0        ),
    .o_hreadyoutarb(readyoutarb0      ),
    .o_hresps     (resp0              ),
    .o_hrdatas    (o_hrdatas0         )
  );

osr_decoder #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN),    
    .P_SLAVE3_ADDR_MSB     (P_SLAVE3_ADDR_MSB    ),
    .P_SLAVE4_RMP12_EN     (1'b1                 )
)u_decoder_1 (
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hreadys    (i_hreadys1         ),
    .i_sel        (sel1               ),
    .i_addr       (addr1              ),
    .i_trans      (trans1             ),
    .i_pendtran   (pendtran1          ),
    .i_active0    (active1to0         ),
    .i_readyout0  (ihreadymuxm0       ),
    .i_resp0      (i_hrespm0          ),
    .i_rdata0     (i_hrdatam0         ),
    .i_active1    (active1to1         ),
    .i_readyout1  (ihreadymuxm1       ),
    .i_resp1      (i_hrespm1          ),
    .i_rdata1     (i_hrdatam1         ),
    .i_active2    (active1to2         ),
    .i_readyout2  (ihreadymuxm2       ),
    .i_resp2      (i_hrespm2          ),
    .i_rdata2     (i_hrdatam2         ),
    .i_active3    (active1to3         ),
    .i_readyout3  (ihreadymuxm3       ),
    .i_resp3      (i_hrespm3          ),
    .i_rdata3     (i_hrdatam3         ),
    .i_active4    (active1to4         ),
    .i_readyout4  (ihreadymuxm4       ),
    .i_resp4      (i_hrespm4          ),
    .i_rdata4     (i_hrdatam4         ),
    .i_active5    (active1to5         ),
    .i_readyout5  (ihreadymuxm5       ),
    .i_resp5      (i_hrespm5          ),
    .i_rdata5     (i_hrdatam5         ),

    .o_sel0       (sel1to0            ),
    .o_sel1       (sel1to1            ),
    .o_sel2       (sel1to2            ),
    .o_sel3       (sel1to3            ),
    .o_sel4       (sel1to4            ),
    .o_sel5       (sel1to5            ),
    .o_data_dft   (o_data_dft1        ),
    .o_active     (active1            ),
    .o_hreadyouts (readyout1          ),
    .o_hreadyoutnd(readyoutnd1        ),
    .o_hreadyoutarb(readyoutarb1      ),
    .o_hresps     (resp1              ),
    .o_hrdatas    (o_hrdatas1         )
  );

osr_decoder #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN),
    .P_SLAVE3_ADDR_MSB     (P_SLAVE3_ADDR_MSB    ),
    .P_SLAVE1_RMP8_EN      (1'b1                 ),
    .P_SLAVE1_ADDR_RMP8    (8'h78                ),
    .P_SLAVE4_RMP8_EN      (1'b1                 ),
    .P_SLAVE4_ADDR_RMP8    (8'h70                )      
)u_decoder_2 (
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hreadys    (i_hreadys2         ),
    .i_sel        (sel2               ),
    .i_addr       (addr2              ),
    .i_trans      (trans2             ),
    .i_pendtran   (pendtran2          ),
    .i_active0    (active2to0         ),
    .i_readyout0  (ihreadymuxm0       ),
    .i_resp0      (i_hrespm0          ),
    .i_rdata0     (i_hrdatam0         ),
    .i_active1    (active2to1         ),
    .i_readyout1  (ihreadymuxm1       ),
    .i_resp1      (i_hrespm1          ),
    .i_rdata1     (i_hrdatam1         ),
    .i_active2    (active2to2         ),
    .i_readyout2  (ihreadymuxm2       ),
    .i_resp2      (i_hrespm2          ),
    .i_rdata2     (i_hrdatam2         ),
    .i_active3    (active2to3         ),
    .i_readyout3  (ihreadymuxm3       ),
    .i_resp3      (i_hrespm3          ),
    .i_rdata3     (i_hrdatam3         ),
    .i_active4    (active2to4         ),
    .i_readyout4  (ihreadymuxm4       ),
    .i_resp4      (i_hrespm4          ),
    .i_rdata4     (i_hrdatam4         ),
    .i_active5    (active2to5         ),
    .i_readyout5  (ihreadymuxm5       ),
    .i_resp5      (i_hrespm5          ),
    .i_rdata5     (i_hrdatam5         ),

    .o_sel0       (sel2to0            ),
    .o_sel1       (sel2to1            ),
    .o_sel2       (sel2to2            ),
    .o_sel3       (sel2to3            ),
    .o_sel4       (sel2to4            ),
    .o_sel5       (sel2to5            ),
    .o_data_dft   (o_data_dft2        ),
    .o_active     (active2            ),
    .o_hreadyouts (readyout2          ),
    .o_hreadyoutnd(readyoutnd2        ),
    .o_hreadyoutarb(readyoutarb2      ),
    .o_hresps     (resp2              ),
    .o_hrdatas    (o_hrdatas2         )
  );

osr_decoder #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN),
    .P_SLAVE3_ADDR_MSB     (P_SLAVE3_ADDR_MSB    ),
    .P_SLAVE1_RMP8_EN      (1'b1                 ),
    .P_SLAVE1_ADDR_RMP8    (8'h78                ),
    .P_SLAVE4_RMP8_EN      (1'b1                 ),
    .P_SLAVE4_ADDR_RMP8    (8'h70                )
)u_decoder_3 (
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hreadys    (i_hreadys3         ),
    .i_sel        (sel3               ),
    .i_addr       (addr3              ),
    .i_trans      (trans3             ),
    .i_pendtran   (pendtran3          ),
    .i_active0    (active3to0         ),
    .i_readyout0  (ihreadymuxm0       ),
    .i_resp0      (i_hrespm0          ),
    .i_rdata0     (i_hrdatam0         ),
    .i_active1    (active3to1         ),
    .i_readyout1  (ihreadymuxm1       ),
    .i_resp1      (i_hrespm1          ),
    .i_rdata1     (i_hrdatam1         ),
    .i_active2    (active3to2         ),
    .i_readyout2  (ihreadymuxm2       ),
    .i_resp2      (i_hrespm2          ),
    .i_rdata2     (i_hrdatam2         ),
    .i_active3    (active3to3         ),
    .i_readyout3  (ihreadymuxm3       ),
    .i_resp3      (i_hrespm3          ),
    .i_rdata3     (i_hrdatam3         ),
    .i_active4    (active3to4         ),
    .i_readyout4  (ihreadymuxm4       ),
    .i_resp4      (i_hrespm4          ),
    .i_rdata4     (i_hrdatam4         ),
    .i_active5    (active3to5         ),
    .i_readyout5  (ihreadymuxm5       ),
    .i_resp5      (i_hrespm5          ),
    .i_rdata5     (i_hrdatam5         ),

    .o_sel0       (sel3to0            ),
    .o_sel1       (sel3to1            ),
    .o_sel2       (sel3to2            ),
    .o_sel3       (sel3to3            ),
    .o_sel4       (sel3to4            ),
    .o_sel5       (sel3to5            ),
    .o_data_dft   (o_data_dft3        ),
    .o_active     (active3            ),
    .o_hreadyouts (readyout3          ),
    .o_hreadyoutnd(readyoutnd3        ),
    .o_hreadyoutarb(readyoutarb3      ),
    .o_hresps     (resp3              ),
    .o_hrdatas    (o_hrdatas3         )
  );

osr_decoder #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN),
    .P_SLAVE3_ADDR_MSB     (P_SLAVE3_ADDR_MSB    ),
    .P_SLAVE1_RMP8_EN      (1'b1                 ),
    .P_SLAVE1_ADDR_RMP8    (8'h78                ),
    .P_SLAVE4_RMP8_EN      (1'b1                 ),
    .P_SLAVE4_ADDR_RMP8    (8'h70                )      
)u_decoder_4 (
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),
    .i_hreadys    (i_hreadys4         ),
    .i_sel        (sel4               ),
    .i_addr       (addr4              ),
    .i_trans      (trans4             ),
    .i_pendtran   (pendtran4          ),
    .i_active0    (active4to0         ),
    .i_readyout0  (ihreadymuxm0       ),
    .i_resp0      (i_hrespm0          ),
    .i_rdata0     (i_hrdatam0         ),
    .i_active1    (active4to1         ),
    .i_readyout1  (ihreadymuxm1       ),
    .i_resp1      (i_hrespm1          ),
    .i_rdata1     (i_hrdatam1         ),
    .i_active2    (active4to2         ),
    .i_readyout2  (ihreadymuxm2       ),
    .i_resp2      (i_hrespm2          ),
    .i_rdata2     (i_hrdatam2         ),
    .i_active3    (active4to3         ),
    .i_readyout3  (ihreadymuxm3       ),
    .i_resp3      (i_hrespm3          ),
    .i_rdata3     (i_hrdatam3         ),
    .i_active4    (active4to4         ),
    .i_readyout4  (ihreadymuxm4       ),
    .i_resp4      (i_hrespm4          ),
    .i_rdata4     (i_hrdatam4         ),
    .i_active5    (active4to5         ),
    .i_readyout5  (ihreadymuxm5       ),
    .i_resp5      (i_hrespm5          ),
    .i_rdata5     (i_hrdatam5         ),

    .o_sel0       (sel4to0            ),
    .o_sel1       (sel4to1            ),
    .o_sel2       (sel4to2            ),
    .o_sel3       (sel4to3            ),
    .o_sel4       (sel4to4            ),
    .o_sel5       (sel4to5            ),
    .o_data_dft   (o_data_dft4        ),
    .o_active     (active4            ),
    .o_hreadyouts (readyout4          ),
    .o_hreadyoutnd(readyoutnd4        ),
    .o_hreadyoutarb(readyoutarb4      ),
    .o_hresps     (resp4              ),
    .o_hrdatas    (o_hrdatas4         )
  );

osr_master_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_master_ctl_0 (
    .hclk        (hclk                ),
    .hresetn     (hresetn             ),

    .i_sel0       (sel0to0            ),
    .i_addr0      (addr0              ),
    .i_trans0     (trans0             ),
    .i_write0     (write0             ),
    .i_size0      (size0              ),
    .i_burst0     (burst0             ),
    .i_prot0      (prot0              ),
    .i_master0    (master0            ),
    .i_mastlock0  (mastlock0          ),
    .i_wdata0     (i_hwdatas0         ),
    .i_heldtran0  (heldtran0          ),

    .i_sel1       (sel1to0            ),
    .i_addr1      (addr1              ),
    .i_trans1     (trans1             ),
    .i_write1     (write1             ),
    .i_size1      (size1              ),
    .i_burst1     (burst1             ),
    .i_prot1      (prot1              ),
    .i_master1    (master1            ),
    .i_mastlock1  (mastlock1          ),
    .i_wdata1     (i_hwdatas1         ),
    .i_heldtran1  (heldtran1          ),

    .i_sel2       (sel2to0            ),
    .i_addr2      (addr2              ),
    .i_trans2     (trans2             ),
    .i_write2     (write2             ),
    .i_size2      (size2              ),
    .i_burst2     (burst2             ),
    .i_prot2      (prot2              ),
    .i_master2    (master2            ),
    .i_mastlock2  (mastlock2          ),
    .i_wdata2     (i_hwdatas2         ),
    .i_heldtran2  (heldtran2          ),

    .i_sel3       (sel3to0            ),
    .i_addr3      (addr3              ),
    .i_trans3     (trans3             ),
    .i_write3     (write3             ),
    .i_size3      (size3              ),
    .i_burst3     (burst3             ),
    .i_prot3      (prot3              ),
    .i_master3    (master3            ),
    .i_mastlock3  (mastlock3          ),
    .i_wdata3     (i_hwdatas3         ),
    .i_heldtran3  (heldtran3          ),

    .i_sel4       (sel4to0            ),
    .i_addr4      (addr4              ),
    .i_trans4     (trans4             ),
    .i_write4     (write4             ),
    .i_size4      (size4              ),
    .i_burst4     (burst4             ),
    .i_prot4      (prot4              ),
    .i_master4    (master4            ),
    .i_mastlock4  (mastlock4          ),
    .i_wdata4     (i_hwdatas4         ),
    .i_heldtran4  (heldtran4          ),

    .i_hreadyoutm (i_hreadyoutm0      ),

    .i_readyoutarb0 (readyoutarb0     ),
    .i_readyoutarb1 (readyoutarb1     ),
    .i_readyoutarb2 (readyoutarb2     ),
    .i_readyoutarb3 (readyoutarb3     ),
    .i_readyoutarb4 (readyoutarb4     ),

    .o_active0    (active0to0         ),
    .o_active1    (active1to0         ),
    .o_active2    (active2to0         ),
    .o_active3    (active3to0         ),
    .o_active4    (active4to0         ),

    .o_hselm      (o_hselm0           ),
    .o_haddrm     (o_haddrm0          ),
    .o_htransm    (o_htransm0         ),
    .o_hwritem    (o_hwritem0         ),
    .o_hsizem     (o_hsizem0          ),
    .o_hburstm    (o_hburstm0         ),
    .o_hprotm     (o_hprotm0          ),
    .o_hmasterm   (o_hmasterm0        ),
    .o_hmastlockm (o_hmastlockm0      ),
    .o_hreadymuxm (ihreadymuxm0       ),
    .o_hwdatam    (o_hwdatam0         )
  );

assign o_hreadymuxm0 = ihreadymuxm0;

osr_master_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_master_ctl_1(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),

    .i_sel0       (sel0to1            ),
    .i_addr0      (addr0              ),
    .i_trans0     (trans0             ),
    .i_write0     (write0             ),
    .i_size0      (size0              ),
    .i_burst0     (burst0             ),
    .i_prot0      (prot0              ),
    .i_master0    (master0            ),
    .i_mastlock0  (mastlock0          ),
    .i_wdata0     (i_hwdatas0         ),
    .i_heldtran0  (heldtran0          ),

    .i_sel1       (sel1to1            ),
    .i_addr1      (addr1              ),
    .i_trans1     (trans1             ),
    .i_write1     (write1             ),
    .i_size1      (size1              ),
    .i_burst1     (burst1             ),
    .i_prot1      (prot1              ),
    .i_master1    (master1            ),
    .i_mastlock1  (mastlock1          ),
    .i_wdata1     (i_hwdatas1         ),
    .i_heldtran1  (heldtran1          ),

    .i_sel2       (sel2to1            ),
    .i_addr2      (addr2              ),
    .i_trans2     (trans2             ),
    .i_write2     (write2             ),
    .i_size2      (size2              ),
    .i_burst2     (burst2             ),
    .i_prot2      (prot2              ),
    .i_master2    (master2            ),
    .i_mastlock2  (mastlock2          ),
    .i_wdata2     (i_hwdatas2         ),
    .i_heldtran2  (heldtran2          ),

    .i_sel3       (sel3to1            ),
    .i_addr3      (addr3              ),
    .i_trans3     (trans3             ),
    .i_write3     (write3             ),
    .i_size3      (size3              ),
    .i_burst3     (burst3             ),
    .i_prot3      (prot3              ),
    .i_master3    (master3            ),
    .i_mastlock3  (mastlock3          ),
    .i_wdata3     (i_hwdatas3         ),
    .i_heldtran3  (heldtran3          ),

    .i_sel4       (sel4to1            ),
    .i_addr4      (addr4              ),
    .i_trans4     (trans4             ),
    .i_write4     (write4             ),
    .i_size4      (size4              ),
    .i_burst4     (burst4             ),
    .i_prot4      (prot4              ),
    .i_master4    (master4            ),
    .i_mastlock4  (mastlock4          ),
    .i_wdata4     (i_hwdatas4         ),
    .i_heldtran4  (heldtran4          ),

    .i_hreadyoutm (i_hreadyoutm1      ),

    .i_readyoutarb0 (readyoutarb0     ),
    .i_readyoutarb1 (readyoutarb1     ),
    .i_readyoutarb2 (readyoutarb2     ),
    .i_readyoutarb3 (readyoutarb3     ),
    .i_readyoutarb4 (readyoutarb4     ),

    .o_active0    (active0to1         ),
    .o_active1    (active1to1         ),
    .o_active2    (active2to1         ),
    .o_active3    (active3to1         ),
    .o_active4    (active4to1         ),

    .o_hselm      (o_hselm1           ),
    .o_haddrm     (o_haddrm1          ),
    .o_htransm    (o_htransm1         ),
    .o_hwritem    (o_hwritem1         ),
    .o_hsizem     (o_hsizem1          ),
    .o_hburstm    (o_hburstm1         ),
    .o_hprotm     (o_hprotm1          ),
    .o_hmasterm   (o_hmasterm1        ),
    .o_hmastlockm (o_hmastlockm1      ),
    .o_hreadymuxm (ihreadymuxm1       ),
    .o_hwdatam    (o_hwdatam1         )
  );

assign o_hreadymuxm1 = ihreadymuxm1;
osr_master_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_master_ctl_2(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),

    .i_sel0       (sel0to2            ),
    .i_addr0      (addr0              ),
    .i_trans0     (trans0             ),
    .i_write0     (write0             ),
    .i_size0      (size0              ),
    .i_burst0     (burst0             ),
    .i_prot0      (prot0              ),
    .i_master0    (master0            ),
    .i_mastlock0  (mastlock0          ),
    .i_wdata0     (i_hwdatas0         ),
    .i_heldtran0  (heldtran0          ),

    .i_sel1       (sel1to2            ),
    .i_addr1      (addr1              ),
    .i_trans1     (trans1             ),
    .i_write1     (write1             ),
    .i_size1      (size1              ),
    .i_burst1     (burst1             ),
    .i_prot1      (prot1              ),
    .i_master1    (master1            ),
    .i_mastlock1  (mastlock1          ),
    .i_wdata1     (i_hwdatas1         ),
    .i_heldtran1  (heldtran1          ),

    .i_sel2       (sel2to2            ),
    .i_addr2      (addr2              ),
    .i_trans2     (trans2             ),
    .i_write2     (write2             ),
    .i_size2      (size2              ),
    .i_burst2     (burst2             ),
    .i_prot2      (prot2              ),
    .i_master2    (master2            ),
    .i_mastlock2  (mastlock2          ),
    .i_wdata2     (i_hwdatas2         ),
    .i_heldtran2  (heldtran2          ),

    .i_sel3       (sel3to2            ),
    .i_addr3      (addr3              ),
    .i_trans3     (trans3             ),
    .i_write3     (write3             ),
    .i_size3      (size3              ),
    .i_burst3     (burst3             ),
    .i_prot3      (prot3              ),
    .i_master3    (master3            ),
    .i_mastlock3  (mastlock3          ),
    .i_wdata3     (i_hwdatas3         ),
    .i_heldtran3  (heldtran3          ),

    .i_sel4       (sel4to2            ),
    .i_addr4      (addr4              ),
    .i_trans4     (trans4             ),
    .i_write4     (write4             ),
    .i_size4      (size4              ),
    .i_burst4     (burst4             ),
    .i_prot4      (prot4              ),
    .i_master4    (master4            ),
    .i_mastlock4  (mastlock4          ),
    .i_wdata4     (i_hwdatas4         ),
    .i_heldtran4  (heldtran4          ),

    .i_hreadyoutm (i_hreadyoutm2      ),

    .i_readyoutarb0 (readyoutarb0     ),
    .i_readyoutarb1 (readyoutarb1     ),
    .i_readyoutarb2 (readyoutarb2     ),
    .i_readyoutarb3 (readyoutarb3     ),
    .i_readyoutarb4 (readyoutarb4     ),

    .o_active0    (active0to2         ),
    .o_active1    (active1to2         ),
    .o_active2    (active2to2         ),
    .o_active3    (active3to2         ),
    .o_active4    (active4to2         ),

    .o_hselm      (o_hselm2           ),
    .o_haddrm     (o_haddrm2          ),
    .o_htransm    (o_htransm2         ),
    .o_hwritem    (o_hwritem2         ),
    .o_hsizem     (o_hsizem2          ),
    .o_hburstm    (o_hburstm2         ),
    .o_hprotm     (o_hprotm2          ),
    .o_hmasterm   (o_hmasterm2        ),
    .o_hmastlockm (o_hmastlockm2      ),
    .o_hreadymuxm (ihreadymuxm2       ),
    .o_hwdatam    (o_hwdatam2         )
  );

assign o_hreadymuxm2 = ihreadymuxm2;

osr_master_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_master_ctl_3(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),

    .i_sel0       (sel0to3            ),
    .i_addr0      (addr0              ),
    .i_trans0     (trans0             ),
    .i_write0     (write0             ),
    .i_size0      (size0              ),
    .i_burst0     (burst0             ),
    .i_prot0      (prot0              ),
    .i_master0    (master0            ),
    .i_mastlock0  (mastlock0          ),
    .i_wdata0     (i_hwdatas0         ),
    .i_heldtran0  (heldtran0          ),

    .i_sel1       (sel1to3            ),
    .i_addr1      (addr1              ),
    .i_trans1     (trans1             ),
    .i_write1     (write1             ),
    .i_size1      (size1              ),
    .i_burst1     (burst1             ),
    .i_prot1      (prot1              ),
    .i_master1    (master1            ),
    .i_mastlock1  (mastlock1          ),
    .i_wdata1     (i_hwdatas1         ),
    .i_heldtran1  (heldtran1          ),

    .i_sel2       (sel2to3            ),
    .i_addr2      (addr2              ),
    .i_trans2     (trans2             ),
    .i_write2     (write2             ),
    .i_size2      (size2              ),
    .i_burst2     (burst2             ),
    .i_prot2      (prot2              ),
    .i_master2    (master2            ),
    .i_mastlock2  (mastlock2          ),
    .i_wdata2     (i_hwdatas2         ),
    .i_heldtran2  (heldtran2          ),

    .i_sel3       (sel3to3            ),
    .i_addr3      (addr3              ),
    .i_trans3     (trans3             ),
    .i_write3     (write3             ),
    .i_size3      (size3              ),
    .i_burst3     (burst3             ),
    .i_prot3      (prot3              ),
    .i_master3    (master3            ),
    .i_mastlock3  (mastlock3          ),
    .i_wdata3     (i_hwdatas3         ),
    .i_heldtran3  (heldtran3          ),

    .i_sel4       (sel4to3            ),
    .i_addr4      (addr4              ),
    .i_trans4     (trans4             ),
    .i_write4     (write4             ),
    .i_size4      (size4              ),
    .i_burst4     (burst4             ),
    .i_prot4      (prot4              ),
    .i_master4    (master4            ),
    .i_mastlock4  (mastlock4          ),
    .i_wdata4     (i_hwdatas4         ),
    .i_heldtran4  (heldtran4          ),

    .i_hreadyoutm (i_hreadyoutm3      ),

    .i_readyoutarb0 (readyoutarb0     ),
    .i_readyoutarb1 (readyoutarb1     ),
    .i_readyoutarb2 (readyoutarb2     ),
    .i_readyoutarb3 (readyoutarb3     ),
    .i_readyoutarb4 (readyoutarb4     ),

    .o_active0    (active0to3         ),
    .o_active1    (active1to3         ),
    .o_active2    (active2to3         ),
    .o_active3    (active3to3         ),
    .o_active4    (active4to3         ),

    .o_hselm      (o_hselm3           ),
    .o_haddrm     (o_haddrm3          ),
    .o_htransm    (o_htransm3         ),
    .o_hwritem    (o_hwritem3         ),
    .o_hsizem     (o_hsizem3          ),
    .o_hburstm    (o_hburstm3         ),
    .o_hprotm     (o_hprotm3          ),
    .o_hmasterm   (o_hmasterm3        ),
    .o_hmastlockm (o_hmastlockm3      ),
    .o_hreadymuxm (ihreadymuxm3       ),
    .o_hwdatam    (o_hwdatam3         )
  );

assign o_hreadymuxm3 = ihreadymuxm3;
osr_master_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_master_ctl_4(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),

    .i_sel0       (sel0to4            ),
    .i_addr0      (addr0              ),
    .i_trans0     (trans0             ),
    .i_write0     (write0             ),
    .i_size0      (size0              ),
    .i_burst0     (burst0             ),
    .i_prot0      (prot0              ),
    .i_master0    (master0            ),
    .i_mastlock0  (mastlock0          ),
    .i_wdata0     (i_hwdatas0         ),
    .i_heldtran0  (heldtran0          ),

    .i_sel1       (sel1to4            ),
    .i_addr1      (addr1              ),
    .i_trans1     (trans1             ),
    .i_write1     (write1             ),
    .i_size1      (size1              ),
    .i_burst1     (burst1             ),
    .i_prot1      (prot1              ),
    .i_master1    (master1            ),
    .i_mastlock1  (mastlock1          ),
    .i_wdata1     (i_hwdatas1         ),
    .i_heldtran1  (heldtran1          ),

    .i_sel2       (sel2to4            ),
    .i_addr2      (addr2              ),
    .i_trans2     (trans2             ),
    .i_write2     (write2             ),
    .i_size2      (size2              ),
    .i_burst2     (burst2             ),
    .i_prot2      (prot2              ),
    .i_master2    (master2            ),
    .i_mastlock2  (mastlock2          ),
    .i_wdata2     (i_hwdatas2         ),
    .i_heldtran2  (heldtran2          ),

    .i_sel3       (sel3to4            ),
    .i_addr3      (addr3              ),
    .i_trans3     (trans3             ),
    .i_write3     (write3             ),
    .i_size3      (size3              ),
    .i_burst3     (burst3             ),
    .i_prot3      (prot3              ),
    .i_master3    (master3            ),
    .i_mastlock3  (mastlock3          ),
    .i_wdata3     (i_hwdatas3         ),
    .i_heldtran3  (heldtran3          ),

    .i_sel4       (sel4to4            ),
    .i_addr4      (addr4              ),
    .i_trans4     (trans4             ),
    .i_write4     (write4             ),
    .i_size4      (size4              ),
    .i_burst4     (burst4             ),
    .i_prot4      (prot4              ),
    .i_master4    (master4            ),
    .i_mastlock4  (mastlock4          ),
    .i_wdata4     (i_hwdatas4         ),
    .i_heldtran4  (heldtran4          ),

    .i_hreadyoutm (i_hreadyoutm4      ),

    .i_readyoutarb0 (readyoutarb0     ),
    .i_readyoutarb1 (readyoutarb1     ),
    .i_readyoutarb2 (readyoutarb2     ),
    .i_readyoutarb3 (readyoutarb3     ),
    .i_readyoutarb4 (readyoutarb4     ),

    .o_active0    (active0to4         ),
    .o_active1    (active1to4         ),
    .o_active2    (active2to4         ),
    .o_active3    (active3to4         ),
    .o_active4    (active4to4         ),

    .o_hselm      (o_hselm4           ),
    .o_haddrm     (o_haddrm4          ),
    .o_htransm    (o_htransm4         ),
    .o_hwritem    (o_hwritem4         ),
    .o_hsizem     (o_hsizem4          ),
    .o_hburstm    (o_hburstm4         ),
    .o_hprotm     (o_hprotm4          ),
    .o_hmasterm   (o_hmasterm4        ),
    .o_hmastlockm (o_hmastlockm4      ),
    .o_hreadymuxm (ihreadymuxm4       ),
    .o_hwdatam    (o_hwdatam4         )
  );

assign o_hreadymuxm4 = ihreadymuxm4;

osr_master_ctl #(
    .P_BUS_MATRIX_AWIDTH   (P_BUS_MATRIX_AWIDTH  ),
    .P_BUS_MATRIX_DWIDTH   (P_BUS_MATRIX_DWIDTH  ),
    .P_BUS_MATRIX_DELAY_EN (P_BUS_MATRIX_DELAY_EN)
)
u_master_ctl_5(
    .hclk         (hclk               ),
    .hresetn      (hresetn            ),

    .i_sel0       (sel0to5            ),
    .i_addr0      (addr0              ),
    .i_trans0     (trans0             ),
    .i_write0     (write0             ),
    .i_size0      (size0              ),
    .i_burst0     (burst0             ),
    .i_prot0      (prot0              ),
    .i_master0    (master0            ),
    .i_mastlock0  (mastlock0          ),
    .i_wdata0     (i_hwdatas0         ),
    .i_heldtran0  (heldtran0          ),

    .i_sel1       (sel1to5            ),
    .i_addr1      (addr1              ),
    .i_trans1     (trans1             ),
    .i_write1     (write1             ),
    .i_size1      (size1              ),
    .i_burst1     (burst1             ),
    .i_prot1      (prot1              ),
    .i_master1    (master1            ),
    .i_mastlock1  (mastlock1          ),
    .i_wdata1     (i_hwdatas1         ),
    .i_heldtran1  (heldtran1          ),

    .i_sel2       (sel2to5            ),
    .i_addr2      (addr2              ),
    .i_trans2     (trans2             ),
    .i_write2     (write2             ),
    .i_size2      (size2              ),
    .i_burst2     (burst2             ),
    .i_prot2      (prot2              ),
    .i_master2    (master2            ),
    .i_mastlock2  (mastlock2          ),
    .i_wdata2     (i_hwdatas2         ),
    .i_heldtran2  (heldtran2          ),

    .i_sel3       (sel3to5            ),
    .i_addr3      (addr3              ),
    .i_trans3     (trans3             ),
    .i_write3     (write3             ),
    .i_size3      (size3              ),
    .i_burst3     (burst3             ),
    .i_prot3      (prot3              ),
    .i_master3    (master3            ),
    .i_mastlock3  (mastlock3          ),
    .i_wdata3     (i_hwdatas3         ),
    .i_heldtran3  (heldtran3          ),

    .i_sel4       (sel4to5            ),
    .i_addr4      (addr4              ),
    .i_trans4     (trans4             ),
    .i_write4     (write4             ),
    .i_size4      (size4              ),
    .i_burst4     (burst4             ),
    .i_prot4      (prot4              ),
    .i_master4    (master4            ),
    .i_mastlock4  (mastlock4          ),
    .i_wdata4     (i_hwdatas4         ),
    .i_heldtran4  (heldtran4          ),

    .i_hreadyoutm (i_hreadyoutm5      ),

    .i_readyoutarb0 (readyoutarb0     ),
    .i_readyoutarb1 (readyoutarb1     ),
    .i_readyoutarb2 (readyoutarb2     ),
    .i_readyoutarb3 (readyoutarb3     ),
    .i_readyoutarb4 (readyoutarb4     ),

    .o_active0    (active0to5         ),
    .o_active1    (active1to5         ),
    .o_active2    (active2to5         ),
    .o_active3    (active3to5         ),
    .o_active4    (active4to5         ),

    .o_hselm      (o_hselm5           ),
    .o_haddrm     (o_haddrm5          ),
    .o_htransm    (o_htransm5         ),
    .o_hwritem    (o_hwritem5         ),
    .o_hsizem     (o_hsizem5          ),
    .o_hburstm    (o_hburstm5         ),
    .o_hprotm     (o_hprotm5          ),
    .o_hmasterm   (o_hmasterm5        ),
    .o_hmastlockm (o_hmastlockm5      ),
    .o_hreadymuxm (ihreadymuxm5       ),
    .o_hwdatam    (o_hwdatam5         )
  );

assign o_hreadymuxm5 = ihreadymuxm5;

endmodule
