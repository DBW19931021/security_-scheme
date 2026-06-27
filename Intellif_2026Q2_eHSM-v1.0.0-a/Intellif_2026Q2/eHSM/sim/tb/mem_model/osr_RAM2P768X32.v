//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
module osr_RAM2P768X32 (
   input  wire        CK   , 
   input  wire        CSAN , 
   input  wire [9:0]  A    , 
   output wire [31:0] DO   , 
   input  wire        CSBN , 
   input  wire [9:0]  B    , 
   input  wire        WEB  , 
   input  wire [31:0] DI      
);

sdp_bw_dc_ram #( 
    .DW               (32               ), 
    .AW               (10               ), 
    .CW               (32               ), 
    .DP               (768              )  
    ) u_sdp_bw_dc_ram (     
  .CKA                               (CK                                   ),
  .CSAN                              (CSAN                                 ),
  .A                                 (A                                    ),
  .DO                                (DO                                   ),
  .CKB                               (CK                                   ),
  .CSBN                              (CSBN                                 ),
  .B                                 (B                                    ),
  .WEB                               (WEB                                  ),
  .DI                                (DI                                   ) 
);

endmodule
