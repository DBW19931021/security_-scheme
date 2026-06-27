//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_RAM128X72 (
  output [71:0]  DO  ,
  input          CK  ,
  input          CSB ,
  input          WEB ,
  input  [6:0]   A   ,
  input  [71:0]  DI  
  );

tsp_bw_sc_ram #( 
    .DW           (72           ), 
    .AW           (7            ), 
    .CW           (72           ), 
    .DP           (128          )
    ) u_tsp_bw_sc_ram ( 
    .CK           (CK           ), // input  [        ]
    .CSB          (CSB          ), // input  [        ]
    .WEB          (WEB          ), // input  [BW-1:0  ]
    .A            (A            ), // input  [AW-1:0  ]
    .DI           (DI           ), // input  [DW-1:0  ]
    .DO           (DO           )  // output [DW-1:0  ]
);

endmodule
