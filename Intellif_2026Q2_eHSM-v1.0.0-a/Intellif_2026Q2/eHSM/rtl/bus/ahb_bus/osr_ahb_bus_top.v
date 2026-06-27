//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================

module osr_ahb_bus_top (
    input  wire          hclk                    ,
    input  wire          hresetn                 ,

    input  wire          i_m_hsel                ,
    input  wire [31:0]   i_m_haddr               ,
    input  wire [1:0]    i_m_htrans              ,
    input  wire          i_m_hwrite              ,
    input  wire [2:0]    i_m_hsize               ,
    input  wire [2:0]    i_m_hburst              ,
    input  wire [3:0]    i_m_hprot               ,
    input  wire [31:0]   i_m_hwdata              ,
    output wire [31:0]   o_m_hrdata              ,
    output wire          o_m_hready              ,
    output wire [1:0]    o_m_hresp               ,

    output wire [3:0]    o_hprot                 ,
    output wire [31:0]   o_haddr                 ,
    output wire [1:0]    o_htrans                ,
    output wire          o_hwrite                ,
    output wire [2:0]    o_hsize                 ,
    output wire [2:0]    o_hburst                ,
    output wire [31:0]   o_hwdata                ,

    input  wire [31:0]   i_hrdata0               ,
    input  wire [1:0]    i_hresp0                ,
    input  wire          i_hreadyout0            ,
    output wire          o_hsel0                 ,

    input  wire [31:0]   i_hrdata1               ,
    input  wire [1:0]    i_hresp1                ,
    input  wire          i_hreadyout1            ,
    output wire          o_hsel1                 ,

    input  wire [31:0]   i_hrdata2               ,
    input  wire [1:0]    i_hresp2                ,
    input  wire          i_hreadyout2            ,
    output wire          o_hsel2                 ,

    input  wire [31:0]   i_hrdata3               ,
    input  wire [1:0]    i_hresp3                ,
    input  wire          i_hreadyout3            ,
    output wire          o_hsel3                 ,

    input  wire [31:0]   i_hrdata4               ,
    input  wire [1:0]    i_hresp4                ,
    input  wire          i_hreadyout4            ,
    output wire          o_hsel4                 ,

    input  wire [31:0]   i_hrdata5               ,
    input  wire [1:0]    i_hresp5                ,
    input  wire          i_hreadyout5            ,
    output wire          o_hsel5                 ,

    input  wire [31:0]   i_hrdata6               ,
    input  wire [1:0]    i_hresp6                ,
    input  wire          i_hreadyout6            ,
    output wire          o_hsel6                 ,

    input  wire [31:0]   i_hrdata7               ,
    input  wire [1:0]    i_hresp7                ,
    input  wire          i_hreadyout7            ,
    output wire          o_hsel7                 ,

    input  wire [31:0]   i_hrdata8               ,
    input  wire [1:0]    i_hresp8                ,
    input  wire          i_hreadyout8            ,
    output wire          o_hsel8                 ,

    input  wire [31:0]   i_hrdata9               ,
    input  wire [1:0]    i_hresp9                ,
    input  wire          i_hreadyout9            ,
    output wire          o_hsel9                 ,

    input  wire [31:0]   i_hrdata10              ,
    input  wire [1:0]    i_hresp10               ,
    input  wire          i_hreadyout10           ,
    output wire          o_hsel10                ,

    input  wire [31:0]   i_hrdata11              ,
    input  wire [1:0]    i_hresp11               ,
    input  wire          i_hreadyout11           ,
    output wire          o_hsel11                ,

    input  wire [31:0]   i_hrdata12              ,
    input  wire [1:0]    i_hresp12               ,
    input  wire          i_hreadyout12           ,
    output wire          o_hsel12                ,

    input  wire [31:0]   i_hrdata13              ,
    input  wire [1:0]    i_hresp13               ,
    input  wire          i_hreadyout13           ,
    output wire          o_hsel13                ,

    input  wire [31:0]   i_hrdata14              ,
    input  wire [1:0]    i_hresp14               ,
    input  wire          i_hreadyout14           ,
    output wire          o_hsel14                ,

    input  wire [31:0]   i_hrdata15              ,
    input  wire [1:0]    i_hresp15               ,
    input  wire          i_hreadyout15           ,
    output wire          o_hsel15                 
);

assign o_haddr   =  i_m_haddr   ;
assign o_htrans  =  i_m_htrans  ;
assign o_hwrite  =  i_m_hwrite  ;
assign o_hsize   =  i_m_hsize   ;
assign o_hburst  =  i_m_hburst  ;
assign o_hprot   =  i_m_hprot   ;
assign o_hwdata  =  i_m_hwdata  ;

osr_ahb_bus_decoder  u_ahb_bus_decoder (
  .i_haddr              (i_m_haddr          ),
  .i_hselin             (i_m_hsel           ),
  .o_hsel0              (o_hsel0            ),
  .o_hsel1              (o_hsel1            ),
  .o_hsel2              (o_hsel2            ),
  .o_hsel3              (o_hsel3            ),
  .o_hsel4              (o_hsel4            ),
  .o_hsel5              (o_hsel5            ),
  .o_hsel6              (o_hsel6            ),
  .o_hsel7              (o_hsel7            ),
  .o_hsel8              (o_hsel8            ),
  .o_hsel9              (o_hsel9            ),
  .o_hsel10             (o_hsel10           ),
  .o_hsel11             (o_hsel11           ),
  .o_hsel12             (o_hsel12           ),
  .o_hsel13             (o_hsel13           ),
  .o_hsel14             (o_hsel14           ),
  .o_hsel15             (o_hsel15           )
  );

osr_ahb_bus_muxs2m  u_ahb_bus_muxs2m
(
  .hclk                 (hclk               ),
  .hresetn              (hresetn            ),

  .i_clk_en             (1'h1               ),
  .i_hsel0              (o_hsel0            ),
  .i_hsel1              (o_hsel1            ),
  .i_hsel2              (o_hsel2            ),
  .i_hsel3              (o_hsel3            ),
  .i_hsel4              (o_hsel4            ),
  .i_hsel5              (o_hsel5            ),
  .i_hsel6              (o_hsel6            ),
  .i_hsel7              (o_hsel7            ),
  .i_hsel8              (o_hsel8            ),
  .i_hsel9              (o_hsel9            ),
  .i_hsel10             (o_hsel10           ),
  .i_hsel11             (o_hsel11           ),
  .i_hsel12             (o_hsel12           ),
  .i_hsel13             (o_hsel13           ),
  .i_hsel14             (o_hsel14           ),
  .i_hsel15             (o_hsel15           ),

  .i_hrdata0            (i_hrdata0          ),
  .i_hreadyout0         (i_hreadyout0       ),
  .i_hresp0             (i_hresp0           ),

  .i_hrdata1            (i_hrdata1          ),
  .i_hreadyout1         (i_hreadyout1       ),
  .i_hresp1             (i_hresp1           ),

  .i_hrdata2            (i_hrdata2          ),
  .i_hreadyout2         (i_hreadyout2       ),
  .i_hresp2             (i_hresp2           ),

  .i_hrdata3            (i_hrdata3          ),
  .i_hreadyout3         (i_hreadyout3       ),
  .i_hresp3             (i_hresp3           ),

  .i_hrdata4            (i_hrdata4          ),
  .i_hreadyout4         (i_hreadyout4       ),
  .i_hresp4             (i_hresp4           ),

  .i_hrdata5            (i_hrdata5          ),
  .i_hreadyout5         (i_hreadyout5       ),
  .i_hresp5             (i_hresp5           ),

  .i_hrdata6            (i_hrdata6          ),
  .i_hreadyout6         (i_hreadyout6       ),
  .i_hresp6             (i_hresp6           ),

  .i_hrdata7            (i_hrdata7          ),
  .i_hreadyout7         (i_hreadyout7       ),
  .i_hresp7             (i_hresp7           ),

  .i_hrdata8            (i_hrdata8          ),
  .i_hreadyout8         (i_hreadyout8       ),
  .i_hresp8             (i_hresp8           ),

  .i_hrdata9            (i_hrdata9          ),
  .i_hreadyout9         (i_hreadyout9       ),
  .i_hresp9             (i_hresp9           ),

  .i_hrdata10           (i_hrdata10         ),
  .i_hreadyout10        (i_hreadyout10      ),
  .i_hresp10            (i_hresp10          ),

  .i_hrdata11           (i_hrdata11         ),
  .i_hreadyout11        (i_hreadyout11      ),
  .i_hresp11            (i_hresp11          ),

  .i_hrdata12           (i_hrdata12         ),
  .i_hreadyout12        (i_hreadyout12      ),
  .i_hresp12            (i_hresp12          ),

  .i_hrdata13           (i_hrdata13         ),
  .i_hreadyout13        (i_hreadyout13      ),
  .i_hresp13            (i_hresp13          ),

  .i_hrdata14           (i_hrdata14         ),
  .i_hreadyout14        (i_hreadyout14      ),
  .i_hresp14            (i_hresp14          ),

  .i_hrdata15           (i_hrdata15         ),
  .i_hreadyout15        (i_hreadyout15      ),
  .i_hresp15            (i_hresp15          ),

  .o_hrdata             (o_m_hrdata         ),
  .o_hready             (o_m_hready         ),
  .o_hresp              (o_m_hresp          )
);

endmodule 
